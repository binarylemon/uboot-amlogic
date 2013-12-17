#ifndef PANEL_TYPICAL_TIMING_H
#define PANEL_TYPICAL_TIMING_H

//*****************************************
// Define LCD Typical Timing Parameters
//*****************************************
#define MODEL_NAME			"LD070WX4"	/** lcd model name */
#define ACITVE_AREA_WIDTH	94	/** lcd active_area or display_area horizontal size(unit in mm, you can find it on the home page of lcd spec) */
#define ACITVE_AREA_HEIGHT	151	/** lcd active_area or display_area vertical size(unit in mm, you can find it on the home page of lcd spec) */
#define LCD_TYPE			LCD_DIGITAL_MIPI   /** lcd interface(LCD_DIGITAL_MIPI, LCD_DIGITAL_LVDS, LCD_DIGITAL_EDP, LCD_DIGITAL_TTL) */
#define LCD_BITS			8	/** lcd bits(6, 8) */
#define BITS_OPTION			1	/** bits_option(0 for only support one mode as LCD_BITS define, 1 for both support 6/8bit) */

#define H_ACTIVE			800		/** horizontal resolution */
#define V_ACTIVE			1280	/** vertical resolution */
#define H_PERIOD			960		/** horizontal period(htotal) */
#define V_PERIOD			1315	/** vertical period(vtotal)*/

#define	LCD_CLK				75700000	/** clock(unit in Hz, both support clk and frame_rate, >200 regard as clk, <200 regard as frame_rate) */
#define CLK_POL				0			/** clk_polarity(only valid for TTL) */
#define HS_WIDTH			8	/** hsync_width */
#define HS_BACK_PORCH		48	/** hsync_backporch(include hsync_width) */
#define HS_POL				0	/** hsync_polarity(0 for negative, 1 for positive) */
#define VS_WIDTH			1	/** vsync_width */
#define VS_BACK_PORCH		3	/** vsync_backporch(include vsync_width) */
#define VS_POL				0	/** vsync_polarity(0 for negative, 1 for positive) */
//************************************************

#endif