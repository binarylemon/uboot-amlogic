/*
 * AMLOGIC TCON controller driver.
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
#include <amlogic/aml_lcd.h>

/* for GAMMA_CNTL_PORT */
   /// GAMMA VCOM POL
   #define LCD_GAMMA_VCOM_POL       7
   /// GAMMA DATA REVERSE OUTPUT FOLLOWING VCOM
   #define LCD_GAMMA_RVS_OUT        6
   /// GAMMA ADDR PORT IS RDY
   #define LCD_ADR_RDY              5
   /// GAMMA DATA PORT IS RDY to Write
   #define LCD_WR_RDY               4
   /// GAMMA DATA PORT IS RDY to Read
   #define LCD_RD_RDY               3
   /// RGB10-->RGB8 using Trancate or Round off
   #define LCD_GAMMA_TR             2
   #define LCD_GAMMA_SET            1
   #define LCD_GAMMA_EN             0

/* for GAMMA_ADDR_PORT */
   /// Host Read/Write
   #define LCD_H_RD                 12
   /// Burst Mode
   #define LCD_H_AUTO_INC           11
   #define LCD_H_SEL_R              10
   #define LCD_H_SEL_G              9
   #define LCD_H_SEL_B              8
   /// 7:0
   #define LCD_HADR_MSB             7
   /// 7:0
   #define LCD_HADR                 0

/* for POL_CNTL_ADDR */
   #define LCD_DCLK_SEL             14    //FOR DCLK OUTPUT
   #define LCD_TCON_VSYNC_SEL_DVI   11	 // FOR RGB format DVI output
   #define LCD_TCON_HSYNC_SEL_DVI   10	 // FOR RGB format DVI output
   #define LCD_TCON_DE_SEL_DVI      9	 // FOR RGB format DVI output
   #define LCD_CPH3_POL             8
   #define LCD_CPH2_POL             7
   #define LCD_CPH1_POL             6
   #define LCD_TCON_DE_SEL          5
   #define LCD_TCON_VS_SEL          4
   #define LCD_TCON_HS_SEL          3
   #define LCD_DE_POL               2
   #define LCD_VS_POL               1
   #define LCD_HS_POL               0

/* for DITH_CNTL_ADDR */
   #define LCD_DITH10_EN            10
   #define LCD_DITH8_EN             9
   #define LCD_DITH_MD              8
   /// 7:4
   #define LCD_DITH10_CNTL_MSB      7
   /// 7:4
   #define LCD_DITH10_CNTL          4
   /// 3:0
   #define LCD_DITH8_CNTL_MSB       3
   /// 3:0
   #define LCD_DITH8_CNTL           0

/* for INV_CNT_ADDR */
   #define LCD_INV_EN               4
   #define LCD_INV_CNT_MSB          3
   #define LCD_INV_CNT              0

/* for TCON_MISC_SEL_ADDR */
   #define LCD_STH2_SEL             12
   #define LCD_STH1_SEL             11
   #define LCD_OEH_SEL              10
   #define LCD_VCOM_SEL             9
   #define LCD_DB_LINE_SW           8
   #define LCD_CPV2_SEL             7
   #define LCD_CPV1_SEL             6
   #define LCD_STV2_SEL             5
   #define LCD_STV1_SEL             4
   #define LCD_OEV_UNITE            3
   #define LCD_OEV3_SEL             2
   #define LCD_OEV2_SEL             1
   #define LCD_OEV1_SEL             0

/* for DUAL_PORT_CNTL_ADDR */
   #define LCD_ANALOG_SEL_CPH3      8
   #define LCD_ANALOG_3PHI_CLK_SEL  7
   #define LCD_LVDS_SEL54           6
   #define LCD_LVDS_SEL27           5
   #define LCD_TTL_SEL              4
   #define LCD_DUAL_PIXEL           3
   #define LCD_PORT_SWP             2
   #define LCD_RGB_SWP              1
   #define LCD_BIT_SWP              0

/* for LVDS_PACK_CNTL_ADDR */
   #define LCD_LD_CNT_MSB           7
   #define LCD_LD_CNT               5
   #define LCD_PN_SWP               4
   #define LCD_RES                  3
   #define LCD_LVDS_PORT_SWP        2
   #define LCD_PACK_RVS             1
   #define LCD_PACK_LITTLE          0   
   
/* for video encoder */
   #define	LVDS_DELAY				0
   #define	TTL_DELAY				19

   #define	MLVDS_DELAY				0
   
//********************************************//
// for clk parameter auto generation
//********************************************//
//**** clk parameters bit ***/
	#define PLL_CTRL_LOCK			31
	#define PLL_CTRL_PD				30
	#define	PLL_CTRL_OD				16
	#define	PLL_CTRL_N				9
	#define	PLL_CTRL_M				0	//[8:0]
	
	#define DIV_CTRL_DIV_POST		12	//[14:12]
	#define DIV_CTRL_LVDS_CLK_EN	11
	#define DIV_CTRL_PHY_CLK_DIV2	10
	#define DIV_CTRL_POST_SEL		8	//[9:8]
	#define	DIV_CTRL_DIV_PRE		4	//[6:4]

	#define	CLK_TEST_FLAG			31
	#define	CLK_CTRL_AUTO			30
	//#define	CLK_CTRL_PLL_SEL		10
	//#define	CLK_CTRL_DIV_SEL		9
	#define	CLK_CTRL_VCLK_SEL		8
	#define	CLK_CTRL_SS				4	//[7:4]
	#define	CLK_CTRL_XD				0	//[3:0]

	#define PLL_WAIT_LOCK_CNT		100
//**** clk frequency limit ***/
	/* PLL */
	#define FIN_FREQ				(24 * 1000)
	#define PLL_M_MIN				2
	#define PLL_M_MAX				100
	#define PLL_N_MIN				1
	#define PLL_N_MAX				1
	
	#define PLL_FREF_MIN			(5 * 1000)
	#define PLL_FREF_MAX			(30 * 1000)
	#define PLL_VCO_MIN				(750 * 1000)
	#define PLL_VCO_MAX				(1500 * 1000)

	/* VID_DIV */
	#define DIV_PRE_MAX_CLK_IN		(1300 * 1000)
	#define DIV_POST_MAX_CLK_IN		(800 * 1000)

	/* CRT_VIDEO */
	#define CRT_VID_MAX_CLK_IN		(600 * 1000)
	/* ENCL */
	#define LCD_VENC_MAX_CLK_IN		(208 * 1000)
	/* lcd interface video clk */
	#define LVDS_MAX_VID_CLK_IN		LCD_VENC_MAX_CLK_IN
	#define TTL_MAX_VID_CLK_IN		LCD_VENC_MAX_CLK_IN
	#define MLVDS_MAX_VID_CLK_IN	LCD_VENC_MAX_CLK_IN
	/* clk max error */
	#define MAX_ERROR				(5 * 1000)
	
#define CRT_VID_DIV_MAX				15
#define OD_SEL_MAX					2
#define DIV_PRE_SEL_MAX				6
	
static const unsigned od_table[OD_SEL_MAX] = {1,2};
static const unsigned div_pre_table[DIV_PRE_SEL_MAX] = {1,2,3,4,5,6};
//********************************************//

/* for lcd power on/off config */
typedef enum
{
    LCD_POWER_TYPE_CPU = 0,
    LCD_POWER_TYPE_PMU,
	LCD_POWER_TYPE_SIGNAL,
	LCD_POWER_TYPE_INITIAL,
    LCD_POWER_TYPE_MAX,
} Lcd_Power_Type_t;
#define LCD_POWER_TYPE_NULL			LCD_POWER_TYPE_MAX

typedef enum
{
	LCD_POWER_PMU_GPIO0 = 0,
	LCD_POWER_PMU_GPIO1,
	LCD_POWER_PMU_GPIO2,
	LCD_POWER_PMU_GPIO3,
	LCD_POWER_PMU_GPIO4,
	LCD_POWER_PMU_GPIO_MAX,
} Lcd_Power_Pmu_Gpio_t;

#define	LCD_POWER_GPIO_OUTPUT_LOW	0
#define	LCD_POWER_GPIO_OUTPUT_HIGH	1
#define	LCD_POWER_GPIO_INPUT		2

static const char* lcd_power_type_table[]={
	"cpu",
	"pmu",
	"signal",
	"init",
	"null",
};

static const char* lcd_power_pmu_gpio_table[]={
	"GPIO0",
	"GPIO1",
	"GPIO2",
	"GPIO3",
	"GPIO4",
	"null",
}; 

typedef enum
{
	//LCD_DIGITAL_MIPI = 0,
	LCD_DIGITAL_LVDS = 1,
	//LCD_DIGITAL_EDP = 2,
	LCD_DIGITAL_TTL = 3,
	LCD_DIGITAL_MINILVDS = 4,
	LCD_TYPE_MAX,
} Lcd_Type_t;

static const char* lcd_type_table[]={
	"MIPI",	
	"LVDS",
	"eDP",
	"TTL",
	"miniLVDS",
	"invalid",
};

static const char* lcd_type_table_match[]={
	"mipi",
	"lvds",
	"edp",
	"ttl",
	"minilvds",
	"invalid",
};

#define SS_LEVEL_MAX	7
static const char *lcd_ss_level_table[]={
	"0",
	"0.5%",
	"1%",
	"2%",
	"3%",
	"4%",
	"5%",
};

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

    Lcd_Type_t lcd_type;  // only support 3 kinds of digital panel, not include analog I/F
    u16 lcd_bits;         // 6 or 8 bits
	u16 lcd_bits_option;  //option=0, means the panel only support one lcd_bits option
}Lcd_Basic_t;

typedef struct {
    u32 pll_ctrl;        /* video PLL settings */
    u32 div_ctrl;        /* video pll div settings */
	u32 clk_ctrl;        /* video clock settings */  //[20]clk_auto, [19:16]ss_ctrl, [12]pll_sel, [8]div_sel, [4]vclk_sel, [3:0]xd
	u32 lcd_clk;		/* lcd clock*/
    u16 sync_duration_num;
    u16 sync_duration_den;
	
	u16 video_on_pixel;
    u16 video_on_line;
	
	u16 hsync_width;
	u16 hsync_bp;
	u16 vsync_width;
	u16 vsync_bp;
	u16 hvsync_valid;
	u16 de_hstart;
	u16 de_vstart;
	u16 de_valid;
	u32 h_offset;
	u32 v_offset;
  u32 vsync_h_phase; //[31]sign [15:0]value
    u16 sth1_hs_addr;
    u16 sth1_he_addr;
    u16 sth1_vs_addr;
    u16 sth1_ve_addr;

    u16 oeh_hs_addr;
    u16 oeh_he_addr;
    u16 oeh_vs_addr;
    u16 oeh_ve_addr;

    //u16 vcom_hswitch_addr;
    //u16 vcom_vs_addr;
    //u16 vcom_ve_addr;

    //u16 cpv1_hs_addr;
    //u16 cpv1_he_addr;
    //u16 cpv1_vs_addr;
    //u16 cpv1_ve_addr;

    u16 stv1_hs_addr;
    u16 stv1_he_addr;
    u16 stv1_vs_addr;
    u16 stv1_ve_addr;

    //u16 oev1_hs_addr;
    //u16 oev1_he_addr;
    //u16 oev1_vs_addr;
    //u16 oev1_ve_addr;

    u16 pol_cntl_addr;
    u16 inv_cnt_addr;
    u16 tcon_misc_sel_addr;
} Lcd_Timing_t;

// Fine Effect Tune
typedef struct {
    u16 gamma_cntl_port;
    u16 gamma_vcom_hswitch_addr;

    u16 rgb_base_addr;
    u16 rgb_coeff_addr;
	u16 dith_user;
    u16 dith_cntl_addr;

	u32 vadj_brightness;
	u32 vadj_contrast;
	u32 vadj_saturation;
	
	unsigned char gamma_revert;
	u16 gamma_r_coeff;
	u16 gamma_g_coeff;
	u16 gamma_b_coeff;
    u16 GammaTableR[256];
    u16 GammaTableG[256];
    u16 GammaTableB[256];
} Lcd_Effect_t;

typedef struct {
	unsigned lvds_vswing;
	unsigned lvds_repack_user;
	unsigned lvds_repack;
	unsigned pn_swap;
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
	LVDS_Config_t *lvds_config;
	TTL_Config_t *ttl_config;
	MLVDS_Config_t *mlvds_config;
	MLVDS_Tcon_Config_t *mlvds_tcon_config;	//Point to TCON0~7
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
} Lcd_Power_Ctrl_t;

typedef struct {
    Lcd_Basic_t lcd_basic;
    Lcd_Timing_t lcd_timing;
    Lcd_Effect_t lcd_effect;
	Lcd_Control_Config_t lcd_control;
	Lcd_Power_Ctrl_t lcd_power_ctrl;
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
	BL_CTL_PWM_NEGATIVE,
	BL_CTL_PWM_POSITIVE,
	BL_CTL_PWM_COMBO,
	BL_CTL_MAX,
} BL_Ctrl_Method_t;

static const char* bl_ctrl_method_table[]={
	"gpio",
	"pwm_negative",
	"pwm_positive",
	"pwm_combo",
	"null"
};

typedef enum {
	BL_PWM_A = 0,
	BL_PWM_B,
	BL_PWM_C,
	BL_PWM_D,
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

typedef enum {
	GPIOZ_0=0,
	GPIOZ_1=1,
	GPIOZ_2=2,
	GPIOZ_3=3,
	GPIOZ_4=4,
	GPIOZ_5=5,
	GPIOZ_6=6,
	GPIOZ_7=7,
	GPIOZ_8=8,
	GPIOZ_9=9,
	GPIOZ_10=10,
	GPIOZ_11=11,
	GPIOZ_12=12,
	GPIOE_0=13,
	GPIOE_1=14,
	GPIOE_2=15,
	GPIOE_3=16,
	GPIOE_4=17,
	GPIOE_5=18,
	GPIOE_6=19,
	GPIOE_7=20,
	GPIOE_8=21,
	GPIOE_9=22,
	GPIOE_10=23,
	GPIOE_11=24,
	GPIOY_0=25,
	GPIOY_1=26,
	GPIOY_2=27,
	GPIOY_3=28,
	GPIOY_4=29,
	GPIOY_5=30,
	GPIOY_6=31,
	GPIOY_7=32,
	GPIOY_8=33,
	GPIOY_9=34,
	GPIOY_10=35,
	GPIOY_11=36,
	GPIOY_12=37,
	GPIOY_13=38,
	GPIOY_14=39,
	GPIOY_15=40,
	GPIOX_0=41,
	GPIOX_1=42,
	GPIOX_2=43,
	GPIOX_3=44,
	GPIOX_4=45,
	GPIOX_5=46,
	GPIOX_6=47,
	GPIOX_7=48,
	GPIOX_8=49,
	GPIOX_9=50,
	GPIOX_10=51,
	GPIOX_11=52,
	GPIOX_12=53,
	GPIOX_13=54,
	GPIOX_14=55,
	GPIOX_15=56,
	GPIOX_16=57,
	GPIOX_17=58,
	GPIOX_18=59,
	GPIOX_19=60,
	GPIOX_20=61,
	GPIOX_21=62,
	GPIOX_22=63,
	GPIOX_23=64,
	GPIOX_24=65,
	GPIOX_25=66,
	GPIOX_26=67,
	GPIOX_27=68,
	GPIOX_28=69,
	GPIOX_29=70,
	GPIOX_30=71,
	GPIOX_31=72,
	GPIOX_32=73,
	GPIOX_33=74,
	GPIOX_34=75,
	GPIOX_35=76,
	BOOT_0=77,
	BOOT_1=78,
	BOOT_2=79,
	BOOT_3=80,
	BOOT_4=81,
	BOOT_5=82,
	BOOT_6=83,
	BOOT_7=84,
	BOOT_8=85,
	BOOT_9=86,
	BOOT_10=87,
	BOOT_11=88,
	BOOT_12=89,
	BOOT_13=90,
	BOOT_14=91,
	BOOT_15=92,
	BOOT_16=93,
	BOOT_17=94,
	GPIOD_0=95,
	GPIOD_1=96,
	GPIOD_2=97,
	GPIOD_3=98,
	GPIOD_4=99,
	GPIOD_5=100,
	GPIOD_6=101,
	GPIOD_7=102,
	GPIOD_8=103,
	GPIOD_9=104,
	GPIOC_0=105,
	GPIOC_1=106,
	GPIOC_2=107,
	GPIOC_3=108,
	GPIOC_4=109,
	GPIOC_5=110,
	GPIOC_6=111,
	GPIOC_7=112,
	GPIOC_8=113,
	GPIOC_9=114,
	GPIOC_10=115,
	GPIOC_11=116,
	GPIOC_12=117,
	GPIOC_13=118,
	GPIOC_14=119,
	GPIOC_15=120,
	CARD_0=121,
	CARD_1=122,
	CARD_2=123,
	CARD_3=124,
	CARD_4=125,
	CARD_5=126,
	CARD_6=127,
	CARD_7=128,
	CARD_8=129,
	GPIOB_0=130,
	GPIOB_1=131,
	GPIOB_2=132,
	GPIOB_3=133,
	GPIOB_4=134,
	GPIOB_5=135,
	GPIOB_6=136,
	GPIOB_7=137,
	GPIOB_8=138,
	GPIOB_9=139,
	GPIOB_10=140,
	GPIOB_11=141,
	GPIOB_12=142,
	GPIOB_13=143,
	GPIOB_14=144,
	GPIOB_15=145,
	GPIOB_16=146,
	GPIOB_17=147,
	GPIOB_18=148,
	GPIOB_19=149,
	GPIOB_20=150,
	GPIOB_21=151,
	GPIOB_22=152,
	GPIOB_23=153,
	GPIOA_0=154,
	GPIOA_1=155,
	GPIOA_2=156,
	GPIOA_3=157,
	GPIOA_4=158,
	GPIOA_5=159,
	GPIOA_6=160,
	GPIOA_7=161,
	GPIOA_8=162,
	GPIOA_9=163,
	GPIOA_10=164,
	GPIOA_11=165,
	GPIOA_12=166,
	GPIOA_13=167,
	GPIOA_14=168,
	GPIOA_15=169,
	GPIOA_16=170,
	GPIOA_17=171,
	GPIOA_18=172,
	GPIOA_19=173,
	GPIOA_20=174,
	GPIOA_21=175,
	GPIOA_22=176,
	GPIOA_23=177,
	GPIOA_24=178,
	GPIOA_25=179,
	GPIOA_26=180,
	GPIOA_27=181,
	GPIOAO_0=182,
	GPIOAO_1=183,
	GPIOAO_2=184,
	GPIOAO_3=185,
	GPIOAO_4=186,
	GPIOAO_5=187,
	GPIOAO_6=188,
	GPIOAO_7=189,
	GPIOAO_8=190,
	GPIOAO_9=191,
	GPIOAO_10=192,
	GPIOAO_11=193,
	GPIO_MAX=194,
}gpio_t;

static const char* amlogic_gpio_type_table[]={
	"GPIOZ_0",
	"GPIOZ_1",
	"GPIOZ_2",
	"GPIOZ_3",
	"GPIOZ_4",
	"GPIOZ_5",
	"GPIOZ_6",
	"GPIOZ_7",
	"GPIOZ_8",
	"GPIOZ_9",
	"GPIOZ_10",
	"GPIOZ_11",
	"GPIOZ_12",
	"GPIOE_0",
	"GPIOE_1",
	"GPIOE_2",
	"GPIOE_3",
	"GPIOE_4",
	"GPIOE_5",
	"GPIOE_6",
	"GPIOE_7",
	"GPIOE_8",
	"GPIOE_9",
	"GPIOE_10",
	"GPIOE_11",
	"GPIOY_0",
	"GPIOY_1",
	"GPIOY_2",
	"GPIOY_3",
	"GPIOY_4",
	"GPIOY_5",
	"GPIOY_6",
	"GPIOY_7",
	"GPIOY_8",
	"GPIOY_9",
	"GPIOY_10",
	"GPIOY_11",
	"GPIOY_12",
	"GPIOY_13",
	"GPIOY_14",
	"GPIOY_15",
	"GPIOX_0",
	"GPIOX_1",
	"GPIOX_2",
	"GPIOX_3",
	"GPIOX_4",
	"GPIOX_5",
	"GPIOX_6",
	"GPIOX_7",
	"GPIOX_8",
	"GPIOX_9",
	"GPIOX_10",
	"GPIOX_11",
	"GPIOX_12",
	"GPIOX_13",
	"GPIOX_14",
	"GPIOX_15",
	"GPIOX_16",
	"GPIOX_17",
	"GPIOX_18",
	"GPIOX_19",
	"GPIOX_20",
	"GPIOX_21",
	"GPIOX_22",
	"GPIOX_23",
	"GPIOX_24",
	"GPIOX_25",
	"GPIOX_26",
	"GPIOX_27",
	"GPIOX_28",
	"GPIOX_29",
	"GPIOX_30",
	"GPIOX_31",
	"GPIOX_32",
	"GPIOX_33",
	"GPIOX_34",
	"GPIOX_35",
	"BOOT_0",
	"BOOT_1",
	"BOOT_2",
	"BOOT_3",
	"BOOT_4",
	"BOOT_5",
	"BOOT_6",
	"BOOT_7",
	"BOOT_8",
	"BOOT_9",
	"BOOT_10",
	"BOOT_11",
	"BOOT_12",
	"BOOT_13",
	"BOOT_14",
	"BOOT_15",
	"BOOT_16",
	"BOOT_17",
	"GPIOD_0",
	"GPIOD_1",
	"GPIOD_2",
	"GPIOD_3",
	"GPIOD_4",
	"GPIOD_5",
	"GPIOD_6",
	"GPIOD_7",
	"GPIOD_8",
	"GPIOD_9",
	"GPIOC_0",
	"GPIOC_1",
	"GPIOC_2",
	"GPIOC_3",
	"GPIOC_4",
	"GPIOC_5",
	"GPIOC_6",
	"GPIOC_7",
	"GPIOC_8",
	"GPIOC_9",
	"GPIOC_10",
	"GPIOC_11",
	"GPIOC_12",
	"GPIOC_13",
	"GPIOC_14",
	"GPIOC_15",
	"CARD_0",
	"CARD_1",
	"CARD_2",
	"CARD_3",
	"CARD_4",
	"CARD_5",
	"CARD_6",
	"CARD_7",
	"CARD_8",
	"GPIOB_0",
	"GPIOB_1",
	"GPIOB_2",
	"GPIOB_3",
	"GPIOB_4",
	"GPIOB_5",
	"GPIOB_6",
	"GPIOB_7",
	"GPIOB_8",
	"GPIOB_9",
	"GPIOB_10",
	"GPIOB_11",
	"GPIOB_12",
	"GPIOB_13",
	"GPIOB_14",
	"GPIOB_15",
	"GPIOB_16",
	"GPIOB_17",
	"GPIOB_18",
	"GPIOB_19",
	"GPIOB_20",
	"GPIOB_21",
	"GPIOB_22",
	"GPIOB_23",
	"GPIOA_0",
	"GPIOA_1",
	"GPIOA_2",
	"GPIOA_3",
	"GPIOA_4",
	"GPIOA_5",
	"GPIOA_6",
	"GPIOA_7",
	"GPIOA_8",
	"GPIOA_9",
	"GPIOA_10",
	"GPIOA_11",
	"GPIOA_12",
	"GPIOA_13",
	"GPIOA_14",
	"GPIOA_15",
	"GPIOA_16",
	"GPIOA_17",
	"GPIOA_18",
	"GPIOA_19",
	"GPIOA_20",
	"GPIOA_21",
	"GPIOA_22",
	"GPIOA_23",
	"GPIOA_24",
	"GPIOA_25",
	"GPIOA_26",
	"GPIOA_27",
	"GPIOAO_0",
	"GPIOAO_1",
	"GPIOAO_2",
	"GPIOAO_3",
	"GPIOAO_4",
	"GPIOAO_5",
	"GPIOAO_6",
	"GPIOAO_7",
	"GPIOAO_8",
	"GPIOAO_9",
	"GPIOAO_10",
	"GPIOAO_11",
	"GPIO_MAX",
}; 

#endif /* LCDOUTC_H */
