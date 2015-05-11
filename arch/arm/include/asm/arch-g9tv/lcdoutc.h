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

#if 0
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

typedef struct {
    u16 gamma_cntl_port;
    u16 gamma_vcom_hswitch_addr;

    u16 rgb_base_addr;
    u16 rgb_coeff_addr;
    u16 dith_cntl_addr;

    s16 brightness[33];
    s16 contrast[33];
    s16 saturation[33];
    s16 hue[33];

    u16 GammaTableR[256];
    u16 GammaTableG[256];
    u16 GammaTableB[256];
} Lcd_Effect_t;

typedef enum
{
   LCD_NULL = 0,
   LCD_DIGITAL_TTL,
   LCD_DIGITAL_LVDS,
  // LCD_DIGITAL_MINILVDS,
   LCD_DIGITAL_VBYONE,
   LCD_TYPE_MAX,
} Lcd_Type_t;

#endif

typedef enum
{
	LCD_DIGITAL_LVDS = 0,
	LCD_DIGITAL_VBYONE,
	LCD_DIGITAL_TTL,
  // LCD_DIGITAL_MINILVDS,
   LCD_TYPE_MAX,
} Lcd_Type_t;

typedef struct {
	Lcd_Type_t lcd_type;		// LCD_DIGITAL_TTL /LCD_DIGITAL_LVDS/LCD_DIGITAL_VBYONE
    u16 h_active;		// Horizontal display area
    u16 v_active;		// Vertical display area
    u16 h_period;		// Horizontal total period time
    u16 v_period;		// Vertical total period time
	u16 video_on_pixel;
    u16 video_on_line;

    u16 screen_ratio_width;		// screen aspect ratio width
    u16 screen_ratio_height;		// screen aspect ratio height
    u32 screen_actual_width;		/* screen physical width in "mm" unit */
    u32 screen_actual_height;		/* screen physical height in "mm" unit */
}Lcd_Basic_t;

typedef struct {
	u32 hpll_clk;
	u32 hpll_od;
	u32 hdmi_pll_cntl5;

    u16 sync_duration_num;
    u16 sync_duration_den;

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
	int lvds_bits;				// 6 / 8 /10  bits
    int lvds_repack;
    int pn_swap;
    int dual_port;
    int port_reverse;
    int lvds_fifo_wr_mode;
} Lvds_Config_t;

typedef struct {
	int lane_count;
    int byte;
    int region;
    int color_fmt;
} Vbyone_Config_t;

typedef struct {
	Lvds_Config_t *lvds_config;
	Vbyone_Config_t *vbyone_config;

	//TTL_Config_t *ttl_config;
	//DSI_Config_t *mipi_config;
	//EDP_Config_t *edp_config;
	//MLVDS_Config_t *mlvds_config;
	//MLVDS_Tcon_Config_t *mlvds_tcon_config;
} Lcd_Control_Config_t;


//*******************************************//

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
	BL_PWM_MAX,
} BL_PWM_t;

typedef struct {
	unsigned pwm_port;
	unsigned level_default;
	unsigned level_min;
	unsigned level_max;
	unsigned pwm_cnt;
	unsigned pwm_pre_div;
	unsigned pwm_max;
	unsigned pwm_min;
} Lcd_Bl_Config_t;

Lcd_Bl_Config_t bl_config_dft;

//********************************//

//****panel power control only for uboot ***//
typedef struct {
	unsigned int gpio;
	unsigned short on_value;
	unsigned short off_value;
	unsigned short panel_on_delay;
	unsigned short panel_off_delay;
} Panel_Power_Config_t;

//****backlight power control only for uboot ***//
typedef struct {
	unsigned int gpio;
	unsigned short on_value;
	unsigned short off_value;
	unsigned short bl_on_delay;
	unsigned short bl_off_delay;
} Bl_Power_Config_t;


// Power Control
typedef struct {
	Panel_Power_Config_t *panel_power;
	Bl_Power_Config_t    *bl_power;
} Lcd_Power_Ctrl_t;

typedef struct {
    Lcd_Basic_t lcd_basic;
    Lcd_Timing_t lcd_timing;

	Lcd_Control_Config_t lcd_control;
    //Lcd_Effect_t lcd_effect;

	Lcd_Power_Ctrl_t lcd_power_ctrl;
} Lcd_Config_t;

Lcd_Config_t lcd_config_dft;

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

    //u16 sync_duration_num;
    //u16 sync_duration_den;

    u16 sth1_hs_addr;
    u16 sth1_he_addr;
    u16 sth1_vs_addr;
    u16 sth1_ve_addr;
    u16 stv1_hs_addr;
    u16 stv1_he_addr;
    u16 stv1_vs_addr;
    u16 stv1_ve_addr;

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

	unsigned int bl_gpio;
	unsigned short bl_on_value;
	unsigned short bl_off_value;
	unsigned short bl_on_delay;
	unsigned short bl_off_delay;
}Ext_Lcd_Config_t;

#define LCD_TYPE_MAX 	15


#endif /* LCDOUTC_H */
