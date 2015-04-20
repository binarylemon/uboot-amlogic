/*
 * AMLOGIC T13 LCD panel driver.
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
 * modified: xing xu <xing.xu@amlogic.com>
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/lcdoutc.h>
#include <asm/arch/osd.h>
#include <asm/arch/osd_hw.h>
#include <aml_i2c.h>
#include <amlogic/aml_lcd.h>
#include <amlogic/vinfo.h>
#include <video_fb.h>
#include <amlogic/gpio.h>
#ifdef CONFIG_AW_AXP20
#include <axp-gpio.h>
#endif

extern void mdelay(unsigned long msec);
extern int lcd_probe(void);
extern int lcd_remove(void);
extern int  video_dac_enable(unsigned char enable_mask);
extern int  video_dac_disable(void);

#define DEBUG

extern GraphicDevice aml_gdev;
vidinfo_t panel_info;

//Define backlight control method
#define BL_CTL_GPIO		0
#define BL_CTL_PWM		1
#define BL_CTL			BL_CTL_GPIO

//#define PWM_MAX         60000   //PWM_MAX <= 65535
#define PWM_MAX     		1331  //CONFIG_AML_BL_PWM_150HZ=y
#define	PWM_PRE_DIV		0		//pwm_freq = 24M / (pre_div + 1) / PWM_MAX

#if(BL_CTL==BL_CTL_PWM)
//#define PWM_MAX         60000   //PWM_MAX <= 65535
#define PWM_MAX     		1331  //CONFIG_AML_BL_PWM_150HZ=y
#define	PWM_PRE_DIV		0		//pwm_freq = 24M / (pre_div + 1) / PWM_MAX
#endif

#define BL_MAX_LEVEL		255
#define BL_MIN_LEVEL		20
#define DEFAULT_BL_LEVEL	204

static unsigned bl_level = 0;

static void backlight_power_ctrl(Bool_t status)
{
	printf("%s: power %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
    if ( status == ON )  // backlight power ctrl : gpio
	{
		amlogic_gpio_direction_output(GPIOY_6,1);
	}
	else
	{
		amlogic_gpio_direction_output(GPIOY_6,0);
	}
}

unsigned get_backlight_level(void)
{
	debug("%s :%d\n", __FUNCTION__,bl_level);
	return bl_level;
}

void set_backlight_level(unsigned level)
{
	debug("%s :%d\n", __FUNCTION__,level);
	int freq = 0,pwm_max = 0;
	char *pwm_freq = getenv("pwmfreq");
	freq = pwm_freq ? simple_strtol(pwm_freq,NULL,10) : 120;
	pwm_max = 199680/freq;

	level = (level > BL_MAX_LEVEL ? BL_MAX_LEVEL : (level < BL_MIN_LEVEL ? BL_MIN_LEVEL : level));
	bl_level=level;
	level = level * 15 / BL_MAX_LEVEL;
	level = 15 - level;

//	WRITE_CBUS_REG(PWM_MISC_REG_AB, ((READ_CBUS_REG(PWM_MISC_REG_AB) &
//	~((1 << 15) | (0x7F << 8) | (0x3 << 4)) | (1 << 0))) |
//	((1 << 15)  | (0x77 << 8) | (0x0<< 4)	| (1 << 0)) ); //0xf701 120div=119
	level = level * pwm_max / BL_MAX_LEVEL ;
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_2, (READ_CBUS_REG(PERIPHS_PIN_MUX_2) | (1 << 0)) );
	WRITE_CBUS_REG( PWM_PWM_A,0x4130120);

	int pwm_reg = READ_CBUS_REG(PWM_PWM_A);
	printf("pwm reg ---> 0x%x \n",pwm_reg);

}

static void lcd_power_ctrl(Bool_t status)
{
	printf("%s: power %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
	if (status) {
		//GPIOA27 -> LCD_PWR_EN#: 1  lcd 3.3v
#ifdef CONFIG_AML_PMU
		amlogic_gpio_direction_output(GPIOH_10,1);
#else
		amlogic_gpio_direction_output(GPIOH_10,1);
#endif

		mdelay(50);

	}else{
		backlight_power_ctrl(OFF);
		mdelay(50);
#ifdef CONFIG_AML_PMU
		amlogic_gpio_direction_output(GPIOH_10,0);
#else
		amlogic_gpio_direction_output(GPIOH_10,0);
#endif

		mdelay(50);
	}
}

#define H_ACTIVE        1920
#define V_ACTIVE        1080
#define H_PERIOD        2200
#define V_PERIOD        1125
#define VIDEO_ON_PIXEL      148
#define VIDEO_ON_LINE       41

static Lvds_Phy_Control_t lcd_lvds_phy_control =
{
	.lvds_prem_ctl  = 0xf,
	.lvds_swing_ctl = 0x3,
	.lvds_vcm_ctl   = 0x2,
	.lvds_ref_ctl   = 0xf,
	.lvds_phy_ctl0 = 0x2,
	.lvds_fifo_wr_mode = 0x3,
};

//Define LVDS data mapping, pn swap.
static Lvds_Config_t lcd_lvds_config={
	.lvds_repack    = 1,   //data mapping  //0:JEDIA mode, 1:VESA mode
	.pn_swap    = 0,       //0:normal, 1:swap
	.dual_port  = 1,
	.port_reverse   = 1,
};

Lcd_Config_t lcd_config =
{
	.lcd_basic = {
		.h_active = H_ACTIVE,
		.v_active = V_ACTIVE,
		.h_period = H_PERIOD,
		.v_period = V_PERIOD,
		.screen_ratio_width   = 16,
		.screen_ratio_height  = 9,
		.screen_actual_width  = 127, //this is value for 160 dpi please set real value according to spec.
		.screen_actual_height = 203, //
		.lcd_type = LCD_DIGITAL_LVDS,   //LCD_DIGITAL_TTL  //LCD_DIGITAL_LVDS  //LCD_DIGITAL_MINILVDS
		.lcd_bits = 8,  //8  //6
	},

	.lcd_timing = {
		.pll_ctrl = 0x40050c82,//0x400514d0, //
		.div_ctrl = 0x00010803,
		.clk_ctrl = 0x1111,  //[19:16]ss_ctrl, [12]pll_sel, [8]div_sel, [4]vclk_sel, [3:0]xd
		//.sync_duration_num = 501,
		//.sync_duration_den = 10,

		.video_on_pixel = VIDEO_ON_PIXEL,
		.video_on_line  = VIDEO_ON_LINE,

		.sth1_hs_addr = 44,
		.sth1_he_addr = 2156,
		.sth1_vs_addr = 0,
		.sth1_ve_addr = V_PERIOD - 1,
		.stv1_hs_addr = 2100,
		.stv1_he_addr = 2164,
		.stv1_vs_addr = 3,
		.stv1_ve_addr = 5,

		.pol_cntl_addr = (0x0 << LCD_CPH1_POL) |(0x0 << LCD_HS_POL) | (0x1 << LCD_VS_POL),
		.inv_cnt_addr = (0<<LCD_INV_EN) | (0<<LCD_INV_CNT),
		.tcon_misc_sel_addr = (1<<LCD_STV1_SEL) | (1<<LCD_STV2_SEL),
		.dual_port_cntl_addr = (1<<LCD_TTL_SEL) | (1<<LCD_ANALOG_SEL_CPH3) | (1<<LCD_ANALOG_3PHI_CLK_SEL) | (0<<LCD_RGB_SWP) | (0<<LCD_BIT_SWP),
	},

	.lcd_effect = {
		.gamma_cntl_port = (0 << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
		.gamma_vcom_hswitch_addr = 0,
		.rgb_base_addr = 0xf0,
		.rgb_coeff_addr = 0x74a,
	},

	.lvds_mlvds_config = {
		.lvds_config = &lcd_lvds_config,
		.lvds_phy_control = &lcd_lvds_phy_control,
	},

	// .power_on=lcd_power_on,
	// .power_off=lcd_power_off,
	// .backlight_on = power_on_backlight,
	// .backlight_off = power_off_backlight,
	// .get_bl_level = get_backlight_level,
	// .set_bl_level = set_backlight_level,
};

static void lcd_video_adjust(Lcd_Config_t *pConf)
{
	int i;

	const signed short video_adjust[33] = { -999, -937, -875, -812, -750, -687, -625, -562, -500, -437, -375, -312, -250, -187, -125, -62, 0, 62, 125, 187, 250, 312, 375, 437, 500, 562, 625, 687, 750, 812, 875, 937, 1000};

	for (i=0; i<33; i++)
	{
		pConf->lcd_effect.brightness[i] = video_adjust[i];
		pConf->lcd_effect.contrast[i]   = video_adjust[i];
		pConf->lcd_effect.saturation[i] = video_adjust[i];
		pConf->lcd_effect.hue[i]        = video_adjust[i];
	}
}

static void lcd_sync_duration(Lcd_Config_t *pConf)
{
	unsigned m, n, od, div, xd;
	unsigned pre_div;
	unsigned sync_duration;

	m = ((pConf->lcd_timing.pll_ctrl) >> 0) & 0x1ff;
	n = ((pConf->lcd_timing.pll_ctrl) >> 9) & 0x1f;
	od = ((pConf->lcd_timing.pll_ctrl) >> 16) & 0x3;
	div = ((pConf->lcd_timing.div_ctrl) >> 4) & 0x7;
	xd = ((pConf->lcd_timing.clk_ctrl) >> 0) & 0xf;

	od = (od == 0) ? 1:((od == 1) ? 2:4);
	switch (pConf->lcd_basic.lcd_type)
	{
		case LCD_DIGITAL_LVDS:
			pre_div = 7;
			break;
		default:
			pre_div = 1;
			break;
	}

	sync_duration = m*24*100/(n*od*(div+1)*xd*pre_div);
	sync_duration = ((sync_duration * 100000 / H_PERIOD) * 10) / V_PERIOD;
	sync_duration = (sync_duration + 5) / 10;

	pConf->lcd_timing.sync_duration_num = sync_duration;
	pConf->lcd_timing.sync_duration_den = 10;
}

static void power_on_backlight(void)
{
	debug("%s\n", __FUNCTION__);
	backlight_power_ctrl(ON);
}

static void power_off_backlight(void)
{
	debug("%s\n", __FUNCTION__);
	backlight_power_ctrl(OFF);
}

static void lcd_power_on(void)
{
	debug("%s\n", __FUNCTION__);
	video_dac_disable();
    //power_on_lcd();
	lcd_power_ctrl(ON);
}
static void lcd_power_off(void)
{
	debug("%s\n", __FUNCTION__);
	//power_off_backlight();
	backlight_power_ctrl(OFF);
    //power_off_lcd();
	lcd_power_ctrl(OFF);
}

static void lcd_io_init(void)
{
	debug("%s\n", __FUNCTION__);

	//power_on_lcd();
	//lcd_power_ctrl(ON);
	//set_backlight_level(DEFAULT_BL_LEVEL);
}

void lcd_enable(void)
{
	printf("%s\n", __FUNCTION__);

	panel_info.vd_base = (void*)simple_strtoul(getenv("fb_addr"), NULL, 0);
	panel_info.vl_col = simple_strtoul(getenv("display_width"), NULL, 0);
	panel_info.vl_row = simple_strtoul(getenv("display_height"), NULL, 0);
	panel_info.vl_bpix = simple_strtoul(getenv("display_bpp"), NULL, 0);
	panel_info.vd_color_fg = simple_strtoul(getenv("display_color_fg"), NULL, 0);
	panel_info.vd_color_bg = simple_strtoul(getenv("display_color_bg"), NULL, 0);

	lcd_sync_duration(&lcd_config);
	lcd_video_adjust(&lcd_config);
	lcd_io_init();
	lcd_probe();
}

void lcd_disable(void)
{
	debug("%s\n", __FUNCTION__);
	//power_off_backlight();
	backlight_power_ctrl(OFF);
	//power_off_lcd();
	lcd_power_ctrl(OFF);
	lcd_remove();
}

vidinfo_t panel_info =
{
	.vl_col	=	0,		/* Number of columns (i.e. 160) */
	.vl_row	=	0,		/* Number of rows (i.e. 100) */

	.vl_bpix	=	0,				/* Bits per pixel */

	.vd_console_address	=	NULL,	/* Start of console buffer	*/
	.console_col	=	0,
	.console_row	=	0,

	.vd_color_fg	=	0,
	.vd_color_bg	=	0,
	.max_bl_level	=	255,

	.cmap	=	NULL,		/* Pointer to the colormap */

	.priv		=	NULL,			/* Pointer to driver-specific data */
};

struct panel_operations panel_oper =
{
	.enable	=	lcd_enable,
	.disable	=	lcd_disable,
	.bl_on	=	power_on_backlight,
	.bl_off	=	power_off_backlight,
	.set_bl_level	=	set_backlight_level,
	.get_bl_level = get_backlight_level,
	.power_on=lcd_power_on,
	.power_off=lcd_power_off,
};
