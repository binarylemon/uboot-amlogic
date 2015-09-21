#ifndef PANEL_TYPICAL_TIMING_H
#define PANEL_TYPICAL_TIMING_H
#include <amlogic/lcdoutc.h>

//*****************************************
// Define LCD Typical Timing Parameters
//*****************************************
#define MODEL_NAME           "CLAA101FP05XG" /** lcd model name */
#define ACITVE_AREA_WIDTH    216 /** lcd active_area or display_area horizontal size(unit in mm, you can find it on the home page of lcd spec) */
#define ACITVE_AREA_HEIGHT   135 /** lcd active_area or display_area vertical size(unit in mm, you can find it on the home page of lcd spec) */
#define LCD_TYPE             LCD_DIGITAL_MIPI   /** lcd interface(LCD_DIGITAL_MIPI, LCD_DIGITAL_LVDS, LCD_DIGITAL_EDP, LCD_DIGITAL_TTL) */
#define LCD_BITS             8 /** lcd bits(6, 8) */
#define BITS_OPTION          0 /** bits_option(0=only support one mode as LCD_BITS define, 1=both support 6/8bit) */

#define H_ACTIVE             1920 /** horizontal resolution */
#define V_ACTIVE             1200 /** vertical resolution */
#define H_PERIOD             2080 /** horizontal period(htotal) */
#define V_PERIOD             1235 /** vertical period(vtotal) */

#define LCD_CLK              154128000 /** clock(unit in Hz, both support clk and frame_rate, >200 regard as clk, <200 regard as frame_rate) */
#define CLK_POL              1  /** clk_polarity(only valid for TTL) */
#define HS_WIDTH             32 /** hsync_width */
#define HS_BACK_PORCH        80 /** hsync_backporch(without hsync_width) */
#define HS_POL               0  /** hsync_polarity(0=negative, 1=positive) */
#define VS_WIDTH             6  /** vsync_width */
#define VS_BACK_PORCH        20 /** vsync_backporch(without vsync_width) */
#define VS_POL               0  /** vsync_polarity(0=negative, 1=positive) */
#define VSYNC_H_ADJUST_SIGN  0  /** 0=positive,1=negative */
#define VSYNC_H_ADJUST       0  /** adj_sign(0=positive, 1=negative), adj_value. default is 0 */

#ifdef CONFIG_AML_LCD_EXTERN
#define EXTERN_INDEX         LCD_EXTERN_INDEX_INVALID
#endif
//************************************************
// MIPI DSI config
//************************************************
#define LCD_MIPI_DSI_CONFIG
#define LANE_NUM            4
#define LANE_BIT_RATE_MAX   1000 /** bit rate limit(unit in MHz) */
#define MIPI_MODE_INIT      1    /** operation mode when init(0=video, 1=command) */
#define MIPI_MODE_DISP      0    /** operation mode when display(0=video, 1=command) */
#define LCD_EXTERN_INIT     0    /** if the init command size is large, should use lcd_extern init */
//******************** mipi command ********************//
//format:  data_type, num, data....
//special: data_type=0xff, num<0xff means delay ms, num=0xff means ending.
//******************************************************//
static unsigned char mipi_init_on_table[] = {//table size < 100
    0x05,1,0x11, //sleep out
    0xff,100,     //delay 100ms
    0x05,1,0x29, //display on
    0xff,100,     //delay 100ms
    0xff,0xff,   //ending flag
};
static unsigned char mipi_init_off_table[] = {//table size < 50
    0x05,1,0x28, //display off
    0xff,10,     //delay 10ms
    0x05,1,0x10, //sleep in
    0xff,10,     //delay 10ms
    0xff,0xff,   //ending flag
};

#endif