#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/uart.h>
#include <amlogic/aml_pmu_common.h>

extern void hard_i2c_init(void);
extern unsigned char hard_i2c_read8(unsigned char SlaveAddr, unsigned char RegAddr);
extern void hard_i2c_write8(unsigned char SlaveAddr, unsigned char RegAddr, unsigned char Data);
extern unsigned char hard_i2c_read168(unsigned char SlaveAddr, unsigned short RegAddr);
extern void hard_i2c_write168(unsigned char SlaveAddr, unsigned short RegAddr, unsigned char Data); 

#ifndef CONFIG_VDDAO_VOLTAGE
#define CONFIG_VDDAO_VOLTAGE 1200
#endif

#ifdef CONFIG_AW_AXP20
    #define DEVID 0x68
#elif defined CONFIG_PMU_ACT8942
    #define DEVID 0xb6
#elif defined CONFIG_AML_PMU
    #define DEVID 0x6A
#elif defined CONFIG_RN5T618
    #define DEVID 0x64
#elif defined CONFIG_AML1216
    #define DEVID 0x6A
#else

#endif

#ifdef CONFIG_AW_AXP20
void axp20_set_bits(uint32_t add, uint8_t bits, uint8_t mask)
{
    uint8_t val;
    val = hard_i2c_read8(DEVID, add);
    val &= ~(mask);                                         // clear bits;
    val |=  (bits & mask);                                  // set bits;
    hard_i2c_write8(DEVID, add, val);
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

int axp20_set_dcdc_voltage(int dcdc, int voltage)
{
    int addr, size, val;
    int idx_to, idx_cur, mask;

    addr = (dcdc == 2 ? 0x23 : 0x27);
    size = (dcdc == 2 ? 64 : 128);
    mask = (dcdc == 2 ? 0x3f : 0x7f);
    idx_to = find_idx(700, voltage, 25, size);                  // step is 25mV
    idx_cur = hard_i2c_read8(DEVID, addr);
#if 1                                                           // for debug
	serial_puts("\nvoltage set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(idx_to, 8);
    serial_puts(", addr:0x");
	serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    val = idx_cur;
    while (idx_cur != idx_to) {
        if (idx_cur < idx_to) {                                 // adjust to target voltage step by step
            idx_cur++;    
        } else {
            idx_cur--;
        }
        val &= ~mask;
        val |= idx_cur; 
        hard_i2c_write168(DEVID, addr, val);
        __udelay(100);                                          // atleast delay 100uS
    }
    __udelay(1 * 1000);
}

static int find_ldo4(int voltage) 
{
    int table[] = {
        1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
        2000, 2500, 2700, 2800, 3000, 3100, 3200, 3300
    };
    int i = 0;

    for (i = 0; i < 16; i++) {
        if (table[i] >= voltage) {
            return i;    
        }    
    }
    return i;
}

static void axp20_ldo_voltage(int ldo, int voltage)
{
    int addr, size, start, step;
    int idx_to, idx_cur, mask;
    int shift;

    switch (ldo) {
    case 2:
        addr  = 0x28; 
        size  = 16;
        step  = 100;
        mask  = 0xf0;
        start = 1800;
        shift = 4;
        break;
    case 3:
        addr  = 0x29;
        size  = 128;
        step  = 25;
        mask  = 0x7f;
        start = 700;
        shift = 0;
        break;
    case 4:
        addr  = 0x28;
        mask  = 0x0f;
        shift = 0;
        break;
    }
    if (ldo != 4) {
        idx_to = find_idx(start, voltage, step, size);
    } else {
        idx_to = find_ldo4(voltage); 
    }
    idx_cur = hard_i2c_read8(DEVID, addr);
#if 1                                                           // for debug
	serial_puts("\nvoltage set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(idx_to, 8);
    serial_puts(", addr:0x");
	serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    idx_cur &= ~mask;
    idx_cur |= (idx_to << shift);
    hard_i2c_write8(DEVID, addr, idx_cur);
}

static void axp20_power_init(int init_mode)
{
    hard_i2c_write8(DEVID, 0x80, 0xe4);
#ifdef CONFIG_DISABLE_LDO3_UNDER_VOLTAGE_PROTECT
    axp20_set_bits(0x81, 0x00, 0x04); 
#endif
#ifdef CONFIG_VDDAO_VOLTAGE
    axp20_set_dcdc_voltage(3, CONFIG_VDDAO_VOLTAGE);
#endif
#ifdef CONFIG_VDDIO_AO 
    axp20_ldo_voltage(2, CONFIG_VDDIO_AO);
#endif
#ifdef CONFIG_AVDD2V5 
    axp20_ldo_voltage(3, CONFIG_AVDD2V5);
#endif
#ifdef CONFIG_AVDD3V3 
    axp20_ldo_voltage(4, CONFIG_AVDD3V3);
#endif
#ifdef CONFIG_CONST_PWM_FOR_DCDC
    axp20_set_bits(0x80, 0x06, 0x06);
#endif
#ifdef CONFIG_DDR_VOLTAGE
    /*
     * must use direct register write...
     */
    hard_i2c_write8(DEVID, 0x23, (CONFIG_DDR_VOLTAGE - 700) / 25);
#endif
}
#endif

#ifdef CONFIG_AML_PMU
#define AML_PMU_DCDC1       1
#define AML_PMU_DCDC2       2
static unsigned int dcdc1_voltage_table[] = {                   // voltage table of DCDC1
    2000, 1980, 1960, 1940, 1920, 1900, 1880, 1860, 
    1840, 1820, 1800, 1780, 1760, 1740, 1720, 1700, 
    1680, 1660, 1640, 1620, 1600, 1580, 1560, 1540, 
    1520, 1500, 1480, 1460, 1440, 1420, 1400, 1380, 
    1360, 1340, 1320, 1300, 1280, 1260, 1240, 1220, 
    1200, 1180, 1160, 1140, 1120, 1100, 1080, 1060, 
    1040, 1020, 1000,  980,  960,  940,  920,  900,  
     880,  860,  840,  820,  800,  780,  760,  740
};

static unsigned int dcdc2_voltage_table[] = {                   // voltage table of DCDC2
    2160, 2140, 2120, 2100, 2080, 2060, 2040, 2020,
    2000, 1980, 1960, 1940, 1920, 1900, 1880, 1860, 
    1840, 1820, 1800, 1780, 1760, 1740, 1720, 1700, 
    1680, 1660, 1640, 1620, 1600, 1580, 1560, 1540, 
    1520, 1500, 1480, 1460, 1440, 1420, 1400, 1380, 
    1360, 1340, 1320, 1300, 1280, 1260, 1240, 1220, 
    1200, 1180, 1160, 1140, 1120, 1100, 1080, 1060, 
    1040, 1020, 1000,  980,  960,  940,  920,  900
};

int find_idx_by_voltage(int voltage, unsigned int *table)
{
    int i;

    /*
     * under this section divide(/ or %) can not be used, may cause exception
     */
    for (i = 0; i < 64; i++) {
        if (voltage >= table[i]) {
            break;    
        }
    }
    if (voltage == table[i]) {
        return i;    
    }
    return i - 1;
}

void aml_pmu_set_voltage(int dcdc, int voltage)
{
    int idx_to = 0xff;
    int idx_cur;
    unsigned char val;
    unsigned char addr;
    unsigned int *table;

    if (dcdc < 0 || dcdc > AML_PMU_DCDC2 || voltage > 2100 || voltage < 840) {
        return ;                                                // current only support DCDC1&2 voltage adjust
    }
    if (dcdc == AML_PMU_DCDC1) {
        addr  = 0x2f; 
        table = dcdc1_voltage_table;
    } else if (dcdc = AML_PMU_DCDC2) {
        addr  = 0x38;
        table = dcdc2_voltage_table;
    }
    val = hard_i2c_read168(DEVID, addr);
    idx_cur = ((val & 0xfc) >> 2);
    idx_to = find_idx_by_voltage(voltage, table);
#if 1                                                           // for debug
	serial_puts("\nvoltage set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(idx_to, 8);
    serial_puts("\n");
#endif
    while (idx_cur != idx_to) {
        if (idx_cur < idx_to) {                                 // adjust to target voltage step by step
            idx_cur++;    
        } else {
            idx_cur--;
        }
        val &= ~0xfc;
        val |= (idx_cur << 2);
        hard_i2c_write168(DEVID, addr, val);
        __udelay(100);                                          // atleast delay 100uS
    }
}

static void aml1212_power_init(int init_mode)
{
#ifdef CONFIG_VDDAO_VOLTAGE
    aml_pmu_set_voltage(AML_PMU_DCDC1, CONFIG_VDDAO_VOLTAGE);
#endif
#ifdef CONFIG_DDR_VOLTAGE
    aml_pmu_set_voltage(AML_PMU_DCDC2, CONFIG_DDR_VOLTAGE);
#endif
}
#endif      /* CONFIG_AML_1212  */

#ifdef CONFIG_RN5T618
void rn5t618_set_bits(uint32_t add, uint8_t bits, uint8_t mask)
{
    uint8_t val;
    val = hard_i2c_read8(DEVID, add);
    val &= ~(mask);                                         // clear bits;
    val |=  (bits & mask);                                  // set bits;
    hard_i2c_write8(DEVID, add, val);
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

int rn5t618_set_dcdc_voltage(int dcdc, int voltage)
{
    int addr;
    int idx_to, idx_cur;
    addr = 0x35 + dcdc;
    idx_to = find_idx(6000, voltage * 10, 125, 256);            // step is 12.5mV
    idx_cur = hard_i2c_read8(DEVID, addr);
#if 1                                                           // for debug
	serial_puts("\nvoltage set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(idx_to, 8);
    serial_puts(", addr:0x");
	serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    hard_i2c_write8(DEVID, addr, idx_to);
    __udelay(5 * 1000);
}

#define LDO_RTC1        10 
#define LDO_RTC2        11
int rn5t618_set_ldo_voltage(int ldo, int voltage)
{
    int addr;
    int idx_to, idx_cur;
    int start = 900;

    switch (ldo) {
    case LDO_RTC1:
        addr  = 0x56;
        start = 1700;
        break;
    case LDO_RTC2:
        addr  = 0x57;
        start = 900;
        break;
    case 1:
    case 2:
    case 4:
    case 5:
        start = 900;
        addr  = 0x4b + ldo;
        break;
    case 3:
        start = 600;
        addr  = 0x4b + ldo;
        break;
    default:
        serial_puts("wrong LDO value\n");
        break;
    }
    idx_to = find_idx(start, voltage, 25, 128);                 // step is 25mV
    idx_cur = hard_i2c_read8(DEVID, addr);
#if 1                                                           // for debug
	serial_puts("\nvoltage set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(idx_to, 8);
    serial_puts(", addr:0x");
	serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    hard_i2c_write8(DEVID, addr, idx_to);
    __udelay(5 * 100);
}

void rn5t618_power_init(int init_mode)
{
    unsigned char val;

    hard_i2c_read8(DEVID, 0x0b);                                // clear watch dog 
    rn5t618_set_bits(0xB3, 0x40, 0x40);
    rn5t618_set_bits(0xB8, 0x02, 0x1f);                         // set charge current to 300mA

    if (init_mode == POWER_INIT_MODE_NORMAL) {
    #ifdef CONFIG_VCCK_VOLTAGE
        rn5t618_set_dcdc_voltage(1, CONFIG_VCCK_VOLTAGE);       // set cpu voltage
    #endif
    #ifdef CONFIG_VDDAO_VOLTAGE
        rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_VOLTAGE);      // set VDDAO voltage
    #endif
    } else if (init_mode == POWER_INIT_MODE_USB_BURNING) {
        /*
         * if under usb burning mode, keep VCCK and VDDEE
         * as low as possible for power saving and stable issue
         */
        rn5t618_set_dcdc_voltage(1, 900);                       // set cpu voltage
        rn5t618_set_dcdc_voltage(2, 950);                       // set VDDAO voltage
    }
#ifdef CONFIG_DDR_VOLTAGE
    rn5t618_set_dcdc_voltage(3, CONFIG_DDR_VOLTAGE);            // set DDR voltage
#endif
    val = hard_i2c_read8(DEVID, 0x30);
    val |= 0x01;
    hard_i2c_write8(DEVID, 0x30, val);                          // Enable DCDC3--DDR
#ifdef CONFIG_VDDIO_AO28
    rn5t618_set_ldo_voltage(1, CONFIG_VDDIO_AO28);              // VDDIO_AO28
#endif
#ifdef CONFIG_VDDIO_AO18
    rn5t618_set_ldo_voltage(2, CONFIG_VDDIO_AO18);              // VDDIO_AO18
#endif
#ifdef CONFIG_VCC1V8
    rn5t618_set_ldo_voltage(3, CONFIG_VCC1V8);                  // VCC1.8V 
#endif
#ifdef CONFIG_VCC2V8
    rn5t618_set_ldo_voltage(4, CONFIG_VCC2V8);                  // VCC2.8V 
#endif
#ifdef CONFIG_AVDD1V8
    rn5t618_set_ldo_voltage(5, CONFIG_AVDD1V8);                 // AVDD1.8V 
#endif
#ifdef CONFIG_VDD_LDO
    rn5t618_set_ldo_voltage(LDO_RTC1, CONFIG_VDD_LDO);          // VDD_LDO
#endif
#ifdef CONFIG_RTC_0V9
    rn5t618_set_ldo_voltage(LDO_RTC2, CONFIG_RTC_0V9);          // RTC_0V9 
#endif
}
#endif

#ifdef CONFIG_AML1216

#define AML1216_DCDC1                1
#define AML1216_DCDC2                2
#define AML1216_DCDC3                3
#define AML1216_BOOST                4

#define AML1216_LDO1                 5
#define AML1216_LDO2                 6
#define AML1216_LDO3                 7
#define AAML1216_LDO4                8
#define AML1216_LDO5                 9

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
 
    val = hard_i2c_read168(DEVID, addr);
    idx_cur = ((val & 0xfc) >> 2);
    idx_to = find_idx_by_vddEE_voltage(voltage, VDDEE_voltage_table);

    current = idx_to*5; 

#if 1                                                           // for debug
    serial_puts("\nVDDEE current set from 0x");
    serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
    serial_put_hex(idx_to, 8);
    serial_puts(", addr:0x");
    serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    
    val &= ~0xfc;
    val |= (idx_to << 2);

    hard_i2c_write168(DEVID, addr, val);
    __udelay(5 * 100);
}

void aml1216_set_bits(uint32_t add, uint8_t bits, uint8_t mask)
{
    uint8_t val;
    val = hard_i2c_read168(DEVID, add);
    val &= ~(mask);                                         // clear bits;
    val |=  (bits & mask);                                  // set bits;
    hard_i2c_write168(DEVID, add, val);
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
void aml1216_set_pfm(int dcdc, int en)
{
    unsigned char val;
    if (dcdc < 1 || dcdc > AML1216_DCDC3 || en > 1 || en < 0) {
        return ;    
    }
    switch(dcdc) {
    case AML1216_DCDC1:
        val = hard_i2c_read168(DEVID, 0x003b);
        if (en) {
            val |=  (1 << 5);                                   // pfm mode
        } else {
            val &= ~(1 << 5);                                   // pwm mode
        }
        hard_i2c_write168(DEVID, 0x003b, val);
        break;
    case AML1216_DCDC2:
        val = hard_i2c_read168(DEVID, 0x0044);
        if (en) {
            val |=  (1 << 5);    
        } else {
            val &= ~(1 << 5);   
        }
        hard_i2c_write168(DEVID, 0x0044, val);
        break;
    case AML1216_DCDC3:
        val = hard_i2c_read168(DEVID, 0x004e);
        if (en) {
            val |=  (1 << 7);    
        } else {
            val &= ~(1 << 7);   
        }
        hard_i2c_write168(DEVID, 0x004e, val);
        break;
    case AML1216_BOOST:
        val = hard_i2c_read168(DEVID, 0x0028);
        if (en) {
            val |=  (1 << 0);    
        } else {
            val &= ~(1 << 0);   
        }
        hard_i2c_write168(DEVID, 0x0028, val);
        break;
    default:
        break;
    }
    __udelay(100);
}

int aml1216_set_dcdc_voltage(int dcdc, int voltage)
{
    int addr;
    int idx_to;
    int range    = 64;
    int step     = 19;
    int start    = 700;
    int bit_mask = 0x3f;
    int idx_cur;
    int val;

    if (dcdc > 3 || dcdc < 0) {
        return -1;    
    }
    addr = 0x34+(dcdc-1)*9;
    if (dcdc == 3) {
        step     = 50; 
        range    = 64; 
        start    = 2050;
        bit_mask = 0x1f;
    }
    idx_cur  = hard_i2c_read168(DEVID, addr);
    idx_to   = find_idx(start, voltage, step, range);
    idx_to  ^= bit_mask; 
#if 1
    serial_puts("\nvoltage of dcdc");
    serial_put_hex(dcdc, 4);
    serial_puts(" set from ");
    serial_put_hex(idx_cur, 8);
    serial_puts(" to ");
    serial_put_hex(idx_to << 2, 8);
    serial_puts(", addr:");
    serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    val = idx_cur;
    idx_cur >>= 2;
    while (idx_cur != idx_to) {
        if (idx_cur < idx_to) {                                 // adjust to target voltage step by step
            idx_cur++;    
        } else {
            idx_cur--;
        }
        val &= ~0xfc;
        val |= (idx_cur << 2);
        hard_i2c_write168(DEVID, addr, val);
        __udelay(100);                                          // atleast delay 100uS
    }
    __udelay(100);                         // wait a moment
    return 0;
}

int aml1216_set_ldo_voltage(int ldo, int voltage)
{
    int addr;
    int idx_to, idx_cur;
    int start = 720;
    int step = 40;
    int size = 46;
    int tmp1;

    if ( ldo >= 2)
    {
        addr  = 0x62 + (ldo-2)*3;
    }
	else
	{
		serial_puts("wrong LDO value\n");
	    return -1;
	}
    switch (ldo) {
         case 2:
             size = 63;
             break;
         case 5:
             start = 720;
             step  = 40;
             break;
         case 3:
         case 4:
         case 6:
         case 7:
             start = 740;
             step  = 50;
             break;
         case 8:
             start = 810;
             step  = 60;
             size  = 63;
             break;
         default:
             serial_puts("wrong LDO value\n");
             serial_put_hex(ldo, 8);
             return -1;
             break;
    }
    idx_to = find_idx(start, voltage, step, size);
    tmp1 = 0x3f - idx_to;

    if ( ldo == 8)
    {
        tmp1 = (idx_to-1) ^ 0x29;
    }

    idx_cur = hard_i2c_read168(DEVID, addr);
    
#if 1                      // for debug
    serial_puts("\nLDO ");
    serial_put_hex(ldo, 8);                                                           
	serial_puts(" voltage: set from 0x");
	serial_put_hex(idx_cur, 8);
    serial_puts(" to 0x");
	serial_put_hex(tmp1, 8);
    serial_puts(", addr:0x");
	serial_put_hex(addr, 8);
    serial_puts("\n");
#endif
    aml1216_set_bits(addr, (uint8_t)tmp1, 0x3f);
    __udelay(5 * 100);
}

void aml1216_check_vbat(int init)
{
    unsigned char val1, val2, val3;

    if (init) {
        val1 = hard_i2c_read168(DEVID, 0x0087);
        val2 = hard_i2c_read168(DEVID, 0x0088);
        val3 = hard_i2c_read168(DEVID, 0x0089);
        serial_puts("-- fault status: ");
        serial_put_hex(val1, 8);
        serial_put_hex(val2, 8);
        serial_put_hex(val3, 8);
        serial_puts("\n");
        __udelay(10 * 1000);
        hard_i2c_write168(DEVID, 0x009B, 0x0c);//enable auto_sample and accumulate IBAT measurement
        hard_i2c_write168(DEVID, 0x009C, 0x10);
        hard_i2c_write168(DEVID, 0x009D, 0x04);//close force charge and discharge sample mask
        hard_i2c_write168(DEVID, 0x009E, 0x08);//enable VBAT measure result average 4 samples
        hard_i2c_write168(DEVID, 0x009F, 0x20);//enable IBAT measure result average 4 samples
        hard_i2c_write168(DEVID, 0x009A, 0x20);
        hard_i2c_write168(DEVID, 0x00B8, 0x00);
        hard_i2c_write168(DEVID, 0x00A9, 0x8f);

        hard_i2c_write168(DEVID, 0x00A0, 0x01);//select auto-sampling timebase is 2ms
        hard_i2c_write168(DEVID, 0x00A1, 0x15);//set the IBAT measure threshold and enable auto IBAT +VBAT_in_active sample
        hard_i2c_write168(DEVID, 0x00C9, 0x06);// open DCIN_OK and USB_OK IRQ
    }
    __udelay(80000);
    val1 = hard_i2c_read168(DEVID, 0x00af);
    val2 = hard_i2c_read168(DEVID, 0x00b0);
    serial_puts("-- vbat: 0x");
    serial_put_hex(val2, 8);
    serial_put_hex(val1, 8);
    serial_puts("\n");
}

void aml1216_power_init(int init_mode)
{
    aml1216_check_vbat(1);
    if (init_mode == POWER_INIT_MODE_NORMAL) {
#ifdef CONFIG_VCCK_VOLTAGE
        aml1216_set_dcdc_voltage(1, CONFIG_VCCK_VOLTAGE);           // set cpu voltage
        __udelay(2000);
#endif

#ifdef CONFIG_VCC1V8
        aml1216_set_ldo_voltage(2, CONFIG_VCC1V8);
        __udelay(2000);
#endif

#ifdef CONFIG_VDDAO_VOLTAGE
        aml1216_set_vddEE_voltage(CONFIG_VDDAO_VOLTAGE); 
#endif

#ifdef CONFIG_VCC1V8
        aml1216_set_ldo_voltage(3, CONFIG_VCC1V8);
        __udelay(2000);

#endif

#ifdef CONFIG_VDDIO_AO28
        aml1216_set_ldo_voltage(7, CONFIG_VDDIO_AO28);
        __udelay(2000);
#endif

#ifdef CONFIG_VCC3V3
        //aml1216_set_dcdc_voltage(3, CONFIG_VCC3V3);
        __udelay(2000);
#endif

#ifdef CONFIG_DDR_VOLTAGE
        aml1216_set_dcdc_voltage(2, CONFIG_DDR_VOLTAGE);
        __udelay(2000);              
#endif

#ifdef CONFIG_VCC2V8
        aml1216_set_ldo_voltage(5, CONFIG_VCC2V8);
        __udelay(2000);
#endif

#ifdef CONFIG_VCC1V8
        aml1216_set_ldo_voltage(4, CONFIG_VCC1V8);
        __udelay(2000);
#endif

#ifdef CONFIG_VCC_CAM
        aml1216_set_ldo_voltage(6, CONFIG_VCC_CAM);
        hard_i2c_write168(DEVID, 0x83, 0x01);                           // open LDO6
#endif
    } else if (init_mode == POWER_INIT_MODE_USB_BURNING) {
        /*
         * if under usb burning mode, keep VCCK and VDDEE
         * as low as possible for power saving and stable issue
         */
        aml1216_set_dcdc_voltage(1, 900);                       // set cpu voltage                      
        aml1216_set_vddEE_voltage(950);                        // set VDDEE voltage
    }
    aml1216_check_vbat(0);
}
#endif
void power_init(int init_mode)
{
    hard_i2c_init();
    
    __udelay(1000);
#ifdef CONFIG_AW_AXP20
    axp20_power_init(init_mode);
#elif defined CONFIG_PMU_ACT8942
    if(CONFIG_VDDAO_VOLTAGE <= 1200)
        hard_i2c_write8(DEVID, 0x21, (CONFIG_VDDAO_VOLTAGE - 600) / 25);
    else if(CONFIG_VDDAO_VOLTAGE <= 2400)
        hard_i2c_write8(DEVID, 0x21, ((CONFIG_VDDAO_VOLTAGE - 1200) / 50) + 0x18);
    else
        hard_i2c_write8(DEVID, 0x21, ((CONFIG_VDDAO_VOLTAGE - 2400) / 100) + 0x30);
#elif defined CONFIG_AML_PMU
    aml1212_power_init(init_mode); 
#elif defined CONFIG_RN5T618
    rn5t618_power_init(init_mode);
#elif defined CONFIG_AML1216
    aml1216_power_init(init_mode);
#endif
    __udelay(1000);
}
