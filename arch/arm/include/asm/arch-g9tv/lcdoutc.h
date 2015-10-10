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
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef LCDOUTC_H
#define LCDOUTC_H

#include <common.h>
#include <linux/list.h>
#include <amlogic/aml_lcd.h>
#ifdef CONFIG_AML_LCD_EXTERN
#include <amlogic/aml_lcd_extern.h>
#endif

//**********************************
//lcd driver version
//**********************************
#define LCD_DRV_TYPE      "g9"
#define LCD_DRV_DATE      "20151009"
//**********************************

typedef enum {
	LCD_DIGITAL_LVDS = 0,
	LCD_DIGITAL_VBYONE,
	LCD_DIGITAL_TTL,
	LCD_TYPE_MAX,
} Lcd_Type_t;

typedef struct {
	char model_name[30];
	Lcd_Type_t lcd_type;	// LCD_DIGITAL_TTL /LCD_DIGITAL_LVDS/LCD_DIGITAL_VBYONE
	u16 h_active;		// Horizontal display area
	u16 v_active;		// Vertical display area
	u16 h_period;		// Horizontal total period time
	u16 v_period;		// Vertical total period time
	u16 video_on_pixel;
	u16 video_on_line;

	u16 screen_ratio_width;		// screen aspect ratio width
	u16 screen_ratio_height;	// screen aspect ratio height
	u32 screen_actual_width;	/* screen physical width in "mm" unit */
	u32 screen_actual_height;	/* screen physical height in "mm" unit */
}Lcd_Basic_t;

typedef struct {
	u32 hpll_clk;
	u32 hpll_od;
	u32 hdmi_pll_cntl5;

	unsigned char clk_auto; /* clk parameters auto generation flag */
	u32 lcd_clk;		/* lcd clock = pixel clock*/
	u32 pll_ctrl;		/* video PLL settings */
	u32 div_ctrl;		/* video pll div settings */
	u32 clk_ctrl;
	unsigned char frame_rate_adj_type; /* 0=htotal adjust, 1=clock adjust */
	unsigned char ss_level;

	u16 sync_duration_num;
	u16 sync_duration_den;

	u16 hsync_width;
	u16 hsync_bp;
	u16 vsync_width;
	u16 vsync_bp;

	u16 sth1_hs_addr;
	u16 sth1_he_addr;
	u16 sth1_vs_addr;
	u16 sth1_ve_addr;

	u16 stv1_hs_addr;
	u16 stv1_he_addr;
	u16 stv1_vs_addr;
	u16 stv1_ve_addr;
} Lcd_Timing_t;

typedef struct {
	unsigned int lvds_bits;		// 6 / 8 /10  bits
	unsigned int lvds_repack;
	unsigned int pn_swap;
	unsigned int dual_port;
	unsigned int port_swap;
	unsigned int port_sel;
	/* for old version */
	unsigned int port_reverse;
	unsigned int lvds_fifo_wr_mode;
} Lvds_Config_t;

typedef struct {
	unsigned int lane_count;
	unsigned int byte;
	unsigned int region;
	unsigned int color_fmt;
	unsigned int phy_div;
	unsigned int bit_rate;
} Vbyone_Config_t;

#ifdef CONFIG_AML_LCD_EXTERN
struct lcd_extern_config_s {
	unsigned int index;
	unsigned int on_delay;
	unsigned int off_delay;
};
#endif

typedef struct {
	Lvds_Config_t *lvds_config;
	Vbyone_Config_t *vbyone_config;
#ifdef CONFIG_AML_LCD_EXTERN
	struct lcd_extern_config_s *ext_config;
#endif
} Lcd_Control_Config_t;

//****panel power control only for uboot ***//
typedef struct {
	unsigned int gpio;
	unsigned short on_value;
	unsigned short off_value;
	unsigned short panel_on_delay;
	unsigned short panel_off_delay;
} Panel_Power_Config_t;

// Power Control
typedef struct {
	Panel_Power_Config_t *panel_power;
} Lcd_Power_Ctrl_t;

typedef struct {
	unsigned char version; /* for driver version compatibility */
	Lcd_Basic_t lcd_basic;
	Lcd_Timing_t lcd_timing;
	Lcd_Control_Config_t lcd_control;
	Lcd_Power_Ctrl_t lcd_power_ctrl;
} Lcd_Config_t;

Lcd_Config_t lcd_config_dft;


//===============backlight control config===================//
//****Backlight pwm control only for uboot ***//
typedef enum {
    OFF = 0,
    ON = 1,
} Bool_t;

typedef enum {
	BL_PWM_A = 0,
	BL_PWM_B,
	BL_PWM_C,
	BL_PWM_D,
	BL_PWM_E,
	BL_PWM_F,
	BL_PWM_VS,
	BL_PWM_MAX,
} BL_PWM_t;

#define XTAL_FREQ_HZ		(24*1000*1000) /* 24M in HZ */
#define XTAL_HALF_FREQ_HZ	(24*1000*500)  /* 24M/2 in HZ */

#define AML_BL_FREQ_DEF			1000	/* unit: HZ */
#define AML_BL_FREQ_VS_DEF		2 /* multiple 2 of vfreq */

//****backlight pwm control only for uboot ***//
typedef struct {
	unsigned pwm_freq;	/* pwm_vs: 1~4(vfreq multiple), other pwm: real freq(unit: Hz) */
	unsigned pwm_duty_max;	/** brightness diminig duty_max(unit: %, positive logic) */
	unsigned pwm_duty_min; 	/** brightness diminig duty_min(unit: %, positive logic) */

	unsigned level_default;
	unsigned level_min;
	unsigned level_max;

	unsigned pwm_on_delay;
	unsigned pwm_off_delay;
	unsigned pwm_gpio;

	unsigned pwm_port;
	unsigned int pwm_cnt;
	unsigned int pre_div;
	unsigned pwm_max;
	unsigned pwm_min;
	unsigned pwm_positive;

	unsigned pinmux_set_num;
	unsigned pinmux_set[15][2];
	unsigned pinmux_clr_num;
	unsigned pinmux_clr[15][2];
} Bl_Pwm_Config_t;

//****backlight power control only for uboot ***//
typedef struct {
	unsigned int gpio;
	unsigned short on_value;
	unsigned short off_value;
	unsigned short bl_on_delay;
	unsigned short bl_off_delay;
} Bl_Power_Config_t;

typedef struct {
	Bl_Power_Config_t bl_power;
	Bl_Pwm_Config_t   bl_pwm;
} Lcd_Bl_Config_t;

Lcd_Bl_Config_t bl_config_dft;


//============lcd & backlight config=================//
typedef struct {
	Lcd_Config_t 	*pConf;
	Lcd_Bl_Config_t *bl_config;
} lcd_dev_t;

#define Rsv_val 0xffffffff
typedef struct {
	const char *panel_type;

	Lcd_Type_t lcd_type;		// LCD_DIGITAL_TTL /LCD_DIGITAL_LVDS/LCD_DIGITAL_VBYONE
	u16 h_active;
	u16 v_active;
	u16 h_period;
	u16 v_period;
	u16 video_on_pixel;
	u16 video_on_line;

	u32 hpll_clk;
	u32 hpll_od;
	u32 hdmi_pll_cntl5;

	/* timing value       //version 1       //version 0 */
	u16 timing_val_0;     //hs_width        //sth1_hs
	u16 timing_val_1;     //hs_backporch    //sth1_he
	u16 timing_val_2;     //vs_width        //sth1_vs
	u16 timing_val_3;     //vs_backporch    //sth1_ve
	u16 timing_val_4;     //none            //stv1_hs
	u16 timing_val_5;     //none            //stv1_he
	u16 timing_val_6;     //none            //stv1_vs
	u16 timing_val_7;     //none            //stv1_ve

	unsigned int customer_val_0; //fr_adjust_type
	unsigned int customer_val_1; //clk_auto_generate
	unsigned int customer_val_2; //ss_level
	unsigned int customer_val_3;
	unsigned int customer_val_4;
	unsigned int customer_val_5;
	unsigned int customer_val_6;
	unsigned int customer_val_7;
	unsigned int customer_val_8;
	unsigned int customer_val_9;

	int lcd_spc_val0;
	int lcd_spc_val1;
	int lcd_spc_val2;
	int lcd_spc_val3;
	int lcd_spc_val4;
	int lcd_spc_val5;
	int lcd_spc_val6;
	int lcd_spc_val7;
	int lcd_spc_val8;
	int lcd_spc_val9;
	int lcd_spc_val10;
	int lcd_spc_val11;
	int lcd_spc_val12;
	int lcd_spc_val13;
	int lcd_spc_val14;
	int lcd_spc_val15;

	unsigned int panel_gpio;
	unsigned short panel_on_value;
	unsigned short panel_off_value;
	unsigned short panel_on_delay;
	unsigned short panel_off_delay;

	int extern_index;
	int extern_on_delay;
	int extern_off_delay;

	unsigned int bl_gpio;
	unsigned short bl_on_value;
	unsigned short bl_off_value;
	unsigned short bl_on_delay;
	unsigned short bl_off_delay;

	unsigned pwm_port;
	unsigned pwm_freq;
	unsigned pwm_duty_max;
	unsigned pwm_duty_min;
	unsigned pwm_positive;

	unsigned level_default;
	unsigned level_min;
	unsigned level_max;

}Ext_Lcd_Config_t;

#define LCD_TYPE_MAX 	15


//#define __DBG__LCD__
#ifdef __DBG__LCD__
#define lcd_printf(fmt,args...) do { printf("[lcd]:FILE:%s:[%d],"fmt"",\
                                                 __FILE__,__LINE__,## args);} \
                                         while (0)
#else
#define lcd_printf(fmt,args...)
#endif


#endif /* LCDOUTC_H */
