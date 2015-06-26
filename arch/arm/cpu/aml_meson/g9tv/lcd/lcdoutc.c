/*
 * AMLOGIC timing controller driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Elvis Yu <elvis.yu@amlogic.com>
 *
 *Modify: <jiaming.huang@amlogic.com>
 */
#include <common.h>
#include <malloc.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/aml_lcd.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include <asm/arch/gpio.h>

#include <amlogic/vinfo.h>
#include <amlogic/gpio.h>

#define PANEL_NAME	"panel"

extern unsigned int lvds_init(Lcd_Config_t *pConf);
extern unsigned int vbyone_init(Lcd_Config_t *pConf);

extern void set_backlight_default_pinmux(Lcd_Bl_Config_t *bl_config);
extern void _set_panel_info(void);

extern int reset_lcd_config(Lcd_Config_t *pConf);
extern int reset_bl_config(Lcd_Bl_Config_t *bl_config);


unsigned int (*init_lcd_port[])(Lcd_Config_t *pConf) = {
	lvds_init,
	vbyone_init,
};

static lcd_dev_t *pDev = NULL;

#define FIN_FREQ				(24 * 1000)  /*XTAL frequency (unit: MHz)*/


static unsigned int aml_lcd_pinmux_reg[] = {
	P_PERIPHS_PIN_MUX_0,
	P_PERIPHS_PIN_MUX_1,
	P_PERIPHS_PIN_MUX_2,
	P_PERIPHS_PIN_MUX_3,
	P_PERIPHS_PIN_MUX_4,
	P_PERIPHS_PIN_MUX_5,
	P_PERIPHS_PIN_MUX_6,
	P_PERIPHS_PIN_MUX_7,
	P_PERIPHS_PIN_MUX_8,
	P_PERIPHS_PIN_MUX_9,
	P_PERIPHS_PIN_MUX_10,
	P_AO_RTI_PIN_MUX_REG,
};

extern void mdelay(unsigned long msec);
static void panel_power_ctrl(Bool_t status)
{
	lcd_printf("statu=%s gpio=%d value=%d \n",(status ? "ON" : "OFF"),
		pDev->pConf->lcd_power_ctrl.panel_power->gpio,
		(status ?pDev->pConf->lcd_power_ctrl.panel_power->on_value:
				pDev->pConf->lcd_power_ctrl.panel_power->off_value));

	if ( ON == status) {
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.panel_power->gpio,
									   pDev->pConf->lcd_power_ctrl.panel_power->on_value);
		mdelay(pDev->pConf->lcd_power_ctrl.panel_power->panel_on_delay);

	} else {
		mdelay(pDev->pConf->lcd_power_ctrl.panel_power->panel_off_delay);
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.panel_power->gpio,
									  pDev->pConf->lcd_power_ctrl.panel_power->off_value);
	}
}

static int aml_lcd_pinmux_set(unsigned int mux_index, unsigned int mux_mask)
{
	lcd_printf("aml_lcd_pinmux_set: reg=%d	mask=%x\n",mux_index,mux_mask);
	if (mux_index < ARRAY_SIZE(aml_lcd_pinmux_reg)) {
		aml_set_reg32_mask(aml_lcd_pinmux_reg[mux_index], mux_mask);

		return 0;
	} else {
		printf("lcd error: wrong pinmux index %d\n", mux_index);
		return -1;
	}
}

static int aml_lcd_pinmux_clr(unsigned int mux_index, unsigned int mux_mask)
{
	lcd_printf("aml_lcd_pinmux_clr: reg=%d  mask=%x\n",mux_index,mux_mask);

	if (mux_index <  ARRAY_SIZE(aml_lcd_pinmux_reg)) {
		aml_clr_reg32_mask(aml_lcd_pinmux_reg[mux_index], mux_mask);
		return 0;
	} else {
		printf("lcd error: wrong pinmux index %d\n", mux_index);
		return -1;
	}
}

static void set_backlight_pwm_freq(unsigned int pwm_freq, unsigned int *cnt, unsigned int *pre_div)
{
	unsigned int i;
	unsigned int bl_freq, pwm_cnt, pwm_pre_div;

	bl_freq = (( pwm_freq >= (FIN_FREQ * 500)) ? (FIN_FREQ * 500) : pwm_freq);

	for (i=0; i<0x7f; i++) {
		pwm_pre_div = i;
		pwm_cnt = FIN_FREQ * 1000 / (pwm_pre_div + 1) / bl_freq - 2;
		if (pwm_cnt <= 0xffff)
			break;
	}
	*cnt = pwm_cnt;
	*pre_div = pwm_pre_div;

	lcd_printf("bl_freq = %d  cnt= %d  pwm_pre_div = %d\n",bl_freq,pwm_cnt,pwm_pre_div);
}

static void set_backlight_pwm_port(unsigned int pwm_port, unsigned int pwm_hi,unsigned int pwm_lo,unsigned int pwm_pre_div)
{
	switch (pwm_port) {
			case BL_PWM_A:
				aml_write_reg32(P_PWM_MISC_REG_AB, (aml_read_reg32(P_PWM_MISC_REG_AB) & ~(0x7f<<8)) | ((1 << 15) | (pwm_pre_div<<8) | (1<<0)));
				aml_write_reg32(P_PWM_PWM_A, (pwm_hi << 16) | (pwm_lo));
				break;
			case BL_PWM_B:
				aml_write_reg32(P_PWM_MISC_REG_AB, (aml_read_reg32(P_PWM_MISC_REG_AB) & ~(0x7f<<16)) | ((1 << 23) | (pwm_pre_div<<16) | (1<<1)));
				aml_write_reg32(P_PWM_PWM_B, (pwm_hi << 16) | (pwm_lo));
				break;
			case BL_PWM_C:
				aml_write_reg32(P_PWM_MISC_REG_CD, (aml_read_reg32(P_PWM_MISC_REG_CD) & ~(0x7f<<8)) | ((1 << 15) | (pwm_pre_div<<8) | (1<<0)));
				aml_write_reg32(P_PWM_PWM_C, (pwm_hi << 16) | (pwm_lo));
				break;
			case BL_PWM_D:
				aml_write_reg32(P_PWM_MISC_REG_CD, (aml_read_reg32(PWM_MISC_REG_CD) & ~(0x7f<<16)) | ((1 << 23) | (pwm_pre_div<<16) | (1<<1)));
				aml_write_reg32(P_PWM_PWM_D, (pwm_hi << 16) | (pwm_lo));
				break;
			case BL_PWM_E:
				aml_write_reg32(P_PWM_MISC_REG_EF, (aml_read_reg32(P_PWM_MISC_REG_EF) & ~(0x7f<<8)) | ((1 << 15) | (pwm_pre_div<<8) | (1<<0)));
				aml_write_reg32(P_PWM_PWM_E, (pwm_hi << 16) | (pwm_lo));
				break;
			case BL_PWM_F:
				aml_write_reg32(P_PWM_MISC_REG_EF, (aml_read_reg32(PWM_MISC_REG_EF) & ~(0x7f<<16)) | ((1 << 23) | (pwm_pre_div<<16) | (1<<1)));
				aml_write_reg32(P_PWM_PWM_F, (pwm_hi << 16) | (pwm_lo));
				break;
			default:
				break;
		}
}

static void set_lcd_backlight_level(Bool_t status, unsigned level)
{
	unsigned int pwm_hi = 0, pwm_lo = 0;
	unsigned int pwm_range,level_range,val_range;
	unsigned int i;
	unsigned int pwm_cnt,pwm_pre_div;

	if ( ON == status) {

		mdelay(pDev->bl_config->bl_pwm.pwm_on_delay);

		set_backlight_pwm_freq(pDev->bl_config->bl_pwm.pwm_freq, &pwm_cnt, &pwm_pre_div);
		pDev->bl_config->bl_pwm.pwm_max = pwm_cnt * pDev->bl_config->bl_pwm.pwm_duty_max / 100;
		pDev->bl_config->bl_pwm.pwm_min = pwm_cnt * pDev->bl_config->bl_pwm.pwm_duty_min / 100;

		level = (level > pDev->bl_config->bl_pwm.level_max ? pDev->bl_config->bl_pwm.level_max :
				(level < pDev->bl_config->bl_pwm.level_min ? pDev->bl_config->bl_pwm.level_min : level));
		lcd_printf("level = %d \n",level);

		val_range   = level - pDev->bl_config->bl_pwm.level_min;
		pwm_range   = pDev->bl_config->bl_pwm.pwm_max - pDev->bl_config->bl_pwm.pwm_min;
		level_range = pDev->bl_config->bl_pwm.level_max - pDev->bl_config->bl_pwm.level_min;
		level = val_range * pwm_range / level_range + pDev->bl_config->bl_pwm.pwm_min;

		if (pDev->bl_config->bl_pwm.pwm_positive) {
			pwm_hi = level;
			pwm_lo = pwm_cnt - level;
		} else {
			pwm_hi = pwm_cnt - level;
			pwm_lo = level;
		}
		lcd_printf("total_level= %u pwm_hi= %u pwm_lo= %u \n", level ,pwm_hi ,pwm_lo);

		set_backlight_pwm_port(pDev->bl_config->bl_pwm.pwm_port,
								pwm_hi,pwm_lo,pwm_pre_div);

		//set pin mux
		for (i=0; i<pDev->bl_config->bl_pwm.pinmux_clr_num; i++) {
			aml_lcd_pinmux_clr(pDev->bl_config->bl_pwm.pinmux_clr[i][0],pDev->bl_config->bl_pwm.pinmux_clr[i][1]);
		}
		for (i=0; i<pDev->bl_config->bl_pwm.pinmux_set_num; i++) {
			aml_lcd_pinmux_set( pDev->bl_config->bl_pwm.pinmux_set[i][0], pDev->bl_config->bl_pwm.pinmux_set[i][1]);
		}
	}else{
		amlogic_gpio_direction_output(pDev->bl_config->bl_pwm.pwm_gpio, level);
		mdelay(pDev->bl_config->bl_pwm.pwm_off_delay);
	}
}

static unsigned get_lcd_backlight_level(void)
{
    printf("%s \n", __FUNCTION__);
	return 0;
}

static void lcd_backlight_power_ctrl(Bool_t status)
{
	lcd_printf("status=%s gpio=%d value=%d \n",(status ? "ON" : "OFF"),
			pDev->bl_config->bl_power.gpio,
			(status ?pDev->bl_config->bl_power.on_value:
					pDev->bl_config->bl_power.off_value));

	if ( ON == status) {
		mdelay(pDev->bl_config->bl_power.bl_on_delay);



		amlogic_gpio_direction_output(pDev->bl_config->bl_power.gpio,
									   pDev->bl_config->bl_power.on_value);
	}else{
		amlogic_gpio_direction_output(pDev->bl_config->bl_power.gpio,
									  pDev->bl_config->bl_power.off_value);
		mdelay(pDev->bl_config->bl_power.bl_off_delay);
	}
}

static inline void _init_display_driver(Lcd_Config_t *pConf_t)
{
	if (pDev->pConf->lcd_basic.lcd_type < LCD_TYPE_MAX) {
		init_lcd_port[pDev->pConf->lcd_basic.lcd_type](pConf_t);
	}else{
		printf("lcd error: no lcd port\n");
		init_lcd_port[LCD_DIGITAL_LVDS](pConf_t);
	}
}

static inline void _disable_display_driver(Lcd_Config_t *pConf)
{
	int vclk_sel;

	vclk_sel = 0;//((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;

	aml_set_reg32_bits(P_HHI_VIID_DIVIDER_CNTL, 0, 11, 1);	//close lvds phy clk gate: 0x104c[11]

	aml_write_reg32(P_ENCT_VIDEO_EN, 0);	//disable enct
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);	//disable encl

	if (vclk_sel)
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 0, 5);		//close vclk2 gate: 0x104b[4:0]
	else
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 0, 5);		//close vclk1 gate: 0x105f[4:0]

	printf("lcd: disable lcd display driver.\n");
}

static inline void _enable_vsync_interrupt(void)
{
	if ((aml_read_reg32(P_ENCT_VIDEO_EN) & 1) || (aml_read_reg32(P_ENCL_VIDEO_EN) & 1)) {
		aml_write_reg32(P_VENC_INTCTRL, 0x200);
	}else{
		aml_write_reg32(P_VENC_INTCTRL, 0x2);
	}
}

static void _lcd_module_enable(void)
{
	_init_display_driver(pDev->pConf);
	_enable_vsync_interrupt();
}

static int lcd_set_current_vmode(vmode_t mode)
{
	_lcd_module_enable();
	return 0;
}

static void _lcd_init(Lcd_Config_t *pConf)
{
	panel_power_ctrl(ON);			// enable panel power
	udelay(50);

	lcd_set_current_vmode(VMODE_LCD);	//init lcd port

	set_lcd_backlight_level(ON,pDev->bl_config->bl_pwm.level_default);// set backlight  pwm on level
	udelay(50);

	lcd_backlight_power_ctrl(ON);  //enable backlight power
	udelay(50);
}

static int lcd_probe(void)
{
	pDev = (lcd_dev_t *)malloc(sizeof(lcd_dev_t));
	if (!pDev) {
		free(pDev);
		printf("lcd error: Not enough memory.\n");
		return -1;
	}
	memset(pDev, 0, sizeof(*pDev));

	pDev->pConf = &lcd_config_dft;
	pDev->bl_config = &bl_config_dft;
	set_backlight_default_pinmux(pDev->bl_config);

	reset_lcd_config(pDev->pConf);
	reset_bl_config(pDev->bl_config);

	_set_panel_info();

	aml_write_reg32(P_VPP_POSTBLEND_H_SIZE, pDev->pConf->lcd_basic.h_active);

	_lcd_init(pDev->pConf);

	return 0;
}

static int lcd_remove(void)
{
	lcd_backlight_power_ctrl(OFF);  //disable backlight power
	udelay(50);

	set_lcd_backlight_level(OFF, 0);// set backlight  pwm off level
	udelay(50);

	_disable_display_driver(pDev->pConf); //disable lcd signal

	panel_power_ctrl(OFF);			// disable panel power
	udelay(50);

	free(pDev);
	pDev = NULL;
	return 0;
}

static void _lcd_enable(void)
{
	lcd_probe();
}

static void _lcd_disable(void)
{
	lcd_remove();
}

static void _panel_power_on(void)
{
	panel_power_ctrl(ON);
}

static void _panel_power_off(void)
{
	panel_power_ctrl(OFF);
}

static void _set_backlight_level(unsigned level)
{
	set_lcd_backlight_level(ON,level);
}

static unsigned _get_backlight_level(void)
{
	return get_lcd_backlight_level();
}

static void _backlight_power_on(void)
{
	lcd_backlight_power_ctrl(ON);
}

static void _backlight_power_off(void)
{
	lcd_backlight_power_ctrl(OFF);
}


struct panel_operations panel_oper =
{
	.enable			=	_lcd_enable,
	.disable		=	_lcd_disable,
	.power_on		=	_panel_power_on,
	.power_off		=	_panel_power_off,
	.set_bl_level	=	_set_backlight_level,
	.get_bl_level	=	_get_backlight_level,
	.bl_on			=	_backlight_power_on,
	.bl_off			=	_backlight_power_off,
};
