#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/uart.h>

#include <hardi2c_lite.c>

#ifndef CONFIG_VDDAO_VOLTAGE
#define CONFIG_VDDAO_VOLTAGE 1200
#endif

#ifdef CONFIG_AW_AXP20
    #define DEVID 0x68
#elif defined CONFIG_PMU_ACT8942
    #define DEVID 0xb6
#elif defined CONFIG_AML_PMU
    #define DEVID 0x6A
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
#endif
    __udelay(1000);
}
