/*
 * AMLOGIC TV LCD panel driver.
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
 * Modify: <jiaming.huang@amlogic.com>
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/aml_lcd.h>
#include <amlogic/vinfo.h>

#include <asm/arch/gpio.h>

//Rsv_val = 0xffffffff

Ext_Lcd_Config_t ext_lcd_config[LCD_TYPE_MAX] = {
		{/* AOC: public Platform lvds : 1920x1080@60hz 8bit pixel clk@74.25mhz 2prot*/
		"lvds_0",LCD_DIGITAL_LVDS,1920,1080,2200,1125,148,41,
		0x500404ad,0x00414400,0x71486900,44,2156,0,1079,2100,2164,3,5,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,1,0,1,1,0x3,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,50,50,
		0xff,0,0,
		GPIOY_6,1,0,50,50,
		BL_PWM_B,180,100,25,1,60,10,255},

		{/*lvds : 1920x1080@50hz 8bit pixel clk@74.25mhz 2prot*/
		"lvds_1",LCD_DIGITAL_LVDS,1920,1080,2200,1350,148,41,
		0x500404ad,0x00414400,0x71486900,44,2156,0,1079,2100,2164,3,5,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,1,0,1,1,0x3,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,50,50,
		0xff,0,0,
		GPIOY_6,1,0,50,50,
		BL_PWM_B,180,100,25,1,128,10,255},

		{/*AUO: T320XVN02.9 lvds : 1366x768@60hz 8bit pixel clk@80mhz 1port*/
		"lvds_2",LCD_DIGITAL_LVDS,1366,768,1648,810,280,42,
		0x5000068c,0x00454400,0x71c86900,20,30,0,809,20,1200,3,5,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,1,0,0,0,0x101,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,20,20,
		0xff,0,0,
		GPIOY_6,1,0,450,10,
		BL_PWM_B,180,100,25,1,128,10,255},

		{/*BOE: HV320WHB-N80 lvds : 1366x768@60hz 8bit pixel clk@74.25mhz 1port*/
		"lvds_3",LCD_DIGITAL_LVDS,1366,768,1560,806,190,30,
		0x500404ad,0x00454400,0x71486900,20,30,0,809,20,1200,3,5,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,1,0,0,0,0x101,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,15,500,
		0xff,0,0,
		GPIOY_6,1,0,200,200,
		BL_PWM_B,180,100,25,1,128,10,255},

		{/*PANDA: TPT315B5-0TU3A.Q lvds : 1366x768@60hz 8bit pixel clk@82mhz 1port*/
		"lvds_4",LCD_DIGITAL_LVDS,1366,768,1696,806,300,38,
		0x5000068c,0x00454400,0x71c86900,20,30,0,809,20,1200,3,5,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,1,0,0,0,0x101,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,50,50,
		0xff,0,0,
		GPIOY_6,1,0,600,200,
		BL_PWM_B,180,100,25,1,128,10,255},

	//==============For vx1==============//
		{/*BOE: HV550QU2-305 vx1 : 3840x2160@60hz 8lane pixel clk@74.5mhz */
		"vbyone_0",LCD_DIGITAL_VBYONE,3840,2160,4400,2250,557,90,
		Rsv_val,Rsv_val,Rsv_val,47,80,47,80,47,47,3,9,
		0,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,4,2,4,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,50,50,
		0xff,0,0,
		GPIOY_6,1,0,50,50,
		BL_PWM_B,180,100,25,1,128,10,255},

		{/*vx1 : 3840x2160@50hz 8lane pixel clk@74.5mhz */
		"vbyone_1",LCD_DIGITAL_VBYONE,3840,2160,4980,2250,557,90,
		Rsv_val,Rsv_val,Rsv_val,47,80,47,80,47,47,3,9,
		2,1,0,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		8,4,2,4,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,Rsv_val,
		GPIOH_10,1,0,50,50,
		0xff,0,0,
		GPIOY_6,1,0,50,50,
		BL_PWM_B,180,100,25,1,128,10,255},
};


//**** Special parameters just for Vbyone ***//
static Vbyone_Config_t lcd_vbyone_config={
	.lane_count = 8,	//lane:  1/2/4/6/8 lanes;
	.byte		= 4,	//byte:  3/4/5 bytes;
	.region		= 2,	//region
	.color_fmt	= 4,	//color_fmt
};

//**** Special parameters just for lvds ***//
static Lvds_Config_t lcd_lvds_config={
	.lvds_bits		= 8,	//6/8/10 bit
	.lvds_repack	= 1,	//0->JEDIA mode,  1->VESA mode
	.pn_swap		= 0,	//0->normal,         1->swap
	.dual_port		= 1,	//0->single lvds,	1->double lvds
	.port_swap		= 0,
};

#ifdef CONFIG_AML_LCD_EXTERN
struct lcd_extern_config_s lcd_extern_config = {
	.index = LCD_EXTERN_INDEX_INVALID,
	.on_delay = 0,
	.off_delay = 0,
};
#endif

//****panel power control only for uboot ***//
static Panel_Power_Config_t lcd_panel_power =
{
	.gpio		=	GPIOH_10,		/** panel power control gpio port */
	.on_value	=	1,				/** panel power on gpio out value*/
	.off_value	=	0,				/** panel power off gpio out value*/
	.panel_on_delay		=	50,		/** panel power on delay time (unit: ms)*/
	.panel_off_delay	=	50 		/** panel power off delay time (unit: ms)*/
};

Lcd_Config_t lcd_config_dft =
{
	.version = 0,
	.lcd_basic = {
		.lcd_type = LCD_DIGITAL_LVDS,	//LCD_DIGITAL_TTL /LCD_DIGITAL_LVDS/LCD_DIGITAL_VBYONE
		.h_active = 1920,
		.v_active = 1080,
		.h_period = 2200,
		.v_period = 1125,
		.video_on_pixel = 148,
		.video_on_line  = 41,
		.screen_ratio_width   = 16,
		.screen_ratio_height  = 9,
		.screen_actual_width  = 127, 	//this is value for 160 dpi please set real value according to spec.
		.screen_actual_height = 203, 	//
	},

	.lcd_timing = {
		.hpll_clk = 0x500404ad,  //0x10c8 : N->bits[13:9]       M->bits[8:0]
		.hpll_od = 0x00414400,   //0x10c9 : od1->bits[17:16]   od2->bits[23:22]   od3->bits[19:18]
		.hdmi_pll_cntl5 = 0x71486900,
		.frame_rate_adj_type = 0,
		.clk_auto = 1,
		.ss_level = 0,

		.sth1_hs_addr = 44,
		.sth1_he_addr = 2156,
		.sth1_vs_addr = 0,
		.sth1_ve_addr = 1125 - 1,
		.stv1_hs_addr = 2100,
		.stv1_he_addr = 2164,
		.stv1_vs_addr = 3,
		.stv1_ve_addr = 5,
	},

	.lcd_control = {
#ifdef CONFIG_AML_LCD_EXTERN
		.ext_config	= &lcd_extern_config,
#endif
		.lvds_config	= &lcd_lvds_config,
		.vbyone_config	= &lcd_vbyone_config,
	},

	.lcd_power_ctrl = {
		.panel_power	=	&lcd_panel_power,
	},
};

//**** backlight PWM pinmux setting ***//
const static const unsigned bl_pwm_pinmux_set[][2] = {{9, 0x2000000}};             //set pwm pinmux : {reg,bit}
const static const unsigned bl_pwm_pinmux_clr[][2] = {{3, 0x20},{9, 0x4000001},{5, 0x4000000},{10, 0x280000}}; //clear other pinmux:{reg, bit}

//****Backlight level control only for uboot ***//
Lcd_Bl_Config_t bl_config_dft = {
	.bl_power = {
		.gpio		=	GPIOY_6,          /** backlight power control gpio port */
		.on_value	=	1,                /** backlight power on gpio output high*/
		.off_value	=	0,                /** backlight power off gpio output low*/
		.bl_on_delay	=	50,           /** backlight power on delay time (unit: ms)*/
		.bl_off_delay	=	50            /** backlight power off delay time (unit: ms)*/
	},
	.bl_pwm = {
		.pwm_port = BL_PWM_B, /** pwm port name(BL_PWM_A, BL_PWM_B, BL_PWM_C, BL_PWM_D, BL_PWM_VS) */
		.pwm_freq = 180,      /** pwm_vs: 1~4(vfreq multiple), other pwm: real freq(unit: Hz) */
		.pwm_duty_max = 100,			/** brightness diminig duty_max(unit: %, positive logic) */
		.pwm_duty_min = 25,				/** brightness diminig duty_min(unit: %, positive logic) */
		.pwm_positive = 1,				/** brightness pwm polarity   1: positive 0: negative */

		.level_default = 128,            /** default brightness level */
		.level_min = 10,                 /** brightness level max, must match the rootfs setting*/
		.level_max = 255,                /** brightness level min, must match the rootfs setting*/

	},
};


void set_backlight_default_pinmux(Lcd_Bl_Config_t *bl_config)
{
	unsigned int i;

	if ( ARRAY_SIZE(bl_pwm_pinmux_set) > ARRAY_SIZE(bl_config->bl_pwm.pinmux_set) ||
		ARRAY_SIZE(bl_pwm_pinmux_clr) > ARRAY_SIZE(bl_config->bl_pwm.pinmux_clr)) {
		printf("error: out of pinmux range !! \n");
		return;
	}
	bl_config->bl_pwm.pinmux_set_num   = ARRAY_SIZE(bl_pwm_pinmux_set);
	bl_config->bl_pwm.pinmux_clr_num   = ARRAY_SIZE(bl_pwm_pinmux_clr);

	for (i=0; i<ARRAY_SIZE(bl_pwm_pinmux_set); i++) {
		bl_config->bl_pwm.pinmux_set[i][0] = bl_pwm_pinmux_set[i][0];
		bl_config->bl_pwm.pinmux_set[i][1] = bl_pwm_pinmux_set[i][1];
	}

	for (i=0; i<ARRAY_SIZE(bl_pwm_pinmux_clr); i++) {
		bl_config->bl_pwm.pinmux_clr[i][0] = bl_pwm_pinmux_clr[i][0];
		bl_config->bl_pwm.pinmux_clr[i][1] = bl_pwm_pinmux_clr[i][1];
	}
}

//*************************For osd show logo***************************//
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
	.cmap			=	NULL,			/* Pointer to the colormap */
	.priv			=	NULL,			/* Pointer to driver-specific data */
};

void _set_panel_info(void)
{
	panel_info.vd_base		= (void*)simple_strtoul(getenv("fb_addr"), NULL, 0);
	panel_info.vl_col		= simple_strtoul(getenv("display_width"), NULL, 0);
	panel_info.vl_row		= simple_strtoul(getenv("display_height"), NULL, 0);
	panel_info.vl_bpix		= simple_strtoul(getenv("display_bpp"), NULL, 0);
	panel_info.vd_color_fg	= simple_strtoul(getenv("display_color_fg"), NULL, 0);
	panel_info.vd_color_bg	= simple_strtoul(getenv("display_color_bg"), NULL, 0);

	printf("panel_info:width x height = %ux%u\n", panel_info.vl_col, panel_info.vl_row);
	printf("panel_info:vl_bpix = %u\n", panel_info.vl_bpix);
}

