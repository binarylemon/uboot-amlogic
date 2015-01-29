/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * I2C 
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/lcd_reg.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include <aml_i2c.h>
#include <amlogic/aml_lcd_extern.h>
#include "edp_anx6345.h"
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif

//#define LCD_EXT_DEBUG_INFO
#ifdef LCD_EXT_DEBUG_INFO
#define DBG_PRINT(...)		printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

#define LCD_EXTERN_DEVICE_NODE  "/lcd_extern_edp_ANX6345"
#define LCD_EXTERN_NAME			"lcd_edp_ANX6345"
#define LCD_EXTERN_TYPE			LCD_EXTERN_I2C
#define LCD_EXTERN_I2C_BUS		AML_I2C_MASTER_B

struct lcd_extern_edp_config {
	int lane_num;
	int bits;
	int link_rate;
};

static unsigned aml_i2c_bus_tmp;
static struct aml_lcd_extern_driver_t lcd_ext_driver;
static struct lcd_extern_config_t lcd_ext_config = {
	.name = LCD_EXTERN_NAME,
	.type = LCD_EXTERN_TYPE,
	.i2c_bus = LCD_EXTERN_I2C_BUS,
};
static struct lcd_extern_edp_config edp_parameter= {
	.lane_num = 1, // 1/2/4
	.bits = 0x00,  // 6bit: 0x00   8bit: 0x10
	.link_rate = 0x0a, //1.62G: 0X06, 2.7G: 0x0a, 5.4G: 0x14
};
extern int aml_i2c_xfer_slow(struct i2c_msg *msgs, int num);

static int aml_lcd_i2c_write(unsigned i2caddr, unsigned char *buff, unsigned len)
{
    int res = 0, i;
    struct i2c_msg msg[] = {
        {
        .addr = i2caddr,
        .flags = 0,
        .len = len,
        .buf = buff,
        }
    };

    DBG_PRINT("%s:", __FUNCTION__);
    for (i=0; i<len; i++) {
        DBG_PRINT(" 0x%02x", buff[i]);
    }
    DBG_PRINT(" [addr 0x%02x]\n", i2caddr);
    //res = aml_i2c_xfer(msg, 1);
    res = aml_i2c_xfer_slow(msg, 1);
    if (res < 0) {
        printf("%s: i2c transfer failed [addr 0x%02x]\n", __FUNCTION__, i2caddr);
    }

    return res;
}

static int aml_lcd_i2c_read(unsigned i2caddr, unsigned char *buff, unsigned len)
{
    int res = 0;
    struct i2c_msg msg[] = {
        {
            .addr  = i2caddr,
            .flags = 0,
            .len   = 1,
            .buf   = buff,
        },
        {
            .addr  = i2caddr,
            .flags = I2C_M_RD,
            .len   = len,
            .buf   = buff,
        }
    };
    //res = aml_i2c_xfer(msg, 2);
    res = aml_i2c_xfer_slow(msg, 2);
    if (res < 0) {
        printf("%s: i2c transfer failed [addr 0x%02x]\n", __FUNCTION__, i2caddr);
    }

    return res;
}
static int SP_TX_Write_Reg(unsigned char addr, unsigned char reg, unsigned char data)
{
	unsigned char buff[2];
	int ret;
	buff[0] = reg;
	buff[1] = data;
	addr =addr >>1;
	ret = aml_lcd_i2c_write( addr, buff, 2);
	return ret;
}
static int SP_TX_Read_Reg(unsigned char addr, unsigned char reg, unsigned char *data)
{
	int ret;
	*data = reg;
	addr =addr >>1;
	ret = aml_lcd_i2c_read(addr, data, 1);
	if(ret <0)
		return -1;
	else 
		return 0;
}
static int SP_TX_Wait_AUX_Finished(void)
{
	unsigned char c;
	unsigned char cCnt;
	cCnt = 0;

	SP_TX_Read_Reg(0x70, SP_TX_AUX_STATUS, &c);
	while(c & 0x10){//aux busy
		cCnt++;
		SP_TX_Read_Reg(0x70, SP_TX_AUX_STATUS, &c);

		if(cCnt>100){
			return 0; //aux fail
		}
	}

	return 1;//aux ok
}

static int SP_TX_AUX_DPCDRead_Bytes(unsigned char addrh, unsigned char addrm, unsigned char addrl,unsigned char cCount,unsigned char *pBuf)
{
	unsigned char c,i;

	//clr buffer
	SP_TX_Write_Reg(0x70, SP_TX_BUF_DATA_COUNT_REG, 0x80);

	//set read cmd and count
	SP_TX_Write_Reg(0x70, SP_TX_AUX_CTRL_REG, ((cCount-1) <<4)|0x09);

	//set aux address15:0
	SP_TX_Write_Reg(0x70, SP_TX_AUX_ADDR_7_0_REG, addrl);
	SP_TX_Write_Reg(0x70, SP_TX_AUX_ADDR_15_8_REG, addrm);

	//set address19:16 and enable aux
	SP_TX_Read_Reg(0x70, SP_TX_AUX_ADDR_19_16_REG, &c);
	SP_TX_Write_Reg(0x70, SP_TX_AUX_ADDR_19_16_REG, (c & 0xf0) | addrh);

	//Enable Aux
	SP_TX_Read_Reg(0x70, SP_TX_AUX_CTRL_REG2, &c);
	SP_TX_Write_Reg(0x70, SP_TX_AUX_CTRL_REG2, c|0x01);

	mdelay(5);
	if(!SP_TX_Wait_AUX_Finished())
	return 0;

	for(i =0;i<cCount;i++) {
		SP_TX_Read_Reg(0x70, SP_TX_BUF_DATA_0_REG+i, &c);
		*(pBuf+i) = c;

		if(i >= MAX_BUF_CNT)
		break;
	}

	return 1;//aux ok
}

static int  anx6345_reg_read(unsigned char reg, unsigned char *buf)
{
    int ret=0;

    return ret;
}

static int  anx6345_reg_write(unsigned char reg, unsigned char value)
{
    int ret=0;

    return ret;
}

static void anx6345_initialize(void)
{
	unsigned lane_num;
	unsigned link_rate;
	unsigned char bits;
	unsigned char device_id;
	unsigned char temp;
	unsigned char temp1;
	unsigned count = 0;
	unsigned count1 = 0;

	lane_num = edp_parameter.lane_num; //1/2/4 lane
	link_rate = edp_parameter.link_rate; //2.7G
	bits = edp_parameter.bits; //0x00: 6bit;  0x10:8bit
  	SP_TX_Write_Reg (0x72, 0x05, 0x00);

	SP_TX_Read_Reg(0x72, 0x01, &device_id);

	if(device_id==0xaa) {
		printf("ANX6345 Chip found\n\n\n");
	}	
	else {
		printf("ANX6345 Chip not found\n\n\n");
	}
	temp = device_id;
	//if aux read fail, do h/w reset,
	while((!SP_TX_AUX_DPCDRead_Bytes(0x00,0x00,0x00,1,&temp1))&&(count<200)) {
		//read fail, h/w reset
		SP_TX_Write_Reg (0x72, 0x06, 0x01);
		SP_TX_Write_Reg (0x72, 0x06, 0x00);
		SP_TX_Write_Reg (0x72, 0x05, 0x00);
		mdelay(10);
		count++;
	}

	//software reset
	SP_TX_Read_Reg(0x72, SP_TX_RST_CTRL_REG, &temp);
	SP_TX_Write_Reg(0x72, SP_TX_RST_CTRL_REG, temp | SP_TX_RST_SW_RST);
	SP_TX_Write_Reg(0x72, SP_TX_RST_CTRL_REG, temp & ~SP_TX_RST_SW_RST);

	SP_TX_Write_Reg(0x70, SP_TX_EXTRA_ADDR_REG, 0x50);//EDID address for AUX access
	SP_TX_Write_Reg(0x70, SP_TX_HDCP_CTRL, 0x00);	//disable HDCP polling mode.
	//SP_TX_Write_Reg(0x70, SP_TX_HDCP_CTRL, 0x02);	//Enable HDCP polling mode.
	SP_TX_Write_Reg(0x70, SP_TX_LINK_DEBUG_REG, 0x30);//enable M value read out

	//SP_TX_Read_Reg(0x70, SP_TX_DEBUG_REG1, &temp);
	SP_TX_Write_Reg(0x70, SP_TX_DEBUG_REG1, 0x00);//disable polling HPD

	SP_TX_Read_Reg(0x70, SP_TX_HDCP_CONTROL_0_REG, &temp);
	SP_TX_Write_Reg(0x70, SP_TX_HDCP_CONTROL_0_REG, temp | 0x03);//set KSV valid

	SP_TX_Read_Reg(0x70, SP_TX_AUX_CTRL_REG2, &temp);
	SP_TX_Write_Reg(0x70, SP_TX_AUX_CTRL_REG2, temp|0x00);//set double AUX output

	SP_TX_Write_Reg(0x72, SP_COMMON_INT_MASK1, 0xbf);//unmask pll change int
	SP_TX_Write_Reg(0x72, SP_COMMON_INT_MASK2, 0xff);//mask all int
	SP_TX_Write_Reg(0x72, SP_COMMON_INT_MASK3, 0xff);//mask all int
	SP_TX_Write_Reg(0x72, SP_COMMON_INT_MASK4, 0xff);//mask all int

	//reset AUX
	SP_TX_Read_Reg(0x72, SP_TX_RST_CTRL2_REG, &temp);
	SP_TX_Write_Reg(0x72, SP_TX_RST_CTRL2_REG, temp |SP_TX_AUX_RST);
	SP_TX_Write_Reg(0x72, SP_TX_RST_CTRL2_REG, temp & (~SP_TX_AUX_RST));

	//Chip initialization

	SP_TX_Write_Reg(0x70, SP_TX_SYS_CTRL1_REG, 0x00);
	mdelay(10);
	
		SP_TX_Write_Reg(0x72, SP_TX_VID_CTRL2_REG, bits);

	//ANX6345 chip analog setting
	SP_TX_Write_Reg(0x70, SP_TX_PLL_CTRL_REG, 0x00);				  //UPDATE: FROM 0X07 TO 0X00

	//ANX chip analog setting
	//	SP_TX_Write_Reg(0x72, ANALOG_DEBUG_REG1, 0x70); 			  //UPDATE: FROM 0XF0 TO 0X70
	SP_TX_Write_Reg(0x70, SP_TX_LINK_DEBUG_REG, 0x30);

	//force HPD
	SP_TX_Write_Reg(0x70, SP_TX_SYS_CTRL3_REG, 0x30);

	/* enable ssc function */
	SP_TX_Write_Reg(0x70, 0xA7, 0x00); 		          // disable SSC first
	SP_TX_Write_Reg(0x70, 0xD0, 0x5f);              // ssc d  0.4%, f0/4 mode
	SP_TX_Write_Reg(0x70, 0xD1, 0x00);
	SP_TX_Write_Reg(0x70, 0xD2, 0x75);              // ctrl_th
	SP_TX_Read_Reg(0x70, 0xA7, &temp);
	SP_TX_Write_Reg(0x70, 0xA7, temp | 0x10);          // enable SSC
	SP_TX_Read_Reg(0x72, 0x07, &temp);                 //reset SSC
	SP_TX_Write_Reg(0x72, 0x07, temp | 0x80);
	SP_TX_Write_Reg(0x72, 0x07, temp & (~0x80));

	//Select 2.7G
	SP_TX_Write_Reg(0x70, SP_TX_LINK_BW_SET_REG, link_rate);		//2.7g:0x0a;1.62g:0x06
	//Select 2 lanes
	SP_TX_Write_Reg(0x70, 0xa1, lane_num);

	SP_TX_Write_Reg(0x70, SP_TX_LINK_TRAINING_CTRL_REG, SP_TX_LINK_TRAINING_CTRL_EN);
	mdelay(5);
	SP_TX_Read_Reg(0x70, SP_TX_LINK_TRAINING_CTRL_REG, &temp);
	/* UPDATE: FROM 0X01 TO 0X80 */
	while((temp&0x80)!=0) {
		//debug_puts("Waiting...\n");
		mdelay(5);
		count1 ++;
		if(count1 > 100) {
			printf("ANX6345 Link training fail\n");
			break;
		}
		SP_TX_Read_Reg(0x70, SP_TX_LINK_TRAINING_CTRL_REG, &temp);
	}



	SP_TX_Write_Reg(0x72, 0x12, 0x2c);
	SP_TX_Write_Reg(0x72, 0x13, 0x06);
	SP_TX_Write_Reg(0x72, 0x14, 0x00);
	SP_TX_Write_Reg(0x72, 0x15, 0x06);
	SP_TX_Write_Reg(0x72, 0x16, 0x02);
	SP_TX_Write_Reg(0x72, 0x17, 0x04);
	SP_TX_Write_Reg(0x72, 0x18, 0x26);
	SP_TX_Write_Reg(0x72, 0x19, 0x50);
	SP_TX_Write_Reg(0x72, 0x1a, 0x04);
	SP_TX_Write_Reg(0x72, 0x1b, 0x00);
	SP_TX_Write_Reg(0x72, 0x1c, 0x04);
	SP_TX_Write_Reg(0x72, 0x1d, 0x18);
	SP_TX_Write_Reg(0x72, 0x1e, 0x00);
	SP_TX_Write_Reg(0x72, 0x1f, 0x10);
	SP_TX_Write_Reg(0x72, 0x20, 0x00);
	SP_TX_Write_Reg(0x72, 0x21, 0x28);
	
	SP_TX_Write_Reg(0x72, 0x11, 0x03);
//enable BIST. In normal mode, don't need to config this reg
//if want to open BIST,must setting right dat 0x11-0x21 base lcd timing.	
//	SP_TX_Write_Reg(0x72, 0x0b, 0x09);//colorbar:08,graystep:09
	SP_TX_Write_Reg(0x72, 0x08, 0x81);		//SDR:0x81;DDR:0x8f

	//force HPD and stream valid
	SP_TX_Write_Reg(0x70, 0x82, 0x33);
  
    printf("%s\n", __FUNCTION__);
}

#ifdef LCD_EXT_DEBUG_INFO
static unsigned char DP_TX_Read_Reg(unsigned char addr, unsigned char reg)
{
	int ret;
	unsigned char *data;
	data = &reg;
	addr =addr >>1;
	ret = aml_lcd_i2c_read(addr, data, 1);
	if(ret <0)
		return -1;
	else 
		return *data;
}

static void test_clk(void)
{
	unsigned char val;
	val = DP_TX_Read_Reg(0x70, 0x80);	//clk detect
	DBG_PRINT("clk detect: %2x\n", val);
	
	val = DP_TX_Read_Reg(0x70, 0x81);	//clk change
	DBG_PRINT("clk change: %2x\nwait for 100ms...\n", val);	
	
	SP_TX_Write_Reg(0x70, 0x81, 0x40);
	mdelay(100);
	val = DP_TX_Read_Reg(0x70, 0x81);	//clk change
	DBG_PRINT("clk change: %2x\n\n", val);
}

static void test_anx9804(void)
{
	unsigned char val;
	DBG_PRINT("\nenter anx9804 test.\n");
	
	test_clk();
	
	val = DP_TX_Read_Reg(0x72, 0x23);	//video status
	DBG_PRINT("video status: %2x\n\n", val);
	
	val = DP_TX_Read_Reg(0x72, 0x24);	//total lines low
	DBG_PRINT("total lines low: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x25);	//total lines high
	DBG_PRINT("total lines high: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x26);	//active lines low
	DBG_PRINT("active lines low: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x27);	//active lines high
	DBG_PRINT("active lines high: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x29);	//vertical sync width
	DBG_PRINT("vsync width: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x2a);	//vertical back porch
	DBG_PRINT("vertical back porch: %2x\n\n", val);
	
	val = DP_TX_Read_Reg(0x72, 0x2b);	//total pixels low
	DBG_PRINT("total pixels low: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x2c);	//total pixels high
	DBG_PRINT("total pixels high: %2x\n", val);
	val = DP_TX_Read_Reg(0x72, 0x2d);	//active pixels low
	DBG_PRINT("active pixels low: %2x\n", val);	
	val = DP_TX_Read_Reg(0x72, 0x2e);	//active pixels high
	DBG_PRINT("active pixels high: %2x\n", val);
	val = DP_TX_Read_Reg(0x72, 0x31);	//horizon sync width low
	DBG_PRINT("hsync width low: %2x\n", val);
	val = DP_TX_Read_Reg(0x72, 0x32);	//horizon sync width high
	DBG_PRINT("hsync width high: %2x\n", val);
	val = DP_TX_Read_Reg(0x72, 0x33);	//horizon back porch low
	DBG_PRINT("horizon back porch low: %2x\n", val);
	val = DP_TX_Read_Reg(0x72, 0x34);	//horizon back porch high
	DBG_PRINT("horizon back porch high: %2x\n\n", val);
}
#endif

static int  anx6345_init(void)
{
	anx6345_initialize();
	
#ifdef LCD_EXT_DEBUG_INFO
	test_anx9804();
#endif
	
	return 0;
}

static int  anx6345_remove(void)
{
    int ret=0;
    printf("%s\n", __FUNCTION__);
    return ret;
}

static struct aml_lcd_extern_pinmux_t aml_lcd_extern_pinmux_set[] = {
    {.reg = 9, .mux = ((1 << 28) | (1 << 29)),},
};

static struct aml_lcd_extern_pinmux_t aml_lcd_extern_pinmux_clr[] = {
    {.reg = 0, .mux = ((1 << 7) | (1 << 10)),},
	{.reg = 6, .mux = ((1 << 20) | (1 << 21)),},
	{.reg = 8, .mux = ((1 << 19) | (1 << 20) |  (1 << 21) | (1 << 22) |(1 << 28)),},
};

static int aml_lcd_extern_port_init(void)
{
    int i;
    unsigned pinmux_reg, pinmux_data;
    int ret=0;

    for (i=0; i<ARRAY_SIZE(aml_lcd_extern_pinmux_set); i++) {
        pinmux_reg = PERIPHS_PIN_MUX_0+aml_lcd_extern_pinmux_set[i].reg;
        pinmux_data = aml_lcd_extern_pinmux_set[i].mux;
        WRITE_LCD_CBUS_REG(pinmux_reg, READ_LCD_CBUS_REG(pinmux_reg) | pinmux_data);
    }
    for (i=0; i<ARRAY_SIZE(aml_lcd_extern_pinmux_clr); i++) {
        pinmux_reg = PERIPHS_PIN_MUX_0+aml_lcd_extern_pinmux_clr[i].reg;
        pinmux_data = ~(aml_lcd_extern_pinmux_clr[i].mux);
        WRITE_LCD_CBUS_REG(pinmux_reg, READ_LCD_CBUS_REG(pinmux_reg) & pinmux_data);
    }

    return ret;
}

static int aml_lcd_extern_change_i2c_bus(unsigned aml_i2c_bus)
{
    int ret=0;
    extern struct aml_i2c_platform g_aml_i2c_plat;

    g_aml_i2c_plat.master_no = aml_i2c_bus;
    ret = aml_i2c_init();

    return ret;
}

static int aml_lcd_extern_init(void)
{
    int ret=0;
    extern struct aml_i2c_platform g_aml_i2c_plat;

    aml_i2c_bus_tmp = g_aml_i2c_plat.master_no;

    aml_lcd_extern_port_init();
    aml_lcd_extern_change_i2c_bus(lcd_ext_config.i2c_bus);
    ret =  anx6345_init();
    aml_lcd_extern_change_i2c_bus(aml_i2c_bus_tmp);
    
    return ret;
}

static int aml_lcd_extern_remove(void)
{
    int ret=0;
    extern struct aml_i2c_platform g_aml_i2c_plat;

    aml_i2c_bus_tmp = g_aml_i2c_plat.master_no;

    aml_lcd_extern_port_init();
    aml_lcd_extern_change_i2c_bus(lcd_ext_config.i2c_bus);
    ret =  anx6345_remove();
    aml_lcd_extern_change_i2c_bus(aml_i2c_bus_tmp);

    return ret;
}

#ifdef CONFIG_OF_LIBFDT
static int get_lcd_extern_config(char *dt_addr)
{
	int nodeoffset;
	char * propdata;
	int ret;
	int value;
	char *of_node = LCD_EXTERN_DEVICE_NODE;
	struct lcd_extern_config_t *pdata = &lcd_ext_config;
	
	if (get_lcd_extern_dt_data(dt_addr, of_node, pdata) != 0){
		printf("[error] %s probe: failed to get dt data\n", LCD_EXTERN_NAME);
    	return -1;
	}
	nodeoffset = fdt_path_offset(dt_addr, of_node);
	if(nodeoffset < 0) {
		printf("dts: not find  node %s.\n",LCD_EXTERN_NAME);
		return ret;
	}
	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "lane_num", NULL);
	if(propdata == NULL){
		edp_parameter.lane_num = 1;
		printf("faild to get %s lane_num\n", LCD_EXTERN_NAME);
	}
	else {
		edp_parameter.lane_num = (unsigned short)(be32_to_cpup((u32*)propdata));
	}
	DBG_PRINT ("lane_num =%d\n", edp_parameter.lane_num);
	
	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "bits", NULL);
	if(propdata == NULL){
		edp_parameter.bits = 0x00;
		printf("faild to get %s bits\n", LCD_EXTERN_NAME);
	}
	else {
		value = (unsigned short)(be32_to_cpup((u32*)propdata));
		if (value == 0)
			edp_parameter.bits = 0x00;
		else if  (value == 1)
			edp_parameter.bits = 0x10;
		else
			edp_parameter.bits = 0x00;
	}
	DBG_PRINT ("bits =%d\n", edp_parameter.bits);
	
	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "link_rate", NULL);
	if(propdata == NULL){
		edp_parameter.link_rate = 0x0a;
		printf("faild to get %s link_rate\n", LCD_EXTERN_NAME);
	}
	else {
		value = (unsigned short)(be32_to_cpup((u32*)propdata));
		if (value == 0)
			edp_parameter.link_rate = 0x06;
		else if (value == 1)
			edp_parameter.link_rate = 0x0a;
		else if (value == 2)
			edp_parameter.link_rate = 0x14;
		else 
			edp_parameter.link_rate = 0x0a;
	}	
	DBG_PRINT ("link_rate =0x%02x\n", edp_parameter.link_rate);	
	return ret;
}
#endif

static struct aml_lcd_extern_driver_t lcd_ext_driver = {
    .name = LCD_EXTERN_NAME,
    .type = LCD_EXTERN_TYPE,
    .reg_read =  anx6345_reg_read,
    .reg_write =  anx6345_reg_write,
    .power_on = aml_lcd_extern_init,
    .power_off = aml_lcd_extern_remove,
    .init_on_cmd_8 = NULL,
    .init_off_cmd_8 = NULL,
#ifdef CONFIG_OF_LIBFDT
    .get_lcd_ext_config = get_lcd_extern_config,
#endif
};

struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void)
{
    return &lcd_ext_driver;
}
