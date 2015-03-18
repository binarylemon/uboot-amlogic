/*
 * READ ME:
 * This file is in charge of power manager. It need match different power control, PMU(if any) and wakeup condition.
 * We need special this file in customer/board dir to replace the default pwr_op.c.
 *
*/

/*
 * This pwr_op.c is supply for axp202.
*/
#include <i2c.h>
#include <asm/arch/ao_reg.h>
#include <arc_pwr.h>

extern void wait_uart_empty();
extern void udelay__(int i);
#ifdef CONFIG_RN5T618
void rn5t618_set_gpio(int gpio, int output);
#endif

#ifdef CONFIG_IR_REMOTE_WAKEUP
#include "irremote2arc.c"
#endif

#ifdef  CONFIG_SARADC_WAKEUP_FOR_ARC
#include "saradc2arc.c"
#endif

/*
 * i2c clock speed define for 32K and 24M mode
 */
#define I2C_CLK_SOURCE_IN_RESUME 24000000	// 24MHz
#define I2C_CLK_IN_RESUME	100000	// 100K
#define I2C_CLK_DIV_IN_RESUME (I2C_CLK_SOURCE_IN_RESUME/I2C_CLK_IN_RESUME/4)

#define I2C_CLK_SOURCE_IN_SUSPEND 32768	// 32.768KHz
#define I2C_CLK_IN_SUSPEND	1000 // 1KHz
//#define I2C_CLK_DIV_IN_SUSPEND (I2C_CLK_SOURCE_IN_SUSPEND/I2C_CLK_IN_SUSPEND/4)
/* High pulse is about 740us and low pulse is 540us if use above setting.
 * Use following setting can set high pulse to 620us and low pulse to 420us.
 */
#define I2C_CLK_DIV_IN_SUSPEND 6
/*
 * use globle virable to fast i2c speed
 */
volatile static unsigned char exit_reason = 0;

#ifdef CONFIG_RN5T618
#define I2C_RN5T618_ADDR   (0x32 << 1)
#define i2c_pmu_write_b(reg, val)       i2c_pmu_write(reg, val)
#define i2c_pmu_read_b(reg)             (unsigned  char)i2c_pmu_read_12(reg, 1)
#define i2c_pmu_read_w(reg)             (unsigned short)i2c_pmu_read_12(reg, 2)
#endif

volatile static unsigned char vbus_status;

static int gpio_sel0;
static int gpio_mask;

void printf_arc(const char *str)
{
    f_serial_puts(str);
    wait_uart_empty();
}

#define  I2C_CONTROL_REG      (volatile unsigned long *)0xc8100500
#define  I2C_SLAVE_ADDR       (volatile unsigned long *)0xc8100504
#define  I2C_TOKEN_LIST_REG0  (volatile unsigned long *)0xc8100508
#define  I2C_TOKEN_LIST_REG1  (volatile unsigned long *)0xc810050c
#define  I2C_TOKEN_WDATA_REG0 (volatile unsigned long *)0xc8100510
#define  I2C_TOKEN_WDATA_REG1 (volatile unsigned long *)0xc8100514
#define  I2C_TOKEN_RDATA_REG0 (volatile unsigned long *)0xc8100518
#define  I2C_TOKEN_RDATA_REG1 (volatile unsigned long *)0xc810051c

#define  I2C_END               0x0
#define  I2C_START             0x1
#define  I2C_SLAVE_ADDR_WRITE  0x2
#define  I2C_SLAVE_ADDR_READ   0x3
#define  I2C_DATA              0x4
#define  I2C_DATA_LAST         0x5
#define  I2C_STOP              0x6

#ifdef CONFIG_RN5T618
unsigned char hard_i2c_read8(unsigned char SlaveAddr, unsigned char RegAddr)
{    
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_STOP  << 24)            |
                             (I2C_DATA_LAST << 20)        |  // Read Data
                             (I2C_SLAVE_ADDR_READ << 16)  |
                             (I2C_START << 12)            |
                             (I2C_DATA << 8)              |  // Read RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit
    while( (*I2C_CONTROL_REG) & (1 << 2) ) {}

    return( (unsigned char)((*I2C_TOKEN_RDATA_REG0) & 0xFF) );
}

void hard_i2c_write8(unsigned char SlaveAddr, unsigned char RegAddr, unsigned char Data)
{
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_STOP << 16)             |
                             (I2C_DATA << 12)             |    // Write Data
                             (I2C_DATA << 8)              |    // Write RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (Data << 8) | (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit
    while( (*I2C_CONTROL_REG) & (1 << 2) ) {}
}

unsigned short hard_i2c_read8_16(unsigned char SlaveAddr, unsigned char RegAddr)
{
    unsigned short data;
    unsigned int ctrl;

    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_STOP << 28)             |
                             (I2C_DATA_LAST << 24)        |  // Read Data
                             (I2C_DATA << 20)  |
                             (I2C_SLAVE_ADDR_READ << 16)  |
                             (I2C_START << 12)            |
                             (I2C_DATA << 8)              |  // Read RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);    // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);     // Set the start bit
    while (1) {
        ctrl = (*I2C_CONTROL_REG);
        if (ctrl & (1 << 3)) {          // error case
            return 0;    
        }
        if (!(ctrl & (1 << 2))) {       // controller becomes idle
            break;    
        }
    }

    data = *I2C_TOKEN_RDATA_REG0;
    return data & 0xffff;
}

void i2c_pmu_write(unsigned char reg, unsigned char val)
{
    return hard_i2c_write8(I2C_RN5T618_ADDR, reg, val);
}

unsigned short i2c_pmu_read_12(unsigned int reg, int size)
{
    unsigned short val;
    if (size == 1) {
        val = hard_i2c_read8(I2C_RN5T618_ADDR, reg);
    } else {
        val = hard_i2c_read8_16(I2C_RN5T618_ADDR, reg);    
    }
    return val;
}
#endif

extern void delay_ms(int ms);
void init_I2C()
{
	unsigned v,reg;
	//struct aml_i2c_reg_ctrl* ctrl;

		//save gpio intr setting
	gpio_sel0 = readl(0xc8100084);
	gpio_mask = readl(0xc8100080);


	f_serial_puts("i2c init\n");


	//1. set pinmux
	v = readl(P_AO_RTI_PIN_MUX_REG);
	//master
	v |= ((1<<5)|(1<<6));
	v &= ~((1<<2)|(1<<24)|(1<<1)|(1<<23));
	//slave
	// v |= ((1<<1)|(1<<2)|(1<<3)|(1<<4));
	writel(v,P_AO_RTI_PIN_MUX_REG);
	udelay__(10000);


	reg = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xCFC00FFF;
	reg |= (I2C_CLK_DIV_IN_RESUME <<12);             // at 24MHz, i2c speed to 100KHz
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
	writel(0,P_AO_I2C_M_0_TOKEN_LIST0);
	writel(0,P_AO_I2C_M_0_TOKEN_LIST1);
//	delay_ms(20);
//	delay_ms(1);
	udelay__(1000);


}

void g9tv_power_off_at_24M()
{
	writel(readl(P_PERIPHS_PIN_MUX_10)&~(1 << 11),P_PERIPHS_PIN_MUX_10);
	writel(readl(P_AO_GPIO_O_EN_N)|(1 << 18),P_AO_GPIO_O_EN_N);
	writel(readl(P_AO_GPIO_O_EN_N)&~(1 << 2),P_AO_GPIO_O_EN_N);
}

void g9tv_power_on_at_24M()
{
	writel(readl(P_PERIPHS_PIN_MUX_10)&~(1 << 11),P_PERIPHS_PIN_MUX_10);
	writel(readl(P_AO_GPIO_O_EN_N)&~(1 << 18),P_AO_GPIO_O_EN_N);
	writel(readl(P_AO_GPIO_O_EN_N)&~(1 << 2),P_AO_GPIO_O_EN_N);
}

unsigned int g9tv_ref_wakeup(unsigned int flags)
{
    int delay_cnt   = 0;
    int power_status = 1;
    int prev_status;
    int battery_voltage;
    int ret = FLAG_WAKEUP_PWRKEY;
    int low_bat_cnt = 0;

#ifdef CONFIG_IR_REMOTE_WAKEUP
    //backup the remote config (on arm)
    backup_remote_register();
    //set the ir_remote to 32k mode at ARC
    init_custom_trigger();
#endif

#ifdef	CONFIG_SARADC_WAKEUP_FOR_ARC
	saradc_enable();
#endif	

    writel(readl(P_AO_GPIO_O_EN_N)|(1 << 3),P_AO_GPIO_O_EN_N);
    writel(readl(P_AO_RTI_PULL_UP_REG)|(1 << 3)|(1<<19),P_AO_RTI_PULL_UP_REG);

    do {
        if ((flags == 0x87654321) && (!power_status)) {      // suspend from uboot
            ret = FLAG_WAKEUP_PWROFF;
            exit_reason = -1;
            break;
        }


#ifdef CONFIG_IR_REMOTE_WAKEUP
		if(readl(P_AO_RTI_STATUS_REG2) == 0x4853ffff){
			break;
		}
		if(remote_detect_key()){
			exit_reason = 6;
			break;
		}
#endif

#ifdef	CONFIG_SARADC_WAKEUP_FOR_ARC
		if(1 == adc_detect_key()){
			exit_reason = 6;
			break;
		}
#endif	


	    if((readl(P_AO_RTC_ADDR1) >> 12) & 0x1) {
            exit_reason = 7;
			ret = FLAG_WAKEUP_ALARM;
            break;
        }

#ifdef CONFIG_BT_WAKEUP
        if(readl(P_PREG_PAD_GPIO0_I)&(0x1<<16)){
			exit_reason = 8;
            ret = FLAG_WAKEUP_BT;
			break;
		}
#endif
#ifdef CONFIG_WIFI_WAKEUP
			if ((flags != 0x87654321) &&(readl(P_AO_GPIO_O_EN_N)&(0x1<<22)))
			if(readl(P_PREG_PAD_GPIO0_I)&(0x1<<21)){
				exit_reason = 9;
				ret = FLAG_WAKEUP_WIFI;
				break;
			}
#endif

    } while (!(readl(0xc8100088) & (1<<8)));            // power key

	writel(1<<8,0xc810008c);
	writel(gpio_sel0, 0xc8100084);
	writel(gpio_mask,0xc8100080);

#ifdef CONFIG_IR_REMOTE_WAKEUP
	resume_remote_register();
#endif

    return ret;
}


void arc_pwr_register(struct arc_pwr_op *pwr_op)
{
//    printf_arc("%s\n", __func__);
	pwr_op->power_off_at_24M    = g9tv_power_off_at_24M;
	pwr_op->power_on_at_24M     	= g9tv_power_on_at_24M;

	pwr_op->power_off_at_32K_1  = 0;
	pwr_op->power_on_at_32K_1   = 0;
	pwr_op->power_off_at_32K_2  = 0;
	pwr_op->power_on_at_32K_2   = 0;

	pwr_op->power_off_ddr15     = 0;//rn5t618_power_off_ddr15;
	pwr_op->power_on_ddr15      = 0;//rn5t618_power_on_ddr15;

	pwr_op->shut_down           = 0;

	pwr_op->detect_key          = g9tv_ref_wakeup;
}


