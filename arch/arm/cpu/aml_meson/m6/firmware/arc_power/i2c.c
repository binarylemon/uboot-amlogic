#include <config.h>
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/dmc.h>
#include <asm/arch/ddr.h>
#include <asm/arch/memtest.h>
#include <asm/arch/pctl.h>

#define AML_I2C_CTRL_CLK_DELAY_MASK			0x3ff
#define AML_I2C_SLAVE_ADDR_MASK				0xff
#define AML_I2C_SLAVE_ADDR_MASK_7BIT   (0x7F)
#define AML_I2C_MAX_TOKENS		8
#define ETIMEDOUT 110
#define EIO       111

#define 	I2C_IDLE		0
#define 	I2C_RUNNING	1

#define CONFIG_AW_AXP20

struct aml_i2c {
	unsigned int		cur_slave_addr;
	unsigned char		token_tag[AML_I2C_MAX_TOKENS];
	unsigned int 		msg_flags;
}i2c;

#define I2C_M_TEN		       0x0010	/* this is a ten bit chip address */
#define I2C_M_RD		       0x0001	/* read data, from slave to master */
#define I2C_M_NOSTART		   0x4000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR 0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	 0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK		 0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN		 0x0400	/* length will be first received byte */

typedef unsigned short __u16;
typedef unsigned char __u8;

struct i2c_msg {
	__u16 addr;	/* slave address			*/
	__u16 flags;
	__u16 len;		/* msg length				*/
	__u8 *buf;		/* pointer to msg data			*/
};

enum aml_i2c_token {
	TOKEN_END,
	TOKEN_START,
	TOKEN_SLAVE_ADDR_WRITE,
	TOKEN_SLAVE_ADDR_READ,
	TOKEN_DATA,
	TOKEN_DATA_LAST,
	TOKEN_STOP
};

struct aml_i2c_platform{
	unsigned int 		wait_count;/*i2c wait ack timeout = 
											wait_count * wait_ack_interval */
	unsigned int 		wait_ack_interval;
	unsigned int 		wait_read_interval;
	unsigned int 		wait_xfer_interval;
	unsigned int 		master_no;
	unsigned int		use_pio;/*0: hardware i2c, 1: manual pio i2c*/
	unsigned int		master_i2c_speed;
};

struct aml_i2c_platform g_aml_i2c_plat = {
    .wait_count         = 1000000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no          = 0,
    .use_pio            = 0,
    .master_i2c_speed   = 160
};

#define v_outs(s,v) serial_puts(s);serial_put_dword(v);

static int aml_i2c_do_address(unsigned int addr)
{
	i2c.cur_slave_addr = addr & AML_I2C_SLAVE_ADDR_MASK_7BIT;
	writel(readl(P_AO_I2C_M_0_SLAVE_ADDR)&(~AML_I2C_SLAVE_ADDR_MASK), P_AO_I2C_M_0_SLAVE_ADDR);
	writel(readl(P_AO_I2C_M_0_SLAVE_ADDR)|(i2c.cur_slave_addr<<1), P_AO_I2C_M_0_SLAVE_ADDR);
//	writel((i2c.cur_slave_addr<<1), P_AO_I2C_M_0_SLAVE_ADDR);
	return 0;
}
static void  aml_i2c_clear_token_list(void)
{	
	int i;
	writel(0, P_AO_I2C_M_0_TOKEN_LIST0);
	writel(0, P_AO_I2C_M_0_TOKEN_LIST1);
	for(i=0; i<AML_I2C_MAX_TOKENS; i++)
	  	i2c.token_tag[i] = TOKEN_END;
}

static void aml_i2c_set_token_list(void)
{
	int i;
	unsigned int token_reg=0;	
	for(i=0; i<AML_I2C_MAX_TOKENS; i++)
		token_reg |= i2c.token_tag[i]<<(i*4);
	writel(token_reg, P_AO_I2C_M_0_TOKEN_LIST0);
}

#if 0
void udelay(int i)
{
	int delays = i ;//* 24;
	unsigned base= readl(P_AO_TIMERE_REG);
	//while(((readl(P_AO_TIMERE_REG)-base)&0xffffff) < (  delays&0xffffff)); //reg value is 24bit case
	while(((readl(P_AO_TIMERE_REG)-base)) < (  delays));
}
#else
void udelay(int i)
{
    int delays = 0;
    for(delays=0;delays<i;delays++)
    {
        asm("mov r0,r0");
    }
}
#endif

static void aml_i2c_start_token_xfer()
{
	writel(readl(P_AO_I2C_M_0_CONTROL_REG)&(~1), P_AO_I2C_M_0_CONTROL_REG);
	writel(readl(P_AO_I2C_M_0_CONTROL_REG)|1, P_AO_I2C_M_0_CONTROL_REG);
	udelay(g_aml_i2c_plat.wait_xfer_interval);
}

static void aml_i2c_stop()
{
	aml_i2c_clear_token_list();
	i2c.token_tag[0]=TOKEN_STOP;
	aml_i2c_set_token_list();
	aml_i2c_start_token_xfer();
}

static int aml_i2c_check_error()
{
    if (readl(P_AO_I2C_M_0_CONTROL_REG)&(1<<3))
		return -EIO;
	else
		return 0;
}

/*poll status*/
static int aml_i2c_wait_ack()
{
	int i;
	for(i=0; i<g_aml_i2c_plat.wait_count; i++) {
		udelay(g_aml_i2c_plat.wait_ack_interval);
	if (((readl(P_AO_I2C_M_0_CONTROL_REG)&(1<<2))>>2)==I2C_IDLE)
			return aml_i2c_check_error();
	}
	return -ETIMEDOUT;			
}

#define min_t(a,b) ((a) < (b) ? (a):(b))
	
static void aml_i2c_get_read_data(unsigned char *buf, unsigned len)
{
	int i;
	unsigned long rdata0 = readl(P_AO_I2C_M_0_RDATA_REG0); //pmaster->i2c_token_rdata_0;
	unsigned long rdata1 = readl(P_AO_I2C_M_0_RDATA_REG1); //pmaster->i2c_token_rdata_1;

	for(i=0; i< min_t(len, AML_I2C_MAX_TOKENS>>1); i++)
		*buf++ = (rdata0 >> (i*8)) & 0xff;

	for(; i< min_t(len, AML_I2C_MAX_TOKENS); i++) 
		*buf++ = (rdata1 >> ((i - (AML_I2C_MAX_TOKENS>>1))*8)) & 0xff;
}

static int aml_i2c_read(unsigned char *buf,unsigned len) 
{
	int i;
	int ret;
	unsigned rd_len;
	int tagnum=0;

	aml_i2c_clear_token_list();
	
	if(!(i2c.msg_flags & I2C_M_NOSTART)){
		i2c.token_tag[tagnum++]=TOKEN_START;
		i2c.token_tag[tagnum++]=TOKEN_SLAVE_ADDR_READ;

		aml_i2c_set_token_list();

		aml_i2c_start_token_xfer();

		udelay(g_aml_i2c_plat.wait_ack_interval);
		
		ret = aml_i2c_wait_ack();
		if(ret<0)
			return ret;	
		aml_i2c_clear_token_list();
	}
	
	while(len){
		tagnum = 0;
		rd_len = min_t(len,AML_I2C_MAX_TOKENS);
		if(rd_len == 1)
			i2c.token_tag[tagnum++]=TOKEN_DATA_LAST;
		else{
			for(i=0; i<rd_len-1; i++)
				i2c.token_tag[tagnum++]=TOKEN_DATA;
			if(len > rd_len)
				i2c.token_tag[tagnum++]=TOKEN_DATA;
			else
				i2c.token_tag[tagnum++]=TOKEN_DATA_LAST;
		}
		aml_i2c_set_token_list();
		aml_i2c_start_token_xfer();

		udelay(g_aml_i2c_plat.wait_ack_interval);
		
		ret = aml_i2c_wait_ack();
		if(ret<0)
			return ret;	
		
		aml_i2c_get_read_data( buf, rd_len);
		len -= rd_len;
		buf += rd_len;

		udelay(g_aml_i2c_plat.wait_read_interval);
		aml_i2c_clear_token_list();
	}
	return 0;
}

static void aml_i2c_fill_data(unsigned char *buf, unsigned len)
{
	int i;
	unsigned int wdata0 = 0;
	unsigned int wdata1 = 0;

	for(i=0; i< min_t(len, AML_I2C_MAX_TOKENS>>1); i++)
		wdata0 |= (*buf++) << (i*8);

	for(; i< min_t(len, AML_I2C_MAX_TOKENS); i++)
		wdata1 |= (*buf++) << ((i - (AML_I2C_MAX_TOKENS>>1))*8); 

	writel(wdata0, P_AO_I2C_M_0_WDATA_REG0);
	writel(wdata1, P_AO_I2C_M_0_WDATA_REG1);
}

static int aml_i2c_write(unsigned char *buf, unsigned len) 
{
	int i;
	int ret;
	unsigned wr_len;
	int tagnum=0;

	aml_i2c_clear_token_list();
	
	if(!(i2c.msg_flags & I2C_M_NOSTART)){
		i2c.token_tag[tagnum++]=TOKEN_START;
		i2c.token_tag[tagnum++]=TOKEN_SLAVE_ADDR_WRITE;
	}
	while(len){
		wr_len = min_t(len, AML_I2C_MAX_TOKENS-tagnum);
		for(i=0; i<wr_len; i++)
			i2c.token_tag[tagnum++]=TOKEN_DATA;
		
		aml_i2c_set_token_list();
		
		aml_i2c_fill_data(buf, wr_len);
		
		aml_i2c_start_token_xfer();

		len -= wr_len;
		buf += wr_len;
		tagnum = 0;

		ret = aml_i2c_wait_ack();
		if(ret<0)
			return ret;
		
		aml_i2c_clear_token_list();
    	}
	return 0;
}
int do_msgs(struct i2c_msg *msgs,int num)
{
	int i,ret;
	struct i2c_msg* p;
	ret = 0;
	for (i = 0; !ret && i < num; i++) {
		p = &msgs[i];
	
		ret = aml_i2c_do_address( p->addr);
		if (ret || !p->len)
		{
			continue;
		}
		if (p->flags & I2C_M_RD)
		{
				ret = aml_i2c_read(p->buf, p->len);
		}
		else{
			ret = aml_i2c_write(p->buf, p->len);
		}
	}
	
	aml_i2c_stop();

	/* Return the number of messages processed, or the error code*/
	if (ret == 0)
		return num;
	else 
		return ret;
}
#ifdef CONFIG_AW_AXP20
#define I2C_AXP202_ADDR   (0x68 >> 1)
void i2c_axp202_write(unsigned char reg, unsigned char val)
{
    unsigned char buff[2];
    buff[0] = reg;
    buff[1] = val;

	struct i2c_msg msg[] = {
        {
        .addr  = I2C_AXP202_ADDR,
        .flags = 0,
        .len   = 2,
        .buf   = buff,
        }
    };

    if (do_msgs(msg, 1) < 0) {
        serial_puts("i2c transfer failed\n");
    }
}
unsigned char i2c_axp202_read(unsigned char reg)
{
    unsigned char val = 0;
    struct i2c_msg msgs[] = {
        {
            .addr = I2C_AXP202_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = I2C_AXP202_ADDR,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &val,
        }
    };

    if (do_msgs(msgs, 2)< 0) {
       serial_puts("<error>");
    }

    return val;
}
#endif//CONFIG_AW_AXP20

#ifdef CONFIG_ACT8942QJ233_PMU
#define I2C_ACT8942QJ233_ADDR   (0x5B)
void i2c_act8942_write(unsigned char reg, unsigned char val)
{
    unsigned char buff[2];
    buff[0] = reg;
    buff[1] = val;

	struct i2c_msg msg[] = {
        {
        .addr  = I2C_ACT8942QJ233_ADDR,
        .flags = 0,
        .len   = 2,
        .buf   = buff,
        }
    };

    if (do_msgs(msg, 1) < 0) {
        serial_puts("i2c transfer failed\n");
    }
}
unsigned char i2c_act8942_read(unsigned char reg)
{
    unsigned char val = 0;
    struct i2c_msg msgs[] = {
        {
            .addr = I2C_ACT8942QJ233_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = I2C_ACT8942QJ233_ADDR,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &val,
        }
    };

    if (do_msgs(msgs, 2)< 0) {
       serial_puts("<error>");
    }

    return val;
}
#endif//CONFIG_ACT8942QJ233_PMU

void init_I2C()
{
	unsigned v,speed,reg;
	struct aml_i2c_reg_ctrl* ctrl;

	//1. set pinmux
	v = readl(P_AO_RTI_PIN_MUX_REG);
	//master
	v |= ((1<<5)|(1<<6));
	//slave
	// v |= ((1<<1)|(1<<2)|(1<<3)|(1<<4));
	writel(v,P_AO_RTI_PIN_MUX_REG);

	//-------------------------------	
	//reset
//	writel(readl(P_AO_RTI_GEN_CNTL_REG0)|(3<<18),P_AO_RTI_GEN_CNTL_REG0);
//	writel(readl(P_AO_RTI_GEN_CNTL_REG0)&(~(3<<18)),P_AO_RTI_GEN_CNTL_REG0);
//	delay_ms(20);
	//---------------------------------
	//config	  
 	//v=20; //(32000/speed) >>2) speed:400K
 	v = 12; //for saving time cost
	reg = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
	reg |= (v <<12);
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
//	delay_ms(20);
	delay_ms(1);
#ifdef CONFIG_ACT8942QJ233_PMU
	v = i2c_act8942_read(0);
	if(v == 0xFF || v == 0)
#endif
#ifdef CONFIG_AW_AXP20
	v = i2c_axp202_read(0x12);
	if(v != 0x5F )
#endif
		serial_puts("Error: I2C init failed!\n");
}

/***************************/
/*******AXP202 PMU*********/
/**************************/
#ifdef CONFIG_AW_AXP20
#define POWER20_DCDC_MODESET        (0x80)
#define POWER20_DC2OUT_VOL          (0x23)
#define POWER20_DC3OUT_VOL          (0x27)
#define POWER20_LDO24OUT_VOL        (0x28)
#define POWER20_LDO3OUT_VOL         (0x29)


unsigned char vddio;
unsigned char avdd33;
unsigned char _3gvcc;
unsigned char ddr15_reg12;//reg=0x12
unsigned char ddr15_reg23;//reg=0x23
unsigned char dcdc_reg;

void dump_pmu_reg()
{
	int i,data;
	for(i=0;i<0x95;i++)
	{
		data=i2c_axp202_read(i);		
		serial_put_hex(i,8);
		serial_puts("	");
		serial_put_hex(data,8);
		serial_puts("\n");
	}

}

void power_off_avdd25()
{
	unsigned char data;
	data = i2c_axp202_read(0x12);
	data &= ~(1<<6);//ldo3
	i2c_axp202_write(0x12,data);
	
	udelay(100);
}

void power_on_avdd25()
{
	unsigned char data;
	data = i2c_axp202_read(0x12);
	data |= 1<<6;//ldo3
	i2c_axp202_write(0x12,data);
	
	udelay(100);
}

void power_off_vddio()
{
	unsigned char data;
	vddio = i2c_axp202_read(0x12);
	data = vddio & 0xfe;//EXTEN
	i2c_axp202_write(0x12,data);

	udelay(100);

}

void power_on_vddio()
{
	unsigned char data;

	data = vddio | 0x01;//EXTEN
	i2c_axp202_write(0x12,data);

	udelay(100);

}

void power_off_avdd33()
{
	unsigned char data;
	avdd33 = i2c_axp202_read(0x12);
	data = avdd33 & 0xf7;
	i2c_axp202_write(0x12,data);

	udelay(100);

}

void power_on_avdd33()
{
	unsigned char data;

	data = avdd33 | 0x08;
	i2c_axp202_write(0x12,data);

	udelay(100);

}


void power_off_3gvcc()
{
	unsigned char data;
	_3gvcc = i2c_axp202_read(0x91);
	data = _3gvcc & 0x0f;//low or hight is enable
	//data = _3gvcc | 0x07;
	i2c_axp202_write(0x91,data);

	udelay(100);

}

void power_on_3gvcc()
{
	unsigned char data;

	data = _3gvcc;// | 0xa0;
	//data = _3gvcc & 0xf8;
	i2c_axp202_write(0x91,data);

	udelay(100);

}

void power_down_ddr15()
{
	unsigned char data;

	ddr15_reg23 = i2c_axp202_read(0x23);
	data = ddr15_reg23 & 0xc0;
	data = data | 0x18;// 1.3v//1.5
	i2c_axp202_write(0x23,data);

	ddr15_reg12 = i2c_axp202_read(0x12);
	data = ddr15_reg12 & 0xef;//Disable DC-DC2 switch
	i2c_axp202_write(0x12,data);
}

void power_up_ddr15()
{
	unsigned char data;

	ddr15_reg12 |= 0x10;
	i2c_axp202_write(0x12, ddr15_reg12);

	i2c_axp202_write(0x23, ddr15_reg23);
}

void dc_dc_pwm_switch(unsigned int flag)
{
	unsigned char data;
	if(flag)
	{
		//data = i2c_axp202_read(0x80);
		//data |= (unsigned char)(3<<1);
		i2c_axp202_write(0x80,dcdc_reg);
	}
	else//PFM
	{
		dcdc_reg = i2c_axp202_read(0x80);
		data = dcdc_reg & (unsigned char)(~(3<<1));
		i2c_axp202_write(0x80,data);
	}
	udelay(100);//>1ms@32k
}

int check_all_regulators(void)
{
	int ret = 0;
	unsigned char reg_data;
	
	f_serial_puts("Check all regulator\n");
	
	//check work mode for DCDC2 & DCDC3
	reg_data = i2c_axp202_read(POWER20_DCDC_MODESET);
	if(!((reg_data&(1<<1) )&&(reg_data&(1<<2) )))
	{
		f_serial_puts("Use constant PWM for DC-DC2 & DC-DC3. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		reg_data |= ((1<<1)|(1<<2));
		i2c_axp202_write(POWER20_DCDC_MODESET, reg_data);	//use constant PWM for DC-DC2 & DC-DC3
		udelay(10000);
		ret = 1;
	}

	reg_data = i2c_axp202_read(0x81);	//check switch for  LDO3 under voltage protect
	if(reg_data & (1<<2))
	{
		f_serial_puts("Disable LDO3 under voltage protect. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		reg_data &= ~(1<<2);	//disable LDO3 under voltage protect
		i2c_axp202_write(0x81, reg_data);	
		udelay(10000);
		ret = 1;
	}

	//check for DCDC2(DDR3_1.5V)
	reg_data = i2c_axp202_read(POWER20_DC2OUT_VOL);
	if(reg_data != 0x20)
	{
		i2c_axp202_write(POWER20_DC2OUT_VOL, 0x20);	//set DCDC2(DDR3_1.5V) to 1.500V
		f_serial_puts("Set DCDC2(DDR3_1.5V) to 1.500V. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		udelay(10000);
		ret = 1;
	}

	//check for DCDC3(VDD_AO)
	reg_data = i2c_axp202_read(POWER20_DC3OUT_VOL);
	if(reg_data != 0x10)
	{
		i2c_axp202_write(POWER20_DC3OUT_VOL, 0x10);	//set DCDC3(VDD_AO) to 1.100V
		f_serial_puts("Set DCDC3(VDD_AO) to 1.100V. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		udelay(10000);
		ret = 1;
	}

	//check for LDO2(VDDIO_AO) & LDO4(AVDD3.3V)
	reg_data = i2c_axp202_read(POWER20_LDO24OUT_VOL);
	if(reg_data != 0xcf)
	{
		i2c_axp202_write(POWER20_LDO24OUT_VOL, 0xcf);	//set LDO2(VDDIO_AO) to 3.000V; set LDO4(AVDD3.3V) to 3.300V
		f_serial_puts("Set  LDO2(VDDIO_AO) to 3.000V; Set LDO4(AVDD3.3V) to 3.300V. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		udelay(10000);
		ret = 1;
	}

	//check for LDO3(AVDD2.5V)
	reg_data = i2c_axp202_read(POWER20_LDO3OUT_VOL);
	if(reg_data != 0x48)
	{
		i2c_axp202_write(POWER20_LDO3OUT_VOL, 0x48);	//set LDO3(AVDD2.5V) to 2.500V;
		f_serial_puts("set LDO3(AVDD2.5V) to 2.500V. But the register is 0x");
		serial_put_hex(reg_data, 8);
		f_serial_puts(" before\n");
 		wait_uart_empty();
		udelay(10000);
		ret = 1;
	}
	return ret;
}


#endif//CONFIG_AW_AXP20

/**************************/
/**************************/
#ifdef CONFIG_ACT8942QJ233_PMU
void vddio_off()
{
	unsigned char reg3;
	//To disable the regulator, set ON[ ] to 1 first then clear it to 0.
	reg3 = i2c_act8942_read(0x42);	//add by Elvis for cut down VDDIO
	reg3 |= (1<<7);	
	i2c_act8942_write(0x42,reg3);
	reg3 = i2c_act8942_read(0x42);
	reg3 &= ~(1<<7);	
	i2c_act8942_write(0x42,reg3);
}

void vddio_on()
{
	unsigned char reg3;
	//To enable the regulator, clear ON[ ] to 0 first then set to 1.
	reg3 = i2c_act8942_read(0x42);	//add by Elvis, Regulator3 Enable for VDDIO
	reg3 &= ~(1<<7);	
	i2c_act8942_write(0x42,reg3);
	reg3 = i2c_act8942_read(0x42);
	reg3 |= (1<<7);	
	i2c_act8942_write(0x42,reg3);
}

//Regulator7 Disable for cut down HDMI_VCC
void reg7_off()
{
	unsigned char data;
	//To disable the regulator, set ON[ ] to 1 first then clear it to 0.
	data = i2c_act8942_read(0x65);
	data |= (1<<7);	
	i2c_act8942_write(0x65,data);
	data = i2c_act8942_read(0x65);
	data &= ~(1<<7);	
	i2c_act8942_write(0x65,data);
}

//Regulator7 Enable for power on HDMI_VCC
void reg7_on()
{
	unsigned char data;
	//To enable the regulator, clear ON[ ] to 0 first then set to 1.
	data = i2c_act8942_read(0x65);
	data &= ~(1<<7);	
	i2c_act8942_write(0x65,data);
	data = i2c_act8942_read(0x65);
	data |= (1<<7);	
	i2c_act8942_write(0x65,data);
}
//Regulator7 Disable for cut down HDMI_VCC
void reg5_off()
{
	unsigned char data;
	//To disable the regulator, set ON[ ] to 1 first then clear it to 0.
	data = i2c_act8942_read(0x55);
	data |= (1<<7);	
	i2c_act8942_write(0x55,data);
	data = i2c_act8942_read(0x55);
	data &= ~(1<<7);	
	i2c_act8942_write(0x55,data);
}

//Regulator7 Enable for power on HDMI_VCC
void reg5_on()
{
	unsigned char data;
	//To enable the regulator, clear ON[ ] to 0 first then set to 1.
	data = i2c_act8942_read(0x55);
	data &= ~(1<<7);	
	i2c_act8942_write(0x55,data);
	data = i2c_act8942_read(0x55);
	data |= (1<<7);	
	i2c_act8942_write(0x55,data);
}
//Regulator7 Disable for cut down HDMI_VCC
void reg6_off()
{
	unsigned char data;
	//To disable the regulator, set ON[ ] to 1 first then clear it to 0.
	data = i2c_act8942_read(0x61);
	data |= (1<<7);	
	i2c_act8942_write(0x61,data);
	data = i2c_act8942_read(0x61);
	data &= ~(1<<7);	
	i2c_act8942_write(0x61,data);
}

//Regulator7 Enable for power on HDMI_VCC
void reg6_on()
{
	unsigned char data;
	//To enable the regulator, clear ON[ ] to 0 first then set to 1.
	data = i2c_act8942_read(0x61);
	data &= ~(1<<7);	
	i2c_act8942_write(0x61,data);
	data = i2c_act8942_read(0x61);
	data |= (1<<7);	
	i2c_act8942_write(0x61,data);
}
#endif //CONFIG_ACT8942QJ233_PMU

