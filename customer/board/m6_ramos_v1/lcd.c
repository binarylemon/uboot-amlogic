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
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/lcdoutc.h>
#include <asm/arch/osd.h>
#include <asm/arch/osd_hw.h>
#include <aml_i2c.h>
#include <amlogic/aml_lcd.h>
#include <amlogic/vinfo.h>
#include <sn7325.h>
#include <video_fb.h>

#ifdef CONFIG_AW_AXP20
#include <axp-gpio.h>
#endif

#define DEBUG

extern GraphicDevice aml_gdev;
vidinfo_t panel_info;

//*****************************************
// Define backlight control method
//*****************************************
#define BL_CTL_GPIO 	0
#define BL_CTL_PWM  	1
#define BL_CTL      	BL_CTL_GPIO

//backlight controlled parameters in driver, define the real backlight level
#if (BL_CTL==BL_CTL_GPIO)
#define	DIM_MAX			0x0
#define	DIM_MIN			0xd	
#elif (BL_CTL==BL_CTL_PWM)
#define	PWM_CNT			60000			//PWM_CNT <= 65535
#define	PWM_PRE_DIV		0				//pwm_freq = 24M / (pre_div + 1) / PWM_CNT
#define PWM_MAX         (PWM_CNT * 8 / 10)		
#define PWM_MIN         (PWM_CNT * 3 / 10)  //according to LED-driver-IC reqirement(or customer reqirement), 0 <= PWM_MIN < PWM_MAX
#endif

//brightness level in Android UI menu
#define BL_MAX_LEVEL    	255
#define BL_MIN_LEVEL    	20	

static unsigned bl_level = 0;
static Bool_t data_status = OFF;
static Bool_t bl_status = OFF;
//*****************************************

static void lvds_ports_ctrl(Bool_t status)
{ 
    debug("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
	if (status) 
	{
        WRITE_MPEG_REG(LVDS_GEN_CNTL,  READ_MPEG_REG(LVDS_GEN_CNTL) | (1 << 3)); // enable fifo
        WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) | (0x2f<<0));  //enable LVDS phy port
    }else {
		WRITE_MPEG_REG(LVDS_PHY_CNTL3, READ_MPEG_REG(LVDS_PHY_CNTL3) & ~(1<<0));
		WRITE_MPEG_REG(LVDS_PHY_CNTL5, READ_MPEG_REG(LVDS_PHY_CNTL5) & ~(1<<11));  //shutdown lvds phy
        WRITE_MPEG_REG(LVDS_PHY_CNTL4, READ_MPEG_REG(LVDS_PHY_CNTL4) & ~(0x7f<<0));  //disable LVDS phy port
        WRITE_MPEG_REG(LVDS_GEN_CNTL,  READ_MPEG_REG(LVDS_GEN_CNTL) & ~(1 << 3)); // disable fifo		
    }
	data_status = status;
}

static void backlight_power_ctrl(Bool_t status)
{	
    if( status == ON )
	{
		if ((bl_status == ON) || (data_status == OFF))
			return;
        WRITE_CBUS_REG_BITS(LED_PWM_REG0, 1, 12, 2);
        mdelay(20); 
	
		//BL_EN: GPIOD_1(PWM_D)
#if (BL_CTL==BL_CTL_GPIO)	
	    set_gpio_val(GPIOD_bank_bit0_9(1), GPIOD_bit_bit0_9(1), 1);
	    set_gpio_mode(GPIOD_bank_bit0_9(1), GPIOD_bit_bit0_9(1), GPIO_OUTPUT_MODE);
#elif (BL_CTL==BL_CTL_PWM)
	    WRITE_CBUS_REG_BITS(PWM_PWM_D, 0, 0, 16);  		//pwm low
		WRITE_CBUS_REG_BITS(PWM_PWM_D, PWM_MAX, 16, 16);	//pwm high
		WRITE_MPEG_REG(PWM_MISC_REG_CD, (READ_MPEG_REG(PWM_MISC_REG_CD) & ~(0x7f<<16)) | ((1 << 23) | (PWM_PRE_DIV<<16) | (1<<1)));  //enable pwm clk & pwm output
		WRITE_MPEG_REG(PERIPHS_PIN_MUX_2, READ_MPEG_REG(PERIPHS_PIN_MUX_2) | (1<<3));  //enable pwm pinmux
#endif    	
	}
	else
	{
		mdelay(20);	
		//BL_EN -> GPIOD_1: 0	
	    WRITE_MPEG_REG(PWM_MISC_REG_CD, READ_MPEG_REG(PWM_MISC_REG_CD) & ~((1 << 23) | (1<<1)));  //disable pwm_clk & pwm port
		set_gpio_val(GPIOD_bank_bit0_9(1), GPIOD_bit_bit0_9(1), 0);
	    set_gpio_mode(GPIOD_bank_bit0_9(1), GPIOD_bit_bit0_9(1), GPIO_OUTPUT_MODE);
		
		bl_level = 0;	    
	}	
	bl_status = status;
	debug("%s: power %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

#define BL_MID_LEVEL    		128
#define BL_MAPPED_MID_LEVEL		102
static unsigned set_level;
void set_backlight_level(unsigned level)
{
	debug("%s :%d\n", __FUNCTION__,level);
	level = (level > BL_MAX_LEVEL ? BL_MAX_LEVEL : (level < BL_MIN_LEVEL ? 0 : level));	
	set_level =  level;
 
    if (set_level > BL_MID_LEVEL) {
                set_level = ((set_level - BL_MID_LEVEL)*(BL_MAX_LEVEL-BL_MAPPED_MID_LEVEL))/(BL_MAX_LEVEL - BL_MID_LEVEL) + BL_MAPPED_MID_LEVEL; 
    } else {  
                set_level = (set_level*BL_MAPPED_MID_LEVEL)/BL_MID_LEVEL;
    }

	if ((set_level == 0)&&(bl_level != 0))
	{
		backlight_power_ctrl(OFF);		
	}
	else if (set_level > 0)
	{	
#if (BL_CTL==BL_CTL_GPIO)
		set_level = DIM_MIN - ((set_level - 15) * (DIM_MIN - DIM_MAX)) / (BL_MAX_LEVEL - 15);	//BL_MIN_LEVEL(20) mapping to 15
		WRITE_CBUS_REG_BITS(LED_PWM_REG0, set_level, 0, 4);
#elif (BL_CTL==BL_CTL_PWM)
		set_level = (PWM_MAX - PWM_MIN) * (set_level - 15) / (BL_MAX_LEVEL - 15) + PWM_MIN;	//BL_MIN_LEVEL(20) mapping to 15
		WRITE_CBUS_REG_BITS(PWM_PWM_D, (PWM_CNT - set_level), 0, 16);  //pwm low
    	WRITE_CBUS_REG_BITS(PWM_PWM_D, set_level, 16, 16);  //pwm high
#endif
		if (bl_level == 0)
		{
			backlight_power_ctrl(ON);				
		}
	}
    bl_level = level;
}

unsigned get_backlight_level(void)
{
    debug("%s :%d\n", __FUNCTION__,bl_level);
    return bl_level;
}

static void lcd_power_ctrl(Bool_t status)
{
	debug("%s: power %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
    if (status) 
	{
		//GPIOA27 -> LCD_PWR_EN#: 0  lcd 3.3v
	    set_gpio_val(GPIOA_bank_bit0_27(27), GPIOA_bit_bit0_27(27), 0);
	    set_gpio_mode(GPIOA_bank_bit0_27(27), GPIOA_bit_bit0_27(27), GPIO_OUTPUT_MODE);
	    mdelay(20);	    
    
#ifdef CONFIG_AW_AXP20
		//AXP_GPIO3 -> VCCx3_EN#: 1
	    axp_gpio_set_io(3,1);
	    axp_gpio_set_value(3, 0);
#else
	    //GPIOD2 -> VCCx3_EN: 1
	    set_gpio_val(GPIOC_bank_bit0_15(2), GPIOC_bit_bit0_15(2), 1);
	    set_gpio_mode(GPIOC_bank_bit0_15(2), GPIOC_bit_bit0_15(2), GPIO_OUTPUT_MODE);
#endif    
    	mdelay(20); 
		
		//data ports enable
	    lvds_ports_ctrl(ON); 
		mdelay(200);
	}
	else
	{
		backlight_power_ctrl(OFF);		
		mdelay(250);
		//data ports disable		
	    lvds_ports_ctrl(OFF);    
	    mdelay(20);   
    
#ifdef CONFIG_AW_AXP20
		//AXP_GPIO3 -> VCCx3_EN#: 0
	    axp_gpio_set_io(3,0);
#else
	    //GPIOD2 -> VCCx3_EN: 0
	    set_gpio_val(GPIOC_bank_bit0_15(2), GPIOC_bit_bit0_15(2), 0);
	    set_gpio_mode(GPIOC_bank_bit0_15(2), GPIOC_bit_bit0_15(2), GPIO_OUTPUT_MODE);
#endif    
    	mdelay(20);
    
	    //GPIOA27 -> LCD_PWR_EN#: 1  lcd 3.3v
	    set_gpio_val(GPIOA_bank_bit0_27(27), GPIOA_bit_bit0_27(27), 1);
	    set_gpio_mode(GPIOA_bank_bit0_27(27), GPIOA_bit_bit0_27(27), GPIO_OUTPUT_MODE);
	    mdelay(500);
	}   
}

int  video_dac_enable(unsigned char enable_mask)
{
	debug("%s\n", __FUNCTION__);
	CLEAR_CBUS_REG_MASK(VENC_VDAC_SETTING, enable_mask&0x1f);
	return 0;
}

int  video_dac_disable(void)
{
	debug("%s\n", __FUNCTION__);
	SET_CBUS_REG_MASK(VENC_VDAC_SETTING, 0x1f);
    return 0;    
}   

#define H_ACTIVE		1024
#define V_ACTIVE		600
#define H_PERIOD		1344
#define V_PERIOD		635
#define VIDEO_ON_PIXEL  80
#define VIDEO_ON_LINE   22

// Define LVDS physical PREM SWING VCM REF
static Lvds_Phy_Control_t lcd_lvds_phy_control = 
{
    .lvds_prem_ctl = 0x0,	
    .lvds_swing_ctl = 0x4,	  
    .lvds_vcm_ctl = 0x7,	
    .lvds_ref_ctl = 0x15, 	
};

//Define LVDS data mapping, pn swap.
static Lvds_Config_t lcd_lvds_config=
{
	.lvds_repack=1,   //data mapping  //0:JEDIA mode, 1:VESA mode
	.pn_swap=0,		  //0:normal, 1:swap	
};

Lcd_Config_t lcd_config = 
{
    .lcd_basic = {
        .h_active = H_ACTIVE,
        .v_active = V_ACTIVE,
        .h_period = H_PERIOD,
        .v_period = V_PERIOD,
    	.screen_ratio_width = 16,
     	.screen_ratio_height = 9,
        .lcd_type = LCD_DIGITAL_LVDS,    //LCD_DIGITAL_TTL  //LCD_DIGITAL_LVDS  //LCD_DIGITAL_MINILVDS
        .lcd_bits = 8,  //8  //6
    },
	
	.lcd_timing = {
        .pll_ctrl = 0x10232,  //42.9M, 50.2Hz
        .div_ctrl = 0x18813,  
        .clk_ctrl = 0x1111, //[19:16]ss_ctrl, [12]pll_sel, [8]div_sel, [4]vclk_sel, [3:0]xd
        //.sync_duration_num = 502,	//this parameter is auto calculated in display driver
        //.sync_duration_den = 10,
  
		.video_on_pixel = VIDEO_ON_PIXEL,
		.video_on_line = VIDEO_ON_LINE,
		
		.sth1_hs_addr = 10,
		.sth1_he_addr = 20,
		.sth1_vs_addr = 0,
		.sth1_ve_addr = V_PERIOD - 1,
		.stv1_hs_addr = 10,
		.stv1_he_addr = 20,
		.stv1_vs_addr = 2,
		.stv1_ve_addr = 4,
		
		.pol_cntl_addr = (0x0 << LCD_CPH1_POL) |(0x1 << LCD_HS_POL) | (0x1 << LCD_VS_POL),
		.inv_cnt_addr = (0<<LCD_INV_EN) | (0<<LCD_INV_CNT),
		.tcon_misc_sel_addr = (1<<LCD_STV1_SEL) | (1<<LCD_STV2_SEL),
		.dual_port_cntl_addr = (1<<LCD_TTL_SEL) | (1<<LCD_ANALOG_SEL_CPH3) | (1<<LCD_ANALOG_3PHI_CLK_SEL) | (0<<LCD_RGB_SWP) | (0<<LCD_BIT_SWP),
    },
	
	.lcd_effect = {
        .gamma_cntl_port = (1 << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
        .gamma_vcom_hswitch_addr = 0,
        .rgb_base_addr = 0xf0,
        .rgb_coeff_addr = 0x74a,        
    },
	
    .lvds_mlvds_config = {
        .lvds_config = &lcd_lvds_config,
		.lvds_phy_control = &lcd_lvds_phy_control,
    },

	//.lcd_power_ctrl = {	
	    // .power_on=lcd_power_on,
	    // .power_off=lcd_power_off,
	    // .backlight_on = power_on_backlight,
	    // .backlight_off = power_off_backlight,
		// .get_bl_level = get_backlight_level,
	    // .set_bl_level = set_backlight_level,
	//},
};

static void lcd_setup_gamma_table(Lcd_Config_t *pConf)
{
    int i;
	debug("%s\n", __FUNCTION__);
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

    for (i=0; i<256; i++) {
        pConf->lcd_effect.GammaTableR[i] = gamma_adjust[i] << 2;
        pConf->lcd_effect.GammaTableG[i] = gamma_adjust[i] << 2;
        pConf->lcd_effect.GammaTableB[i] = gamma_adjust[i] << 2;
    }
}

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

static void power_on_backlight(void)
{
	//debug("%s\n", __FUNCTION__);
	backlight_power_ctrl(ON);
}

static void power_off_backlight(void)
{
	//debug("%s\n", __FUNCTION__);
	backlight_power_ctrl(OFF);
}

static void lcd_power_on(void)
{
	debug("%s\n", __FUNCTION__);
	video_dac_disable();    
	lcd_power_ctrl(ON);       
}
static void lcd_power_off(void)
{
	debug("%s\n", __FUNCTION__);	
	backlight_power_ctrl(OFF);    
	lcd_power_ctrl(OFF);
}

static void lcd_io_init(void)
{
    debug("%s\n", __FUNCTION__);    
    
	//lcd_power_ctrl(ON);     
    //set_backlight_level(DEFAULT_BL_LEVEL);
}

static int lcd_enable(void)
{
	debug("%s\n", __FUNCTION__);

	panel_info.vd_base = simple_strtoul(getenv("fb_addr"), NULL, NULL);
	panel_info.vl_col = simple_strtoul(getenv("display_width"), NULL, NULL);
	panel_info.vl_row = simple_strtoul(getenv("display_height"), NULL, NULL);
	panel_info.vl_bpix = simple_strtoul(getenv("display_bpp"), NULL, NULL);
	panel_info.vd_color_fg = simple_strtoul(getenv("display_color_fg"), NULL, NULL);
	panel_info.vd_color_bg = simple_strtoul(getenv("display_color_bg"), NULL, NULL);

	lcd_setup_gamma_table(&lcd_config);
	lcd_video_adjust(&lcd_config);
    lcd_io_init();
    lcd_probe();

    return 0;
}

void lcd_disable(void)
{
	debug("%s\n", __FUNCTION__);
	backlight_power_ctrl(OFF);   
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
	.disable=	lcd_disable,
	.bl_on	=	power_on_backlight,
	.bl_off	=	power_off_backlight,
	.set_bl_level =	set_backlight_level,
	.get_bl_level = get_backlight_level,	
	.power_on=lcd_power_on,
    .power_off=lcd_power_off,
};
