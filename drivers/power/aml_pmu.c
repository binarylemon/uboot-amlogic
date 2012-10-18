#include <common.h>
#include <div64.h>
#include <asm/setup.h>
#include <asm/arch/i2c.h>
#include <asm/arch/gpio.h>
#include <aml_i2c.h>
#include <amlogic/aml_lcd.h>
#include <amlogic/aml_pmu.h>

#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
#include <amlogic/battery_parameter.h>
#endif

#define ABS(x)			((x) >0 ? (x) : -(x) )
#define AML_PMU_ADDR    0x35
#define MAX_BUF         100
int aml_pmu_write(uint32_t add, uint32_t val)
{
    int ret;
    uint8_t buf[3] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    buf[2] = val & 0xff;
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = sizeof(buf),
            .buf   = buf,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 1);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_write16(uint32_t add, uint32_t val)
{
    int ret;
    uint8_t buf[4] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    buf[2] = val & 0xff;
    buf[3] = (val >> 8) & 0xff;
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = sizeof(buf),
            .buf   = buf,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 1);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_writes(uint32_t add, uint32_t len, uint8_t *buff)
{
    int ret;
    uint8_t buf[MAX_BUF] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    memcpy(buf + 2, buff, len);
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = len + 2,
            .buf   = buf,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 1);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_read(uint32_t add, uint8_t *val)
{
    int ret;
    uint8_t buf[2] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = sizeof(buf),
            .buf   = buf,
        },
        {
            .addr  = AML_PMU_ADDR,
            .flags = I2C_M_RD,
            .len   = 1,
            .buf   = val,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 2);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_read16(uint32_t add, uint16_t *val)
{
    int ret;
    uint8_t buf[2] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = sizeof(buf),
            .buf   = buf,
        },
        {
            .addr  = AML_PMU_ADDR,
            .flags = I2C_M_RD,
            .len   = 2, 
            .buf   = val,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 2);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_reads(uint32_t add, uint32_t len, uint8_t *buff)
{
    int ret;
    uint8_t buf[2] = {};
    buf[0] = add & 0xff;
    buf[1] = (add >> 8) & 0x0f;
    struct i2c_msg msg[] = {
        {
            .addr  = AML_PMU_ADDR,
            .flags = 0,
            .len   = sizeof(buf),
            .buf   = buf,
        },
        {
            .addr  = AML_PMU_ADDR,
            .flags = I2C_M_RD,
            .len   = len,
            .buf   = buff,
        }
    };
    ret = aml_i2c_xfer_slow(msg, 2);
    if (ret < 0) {
        printf("%s: i2c transfer failed, ret:%d\n", __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

int aml_pmu_get_voltage(void)
{
    uint8_t buf[2] = {};
    int result;
    aml_pmu_write(0x009A, 0x04);
    udelay(100);
    aml_pmu_reads(0x00AF, 2, buf);
  //printf("voltage, reg:%02x, %02x\n", buf[0], buf[1]);
    result = ((((buf[1] & 0x0f) << 8) + buf[0]) * 36) / 10;           // LSB of VBAT ADC is 3.6mV
    return result;
}

int aml_pmu_get_current(void)
{
    uint8_t   buf[2] = {};
    uint32_t tmp;
    int      result;
    aml_pmu_write(0x009A, 0x41);
    udelay(100);
    aml_pmu_reads(0x00AB, 2, buf);
  //printf("current, reg:%02x, %02x\n", buf[0], buf[1]);
    tmp = ((buf[1] & 0x0f) << 8) + buf[0];
    if (tmp & 0x800) {                                              // complement code
        tmp = (tmp ^ 0xfff) + 1;
    }
    result = tmp * 2;                                               // LSB of IBAT ADC is 2mA
    return result;
}

int aml_pmu_get_coulomb_acc(void)
{
    uint8_t buf[4] = {};
    int result;
    int coulomb;

    aml_pmu_write(0x009A, 0x40);
    aml_pmu_reads(0x00B5, 4, buf);

    result  = (buf[0] <<  0) | 
              (buf[1] <<  8) | 
              (buf[2] << 16) | 
              (buf[3] << 24);
  //printf("coulomb, reg:%02x, %02x, %02x, %02x\n", buf[0], buf[1], buf[2], buf[3]);
    coulomb = (result * 2) / (3600 * 100);                          // LSB of coulomb is 2mA
    return coulomb;
}

uint32_t aml_pmu_get_coulomb_cnt(void)
{
    uint8_t buf[4] = {};
    uint32_t result;
    
    aml_pmu_write(0x009A, 0x40);
    aml_pmu_reads(0x00b9, 4, buf);

    result = (buf[0] <<  0) | 
             (buf[1] <<  8) | 
             (buf[2] << 16) | 
             (buf[3] << 24);
    return result;
}

int aml_pmu_is_ac_online(void)
{
    uint8_t buf;
    aml_pmu_read(0x00e0, &buf);
    if (buf & 0x18) {
        return 1;    
    } else {
        return 0;    
    }
}

int aml_pmu_power_off(void)
{
    uint8_t buf = (1 << 5);                                 // software goto OFF state
    printf("[AML_PMU] software goto OFF state\n");
    aml_pmu_write(0x0081, buf);
}

int aml_pmu_get_charge_status(void)
{
    uint8_t val = 0;
    aml_pmu_read(0x00e0, &val);
    if (val & 0x04) {
        return 1;                                           // charging    
    } else {
        return 0;                                           // not charging    
    }
}

#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
int aml_pmu_get_ocv(int charge_status)
{
    int vbat = aml_pmu_get_voltage();
    int ibat = aml_pmu_get_current();
    int ocv;

    if (charge_status) {
        ocv = vbat - (ibat * board_battery_para.pmu_battery_rdc) / 1000;
    } else {
        ocv = vbat + (ibat * board_battery_para.pmu_battery_rdc) / 1000;    
    }
    return ocv;
}
#endif

int aml_pmu_get_charging_percent(void)
{
    int rest_vol;
#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
    int i;
    int ocv = 0;
    int ocv_diff, percent_diff, ocv_diff2;
    int charge_status = aml_pmu_get_charge_status();
    static int ocv_full  = 0;
    static int ocv_empty = 0;

    if (get_battery_para_flag() == PARA_UNPARSED) {
        /*
         * this code runs earlier than get_battery_para(), 
         * we need to know battery parameters first.
         */
        if (parse_battery_parameters() > 0) {
            set_battery_para_flag(PARA_PARSE_SUCCESS);
            for (i = 0; i < 16; i++) {                  	// find out full & empty ocv in battery curve
                if (!ocv_empty && board_battery_para.pmu_bat_curve[i].discharge_percent > 0) {
                    ocv_empty = board_battery_para.pmu_bat_curve[i - 1].ocv;    
                }
                if (!ocv_full && board_battery_para.pmu_bat_curve[i].discharge_percent == 100) {
                    ocv_full = board_battery_para.pmu_bat_curve[i].ocv;    
                }
            }
        } else {
            set_battery_para_flag(PARA_PARSE_FAILED);
        }
    }
    if (get_battery_para_flag() == PARA_PARSE_SUCCESS) {
        for (i = 0; i < 8; i++) {                           // calculate average ocv
            ocv += aml_pmu_get_ocv(charge_status); 
            udelay(10000); 
        }
        ocv = ocv / 8;
        if (ocv >= ocv_full) {
            return 100;    
        } else if (ocv <= ocv_empty) {
            return 0;    
        }
        for (i = 0; i < 15; i++) {                          // find which range this ocv is in
            if (ocv >= board_battery_para.pmu_bat_curve[i].ocv &&
                ocv <  board_battery_para.pmu_bat_curve[i + 1].ocv) {
                break;
            }
        }
        if (charge_status) {                                // calculate capability of battery according curve
            percent_diff = board_battery_para.pmu_bat_curve[i + 1].charge_percent -
                           board_battery_para.pmu_bat_curve[i].charge_percent;
        } else {
            percent_diff = board_battery_para.pmu_bat_curve[i + 1].discharge_percent -
                           board_battery_para.pmu_bat_curve[i].discharge_percent;
        }
        ocv_diff  = board_battery_para.pmu_bat_curve[i + 1].ocv -
                    board_battery_para.pmu_bat_curve[i].ocv;
        ocv_diff2 = ocv - board_battery_para.pmu_bat_curve[i].ocv;
        rest_vol  = (percent_diff * ocv_diff2 + ocv_diff / 2)/ocv_diff;
        if (charge_status) {
            rest_vol += board_battery_para.pmu_bat_curve[i].charge_percent;
        } else {
            rest_vol += board_battery_para.pmu_bat_curve[i].discharge_percent;
        }
        if (rest_vol > 100) {
            rest_vol = 100;    
        } else if (rest_vol < 0) {
            rest_vol = 0;    
        }
        return rest_vol;
    }
#endif      /* CONFIG_UBOOT_BATTERY_PARAMETERS */
    /*
     * TODO, need add code to calculate battery capability when cannot get battery parameter
     */
    return 50;
}

int amp_pmu_set_charging_current(int current)
{
    uint8_t val;
    if (current != 1500 && current != 2000) {
        printf("bad charge current:%d\n", current);
        return -1;    
    } 
    aml_pmu_read(0x002d, &val);
    if (current == 1500) {                              // only support 1.5A or 2A now
        val &= ~(1 << 5);    
    } else {
        val |=  (1 << 5);
    }
    return 0;
}

int aml_pmu_set_gpio(int pin, int val)
{
    int ret;
    uint8_t data;
    if (pin <= 0 || pin > 4 || val > 1 || val < 0) {
        printf("ERROR, invalid input value, pin = %d, val= %d\n", pin, val);
        return -1;
    }
    ret = aml_pmu_read(0x00C3, &data);
    if (ret) {
        return ret;
    }
    if (val) {
        data |=  (0x01 << (pin - 1));                       // set pin
    } else {
        data &= ~(0x01 << (pin - 1));                       // clear pin
    }
    return aml_pmu_write(0x00C3, data);
}

int aml_pmu_get_gpio(int pin, uint8_t *val)
{
    int ret;
    uint8_t data;

    if (pin <= 0 || pin > 4 || !val) {
        printf("ERROR, invalid input value, pin = %d, val= %d\n", pin, val);
        return -1;
    }
    ret = aml_pmu_read(0x00C4, &data);
    if (ret) {
        return ret;
    }
    if (data & (1 << (pin - 1))) {
        *val = 1;
    } else {
        *val = 0;    
    }

    return 0;
}

int set_power_down_slot(void)
{
    /*
     * power down sequence of each application is different,
     * you need to change this sequence of each macro to fit 
     * your application according datasheet
     */
#define UNUSED_SLOT_IDX     0
#define BOOST_SLOT_IDX      1
#define DCDC1_SLOT_IDX      2
#define DCDC2_SLOT_IDX      3
#define DCDC3_SLOT_IDX      4
#define LDO2_SLOT_IDX       5
#define LOD3_SLOT_IDX       6
#define LDO4_SLOT_IDx       7
#define LOD5_SLOT_IDX       8
#define LOD6_SLOT_IDX       9
#define EXT_DCDC_SLOT_IDX   10
#define GPIO1_SLOT_IDX      11
#define GPIO2_SLOT_IDX      12
#define GPIO3_SLOT_IDX      13
#define GPIO4_SLOT_IDX      14
    uint8_t power_down_seq[8] = {
        (DCDC1_SLOT_IDX    << 4) | BOOST_SLOT_IDX,          // 0x09, boost | dc1
        (DCDC3_SLOT_IDX    << 4) | DCDC2_SLOT_IDX,          // 0x0a, dc3   | dc2
        (LOD3_SLOT_IDX     << 4) | LDO2_SLOT_IDX,           // 0x0b, ldo3  | ldo2 
        (LOD5_SLOT_IDX     << 4) | LDO4_SLOT_IDx,           // 0x0c, ldo5  | ldo4
        (UNUSED_SLOT_IDX   << 4) | LOD6_SLOT_IDX,           // 0x0d, unuse | ldo6
        (EXT_DCDC_SLOT_IDX << 4) | UNUSED_SLOT_IDX,         // 0x0e, ex_dc | unuse
        (GPIO2_SLOT_IDX    << 4) | GPIO1_SLOT_IDX,          // 0x0f, gpio2 | gpio1
        (GPIO4_SLOT_IDX    << 4) | GPIO3_SLOT_IDX,          // 0x10, gpio4 | gpio3
    };

    return aml_pmu_writes(0x0009, 8, power_down_seq);
}

#define DEVICE  2
int aml_pmu_init(void)
{
    uint8_t val;

    printf("Call %s\n", __func__);

    set_power_down_slot();                      // set power down sequence, for test

    aml_pmu_write(0x0052, 0x92);
    aml_pmu_write(0x0029, 0x8b);                // target charge voltage = 4.2v
    aml_pmu_write(0x002c, 0xdf);                // all charge time out to max
    aml_pmu_write(0x0017, 0x01);                // enable charging
  //aml_pmu_write(0x0072, 0xd0);                // trimming VREFC to 1.2V
    aml_pmu_write(0x0072, 0xf0);                // trimming VREFC to 1.17V
    aml_pmu_write(0x0078, 0x04);                // enable LDO6
  //aml_pmu_write(0x006d, 0xc0);                // make LDO6 out 3.3V
  //aml_pmu_write(0x006d, 0x90);                // make LDO6 out 3.0V
    aml_pmu_write(0x006d, 0x70);                // make LDO6 out 2.8V
#if (DEVICE == 1)
    aml_pmu_write(0x0073, 0xc0);                // trimming SARADC voltage reference, c0 for device 1
#elif (DEVICE == 2)
    aml_pmu_write(0x0073, 0xb8);                // trimming SARADC voltage reference, b8 for device 2
#endif
    aml_pmu_write(0x009d, 0x64);                // select vref=2.4V
    aml_pmu_write(0x009e, 0x14);                // close the useless channel input

    // otp program
    aml_pmu_write(0x0048, 0x02);
    aml_pmu_write(0x0049, 0x80);
    aml_pmu_write(0x004e, 0x04);
    aml_pmu_write(0x0052, 0x80);
    aml_pmu_write(0x0055, 0xba);
    aml_pmu_write(0x0058, 0x10);
    aml_pmu_write(0x005d, 0x46);
    aml_pmu_write(0x005e, 0x84);
    aml_pmu_write(0x005f, 0x49);
    aml_pmu_write(0x0060, 0x64);
    aml_pmu_write(0x0061, 0x46);
    aml_pmu_write(0x0062, 0x84);
    aml_pmu_write(0x0063, 0x49);
    aml_pmu_write(0x0064, 0x64);
    aml_pmu_write(0x0065, 0xa6);
    aml_pmu_write(0x0066, 0x84);
    aml_pmu_write(0x0067, 0x49);
    aml_pmu_write(0x0068, 0x64);
    aml_pmu_write(0x0069, 0x6e);
    aml_pmu_write(0x006a, 0x84);
    aml_pmu_write(0x006b, 0x49);
    aml_pmu_write(0x006c, 0x64);

    aml_pmu_write(0x002e, 0xa4);                // calibrate reference
    aml_pmu_write(0x0029, 0xa3);
    aml_pmu_write(0x002e, 0x24);                //

    aml_pmu_write(0x009A, 0x80);                // clear coulomb value
    aml_pmu_write(0x00A0, 0x40);                // IBAT_AUTO time 10ms
    aml_pmu_write(0x009B, 0x0c);                // enable IBAT_AUTO, ACCUM

    aml_pmu_write(0x0019, 0x10);                // manual VBUS off
    aml_pmu_write16(0x0082, 0x0001);            // software boost up
    aml_pmu_set_gpio(1, 0);                     // open LCD power
    aml_pmu_set_gpio(2, 0);                     // open vccx2 power

    aml_pmu_read(0x0086, &val);
    printf("AML PMU addr[0x86] = 0x%02x\n", val);
    aml_pmu_read(0x00df, &val);
    printf("AML PMU addr[0xdf] = 0x%02x\n", val);

    return 1;
}

#ifdef CONFIG_UBOOT_BATTERY_PARAMETER_TEST
////////////////////////////////////////////////////////////////////////
//     This code is add for battery curve calibrate                   //
////////////////////////////////////////////////////////////////////////
// charecter attribute
#define     NoneAttribute       0   
#define     BoldFace            1   
#define     UnderLine           4   
#define     Flicker             5   
#define     ReverseDisplay      7   
#define     Hide                8   
#define     Un_BoldFace         22  
#define     Un_Flicker          25  
#define     Un_ReverseDisplay   27  

// background color
#define     Back_Black          40 
#define     Back_Red            41 
#define     Back_Yellow         43 
#define     Back_Blue           44 
#define     Back_Purple         45 
#define     Back_BottleGreen    46 
#define     Back_White          47

// front color
#define     Font_Black          30
#define     Font_Red            31
#define     Font_Green          42
#define     Font_Yellow         43
#define     Font_Blue           44
#define     Font_Purple         45
#define     Font_BottleGreen    46
#define     Font_White          47

void terminal_print(int x, int y, char *str)
{
    char   buff[200] = {};
    if (y == 35) {
        sprintf(buff,
                "\033[%d;%dH\033[0;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Black, 
                "                                                     "
                "                                                     ");
        printf(buff);
        sprintf(buff,
                "\033[%d;%dH\033[1;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Red, str);
    } else {
        sprintf(buff,
                "\033[%d;%dH\033[0;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Green, str);
    }
    printf(buff);
}

int32_t coulomb = 0;
int32_t ocv     = 0;
int32_t ibat    = 0;
int32_t vbat    = 0;
int32_t rdc     = 87;

int aml_calculate_rdc(void)
{
    char    buf[100];
    int32_t i_lo, i_hi;
    int32_t v_lo, v_hi;
    int32_t rdc_cal = 0;

    if (ocv > 4000) {                           // don't calculate rdc when ocv is too high
        return 0;
    }
    aml_pmu_read(0x2d, buf);
    buf[0] &= ~0x20;
    aml_pmu_write(0x2d, buf[0]);                // charge current to 1.5A;
    udelay(500000);
    i_lo = aml_pmu_get_current();
    v_lo = aml_pmu_get_voltage();
    buf[0] |= 0x20;
    aml_pmu_write(0x2d, buf[0]);                // charge current to 2A;
    udelay(500000);
    i_hi = aml_pmu_get_current();
    v_hi = aml_pmu_get_voltage();
    rdc_cal = (v_hi - v_lo) * 1000 / (i_hi - i_lo);
  // sprintf(buf, "i_lo:%4d, i_hi:%4d, u_lo:%4d, u_hi:%4d, rdc:%4d\n", i_lo, i_hi, v_lo, v_hi, rdc_cal);
  // terminal_print(0, 36, buf);
    printf("i_lo:%4d, i_hi:%4d, u_lo:%4d, u_hi:%4d, rdc:%4d\n", i_lo, i_hi, v_lo, v_hi, rdc_cal);
    if (rdc_cal < 0 || rdc_cal >= 300) {        // usually RDC will not greater than 300 mhom
        return 0;
    }
    return rdc_cal;
}

int aml_update_calibrate(int charge)
{
    uint32_t voltage, current;
    uint8_t  status;

    coulomb = aml_pmu_get_coulomb_acc(); 
    voltage = aml_pmu_get_voltage(); 
    current = aml_pmu_get_current();
    aml_pmu_read(0x00E0, &status); 
    if (status & 0x04) {                        // charging
        ocv = voltage - (current * rdc) / 1000;
    } else {
        ocv = voltage + (current * rdc) / 1000;
    }
    ibat = current;
    vbat = voltage;

    return 0;
}

static struct energy_array {
    int     ocv;                            // mV
    int     coulomb;                        // mAh read 
    int     coulomb_p;                      // mAh @ 3700mV
    int64_t energy;                         // mV * mAh
    int     updated_flag;           
};

static struct energy_array battery_energy_charge[16] = {
    {3132, 0, 0, 0, 0},                     // pmu_bat_para1
    {3273, 0, 0, 0, 0},                     // pmu_bat_para2
    {3414, 0, 0, 0, 0},                     // pmu_bat_para3
    {3555, 0, 0, 0, 0},                     // pmu_bat_para4
    {3625, 0, 0, 0, 0},                     // pmu_bat_para5
    {3660, 0, 0, 0, 0},                     // pmu_bat_para6
    {3696, 0, 0, 0, 0},                     // pmu_bat_para7
    {3731, 0, 0, 0, 0},                     // pmu_bat_para8
    {3766, 0, 0, 0, 0},                     // pmu_bat_para9
    {3801, 0, 0, 0, 0},                     // pmu_bat_para10
    {3836, 0, 0, 0, 0},                     // pmu_bat_para11
    {3872, 0, 0, 0, 0},                     // pmu_bat_para12
    {3942, 0, 0, 0, 0},                     // pmu_bat_para13
    {4012, 0, 0, 0, 0},                     // pmu_bat_para14
    {4083, 0, 0, 0, 0},                     // pmu_bat_para15
    {4153, 0, 0, 0, 0}                      // pmu_bat_para16   
};

static struct energy_array battery_energy_discharge[16] = {
    {3132, 0, 0, 0, 0},                     // pmu_bat_para1
    {3273, 0, 0, 0, 0},                     // pmu_bat_para2
    {3414, 0, 0, 0, 0},                     // pmu_bat_para3
    {3555, 0, 0, 0, 0},                     // pmu_bat_para4
    {3625, 0, 0, 0, 0},                     // pmu_bat_para5
    {3660, 0, 0, 0, 0},                     // pmu_bat_para6
    {3696, 0, 0, 0, 0},                     // pmu_bat_para7
    {3731, 0, 0, 0, 0},                     // pmu_bat_para8
    {3766, 0, 0, 0, 0},                     // pmu_bat_para9
    {3801, 0, 0, 0, 0},                     // pmu_bat_para10
    {3836, 0, 0, 0, 0},                     // pmu_bat_para11
    {3872, 0, 0, 0, 0},                     // pmu_bat_para12
    {3942, 0, 0, 0, 0},                     // pmu_bat_para13
    {4012, 0, 0, 0, 0},                     // pmu_bat_para14
    {4083, 0, 0, 0, 0},                     // pmu_bat_para15
    {4153, 0, 0, 0, 0}                      // pmu_bat_para16   
};

int32_t  ocv_history = 0;
uint32_t ocv_float   = 0;

static int32_t update_ocv(int32_t ocv)
{
#if 0
    int32_t i = 0;
    int32_t total = ocv * 4;

    for (i = 0; i < 3; i++) {
        total += ocv_array[i + 1] * (i+1);
        ocv_array[i] = ocv_array[i+1];
    }
    ocv_array[3] = ocv;
    return total / 10;
#else
    int tmp;
    if (ocv - ocv_history >= 16) {                          // truncated difference to 16mV
        ocv = ocv_history + 16;
    } else if (ocv_history - ocv >= 16) {
        ocv = ocv_history - 16;
    }
    tmp = (ocv_history * 7 + ocv);
    ocv_float = (ocv_float * 7 + ((tmp & 0x07) << 28)) / 8;   // Q27
    ocv_history = tmp / 8;
    if (ocv_float & 0x10000000) {                           // over flow
        ocv_history += 1;
        ocv_float &= ~0x10000000;
    }
    return ocv_float & 0x08000000 ? (ocv_history + 1) : ocv_history;
#endif
}

static void inline update_energy_charge(int ocv, int energy, int coulomb, int coulomb_p) 
{
    int i = 0;    
    char    buf[100] = {};
    for (i = 0; i < 16; i++) {
        if (ocv >= battery_energy_charge[i].ocv && !battery_energy_charge[i].updated_flag) {
            battery_energy_charge[i].energy = energy;
            battery_energy_charge[i].coulomb = coulomb;
            battery_energy_charge[i].coulomb_p = coulomb_p;
            battery_energy_charge[i].updated_flag = 1;
          //sprintf(buf, "update energy %9lld for %4d mV, index:%2d\n", 
          //        battery_energy_charge[i].energy, battery_energy_charge[i].ocv, i);
          //terminal_print(0, 35, buf);
            sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,\n", 
                    i,
                    battery_energy_charge[i].ocv, 
                    battery_energy_charge[i].energy, 
                    battery_energy_charge[i].coulomb,
                    battery_energy_charge[i].coulomb_p);
            terminal_print(0, 12 + i, buf);
        }
    }
}

static void check_energy_charge(int ocv, int energy, int coulomb, int coulomb_p)
{
    int i;
    for (i = 0; i < 16; i++) {
        if (!battery_energy_charge[i].updated_flag) {
            battery_energy_charge[i].energy       = energy;
            battery_energy_charge[i].coulomb      = coulomb;
            battery_energy_charge[i].coulomb_p    = coulomb_p;
            battery_energy_charge[i].updated_flag = 1;
        }
    }
}

static void inline update_energy_discharge(int ocv, int energy, int coulomb, int coulomb_p) 
{
    int i = 0;
    char    buf[100] = {};
    for (i = 0; i < 16; i++) {
        if (ocv < battery_energy_discharge[i].ocv && !battery_energy_discharge[i].updated_flag) {
            battery_energy_discharge[i].energy = energy;
            battery_energy_discharge[i].coulomb = coulomb;
            battery_energy_discharge[i].coulomb_p = coulomb_p;
            battery_energy_discharge[i].updated_flag = 1;
          //sprintf(buf, "update energy %9lld for %4d mV, index:%2d\n", 
          //       battery_energy_discharge[i].energy, battery_energy_discharge[i].ocv, i);
          //terminal_print(0, 35, buf);
            sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,\n", 
                    i,
                    battery_energy_discharge[i].ocv, 
                    battery_energy_discharge[i].energy, 
                    battery_energy_discharge[i].coulomb,
                    battery_energy_discharge[i].coulomb_p);
            terminal_print(60, 12 + i, buf);
        }
    }
}

static void check_energy_discharge(int ocv, int energy, int coulomb, int coulomb_p)
{
    int i = 0;
    for (i = 0; i < 16; i++) {
        if (!battery_energy_discharge[i].updated_flag) {
            battery_energy_charge[i].energy       = energy;
            battery_energy_charge[i].coulomb      = coulomb;
            battery_energy_charge[i].coulomb_p    = coulomb_p;
            battery_energy_charge[i].updated_flag = 1;
        }
    }
}

void  ClearScreen(void)                                 // screen clear for terminal
{
    char    buff[15] = {};
    int     length=0;
                
    sprintf(buff, "\033[2J\033[0m");
    while (buff[length] != '\0') { 
        length ++;
    }
    printf(buff);
}

extern panel_operations_t panel_oper;

int aml_battery_calibrate(void)
{
    int64_t energy_c = 0;
    int64_t energy_p = 0;
    int     prev_coulomb = 0;
    int     prev_ocv  = 0;
    int     prev_ibat = 0;
    int     key;
    int     ibat_cnt = 0;
    int     i;
    int64_t energy_top, energy_visible;
    int     base, offset, range_charge, percent, range_discharge;
    char    buf[200] = {};
    int     size;
    int     ocv_0 = 2;
    int     ocv_avg = 0;

    ClearScreen();
    terminal_print(0,  7, "=============================== WARNING ================================\n");
    terminal_print(0,  8, "Battery calibrate will take several hours to finish. Before calibrate,  \n");
    terminal_print(0,  9, "make sure you have discharge battery with voltage between to 3.0V ~ 3.05V.\n");
    terminal_print(0, 10, "during test, you can press key 'Q' to quit this process.\n");
    terminal_print(0, 11, "'R' = run calibration, 'Q' = quit. Your Choise:\n");

    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'r' || key == 'R') {
                break;
            }
            if (key == 'q' || key == 'Q') {
                goto out;
            }
        }
        udelay(10000);
    } 

#if 1
    ClearScreen(); 
    terminal_print(0, 1, "'Q' = quit, 'S' = Skip this step\n");
    terminal_print(0, 4, "coulomb     energy_c    ibat   prev_ibat    ocv"
                         "     ocv_hist    coulomb_p   vbat    rdc\n");
    aml_pmu_init();
    aml_update_calibrate(1);
    prev_coulomb = coulomb;
    prev_ocv = ocv;
    prev_ibat = ibat;
    for (i = 0; i < 4; i++) {
        aml_update_calibrate(1);
        ocv_avg += ocv;
    }
    ocv_history = ocv_avg / 4;
    if (panel_oper.set_bl_level) {
        panel_oper.set_bl_level(40);
    }
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'Q' || key == 'q') {
                terminal_print(0, 35, "You have aborted calibrate manually\n");
                goto out;
            }
            if (key == 'S' || key == 's') {
                terminal_print(0, 35, "Skip charging calibrate\n");
                break;
            }
        }
        aml_update_calibrate(1);
        energy_c += ABS((coulomb - prev_coulomb) * ((ocv + prev_ocv) / 2));
        energy_p = energy_c;
        do_div(energy_p, 3700);
        update_energy_charge(update_ocv(ocv), energy_c, coulomb, energy_p);
        size  = sprintf(buf, 
                        "%4d,   %12lld,   %4d,       %4d,  %4d,        %4d,",
                        coulomb, energy_c, ibat, prev_ibat, ocv, ocv_history);
        size += sprintf(buf + size,
                        "        %4d,  %4d,  %4d\n",
                        (int32_t)energy_p, vbat, rdc);
        buf[size] = '\0';
        terminal_print(0, 5, buf);
        prev_coulomb = coulomb;
        prev_ocv = ocv;
        prev_ibat = ibat;
        udelay(1000000);
        if (ibat <= 10) {                        // charging finished
            ibat_cnt++;
            if (ibat_cnt > 50) {
                break;
            }
        }
    }
    check_energy_charge(ocv, energy_c, coulomb, energy_p);

    terminal_print(0, 36, buf);

    energy_top = energy_c;
    terminal_print(0, 10, "============= RESULT FOR CHARGE ================\n");
    terminal_print(0, 11, "i,    ocv,     energy,     c,   c_e,   off,    %%\n");
    offset = battery_energy_charge[15].coulomb_p - battery_energy_charge[2].coulomb_p;
    i = (battery_energy_charge[3].coulomb_p - battery_energy_charge[2].coulomb_p) * 100;
    if ((i / offset) >= 3) {
        ocv_0 = 2; 
        terminal_print(0, 35, "We set zero reference ocv to 3414mV\n");
    } else {
        ocv_0 = 3;    
        terminal_print(0, 35, "We set zero reference ocv to 3555mV\n");
    }
    base = battery_energy_charge[ocv_0].coulomb_p;
    range_charge = battery_energy_charge[15].coulomb_p - base;
    for (i = 0; i < 16; i++) {
        energy_p = battery_energy_charge[i].energy * 100;
        if (i <= ocv_0) {
            offset  = 0;
            percent = 0;
        } else {
            offset = battery_energy_charge[i].coulomb_p - base;
            percent = 100 * (offset + range_charge / 200) / range_charge;
        }
        size = sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,  %4d,  %3d\n", 
                       i,
                       battery_energy_charge[i].ocv, 
                       battery_energy_charge[i].energy, 
                       battery_energy_charge[i].coulomb,
                       battery_energy_charge[i].coulomb_p,
                       offset,
                       percent);
        buf[size] = '\0';
        terminal_print(0, 12 + i, buf);
    }
    energy_p = energy_top;
    do_div(energy_p, 3700);
    size = sprintf(buf, "Total charge energy:%9lld(mV * mAh) = %5dmAh@3700mV\n", 
                   energy_top, (int32_t)energy_p);
    buf[size] = '\0';
    terminal_print(0, 30, buf);
    size = sprintf(buf, "Energy visible:%5dmAh@3700mV, percent:%2d\n", 
                   range_charge, range_charge * 100 / (int32_t)energy_p);
    buf[size] = '\0';
    terminal_print(0, 31, buf);

    /*
     * test for discharge
     */
    terminal_print(0, 35, "Please unplug DC power, then press 'R' to contine\n");
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'r' || key == 'R') {
                break;
            }
            if (key == 'q' || key == 'Q') {
                goto out;
            }
        }
        udelay(10000);
    } 

    terminal_print(0, 35, "do discharge calibration now, please don't plug DC power during test!\n");

    if (panel_oper.set_bl_level) {
        panel_oper.set_bl_level(255);
    }
    energy_c = 0;
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'Q' || key == 'q') {
                terminal_print(0, 35, "You have aborted calibrate manually\n");
                goto out;
            }
            if (key == 'S' || key == 's') {
                terminal_print(0, 35, "Skip discharging calibrate\n");
                break;
            }
        }
        aml_update_calibrate(0); 
        energy_c += ABS((prev_coulomb - coulomb) * ((ocv + prev_ocv) / 2));
        energy_p = energy_c;
        do_div(energy_p, 3700);
        update_energy_discharge(update_ocv(ocv), energy_c, coulomb, energy_p);
        size  = sprintf(buf, 
                        "%4d,   %12lld,   %4d,       %4d,  %4d,        %4d,",
                        coulomb, energy_c, ibat, prev_ibat, ocv, ocv_history);
        size += sprintf(buf + size,
                        "        %4d,  %4d,  %4d\n",
                        (int32_t)energy_p, vbat, rdc);
        buf[size] = '\0';
        terminal_print(0, 5, buf);
        prev_coulomb = coulomb;
        prev_ocv = ocv;
        prev_ibat = ibat;
        udelay(1000000);
        if (ocv < 3300) {
            terminal_print(0, 35, "ocv is too low, we stop discharging test now!\n");
            break;
        }
    }

    check_energy_discharge(ocv, energy_c, coulomb, energy_p);
    energy_top = energy_c;
    terminal_print(60, 10, "============ RESULT FOR DISCHARGE ==============\n");
    terminal_print(60, 11, "i,    ocv,     energy,     c,   c_e,   off,    %%\n");
    base = battery_energy_discharge[15].coulomb_p;
    range_discharge = battery_energy_discharge[ocv_0].coulomb_p - base;
    for (i = 0; i < 16; i++) {
        energy_p = battery_energy_discharge[i].energy * 100;
        if (i < ocv_0) {
            offset  = 0;
            percent = 0;
        } else {
            offset = battery_energy_discharge[i].coulomb_p - base;
            percent = 100 - 100 * (offset + range_discharge / 200) / range_discharge;
        }
        size = sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,  %4d,  %3d\n", 
                       i,
                       battery_energy_discharge[i].ocv, 
                       battery_energy_discharge[i].energy, 
                       battery_energy_discharge[i].coulomb,
                       battery_energy_discharge[i].coulomb_p,
                       offset,
                       percent);
        buf[size] = '\0';
        terminal_print(60, 12 + i, buf);
    }
    energy_p = energy_top;
    do_div(energy_p, 3700);
    size = sprintf(buf, "Energy visible:%5dmAh@3700mV\n", range_discharge);
    buf[size] = '\0';
    terminal_print(60, 30, buf);
    size = sprintf(buf, "Charging efficient:%d%%\n", (100 * (range_discharge + range_charge / 200)) / range_charge);
    buf[size] = '\0';
    terminal_print(60, 31, buf);
#else
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'Q' || key == 'q') {
                terminal_print(0, 35, "You have aborted calibrate manually\n");
                goto out;
            }
            if (key == 'S' || key == 's') {
                terminal_print(0, 35, "Skip charging calibrate\n");
                break;
            }
        }
        vbat = aml_pmu_get_voltage();
        ibat = aml_pmu_get_current();
        coulomb = aml_pmu_get_coulomb_acc();
//        aml_calculate_rdc();
        aml_pmu_reads(0x00de, 5, charge_status);
        printf("AML PMU, charge_status:%02x, %02x, %02x, %02x, %02x\n",
               charge_status[0], charge_status[1], charge_status[2], charge_status[3], charge_status[4]);
        aml_pmu_read(0x0086, charge_status);
        printf("AML PMU, GEN_STATUS0:%02x\n", charge_status[0]);
        printf("AML PMU, vbat:%4d, ibat:%4d, coulomb:%12d\n\n", vbat, ibat, coulomb);
        udelay(1000000);
    }
#endif

out:
    terminal_print(0, 38, "\n\n");
    return 1;
}
#endif /* CONFIG_UBOOT_BATTERY_PARAMETER_TEST */

