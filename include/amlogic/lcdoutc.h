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
 * Modify:  Evoke Zhang <evoke.zhang@amlogic.com>
 */

#ifndef LCDOUTC_H
#define LCDOUTC_H

#include <common.h>
#include <linux/list.h>
#include <asm/arch/cpu.h>
#include <amlogic/aml_lcd.h>
#include <asm/arch/aml_lcd_gpio.h>

//**********************************************
//debug print define
//***********************************************
//#define LCD_DEBUG_INFO

extern unsigned int lcd_print_flag;
extern void lcd_print(const char *fmt, ...);
//**********************************************
//global define
//***********************************************
	#define FIN_FREQ                 (24 * 1000)
	
	//clk_ctrl
	#define CLK_CTRL_AUTO             31
	//#define CLK_CTRL_VCLK_SEL         30
	//#define CLK_CTRL_DIV_SEL          29
	//#define CLK_CTRL_PLL_SEL          28
	#define CLK_CTRL_RESERVED         12 //[27:12] //for specific CPU define
	#define CLK_CTRL_SS               8  //[11:8]
	#define CLK_CTRL_XD               0  //[7:0]

	//pol_ctrl
	#define POL_CTRL_CLK              6
	#define POL_CTRL_DE               2
	#define POL_CTRL_VS               1
	#define POL_CTRL_HS               0
	
	//gamma_ctrl
	#define GAMMA_CTRL_REVERSE        4
	#define GAMMA_CTRL_EN             0

typedef enum {
	GAMMA_SEL_R = 0,
	GAMMA_SEL_G,
	GAMMA_SEL_B,
} Lcd_Gamma_Sel_t;
//***************************************

typedef enum {
    LCD_POWER_TYPE_CPU = 0,
    LCD_POWER_TYPE_PMU,
    LCD_POWER_TYPE_SIGNAL,
    LCD_POWER_TYPE_INITIAL,
    LCD_POWER_TYPE_MAX,
} Lcd_Power_Type_t;
#define LCD_POWER_TYPE_NULL			LCD_POWER_TYPE_MAX

typedef enum {
	LCD_POWER_PMU_GPIO0 = 0,
	LCD_POWER_PMU_GPIO1,
	LCD_POWER_PMU_GPIO2,
	LCD_POWER_PMU_GPIO3,
	LCD_POWER_PMU_GPIO4,
	LCD_POWER_PMU_GPIO_MAX,
} Lcd_Power_Pmu_Gpio_t;

#define LCD_POWER_GPIO_OUTPUT_LOW	0
#define LCD_POWER_GPIO_OUTPUT_HIGH	1
#define LCD_POWER_GPIO_INPUT		2
#define LCD_GPIO_OUTPUT_LOW	0
#define LCD_GPIO_OUTPUT_HIGH	1
#define LCD_GPIO_INPUT		2

typedef enum {
	LCD_DIGITAL_MIPI = 0,
	LCD_DIGITAL_LVDS = 1,
	LCD_DIGITAL_EDP = 2,
	LCD_DIGITAL_TTL = 3,
	LCD_DIGITAL_MINILVDS = 4,
	LCD_TYPE_MAX,
} Lcd_Type_t;

#define PANEL_MODEL_DEFAULT	"Panel_Default"
typedef struct {
	char *model_name;
    u16 h_active;   	// Horizontal display area
    u16 v_active;     	// Vertical display area
    u16 h_period;       // Horizontal total period time
    u16 v_period;       // Vertical total period time
    u32 screen_ratio_width;      // screen aspect ratio width 
    u32 screen_ratio_height;     // screen aspect ratio height 
    u32 h_active_area;/* screen physical width in "mm" unit */
    u32 v_active_area;/* screen physical height in "mm" unit */

    Lcd_Type_t lcd_type;
    u16 lcd_bits;         // 6 or 8 bits
    u16 lcd_bits_option;  //option=0, means the panel only support one lcd_bits option
}Lcd_Basic_t;

typedef struct {
	u32 pll_ctrl;		/* video PLL settings */
	u32 div_ctrl;		/* video pll div settings */
	u32 clk_ctrl;		/* video clock settings */  //[31]clk_auto, [11:8]ss_ctrl, [7:0]xd
	u32 lcd_clk;		/* lcd clock*/
	u16 sync_duration_num;
	u16 sync_duration_den;
	
	u16 pol_ctrl;
	//u16 inv_cnt_addr;
	//u16 tcon_misc_sel_addr;
	
	u16 video_on_pixel;
	u16 video_on_line;
	
	u16 hsync_width;
	u16 hsync_bp;
	u16 vsync_width;
	u16 vsync_bp;
	u32 vsync_h_phase; //[31]sign [15:0]value
	u16 hvsync_valid;
	//u16 de_hstart;
	//u16 de_vstart;
	u16 de_valid;
	u32 h_offset;
	u32 v_offset;

	u16 de_hs_addr;
	u16 de_he_addr;
	u16 de_vs_addr;
	u16 de_ve_addr;

	u16 hs_hs_addr;
	u16 hs_he_addr;
	u16 hs_vs_addr;
	u16 hs_ve_addr;

	u16 vs_hs_addr;
	u16 vs_he_addr;
	u16 vs_vs_addr;
	u16 vs_ve_addr;
	
	u16 vso_hstart;
	u16 vso_vstart;
	u16 vso_user;
} Lcd_Timing_t;

// Fine Effect Tune
typedef struct {
	u32 rgb_base_addr;
	u32 rgb_coeff_addr;
	unsigned char dith_user;
	u32 dith_cntl_addr;

	u32 vadj_brightness;
	u32 vadj_contrast;
	u32 vadj_saturation;
	
	unsigned char gamma_ctrl;
	u16 gamma_r_coeff;
	u16 gamma_g_coeff;
	u16 gamma_b_coeff;
	u16 GammaTableR[256];
	u16 GammaTableG[256];
	u16 GammaTableB[256];
} Lcd_Effect_t;

//mipi-dsi config
#define DSI_CMD_CNT_INDEX         1 //byte[1]
#define DSI_INIT_ON_MAX           100
#define DSI_INIT_OFF_MAX          30

#define BIT_OPERATION_MODE_INIT   0
#define BIT_OPERATION_MODE_DISP   4
#define BIT_TRANS_CTRL_CLK        0
#define BIT_TRANS_CTRL_SWITCH     4 //[5:4]
typedef struct DSI_Config_s{
    unsigned char lane_num;
    unsigned int bit_rate_max;
    unsigned int bit_rate_min;
    unsigned int bit_rate;
    unsigned int factor_denominator;
    unsigned int factor_numerator;

    unsigned int venc_data_width;
    unsigned int dpi_data_format;
    unsigned int venc_fmt;
    unsigned int operation_mode;  //mipi-dsi operation mode: video, command. [4]display , [0]init
    unsigned int transfer_ctrl;  //[0]LP mode auto stop clk lane, [5:4]phy switch between LP and HS
    unsigned char video_mode_type;  //burst, non-burst(sync pulse, sync event)

    unsigned char *dsi_init_on;
    unsigned char *dsi_init_off;
    unsigned char lcd_extern_init;
}DSI_Config_t;

typedef struct {
	unsigned char max_lane_count;
	unsigned char link_user;
	unsigned char lane_count;
	unsigned char link_rate;
	unsigned char link_adaptive;
	unsigned char vswing;
	unsigned char preemphasis;
	unsigned int bit_rate;
	unsigned int sync_clock_mode;
	unsigned char edid_timing_used;
} EDP_Config_t;

typedef struct {
	unsigned lvds_vswing;
	unsigned lvds_repack_user;
	unsigned lvds_repack;
	unsigned pn_swap;
	unsigned dual_port;
	unsigned port_swap;
	unsigned port_sel;
} LVDS_Config_t;

typedef struct {
	unsigned char rb_swap;
	unsigned char bit_swap;
} TTL_Config_t;

typedef struct {
	int channel_num;
	int hv_sel;
	int tcon_1st_hs_addr;
	int tcon_1st_he_addr;
	int tcon_1st_vs_addr;
	int tcon_1st_ve_addr;
	int tcon_2nd_hs_addr;
	int tcon_2nd_he_addr;
	int tcon_2nd_vs_addr;
	int tcon_2nd_ve_addr;
} MLVDS_Tcon_Config_t;

typedef struct {
	unsigned *tcon_pinmux;
	unsigned *tcon_pinmux_pins;
	unsigned *tcon_gpio;
} MLVDS_Pinmux_t;

typedef struct {
	int mlvds_insert_start;
	int total_line_clk;
	int test_dual_gate;
	int test_pair_num;
	int phase_select;
	int TL080_phase;
	int scan_function;
	MLVDS_Pinmux_t *mlvds_pinmux;
} MLVDS_Config_t;

typedef struct {
	DSI_Config_t *mipi_config;
	EDP_Config_t *edp_config;
	LVDS_Config_t *lvds_config;
	TTL_Config_t *ttl_config;
	MLVDS_Config_t *mlvds_config;
	MLVDS_Tcon_Config_t *mlvds_tcon_config;
} Lcd_Control_Config_t;

typedef enum {
    OFF = 0,
    ON = 1,
} Bool_t;

// Power Control
typedef struct {
	unsigned char type;
	int gpio;
	unsigned short value;
	unsigned short delay;
} Lcd_Power_Config_t;

#define LCD_POWER_CTRL_STEP_MAX		15
typedef struct {
	Lcd_Power_Config_t power_on_uboot;
	Lcd_Power_Config_t power_off_uboot;
	Lcd_Power_Config_t power_on_config[LCD_POWER_CTRL_STEP_MAX];
	Lcd_Power_Config_t power_off_config[LCD_POWER_CTRL_STEP_MAX];
	int power_on_step;
	int power_off_step;
	int (*power_ctrl)(Bool_t status);
	void (*ports_ctrl)(Bool_t status);
	int (*power_ctrl_video)(Bool_t status);
} Lcd_Power_Ctrl_t;

typedef struct {
    void (*module_enable)(void);
    void (*module_disable)(void);
    void (*lcd_test)(unsigned num);
    void (*print_version)(void);
    void (*print_clk)(void);
    void (*edp_edid_load)(void);
} Lcd_Misc_Ctrl_t;

typedef struct {
    Lcd_Basic_t lcd_basic;
    Lcd_Timing_t lcd_timing;
    Lcd_Effect_t lcd_effect;
    Lcd_Control_Config_t lcd_control;
    Lcd_Power_Ctrl_t lcd_power_ctrl;
    Lcd_Misc_Ctrl_t lcd_misc_ctrl;
} Lcd_Config_t;

Lcd_Config_t lcd_config_dft;

//****************************************//
// backlight control
//****************************************//
#define BL_LEVEL_MAX_DFT   			255
#define BL_LEVEL_MIN_DFT   			10
#define BL_LEVEL_OFF				1

#define BL_LEVEL_MID_DFT    		128
#define BL_LEVEL_MID_MAPPED_DFT		102

#define BL_LEVEL_DFT				128

typedef enum {
	BL_CTL_GPIO = 0,
	BL_CTL_PWM_NEGATIVE = 1,
	BL_CTL_PWM_POSITIVE = 2,
	BL_CTL_PWM_COMBO = 3,
	BL_CTL_EXTERN = 4,
	BL_CTL_MAX = 5,
} BL_Ctrl_Method_t;

typedef enum {
	BL_PWM_A = 0,
	BL_PWM_B,
	BL_PWM_C,
	BL_PWM_D,
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON8
	BL_PWM_E,
	BL_PWM_F,
#endif
	BL_PWM_MAX,
} BL_PWM_t;

typedef struct {
	unsigned level_default;
	unsigned level_mid;
	unsigned level_mid_mapping;
	unsigned level_min;
	unsigned level_max;
	unsigned short power_on_delay;
	unsigned char method;
	int gpio;
	unsigned char gpio_on;
	unsigned char gpio_off;
	unsigned dim_max;
	unsigned dim_min;
	unsigned char pwm_port;
	unsigned char pwm_gpio_used;
	unsigned pwm_cnt;
	unsigned pwm_pre_div;
	unsigned pwm_max;
	unsigned pwm_min;
	
	unsigned combo_level_switch;
	unsigned char combo_high_port;
	unsigned char combo_high_method;
	unsigned char combo_low_port;
	unsigned char combo_low_method;
	unsigned combo_high_cnt;
	unsigned combo_high_pre_div;
	unsigned combo_high_duty_max;
	unsigned combo_high_duty_min;
	unsigned combo_low_cnt;
	unsigned combo_low_pre_div;
	unsigned combo_low_duty_max;
	unsigned combo_low_duty_min;
	
	unsigned pinmux_set_num;
	unsigned pinmux_set[5][2];
	unsigned pinmux_clr_num;
	unsigned pinmux_clr[5][2];
} Lcd_Bl_Config_t;

Lcd_Bl_Config_t bl_config_dft;
//*************************************//

extern void mdelay(unsigned long msec);

extern Lcd_Config_t* get_lcd_config(void);
extern void lcd_config_init(Lcd_Config_t *pConf);
extern void lcd_config_probe(Lcd_Config_t *pConf);
extern void lcd_config_remove(void);

#endif /* LCDOUTC_H */
