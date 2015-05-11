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

extern void backlight_default_config_init(Lcd_Bl_Config_t *bl_config);

extern unsigned int lvds_init(Lcd_Config_t *pConf);
extern unsigned int vbyone_init(Lcd_Config_t *pConf);

unsigned int (*init_lcd_port[])(Lcd_Config_t *pConf) = {
	lvds_init,
	vbyone_init,
};


typedef struct {
	Lcd_Config_t 	*pConf;
	Lcd_Bl_Config_t *bl_config;
} lcd_dev_t;

static lcd_dev_t *pDev = NULL;

extern void mdelay(unsigned long msec);
static void panel_power_ctrl(Bool_t status)
{
	printf("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
	printf("%s: gpio %d value %d \n",__FUNCTION__,pDev->pConf->lcd_power_ctrl.panel_power->gpio,pDev->pConf->lcd_power_ctrl.panel_power->on_value);
	if ( ON == status) {
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.panel_power->gpio,
									   pDev->pConf->lcd_power_ctrl.panel_power->on_value);
		mdelay(pDev->pConf->lcd_power_ctrl.panel_power->panel_on_delay);

	}else{
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.panel_power->gpio,
									  pDev->pConf->lcd_power_ctrl.panel_power->off_value);
		mdelay(pDev->pConf->lcd_power_ctrl.panel_power->panel_off_delay);
	}

//======================================================
printf("gpio value : %d - %d  time %d \n",pDev->pConf->lcd_power_ctrl.panel_power->gpio,
	                                   pDev->pConf->lcd_power_ctrl.panel_power->on_value,
	                                   pDev->pConf->lcd_power_ctrl.panel_power->panel_on_delay);
//======================================================
}


//****************************************//
// backlight control
//****************************************//
#define BL_LEVEL_MAX_DFT   			255
#define BL_LEVEL_MIN_DFT   			10
#define BL_LEVEL_DFT					128

volatile static Lcd_Bl_Config_t bl_config = {
	.level_default = BL_LEVEL_DFT,
	.level_min = BL_LEVEL_MIN_DFT,
	.level_max = BL_LEVEL_MAX_DFT,
};

static void set_lcd_backlight_level(unsigned level)
{
	unsigned pwm_hi = 0, pwm_lo = 0;
	printf("set_backlight_level: %u \n", level);

	level = (level > pDev->bl_config->level_max ? pDev->bl_config->level_max : (level < pDev->bl_config->level_min ? pDev->bl_config->level_min : level));
	level = ((level - pDev->bl_config->level_min) / (pDev->bl_config->level_max - pDev->bl_config->level_min)) * (pDev->bl_config->pwm_max - pDev->bl_config->pwm_min) + pDev->bl_config->pwm_min;

	pwm_hi = level;
	pwm_lo = pDev->bl_config->pwm_cnt - level;


	switch (pDev->bl_config->pwm_port) {
		case BL_PWM_A:
			aml_write_reg32(P_PWM_MISC_REG_AB, (aml_read_reg32(P_PWM_MISC_REG_AB) & ~(0x7f<<8)) | ((1 << 15) | (pDev->bl_config->pwm_pre_div<<8) | (1<<0)));
			aml_write_reg32(P_PWM_PWM_A, (pwm_hi << 16) | (pwm_lo));
			break;
		case BL_PWM_B:
			aml_write_reg32(P_PWM_MISC_REG_AB, (aml_read_reg32(P_PWM_MISC_REG_AB) & ~(0x7f<<16)) | ((1 << 23) | (pDev->bl_config->pwm_pre_div<<16) | (1<<1)));
			aml_write_reg32(P_PWM_PWM_B, (pwm_hi << 16) | (pwm_lo));
			break;
		case BL_PWM_C:
			aml_write_reg32(P_PWM_MISC_REG_CD, (aml_read_reg32(P_PWM_MISC_REG_CD) & ~(0x7f<<8)) | ((1 << 15) | (pDev->bl_config->pwm_pre_div<<8) | (1<<0)));  //enable pwm clk & pwm output
			aml_write_reg32(P_PWM_PWM_C, (pwm_hi << 16) | (pwm_lo));
			break;
		case BL_PWM_D:
			aml_write_reg32(P_PWM_MISC_REG_CD, (aml_read_reg32(PWM_MISC_REG_CD) & ~(0x7f<<16)) | ((1 << 23) | (pDev->bl_config->pwm_pre_div<<16) | (1<<1)));  //enable pwm clk & pwm output
			aml_write_reg32(P_PWM_PWM_D, (pwm_hi << 16) | (pwm_lo));
			break;
		case BL_PWM_E:
			aml_write_reg32(P_PWM_MISC_REG_EF, (aml_read_reg32(P_PWM_MISC_REG_EF) & ~(0x7f<<8)) | ((1 << 15) | (pDev->bl_config->pwm_pre_div<<8) | (1<<0)));  //enable pwm clk & pwm output
			aml_write_reg32(P_PWM_PWM_E, (pwm_hi << 16) | (pwm_lo));
			break;
		case BL_PWM_F:
			aml_write_reg32(P_PWM_MISC_REG_EF, (aml_read_reg32(PWM_MISC_REG_EF) & ~(0x7f<<16)) | ((1 << 23) | (pDev->bl_config->pwm_pre_div<<16) | (1<<1)));  //enable pwm clk & pwm output
			aml_write_reg32(P_PWM_PWM_F, (pwm_hi << 16) | (pwm_lo));
			break;
		default:
			break;
	}
}

static unsigned get_lcd_backlight_level(void)
{
    printf("%s \n", __FUNCTION__);
	return 0;
}

static void lcd_backlight_power_ctrl(Bool_t status)
{
	printf("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));

	printf("gpio:%d value %d \n",pDev->pConf->lcd_power_ctrl.bl_power->gpio,pDev->pConf->lcd_power_ctrl.bl_power->on_value);
	if ( ON == status) {
		mdelay(pDev->pConf->lcd_power_ctrl.bl_power->bl_on_delay);
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.bl_power->gpio,
									   pDev->pConf->lcd_power_ctrl.bl_power->on_value);
	}else{
		mdelay(pDev->pConf->lcd_power_ctrl.bl_power->bl_off_delay);
		amlogic_gpio_direction_output(pDev->pConf->lcd_power_ctrl.bl_power->gpio,
									  pDev->pConf->lcd_power_ctrl.bl_power->off_value);
	}
}

static inline void _init_display_driver(Lcd_Config_t *pConf_t)
{
	if (pDev->pConf->lcd_basic.lcd_type < LCD_TYPE_MAX) {
		init_lcd_port[pDev->pConf->lcd_basic.lcd_type](pConf_t);
	}else{
		printf("error: no lcd port\n");
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

	printf("disable lcd display driver.\n");
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
	panel_oper.power_on();			// enable panel power
	udelay(50);

	lcd_set_current_vmode(VMODE_LCD);	//init lcd port
}

extern void _set_panel_info(void);
extern int reset_lcd_config(Lcd_Config_t *pConf);

int lcd_probe(void)
{
	pDev = (lcd_dev_t *)malloc(sizeof(lcd_dev_t));
	if (!pDev) {
		printf("[tcon]: Not enough memory.\n");
		return -1;
	}
	pDev->pConf = &lcd_config_dft;
	if (!reset_lcd_config(pDev->pConf))
		pDev->pConf = &lcd_config_dft;


	pDev->bl_config = &bl_config_dft;					//==get defult blacklight config==//
	backlight_default_config_init(pDev->bl_config);  //=====init backlight contrl======//

	_set_panel_info();

	aml_write_reg32(P_VPP_POSTBLEND_H_SIZE, pDev->pConf->lcd_basic.h_active);

	_lcd_init(pDev->pConf);

	return 0;
}

int lcd_remove(void)
{
	panel_oper.bl_on();
	panel_oper.power_on();			// enable panel power

	_disable_display_driver(pDev->pConf);
	free(pDev);
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
		set_lcd_backlight_level(level);
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
