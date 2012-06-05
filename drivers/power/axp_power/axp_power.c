#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/i2c.h>
#include <aml_i2c.h>

#include <axp-mfd.h>

#define AXP_I2C_ADDR 0x34

#define MAX_I2C_BUFF 64

int axp_write(int reg, uint8_t val)
{
	int ret;
	uint8_t buff[2];
    buff[0] = reg;
    buff[1] = val;
    struct i2c_msg msg[] = {
        {
        .addr = AXP_I2C_ADDR,
        .flags = 0,
        .len = 2,
        .buf = buff,
        }
    };

	ret = aml_i2c_xfer(msg, 1);
    if (ret < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
		return ret;
    }
	return 0;
}


int axp_writes(int reg, int len, uint8_t *val)
{
	int ret;
	uint8_t buff[MAX_I2C_BUFF + 1];
	if(len > MAX_I2C_BUFF)
	{
		printf("%s: i2c len must <= %d\n", __FUNCTION__, MAX_I2C_BUFF);
		return -1;
	}
    buff[0] = reg;
	memcpy((buff+1), val, len);
    struct i2c_msg msg[] = {
        {
        .addr = AXP_I2C_ADDR,
        .flags = 0,
        .len = len + 1,
        .buf = buff,
        }
    };

	ret = aml_i2c_xfer(msg, 1);
    if (ret < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
		return ret;
    }
	return 0;
}


int axp_read(int reg, uint8_t *val)
{
	int ret;
	struct i2c_msg msgs[] = {
        {
            .addr = AXP_I2C_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = AXP_I2C_ADDR,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = val,
        }
    };
	ret = aml_i2c_xfer(msgs, 2);
    if (ret < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
		return ret;
    }

    return 0;
}


int axp_reads(int reg, int len, uint8_t *val)
{
	int ret;
	struct i2c_msg msgs[] = {
        {
            .addr = AXP_I2C_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = AXP_I2C_ADDR,
            .flags = I2C_M_RD,
            .len = len,
            .buf = val,
        }
    };
	ret = aml_i2c_xfer(msgs, 2);
    if (ret < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
		return ret;
    }
    return 0;
}

int axp_set_bits(int reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = axp_write(reg, reg_val);
	}
out:
	return ret;
}

int axp_clr_bits(int reg, uint8_t bit_mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = axp_write(reg, reg_val);
	}
out:
	return ret;
}

int axp_update(int reg, uint8_t val, uint8_t mask)
{
	uint8_t reg_val;
	int ret = 0;

	ret = axp_read(reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = axp_write(reg, reg_val);
	}
out:
	return ret;
}

void axp_power_off(void)
{
	printf("[axp] send power-off command!\n");
	mdelay(20);
	axp_set_bits(POWER20_OFF_CTL, 0x80);
	mdelay(20);
	printf("[axp] warning!!! axp can't power-off, maybe some error happend!\n");
}

int ldo4_voltage_table[] = { 1250, 1300, 1400, 1500, 1600, 1700,
				   1800, 1900, 2000, 2500, 2700, 2800,
				   3000, 3100, 3200, 3300 };


int check_axp_regulator_for_m6_board(void)
{
	int ret = 0;
	uint8_t reg_data, val;

#ifdef CONFIG_CONST_PWM_FOR_DCDC
	//check work mode for DCDC2 & DCDC3
	axp_read(POWER20_DCDC_MODESET, &reg_data);
	if(!((reg_data&(1<<1) )&&(reg_data&(1<<2) )))
	{
		axp_set_bits(POWER20_DCDC_MODESET, ((1<<1)|(1<<2)));	//use constant PWM for DC-DC2 & DC-DC3
		printf("Use constant PWM for DC-DC2 & DC-DC3. But the register is 0x%x before\n", reg_data);
		mdelay(10);
		ret = 1;
	}
#endif


#ifdef CONFIG_DISABLE_LDO3_UNDER_VOLTAGE_PROTECT
	axp_read(0x81, &reg_data);	//check switch for  LDO3 under voltage protect
	if(reg_data & (1<<2))
	{
		printf("Disable LDO3 under voltage protect. But the register is 0x%x before\n", reg_data);
		reg_data &= ~(1<<2);	//disable LDO3 under voltage protect
		axp_write(0x81, reg_data);	
		mdelay(10);
		ret = 1;
	}
#endif

	//check for DCDC2(DDR3_1.5V)
#ifdef CONFIG_DCDC2_VOLTAGE
#if ((CONFIG_DCDC2_VOLTAGE<700) || (CONFIG_DCDC2_VOLTAGE>2275))
#error CONFIG_DCDC2_VOLTAGE not in the range 700~2275mV
#endif
	val = (CONFIG_DCDC2_VOLTAGE-700)/25;
	axp_read(POWER20_DC2OUT_VOL, &reg_data);
	if(reg_data != val)
	{
		axp_write(POWER20_DC2OUT_VOL, val);	//set DCDC2(DDR3_1.5V) to 1.500V
		printf("Set DCDC2(DDR3_1.5V) to %dmV(0x%x). But the register is 0x%x before\n", CONFIG_DCDC2_VOLTAGE, val, reg_data);
		mdelay(10);
		ret = 1;
	}
#endif
	//check for DCDC3(VDD_AO)
#ifdef CONFIG_DCDC3_VOLTAGE
#if ((CONFIG_DCDC3_VOLTAGE<700) || (CONFIG_DCDC3_VOLTAGE>3500))
#error CONFIG_DCDC3_VOLTAGE not in the range 700~3500mV
#endif
	val = (CONFIG_DCDC3_VOLTAGE-700)/25;
	axp_read(POWER20_DC3OUT_VOL, &reg_data);
	if(reg_data != val)
	{
		axp_write(POWER20_DC3OUT_VOL, val);	//set DCDC3(VDD_AO) to 1.100V
		printf("Set DCDC3(VDD_AO) to %dV(0x%x). But the register is 0x%x before\n", CONFIG_DCDC2_VOLTAGE, val, reg_data);
		mdelay(10);
		ret = 1;
	}
#endif

	//check for LDO2(VDDIO_AO)
#ifdef CONFIG_LDO2_VOLTAGE
#if ((CONFIG_LDO2_VOLTAGE<1800) || (CONFIG_LDO2_VOLTAGE>3300))
#error CONFIG_LDO2_VOLTAGE not in the range 1800~3300mV
#endif
	val = (CONFIG_LDO2_VOLTAGE-1800)/100;
	axp_read(POWER20_LDO24OUT_VOL, &reg_data);
	if(((reg_data>>4)&0xf)  != val)
	{
		val = reg_data & 0xf0 | (val<<4);
		axp_write(POWER20_LDO24OUT_VOL, val);	//set LDO2(VDDIO_AO) to 3.000V
		printf("Set  LDO2(VDDIO_AO) to %dmV(0x%x). But the register is 0x%x before\n", CONFIG_LDO2_VOLTAGE, val, reg_data);
		mdelay(10);
		ret = 1;
	}
#endif

	//check for LDO4(AVDD3.3V)
#ifdef CONFIG_LDO4_VOLTAGE
#if ((CONFIG_LDO4_VOLTAGE<1250) || (CONFIG_LDO4_VOLTAGE>3300))
#error CONFIG_LDO4_VOLTAGE not in the range 1250~3300mV
#endif
	for(val = 0; val < sizeof(ldo4_voltage_table); val++){
		if(CONFIG_LDO4_VOLTAGE <= ldo4_voltage_table[val]){
			break;
		}
	}
	axp_read(POWER20_LDO24OUT_VOL, &reg_data);
	if((reg_data&0xf) != val)
	{
		axp_write(POWER20_LDO24OUT_VOL, val);	//set LDO4(AVDD3.3V) to 3.300V
		printf("Set LDO4(AVDD3.3V) to %dmV(0x%x). But the register is 0x%x before\n", CONFIG_LDO4_VOLTAGE, val, reg_data);
		mdelay(10);
		ret = 1;
	}
#endif

	//check for LDO3(AVDD2.5V)
#ifdef CONFIG_LDO3_VOLTAGE
#if ((CONFIG_LDO3_VOLTAGE<700) || (CONFIG_LDO3_VOLTAGE>3500))
#error CONFIG_LDO3_VOLTAGE not in the range 700~3500mV
#endif
	val = (CONFIG_LDO3_VOLTAGE -700)/25;
	axp_read(POWER20_LDO3OUT_VOL, &reg_data);
	if(reg_data != val)
	{
		axp_write(POWER20_LDO3OUT_VOL, val);	//set LDO3(AVDD2.5V) to 2.500V;
		printf("set LDO3(AVDD2.5V) to %dmV(0x%x). But the register is 0x%x before\n",CONFIG_LDO3_VOLTAGE, val, reg_data);
		mdelay(10);
		ret = 1;
	}
#endif
	return ret;
}

