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
	#define	MIPI_DELAY				2
	#define	LVDS_DELAY				8
	#define	EDP_DELAY				8
	#define	TTL_DELAY				19

//********************************************//
// for clk parameter auto generation
//********************************************//
//**** clk parameters bit ***/
	#define PLL_CTRL_LOCK			31
	#define PLL_CTRL_EN				30
	#define PLL_CTRL_RST			29
	#define	PLL_CTRL_OD				9	//[10:9]
	#define	PLL_CTRL_N				24	//[28:24]
	#define	PLL_CTRL_M				0	//[8:0]

	#define	DIV_CTRL_EDP_DIV1		24	//[26:24]
	#define	DIV_CTRL_EDP_DIV0		20	//[23:20]
	#define DIV_CTRL_DIV_POST		12	//[14:12]
	#define DIV_CTRL_LVDS_CLK_EN	11
	#define DIV_CTRL_PHY_CLK_DIV2	10
	#define DIV_CTRL_POST_SEL		8	//[9:8]
	#define	DIV_CTRL_DIV_PRE		4	//[6:4]

	#define	CLK_TEST_FLAG			31
	#define	CLK_CTRL_AUTO			30
	#define	CLK_CTRL_FRAC			16	//[27:16]
	#define	CLK_CTRL_LEVEL			12	//[13:12]
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
	#define PLL_M_MAX				511
	#define PLL_N_MIN				1
	#define PLL_N_MAX				2
	
	#define PLL_FREF_MIN			(5 * 1000)
	#define PLL_FREF_MAX			(25 * 1000)
	#define PLL_VCO_MIN				(1200 * 1000)
	#define PLL_VCO_MAX				(3000 * 1000)
	/* MIPI-DSI PHY */
	#define MIPI_PHY_MAX_CLK_IN		(1000 * 1000)
	/* VID_DIV */
	#define DIV_PRE_MAX_CLK_IN		(1500 * 1000)
	#define DIV_POST_MAX_CLK_IN		(1000 * 1000)
	/* CRT_VIDEO */
	#define CRT_VID_MAX_CLK_IN		(600 * 1000)
	/* ENCL */
	#define ENCL_MAX_CLK_IN			(333 * 1000)
	/* lcd interface video clk */
	#define MIPI_MAX_VID_CLK_IN		ENCL_MAX_CLK_IN
	#define LVDS_MAX_VID_CLK_IN		ENCL_MAX_CLK_IN
	#define EDP_MAX_VID_CLK_IN		(235 * 1000)
	#define TTL_MAX_VID_CLK_IN		ENCL_MAX_CLK_IN
	/* clk max error */
	#define MAX_ERROR				(2 * 1000)

#define CRT_VID_DIV_MAX				15
#define OD_SEL_MAX					4
#define DIV_PRE_SEL_MAX				6
#define EDP_DIV0_SEL_MAX			15
#define EDP_DIV1_SEL_MAX			8

static const unsigned od_table[OD_SEL_MAX] = {1,2,4,8};
static const unsigned div_pre_table[DIV_PRE_SEL_MAX] = {1,2,3,4,5,6};
static const unsigned edp_div0_table[EDP_DIV0_SEL_MAX]={1,2,3,4,5,7,8,9,11,13,17,19,23,29,31};
static const unsigned edp_div1_table[EDP_DIV1_SEL_MAX]={1,2,4,5,6,7,9,13};
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
	LCD_DIGITAL_MIPI = 0,
	LCD_DIGITAL_LVDS = 1,
	LCD_DIGITAL_EDP = 2,
	LCD_DIGITAL_TTL = 3,
	//LCD_DIGITAL_MINILVDS = 4,
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

typedef struct DSI_Config_s{
        unsigned int    dsi_clk_div;
        unsigned int    dsi_clk_max;
        unsigned int    dsi_clk_min;
        unsigned int    dsi_cur_clk;
        unsigned int    pol_vs_hs_de;
        unsigned int    venc_color_type;
        unsigned int    dpi_color_type;
        unsigned char   dpi_chroma_subsamp;
        unsigned int    venc_fmt;
        unsigned char   lane_num;
        unsigned char   refresh_rate;
        unsigned char   trans_mode;

        unsigned char   trans_type;    //such ad hs or lp
        unsigned char   ack_type;      //if need bta ack check
        unsigned char   tear_switch;

        unsigned char   is_rgb;        //whether dpi color type is rgb
}DSI_Config_t;

typedef struct {
	unsigned lvds_vswing;
	unsigned lvds_repack_user;
	unsigned lvds_repack;
	unsigned pn_swap;
} LVDS_Config_t;

typedef struct {
	unsigned char link_user;
	unsigned char lane_count;
	unsigned char link_rate;
	unsigned char link_adaptive;
	unsigned char vswing;
	unsigned char preemphasis;
	unsigned int bit_rate;
} EDP_Config_t;

typedef struct {
	unsigned char rb_swap;
	unsigned char bit_swap;
} TTL_Config_t;

typedef struct {
	unsigned phy_ctrl;
} DPHY_Config_t;

typedef struct {
	DSI_Config_t *mipi_config;
	LVDS_Config_t *lvds_config;
	EDP_Config_t *edp_config;
	TTL_Config_t *ttl_config;
	DPHY_Config_t *phy_config;
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
#define BL_CTL_GPIO				0
#define BL_CTL_PWM_NEGATIVE		1
#define BL_CTL_PWM_POSITIVE		2
#define BL_PWM_A				0
#define BL_PWM_B				1
#define BL_PWM_C				2
#define BL_PWM_D				3

#define BL_LEVEL_MAX    		255
#define BL_LEVEL_MIN    		10
#define BL_LEVEL_OFF			1

#define BL_LEVEL_MID    		128
#define BL_LEVEL_MID_MAPPED		102

#define BL_LEVEL_DEFAULT		128

typedef struct {
	unsigned level_default;
	unsigned level_mid;
	unsigned level_mid_mapping;
	unsigned level_min;
	unsigned level_max;
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

Lcd_Bl_Config_t bl_config_dft;
//*************************************//

extern void mdelay(unsigned long msec);

typedef enum {
	GPIOAO_0=0,
	GPIOAO_1=1,
	GPIOAO_2=2,
	GPIOAO_3=3,
	GPIOAO_4=4,
	GPIOAO_5=5,
	GPIOAO_6=6,
	GPIOAO_7=7,
	GPIOAO_8=8,
	GPIOAO_9=9,
	GPIOAO_10=10,
	GPIOAO_11=11,
	GPIOAO_12=12,
	GPIOAO_13=13,
	GPIOZ_0=14,
	GPIOZ_1=15,
	GPIOZ_2=16,
	GPIOZ_3=17,
	GPIOZ_4=18,
	GPIOZ_5=19,
	GPIOZ_6=20,
	GPIOZ_7=21,
	GPIOZ_8=22,
	GPIOZ_9=23,
	GPIOZ_10=24,
	GPIOZ_11=25,
	GPIOZ_12=26,
	GPIOZ_13=27,
	GPIOZ_14=28,
	GPIOH_0=29,
	GPIOH_1=30,
	GPIOH_2=31,
	GPIOH_3=32,
	GPIOH_4=33,
	GPIOH_5=34,
	GPIOH_6=35,
	GPIOH_7=36,
	GPIOH_8=37,
	GPIOH_9=38,
	BOOT_0=39,
	BOOT_1=40,
	BOOT_2=41,
	BOOT_3=42,
	BOOT_4=43,
	BOOT_5=44,
	BOOT_6=45,
	BOOT_7=46,
	BOOT_8=47,
	BOOT_9=48,
	BOOT_10=49,
	BOOT_11=50,
	BOOT_12=51,
	BOOT_13=52,
	BOOT_14=53,
	BOOT_15=54,
	BOOT_16=55,
	BOOT_17=56,
	BOOT_18=57,
	CARD_0=58,
	CARD_1=59,
	CARD_2=60,
	CARD_3=61,
	CARD_4=62,
	CARD_5=63,
	CARD_6=64,
	GPIODV_0=65,
	GPIODV_1=66,
	GPIODV_2=67,
	GPIODV_3=68,
	GPIODV_4=69,
	GPIODV_5=70,
	GPIODV_6=71,
	GPIODV_7=72,
	GPIODV_8=73,
	GPIODV_9=74,
	GPIODV_10=75,
	GPIODV_11=76,
	GPIODV_12=77,
	GPIODV_13=78,
	GPIODV_14=79,
	GPIODV_15=80,
	GPIODV_16=81,
	GPIODV_17=82,
	GPIODV_18=83,
	GPIODV_19=84,
	GPIODV_20=85,
	GPIODV_21=86,
	GPIODV_22=87,
	GPIODV_23=88,
	GPIODV_24=89,
	GPIODV_25=90,
	GPIODV_26=91,
	GPIODV_27=92,
	GPIODV_28=93,
	GPIODV_29=94,
	GPIOY_0=95,
	GPIOY_1=96,
	GPIOY_2=97,
	GPIOY_3=98,
	GPIOY_4=99,
	GPIOY_5=100,
	GPIOY_6=101,
	GPIOY_7=102,
	GPIOY_8=103,
	GPIOY_9=104,
	GPIOY_10=105,
	GPIOY_11=106,
	GPIOY_12=107,
	GPIOY_13=108,
	GPIOY_14=109,
	GPIOY_15=110,
	GPIOY_16=111,
	GPIOX_0=112,
	GPIOX_1=113,
	GPIOX_2=114,
	GPIOX_3=115,
	GPIOX_4=116,
	GPIOX_5=117,
	GPIOX_6=118,
	GPIOX_7=119,
	GPIOX_8=120,
	GPIOX_9=121,
	GPIOX_10=122,
	GPIOX_11=123,
	GPIOX_12=124,
	GPIOX_13=125,
	GPIOX_14=126,
	GPIOX_15=127,
	GPIOX_16=128,
	GPIOX_17=129,
	GPIOX_18=130,
	GPIOX_19=131,
	GPIOX_20=132,
	GPIOX_21=133,
	GPIO_TEST_N=134,
	GPIO_MAX=135,
}gpio_t;

static const char* amlogic_gpio_type_table[]={
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
	"GPIOAO_12",
	"GPIOAO_13",
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
	"GPIOZ_13",
	"GPIOZ_14",
	"GPIOH_0",
	"GPIOH_1",
	"GPIOH_2",
	"GPIOH_3",
	"GPIOH_4",
	"GPIOH_5",
	"GPIOH_6",
	"GPIOH_7",
	"GPIOH_8",
	"GPIOH_9",
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
	"BOOT_18",
	"CARD_0",
	"CARD_1",
	"CARD_2",
	"CARD_3",
	"CARD_4",
	"CARD_5",
	"CARD_6",
	"GPIODV_0",
	"GPIODV_1",
	"GPIODV_2",
	"GPIODV_3",
	"GPIODV_4",
	"GPIODV_5",
	"GPIODV_6",
	"GPIODV_7",
	"GPIODV_8",
	"GPIODV_9",
	"GPIODV_10",
	"GPIODV_11",
	"GPIODV_12",
	"GPIODV_13",
	"GPIODV_14",
	"GPIODV_15",
	"GPIODV_16",
	"GPIODV_17",
	"GPIODV_18",
	"GPIODV_19",
	"GPIODV_20",
	"GPIODV_21",
	"GPIODV_22",
	"GPIODV_23",
	"GPIODV_24",
	"GPIODV_25",
	"GPIODV_26",
	"GPIODV_27",
	"GPIODV_28",
	"GPIODV_29",
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
	"GPIOY_16",
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
	"GPIO_TEST_N",
	"GPIO_MAX",
}; 

#endif /* LCDOUTC_H */
