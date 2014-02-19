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



#define CONFIG_IR_REMOTE_WAKEUP 1//for M8 MBox

#ifdef CONFIG_IR_REMOTE_WAKEUP
#include "irremote2arc.c"
#endif

/*
 * i2c clock speed define for 32K and 24M mode
 */
#define I2C_SUSPEND_SPEED    6                  // speed = 8KHz / I2C_SUSPEND_SPEED
#define I2C_RESUME_SPEED    60                  // speed = 6MHz / I2C_RESUME_SPEED

/*
 * use globle virable to fast i2c speed
 */
static unsigned char exit_reason = 0;

#ifdef CONFIG_RN5T618
#define I2C_RN5T618_ADDR   (0x32 << 1)
#define i2c_pmu_write_b(reg, val)       i2c_pmu_write(reg, val)
#define i2c_pmu_read_b(reg)             (unsigned  char)i2c_pmu_read_12(reg, 1)
#define i2c_pmu_read_w(reg)             (unsigned short)i2c_pmu_read_12(reg, 2)
#endif

#ifdef CONFIG_AML1216
#define AML1216_DCDC1                1
#define AML1216_DCDC2                2
#define AML1216_DCDC3                3
#define AML1216_BOOST                4
#define I2C_AML1216_ADDR                (0x35 << 1)
#define i2c_pmu_write_b(reg, val)       hard_i2c_write168(I2C_AML1216_ADDR, reg, val); 
#define i2c_pmu_write_w(reg, val)       hard_i2c_write1616(I2C_AML1216_ADDR, reg, val); 
#define i2c_pmu_read_b(reg)             (unsigned  char)hard_i2c_read168(I2C_AML1216_ADDR, reg)
#define i2c_pmu_read_w(reg)             (unsigned short)hard_i2c_read1616(I2C_AML1216_ADDR, reg)

/*
 * Amlogic PMU suspend/resume interface
 */
#define PWR_UP_SW_ENABLE_ADDR       0x82
#define PWR_DN_SW_ENABLE_ADDR       0x84

#define   AML1216_POWER_EXT_DCDC_VCCK_BIT   11
#define   AML1216_POWER_LDO6_BIT            8
#define   AML1216_POWER_LDO5_BIT            7
#define   AML1216_POWER_LDO4_BIT            6
#define   AML1216_POWER_LDO3_BIT            5

#define   AML1216_POWER_LDO2_BIT            4
#define   AML1216_POWER_DC4_BIT             0
#define   AML1216_POWER_DC3_BIT             3
#define   AML1216_POWER_DC2_BIT             2
#define   AML1216_POWER_DC1_BIT             1

#endif  /* CONFIG_AML1216 */

static unsigned char vbus_status;

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

#ifdef CONFIG_AML1216
#define I2C_WAIT_CNT        (24 * 8 * 1000)
int hard_i2c_check_error(void)
{
    if (*I2C_CONTROL_REG & 0x08) {
        printf_arc("-- i2c error, CTRL:");
        serial_put_hex(*I2C_CONTROL_REG, 32);
        printf_arc("\n");
        return -1;
    }
    return 0;
}

int hard_i2c_wait_complete(void)
{
    int delay = 0;

    while (delay < I2C_WAIT_CNT) {
        if (!((*I2C_CONTROL_REG) & (1 << 2))) {     // idle
            break;
        }
        delay++;
    }
    if (delay >= I2C_WAIT_CNT) {
        printf_arc("i2c timeout\n");
    }
    return hard_i2c_check_error();
}

unsigned short hard_i2c_read1616(unsigned char SlaveAddr, unsigned short RegAddr)
{
    unsigned short data;
    unsigned int ctrl;

    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_DATA_LAST << 28)   |
                             (I2C_DATA << 24)        |  // Read Data
                             (I2C_SLAVE_ADDR_READ << 20)  |
                             (I2C_START << 16)            |
                             (I2C_DATA << 12)  |
                             (I2C_DATA << 8)              |  // Read RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);
    (*I2C_TOKEN_LIST_REG1) = (I2C_END);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);    // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);     // Set the start bit
    hard_i2c_wait_complete();

    data = *I2C_TOKEN_RDATA_REG0;
    return data & 0xffff;
}

unsigned char hard_i2c_read168(unsigned char SlaveAddr, unsigned short RegAddr)
{
    unsigned char data;

    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_END << 28)              |
                             (I2C_DATA_LAST << 24)        |  // Read Data
                             (I2C_SLAVE_ADDR_READ << 20)  |
                             (I2C_START << 16)            |
                             (I2C_DATA << 12)             |
                             (I2C_DATA << 8)              |  // Read RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);
    (*I2C_TOKEN_LIST_REG1) = (0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (RegAddr << 0); 
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit

    hard_i2c_wait_complete();

    data = *I2C_TOKEN_RDATA_REG0 & 0xff;
    return data;
}

void hard_i2c_write1616(unsigned char SlaveAddr, unsigned short RegAddr, unsigned short Data)
{
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_END  << 24)             |
                             (I2C_DATA << 20)             |    // Write Data
                             (I2C_DATA << 16)             |    // Write Data
                             (I2C_DATA << 12)             |
                             (I2C_DATA << 8)              |    // Write RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);
    (*I2C_TOKEN_LIST_REG1) = (0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (Data << 16) | (RegAddr << 0); 
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit

    hard_i2c_wait_complete();
}

void hard_i2c_write168(unsigned char SlaveAddr, unsigned short RegAddr, unsigned char Data)
{
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_END  << 20)             |
                             (I2C_DATA << 16)             |    // Write Data
                             (I2C_DATA << 12)             |
                             (I2C_DATA << 8)              |    // Write RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);
    (*I2C_TOKEN_LIST_REG1) = (0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (Data << 16) | (RegAddr << 0); 
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit

    hard_i2c_wait_complete();
}

int find_idx(int start, int target, int step, int size)
{
    int i = 0;  
    do { 
        if (start >= target) {
            break;    
        }    
        start += step;
        i++; 
    } while (i < size);
    return i;
}
#endif  /* CONFIG_AML1216 */ 

extern void delay_ms(int ms);
void init_I2C()
{
	unsigned v,speed,reg;
	struct aml_i2c_reg_ctrl* ctrl;

		//save gpio intr setting
	gpio_sel0 = readl(0xc8100084);
	gpio_mask = readl(0xc8100080);

	if(!(readl(0xc8100080) & (1<<8)))//kernel enable gpio interrupt already?
	{
		writel(readl(0xc8100084) | (1<<18) | (1<<16) | (0x3<<0),0xc8100084);
		writel(readl(0xc8100080) | (1<<8),0xc8100080);
		writel(1<<8,0xc810008c); //clear intr
	}

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
	reg &= 0xFFC00FFF;
	reg |= (I2C_RESUME_SPEED <<12);             // at 24MHz, i2c speed to 100KHz
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
//	delay_ms(20);
//	delay_ms(1);
	udelay__(1000);

#ifdef CONFIG_PLATFORM_HAS_PMU
    v = i2c_pmu_read_b(0x0000);                 // read version
    if (v == 0x00 || v == 0xff)
	{
        serial_put_hex(v, 8);
        f_serial_puts(" Error: I2C init failed!\n");
    }
	else
	{
		serial_put_hex(v, 8);
		f_serial_puts("Success.\n");
	}
#endif
}

#ifdef CONFIG_RN5T618
static unsigned char reg_ldo     = 0;
static unsigned char reg_ldo_rtc = 0;
static unsigned char dcdc1_ctrl  = 0;
static unsigned char charge_timeout = 0;

void rn5t618_set_bits(unsigned char addr, unsigned char bit, unsigned char mask)
{
    unsigned char val;
    val = i2c_pmu_read_b(addr);
    val &= ~(mask);
    val |=  (bit & mask);
    i2c_pmu_write_b(addr, val);
}

#define power_off_vddio_ao28()  rn5t618_set_bits(0x0044, 0x00, 0x01)    // LDO1
#define power_on_vddio_ao28()   rn5t618_set_bits(0x0044, 0x01, 0x01)
#define power_off_vddio_ao18()  rn5t618_set_bits(0x0044, 0x00, 0x02)    // LDO2
#define power_on_vddio_ao18()   rn5t618_set_bits(0x0044, 0x02, 0x02)
#define power_off_vcc18()       rn5t618_set_bits(0x0044, 0x00, 0x04)    // LDO3
#define power_on_vcc18()        rn5t618_set_bits(0x0044, 0x04, 0x04)
#define power_off_vcc28()       rn5t618_set_bits(0x0044, 0x00, 0x08)    // LDO4
#define power_on_vcc28()        rn5t618_set_bits(0x0044, 0x08, 0x08)
#define power_off_avdd18()      rn5t618_set_bits(0x0044, 0x00, 0x10)    // LDO5
#define power_on_avdd18()       rn5t618_set_bits(0x0044, 0x10, 0x10)

#define LDO2_BIT                0x02
#define LDO3_BIT                0x04
#define LDO4_BIT                0x08
#define LDO5_BIT                0x10

#define MODE_PWM                1
#define MODE_PSM                2
#define MODE_AUTO               0

inline void power_off_ddr15() 
{
    rn5t618_set_bits(0x0030, 0x00, 0x01);    // DCDC3
}
inline void power_on_ddr15() 
{
    rn5t618_set_bits(0x0030, 0x01, 0x01);
}

int get_charging_state()
{
    unsigned char status;
    status = i2c_pmu_read_b(0x00bd);
    return (status & 0xc0) ? 1 : 0;
}

void rn5t618_shut_down()
{
#ifdef CONFIG_RESET_TO_SYSTEM
    rn5t618_set_bits(0x0007, 0x00, 0x01);                   // clear flag
#endif
    rn5t618_set_gpio(0, 1);
    rn5t618_set_gpio(1, 1);
    udelay__(100 * 1000);
    rn5t618_set_bits(0x00EF, 0x00, 0x10);                     // disable coulomb counter
    rn5t618_set_bits(0x00E0, 0x00, 0x01);                     // disable fuel gauge 
    rn5t618_set_bits(0x000f, 0x00, 0x01);
    rn5t618_set_bits(0x000E, 0x01, 0x01);
    while (1);
}

int find_idx(int start, int target, int step, int size)
{
    int i = 0;  
    do { 
        if (start >= target) {
            break;    
        }    
        start += step;
        i++; 
    } while (i < size);
    return i;
}

void rn5t618_set_dcdc_voltage(int dcdc, int voltage)
{
    int addr;
    int idx_to;
    addr = 0x35 + dcdc;
    idx_to = find_idx(6000, voltage * 10, 125, 256);            // step is 12.5mV
    i2c_pmu_write_b(addr, idx_to);
#if 1
    printf_arc("set dcdc");
    serial_put_hex(addr-36, 4);
    wait_uart_empty();
    printf_arc(" to 0x");
    serial_put_hex(idx_to, 8);
    wait_uart_empty();
    printf_arc("\n");
#endif
}

void rn5t618_set_dcdc_mode(int dcdc, int mode)
{
    int addr = 0x2C + (dcdc - 1) * 2;
    unsigned char bits = (mode << 4) & 0xff; 

    rn5t618_set_bits(addr, bits, 0x30);
    udelay__(50);
}

static unsigned char gpio_dir = 0xff;
static unsigned char gpio_out = 0;

void rn5t618_set_gpio(int gpio, int output)
{
    int val = output ? 1 : 0;
    if (gpio < 0 || gpio > 3) {
        return ;
    }
    if (gpio_dir == 0xff) {
        gpio_dir = i2c_pmu_read_b(0x0090);
    }
    if (!gpio_out) {
        gpio_out = i2c_pmu_read_b(0x0091);    
    }
    gpio_out &= ~(1 << gpio);
    gpio_out |= (val << gpio);
    i2c_pmu_write_b(0x0091, gpio_out);                      // set output
    gpio_dir |= (1 << gpio);
    i2c_pmu_write_b(0x0090, gpio_dir);                      // set output mode 
    udelay__(50);
}

void rn5t618_get_gpio(int gpio, unsigned char *val)
{
    int value;
    if (gpio < 0 || gpio > 3) {
        return ;
    }
    value = i2c_pmu_read_b(0x0097);                         // read status
    *val = (value & (1 << gpio)) ? 1 : 0;
}

#endif

#ifdef CONFIG_RN5T618
int pmu_get_battery_voltage(void)
{
    unsigned char val[2];
    int result;

    result = i2c_pmu_read_w(0x006A);
    val[0] = result & 0xff;
    val[1] = (result >> 8) & 0xff;
    result = (val[0] << 4) | (val[1] & 0x0f);
    result = (result * 5000) / 4096;                        // resolution: 1.221mV

    return result | (val[0] << 24 | val[1] << 16);
}

#if defined(CONFIG_ENABLE_PMU_WATCHDOG) || defined(CONFIG_RESET_TO_SYSTEM)
void pmu_feed_watchdog(unsigned int flags)
{
    i2c_pmu_write_b(0x0013, 0x00);                      // clear watch dog IRQ
}
#endif /* CONFIG_ENABLE_PMU_WATCHDOG */

void rn5t618_power_off_at_24M()
{
    i2c_pmu_write_b(0x66, 0x29);                                        // select vbat channel
#if 0
    rn5t618_get_gpio(1, &vbus_status);
    if (!vbus_status) {
    //  rn5t618_set_gpio(1, 1);                                         // close boost
    }
#endif
	rn5t618_set_gpio(1, 1);                                             // close vccx2
    rn5t618_set_gpio(0, 1);                                             // close vccx3
    udelay__(500);

#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_SUSPEND_VOLTAGE);
#endif
#endif
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    rn5t618_set_dcdc_mode(2, MODE_PSM);
    printf_arc("dc2 set to PSM\n");
    rn5t618_set_dcdc_mode(3, MODE_PSM);
    printf_arc("dc3 set to PSM\n");
#endif
#endif
    reg_ldo     = i2c_pmu_read_b(0x0044);
    reg_ldo_rtc = i2c_pmu_read_b(0x0045);
    dcdc1_ctrl  = i2c_pmu_read_b(0x002c);
    reg_ldo &= ~(LDO3_BIT | LDO4_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);                                   // close LDO3 & 4

    dcdc1_ctrl &= ~(0x01);                                              // close DCDC1, vcck
    i2c_pmu_write_b(0x002c, dcdc1_ctrl);

#if defined(CONFIG_ENABLE_PMU_WATCHDOG) || defined(CONFIG_RESET_TO_SYSTEM)
    i2c_pmu_write_b(0x000b, 0x0c);                      // time out to 1s
    i2c_pmu_write_b(0x0013, 0x00);                      // clear watch dog IRQ
    i2c_pmu_write_b(0x0012, 0x40);                      // enable watchdog
#endif
    printf_arc("enter 32K\n");
}

void rn5t618_power_on_at_24M()                                          // need match power sequence of  power_off_at_24M
{
    printf_arc("enter 24MHz. reason:");

#if defined(CONFIG_ENABLE_PMU_WATCHDOG) || defined(CONFIG_RESET_TO_SYSTEM)
    i2c_pmu_write_b(0x0012, 0x00);                      // disable watchdog
    i2c_pmu_write_b(0x000b, 0x01);                      // disable watchdog 
    i2c_pmu_write_b(0x0013, 0x00);                      // clear watch dog IRQ
#endif

    serial_put_hex(exit_reason, 32);
    wait_uart_empty();
    printf_arc("\n");


    rn5t618_set_gpio(3, 1);                                             // should open LDO1.2v before open VCCK
    udelay__(6 * 1000);                                                 // must delay 25 ms before open vcck
    dcdc1_ctrl |= 0x01;
    i2c_pmu_write_b(0x002c, dcdc1_ctrl);                                // open DCDC1, vcck

    reg_ldo |= (LDO3_BIT | LDO4_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);
	
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    rn5t618_set_dcdc_mode(3, MODE_PWM);
    printf_arc("dc3 set to pwm\n");
    rn5t618_set_dcdc_mode(2, MODE_PWM);
    printf_arc("dc2 set to pwm\n");
#endif
#endif
#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_VOLTAGE);
#endif
#endif
	rn5t618_set_gpio(1, 0);                                     // close vccx2
#if 0
    if (!vbus_status) {
        //rn5t618_set_gpio(1, 0);
    }
#endif
    if (charge_timeout & 0x20) {
        printf_arc("charge timeout detect, reset charger\n");
        rn5t618_set_bits(0x00C5, 0x00, 0x20);                   // clear flag
        rn5t618_set_bits(0x00B3, 0x00, 0x03);
        udelay__(10 * 1000);
        rn5t618_set_bits(0x00B3, 0x03, 0x03);
    }
    rn5t618_set_gpio(3, 0);                                     // close ldo 1.2v when vcck is opened 
}

void rn5t618_power_off_at_32K_1()
{
    unsigned int reg;                               // change i2c speed to 1KHz under 32KHz cpu clock
    unsigned int sleep_flag = readl(P_AO_RTI_STATUS_REG2);

	reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
	if  (readl(P_AO_RTI_STATUS_REG2) == 0x87654321) {
    	reg |= (10 << 12);              // suspended from uboot 
    } else {
		reg |= (5 << 12);               // suspended from kernel
    }
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
	udelay__(10);

    reg_ldo &= ~(LDO5_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);                   // close LDO5, AVDD1.8

    reg_ldo_rtc &= ~(0x10);
    i2c_pmu_write_b(0x0045, reg_ldo_rtc);               // close ext DCDC 3.3v
}

void rn5t618_power_on_at_32K_1()        // need match power sequence of  power_off_at_32K_1
{
    unsigned int    reg;

    reg_ldo_rtc |= 0x10;
    i2c_pmu_write_b(0x0045, reg_ldo_rtc);               // open ext DCDC 3.3v

    reg_ldo |= (LDO5_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);                   // open LDO5, AVDD1.8

	reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
	reg |= (I2C_RESUME_SPEED << 12);
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
	udelay__(10);
	
}

void rn5t618_power_off_at_32K_2()       // If has nothing to do, just let null
{
    // TODO: add code here
}

void rn5t618_power_on_at_32K_2()        // need match power sequence of power_off_at_32k
{
    // TODO: add code here
}

unsigned int rn5t618_detect_key(unsigned int flags)
{
    int delay_cnt   = 0;
    int power_status;
    int prev_status;
    int battery_voltage;
    int ret = 0;
//    int gpio_sel0;
//    int gpio_mask;
    int low_bat_cnt = 0;

#ifdef CONFIG_IR_REMOTE_WAKEUP
    //backup the remote config (on arm)
    backup_remote_register();
    //set the ir_remote to 32k mode at ARC
    init_custom_trigger();
#endif

    writel(readl(P_AO_GPIO_O_EN_N)|(1 << 3),P_AO_GPIO_O_EN_N);
    writel(readl(P_AO_RTI_PULL_UP_REG)|(1 << 3)|(1<<19),P_AO_RTI_PULL_UP_REG);
/*
	//save gpio intr setting
	gpio_sel0 = readl(0xc8100084);
	gpio_mask = readl(0xc8100080);

	writel(readl(0xc8100084) | (1<<18) | (1<<16) | (0x3<<0),0xc8100084);
	writel(readl(0xc8100080) | (1<<8),0xc8100080);
	writel(1<<8,0xc810008c); //clear intr
*/
	prev_status = get_charging_state();
    do {
        /*
         * when extern power status has changed, we need break 
         * suspend loop and resume system.
         */
	    power_status = get_charging_state();
        if ((flags == 0x87654321) && (!power_status)) {      // suspend from uboot
            ret = 1;
            exit_reason = 8;
            break;
        }
        if (power_status ^ prev_status) {
            exit_reason = 1;
            break;
        }
        delay_cnt++;

    #if defined(CONFIG_ENABLE_PMU_WATCHDOG) || defined(CONFIG_RESET_TO_SYSTEM)
        pmu_feed_watchdog(flags);
    #endif
        if (delay_cnt >= 3000) {
            if (flags != 0x87654321 && !power_status) {
                /*
                 * when battery voltage is too low but no power supply and suspended from kernel,
                 * we need to break suspend loop to resume system, then system will shut down
                 */
                battery_voltage = pmu_get_battery_voltage();
                if (((battery_voltage & 0xffff) < 3480) && (battery_voltage & 0xffff)) {
                    low_bat_cnt++;
                    if (low_bat_cnt >= 3) {
                        exit_reason = (battery_voltage & 0xffff0000) | 2;
                        break;
                    }
                } else {
                    low_bat_cnt = 0;
                }
                if ((readl(0xc8100088) & (1<<8))) {        // power key
                    exit_reason = 3;
                    break;
                }
            } else if (power_status) {
                charge_timeout = i2c_pmu_read_b(0x00c5);
                if (charge_timeout & 0x20) {
                    exit_reason = 4;
                    break;    
                } else {
                    charge_timeout = 0;    
                }
                if ((readl(0xc8100088) & (1<<8))) {        // power key
                    exit_reason = 5;
                    break;
                }
            }
            delay_cnt = 0;
        }

#ifdef CONFIG_IR_REMOTE_WAKEUP
        if(remote_detect_key()){
            exit_reason = 6;
        	break;
        }
#endif

	    if((readl(P_AO_RTC_ADDR1) >> 12) & 0x1) {
            exit_reason = 7;
		    break;
        }

    } while (!(readl(0xc8100088) & (1<<8))/* && (readl(P_AO_GPIO_I)&(1<<3))*/);            // power key

	writel(1<<8,0xc810008c);
	writel(gpio_sel0, 0xc8100084);
	writel(gpio_mask,0xc8100080);

#ifdef CONFIG_IR_REMOTE_WAKEUP
	resume_remote_register();
#endif

    return ret;
}
#endif

#ifdef CONFIG_AML1216
static unsigned char otg_status = 0;

void aml1216_set_bits(unsigned short addr, unsigned char bit, unsigned char mask)
{
    unsigned char val;
    val = i2c_pmu_read_b(addr);
    val &= ~(mask);
    val |=  (bit & mask);
    i2c_pmu_write_b(addr, val);
}

void aml1216_set_pfm(int dcdc, int en)
{
    unsigned char val;
    if (dcdc < 1 || dcdc > 4 || en > 1 || en < 0) {
        return ;    
    }
    switch(dcdc) {
    case AML1216_DCDC1:
        val = i2c_pmu_read_b(0x003b);
        if (en) {
            val |=  (1 << 5);                                   // pfm mode
        } else {
            val &= ~(1 << 5);                                   // pwm mode
        }
        i2c_pmu_write_b(0x003b, val);
        break;
    case AML1216_DCDC2:
        val = i2c_pmu_read_b(0x0044);
        if (en) {
            val |=  (1 << 5);    
        } else {
            val &= ~(1 << 5);   
        }
        i2c_pmu_write_b(0x0044, val);
        break;
    case AML1216_DCDC3:
        val = i2c_pmu_read_b(0x004e);
        if (en) {
            val |=  (1 << 7);    
        } else {
            val &= ~(1 << 7);   
        }
        i2c_pmu_write_b(0x004e, val);
        break;
    case AML1216_BOOST:
        val = i2c_pmu_read_b(0x0028);
        if (en) {
            val |=  (1 << 0);    
        } else {
            val &= ~(1 << 0);   
        }
        i2c_pmu_write_b(0x0028, val);
        break;
    default:
        break;
    }
     udelay__(1000); 
}

int aml1216_set_gpio(int pin, int val)
{
    unsigned char data;

    if (pin <= 0 || pin > 3 || val > 1 || val < 0) {
        printf_arc("ERROR, invalid input value\n");
        return -1;
    }
    if (val < 2) {
        data = ((val ? 1 : 0) << (pin));
    } else {
        return -1;
    }
    aml1216_set_bits(0x0013, data, (1 << pin));
    udelay__(50);
    return 0;
}

static unsigned int VDDEE_voltage_table[] = {                   // voltage table of VDDEE
    1184, 1170, 1156, 1142, 1128, 1114, 1100, 1086, 
    1073, 1059, 1045, 1031, 1017, 1003, 989, 975, 
    961,  947,  934,  920,  906,  892,  878, 864, 
    850,  836,  822,  808,  794,  781,  767, 753 
};

int find_idx_by_vddEE_voltage(int voltage, unsigned int *table)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (voltage >= table[i]) {
            break;    
        }
    }
    if (voltage == table[i]) {
        return i;    
    }
    return i - 1;
}


int aml1216_set_vddEE_voltage(int voltage)
{
    int addr = 0x005d;
    int idx_to, idx_cur;
    unsigned current;
    unsigned char val;
 
    val = i2c_pmu_read_b(addr);
    idx_cur = ((val & 0xfc) >> 2);
    idx_to = find_idx_by_vddEE_voltage(voltage, VDDEE_voltage_table);

    current = idx_to*5; 

#if 1                                                           // for debug
    printf_arc("\nVDDEE current set from 0x");
    serial_put_hex(idx_cur, 8);
    printf_arc(" to 0x");
    serial_put_hex(idx_to, 8);
    printf_arc(", addr:0x");
    serial_put_hex(addr, 8);
    printf_arc("\n");
#endif
    
    val &= ~0xfc;
    val |= (idx_to << 2);

    i2c_pmu_write_b(addr, val);
    __udelay(5 * 100);
}

int aml1216_get_battery_voltage()
{
    unsigned short val; 
    int result = 0;
    
    val = i2c_pmu_read_w(0x00AF); 
    
    result = (val * 4800) / 4096;
    
    return result;
}

int aml1216_get_charge_status(void)
{
    unsigned short val = 0;
    int vbus_vol = 0, dcin_vol = 0;
    /*
     * tmp work around for charge status register can't update
     */
    i2c_pmu_write_b(0x00AA, 0xc1);
    i2c_pmu_write_b(0x009A, 0x28);
    __udelay(100);
    val = i2c_pmu_read_w(0x00B1);
    if (val & 0x1000) {
        dcin_vol = 0;    
    } else {
        dcin_vol = (val * 12800) / 4096;
    }
    i2c_pmu_write_b(0x00AA, 0xc2);
    i2c_pmu_write_b(0x009A, 0x28);
    __udelay(100);
    val = i2c_pmu_read_w(0x00B1);
    if (val & 0x1000) {
        vbus_vol = 0;    
    } else {
        vbus_vol = (val * 6400) / 4096;
    }
    if (vbus_vol >= 4500 || dcin_vol >= 4500) {
        return 1;    
    } else {
        return 0;
    }
}

// modify by endy
void aml_pmu_power_ctrl(int on, int bit_mask)
{
    unsigned short addr = on ? PWR_UP_SW_ENABLE_ADDR : PWR_DN_SW_ENABLE_ADDR;
    i2c_pmu_write_w(addr, (unsigned short)bit_mask);
    udelay(100);
}

#define power_off_ao18()            aml_pmu_power_ctrl(0, 1 << AML1216_POWER_LDO3_BIT)
#define power_on_ao18()             aml_pmu_power_ctrl(1, 1 << AML1216_POWER_LDO3_BIT)
#define power_off_vcc18()           aml_pmu_power_ctrl(0, 1 << AML1216_POWER_LDO4_BIT)
#define power_on_vcc18()            aml_pmu_power_ctrl(1, 1 << AML1216_POWER_LDO4_BIT) 
#define power_off_vcc_cam()         aml_pmu_power_ctrl(0, 1 << AML1216_POWER_LDO6_BIT)
#define power_on_vcc_cam()          aml_pmu_power_ctrl(1, 1 << AML1216_POWER_LDO6_BIT)

#define power_off_vcc28()           aml_pmu_power_ctrl(0, 1 << AML1216_POWER_LDO5_BIT)
#define power_on_vcc28()            aml_pmu_power_ctrl(1, 1 << AML1216_POWER_LDO5_BIT) 

#define power_off_vcck()            aml_pmu_power_ctrl(0, 1 << AML1216_POWER_DC1_BIT)  
#define power_on_vcck()             aml_pmu_power_ctrl(1, 1 << AML1216_POWER_DC1_BIT)

#define power_off_vcc33()           aml_pmu_power_ctrl(0, 1 << AML1216_POWER_DC3_BIT)  
#define power_on_vcc33()            aml_pmu_power_ctrl(1, 1 << AML1216_POWER_DC3_BIT)
#define power_off_vcc50()           aml_pmu_power_ctrl(0, 1 << AML1216_POWER_DC4_BIT)
#define power_on_vcc50()            aml_pmu_power_ctrl(1, 1 << AML1216_POWER_DC4_BIT)
#define power_off_vcck12()          aml_pmu_power_ctrl(0, 1 << AML1216_POWER_EXT_DCDC_VCCK_BIT)
#define power_on_vcck12()           aml_pmu_power_ctrl(1, 1 << AML1216_POWER_EXT_DCDC_VCCK_BIT)


void aml1216_power_off_at_24M()
{
    otg_status = i2c_pmu_read_b(0x0019);
    i2c_pmu_write_b(0x0019, 0x10);                                      // cut usb output
    aml1216_set_gpio(1, 1);                                             // close vccx3
    aml1216_set_gpio(2, 1);                                             // close vccx2
    udelay__(500);

#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    aml1216_set_vddEE_voltage(CONFIG_VDDAO_SUSPEND_VOLTAGE);
#endif
#endif
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    aml1216_set_pfm(2, 1);
    printf_arc("dc2 set to PFM\n");
#endif
#endif

    power_off_vcc_cam();                                                // close LDO6
    power_off_vcc28();                                                  // close LDO5
    power_off_vcck();                                                   // close DCDC1, vcck
    printf_arc("enter 32K\n");
}

void aml1216_power_on_at_24M()
{
    printf_arc("enter 24MHz. reason:");

    serial_put_hex(exit_reason, 32);
    wait_uart_empty();
    printf_arc("\n");

    aml1216_set_gpio(3, 1);                                             // should open LDO1.2v before open VCCK
    udelay__(6 * 1000);                                                 // must delay 25 ms before open vcck

    power_on_vcck();                                                    // open DCDC1, vcck
    power_on_vcc28();                                                   // open LDO5, VCC2.8
    power_on_vcc_cam();
    
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    aml1216_set_pfm(2, 0);
    printf_arc("dc2 set to pwm\n");
#endif
#endif
#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    aml1216_set_vddEE_voltage(CONFIG_VDDAO_VOLTAGE);
#endif
#endif
    aml1216_set_gpio(2, 0);                                     // open vccx2

    aml1216_set_gpio(3, 0);                                     // close ldo 1.2v when vcck is opened
    udelay__(1 * 1000);
    i2c_pmu_write_b(0x0019, otg_status);
}

void aml1216_power_off_at_32K_1()
{
    unsigned int reg;                               // change i2c speed to 1KHz under 32KHz cpu clock
    unsigned int sleep_flag = readl(P_AO_RTI_STATUS_REG2);

    reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
    reg &= 0xFFC00FFF;
    if  (readl(P_AO_RTI_STATUS_REG2) == 0x87654321) {
        reg |= (10 << 12);              // suspended from uboot 
    } else {
        reg |= (5 << 12);               // suspended from kernel
    }
    writel(reg,P_AO_I2C_M_0_CONTROL_REG);
    udelay__(10);

    power_off_vcc18();
    power_off_vcc33();                                  // close DCDC3, VCC3.3v
}

void aml1216_power_on_at_32K_1()
{
    unsigned int    reg;

    power_on_vcc33();                                // open ext DCDC 3.3v
    power_on_vcc18();

    reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
    reg &= 0xFFC00FFF;
    reg |= (I2C_RESUME_SPEED << 12);
    writel(reg,P_AO_I2C_M_0_CONTROL_REG);
    udelay__(10);
}

aml1216_power_off_at_32K_2()
{
       // TODO: add code here
}

void aml1216_power_on_at_32K_2()
{
       // TODO: add code here
}


void aml1216_shut_down()
{
    aml1216_set_gpio(1, 1);
    aml1216_set_gpio(2, 1);
    aml1216_set_gpio(3, 1);
    udelay__(100 * 1000);
    i2c_pmu_write_b(0x0081, 0x20);                                      // soft power off
}

unsigned int aml1216_detect_key(unsigned int flags)
{
    int delay_cnt   = 0;
    int power_status;
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

    writel(readl(P_AO_GPIO_O_EN_N)|(1 << 3),P_AO_GPIO_O_EN_N);
    writel(readl(P_AO_RTI_PULL_UP_REG)|(1 << 3)|(1<<19),P_AO_RTI_PULL_UP_REG);

    prev_status = aml1216_get_charge_status();
    do {
        /*
         * when extern power status has changed, we need break 
         * suspend loop and resume system.
         */
        power_status = aml1216_get_charge_status();
        if (power_status ^ prev_status) {
            if (flags == 0x87654321) {      // suspend from uboot
                ret = FLAG_WAKEUP_PWROFF;
            }
            exit_reason = 1;
            break;
        }
        delay_cnt++;

    #if defined(CONFIG_ENABLE_PMU_WATCHDOG) || defined(CONFIG_RESET_TO_SYSTEM)
        //pmu_feed_watchdog(flags);
    #endif
        if (delay_cnt >= 3000) {
            if (flags != 0x87654321 && !power_status) {
                /*
                 * when battery voltage is too low but no power supply and suspended from kernel,
                 * we need to break suspend loop to resume system, then system will shut down
                 */
                battery_voltage = aml1216_get_battery_voltage();
                if (((battery_voltage & 0xffff) < 3480) && (battery_voltage & 0xffff)) {
                    low_bat_cnt++;
                    if (low_bat_cnt >= 3) {
                        exit_reason = (battery_voltage & 0xffff0000) | 2;
                        break;
                    }
                } else {
                    low_bat_cnt = 0;
                }
                if ((readl(0xc8100088) & (1<<8))) {        // power key
                    exit_reason = 3;
                    break;
                }
            }
            delay_cnt = 0;
        }

#ifdef CONFIG_IR_REMOTE_WAKEUP
        if(remote_detect_key()){
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

    } while (!(readl(0xc8100088) & (1<<8)));            // power key

    writel(1<<8,0xc810008c);
    writel(gpio_sel0, 0xc8100084);
    writel(gpio_mask,0xc8100080);

#ifdef CONFIG_IR_REMOTE_WAKEUP
    resume_remote_register();
#endif
    return ret;
}
#endif  /* CONFIG_AML1216 */

void arc_pwr_register(struct arc_pwr_op *pwr_op)
{
//    printf_arc("%s\n", __func__);
#ifdef CONFIG_RN5T618
	pwr_op->power_off_at_24M    = rn5t618_power_off_at_24M;
	pwr_op->power_on_at_24M     = rn5t618_power_on_at_24M;

	pwr_op->power_off_at_32K_1  = rn5t618_power_off_at_32K_1;
	pwr_op->power_on_at_32K_1   = rn5t618_power_on_at_32K_1;
	pwr_op->power_off_at_32K_2  = rn5t618_power_off_at_32K_2;
	pwr_op->power_on_at_32K_2   = rn5t618_power_on_at_32K_2;

	pwr_op->power_off_ddr15     = 0;//rn5t618_power_off_ddr15;
	pwr_op->power_on_ddr15      = 0;//rn5t618_power_on_ddr15;

	pwr_op->shut_down           = rn5t618_shut_down;

	pwr_op->detect_key          = rn5t618_detect_key;
 #endif
 #ifdef CONFIG_AML1216
 	pwr_op->power_off_at_24M    = aml1216_power_off_at_24M;
	pwr_op->power_on_at_24M     = aml1216_power_on_at_24M;
	pwr_op->power_off_at_32K_1  = aml1216_power_off_at_32K_1;
	pwr_op->power_on_at_32K_1   = aml1216_power_on_at_32K_1;
	pwr_op->power_off_at_32K_2  = aml1216_power_off_at_32K_2;
	pwr_op->power_on_at_32K_2   = aml1216_power_on_at_32K_2;
	pwr_op->power_off_ddr15     = 0;//aml1216_power_off_ddr15;
	pwr_op->power_on_ddr15      = 0;//aml1216_power_on_ddr15;
	pwr_op->shut_down           = aml1216_shut_down;
	pwr_op->detect_key          = aml1216_detect_key;
#endif
}


