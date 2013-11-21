/*
 * AMLOGIC lcd controller driver.
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
#include <asm/arch/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/aml_lcd.h>
#include <amlogic/vinfo.h>
#include <asm/arch/mlvds_regs.h>
#include <asm/arch/clock.h> 
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif

#define PANEL_NAME	"panel"
#define DRIVER_DATE		"20130718"
#define DRIVER_VER		"u-dt"

#define VPP_OUT_SATURATE            (1 << 0)

#define PRINT_DEBUG_INFO
#ifdef PRINT_DEBUG_INFO
#define DPRINT(...)		printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

vidinfo_t panel_info;
typedef struct {
    Lcd_Config_t conf;
    vinfo_t lcd_info;
} lcd_dev_t;

static lcd_dev_t *pDev = NULL;
static int dts_ready = 0;

static void _lcd_init(Lcd_Config_t *pConf);
int lcd_probe(void);
int lcd_remove(void);

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

#ifdef CONFIG_OF_LIBFDT
#define BL_LEVEL_MAX    		255
#define BL_LEVEL_MIN    		10
#define BL_LEVEL_DEFAULT		102
#define BL_LEVEL_MID    		128
#define BL_LEVEL_MAPPED_MID		102

#define BL_CTL_GPIO			0
#define BL_CTL_PWM			1
#define BL_PWM_A			0
#define BL_PWM_B			1
#define BL_PWM_C			2
#define BL_PWM_D			3

static char * dt_addr;
static int amlogic_gpio_name_map_num(const char *name);
static int amlogic_gpio_set(int gpio, int flag);

typedef struct {
	unsigned level_default;
    unsigned char method;
	int gpio;
	unsigned dim_max;
	unsigned dim_min;
	unsigned short pwm_port;
	unsigned char pwm_gpio_used;
	unsigned pwm_cnt;
	unsigned pwm_pre_div;
	unsigned pwm_max;
	unsigned pwm_min;
	unsigned pinmux_set_num;
	unsigned pinmux_set[5][2];
	unsigned pinmux_clr_num;
	unsigned pinmux_clr[5][2];
} Lcd_Bl_Config_t;

static Lcd_Bl_Config_t bl_config;
static unsigned bl_level;

static Lvds_Config_t lcd_lvds_config=
{
	.lvds_repack = 0,   //data mapping  //0:JEIDA mode, 1:VESA mode
	.pn_swap = 0,		  //0:normal, 1:swap
};

static Lcd_Config_t lcd_config_dft = {
	.lcd_timing = {
		.lcd_clk = 40000000,
		.clk_ctrl = (1 << CLK_CTRL_AUTO) | 0x1117,
		.video_on_pixel = 80,
		.video_on_line = 32,
		.hvsync_valid = 0,
		.de_valid = 1,
		.pol_cntl_addr = (0 << LCD_CPH1_POL) |(0 << LCD_HS_POL) | (0 << LCD_VS_POL),
		.inv_cnt_addr = (0 << LCD_INV_EN) | (0 << LCD_INV_CNT),
		.tcon_misc_sel_addr = (1 << LCD_STV1_SEL) | (1 << LCD_STV2_SEL),
		.dual_port_cntl_addr = (0 << LCD_RGB_SWP) | (0 << LCD_BIT_SWP),
	},
	.lcd_effect = {
		.gamma_cntl_port = (1 << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
		.rgb_base_addr = 0xf0,
		.rgb_coeff_addr = 0x74a,
		.dith_user = 0,
		.vadj_brightness = 0x0,
		.vadj_contrast = 0x80,
		.vadj_saturation = 0x100,
		.gamma_r_coeff = 100,
		.gamma_g_coeff = 100,
		.gamma_b_coeff = 100,
	},
	.lvds_mlvds_config = {
		.lvds_config = &lcd_lvds_config,
		.lvds_phy_ctrl = 0xaf40,
	},
};

static void lcd_setup_gamma_table(Lcd_Config_t *pConf, unsigned int rgb_flag)
{
	int i;
	
	const unsigned short gamma_adjust[256] = {
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
		64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
		96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
		128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
		160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
		224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
	};

	if (rgb_flag == 0) {	//r
		for (i=0; i<256; i++) {
			pConf->lcd_effect.GammaTableR[i] = gamma_adjust[i] << 2;
		}
	}
	else if (rgb_flag == 1)	{	//g
		for (i=0; i<256; i++) {
			pConf->lcd_effect.GammaTableG[i] = gamma_adjust[i] << 2;
		}
	}
	else if (rgb_flag == 2)	{	//b
		for (i=0; i<256; i++) {
			pConf->lcd_effect.GammaTableB[i] = gamma_adjust[i] << 2;
		}
	}
	else if (rgb_flag == 3)	{	//rgb
		for (i=0; i<256; i++) {
			pConf->lcd_effect.GammaTableR[i] = gamma_adjust[i] << 2;
			pConf->lcd_effect.GammaTableG[i] = gamma_adjust[i] << 2;
			pConf->lcd_effect.GammaTableB[i] = gamma_adjust[i] << 2;
		}
	}
}

static void lvds_ports_ctrl(Bool_t status)
{
	if (status) {
		WRITE_MPEG_REG(LVDS_GEN_CNTL,  READ_MPEG_REG(LVDS_GEN_CNTL) | (1 << 3)); // enable fifo
		if (pDev->conf.lcd_basic.lcd_bits == 6)
			WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) | (0x27<<0));  //enable LVDS phy 3 channels
		else
			WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) | (0x2f<<0));  //enable LVDS phy 4 channels
	}else {
		WRITE_MPEG_REG(LVDS_PHY_CNTL3, READ_MPEG_REG(LVDS_PHY_CNTL3) & ~(1<<0));
		WRITE_MPEG_REG(LVDS_PHY_CNTL5, READ_MPEG_REG(LVDS_PHY_CNTL5) & ~(1<<11));  //shutdown lvds phy
		WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) & ~(0x7f<<0));  //disable LVDS phy port
		WRITE_MPEG_REG(LVDS_GEN_CNTL,  READ_MPEG_REG(LVDS_GEN_CNTL) & ~(1 << 3)); // disable fifo
	}
	printf("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static void ttl_ports_ctrl(Bool_t status)
{
	if (status) 
	{
		WRITE_MPEG_REG(PERIPHS_PIN_MUX_1, READ_MPEG_REG(PERIPHS_PIN_MUX_1) | ((1<<14)|(1<<17)|(1<<18)|(1<<19))); //set tcon pinmux
		if (pDev->conf.lcd_basic.lcd_bits == 6)
			WRITE_MPEG_REG(PERIPHS_PIN_MUX_0, READ_MPEG_REG(PERIPHS_PIN_MUX_0) | ((1<<0)|(1<<2)|(1<<4)));  //enable RGB 18bit
		else
			WRITE_MPEG_REG(PERIPHS_PIN_MUX_0, READ_MPEG_REG(PERIPHS_PIN_MUX_0) | ((3<<0)|(3<<2)|(3<<4)));  //enable RGB 24bit
	}else {
		if (pDev->conf.lcd_basic.lcd_bits == 6) {
			WRITE_MPEG_REG(PERIPHS_PIN_MUX_0, READ_MPEG_REG(PERIPHS_PIN_MUX_0) & ~((1<<0)|(1<<2)|(1<<4))); //disable RGB 18bit
			WRITE_MPEG_REG(PREG_PAD_GPIO1_EN_N, READ_MPEG_REG(PREG_PAD_GPIO1_EN_N) | (0x3f << 2) | (0x3f << 10) | (0x3f << 18));
		}
		else {
			WRITE_MPEG_REG(PERIPHS_PIN_MUX_0, READ_MPEG_REG(PERIPHS_PIN_MUX_0) & ~((3<<0)|(3<<2)|(3<<4))); //disable RGB 24bit
			WRITE_MPEG_REG(PREG_PAD_GPIO1_EN_N, READ_MPEG_REG(PREG_PAD_GPIO1_EN_N) | (0xff << 0) | (0xff << 8) | (0xff << 16));
		}
		WRITE_MPEG_REG(PERIPHS_PIN_MUX_1, READ_MPEG_REG(PERIPHS_PIN_MUX_1) & ~((1<<14)|(1<<17)|(1<<18)|(1<<19)));  //clear tcon pinmux
		WRITE_MPEG_REG(PREG_PAD_GPIO2_EN_N, READ_MPEG_REG(PREG_PAD_GPIO2_EN_N) | ((1<<18)|(1<<19)|(1<<20)|(1<<23)));  //GPIOD_2 D_3 D_4 D_7 
	}
	printf("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));	
}

static void lcd_signals_ports_ctrl(Bool_t status)
{	
	switch(pDev->conf.lcd_basic.lcd_type){
		case LCD_DIGITAL_TTL:
			ttl_ports_ctrl(status);
			break;
		case LCD_DIGITAL_LVDS:
			lvds_ports_ctrl(status);
			break;
		case LCD_DIGITAL_MINILVDS:
			//minilvds_ports_ctrl(status);
			 break;
		default:
			printf("Invalid LCD type.\n");
			break;
	}
}

static void lcd_backlight_power_ctrl(Bool_t status)
{
    int i;

	if( status == ON ) {
		WRITE_CBUS_REG_BITS(LED_PWM_REG0, 1, 12, 2);
	    if (bl_config.method == BL_CTL_GPIO) {
			amlogic_gpio_set(bl_config.gpio, LCD_POWER_GPIO_OUTPUT_HIGH);
		}
		else {
			if (bl_config.pwm_port == BL_PWM_A) {
				WRITE_MPEG_REG(PWM_MISC_REG_AB, (READ_MPEG_REG(PWM_MISC_REG_AB) & ~(0x7f<<8)) | ((1 << 15) | (bl_config.pwm_pre_div<<8) | (1<<0)));
			}
			else if (bl_config.pwm_port == BL_PWM_B) {
				WRITE_MPEG_REG(PWM_MISC_REG_AB, (READ_MPEG_REG(PWM_MISC_REG_AB) & ~(0x7f<<16)) | ((1 << 23) | (bl_config.pwm_pre_div<<16) | (1<<1)));
			}
			else if (bl_config.pwm_port == BL_PWM_C) {
				WRITE_MPEG_REG(PWM_MISC_REG_CD, (READ_MPEG_REG(PWM_MISC_REG_CD) & ~(0x7f<<8)) | ((1 << 15) | (bl_config.pwm_pre_div<<8) | (1<<0)));  //enable pwm clk & pwm output
			}
			else if (bl_config.pwm_port == BL_PWM_D) {
				WRITE_MPEG_REG(PWM_MISC_REG_CD, (READ_MPEG_REG(PWM_MISC_REG_CD) & ~(0x7f<<16)) | ((1 << 23) | (bl_config.pwm_pre_div<<16) | (1<<1)));  //enable pwm clk & pwm output
			}
			for (i=0; i<bl_config.pinmux_clr_num; i++) {
				clear_mio_mux(bl_config.pinmux_clr[i][0], bl_config.pinmux_clr[i][1]);
			}
			for (i=0; i<bl_config.pinmux_set_num; i++) {
				set_mio_mux(bl_config.pinmux_set[i][0], bl_config.pinmux_set[i][1]);
			}
			
			if (bl_config.pwm_gpio_used)
				amlogic_gpio_set(bl_config.gpio, LCD_POWER_GPIO_OUTPUT_HIGH);
		}
	}
	else {
		if (bl_config.method == BL_CTL_GPIO) {
			amlogic_gpio_set(bl_config.gpio, LCD_POWER_GPIO_OUTPUT_LOW);
		}
		else {
			if (bl_config.pwm_gpio_used) {
				if (bl_config.gpio)
					amlogic_gpio_set(bl_config.gpio, LCD_POWER_GPIO_OUTPUT_LOW);
			}
			if (bl_config.pwm_port == BL_PWM_A) {
				WRITE_MPEG_REG(PWM_MISC_REG_AB, READ_MPEG_REG(PWM_MISC_REG_AB) & ~((1 << 15) | (1<<0)));
			}
			else if (bl_config.pwm_port == BL_PWM_B) {
				WRITE_MPEG_REG(PWM_MISC_REG_AB, READ_MPEG_REG(PWM_MISC_REG_AB) & ~((1 << 23) | (1<<1)));
			}
			else if (bl_config.pwm_port == BL_PWM_C) {
				WRITE_MPEG_REG(PWM_MISC_REG_CD, READ_MPEG_REG(PWM_MISC_REG_CD) & ~((1 << 15) | (1<<0)));  //disable pwm_clk & pwm port
			}
			else if (bl_config.pwm_port == BL_PWM_D) {
				WRITE_MPEG_REG(PWM_MISC_REG_CD, READ_MPEG_REG(PWM_MISC_REG_CD) & ~((1 << 23) | (1<<1)));  //disable pwm_clk & pwm port
			}
		}
	}
	printf("%s(): %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static void set_lcd_backlight_level(unsigned level)
{
	printf("set_backlight_level: %u, last level: %u\n", level, bl_level);
	level = (level > BL_LEVEL_MAX ? BL_LEVEL_MAX : (level < BL_LEVEL_MIN ? BL_LEVEL_MIN : level));
	bl_level = level;

	if (level > BL_LEVEL_MID)
		level = ((level - BL_LEVEL_MID) * (BL_LEVEL_MAX - BL_LEVEL_MAPPED_MID)) / (BL_LEVEL_MAX - BL_LEVEL_MID) + BL_LEVEL_MAPPED_MID;
	else
		level = ((level - BL_LEVEL_MIN) * (BL_LEVEL_MAPPED_MID - BL_LEVEL_MIN)) / (BL_LEVEL_MID - BL_LEVEL_MIN) + BL_LEVEL_MIN;
	
	if (bl_config.method == BL_CTL_GPIO) {
		level = bl_config.dim_min - ((level - BL_LEVEL_MIN) * (bl_config.dim_min - bl_config.dim_max)) / (BL_LEVEL_MAX - BL_LEVEL_MIN);
		WRITE_CBUS_REG_BITS(LED_PWM_REG0, level, 0, 4);
	}
	else {
		level = (bl_config.pwm_max - bl_config.pwm_min) * (level - BL_LEVEL_MIN) / (BL_LEVEL_MAX - BL_LEVEL_MIN) + bl_config.pwm_min;
		if (bl_config.pwm_port == BL_PWM_A) {
			WRITE_MPEG_REG(PWM_PWM_A, (level << 16) | (bl_config.pwm_cnt - level));
		}
		else if (bl_config.pwm_port == BL_PWM_B) {
			WRITE_MPEG_REG(PWM_PWM_B, (level << 16) | (bl_config.pwm_cnt - level));
		}
		else if (bl_config.pwm_port == BL_PWM_C) {
			WRITE_MPEG_REG(PWM_PWM_C, (level << 16) | (bl_config.pwm_cnt - level));
		}
		else if (bl_config.pwm_port == BL_PWM_D) {
			WRITE_MPEG_REG(PWM_PWM_D, (level << 16) | (bl_config.pwm_cnt - level));
		}
	}
}

static unsigned get_lcd_backlight_level(void)
{
    DPRINT("%s :%d\n", __FUNCTION__, bl_level);
    return bl_level;
}

static void lcd_power_ctrl(Bool_t status)
{
	int i;
	
	printf("%s(): %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
	if (status) {
		if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.type < LCD_POWER_TYPE_NULL) {
			DPRINT("lcd_power_on_uboot\n");
			switch (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.type) {
				case LCD_POWER_TYPE_CPU:
					amlogic_gpio_set(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.value);
					break;
				case LCD_POWER_TYPE_AXP202:
#ifdef CONFIG_AW_AXP20
					if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.value == LCD_POWER_GPIO_OUTPUT_LOW) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 0);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.value == LCD_POWER_GPIO_OUTPUT_HIGH) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 1);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.value == LCD_POWER_GPIO_INPUT) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 0);
					}
#endif
					break;
				case LCD_POWER_TYPE_AML1212:
#ifdef CONFIG_AML_PMU
					if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.value == LCD_POWER_GPIO_OUTPUT_LOW) {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 0);
					}
					else {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.gpio, 1);
					}
#endif
					break;
				default:
					break;
			}
			if (pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.delay > 0)
				mdelay(pDev->conf.lcd_power_ctrl.lcd_power_on_uboot.delay);
		}
		for (i=0; i<pDev->conf.lcd_power_ctrl.lcd_power_on_step; i++) {
			DPRINT("%s %s step %d\n", __FUNCTION__, (status ? "ON" : "OFF"), i+1);
			switch (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].type) {
				case LCD_POWER_TYPE_CPU:
					amlogic_gpio_set(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].value);
					break;
				case LCD_POWER_TYPE_AXP202:
#ifdef CONFIG_AW_AXP20
					if (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].value == LCD_POWER_GPIO_OUTPUT_LOW) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 0);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].value == LCD_POWER_GPIO_OUTPUT_HIGH) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 1);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].value == LCD_POWER_GPIO_INPUT) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 0);
					}
#endif			
					break;
				case LCD_POWER_TYPE_AML1212:
#ifdef CONFIG_AML_PMU
					if (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].value == LCD_POWER_GPIO_OUTPUT_LOW) {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 0);
					}
					else {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].gpio, 1);
					}
#endif
					break;
				case LCD_POWER_TYPE_SIGNAL:
					lcd_signals_ports_ctrl(ON);
					break;
				case LCD_POWER_TYPE_INIT:
					printf("lcd power ctrl ON step %d lcd_init function is to be done.\n", i+1);
					break;
				default:
					printf("lcd power ctrl ON step %d is null.\n", i+1);
					break;
			}
			if (pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].delay > 0)
				mdelay(pDev->conf.lcd_power_ctrl.lcd_power_on_config[i].delay);
		}
	}
	else {
		mdelay(30);
		for (i=0; i<pDev->conf.lcd_power_ctrl.lcd_power_off_step; i++) {
			DPRINT("%s %s step %d\n", __FUNCTION__, (status ? "ON" : "OFF"), i+1);
			switch (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].type) {
				case LCD_POWER_TYPE_CPU:
					amlogic_gpio_set(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].value);
					break;
				case LCD_POWER_TYPE_AXP202:
#ifdef CONFIG_AW_AXP
					if (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].value == LCD_POWER_GPIO_OUTPUT_LOW) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio,1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, 0);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].value == LCD_POWER_GPIO_OUTPUT_HIGH) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio,1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, 1);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].value == LCD_POWER_GPIO_INPUT) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, 0);
					}
#endif			
					break;
				case LCD_POWER_TYPE_AML1212:
#ifdef CONFIG_AML1212
					if (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].value == LCD_POWER_GPIO_OUTPUT_LOW) {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, 0);
					}
					else {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].gpio, 1);
					}
#endif
					break;
				case LCD_POWER_TYPE_SIGNAL:
					lcd_signals_ports_ctrl(OFF);
					break;
				case LCD_POWER_TYPE_INIT:
					printf("lcd power ctrl OFF step %d lcd_init function is to be done.\n", i+1);
					break;
				default:
					printf("lcd power ctrl OFF step %d is null.\n", i+1);
					break;
			}
			if (pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].delay > 0)
				mdelay(pDev->conf.lcd_power_ctrl.lcd_power_off_config[i].delay);
		}
		if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.type < LCD_POWER_TYPE_NULL) {
			DPRINT("lcd_power_off_uboot\n");
			switch (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.type) {
				case LCD_POWER_TYPE_CPU:
					amlogic_gpio_set(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.value);
					break;
				case LCD_POWER_TYPE_AXP202:
#ifdef CONFIG_AW_AXP20
					if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.value == LCD_POWER_GPIO_OUTPUT_LOW) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 0);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.value == LCD_POWER_GPIO_OUTPUT_HIGH) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 1);
						axp_gpio_set_value(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 1);
					}
					else if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.value == LCD_POWER_GPIO_INPUT) {
						axp_gpio_set_io(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 0);
					}
#endif
					break;
				case LCD_POWER_TYPE_AML1212:
#ifdef CONFIG_AML_PMU
					if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.value == LCD_POWER_GPIO_OUTPUT_LOW) {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 0);
					}
					else {
						aml_pmu_set_gpio(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.gpio, 1);
					}
#endif
					break;
				default:
					break;
			}
			if (pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.delay > 0)
				mdelay(pDev->conf.lcd_power_ctrl.lcd_power_off_uboot.delay);
		}
	}
	DPRINT("%s(): %s finished.\n", __FUNCTION__, (status ? "ON" : "OFF"));
}
#endif

static void set_lcd_gamma_table_ttl(u16 *data, u32 rgb_mask, u16 gamma_coeff)
{
    int i;

    while (!(READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x0 << HADR));
    for (i=0;i<256;i++) {
        while (!( READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        WRITE_MPEG_REG(GAMMA_DATA_PORT, (data[i] * gamma_coeff / 100));
    }
    while (!(READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x23 << HADR));
}

void set_lcd_gamma_table_lvds(u16 *data, u32 rgb_mask, u16 gamma_coeff)
{
    int i;

    while (!(READ_MPEG_REG(L_GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(L_GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x0 << HADR));
    for (i=0;i<256;i++) {
        while (!( READ_MPEG_REG(L_GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        WRITE_MPEG_REG(L_GAMMA_DATA_PORT, (data[i] * gamma_coeff / 100));
    }
    while (!(READ_MPEG_REG(L_GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(L_GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x23 << HADR));
}

static void set_video_adjust(Lcd_Config_t *pConf)
{
	DPRINT("vadj_brightness = 0x%x, vadj_contrast = 0x%x, vadj_saturation = 0x%x.\n", pConf->lcd_effect.vadj_brightness, pConf->lcd_effect.vadj_contrast, pConf->lcd_effect.vadj_saturation);
	WRITE_MPEG_REG(VPP_VADJ2_Y, (pConf->lcd_effect.vadj_brightness << 8) | (pConf->lcd_effect.vadj_contrast << 0));
	WRITE_MPEG_REG(VPP_VADJ2_MA_MB, (pConf->lcd_effect.vadj_saturation << 16));
	WRITE_MPEG_REG(VPP_VADJ2_MC_MD, (pConf->lcd_effect.vadj_saturation << 0));
	WRITE_MPEG_REG(VPP_VADJ_CTRL, 0xf);	//enable video adjust
}

static void write_tcon_double(Mlvds_Tcon_Config_t *mlvds_tcon)
{
    unsigned int tmp;
    int channel_num = mlvds_tcon->channel_num;
    int hv_sel = mlvds_tcon->hv_sel;
    int hstart_1 = mlvds_tcon->tcon_1st_hs_addr;
    int hend_1 = mlvds_tcon->tcon_1st_he_addr;
    int vstart_1 = mlvds_tcon->tcon_1st_vs_addr;
    int vend_1 = mlvds_tcon->tcon_1st_ve_addr;
    int hstart_2 = mlvds_tcon->tcon_2nd_hs_addr;
    int hend_2 = mlvds_tcon->tcon_2nd_he_addr;
    int vstart_2 = mlvds_tcon->tcon_2nd_vs_addr;
    int vend_2 = mlvds_tcon->tcon_2nd_ve_addr;

    tmp = READ_MPEG_REG(L_TCON_MISC_SEL_ADDR);
    switch(channel_num)
    {
        case 0 :
            WRITE_MPEG_REG(MTCON0_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON0_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON0_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON0_1ST_VE_ADDR, vend_1);
            WRITE_MPEG_REG(MTCON0_2ND_HS_ADDR, hstart_2);
            WRITE_MPEG_REG(MTCON0_2ND_HE_ADDR, hend_2);
            WRITE_MPEG_REG(MTCON0_2ND_VS_ADDR, vstart_2);
            WRITE_MPEG_REG(MTCON0_2ND_VE_ADDR, vend_2);
            tmp &= (~(1 << LCD_STH1_SEL)) | (hv_sel << LCD_STH1_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 1 :
            WRITE_MPEG_REG(MTCON1_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON1_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON1_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON1_1ST_VE_ADDR, vend_1);
            WRITE_MPEG_REG(MTCON1_2ND_HS_ADDR, hstart_2);
            WRITE_MPEG_REG(MTCON1_2ND_HE_ADDR, hend_2);
            WRITE_MPEG_REG(MTCON1_2ND_VS_ADDR, vstart_2);
            WRITE_MPEG_REG(MTCON1_2ND_VE_ADDR, vend_2);
            tmp &= (~(1 << LCD_CPV1_SEL)) | (hv_sel << LCD_CPV1_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 2 :
            WRITE_MPEG_REG(MTCON2_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON2_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON2_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON2_1ST_VE_ADDR, vend_1);
            WRITE_MPEG_REG(MTCON2_2ND_HS_ADDR, hstart_2);
            WRITE_MPEG_REG(MTCON2_2ND_HE_ADDR, hend_2);
            WRITE_MPEG_REG(MTCON2_2ND_VS_ADDR, vstart_2);
            WRITE_MPEG_REG(MTCON2_2ND_VE_ADDR, vend_2);
            tmp &= (~(1 << LCD_STV1_SEL)) | (hv_sel << LCD_STV1_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 3 :
            WRITE_MPEG_REG(MTCON3_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON3_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON3_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON3_1ST_VE_ADDR, vend_1);
            WRITE_MPEG_REG(MTCON3_2ND_HS_ADDR, hstart_2);
            WRITE_MPEG_REG(MTCON3_2ND_HE_ADDR, hend_2);
            WRITE_MPEG_REG(MTCON3_2ND_VS_ADDR, vstart_2);
            WRITE_MPEG_REG(MTCON3_2ND_VE_ADDR, vend_2);
            tmp &= (~(1 << LCD_OEV1_SEL)) | (hv_sel << LCD_OEV1_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 4 :
            WRITE_MPEG_REG(MTCON4_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON4_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON4_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON4_1ST_VE_ADDR, vend_1);
            tmp &= (~(1 << LCD_STH2_SEL)) | (hv_sel << LCD_STH2_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 5 :
            WRITE_MPEG_REG(MTCON5_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON5_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON5_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON5_1ST_VE_ADDR, vend_1);
            tmp &= (~(1 << LCD_CPV2_SEL)) | (hv_sel << LCD_CPV2_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 6 :
            WRITE_MPEG_REG(MTCON6_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON6_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON6_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON6_1ST_VE_ADDR, vend_1);
            tmp &= (~(1 << LCD_OEH_SEL)) | (hv_sel << LCD_OEH_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        case 7 :
            WRITE_MPEG_REG(MTCON7_1ST_HS_ADDR, hstart_1);
            WRITE_MPEG_REG(MTCON7_1ST_HE_ADDR, hend_1);
            WRITE_MPEG_REG(MTCON7_1ST_VS_ADDR, vstart_1);
            WRITE_MPEG_REG(MTCON7_1ST_VE_ADDR, vend_1);
            tmp &= (~(1 << LCD_OEV3_SEL)) | (hv_sel << LCD_OEV3_SEL);
            WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, tmp);
            break;
        default:
            break;
    }
}

static void set_tcon_ttl(Lcd_Config_t *pConf)
{
    printf("setup tcon ttl.\n");
	Lcd_Timing_t *tcon_adr = &(pConf->lcd_timing);
    set_lcd_gamma_table_ttl(pConf->lcd_effect.GammaTableR, H_SEL_R, pConf->lcd_effect.gamma_r_coeff);
    set_lcd_gamma_table_ttl(pConf->lcd_effect.GammaTableG, H_SEL_G, pConf->lcd_effect.gamma_g_coeff);
    set_lcd_gamma_table_ttl(pConf->lcd_effect.GammaTableB, H_SEL_B, pConf->lcd_effect.gamma_b_coeff);

    WRITE_MPEG_REG(GAMMA_CNTL_PORT, pConf->lcd_effect.gamma_cntl_port);
    WRITE_MPEG_REG(GAMMA_VCOM_HSWITCH_ADDR, pConf->lcd_effect.gamma_vcom_hswitch_addr);

    WRITE_MPEG_REG(RGB_BASE_ADDR,   pConf->lcd_effect.rgb_base_addr);
    WRITE_MPEG_REG(RGB_COEFF_ADDR,  pConf->lcd_effect.rgb_coeff_addr);
	WRITE_MPEG_REG(POL_CNTL_ADDR,   pConf->lcd_timing.pol_cntl_addr);
	if (pConf->lcd_effect.dith_user) {
		WRITE_MPEG_REG(DITH_CNTL_ADDR,  pConf->lcd_effect.dith_cntl_addr);
	}
	else {
		if(pConf->lcd_basic.lcd_bits == 8)
			WRITE_MPEG_REG(DITH_CNTL_ADDR,  0x400);
		else
			WRITE_MPEG_REG(DITH_CNTL_ADDR,  0x600);
	}
	
    WRITE_MPEG_REG(STH1_HS_ADDR,    tcon_adr->sth1_hs_addr);
    WRITE_MPEG_REG(STH1_HE_ADDR,    tcon_adr->sth1_he_addr);
    WRITE_MPEG_REG(STH1_VS_ADDR,    tcon_adr->sth1_vs_addr);
    WRITE_MPEG_REG(STH1_VE_ADDR,    tcon_adr->sth1_ve_addr);

    WRITE_MPEG_REG(OEH_HS_ADDR,     tcon_adr->oeh_hs_addr);
    WRITE_MPEG_REG(OEH_HE_ADDR,     tcon_adr->oeh_he_addr);
    WRITE_MPEG_REG(OEH_VS_ADDR,     tcon_adr->oeh_vs_addr);
    WRITE_MPEG_REG(OEH_VE_ADDR,     tcon_adr->oeh_ve_addr);
/*
    WRITE_MPEG_REG(VCOM_HSWITCH_ADDR, tcon_adr->vcom_hswitch_addr);
    WRITE_MPEG_REG(VCOM_VS_ADDR,    tcon_adr->vcom_vs_addr);
    WRITE_MPEG_REG(VCOM_VE_ADDR,    tcon_adr->vcom_ve_addr);

    WRITE_MPEG_REG(CPV1_HS_ADDR,    tcon_adr->cpv1_hs_addr);
    WRITE_MPEG_REG(CPV1_HE_ADDR,    tcon_adr->cpv1_he_addr);
    WRITE_MPEG_REG(CPV1_VS_ADDR,    tcon_adr->cpv1_vs_addr);
    WRITE_MPEG_REG(CPV1_VE_ADDR,    tcon_adr->cpv1_ve_addr);
*/
    WRITE_MPEG_REG(STV1_HS_ADDR,    tcon_adr->stv1_hs_addr);
    WRITE_MPEG_REG(STV1_HE_ADDR,    tcon_adr->stv1_he_addr);
    WRITE_MPEG_REG(STV1_VS_ADDR,    tcon_adr->stv1_vs_addr);
    WRITE_MPEG_REG(STV1_VE_ADDR,    tcon_adr->stv1_ve_addr);
/*
    WRITE_MPEG_REG(OEV1_HS_ADDR,    tcon_adr->oev1_hs_addr);
    WRITE_MPEG_REG(OEV1_HE_ADDR,    tcon_adr->oev1_he_addr);
    WRITE_MPEG_REG(OEV1_VS_ADDR,    tcon_adr->oev1_vs_addr);
    WRITE_MPEG_REG(OEV1_VE_ADDR,    tcon_adr->oev1_ve_addr);
*/
    WRITE_MPEG_REG(INV_CNT_ADDR,    tcon_adr->inv_cnt_addr);
    WRITE_MPEG_REG(TCON_MISC_SEL_ADDR, 	tcon_adr->tcon_misc_sel_addr);
    WRITE_MPEG_REG(DUAL_PORT_CNTL_ADDR, tcon_adr->dual_port_cntl_addr);

    CLEAR_MPEG_REG_MASK(VPP_MISC, VPP_OUT_SATURATE);    
}

static void set_tcon_lvds(Lcd_Config_t *pConf)
{
    printf("%s.\n",__FUNCTION__);
	Lcd_Timing_t *tcon_adr = &(pConf->lcd_timing);
    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableR, H_SEL_R, pConf->lcd_effect.gamma_r_coeff);
    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableG, H_SEL_G, pConf->lcd_effect.gamma_g_coeff);
    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableB, H_SEL_B, pConf->lcd_effect.gamma_b_coeff);

    WRITE_MPEG_REG(L_GAMMA_CNTL_PORT, pConf->lcd_effect.gamma_cntl_port);
    WRITE_MPEG_REG(L_GAMMA_VCOM_HSWITCH_ADDR, pConf->lcd_effect.gamma_vcom_hswitch_addr);

    WRITE_MPEG_REG(L_RGB_BASE_ADDR,   pConf->lcd_effect.rgb_base_addr);
    WRITE_MPEG_REG(L_RGB_COEFF_ADDR,  pConf->lcd_effect.rgb_coeff_addr);
    //WRITE_MPEG_REG(L_POL_CNTL_ADDR,   pConf->lcd_timing.pol_cntl_addr);
    if(pConf->lcd_basic.lcd_bits == 8)
		WRITE_MPEG_REG(L_DITH_CNTL_ADDR,  0x400);
	else
		WRITE_MPEG_REG(L_DITH_CNTL_ADDR,  0x600);
	
    WRITE_MPEG_REG(L_INV_CNT_ADDR,    tcon_adr->inv_cnt_addr);
    WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, 	tcon_adr->tcon_misc_sel_addr);
    WRITE_MPEG_REG(L_DUAL_PORT_CNTL_ADDR, tcon_adr->dual_port_cntl_addr);

    CLEAR_MPEG_REG_MASK(VPP_MISC, VPP_OUT_SATURATE);
}

// Set the mlvds TCON
// this function should support dual gate or singal gate TCON setting.
// singal gate TCON, Scan Function TO DO.
// scan_function   // 0 - Z1, 1 - Z2, 2- Gong
static void set_tcon_mlvds(Lcd_Config_t *pConf)
{
    printf("%s.\n",__FUNCTION__);
	Mlvds_Tcon_Config_t *mlvds_tconfig_l = pConf->lvds_mlvds_config.mlvds_tcon_config;
    int dual_gate = pConf->lvds_mlvds_config.mlvds_config->test_dual_gate;
    int bit_num = pConf->lcd_basic.lcd_bits;
    int pair_num = pConf->lvds_mlvds_config.mlvds_config->test_pair_num;

    unsigned int data32;

    int pclk_div;
    int ext_pixel = dual_gate ? pConf->lvds_mlvds_config.mlvds_config->total_line_clk : 0;
    int dual_wr_rd_start;
    int i = 0;

//    debug(" Notice: Setting VENC_DVI_SETTING[0x%4x] and GAMMA_CNTL_PORT[0x%4x].LCD_GAMMA_EN as 0 temporary\n", VENC_DVI_SETTING, GAMMA_CNTL_PORT);
//    debug(" Otherwise, the panel will display color abnormal.\n");
//    WRITE_MPEG_REG(VENC_DVI_SETTING, 0);

    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableR, H_SEL_R, pConf->lcd_effect.gamma_r_coeff);
    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableG, H_SEL_G, pConf->lcd_effect.gamma_g_coeff);
    set_lcd_gamma_table_lvds(pConf->lcd_effect.GammaTableB, H_SEL_B, pConf->lcd_effect.gamma_b_coeff);

    WRITE_MPEG_REG(L_GAMMA_CNTL_PORT, pConf->lcd_effect.gamma_cntl_port);
	WRITE_MPEG_REG(L_GAMMA_VCOM_HSWITCH_ADDR, pConf->lcd_effect.gamma_vcom_hswitch_addr);

    WRITE_MPEG_REG(L_RGB_BASE_ADDR, pConf->lcd_effect.rgb_base_addr);
    WRITE_MPEG_REG(L_RGB_COEFF_ADDR, pConf->lcd_effect.rgb_coeff_addr);
    //WRITE_MPEG_REG(L_POL_CNTL_ADDR, pConf->lcd_timing.pol_cntl_addr);
    if(pConf->lcd_basic.lcd_bits == 8)
		WRITE_MPEG_REG(L_DITH_CNTL_ADDR,  0x400);
	else
		WRITE_MPEG_REG(L_DITH_CNTL_ADDR,  0x600);

//    WRITE_MPEG_REG(L_INV_CNT_ADDR, pConf->lcd_timing.inv_cnt_addr);
//    WRITE_MPEG_REG(L_TCON_MISC_SEL_ADDR, pConf->lcd_timing.tcon_misc_sel_addr);
//    WRITE_MPEG_REG(L_DUAL_PORT_CNTL_ADDR, pConf->lcd_timing.dual_port_cntl_addr);
//
//    CLEAR_MPEG_REG_MASK(VPP_MISC, VPP_OUT_SATURATE);

    data32 = (0x9867 << tcon_pattern_loop_data) |
             (1 << tcon_pattern_loop_start) |
             (4 << tcon_pattern_loop_end) |
             (1 << ((mlvds_tconfig_l[6].channel_num)+tcon_pattern_enable)); // POL_CHANNEL use pattern generate

    WRITE_MPEG_REG(L_TCON_PATTERN_HI,  (data32 >> 16));
    WRITE_MPEG_REG(L_TCON_PATTERN_LO, (data32 & 0xffff));

    pclk_div = (bit_num == 8) ? 3 : // phy_clk / 8
                                2 ; // phy_clk / 6
   data32 = (1 << ((mlvds_tconfig_l[7].channel_num)-2+tcon_pclk_enable)) |  // enable PCLK_CHANNEL
            (pclk_div << tcon_pclk_div) |
            (
              (pair_num == 6) ?
              (
              ((bit_num == 8) & dual_gate) ?
              (
                (0 << (tcon_delay + 0*3)) |
                (0 << (tcon_delay + 1*3)) |
                (0 << (tcon_delay + 2*3)) |
                (0 << (tcon_delay + 3*3)) |
                (0 << (tcon_delay + 4*3)) |
                (0 << (tcon_delay + 5*3)) |
                (0 << (tcon_delay + 6*3)) |
                (0 << (tcon_delay + 7*3))
              ) :
              (
                (0 << (tcon_delay + 0*3)) |
                (0 << (tcon_delay + 1*3)) |
                (0 << (tcon_delay + 2*3)) |
                (0 << (tcon_delay + 3*3)) |
                (0 << (tcon_delay + 4*3)) |
                (0 << (tcon_delay + 5*3)) |
                (0 << (tcon_delay + 6*3)) |
                (0 << (tcon_delay + 7*3))
              )
              ) :
              (
              ((bit_num == 8) & dual_gate) ?
              (
                (0 << (tcon_delay + 0*3)) |
                (0 << (tcon_delay + 1*3)) |
                (0 << (tcon_delay + 2*3)) |
                (0 << (tcon_delay + 3*3)) |
                (0 << (tcon_delay + 4*3)) |
                (0 << (tcon_delay + 5*3)) |
                (0 << (tcon_delay + 6*3)) |
                (0 << (tcon_delay + 7*3))
              ) :
              (bit_num == 8) ?
              (
                (0 << (tcon_delay + 0*3)) |
                (0 << (tcon_delay + 1*3)) |
                (0 << (tcon_delay + 2*3)) |
                (0 << (tcon_delay + 3*3)) |
                (0 << (tcon_delay + 4*3)) |
                (0 << (tcon_delay + 5*3)) |
                (0 << (tcon_delay + 6*3)) |
                (0 << (tcon_delay + 7*3))
              ) :
              (
                (0 << (tcon_delay + 0*3)) |
                (0 << (tcon_delay + 1*3)) |
                (0 << (tcon_delay + 2*3)) |
                (0 << (tcon_delay + 3*3)) |
                (0 << (tcon_delay + 4*3)) |
                (0 << (tcon_delay + 5*3)) |
                (0 << (tcon_delay + 6*3)) |
                (0 << (tcon_delay + 7*3))
              )
              )
            );

    WRITE_MPEG_REG(TCON_CONTROL_HI,  (data32 >> 16));
    WRITE_MPEG_REG(TCON_CONTROL_LO, (data32 & 0xffff));

    WRITE_MPEG_REG( L_TCON_DOUBLE_CTL,
                   (1<<(mlvds_tconfig_l[3].channel_num))   // invert CPV
                  );

    // for channel 4-7, set second setting same as first
    WRITE_MPEG_REG(L_DE_HS_ADDR, (0x3 << 14) | ext_pixel);   // 0x3 -- enable double_tcon fir channel7:6
    WRITE_MPEG_REG(L_DE_HE_ADDR, (0x3 << 14) | ext_pixel);   // 0x3 -- enable double_tcon fir channel5:4
    WRITE_MPEG_REG(L_DE_VS_ADDR, (0x3 << 14) | 0);	// 0x3 -- enable double_tcon fir channel3:2
    WRITE_MPEG_REG(L_DE_VE_ADDR, (0x3 << 14) | 0);	// 0x3 -- enable double_tcon fir channel1:0

    dual_wr_rd_start = 0x5d;
    WRITE_MPEG_REG(MLVDS_DUAL_GATE_WR_START, dual_wr_rd_start);
    WRITE_MPEG_REG(MLVDS_DUAL_GATE_WR_END, dual_wr_rd_start + 1280);
    WRITE_MPEG_REG(MLVDS_DUAL_GATE_RD_START, dual_wr_rd_start + ext_pixel - 2);
    WRITE_MPEG_REG(MLVDS_DUAL_GATE_RD_END, dual_wr_rd_start + 1280 + ext_pixel - 2);

    WRITE_MPEG_REG(MLVDS_SECOND_RESET_CTL, (pConf->lvds_mlvds_config.mlvds_config->mlvds_insert_start + ext_pixel));

    data32 = (0 << ((mlvds_tconfig_l[5].channel_num)+mlvds_tcon_field_en)) |  // enable EVEN_F on TCON channel 6
             ( (0x0 << mlvds_scan_mode_odd) | (0x0 << mlvds_scan_mode_even)
             ) | (0 << mlvds_scan_mode_start_line);

    WRITE_MPEG_REG(MLVDS_DUAL_GATE_CTL_HI,  (data32 >> 16));
    WRITE_MPEG_REG(MLVDS_DUAL_GATE_CTL_LO, (data32 & 0xffff));

    debug("write minilvds tcon 0~7.\n");
    for(i = 0; i < 8; i++)
    {
		write_tcon_double(&mlvds_tconfig_l[i]);
    }
}

static void set_video_spread_spectrum(int video_pll_sel, int video_ss_level)
{ 
    if (video_pll_sel){
	    switch (video_ss_level)
		{
			case 0:  // disable ss
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x814d3928 );
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x6b425012 );
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x110 );
				break;
			case 1:  //about 1%
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x4d625012);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x130);
				break;
			case 2:  //about 2%
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x2d425012);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x130);
				break;
			case 3:  //about 3%
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x1d425012);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x130);
				break;		
			case 4:  //about 4%
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x0d125012);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x130);
				break;
			case 5:  //about 5%
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x0e425012);
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x130);
				break;	
			default:  //disable ss
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL2, 0x814d3928 );
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL3, 0x6b425012 );
				WRITE_MPEG_REG(HHI_VIID_PLL_CNTL4, 0x110 );		
		}
	}
	else{
		switch (video_ss_level)
		{
			case 0:  // disable ss
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x814d3928 );
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x6b425012 );
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x110 );
				break;
			case 1:  //about 1%
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x4d625012);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x130);
				break;
			case 2:  //about 2%
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x2d425012);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x130);
				break;
			case 3:  //about 3%
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x1d425012);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x130);
				break;
			case 4:  //about 4%
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x0d125012);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x130);
				break;
			case 5:  //about 5%
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x16110696);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x0e425012);
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x130);
				break;	
			default:  //disable ss
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL2, 0x814d3928 );
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL3, 0x6b425012 );
				WRITE_MPEG_REG(HHI_VID_PLL_CNTL4, 0x110 );
		}
	}
}

static void vclk_set_lcd(int lcd_type, int pll_sel, int pll_div_sel, int vclk_sel, unsigned long pll_reg, unsigned long vid_div_reg, unsigned int xd)
{
    DPRINT("setup lcd clk.\n");
    vid_div_reg |= (1 << 16) ; // turn clock gate on
    vid_div_reg |= (pll_sel << 15); // vid_div_clk_sel

    if(vclk_sel) {
      WRITE_MPEG_REG( HHI_VIID_CLK_CNTL, READ_MPEG_REG(HHI_VIID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0
    }
    else {
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) & ~(1 << 20) );     //disable clk_div1
    }

    // delay 2uS to allow the sync mux to switch over
    //WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 2 ) {}
	udelay(2);

    if(pll_sel){
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL, pll_reg|(1<<29) );        
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL2, 0x814d3928 );
		WRITE_MPEG_REG( HHI_VIID_PLL_CNTL3, 0x6b425012 );
		WRITE_MPEG_REG( HHI_VIID_PLL_CNTL4, 0x110 );
		WRITE_MPEG_REG( HHI_VIID_PLL_CNTL, pll_reg );
    }
    else{
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL, pll_reg|(1<<29) );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL2, 0x814d3928 );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL3, 0x6b425012 );
		WRITE_MPEG_REG( HHI_VID_PLL_CNTL4, 0x110 );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL, pll_reg );
    }
	udelay(10);
	
    if(pll_div_sel ) {
		WRITE_MPEG_REG( HHI_VIID_DIVIDER_CNTL,   vid_div_reg);
		//reset HHI_VIID_DIVIDER_CNTL
		WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)|(1<<7));    //0x104c[7]:SOFT_RESET_POST
        WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)|(1<<3));    //0x104c[3]:SOFT_RESET_PRE
        WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)&(~(1<<1)));    //0x104c[1]:RESET_N_POST
        WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)&(~(1<<0)));    //0x104c[0]:RESET_N_PRE
        mdelay(2);
        WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)&(~((1<<7)|(1<<3))));
        WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL)|((1<<1)|(1<<0)));
	}	
    else {
		WRITE_MPEG_REG( HHI_VID_DIVIDER_CNTL,   vid_div_reg);
        //reset HHI_VID_DIVIDER_CNTL
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)|(1<<7));    //0x1066[7]:SOFT_RESET_POST
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)|(1<<3));    //0x1066[3]:SOFT_RESET_PRE
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)&(~(1<<1)));    //0x1066[1]:RESET_N_POST
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)&(~(1<<0)));    //0x1066[0]:RESET_N_PRE
        mdelay(2);
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)&(~((1<<7)|(1<<3))));
        WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL)|((1<<1)|(1<<0)));
    }

    if(vclk_sel) WRITE_MPEG_REG( HHI_VIID_CLK_DIV, (READ_MPEG_REG(HHI_VIID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value
    else WRITE_MPEG_REG( HHI_VID_CLK_DIV, (READ_MPEG_REG(HHI_VID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value

    // delay 5uS
    //WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 5 ) {}
	udelay(5);

    if(vclk_sel) {
      if(pll_div_sel) WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      WRITE_MPEG_REG( HHI_VIID_CLK_CNTL, READ_MPEG_REG(HHI_VIID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0
    }
    else {
      if(pll_div_sel) WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) |  (1 << 20) );     //enable clk_div1
    }
    // delay 2uS
    //WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 2 ) {}
	udelay(2);

    // set tcon_clko setting
    WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL,
                    (
                    (0 << 11) |     //clk_div1_sel
                    (1 << 10) |     //clk_inv
                    (0 << 9)  |     //neg_edge_sel
                    (0 << 5)  |     //tcon high_thresh
                    (0 << 1)  |     //tcon low_thresh
                    (1 << 0)        //cntl_clk_en1
                    ),
                    20, 12);

    if(lcd_type == LCD_DIGITAL_TTL){
		if(vclk_sel) {
			WRITE_MPEG_REG_BITS (HHI_VID_CLK_DIV, 8, 20, 4); // [23:20] enct_clk_sel
		}
		else {
			WRITE_MPEG_REG_BITS (HHI_VID_CLK_DIV, 0, 20, 4); // [23:20] enct_clk_sel
		}
	}
	else {
		if(vclk_sel) {
			WRITE_MPEG_REG_BITS (HHI_VIID_CLK_DIV, 8, 12, 4); // [23:20] encl_clk_sel
		}
		else {
			WRITE_MPEG_REG_BITS (HHI_VIID_CLK_DIV, 0, 12, 4); // [23:20] encl_clk_sel
		}
	}
	
    if(vclk_sel) {
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL,
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 1, 15, 1);  //soft reset
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    else {
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL,
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 1, 15, 1);  //soft reset
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }

    //printf("video pll1 clk = %d\n", (int)clk_util_clk_msr(VID_PLL_CLK));
    printf("video pll2 clk = %d\n", (int)clk_util_clk_msr(VID2_PLL_CLK));
    printf("cts_enct clk = %d\n", (int)clk_util_clk_msr(CTS_ENCT_CLK));
	printf("cts_encl clk = %d\n", (int)clk_util_clk_msr(CTS_ENCL_CLK));
}

static void set_pll_ttl(Lcd_Config_t *pConf)
{
	unsigned pll_reg, div_reg, xd;
	int pll_sel, pll_div_sel, vclk_sel;
	int lcd_type, ss_level;

	pll_reg = pConf->lcd_timing.pll_ctrl;
	div_reg = pConf->lcd_timing.div_ctrl | 0x3;	
	ss_level = ((pConf->lcd_timing.clk_ctrl) >>16) & 0xf;
	pll_sel = ((pConf->lcd_timing.clk_ctrl) >>12) & 0x1;
	pll_div_sel = ((pConf->lcd_timing.clk_ctrl) >>8) & 0x1;
	vclk_sel = ((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;
	xd = pConf->lcd_timing.clk_ctrl & 0xf;
	
	lcd_type = pConf->lcd_basic.lcd_type;

	printf("ss_level=%d, pll_reg=0x%x, div_reg=0x%x, xd=%d.\n", ss_level, pll_reg, div_reg, xd);
	vclk_set_lcd(lcd_type, pll_sel, pll_div_sel, vclk_sel, pll_reg, div_reg, xd);	
	set_video_spread_spectrum(pll_sel, ss_level);
}

static void clk_util_lvds_set_clk_div(  unsigned long divn_sel, unsigned long divn_tcnt, unsigned long div2_en)
{
    // assign          lvds_div_phy_clk_en     = tst_lvds_tmode ? 1'b1         : phy_clk_cntl[10];
    // assign          lvds_div_div2_sel       = tst_lvds_tmode ? atest_i[5]   : phy_clk_cntl[9];
    // assign          lvds_div_sel            = tst_lvds_tmode ? atest_i[7:6] : phy_clk_cntl[8:7];
    // assign          lvds_div_tcnt           = tst_lvds_tmode ? 3'd6         : phy_clk_cntl[6:4];
    // If dividing by 1, just select the divide by 1 path
    if( divn_tcnt == 1 ) {
        divn_sel = 0;
    }
    WRITE_MPEG_REG( LVDS_PHY_CLK_CNTL, ((READ_MPEG_REG(LVDS_PHY_CLK_CNTL) & ~((0x3 << 7) | (1 << 9) | (0x7 << 4))) | ((1 << 10) | (divn_sel << 7) | (div2_en << 9) | (((divn_tcnt-1)&0x7) << 4))) );
}

static void set_pll_lvds(Lcd_Config_t *pConf)
{
    debug("%s\n", __FUNCTION__);

    unsigned int pll_div_post, phy_clk_div2, FIFO_CLK_SEL;
    unsigned long rd_data;

    unsigned pll_reg, div_reg, xd;
    int pll_sel, pll_div_sel, vclk_sel;
	int lcd_type, ss_level;
	
    pll_reg = pConf->lcd_timing.pll_ctrl;
    div_reg = pConf->lcd_timing.div_ctrl | 0x3;    
	ss_level = ((pConf->lcd_timing.clk_ctrl) >>16) & 0xf;
    pll_sel = ((pConf->lcd_timing.clk_ctrl) >>12) & 0x1;
    //pll_div_sel = ((pConf->lcd_timing.clk_ctrl) >>8) & 0x1;
	pll_div_sel = 1;
    vclk_sel = ((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;
	//xd = pConf->lcd_timing.clk_ctrl & 0xf;
	xd = 1;
	
	lcd_type = pConf->lcd_basic.lcd_type;
	
    pll_div_post = 7;

    phy_clk_div2 = 0;
	
	div_reg = (div_reg | (1 << 8) | (1 << 11) | ((pll_div_post-1) << 12) | (phy_clk_div2 << 10));
	printf("ss_level=%d, pll_reg=0x%x, div_reg=0x%x, xd=%d.\n", ss_level, pll_reg, div_reg, xd);
    vclk_set_lcd(lcd_type, pll_sel, pll_div_sel, vclk_sel, pll_reg, div_reg, xd);
	set_video_spread_spectrum(pll_sel, ss_level);
	
	clk_util_lvds_set_clk_div(1, pll_div_post, phy_clk_div2);
	
    WRITE_MPEG_REG( LVDS_PHY_CNTL0, 0xffff );

    //    lvds_gen_cntl       <= {10'h0,      // [15:4] unused
    //                            2'h1,       // [5:4] divide by 7 in the PHY
    //                            1'b0,       // [3] fifo_en
    //                            1'b0,       // [2] wr_bist_gate
    //                            2'b00};     // [1:0] fifo_wr mode
    FIFO_CLK_SEL = 1; // div7
    rd_data = READ_MPEG_REG(LVDS_GEN_CNTL);
    rd_data &= 0xffcf | (FIFO_CLK_SEL<< 4);
    WRITE_MPEG_REG( LVDS_GEN_CNTL, rd_data);

    // Set Hard Reset lvds_phy_ser_top
    WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) & 0x7fff);
    // Release Hard Reset lvds_phy_ser_top
    WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) | 0x8000);
}

static void set_pll_mlvds(Lcd_Config_t *pConf)
{
    debug("%s\n", __FUNCTION__);

    int test_bit_num = pConf->lcd_basic.lcd_bits;
    int test_dual_gate = pConf->lvds_mlvds_config.mlvds_config->test_dual_gate;
    int test_pair_num= pConf->lvds_mlvds_config.mlvds_config->test_pair_num;
    int pll_div_post, phy_clk_div2, FIFO_CLK_SEL, MPCLK_DELAY, MCLK_half, MCLK_half_delay;
    unsigned int data32;
    unsigned long mclk_pattern_dual_6_6;
    int test_high_phase = (test_bit_num != 8) | test_dual_gate;
    unsigned long rd_data;

    unsigned pll_reg, div_reg, xd;
    int pll_sel, pll_div_sel, vclk_sel;
	int lcd_type, ss_level;
	
    pll_reg = pConf->lcd_timing.pll_ctrl;
    div_reg = pConf->lcd_timing.div_ctrl | 0x3;    
	ss_level = ((pConf->lcd_timing.clk_ctrl) >>16) & 0xf;
    pll_sel = ((pConf->lcd_timing.clk_ctrl) >>12) & 0x1;
    //pll_div_sel = ((pConf->lcd_timing.clk_ctrl) >>8) & 0x1;
	pll_div_sel = 1;
    vclk_sel = ((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;
	//xd = pConf->lcd_timing.clk_ctrl & 0xf;
	xd = 1;
	
	lcd_type = pConf->lcd_basic.lcd_type;

    switch(pConf->lvds_mlvds_config.mlvds_config->TL080_phase) {
      case 0 :
        mclk_pattern_dual_6_6 = 0xc3c3c3;
        MCLK_half = 1;
        break;
      case 1 :
        mclk_pattern_dual_6_6 = 0xc3c3c3;
        MCLK_half = 0;
        break;
      case 2 :
        mclk_pattern_dual_6_6 = 0x878787;
        MCLK_half = 1;
        break;
      case 3 :
        mclk_pattern_dual_6_6 = 0x878787;
        MCLK_half = 0;
        break;
      case 4 :
        mclk_pattern_dual_6_6 = 0x3c3c3c;
        MCLK_half = 1;
        break;
       case 5 :
        mclk_pattern_dual_6_6 = 0x3c3c3c;
        MCLK_half = 0;
        break;
       case 6 :
        mclk_pattern_dual_6_6 = 0x787878;
        MCLK_half = 1;
        break;
      default : // case 7
        mclk_pattern_dual_6_6 = 0x787878;
        MCLK_half = 0;
        break;
    }

    pll_div_post = (test_bit_num == 8) ?
                      (test_dual_gate ? 4 : 8) :
                      (test_dual_gate ? 3 : 6) ;

    phy_clk_div2 = (test_pair_num != 3);
	
	div_reg = (div_reg | (1 << 8) | (1 << 11) | ((pll_div_post-1) << 12) | (phy_clk_div2 << 10));
	printf("ss_level=%d, pll_reg=0x%x, div_reg=0x%x, xd=%d.\n", ss_level, pll_reg, div_reg, xd);
    vclk_set_lcd(lcd_type, pll_sel, pll_div_sel, vclk_sel, pll_reg, div_reg, xd);
	set_video_spread_spectrum(pll_sel, ss_level);
	
	clk_util_lvds_set_clk_div(1, pll_div_post, phy_clk_div2);
	
	//enable v2_clk div
    // WRITE_MPEG_REG( HHI_VIID_CLK_CNTL, READ_MPEG_REG(HHI_VIID_CLK_CNTL) | (0xF << 0) );
    // WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) | (0xF << 0) );

    WRITE_MPEG_REG( LVDS_PHY_CNTL0, 0xffff );

    //    lvds_gen_cntl       <= {10'h0,      // [15:4] unused
    //                            2'h1,       // [5:4] divide by 7 in the PHY
    //                            1'b0,       // [3] fifo_en
    //                            1'b0,       // [2] wr_bist_gate
    //                            2'b00};     // [1:0] fifo_wr mode

    FIFO_CLK_SEL = (test_bit_num == 8) ? 2 : // div8
                                    0 ; // div6
    rd_data = READ_MPEG_REG(LVDS_GEN_CNTL);
    rd_data = (rd_data & 0xffcf) | (FIFO_CLK_SEL<< 4);
    WRITE_MPEG_REG( LVDS_GEN_CNTL, rd_data);

    MPCLK_DELAY = (test_pair_num == 6) ?
                  ((test_bit_num == 8) ? (test_dual_gate ? 5 : 3) : 2) :
                  ((test_bit_num == 8) ? 3 : 3) ;

    MCLK_half_delay = pConf->lvds_mlvds_config.mlvds_config->phase_select ? MCLK_half :
                      (test_dual_gate & (test_bit_num == 8) & (test_pair_num != 6));

    if(test_high_phase)
    {
        if(test_dual_gate)
        data32 = (MPCLK_DELAY << mpclk_dly) |
                 (((test_bit_num == 8) ? 3 : 2) << mpclk_div) |
                 (1 << use_mpclk) |
                 (MCLK_half_delay << mlvds_clk_half_delay) |
                 (((test_bit_num == 8) ? (
                                           (test_pair_num == 6) ? 0x999999 : // DIV4
                                                                  0x555555   // DIV2
                                         ) :
                                         (
                                           (test_pair_num == 6) ? mclk_pattern_dual_6_6 : // DIV8
                                                                  0x999999   // DIV4
                                         )
                 ) << mlvds_clk_pattern);
        else if(test_bit_num == 8)
            data32 = (MPCLK_DELAY << mpclk_dly) |
                     (((test_bit_num == 8) ? 3 : 2) << mpclk_div) |
                     (1 << use_mpclk) |
                     (0 << mlvds_clk_half_delay) |
                     (0xc3c3c3 << mlvds_clk_pattern);      // DIV 8
        else
            data32 = (MPCLK_DELAY << mpclk_dly) |
                     (((test_bit_num == 8) ? 3 : 2) << mpclk_div) |
                     (1 << use_mpclk) |
                     (0 << mlvds_clk_half_delay) |
                     (
                       (
                         (test_pair_num == 6) ? 0xc3c3c3 : // DIV8
                                                0x999999   // DIV4
                       ) << mlvds_clk_pattern
                     );
    }
    else {
        if(test_pair_num == 6) {
            data32 = (MPCLK_DELAY << mpclk_dly) |
                     (((test_bit_num == 8) ? 3 : 2) << mpclk_div) |
                     (1 << use_mpclk) |
                     (0 << mlvds_clk_half_delay) |
                     (
                       (
                         (test_pair_num == 6) ? 0x999999 : // DIV4
                                                0x555555   // DIV2
                       ) << mlvds_clk_pattern
                     );
        }
        else {
            data32 = (1 << mlvds_clk_half_delay) |
                   (0x555555 << mlvds_clk_pattern);      // DIV 2
        }
    }

    WRITE_MPEG_REG(MLVDS_CLK_CTL_HI,  (data32 >> 16));
    WRITE_MPEG_REG(MLVDS_CLK_CTL_LO, (data32 & 0xffff));

	//pll_div_sel
    if(1){
		// Set Soft Reset vid_pll_div_pre
		WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL) | 0x00008);
		// Set Hard Reset vid_pll_div_post
		WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL) & 0x1fffd);
		// Set Hard Reset lvds_phy_ser_top
		WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) & 0x7fff);
		// Release Hard Reset lvds_phy_ser_top
		WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) | 0x8000);
		// Release Hard Reset vid_pll_div_post
		WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL) | 0x00002);
		// Release Soft Reset vid_pll_div_pre
		WRITE_MPEG_REG(HHI_VIID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VIID_DIVIDER_CNTL) & 0x1fff7);
	}
	else{
		// Set Soft Reset vid_pll_div_pre
		WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL) | 0x00008);
		// Set Hard Reset vid_pll_div_post
		WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL) & 0x1fffd);
		// Set Hard Reset lvds_phy_ser_top
		WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) & 0x7fff);
		// Release Hard Reset lvds_phy_ser_top
		WRITE_MPEG_REG(LVDS_PHY_CLK_CNTL, READ_MPEG_REG(LVDS_PHY_CLK_CNTL) | 0x8000);
		// Release Hard Reset vid_pll_div_post
		WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL) | 0x00002);
		// Release Soft Reset vid_pll_div_pre
		WRITE_MPEG_REG(HHI_VID_DIVIDER_CNTL, READ_MPEG_REG(HHI_VID_DIVIDER_CNTL) & 0x1fff7);
    }	
}

static void venc_set_ttl(Lcd_Config_t *pConf)
{
    DPRINT("%s.\n",__FUNCTION__);
	WRITE_MPEG_REG(ENCT_VIDEO_EN,           0);
    WRITE_MPEG_REG(VPU_VIU_VENC_MUX_CTRL,
       (3<<0) |    // viu1 select enct
       (3<<2)      // viu2 select enct
    );
    WRITE_MPEG_REG(ENCT_VIDEO_MODE,        0);
    WRITE_MPEG_REG(ENCT_VIDEO_MODE_ADV,    0x0418);
	
	// bypass filter
    WRITE_MPEG_REG(ENCT_VIDEO_FILT_CTRL,    0x1000);
    WRITE_MPEG_REG(VENC_DVI_SETTING,        0x11);
    WRITE_MPEG_REG(VENC_VIDEO_PROG_MODE,    0x100);

    WRITE_MPEG_REG(ENCT_VIDEO_MAX_PXCNT,    pConf->lcd_basic.h_period - 1);
    WRITE_MPEG_REG(ENCT_VIDEO_MAX_LNCNT,    pConf->lcd_basic.v_period - 1);

    WRITE_MPEG_REG(ENCT_VIDEO_HAVON_BEGIN,  pConf->lcd_timing.video_on_pixel);
    WRITE_MPEG_REG(ENCT_VIDEO_HAVON_END,    pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel );
    WRITE_MPEG_REG(ENCT_VIDEO_VAVON_BLINE,  pConf->lcd_timing.video_on_line);
    WRITE_MPEG_REG(ENCT_VIDEO_VAVON_ELINE,  pConf->lcd_basic.v_active + 3  + pConf->lcd_timing.video_on_line);

    WRITE_MPEG_REG(ENCT_VIDEO_HSO_BEGIN,    15);
    WRITE_MPEG_REG(ENCT_VIDEO_HSO_END,      31);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_BEGIN,    15);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_END,      31);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_BLINE,    0);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_ELINE,    2);

    // enable enct
    WRITE_MPEG_REG(ENCT_VIDEO_EN,           1);
}

static void venc_set_lvds(Lcd_Config_t *pConf)
{
    DPRINT("%s.\n",__FUNCTION__);

    WRITE_MPEG_REG(ENCL_VIDEO_EN,           0);
    
    WRITE_MPEG_REG(VPU_VIU_VENC_MUX_CTRL,
    (0<<0) |    // viu1 select encl
    (0<<2)      // viu2 select encl
    );

    WRITE_MPEG_REG(ENCL_VIDEO_MODE,         0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    WRITE_MPEG_REG(ENCL_VIDEO_MODE_ADV,     0x0008); // Sampling rate: 1
	
	// bypass filter
    WRITE_MPEG_REG(ENCL_VIDEO_FILT_CTRL	,0x1000);
	
    WRITE_MPEG_REG(ENCL_VIDEO_MAX_PXCNT,	pConf->lcd_basic.h_period - 1);
    WRITE_MPEG_REG(ENCL_VIDEO_MAX_LNCNT,	pConf->lcd_basic.v_period - 1);

    WRITE_MPEG_REG(ENCL_VIDEO_HAVON_BEGIN,	pConf->lcd_timing.video_on_pixel);
    WRITE_MPEG_REG(ENCL_VIDEO_HAVON_END,	pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel);
    WRITE_MPEG_REG(ENCL_VIDEO_VAVON_BLINE,	pConf->lcd_timing.video_on_line);
    WRITE_MPEG_REG(ENCL_VIDEO_VAVON_ELINE,	pConf->lcd_basic.v_active + 1  + pConf->lcd_timing.video_on_line);

    WRITE_MPEG_REG(ENCL_VIDEO_HSO_BEGIN,	pConf->lcd_timing.sth1_hs_addr);//10);
    WRITE_MPEG_REG(ENCL_VIDEO_HSO_END,		pConf->lcd_timing.sth1_he_addr);//20);
    WRITE_MPEG_REG(ENCL_VIDEO_VSO_BEGIN,	pConf->lcd_timing.stv1_hs_addr);//10);
    WRITE_MPEG_REG(ENCL_VIDEO_VSO_END,		pConf->lcd_timing.stv1_he_addr);//20);
    WRITE_MPEG_REG(ENCL_VIDEO_VSO_BLINE,	pConf->lcd_timing.stv1_vs_addr);//2);
    WRITE_MPEG_REG(ENCL_VIDEO_VSO_ELINE,	pConf->lcd_timing.stv1_ve_addr);//4);

    WRITE_MPEG_REG( ENCL_VIDEO_RGBIN_CTRL, 	0);    

    // enable encl
    WRITE_MPEG_REG( ENCL_VIDEO_EN,           1);
}

static void venc_set_mlvds(Lcd_Config_t *pConf)
{
    DPRINT("%s\n", __FUNCTION__);

    WRITE_MPEG_REG(ENCL_VIDEO_EN,           0);

    WRITE_MPEG_REG(VPU_VIU_VENC_MUX_CTRL,
       (0<<0) |    // viu1 select encl
       (0<<2)      // viu2 select encl
       );

	int ext_pixel = pConf->lvds_mlvds_config.mlvds_config->test_dual_gate ? pConf->lvds_mlvds_config.mlvds_config->total_line_clk : 0;
	int active_h_start = pConf->lcd_timing.video_on_pixel;
	int active_v_start = pConf->lcd_timing.video_on_line;
	int width = pConf->lcd_basic.h_active;
	int height = pConf->lcd_basic.v_active;
	int max_height = pConf->lcd_basic.v_period;

	WRITE_MPEG_REG(ENCL_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	WRITE_MPEG_REG(ENCL_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
	
	// bypass filter
 	WRITE_MPEG_REG(	ENCL_VIDEO_FILT_CTRL,		0x1000);
	
	WRITE_MPEG_REG(ENCL_VIDEO_YFP1_HTIME,       active_h_start);
	WRITE_MPEG_REG(ENCL_VIDEO_YFP2_HTIME,       active_h_start + width);

	WRITE_MPEG_REG(ENCL_VIDEO_MAX_PXCNT,        pConf->lvds_mlvds_config.mlvds_config->total_line_clk - 1 + ext_pixel);
	WRITE_MPEG_REG(ENCL_VIDEO_MAX_LNCNT,        max_height - 1);

	WRITE_MPEG_REG(ENCL_VIDEO_HAVON_BEGIN,      active_h_start);
	WRITE_MPEG_REG(ENCL_VIDEO_HAVON_END,        active_h_start + width - 1);  // for dual_gate mode still read 1408 pixel at first half of line
	WRITE_MPEG_REG(ENCL_VIDEO_VAVON_BLINE,      active_v_start);
	WRITE_MPEG_REG(ENCL_VIDEO_VAVON_ELINE,      active_v_start + height -1);  //15+768-1);

	WRITE_MPEG_REG(ENCL_VIDEO_HSO_BEGIN,        24);
	WRITE_MPEG_REG(ENCL_VIDEO_HSO_END,          1420 + ext_pixel);
	WRITE_MPEG_REG(ENCL_VIDEO_VSO_BEGIN,        1400 + ext_pixel);
	WRITE_MPEG_REG(ENCL_VIDEO_VSO_END,          1410 + ext_pixel);
	WRITE_MPEG_REG(ENCL_VIDEO_VSO_BLINE,        1);
	WRITE_MPEG_REG(ENCL_VIDEO_VSO_ELINE,        3);

	WRITE_MPEG_REG( ENCL_VIDEO_RGBIN_CTRL, 	0);

	// enable encl
    WRITE_MPEG_REG( ENCL_VIDEO_EN,           1);
}

static void set_control_lvds(Lcd_Config_t *pConf)
{
    DPRINT("%s.\n",__FUNCTION__);

	int lvds_repack, pn_swap, bit_num;
    unsigned long data32;
	
	WRITE_MPEG_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1);	//disable lvds fifo
	
    data32 = (0x00 << LVDS_blank_data_r) |
             (0x00 << LVDS_blank_data_g) |
             (0x00 << LVDS_blank_data_b) ;
    WRITE_MPEG_REG(LVDS_BLANK_DATA_HI,  (data32 >> 16));
    WRITE_MPEG_REG(LVDS_BLANK_DATA_LO, (data32 & 0xffff));

    WRITE_MPEG_REG( LVDS_PHY_CNTL0, 0xffff );
    WRITE_MPEG_REG( LVDS_PHY_CNTL1, 0xff00 );

    //WRITE_MPEG_REG(ENCL_VIDEO_EN,           1);
	lvds_repack = (pConf->lvds_mlvds_config.lvds_config->lvds_repack) & 0x1;
	pn_swap = (pConf->lvds_mlvds_config.lvds_config->pn_swap) & 0x1;
	switch(pConf->lcd_basic.lcd_bits)
	{
		case 10:
			bit_num=0;
			break;
		case 8:
			bit_num=1;
			break;
		case 6:
			bit_num=2;
			break;
		case 4:
			bit_num=3;
			break;
		default:
			bit_num=1;
			break;
	}
	
	WRITE_MPEG_REG(MLVDS_CONTROL,  (READ_MPEG_REG(MLVDS_CONTROL) & ~(1 << 0)));  //disable mlvds	
	
    WRITE_MPEG_REG(LVDS_PACK_CNTL_ADDR,
					( lvds_repack<<0 ) | // repack
					( 0<<2 ) | // odd_even
					( 0<<3 ) | // reserve
					( 0<<4 ) | // lsb first
					( pn_swap<<5 ) | // pn swap
					( 0<<6 ) | // dual port
					( 0<<7 ) | // use tcon control
					( bit_num<<8 ) | // 0:10bits, 1:8bits, 2:6bits, 3:4bits.
					( 0<<10 ) | //r_select  //0:R, 1:G, 2:B, 3:0
					( 1<<12 ) | //g_select  //0:R, 1:G, 2:B, 3:0
					( 2<<14 ));  //b_select  //0:R, 1:G, 2:B, 3:0; 

    WRITE_MPEG_REG( LVDS_GEN_CNTL, (READ_MPEG_REG(LVDS_GEN_CNTL) | (1 << 0)) );  //fifo enable 
    //WRITE_MPEG_REG( LVDS_GEN_CNTL, (READ_MPEG_REG(LVDS_GEN_CNTL) | (1 << 3))); // enable fifo
	
	printf("lvds fifo clk = %d\n", (int)clk_util_clk_msr(LVDS_FIFO_CLK));
}

static void set_control_mlvds(Lcd_Config_t *pConf)
{
	DPRINT("%s\n", __FUNCTION__);
	
	int test_bit_num = pConf->lcd_basic.lcd_bits;
    int test_pair_num = pConf->lvds_mlvds_config.mlvds_config->test_pair_num;
    int test_dual_gate = pConf->lvds_mlvds_config.mlvds_config->test_dual_gate;
    int scan_function = pConf->lvds_mlvds_config.mlvds_config->scan_function;     //0:U->D,L->R  //1:D->U,R->L
    int mlvds_insert_start;
    unsigned int reset_offset;
    unsigned int reset_length;

    unsigned long data32;
    
    mlvds_insert_start = test_dual_gate ?
                           ((test_bit_num == 8) ? ((test_pair_num == 6) ? 0x9f : 0xa9) :
                                                  ((test_pair_num == 6) ? pConf->lvds_mlvds_config.mlvds_config->mlvds_insert_start : 0xa7)
                           ) :
                           (
                             (test_pair_num == 6) ? ((test_bit_num == 8) ? 0xa9 : 0xa7) :
                                                    ((test_bit_num == 8) ? 0xae : 0xad)
                           );

    // Enable the LVDS PHY (power down bits)
    WRITE_MPEG_REG(LVDS_PHY_CNTL1,READ_MPEG_REG(LVDS_PHY_CNTL1) | (0x7F << 8) );

    data32 = (0x00 << LVDS_blank_data_r) |
             (0x00 << LVDS_blank_data_g) |
             (0x00 << LVDS_blank_data_b) ;
    WRITE_MPEG_REG(LVDS_BLANK_DATA_HI,  (data32 >> 16));
    WRITE_MPEG_REG(LVDS_BLANK_DATA_LO, (data32 & 0xffff));

    data32 = 0x7fffffff; //  '0'x1 + '1'x32 + '0'x2
    WRITE_MPEG_REG(MLVDS_RESET_PATTERN_HI,  (data32 >> 16));
    WRITE_MPEG_REG(MLVDS_RESET_PATTERN_LO, (data32 & 0xffff));
    data32 = 0x8000; // '0'x1 + '1'x32 + '0'x2
    WRITE_MPEG_REG(MLVDS_RESET_PATTERN_EXT,  (data32 & 0xffff));

    reset_length = 1+32+2;
    reset_offset = test_bit_num - (reset_length%test_bit_num);

    data32 = (reset_offset << mLVDS_reset_offset) |
             (reset_length << mLVDS_reset_length) |
             ((test_pair_num == 6) << mLVDS_data_write_toggle) |
             ((test_pair_num != 6) << mLVDS_data_write_ini) |
             ((test_pair_num == 6) << mLVDS_data_latch_1_toggle) |
             (0 << mLVDS_data_latch_1_ini) |
             ((test_pair_num == 6) << mLVDS_data_latch_0_toggle) |
             (1 << mLVDS_data_latch_0_ini) |
             ((test_pair_num == 6) << mLVDS_reset_1_select) |
             (mlvds_insert_start << mLVDS_reset_start);
    WRITE_MPEG_REG(MLVDS_CONFIG_HI,  (data32 >> 16));
    WRITE_MPEG_REG(MLVDS_CONFIG_LO, (data32 & 0xffff));

    data32 = (1 << mLVDS_double_pattern) |  //POL double pattern
			 (0x3f << mLVDS_ins_reset) |
             (test_dual_gate << mLVDS_dual_gate) |
             ((test_bit_num == 8) << mLVDS_bit_num) |
             ((test_pair_num == 6) << mLVDS_pair_num) |
             (0 << mLVDS_msb_first) |
             (0 << mLVDS_PORT_SWAP) |
             ((scan_function==1 ? 1:0) << mLVDS_MLSB_SWAP) |
             (0 << mLVDS_PN_SWAP) |
             (1 << mLVDS_en);
    WRITE_MPEG_REG(MLVDS_CONTROL,  (data32 & 0xffff));

    WRITE_MPEG_REG(LVDS_PACK_CNTL_ADDR,
                   ( 0 ) | // repack
                   ( 0<<2 ) | // odd_even
                   ( 0<<3 ) | // reserve
                   ( 0<<4 ) | // lsb first
                   ( 0<<5 ) | // pn swap
                   ( 0<<6 ) | // dual port
                   ( 0<<7 ) | // use tcon control
                   ( 1<<8 ) | // 0:10bits, 1:8bits, 2:6bits, 3:4bits.
                   ( (scan_function==1 ? 2:0)<<10 ) |  //r_select // 0:R, 1:G, 2:B, 3:0
                   ( 1<<12 ) |                        //g_select
                   ( (scan_function==1 ? 0:2)<<14 ));  //b_select

    WRITE_MPEG_REG(L_POL_CNTL_ADDR,  (1 << LCD_DCLK_SEL) |
       //(0x1 << LCD_HS_POL) |
       (0x1 << LCD_VS_POL)
    );

    //WRITE_MPEG_REG( LVDS_GEN_CNTL, (READ_MPEG_REG(LVDS_GEN_CNTL) | (1 << 3))); // enable fifo
}

static void init_lvds_phy(Lcd_Config_t *pConf)
{
    DPRINT("%s.\n",__FUNCTION__);

    unsigned tmp_add_data;    

    WRITE_MPEG_REG(LVDS_PHY_CNTL3, 0xee1);  //0xee0
	WRITE_MPEG_REG(LVDS_PHY_CNTL4 ,0);

	if (dts_ready) {
		WRITE_MPEG_REG(LVDS_PHY_CNTL5, pConf->lvds_mlvds_config.lvds_phy_ctrl);
	}
	else {
		tmp_add_data  = 0;
		tmp_add_data |= (pConf->lvds_mlvds_config.lvds_phy_control->lvds_prem_ctl & 0xf) << 0; //LVDS_PREM_CTL<3:0>=<1111>
		tmp_add_data |= (pConf->lvds_mlvds_config.lvds_phy_control->lvds_swing_ctl & 0xf) << 4; //LVDS_SWING_CTL<3:0>=<0011>    
		tmp_add_data |= (pConf->lvds_mlvds_config.lvds_phy_control->lvds_vcm_ctl & 0x7) << 8; //LVDS_VCM_CTL<2:0>=<001>
		tmp_add_data |= (pConf->lvds_mlvds_config.lvds_phy_control->lvds_ref_ctl & 0x1f) << 11; //LVDS_REFCTL<4:0>=<01111> 
		WRITE_MPEG_REG(LVDS_PHY_CNTL5, tmp_add_data);
	}

    WRITE_MPEG_REG(LVDS_PHY_CNTL0,0x1f);  //0xfff
    WRITE_MPEG_REG(LVDS_PHY_CNTL1,0xffff);

    WRITE_MPEG_REG(LVDS_PHY_CNTL6,0xcccc);
    WRITE_MPEG_REG(LVDS_PHY_CNTL7,0xcccc);
    WRITE_MPEG_REG(LVDS_PHY_CNTL8,0xcccc);

    //WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) & ~(0x7f<<0));  //disable LVDS phy port. wait for power on sequence.
}

static unsigned error_abs(unsigned num1, unsigned num2)
{
	if (num1 >= num2)
		return num1 - num2;
	else
		return num2 - num1;
}

static void generate_clk_parameter(Lcd_Config_t *pConf)
{
	unsigned pll_n = 0;
	unsigned pll_m = 0;
	unsigned pll_od = 0;
	unsigned vid_div_pre = 0;
	unsigned crt_xd = 0;

	unsigned m, n, od, div_pre, div_post, xd;
	unsigned f_ref, pll_vco, od_index, fout_pll, div_pre_out, div_post_out, final_freq;
	unsigned crt_xd_max;
	unsigned min_error = MAX_ERROR;
	unsigned error = MAX_ERROR;
	
	unsigned fin = 24000;	//kHz
	unsigned fout = (pConf->lcd_timing.lcd_clk) / 1000;  //kHz
	
	unsigned clk_num = 0;	
	for (n = PLL_N_MIN; n <= PLL_N_MAX; n++) {
		f_ref = fin / n;
		if ((f_ref >= PLL_FREF_MIN) && (f_ref <= PLL_FREF_MAX))	{
			for (m = PLL_M_MIN; m <= PLL_M_MAX; m++) {
				pll_vco = f_ref * m;
				if ((pll_vco >= PLL_VCO_MIN) && (pll_vco <= PLL_VCO_MAX)) {
					for (od_index = PLL_OD_MIN; od_index <= PLL_OD_MAX; od_index++) {
						od_index = PLL_OD_MAX - od_index;
						od = (od_index == 0 ? 1 : 2);
						fout_pll = pll_vco / od;
						if (fout_pll <= DIV_PRE_MAX_CLK_IN)	{
							for (div_pre = DIV_PRE_MIN; div_pre <= DIV_PRE_MAX; div_pre++) {
								div_pre_out = fout_pll / div_pre;
								if (div_pre_out <= DIV_POST_MAX_CLK_IN) {
									switch (pConf->lcd_basic.lcd_type) {
										case LCD_DIGITAL_TTL:
											div_post = 1;
											crt_xd_max = CRT_VID_DIV_MAX;
											break;
										case LCD_DIGITAL_LVDS:
											div_post = 7;
											crt_xd_max = 1;
											break;
										case LCD_DIGITAL_MINILVDS:
											div_post = 6;
											crt_xd_max = 1;
											break;
										default:
											div_post = 1;
											crt_xd_max = 1;
											break;
									}
									div_post_out = div_pre_out / div_post;
									if (div_post_out <= CRT_VID_MAX_CLK_IN) {
										for (xd = 1; xd <= crt_xd_max; xd++) {
											final_freq = div_post_out / xd;
											error = error_abs(final_freq, fout);
											if (error < min_error) {
												min_error = error;
												pll_m = m;
												pll_n = n;
												pll_od = od_index;
												vid_div_pre = div_pre - 1;
												crt_xd = xd;
												clk_num++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (clk_num > 0) {
		pConf->lcd_timing.pll_ctrl = (pll_od << PLL_CTRL_OD) | (pll_n << PLL_CTRL_N) | (pll_m << PLL_CTRL_M);
		pConf->lcd_timing.div_ctrl = 0x18803 | (vid_div_pre << DIV_CTRL_DIV_PRE);
		pConf->lcd_timing.clk_ctrl = (pConf->lcd_timing.clk_ctrl & ~(0xf << CLK_CTRL_XD)) | (crt_xd << CLK_CTRL_XD);
	}
	else {
		pConf->lcd_timing.pll_ctrl = 0x10220;
		pConf->lcd_timing.div_ctrl = 0x18803;
		pConf->lcd_timing.clk_ctrl = (pConf->lcd_timing.clk_ctrl & ~(0xf << CLK_CTRL_XD)) | (7 << CLK_CTRL_XD);
		printf("Out of clock range, reset to default setting!\n");
	}
}

static void lcd_sync_duration(Lcd_Config_t *pConf)
{
	unsigned m, n, od, pre_div, xd, post_div;
	unsigned h_period, v_period, sync_duration;	
	unsigned lcd_clk;

	m = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_M) & 0x1ff;
	n = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_N) & 0x1f;
	od = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_OD) & 0x3;
	pre_div = ((pConf->lcd_timing.div_ctrl) >> 4) & 0x7;
	h_period = pConf->lcd_basic.h_period;
	v_period = pConf->lcd_basic.v_period;
	
	od = (od == 0) ? 1:((od == 1) ? 2:4);
	switch(pConf->lcd_basic.lcd_type) {
		case LCD_DIGITAL_TTL:
			xd = ((pConf->lcd_timing.clk_ctrl) >> 0) & 0xf;
			post_div = 1;
			break;
		case LCD_DIGITAL_LVDS:
			xd = 1;
			post_div = 7;
			break;
		case LCD_DIGITAL_MINILVDS:
			xd = 1;
			post_div = 6;
			break;
		default:
			xd = ((pConf->lcd_timing.clk_ctrl) >> 0) & 0xf;
			post_div = 1;
			break;
	}
	
	lcd_clk = ((m * 24 * 1000) / (n * od * (pre_div + 1) * xd * post_div)) * 1000;
	pConf->lcd_timing.lcd_clk = lcd_clk;
	sync_duration = ((lcd_clk / h_period) * 100) / v_period;
	sync_duration = (sync_duration + 5) / 10;
	
	pConf->lcd_timing.sync_duration_num = sync_duration;
	pConf->lcd_timing.sync_duration_den = 10;
	printf("lcd_clk=%u.%uMHz, frame_rate=%u.%uHz.\n\n", (lcd_clk / 1000000), ((lcd_clk / 1000) % 1000), (sync_duration / pConf->lcd_timing.sync_duration_den), ((sync_duration * 10 / pConf->lcd_timing.sync_duration_den) % 10));
}

static void lcd_tcon_config(Lcd_Config_t *pConf)
{
	unsigned short hs_pol, vs_pol;
	unsigned short hstart, hend, vstart, vend;
	unsigned short h_delay = 0;
	unsigned short h_offset = 0, v_offset = 0;
	
	hs_pol = (pConf->lcd_timing.pol_cntl_addr >> LCD_HS_POL) & 1;
	vs_pol = (pConf->lcd_timing.pol_cntl_addr >> LCD_VS_POL) & 1;
	
	switch (pConf->lcd_basic.lcd_type) {
		case LCD_DIGITAL_TTL:
			h_delay = TTL_DELAY;
			break;
		case LCD_DIGITAL_LVDS:
			h_delay = LVDS_DELAY;
			break;
		case LCD_DIGITAL_MINILVDS:
			h_delay = MLVDS_DELAY;
			break;
		
		default:
			h_delay = 0;
			break;
	}
	
	h_offset = (pConf->lcd_timing.h_offset & 0xffff);
	v_offset = (pConf->lcd_timing.v_offset & 0xffff);
	if ((pConf->lcd_timing.h_offset >> 31) & 1)
		pConf->lcd_timing.de_hstart = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period + h_delay + h_offset) % pConf->lcd_basic.h_period;
	else
		pConf->lcd_timing.de_hstart = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period + h_delay - h_offset) % pConf->lcd_basic.h_period;
	if ((pConf->lcd_timing.v_offset >> 31) & 1)
		pConf->lcd_timing.de_vstart = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period + v_offset) % pConf->lcd_basic.v_period;
	else
		pConf->lcd_timing.de_vstart = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period - v_offset) % pConf->lcd_basic.v_period;
	
	if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_TTL) {
		hstart = (pConf->lcd_timing.de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp) % pConf->lcd_basic.h_period;
		hend = (pConf->lcd_timing.de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp + pConf->lcd_timing.hsync_width) % pConf->lcd_basic.h_period;
		if (hs_pol) {
			pConf->lcd_timing.sth1_hs_addr = hstart;
			pConf->lcd_timing.sth1_he_addr = hend;
		}
		else {
			pConf->lcd_timing.sth1_he_addr = hstart;
			pConf->lcd_timing.sth1_hs_addr = hend;
		}
		pConf->lcd_timing.sth1_vs_addr = 0;
		pConf->lcd_timing.sth1_ve_addr = pConf->lcd_basic.v_period - 1;
		
		vstart = (pConf->lcd_timing.de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp) % pConf->lcd_basic.v_period;
		vend = (pConf->lcd_timing.de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp + pConf->lcd_timing.vsync_width - 1) % pConf->lcd_basic.v_period;
		if (vs_pol) {
			pConf->lcd_timing.stv1_vs_addr = vstart;
			pConf->lcd_timing.stv1_ve_addr = vend;
		}
		else {
			pConf->lcd_timing.stv1_ve_addr = vstart;
			pConf->lcd_timing.stv1_vs_addr = vend;
		}
		pConf->lcd_timing.stv1_hs_addr = 0;
		pConf->lcd_timing.stv1_he_addr = pConf->lcd_basic.h_period - 1;
		
		pConf->lcd_timing.oeh_hs_addr = pConf->lcd_timing.de_hstart;
		pConf->lcd_timing.oeh_he_addr = pConf->lcd_timing.de_hstart + pConf->lcd_basic.h_active;
		pConf->lcd_timing.oeh_vs_addr = pConf->lcd_timing.de_vstart;
		pConf->lcd_timing.oeh_ve_addr = pConf->lcd_timing.de_vstart + pConf->lcd_basic.v_active - 1;
	}
	else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_LVDS) {
		hstart = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp) % pConf->lcd_basic.h_period;
		hend = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp + pConf->lcd_timing.hsync_width) % pConf->lcd_basic.h_period;
		if (hs_pol) {
			pConf->lcd_timing.sth1_he_addr = hstart;
			pConf->lcd_timing.sth1_hs_addr = hend;
		}
		else {
			pConf->lcd_timing.sth1_hs_addr = hstart;
			pConf->lcd_timing.sth1_he_addr = hend;
		}
		pConf->lcd_timing.sth1_vs_addr = 0;
		pConf->lcd_timing.sth1_ve_addr = pConf->lcd_basic.v_period - 1;
		
		vstart = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp) % pConf->lcd_basic.v_period;
		vend = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp + pConf->lcd_timing.vsync_width - 1) % pConf->lcd_basic.v_period;
		if (vs_pol) {
			pConf->lcd_timing.stv1_ve_addr = vstart;
			pConf->lcd_timing.stv1_vs_addr = vend;
		}
		else {
			pConf->lcd_timing.stv1_vs_addr = vstart;
			pConf->lcd_timing.stv1_ve_addr = vend;
		}
		pConf->lcd_timing.stv1_hs_addr = 0;
		pConf->lcd_timing.stv1_he_addr = pConf->lcd_basic.h_period - 1;
		
		pConf->lcd_timing.oeh_hs_addr = pConf->lcd_timing.video_on_pixel;
		pConf->lcd_timing.oeh_he_addr = pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_active;
		pConf->lcd_timing.oeh_vs_addr = pConf->lcd_timing.video_on_line;
		pConf->lcd_timing.oeh_ve_addr = pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_active - 1;
	}
	else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_MINILVDS) {
		//to do
	}
	DPRINT("sth1_hs_addr=%d, sth1_he_addr=%d, sth1_vs_addr=%d, sth1_ve_addr=%d\n", pConf->lcd_timing.sth1_hs_addr, pConf->lcd_timing.sth1_he_addr, pConf->lcd_timing.sth1_vs_addr, pConf->lcd_timing.sth1_ve_addr);
	DPRINT("stv1_hs_addr=%d, stv1_he_addr=%d, stv1_vs_addr=%d, stv1_ve_addr=%d\n", pConf->lcd_timing.stv1_hs_addr, pConf->lcd_timing.stv1_he_addr, pConf->lcd_timing.stv1_vs_addr, pConf->lcd_timing.stv1_ve_addr);
	DPRINT("oeh_hs_addr=%d, oeh_he_addr=%d, oeh_vs_addr=%d, oeh_ve_addr=%d\n", pConf->lcd_timing.oeh_hs_addr, pConf->lcd_timing.oeh_he_addr, pConf->lcd_timing.oeh_vs_addr, pConf->lcd_timing.oeh_ve_addr);
}

static void _lcd_config_init(Lcd_Config_t *pConf)
{	
	if (pConf->lcd_timing.clk_ctrl & (1 << CLK_CTRL_AUTO)) {
		printf("\nAuto generate clock parameters.\n");
		generate_clk_parameter(pConf);
	}
	else {
		printf("\nCustome clock parameters.\n");
	}
	printf("pll_ctrl=0x%x, div_ctrl=0x%x, clk_ctrl=0x%x.\n", pConf->lcd_timing.pll_ctrl, pConf->lcd_timing.div_ctrl, pConf->lcd_timing.clk_ctrl);
	lcd_sync_duration(pConf);
	lcd_tcon_config(pConf);
	if (dts_ready) {		
#ifdef CONFIG_OF_LIBFDT
		set_lcd_backlight_level(BL_LEVEL_DEFAULT);
#endif
	}
	else {
		pConf->lcd_timing.hvsync_valid = 1;
		pConf->lcd_timing.de_valid = 1;
		pConf->lcd_effect.dith_user = 0;
		pConf->lcd_effect.gamma_r_coeff = 100;
		pConf->lcd_effect.gamma_g_coeff = 100;
		pConf->lcd_effect.gamma_b_coeff = 100;
		pConf->lcd_effect.vadj_brightness = 0x0;
		pConf->lcd_effect.vadj_contrast = 0x80;
		pConf->lcd_effect.vadj_saturation = 0x100;
	}
}

static inline void _init_display_driver(Lcd_Config_t *pConf)
{
	int lcd_type;

	lcd_type = pConf->lcd_basic.lcd_type;
	printf("Init LCD mode: %s %ux%u@%u.%uHz, %ubit\n\n", lcd_type_table[lcd_type], pConf->lcd_basic.h_active, pConf->lcd_basic.v_active, (pConf->lcd_timing.sync_duration_num / 10), (pConf->lcd_timing.sync_duration_num % 10), pConf->lcd_basic.lcd_bits);

	switch(lcd_type) {
		case LCD_DIGITAL_TTL:
			set_pll_ttl(pConf);
			venc_set_ttl(pConf);
			set_tcon_ttl(pConf);    
			break;
		case LCD_DIGITAL_LVDS:
			set_pll_lvds(pConf);
			venc_set_lvds(pConf);
			set_control_lvds(pConf);
			init_lvds_phy(pConf);
			set_tcon_lvds(pConf);
			break;
		case LCD_DIGITAL_MINILVDS:
			set_pll_mlvds(pConf);
			venc_set_mlvds(pConf);
			set_control_mlvds(pConf);
			init_lvds_phy(pConf);
			set_tcon_mlvds(pConf);
			break;
		default:
			printf("Invalid LCD type.\n");
			break;
	}	
	set_video_adjust(pConf);
	printf("%s finished.\n", __FUNCTION__);
}

static inline void _disable_display_driver(Lcd_Config_t *pConf)
{	
    int pll_sel, vclk_sel;	    
    
    pll_sel = ((pConf->lcd_timing.clk_ctrl) >>12) & 0x1;
    vclk_sel = ((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;
	
	WRITE_MPEG_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1);	//disable lvds fifo
	WRITE_MPEG_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 11, 1);	//close lvds phy clk gate: 0x104c[11]
	
	WRITE_MPEG_REG(ENCT_VIDEO_EN, 0);	//disable enct
	WRITE_MPEG_REG(ENCL_VIDEO_EN, 0);	//disable encl
	
	if (vclk_sel)
		WRITE_MPEG_REG_BITS(HHI_VIID_CLK_CNTL, 0, 0, 5);		//close vclk2 gate: 0x104b[4:0]
	else
		WRITE_MPEG_REG_BITS(HHI_VID_CLK_CNTL, 0, 0, 5);		//close vclk1 gate: 0x105f[4:0]
	
	if (pll_sel){	
		WRITE_MPEG_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 16, 1);	//close vid2_pll gate: 0x104c[16]
		WRITE_MPEG_REG_BITS(HHI_VIID_PLL_CNTL, 1, 30, 1);		//power down vid2_pll: 0x1047[30]
	}
	else{
		WRITE_MPEG_REG_BITS(HHI_VID_DIVIDER_CNTL, 0, 16, 1);	//close vid1_pll gate: 0x1066[16]
		WRITE_MPEG_REG_BITS(HHI_VID_PLL_CNTL, 1, 30, 1);		//power down vid1_pll: 0x105c[30]
	}
	printf("disable lcd display driver.\n");
}

static inline void _enable_vsync_interrupt(void)
{
    if ((READ_MPEG_REG(ENCT_VIDEO_EN) & 1) || (READ_MPEG_REG(ENCL_VIDEO_EN) & 1)) {
        WRITE_MPEG_REG(VENC_INTCTRL, 0x200);
    }
    else{
        WRITE_MPEG_REG(VENC_INTCTRL, 0x2);
    }
}

//for lcd_function_call by other module, compatible dts
static void _enable_backlight(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		lcd_backlight_power_ctrl(ON);
#endif
	}
}
static void _disable_backlight(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		lcd_backlight_power_ctrl(OFF);
#endif
	}	
}
static void _set_backlight_level(unsigned level)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		set_lcd_backlight_level(level);
#endif
	}
}
static unsigned _get_backlight_level(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		return get_lcd_backlight_level();
#endif
	}
}
static void _lcd_enable(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		lcd_power_ctrl(OFF);
		lcd_probe();
#endif
	}
}
static void _lcd_disable(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		lcd_backlight_power_ctrl(OFF);
		lcd_power_ctrl(OFF);
		lcd_remove();
#endif
	}
}

static void _lcd_test(unsigned num)
{
	unsigned venc_video_mode, venc_test_base;
	
	if (pDev->conf.lcd_basic.lcd_type == LCD_DIGITAL_TTL) {
		venc_video_mode = ENCT_VIDEO_MODE_ADV;
		venc_test_base = ENCT_TST_EN;
	}
	else {
		venc_video_mode = ENCL_VIDEO_MODE_ADV;
		venc_test_base = ENCL_TST_EN;
	}
	
	switch (num) {
		case 0:
			WRITE_MPEG_REG(venc_video_mode, 0x8);
			printf("disable bist pattern\n");
			printf("video dev test 1/2/3: show different test pattern\n");
			break;
		case 1:
			WRITE_MPEG_REG(venc_video_mode, 0);
			WRITE_MPEG_REG(venc_test_base+1, 1);
			WRITE_MPEG_REG(venc_test_base+5, pDev->conf.lcd_basic.h_active / 8);
			WRITE_MPEG_REG(venc_test_base+6, pDev->conf.lcd_basic.h_active / 8);
			WRITE_MPEG_REG(venc_test_base, 1);
			printf("show test pattern 1\n");
			printf("video dev test 0: disable test pattern\n");
			break;
		case 2:
			WRITE_MPEG_REG(venc_video_mode, 0);
			WRITE_MPEG_REG(venc_test_base+1, 2);
			WRITE_MPEG_REG(venc_test_base, 1);
			printf("show test pattern 2\n");
			printf("video dev test 0: disable test pattern\n");
			break;
		case 3:
			WRITE_MPEG_REG(venc_video_mode, 0);
			WRITE_MPEG_REG(venc_test_base+1, 3);
			WRITE_MPEG_REG(venc_test_base, 1);
			printf("show test pattern 3\n");
			printf("video dev test 0: disable test pattern\n");
			break;
		default:
			printf("un-support pattern num\n");
			printf("video dev test 1/2/3: show different test pattern\n");
			printf("video dev test 0: disable test pattern\n");
			break;
	}
}

static update_panel_operation(void)
{
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		panel_oper.enable = _lcd_enable;
		panel_oper.disable = _lcd_disable;
		panel_oper.bl_on = _enable_backlight;
		panel_oper.bl_off = _disable_backlight;
		panel_oper.set_bl_level = _set_backlight_level;
		panel_oper.get_bl_level = _get_backlight_level;
#endif
	}
	panel_oper.test = _lcd_test;
}
//****************************************

static void _lcd_module_enable(void)
{
    BUG_ON(pDev==NULL);    
	_init_display_driver(&pDev->conf);
	if (dts_ready) {
#ifdef CONFIG_OF_LIBFDT
		lcd_power_ctrl(ON);
#endif
	}
	else {
		panel_oper.power_on();
	}
    //_enable_backlight();	//disable backlight at pannel init
    _enable_vsync_interrupt();
}

//static const vinfo_t *lcd_get_current_info(void)
//{
//	return &pDev->lcd_info;
//}

static int lcd_set_current_vmode(vmode_t mode)	//set display width
{
    if (mode != VMODE_LCD)
        return -1;
    WRITE_MPEG_REG(VPP_POSTBLEND_H_SIZE, pDev->lcd_info.width);
    _lcd_module_enable();
    if (VMODE_INIT_NULL == pDev->lcd_info.mode)
        pDev->lcd_info.mode = VMODE_LCD;
    else
        _enable_backlight();
	return 0;
}

/*static vmode_t lcd_validate_vmode(char *mode)
{
	if ((strncmp(mode, PANEL_NAME, strlen(PANEL_NAME))) == 0)
		return VMODE_LCD;

	return VMODE_MAX;
}
static int lcd_vmode_is_supported(vmode_t mode)
{
	mode&=VMODE_MODE_BIT_MASK;
	if(mode == VMODE_LCD)
	    return 1;
	return 0;
}
static int lcd_module_disable(vmode_t cur_vmod)
{
    BUG_ON(pDev==NULL);
    _disable_backlight();
    //pDev->conf.power_off?pDev->conf.power_off():0;
    panel_oper.power_off();
    return 0;
}*/

static void _init_vout(lcd_dev_t *pDev)
{
    pDev->lcd_info.name = PANEL_NAME;
    pDev->lcd_info.mode = VMODE_INIT_NULL;
    pDev->lcd_info.width = pDev->conf.lcd_basic.h_active;
    pDev->lcd_info.height = pDev->conf.lcd_basic.v_active;
    pDev->lcd_info.field_height = pDev->conf.lcd_basic.v_active;
    pDev->lcd_info.aspect_ratio_num = pDev->conf.lcd_basic.screen_ratio_width;
    pDev->lcd_info.aspect_ratio_den = pDev->conf.lcd_basic.screen_ratio_height;
    pDev->lcd_info.sync_duration_num = pDev->conf.lcd_timing.sync_duration_num;
    pDev->lcd_info.sync_duration_den = pDev->conf.lcd_timing.sync_duration_den;
}

static void _lcd_init(Lcd_Config_t *pConf)
{
    _lcd_config_init(pConf);
	_init_vout(pDev);
    	//_lcd_module_enable();		//remove repeatedly lcd_module_enable
	lcd_set_current_vmode(VMODE_LCD);
}

#ifdef CONFIG_OF_LIBFDT
static int amlogic_gpio_name_map_num(const char *name)
{
	int i;
	
	for(i = 0; i <= GPIO_NULL; i++) {
		if(!strcmp(name, amlogic_gpio_type_table[i]))
			break;
	}
	if (i == GPIO_NULL) {
		printf("wrong gpio name %s, i=%d\n", name, i);
		i = -1;
	}
	return i;
}

static int amlogic_gpio_set(int gpio, int flag)
{
	int gpio_bank, gpio_bit;
	
	if ((gpio>=GPIOZ_0) && (gpio<=GPIOZ_12)) {	//GPIOZ_0~12
		gpio_bit = gpio - GPIOZ_0 + 16;
		gpio_bank = PREG_PAD_GPIO6_EN_N;
	}
	else if ((gpio>=GPIOE_0) && (gpio<=GPIOE_11)) {	//GPIOE_0~11
		gpio_bit = gpio - GPIOE_0;
		gpio_bank = PREG_PAD_GPIO6_EN_N;
	}
	else if ((gpio>=GPIOY_0) && (gpio<=GPIOY_15)) {	//GPIOY_0~15
		gpio_bit = gpio - GPIOY_0;
		gpio_bank = PREG_PAD_GPIO5_EN_N;
	}
	else if ((gpio>=GPIOX_0) && (gpio<=GPIOX_31)) {	//GPIOX_0~31
		gpio_bit = gpio - GPIOX_0;
		gpio_bank = PREG_PAD_GPIO4_EN_N;
	}
	else if ((gpio>=GPIOX_32) && (gpio<=GPIOX_35)) {	//GPIOX_32~35
		gpio_bit = gpio - GPIOX_32 + 20;
		gpio_bank = PREG_PAD_GPIO3_EN_N;
	}
	else if ((gpio>=BOOT_0) && (gpio<=BOOT_17)) {	//BOOT_0~17
		gpio_bit = gpio - BOOT_0;
		gpio_bank = PREG_PAD_GPIO3_EN_N;
	}
	else if ((gpio>=GPIOD_0) && (gpio<=GPIOD_9)) {	//GPIOD_0~9
		gpio_bit = gpio - GPIOD_0 + 16;
		gpio_bank = PREG_PAD_GPIO2_EN_N;
	}
	else if ((gpio>=GPIOC_0) && (gpio<=GPIOC_15)) {	//GPIOC_0~15
		gpio_bit = gpio - GPIOC_0;
		gpio_bank = PREG_PAD_GPIO2_EN_N;
	}
	else if ((gpio>=CARD_0) && (gpio<=CARD_8)) {	//CARD_0~8
		gpio_bit = gpio - CARD_0 + 23;
		gpio_bank = PREG_PAD_GPIO5_EN_N;
	}
	else if ((gpio>=GPIOB_0) && (gpio<=GPIOB_23)) {	//GPIOB_0~23
		gpio_bit = gpio - GPIOB_0;
		gpio_bank = PREG_PAD_GPIO1_EN_N;
	}
	else if ((gpio>=GPIOA_0) && (gpio<=GPIOA_27)) {	//GPIOA_0~27
		gpio_bit = gpio - GPIOA_0;
		gpio_bank = PREG_PAD_GPIO0_EN_N;
	}
	else if ((gpio>=GPIOAO_0) && (gpio<=GPIOAO_11)) {	//GPIOAO_0~11
		gpio_bit = gpio - GPIOAO_0;
		gpio_bank = GPIOAO_bank_bit0_11(bit);
		printf("don't support GPIOAO Port yet\n");
		return -2;
	}
	else {
		printf("Wrong GPIO Port number: %d\n", gpio);
		return -1;
	}
	
	if (flag == LCD_POWER_GPIO_OUTPUT_LOW) {
		WRITE_CBUS_REG_BITS(gpio_bank+1, 0, gpio_bit, 1);
		WRITE_CBUS_REG_BITS(gpio_bank, 0, gpio_bit, 1);
	}
	else if (flag == LCD_POWER_GPIO_OUTPUT_HIGH) {
		WRITE_CBUS_REG_BITS(gpio_bank+1, 1, gpio_bit, 1);
		WRITE_CBUS_REG_BITS(gpio_bank, 0, gpio_bit, 1);
	}
	else {
		WRITE_CBUS_REG_BITS(gpio_bank, 1, gpio_bit, 1);
	}
	return 0;
}

static int amlogic_pmu_gpio_name_map_num(const char *name)
{
	int index;
	
	for(index = 0; index <= LCD_POWER_PMU_GPIO_NULL; index++) {
		if(!strcmp(name, lcd_power_pmu_gpio_table[index]))
			break;
	}
	return index;
}

static inline int _get_lcd_model_timing(Lcd_Config_t *pConf)
{
	int ret=0;
	int nodeoffset;
	char* lcd_model;
	char* propdata;
	char propname[30];
	int i;
	
	nodeoffset = fdt_path_offset(dt_addr, "/lcd");
	if(nodeoffset < 0) {
		printf("lcd dts: not find /lcd node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}
	
	lcd_model = fdt_getprop(dt_addr, nodeoffset, "lcd_model_name", NULL);
	sprintf(propname, "/%s", lcd_model);
	nodeoffset = fdt_path_offset(dt_addr, propname);
	if(nodeoffset < 0) {
		printf("lcd dts: not find %s node %s.\n", propname, fdt_strerror(nodeoffset));
		return ret;
	}
	
	lcd_model = fdt_getprop(dt_addr, nodeoffset, "model_name", NULL);
	if (lcd_model == NULL) {
		printf("faild to get model_name\n");
		lcd_model = PANEL_MODEL_DEFAULT;
	}
	pConf->lcd_basic.model_name = lcd_model;
	printf("\nlcd dts: load model %s typical timing.\n", pConf->lcd_basic.model_name);

	propdata = fdt_getprop(dt_addr, nodeoffset, "resolution", NULL);
	if(propdata == NULL){
		printf("faild to get resolution\n");
	}
	else {
		pConf->lcd_basic.h_active = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_basic.v_active = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "period", NULL);
	if(propdata == NULL){
		printf("faild to get period\n");
	}
	else {
		pConf->lcd_basic.h_period = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_basic.v_period = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
	}
	DPRINT("h_active = %u, v_active =%u, h_period = %u, v_period = %u\n", pConf->lcd_basic.h_active, pConf->lcd_basic.v_active, pConf->lcd_basic.h_period, pConf->lcd_basic.v_period);
	propdata = fdt_getprop(dt_addr, nodeoffset, "active_area", NULL);
	if(propdata == NULL){
		printf("faild to get active_area\n");
	}
	else {
		pConf->lcd_basic.screen_actual_width = be32_to_cpup((u32*)propdata);
		pConf->lcd_basic.screen_actual_height = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_basic.screen_ratio_width = be32_to_cpup((u32*)propdata);
		pConf->lcd_basic.screen_ratio_height = be32_to_cpup((((u32*)propdata)+1));
	}
	DPRINT("h_active_area = %u, v_active_area =%u\n", pConf->lcd_basic.screen_actual_width, pConf->lcd_basic.screen_actual_width);
	propdata = fdt_getprop(dt_addr, nodeoffset, "interface", NULL);
	if (propdata == NULL) {
		printf("faild to get lcd_type!\n");
		propdata = "invalid";
	}	
	for(i = 0; i <= LCD_TYPE_NULL; i++) {
		if(!strcmp(propdata, lcd_type_table_match[i]))
			break;
	}
	pConf->lcd_basic.lcd_type = i;
	DPRINT("lcd_type= %s(%u),\n", propdata, pConf->lcd_basic.lcd_type);
	propdata = fdt_getprop(dt_addr, nodeoffset, "lcd_bits_option", NULL);
	if(propdata == NULL){
		printf("faild to get lcd_bits_option\n");
	}
	else {
		pConf->lcd_basic.lcd_bits = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_basic.lcd_bits_option = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
	}
	DPRINT("lcd_bits = %u, lcd_bits_option = %u\n", pConf->lcd_basic.lcd_bits, pConf->lcd_basic.lcd_bits_option);
	propdata = fdt_getprop(dt_addr, nodeoffset, "clock_hz_pol", NULL);
	if(propdata == NULL){
		printf("faild to get clock_hz_pol\n");
	}
	else {
		pConf->lcd_timing.lcd_clk = be32_to_cpup((u32*)propdata);
		pConf->lcd_timing.pol_cntl_addr = (be32_to_cpup((((u32*)propdata)+1)) << LCD_CPH1_POL);
	}
	DPRINT("pclk = %uHz, pol=%u\n", pConf->lcd_timing.lcd_clk, (pConf->lcd_timing.pol_cntl_addr >> LCD_CPH1_POL) & 1);
	propdata = fdt_getprop(dt_addr, nodeoffset, "hsync_width_backporch", NULL);
	if(propdata == NULL){
		printf("faild to get hsync_width_backporch\n");
	}
	else {
		pConf->lcd_timing.hsync_width = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_timing.hsync_bp = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
	}
	DPRINT("hsync width = %u, backporch = %u\n", pConf->lcd_timing.hsync_width, pConf->lcd_timing.hsync_bp);
	propdata = fdt_getprop(dt_addr, nodeoffset, "vsync_width_backporch", NULL);
	if(propdata == NULL){
		printf("faild to get vsync_width_backporch\n");
	}
	else {
		pConf->lcd_timing.vsync_width = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_timing.vsync_bp = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
	}
	DPRINT("vsync width = %u, backporch = %u\n", pConf->lcd_timing.vsync_width, pConf->lcd_timing.vsync_bp);
	propdata = fdt_getprop(dt_addr, nodeoffset, "pol_hsync_vsync", NULL);
	if(propdata == NULL){
		printf("faild to get pol_hsync_vsync\n");
	}
	else {
		pConf->lcd_timing.pol_cntl_addr = (pConf->lcd_timing.pol_cntl_addr & ~((1 << LCD_HS_POL) | (1 << LCD_VS_POL))) | ((be32_to_cpup((u32*)propdata) << LCD_HS_POL) | (be32_to_cpup((((u32*)propdata)+1)) << LCD_VS_POL));
	}
	DPRINT("pol hsync = %u, vsync = %u\n", (pConf->lcd_timing.pol_cntl_addr >> LCD_HS_POL) & 1, (pConf->lcd_timing.pol_cntl_addr >> LCD_VS_POL) & 1);
    return ret;
}

static inline int _get_lcd_default_config(Lcd_Config_t *pConf)
{
	int ret=0;
	int nodeoffset;
	char * propdata;
	int i;
	
	nodeoffset = fdt_path_offset(dt_addr, "/lcd");
	if(nodeoffset < 0) {
		printf("lcd driver init: not find /lcd node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}
	
	if (pConf->lcd_basic.lcd_bits_option == 1) {
		propdata = fdt_getprop(dt_addr, nodeoffset, "lcd_bits", NULL);
		if(propdata == NULL){
			DPRINT("don't find to match lcd_bits, use panel typical setting.\n");
		}
		else {
			pConf->lcd_basic.lcd_bits = (unsigned short)(be32_to_cpup((u32*)propdata));
			printf("lcd_bits = %u\n", pConf->lcd_basic.lcd_bits);
		}
	}
	//ttl & lvds config
	propdata = fdt_getprop(dt_addr, nodeoffset, "valid_hvsync_de", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match valid_hvsync_de, use default setting.\n");
	}
	else {
		pConf->lcd_timing.hvsync_valid = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_timing.de_valid = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
		printf("valid hvsync = %u, de = %u\n", pConf->lcd_timing.hvsync_valid, pConf->lcd_timing.de_valid);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "ttl_rb_bit_swap", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match ttl_rb_bit_swap, use default setting.\n");
	}
	else {
		pConf->lcd_timing.dual_port_cntl_addr = (be32_to_cpup((u32*)propdata) << LCD_RGB_SWP) | (be32_to_cpup((((u32*)propdata)+1)) << LCD_BIT_SWP);
		printf("ttl rb swap = %u, bit swap = %u\n", (pConf->lcd_timing.dual_port_cntl_addr >> LCD_RGB_SWP) & 1, (pConf->lcd_timing.dual_port_cntl_addr >> LCD_BIT_SWP) & 1);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "lvds_user_repack", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match lvds_user_repack, use default setting.\n");
		if (pConf->lcd_basic.lcd_bits == 6) 	//add auto setting lvds_repack by lcd_bits
			pConf->lvds_mlvds_config.lvds_config->lvds_repack = 0;
		else
			pConf->lvds_mlvds_config.lvds_config->lvds_repack = 1;
	}
	else {
		if ((be32_to_cpup((u32*)propdata)) == 0) {
			if (pConf->lcd_basic.lcd_bits == 6) 	//add auto setting lvds_repack by lcd_bits
				pConf->lvds_mlvds_config.lvds_config->lvds_repack = 0;
			else
				pConf->lvds_mlvds_config.lvds_config->lvds_repack = 1;
			DPRINT("lvds_repack = %u\n", pConf->lvds_mlvds_config.lvds_config->lvds_repack);
		}
		else {
			pConf->lvds_mlvds_config.lvds_config->lvds_repack = (be32_to_cpup((((u32*)propdata)+1)));
			printf("lvds_repack = %u\n", pConf->lvds_mlvds_config.lvds_config->lvds_repack);
		}
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "lvds_channel_pn_swap", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match lvds_channel_pn_swap, use default setting.\n");
	}
	else {
		pConf->lvds_mlvds_config.lvds_config->pn_swap = be32_to_cpup((u32*)propdata);
		printf("lvds_pn_swap = %u\n", pConf->lvds_mlvds_config.lvds_config->pn_swap);
	}
	//recommend setting
	propdata = fdt_getprop(dt_addr, nodeoffset, "clock_auto_generation", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match clock_auto_generation, use default setting.\n");
	}
	else {
		pConf->lcd_timing.clk_ctrl = ((pConf->lcd_timing.clk_ctrl & ~(1 << CLK_CTRL_AUTO)) | (be32_to_cpup((u32*)propdata) << CLK_CTRL_AUTO));
		printf("lcd clock auto generation =%u\n", (pConf->lcd_timing.clk_ctrl >> CLK_CTRL_AUTO) & 1);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "clock_spread_spectrum", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match clock_spread_spectrum, use default setting.\n");
	}
	else {
		pConf->lcd_timing.clk_ctrl = (pConf->lcd_timing.clk_ctrl & ~(0xf << CLK_CTRL_SS)) | (be32_to_cpup((u32*)propdata) << CLK_CTRL_SS);
		printf("lcd clock spread spectrum =%u\n", (pConf->lcd_timing.clk_ctrl >> CLK_CTRL_SS) & 0xf);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "clk_pll_div_clk_ctrl", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match clk_pll_div_clk_ctrl, use default setting.\n");
	}
	else {
		pConf->lcd_timing.pll_ctrl = be32_to_cpup((u32*)propdata);
		pConf->lcd_timing.div_ctrl = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_timing.clk_ctrl = (pConf->lcd_timing.clk_ctrl & ~(0xffff >> 0)) | (be32_to_cpup((((u32*)propdata)+2)) >> 0);
		printf("pll_ctrl = 0x%x, div_ctrl = 0x%x, clk_ctrl=0x%x\n", pConf->lcd_timing.pll_ctrl, pConf->lcd_timing.div_ctrl, (pConf->lcd_timing.clk_ctrl & 0xffff));
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "video_on_pixel_line", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match video_on_pixel_line, use default setting.\n");
	}
	else {
		pConf->lcd_timing.video_on_pixel = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_timing.video_on_line = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
		printf("video_on_pixel = %u, video_on_line = %u\n", pConf->lcd_timing.video_on_pixel, pConf->lcd_timing.video_on_line);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "hsign_hoffset_vsign_voffset", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match hsign_hoffset_vsign_voffset, use default setting.\n");
		pDev->pConf->lcd_timing.h_offset = 0;
		pDev->pConf->lcd_timing.v_offset = 0;
	}
	else {
		pConf->lcd_timing.h_offset = ((be32_to_cpup((u32*)propdata) << 31) | ((be32_to_cpup((((u32*)propdata)+1)) & 0xffff) << 0));
		pConf->lcd_timing.v_offset = ((be32_to_cpup((((u32*)propdata)+2)) << 31) | ((be32_to_cpup((((u32*)propdata)+3)) & 0xffff) << 0));
		printf("h_offset = %s%u, ", (((pConf->lcd_timing.h_offset >> 31) & 1) ? "+" : "-"), (pConf->lcd_timing.h_offset & 0xffff));
		printf("v_offset = %s%u\n", (((pConf->lcd_timing.v_offset >> 31) & 1) ? "+" : "-"), (pConf->lcd_timing.v_offset & 0xffff));
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "lvds_phy_ctrl", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match lvds_phy_ctrl, use default setting.\n");
	}
	else {
		pConf->lvds_mlvds_config.lvds_phy_ctrl = be32_to_cpup((u32*)propdata);
		printf("lvds_phy_ctrl = 0x%x\n", pConf->lvds_mlvds_config.lvds_phy_ctrl);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "dither_user_ctrl", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match dither_user_ctrl, use default setting.\n");
	}
	else {
		pConf->lcd_effect.dith_user = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_effect.dith_cntl_addr = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
		printf("dither user = %u, dither ctrl = 0x%x\n", pConf->lcd_effect.dith_user, pConf->lcd_effect.dith_cntl_addr);
	}
	//lcd effect
	propdata = fdt_getprop(dt_addr, nodeoffset, "vadj_brightness_contrast_saturation", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match vadj_brightness_contrast_saturation, use default setting.\n");
	}
	else {
		pConf->lcd_effect.vadj_brightness = be32_to_cpup((u32*)propdata);
		pConf->lcd_effect.vadj_contrast = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_effect.vadj_saturation = be32_to_cpup((((u32*)propdata)+2));
		printf("vadj_brightness = 0x%x, vadj_contrast = 0x%x, vadj_saturation = 0x%x\n", pConf->lcd_effect.vadj_brightness, pConf->lcd_effect.vadj_contrast, pConf->lcd_effect.vadj_saturation);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "rgb_base_coeff", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match rgb_base_coeff, use default setting.\n");
	}
	else {
		pConf->lcd_effect.rgb_base_addr = (unsigned short)(be32_to_cpup((u32*)propdata));
		pConf->lcd_effect.rgb_coeff_addr = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
		printf("rgb_base = 0x%x, rgb_coeff = 0x%x\n", pConf->lcd_effect.rgb_base_addr, pConf->lcd_effect.rgb_coeff_addr);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_en_revert", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match gamma_en_revert, use default setting.\n");
	}
	else {
		pConf->lcd_effect.gamma_cntl_port = (be32_to_cpup((u32*)propdata) << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL);
		pConf->lcd_effect.gamma_vcom_hswitch_addr = 0;
		printf("gamma_en = %u\n", (pConf->lcd_effect.gamma_cntl_port >> LCD_GAMMA_EN) & 1);
	}
	unsigned int lcd_gamma_multi = 0;
	propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_multi_rgb_coeff", NULL);
	if(propdata == NULL){
		DPRINT("don't find to match gamma_multi_rgb_coeff, use default setting.\n");
	}
	else {
		lcd_gamma_multi = be32_to_cpup((u32*)propdata);
		pConf->lcd_effect.gamma_r_coeff = (unsigned short)(be32_to_cpup((((u32*)propdata)+1)));
		pConf->lcd_effect.gamma_g_coeff = (unsigned short)(be32_to_cpup((((u32*)propdata)+2)));
		pConf->lcd_effect.gamma_b_coeff = (unsigned short)(be32_to_cpup((((u32*)propdata)+3)));
		printf("gamma_multi = %u, gamma_r_coeff = %u, gamma_g_coeff = %u, gamma_b_coeff = %u\n", lcd_gamma_multi, pConf->lcd_effect.gamma_r_coeff, pConf->lcd_effect.gamma_g_coeff, pConf->lcd_effect.gamma_b_coeff);
	}
	if (lcd_gamma_multi == 1) {
		propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_table_r", NULL);
		if(propdata == NULL){
			DPRINT("don't find to match gamma_table_r, use default table.\n");
			lcd_setup_gamma_table(pConf, 0);
		}
		else {
			for (i=0; i<256; i++) {
				pConf->lcd_effect.GammaTableR[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
			}
			printf("load gamma_table_r.\n");
		}
		propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_table_g", NULL);
		if(propdata == NULL){
			DPRINT("don't find to match gamma_table_g, use default table.\n");
			lcd_setup_gamma_table(pConf, 1);
		}
		else {
			for (i=0; i<256; i++) {
				pConf->lcd_effect.GammaTableG[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
			}
			printf("load gamma_table_g.\n");
		}
		propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_table_b", NULL);
		if(propdata == NULL){
			DPRINT("don't find to match gamma_table_b, use default table.\n");
			lcd_setup_gamma_table(pConf, 2);
		}
		else {
			for (i=0; i<256; i++) {
				pConf->lcd_effect.GammaTableB[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
			}
			printf("load gamma_table_b.\n");
		}
	}
	else {
		propdata = fdt_getprop(dt_addr, nodeoffset, "gamma_table", NULL);
		if(propdata == NULL){
			DPRINT("don't find to match gamma_table, use default table.\n");
			lcd_setup_gamma_table(pConf, 3);
		}
		else {
			for (i=0; i<256; i++) {
				pConf->lcd_effect.GammaTableR[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
				pConf->lcd_effect.GammaTableG[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
				pConf->lcd_effect.GammaTableB[i] = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)) << 2);
			}
			printf("load gamma_table.\n");
		}
	}	
	return ret;
}

static inline int _get_lcd_power_config(Lcd_Config_t *pConf)
{
	int i;
	int index;
	int ret=0;
	int nodeoffset;
	char* propdata;
	char propname[20];
	struct fdt_property *prop;
	char *p;
	const char * str;
	
	nodeoffset = fdt_path_offset(dt_addr, "/lcd");
	if(nodeoffset < 0) {
		printf("lcd driver init: not find /lcd node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}
	
	//lcd power on/off only for uboot
	propdata = fdt_getprop(dt_addr, nodeoffset, "power_on_uboot", NULL);
	if (propdata == NULL) {
		DPRINT("faild to get power_on_uboot\n");
		pConf->lcd_power_ctrl.lcd_power_on_uboot.type = LCD_POWER_TYPE_NULL;
		pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = GPIO_NULL;
		pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_INPUT;
		pConf->lcd_power_ctrl.lcd_power_on_uboot.delay = 0;
	}
	else {
		prop = container_of(propdata, struct fdt_property, data);
		p = prop->data;
		str = p;
		if (strcmp(str, "null") == 0) {
			DPRINT("faild to get power_on_uboot\n");
			pConf->lcd_power_ctrl.lcd_power_on_uboot.type = LCD_POWER_TYPE_NULL;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = GPIO_NULL;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_INPUT;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.delay = 0;
		}
		else {
			for(index = 0; index <= LCD_POWER_TYPE_NULL; index++) {
				if(!strcmp(str, lcd_power_type_table[index]))
					break;
			}
			pConf->lcd_power_ctrl.lcd_power_on_uboot.type = index;
			if (pConf->lcd_power_ctrl.lcd_power_on_uboot.type < LCD_POWER_TYPE_SIGNAL) {
				p += strlen(p) + 1;
				str = p;
				if (pConf->lcd_power_ctrl.lcd_power_on_uboot.type == LCD_POWER_TYPE_CPU) {
					pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = amlogic_gpio_name_map_num(str);
				}
				else if ((pConf->lcd_power_ctrl.lcd_power_on_uboot.type == LCD_POWER_TYPE_AXP202) || (pConf->lcd_power_ctrl.lcd_power_on_uboot.type == LCD_POWER_TYPE_AML1212)) {
					pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = amlogic_pmu_gpio_name_map_num(str);
				}

				p += strlen(p) + 1;
				str = p;
				if (strcmp(str, "output_low") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_OUTPUT_LOW;
				}
				else if (strcmp(str, "output_high") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_OUTPUT_HIGH;
				}
				else if (strcmp(str, "input") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_INPUT;
				}
			}
			pConf->lcd_power_ctrl.lcd_power_on_uboot.delay = 50;
			printf("find power_on_uboot: type = %s(%d), ", lcd_power_type_table[pConf->lcd_power_ctrl.lcd_power_on_uboot.type], pConf->lcd_power_ctrl.lcd_power_on_uboot.type);
			printf("gpio = %d, ", pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio);
			printf("value = %d\n", pConf->lcd_power_ctrl.lcd_power_on_uboot.value);
		}
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "power_off_uboot", NULL);
	if (propdata == NULL) {
		DPRINT("faild to get power_off_uboot\n");
		pConf->lcd_power_ctrl.lcd_power_off_uboot.type = LCD_POWER_TYPE_NULL;
		pConf->lcd_power_ctrl.lcd_power_off_uboot.gpio = GPIO_NULL;
		pConf->lcd_power_ctrl.lcd_power_off_uboot.value = LCD_POWER_GPIO_INPUT;
		pConf->lcd_power_ctrl.lcd_power_off_uboot.delay = 0;
	}
	else {
		prop = container_of(propdata, struct fdt_property, data);
		p = prop->data;
		str = p;
		if (strcmp(str, "null") == 0) {
			DPRINT("faild to get power_on_uboot\n");
			pConf->lcd_power_ctrl.lcd_power_on_uboot.type = LCD_POWER_TYPE_NULL;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.gpio = GPIO_NULL;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.value = LCD_POWER_GPIO_INPUT;
			pConf->lcd_power_ctrl.lcd_power_on_uboot.delay = 0;
		}
		else {
			for(index = 0; index <= LCD_POWER_TYPE_NULL; index++) {
				if(!strcmp(str, lcd_power_type_table[index]))
					break;
			}
			pConf->lcd_power_ctrl.lcd_power_off_uboot.type = index;
			if (pConf->lcd_power_ctrl.lcd_power_off_uboot.type < LCD_POWER_TYPE_SIGNAL) {
				p += strlen(p) + 1;
				str = p;
				if (pConf->lcd_power_ctrl.lcd_power_off_uboot.type == LCD_POWER_TYPE_CPU) {
					pConf->lcd_power_ctrl.lcd_power_off_uboot.gpio = amlogic_gpio_name_map_num(str);
				}
				else if ((pConf->lcd_power_ctrl.lcd_power_off_uboot.type == LCD_POWER_TYPE_AXP202) || (pConf->lcd_power_ctrl.lcd_power_off_uboot.type == LCD_POWER_TYPE_AML1212)) {
					pConf->lcd_power_ctrl.lcd_power_off_uboot.gpio = amlogic_pmu_gpio_name_map_num(str);
				}

				p += strlen(p) + 1;
				str = p;
				if (strcmp(str, "output_low") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_uboot.value = LCD_POWER_GPIO_OUTPUT_LOW;
				}
				else if (strcmp(str, "output_high") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_uboot.value = LCD_POWER_GPIO_OUTPUT_HIGH;
				}
				else if (strcmp(str, "input") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_uboot.value = LCD_POWER_GPIO_INPUT;
				}
			}
			pConf->lcd_power_ctrl.lcd_power_off_uboot.delay = 0;
			printf("find power_off_uboot: type = %s(%d), ", lcd_power_type_table[pConf->lcd_power_ctrl.lcd_power_off_uboot.type], pConf->lcd_power_ctrl.lcd_power_off_uboot.type);
			printf("gpio = %d, ", pConf->lcd_power_ctrl.lcd_power_off_uboot.gpio);
			printf("value = %d\n", pConf->lcd_power_ctrl.lcd_power_off_uboot.value);
		}
	}
	
	//lcd power on
	for (i=0; i < 10; i++) {
		sprintf(propname, "power_on_step_%d", i+1);
		propdata = fdt_getprop(dt_addr, nodeoffset, propname, NULL);
		if (propdata == NULL) {
			DPRINT("faild to get %s\n", propname);
			break;
		}
		else {
			prop = container_of(propdata, struct fdt_property, data);
			p = prop->data;
			str = p;
			if (strcmp(str, "null") == 0) {
				break;
			}
			for(index = 0; index <= LCD_POWER_TYPE_NULL; index++) {
				if(!strcmp(str, lcd_power_type_table[index]))
					break;
			}
			pConf->lcd_power_ctrl.lcd_power_on_config[i].type = index;	
			if (pConf->lcd_power_ctrl.lcd_power_on_config[i].type < LCD_POWER_TYPE_SIGNAL) {
				p += strlen(p) + 1;
				str = p;
				if (pConf->lcd_power_ctrl.lcd_power_on_config[i].type == LCD_POWER_TYPE_CPU) {
					pConf->lcd_power_ctrl.lcd_power_on_config[i].gpio = amlogic_gpio_name_map_num(str);
				}
				else if ((pConf->lcd_power_ctrl.lcd_power_on_config[i].type == LCD_POWER_TYPE_AXP202) || (pConf->lcd_power_ctrl.lcd_power_on_config[i].type == LCD_POWER_TYPE_AML1212)) {
					pConf->lcd_power_ctrl.lcd_power_on_config[i].gpio = amlogic_pmu_gpio_name_map_num(str);
				}

				p += strlen(p) + 1;
				str = p;
				if (strcmp(str, "output_low") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_config[i].value = LCD_POWER_GPIO_OUTPUT_LOW;
				}
				else if (strcmp(str, "output_high") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_config[i].value = LCD_POWER_GPIO_OUTPUT_HIGH;
				}
				else if (strcmp(str, "input") == 0) {
					pConf->lcd_power_ctrl.lcd_power_on_config[i].value = LCD_POWER_GPIO_INPUT;
				}
			}
		}
	}
	pConf->lcd_power_ctrl.lcd_power_on_step = i;
	DPRINT("lcd_power_on_step = %d\n", pConf->lcd_power_ctrl.lcd_power_on_step);

	propdata = fdt_getprop(dt_addr, nodeoffset, "power_on_delay", NULL);
	if (propdata == NULL) {
		printf("faild to get power_on_delay\n");
	}
	else {
		for (i=0; i<pConf->lcd_power_ctrl.lcd_power_on_step; i++) {
			pConf->lcd_power_ctrl.lcd_power_on_config[i].delay = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)));
		}
	}

	//lcd power off
	for (i=0; i < 10; i++) {
		sprintf(propname, "power_off_step_%d", i+1);
		propdata = fdt_getprop(dt_addr, nodeoffset, propname, NULL);
		if (propdata == NULL) {
			DPRINT("faild to get %s\n", propname);
			break;
		}	
		else {
			prop = container_of(propdata, struct fdt_property, data);
			p = prop->data;
			str = p;
			if (strcmp(str, "null") == 0) {
				break;
			}
			for(index = 0; index <= LCD_POWER_TYPE_NULL; index++) {
				if(!strcmp(str, lcd_power_type_table[index]))
					break;
			}
			pConf->lcd_power_ctrl.lcd_power_off_config[i].type = index;
			if (pConf->lcd_power_ctrl.lcd_power_off_config[i].type < LCD_POWER_TYPE_SIGNAL) {
				p += strlen(p) + 1;
				str = p;
				if (pConf->lcd_power_ctrl.lcd_power_off_config[i].type == LCD_POWER_TYPE_CPU) {
					pConf->lcd_power_ctrl.lcd_power_off_config[i].gpio = amlogic_gpio_name_map_num(str);
				}
				else if ((pConf->lcd_power_ctrl.lcd_power_off_config[i].type == LCD_POWER_TYPE_AXP202) || (pConf->lcd_power_ctrl.lcd_power_off_config[i].type == LCD_POWER_TYPE_AML1212)) {
					pConf->lcd_power_ctrl.lcd_power_off_config[i].gpio = amlogic_pmu_gpio_name_map_num(str);
				}

				p += strlen(p) + 1;
				str = p;
				if (strcmp(str, "output_low") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_config[i].value = LCD_POWER_GPIO_OUTPUT_LOW;
				}
				else if (strcmp(str, "output_high") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_config[i].value = LCD_POWER_GPIO_OUTPUT_HIGH;
				}
				else if (strcmp(str, "input") == 0) {
					pConf->lcd_power_ctrl.lcd_power_off_config[i].value = LCD_POWER_GPIO_INPUT;
				}
			}
		}
	}
	pConf->lcd_power_ctrl.lcd_power_off_step = i;
	DPRINT("lcd_power_off_step = %d\n", pConf->lcd_power_ctrl.lcd_power_off_step);
	
	propdata = fdt_getprop(dt_addr, nodeoffset, "power_off_delay", NULL);
	if (propdata == NULL) {
		printf("faild to get power_off_delay\n");
	}
	else {
		for (i=0; i<pConf->lcd_power_ctrl.lcd_power_off_step; i++) {
			pConf->lcd_power_ctrl.lcd_power_off_config[i].delay = (unsigned short)(be32_to_cpup((((u32*)propdata)+i)));
		}
	}
	
	for (i=0; i<pConf->lcd_power_ctrl.lcd_power_on_step; i++) {
		DPRINT("power on step %d: type = %s(%d)\n", i+1, lcd_power_type_table[pConf->lcd_power_ctrl.lcd_power_on_config[i].type], pConf->lcd_power_ctrl.lcd_power_on_config[i].type);
		DPRINT("power on step %d: gpio = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_on_config[i].gpio);
		DPRINT("power on step %d: value = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_on_config[i].value);
		DPRINT("power on step %d: delay = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_on_config[i].delay);
	}
	
	for (i=0; i<pConf->lcd_power_ctrl.lcd_power_off_step; i++) {
		DPRINT("power off step %d: type = %s(%d)\n", i+1, lcd_power_type_table[pConf->lcd_power_ctrl.lcd_power_off_config[i].type], pConf->lcd_power_ctrl.lcd_power_off_config[i].type);
		DPRINT("power off step %d: gpio = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_off_config[i].gpio);
		DPRINT("power off step %d: value = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_off_config[i].value);
		DPRINT("power off step %d: delay = %d\n", i+1, pConf->lcd_power_ctrl.lcd_power_off_config[i].delay);
	}
	return ret;
}

static inline int _get_lcd_backlight_config(void)
{
	int ret=0;
	int nodeoffset;
	char * propdata;
	unsigned int bl_para[2];
	int i;
	
	nodeoffset = fdt_path_offset(dt_addr, "/backlight");
	if(nodeoffset < 0) {
		printf("backlight init: not find /backlight node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}

	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_level_default", NULL);
	if(propdata == NULL){
		printf("faild to get bl_level_default\n");
		bl_config.level_default = BL_LEVEL_DEFAULT;
	}
	else {
		bl_config.level_default = be32_to_cpup((u32*)propdata);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_ctrl_method", NULL);
	if(propdata == NULL){
		printf("faild to get bl_ctrl_method\n");
		bl_config.method = BL_CTL_GPIO;
	}
	else {
		if (strcmp(propdata, "gpio") == 0)
			bl_config.method = BL_CTL_GPIO;
		else
			bl_config.method = BL_CTL_PWM;
	}
	DPRINT("bl control method: %s(%u)\n", propdata, bl_config.method);
	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_pwm_port_gpio_used", NULL);
	if(propdata == NULL){
		printf("faild to get bl_pwm_port_gpio_used\n");
		bl_config.pwm_port = BL_PWM_D;
		bl_config.pwm_gpio_used = 0;
	}
	else {
		if (strcmp(propdata, "PWM_C") == 0)
			bl_config.pwm_port = BL_PWM_C;
		else if (strcmp(propdata, "PWM_D") == 0)
			bl_config.pwm_port = BL_PWM_D;
		
		if (strcmp(propdata, "1") == 0)
			bl_config.pwm_gpio_used = 1;
		else
			bl_config.pwm_gpio_used = 0;
	}
	DPRINT("bl pwm port: %s(%u)\n", propdata, bl_config.pwm_port);
	DPRINT("bl pwm gpio used: %u\n", bl_config.pwm_gpio_used);
	if ((bl_config.method == BL_CTL_GPIO) || (bl_config.pwm_gpio_used == 1)) {
		propdata = fdt_getprop(dt_addr, nodeoffset, "bl_gpio_port", NULL);
		if (propdata == NULL) {
			printf("faild to get bl_gpio_port\n");
			bl_config.gpio = GPIOD_1;
		}
		else {
			bl_config.gpio = amlogic_gpio_name_map_num(propdata);
		}
		DPRINT("bl gpio = %s(%d)\n", propdata, bl_config.gpio);
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_gpio_dim_max_min", NULL);
	if(propdata == NULL){
		printf("faild to get bl_gpio_dim_max_min\n");
		bl_config.dim_max = 0x0;
		bl_config.dim_min = 0xf;
	}
	else {
		bl_config.dim_max = (be32_to_cpup((u32*)propdata));
		bl_config.dim_min = (be32_to_cpup((((u32*)propdata)+1)));
	}
	DPRINT("bl dim max=%u, min=%u\n", bl_config.dim_max, bl_config.dim_min);
	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_pwm_cnt_div", NULL);
	if(propdata == NULL){
		printf("faild to get bl_pwm_cnt_div\n");
		bl_config.pwm_cnt = 60000;
		bl_config.pwm_pre_div = 0;
	}
	else {
		bl_config.pwm_cnt = be32_to_cpup((u32*)propdata);
		bl_config.pwm_pre_div = be32_to_cpup((((u32*)propdata)+1));
	}
	DPRINT("bl pwm cnt=%u, div=%u\n", bl_config.pwm_cnt, bl_config.pwm_pre_div);
	propdata = fdt_getprop(dt_addr, nodeoffset, "bl_pwm_duty_max_min", NULL);
	if(propdata == NULL){
		printf("faild to get bl_pwm_duty_max_min\n");
		bl_para[0] = 100;
		bl_para[1] = 20;
	}
	else {
		bl_para[0] = be32_to_cpup((u32*)propdata);
		bl_para[1] = be32_to_cpup((((u32*)propdata)+1));
	}
	bl_config.pwm_max = (bl_config.pwm_cnt * bl_para[0] / 100);
	bl_config.pwm_min = (bl_config.pwm_cnt * bl_para[1] / 100);
	DPRINT("bl pwm max=%u\%, min=%u\%\n", bl_para[0], bl_para[1]);
	
	//get backlight pinmux for pwm
	int len;
	nodeoffset = fdt_path_offset(dt_addr, "/pinmux/lcd_backlight");
	if(nodeoffset < 0) {
		printf("backlight init: not find /pinmux/lcd_backlight node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}
	
	propdata = fdt_getprop(dt_addr, nodeoffset, "amlogic,setmask", &len);
	if(propdata == NULL){
		printf("faild to get amlogic,setmask\n");
		bl_config.pinmux_set_num = 0;
	}
	else {
		bl_config.pinmux_set_num = len / 8;
		for (i=0; i<bl_config.pinmux_set_num; i++) {
			bl_config.pinmux_set[i][0] = be32_to_cpup((((u32*)propdata)+2*i));
			bl_config.pinmux_set[i][1] = be32_to_cpup((((u32*)propdata)+2*i+1));
		}
	}
	propdata = fdt_getprop(dt_addr, nodeoffset, "amlogic,clrmask", &len);
	if(propdata == NULL){
		printf("faild to get amlogic,clrmask\n");
		bl_config.pinmux_clr_num = 0;
	}
	else {
		bl_config.pinmux_clr_num = len / 8;
		for (i=0; i<bl_config.pinmux_clr_num; i++) {
			bl_config.pinmux_clr[i][0] = be32_to_cpup((((u32*)propdata)+2*i));
			bl_config.pinmux_clr[i][1] = be32_to_cpup((((u32*)propdata)+2*i+1));
		}
	}
	
	for (i=0; i<bl_config.pinmux_set_num; i++) {
		DPRINT("backlight pinmux set %d: mux_num = %d, mux_mask = 0x%x\n", i+1, bl_config.pinmux_set[i][0], bl_config.pinmux_set[i][1]);
	}
	for (i=0; i<bl_config.pinmux_clr_num; i++) {
		DPRINT("backlight pinmux clr %d: mux_num = %d, mux_mask = 0x%x\n", i+1, bl_config.pinmux_clr[i][0], bl_config.pinmux_clr[i][1]);
	}
	
	return ret;
}
#endif

static inline void _set_panel_info(void)
{
	panel_info.vd_base = simple_strtoul(getenv("fb_addr"), NULL, NULL);
	panel_info.vl_col = simple_strtoul(getenv("display_width"), NULL, NULL);
	panel_info.vl_row = simple_strtoul(getenv("display_height"), NULL, NULL);
	panel_info.vl_bpix = simple_strtoul(getenv("display_bpp"), NULL, NULL);
	panel_info.vd_color_fg = simple_strtoul(getenv("display_color_fg"), NULL, NULL);
	panel_info.vd_color_bg = simple_strtoul(getenv("display_color_bg"), NULL, NULL);
}

int lcd_probe(void)
{
    pDev = (lcd_dev_t *)malloc(sizeof(lcd_dev_t));
    if (!pDev) {
        printf("[tcon]: Not enough memory.\n");
        return -1;
    }
	printf("lcd driver version: %s@%s\n", DRIVER_DATE, DRIVER_VER);
	_set_panel_info();
#ifdef CONFIG_OF_LIBFDT
#ifdef CONFIG_DT_PRELOAD
	pDev->conf = lcd_config_dft;
	int ret;

	if (getenv("dtbaddr") == NULL) {
#ifdef CONFIG_DTB_LOAD_ADDR
		dt_addr = CONFIG_DTB_LOAD_ADDR;
#else
		dt_addr = 0x83000000;
#endif
	}
	else {
		dt_addr = simple_strtoul (getenv ("dtbaddr"), NULL, 16);
	}
	ret = fdt_check_header(dt_addr);
	if(ret != 0) {
		dts_ready = 0;
		pDev->conf = lcd_config;
		printf("check dts: %s, load default lcd parameters\n", fdt_strerror(ret));
		if (pDev->conf.lcd_basic.model_name == NULL)
			pDev->conf.lcd_basic.model_name = PANEL_MODEL_DEFAULT;
		printf("load lcd model: %s\n", pDev->conf.lcd_basic.model_name);
	}
	else {
		dts_ready = 1;
		printf("load lcd model from dtb\n");
		_get_lcd_model_timing(&pDev->conf);
		_get_lcd_default_config(&pDev->conf);
		_get_lcd_power_config(&pDev->conf);
		_get_lcd_backlight_config();
	}
#else
	dts_ready = 0;
	pDev->conf = lcd_config;
	if (pDev->conf.lcd_basic.model_name == NULL)
		pDev->conf.lcd_basic.model_name = PANEL_MODEL_DEFAULT;
	printf("load lcd model: %s\n", pDev->conf.lcd_basic.model_name);
#endif
#else
	dts_ready = 0;
	pDev->conf = lcd_config;
	if (pDev->conf.lcd_basic.model_name == NULL)
		pDev->conf.lcd_basic.model_name = PANEL_MODEL_DEFAULT;
	printf("load lcd model: %s\n", pDev->conf.lcd_basic.model_name);
#endif
	
    _lcd_init(&pDev->conf);
	
	update_panel_operation();
	
    return 0;
}

int lcd_remove(void)
{
	_disable_display_driver(&pDev->conf);
	free(pDev);
    return 0;
}
