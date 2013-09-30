/*
 * AMLOGIC LCD panel parameter.
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
 * Author:  Evoke Zhang <evoke.zhang@amlogic.com>
 *
 */

#include <asm/arch/lcdoutc.h>

//**********************************************//
// backlight control
//*********************************************//
//**** define backlight default level ***//
#define BRIGHTNESS_LEVEL_DEFAULT	128

//**** define backlight control method ***//
// #define BL_CTL_GPIO				0
// #define BL_CTL_PWM_NEGATIVE		1
// #define BL_CTL_PWM_POSITIVE		2
#define BL_CTL				BL_CTL_PWM_POSITIVE
#define BL_GPIO				GPIODV_28

//**** define backlight GPIO control ***//
#define	BL_DIM_MAX			0x0
#define	BL_DIM_MIN			0xd

//**** define backlight GPIO control ***//
// #define BL_PWM_A				0
// #define BL_PWM_B				1
// #define BL_PWM_C				2
// #define BL_PWM_D				3
#define BL_PWM_PORT			BL_PWM_C
#define BL_PWM_USE_GPIO		1

#define	BL_PWM_FREQ			40000	//unit: Hz
#define BL_PWM_MAX         	100		//Unit: %
#define BL_PWM_MIN         	20		//Unit: %

#define BL_PWM_PINMUX_SET_NUM	1
#define BL_PWM_PINMUX_CLR_NUM	2
const static unsigned bl_pwm_pinmux_set[BL_PWM_PINMUX_SET_NUM][2] = {{3, 0x1000000},};
const static unsigned bl_pwm_pinmux_clr[BL_PWM_PINMUX_CLR_NUM][2] = {{0, 0x48}, {7, 0x10000200},};
//*********************************************//

//*********************************************//
// lcd parameter
//*********************************************//
//**** define lcd timing ***//
#define MODEL_NAME			"LP097QX1"	/** lcd model name */
#define ACITVE_AREA_WIDTH	197	/** lcd active area (display area) size, unit: mm; you can find it on the home page of lcd spec */
#define ACITVE_AREA_HEIGHT	147	/** lcd active area (display area) size, unit: mm */
#define LCD_TYPE			LCD_DIGITAL_EDP   /** LCD_DIGITAL_MIPI, LCD_DIGITAL_LVDS, LCD_DIGITAL_EDP, LCD_DIGITAL_TTL */
#define LCD_BITS			8	/** 6, 8 */
#define BITS_OPTION			0	/** option=0, means this lcd don't support 6/8bit switch, only support one bit mode */

#define H_ACTIVE			2048
#define V_ACTIVE			1536
#define H_PERIOD			2239//2208
#define V_PERIOD			1546//1549

#define	LCD_CLK				205200000	/** unit: Hz. both support clk and frame_rate, >200 regard as clk, <200 regard as frame_rate */
#define CLK_POL				1			/** clk_pol is only valid for TTL */
#define HS_WIDTH			5
#define HS_BACK_PORCH		10	/** hsync_backporch include hsync_width */
#define HS_POL				0	/** 0 for negative, 1 for positive */
#define VS_WIDTH			1
#define VS_BACK_PORCH		8//10	/** vsync_backporch include vsync_width */
#define VS_POL				0	/** 0 for negative, 1 for positive */

//**** define customer hardware design ***//
#if (BITS_OPTION == 1)
#undef LCD_BITS
#define LCD_BITS			6	/** 6 or 8, desided by hardware design; only valid when lcd_bits_option=1 */
#endif

#define TTL_RB_SWAP			0	/** 0 for normal, 1 for swap */
#define TTL_BIT_SWAP		0	/** 0 for normal, 1 for swap */

#define LVDS_PN_SWAP		0	/** 0 for normal, 1 for swap */

//**** advanced settings, modify them when needed ***//
#define VALID_HVSYNC		1	/** 0 for disable signal, 1 for enable signal */
#define VALID_DE			1	/** 0 for disable signal, 1 for enable signal */

#define VADJ_BRIGHTNESS		0x0		/** video adjust control */
#define VADJ_CONTRAST		0x80	/** video adjust control */
#define VADJ_SATURATION		0x100	/** video adjust control */

#define GAMMA_EN			0		/** 1 for enable gamma table, 0 for disable gamma table */
#define GAMMA_REVERT		0		/** 1 for revert gamma table % */
#define GAMMA_MULTI			0		/** gamma_multi: 0 for single gamma, means RGB use the same gamma; 1 for multi gamma, means RGB use different gamma */
									/** if gamma_multi=1, there must be 3 gamma tables, named as gamma_table_r, gamma_table_g, gamma_table_b */
#define	GAMMA_R_COEFF		100		/** unit: % */
#define	GAMMA_G_COEFF		100		/** unit: % */
#define	GAMMA_B_COEFF		100		/** unit: % */
static unsigned short gamma_table[256] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
        64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
        96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
        192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
        224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

//**** default settings, don't modify them unless there is display problem ***//
#define CLK_SPREAD_SPECTRUM		0	/** ss_level: 0=disable, 1=0.5%, 2=1%, 3=2%, 4=3%, 5=4%, 6=5% */
#define CLK_AUTO_GENERATION		1	/** 1 for auto generate clock parameters by lcd_clock, 0 for using customer clock parameters, as clk_pll_div_clk_ctrl defined */
#define PLL_CTRL				0x10220	/** only valid when clock_auto=0 */
#define DIV_CTRL				0x18803	/** only valid when clock_auto=0 */
#define CLK_CTRL				0x1111	/** only valid when clock_auto=0 */

#define H_OFFSET_SIGN			1	/** "sign-offset" is a pair; sign: 1 for positive, 0 for negative */
#define H_OFFSET				0
#define V_OFFSET_SIGN			1	/** "sign-offset" is a pair; sign: 1 for positive, 0 for negative */
#define V_OFFSET				0

#define LVDS_DPHY_CTRL			0xaf40	/** lvds signals voltage control */
#define LVDS_REPACK_USER		0
#define LVDS_REPACK				1		/** user define lvds data mapping, only valid when user=1, for special condition */

#define DITHER_USER				0		/** user define dither control, for special condition */
#define DITHER_CTRL				0x600
#define RGB_BASE				0xf0	/** rgb base control */
#define RGB_COEFF				0x74a	/** rgb coeff control */

#define VIDEO_ON_PIXEL			80	/** cpu internal video hold time */
#define VIDEO_ON_LINE			32	/** cpu internal video hold time */
//*********************************************//

//**** power control ***//
#define LCD_POWER_ON_STEP		3
#define LCD_POWER_OFF_STEP		3

static Lcd_Power_Config_t lcd_power_on_uboot = {.type = LCD_POWER_TYPE_MAX, .gpio = 0, .value = 0, .delay = 0};
static Lcd_Power_Config_t lcd_power_off_uboot = {.type = LCD_POWER_TYPE_MAX, .gpio = 0, .value = 0, .delay = 0};

static Lcd_Power_Config_t lcd_power_on_config[LCD_POWER_ON_STEP] = {
	{//step 1
		.type = LCD_POWER_TYPE_CPU, 
		.gpio = GPIODV_29, 
		.value = LCD_POWER_GPIO_OUTPUT_LOW,
		.delay = 20,
	},
	{//step 2
		.type = LCD_POWER_TYPE_PMU, 
		.gpio = LCD_POWER_PMU_GPIO0, 
		.value = LCD_POWER_GPIO_OUTPUT_LOW,
		.delay = 20,
	},
	{//step 3
		.type = LCD_POWER_TYPE_SIGNAL, 
		.gpio = 0, 
		.value = 0,
		.delay = 50,
	},
};

static Lcd_Power_Config_t lcd_power_off_config[LCD_POWER_OFF_STEP] = {	
	{//step 1
		.type = LCD_POWER_TYPE_SIGNAL, 
		.gpio = 0, 
		.value = 0,
		.delay = 20,
	},
	{//step 2
		.type = LCD_POWER_TYPE_PMU, 
		.gpio = LCD_POWER_PMU_GPIO0, 
		.value = LCD_POWER_GPIO_INPUT,
		.delay = 20,
	},
	{//step 3
		.type = LCD_POWER_TYPE_CPU, 
		.gpio = GPIODV_29, 
		.value = LCD_POWER_GPIO_OUTPUT_HIGH,
		.delay = 100,
	},
};
//*********************************************//

//*********************************************//
// lcd parameter API struct, DO NOT modify them!!
//*********************************************//
Lcd_Bl_Config_t bl_config_dft = {
	.level_default = BRIGHTNESS_LEVEL_DEFAULT,
	.method = BL_CTL,
	.gpio = BL_GPIO,
	.dim_max = BL_DIM_MAX,
	.dim_min = BL_DIM_MIN,
	.pwm_port = BL_PWM_PORT,
	.pwm_gpio_used = BL_PWM_USE_GPIO,
	.pinmux_set_num = BL_PWM_PINMUX_SET_NUM,
	.pinmux_clr_num = BL_PWM_PINMUX_CLR_NUM,
};

static DSI_Config_t lcd_mipi_config = {
	//to do
};

static LVDS_Config_t lcd_lvds_config = {
#if (LVDS_REPACK_USER == 1)
	.lvds_repack = LVDS_REPACK,
#else
#if (LCD_BITS == 6)
	.lvds_repack = 0,
#else
	.lvds_repack = 1,
#endif
#endif
	.pn_swap = LVDS_PN_SWAP,
};

static EDP_Config_t lcd_edp_config = {
	.link_rate = 0,
	.lane_count = 4,
	.vswing = 0,
	.preemphasis = 0,
};

static TTL_Config_t lcd_ttl_config = {
	.rb_swap = TTL_RB_SWAP,
	.bit_swap = TTL_BIT_SWAP,
};

static DPHY_Config_t lcd_dphy_config = {
	.dphy_ctrl = LVDS_DPHY_CTRL,
};

Lcd_Config_t lcd_config_dft = {
	.lcd_basic = {
		.model_name = MODEL_NAME,
        .h_active = H_ACTIVE,
        .v_active = V_ACTIVE,
        .h_period = H_PERIOD,
        .v_period = V_PERIOD,
    	.screen_ratio_width = ACITVE_AREA_WIDTH,
     	.screen_ratio_height = ACITVE_AREA_HEIGHT,
		.h_active_area = ACITVE_AREA_WIDTH,
     	.v_active_area = ACITVE_AREA_HEIGHT,
        .lcd_type = LCD_TYPE,
        .lcd_bits = LCD_BITS,
    },
	.lcd_timing = {
		.lcd_clk = LCD_CLK,
		.clk_ctrl = (CLK_AUTO_GENERATION<<CLK_CTRL_AUTO) | (1<<CLK_CTRL_VCLK_SEL) | (7<<CLK_CTRL_XD),
		.video_on_pixel = VIDEO_ON_PIXEL,
		.video_on_line = VIDEO_ON_LINE,
		
		.hsync_width = HS_WIDTH,
		.hsync_bp = HS_BACK_PORCH,
		.vsync_width = VS_WIDTH,
		.vsync_bp = VS_BACK_PORCH,
		.hvsync_valid = VALID_HVSYNC,
		.de_valid = VALID_DE,
		.h_offset = (H_OFFSET_SIGN << 31) | (H_OFFSET << 0),
		.v_offset = (V_OFFSET_SIGN << 31) | (V_OFFSET << 0),
		
        .pol_cntl_addr = (CLK_POL << LCD_CPH1_POL) |(HS_POL << LCD_HS_POL) | (VS_POL << LCD_VS_POL),
		.inv_cnt_addr = (0<<LCD_INV_EN) | (0<<LCD_INV_CNT),
		.tcon_misc_sel_addr = (1<<LCD_STV1_SEL) | (1<<LCD_STV2_SEL),
	},
	.lcd_effect = {
		.gamma_cntl_port = (GAMMA_EN << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
		.rgb_base_addr = RGB_BASE,
		.rgb_coeff_addr = RGB_COEFF,
		.dith_user = DITHER_USER,
		.vadj_brightness = VADJ_BRIGHTNESS,
		.vadj_contrast = VADJ_CONTRAST,
		.vadj_saturation = VADJ_SATURATION,
		.gamma_revert = GAMMA_REVERT,
		.gamma_r_coeff = GAMMA_R_COEFF,
		.gamma_g_coeff = GAMMA_G_COEFF,
		.gamma_b_coeff = GAMMA_B_COEFF,
	},
	.lcd_control = {
		.mipi_config = &lcd_mipi_config,
		.lvds_config = &lcd_lvds_config,
		.edp_config = &lcd_edp_config,
		.ttl_config = &lcd_ttl_config,
		.dphy_config = &lcd_dphy_config,
	},
	.lcd_power_ctrl = {
		.lcd_power_on_step = LCD_POWER_ON_STEP,
		.lcd_power_off_step = LCD_POWER_OFF_STEP,
	},
};

void lcd_default_config_init(Lcd_Config_t *pConf)
{
	int i;
	
	for (i=0; i<256; i++) {
		pConf->lcd_effect.GammaTableR[i] =  (gamma_table[i] << 2);
		pConf->lcd_effect.GammaTableG[i] =  (gamma_table[i] << 2);
		pConf->lcd_effect.GammaTableB[i] =  (gamma_table[i] << 2);
	}
	
	pConf->lcd_power_ctrl.lcd_power_on_uboot.type = lcd_power_on_uboot.type;
	pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = lcd_power_on_uboot.gpio;
	pConf->lcd_power_ctrl.lcd_power_on_uboot.value = lcd_power_on_uboot.value;
	pConf->lcd_power_ctrl.lcd_power_on_uboot.delay = lcd_power_on_uboot.delay;
	
	pConf->lcd_power_ctrl.lcd_power_off_uboot.type = lcd_power_off_uboot.type;
	pConf->lcd_power_ctrl.lcd_power_off_uboot.gpio = lcd_power_off_uboot.gpio;
	pConf->lcd_power_ctrl.lcd_power_off_uboot.value = lcd_power_off_uboot.value;
	pConf->lcd_power_ctrl.lcd_power_off_uboot.delay = lcd_power_off_uboot.delay;
	
	for (i=0; i<pConf->lcd_power_ctrl.lcd_power_on_step; i++) {
		pConf->lcd_power_ctrl.lcd_power_on_config[i].type = lcd_power_on_config[i].type;
		pConf->lcd_power_ctrl.lcd_power_on_config[i].gpio = lcd_power_on_config[i].gpio;
		pConf->lcd_power_ctrl.lcd_power_on_config[i].value = lcd_power_on_config[i].value;
		pConf->lcd_power_ctrl.lcd_power_on_config[i].delay = lcd_power_on_config[i].delay;
	}
	
	for (i=0; i<pConf->lcd_power_ctrl.lcd_power_off_step; i++) {
		pConf->lcd_power_ctrl.lcd_power_off_config[i].type = lcd_power_off_config[i].type;
		pConf->lcd_power_ctrl.lcd_power_off_config[i].gpio = lcd_power_off_config[i].gpio;
		pConf->lcd_power_ctrl.lcd_power_off_config[i].value = lcd_power_off_config[i].value;
		pConf->lcd_power_ctrl.lcd_power_off_config[i].delay = lcd_power_off_config[i].delay;
	}
}

void backlight_default_config_init(Lcd_Bl_Config_t *bl_config)
{
	int i;
	unsigned pwm_freq, pwm_cnt, pwm_pre_div;
	
	pwm_freq = ((BL_PWM_FREQ >= (FIN_FREQ * 500)) ? (FIN_FREQ * 500) : BL_PWM_FREQ);
	
	for (i=0; i<0x7f; i++) {
		pwm_pre_div = i;
		pwm_cnt = FIN_FREQ * 1000 / (pwm_freq * (pwm_pre_div + 1)) - 2;
		if (pwm_cnt <= 0xffff)
			break;
	}
	
	bl_config->pwm_cnt = pwm_cnt;
	bl_config->pwm_pre_div = pwm_pre_div;
	bl_config->pwm_max = pwm_cnt * BL_PWM_MAX / 100;
	bl_config->pwm_min = pwm_cnt * BL_PWM_MIN / 100;

	for (i=0; i<bl_config->pinmux_set_num; i++) {
		bl_config->pinmux_set[i][0] = bl_pwm_pinmux_set[i][0];
		bl_config->pinmux_set[i][1] = bl_pwm_pinmux_set[i][1];
	}
	
	for (i=0; i<bl_config->pinmux_clr_num; i++) {
		bl_config->pinmux_clr[i][0] = bl_pwm_pinmux_clr[i][0];
		bl_config->pinmux_clr[i][1] = bl_pwm_pinmux_clr[i][1];
	}	
}
