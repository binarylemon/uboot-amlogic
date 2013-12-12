#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/uart.h>

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
#else

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
}

void rn5t618_power_init()
{
    unsigned char val;

    hard_i2c_read8(DEVID, 0x0b);                                // clear watch dog 
    rn5t618_set_bits(0xB3, 0x40, 0x40);
    rn5t618_set_bits(0xB8, 0x02, 0x1f);                         // set charge current to 300mA
    __udelay(100*1000);
#ifdef CONFIG_VCCK_VOLTAGE
    rn5t618_set_dcdc_voltage(1, CONFIG_VCCK_VOLTAGE);           // set cpu voltage
    __udelay(2000);
#endif
#ifdef CONFIG_VDDAO_VOLTAGE
    rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_VOLTAGE);          // set VDDAO voltage
    __udelay(2000);
#endif
#ifdef CONFIG_DDR_VOLTAGE
    rn5t618_set_dcdc_voltage(3, CONFIG_DDR_VOLTAGE);            // set DDR voltage
    __udelay(100 * 1000);                                       // delay a short time
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

void power_init(void)
{
    hard_i2c_init();
    
    __udelay(1000);
#ifdef CONFIG_AW_AXP20
    hard_i2c_write8(DEVID, 0x27, (CONFIG_VDDAO_VOLTAGE - 700) / 25);
    hard_i2c_write8(DEVID, 0x80, 0xe4);  //DCDC2 PWM MODE
    hard_i2c_write8(DEVID, 0x23, 32);  //DCDC2 1.5V
#elif defined CONFIG_PMU_ACT8942
    if(CONFIG_VDDAO_VOLTAGE <= 1200)
        hard_i2c_write8(DEVID, 0x21, (CONFIG_VDDAO_VOLTAGE - 600) / 25);
    else if(CONFIG_VDDAO_VOLTAGE <= 2400)
        hard_i2c_write8(DEVID, 0x21, ((CONFIG_VDDAO_VOLTAGE - 1200) / 50) + 0x18);
    else
        hard_i2c_write8(DEVID, 0x21, ((CONFIG_VDDAO_VOLTAGE - 2400) / 100) + 0x30);
#elif defined CONFIG_AML_PMU
    aml_pmu_set_voltage(AML_PMU_DCDC1, CONFIG_VDDAO_VOLTAGE);   
#elif defined CONFIG_RN5T618
    rn5t618_power_init();
#endif
    __udelay(1000);
}
