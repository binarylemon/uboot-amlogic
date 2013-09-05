/*******************************************************************
 * 
 *  Copyright C 2008 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: 1/18/2007
 *
 *******************************************************************/
#ifndef TV_ENC_H
#define TV_ENC_H

typedef enum {
    TV_ENC_480i = 0,
    TV_ENC_576i,
    TV_ENC_480p,
    TV_ENCP_480p,
    TV_ENC_576p,
    TV_ENC_1080i, 
    TV_ENC_720p, 
    TV_ENC_1080p, 
    TV_ENC_2205p,
    TV_ENC_2440p, 
    TV_ENC_3840x2160p_vic01, 
    TV_ENC_3840x2160p_vic03, 
    TV_ENC_4096x2160p_vic04, 
    TV_ENC_TYPE_MAX
} tv_enc_type_t;   /* tv encoder output format */

typedef enum {
    TV_ENC_LCD480x234 = 0,
    TV_ENC_LCD480x234_dsi36b = 1,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD240x160 = 2,
    TV_ENC_LCD240x160_dsi36b = 3,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD720x480 = 4,
    TV_ENC_LCD720x480_dsi36b = 5,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD720x576 = 6,
    TV_ENC_LCD720x576_dsi36b = 7,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD1280x720 = 8,
    TV_ENC_LCD1280x720_dsi36b = 9,      // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD1920x1080 = 10,
    TV_ENC_LCD1920x1080_dsi36b = 11,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD1920x2205 = 12,
    TV_ENC_LCD1920x2205_dsi36b = 13,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD2560x1600 = 14,
    TV_ENC_LCD2560x1600_dsi36b = 15,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD3840x2440 = 16,
    TV_ENC_LCD3840x2440_dsi36b = 17,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
    TV_ENC_LCD3840x2160p_vic03 = 18,
    TV_ENC_LCD4096x2160p_vic04 = 19,
    TV_ENC_LCD640x480 = 20,
    TV_ENC_LCD1920x1200p = 21, 
    TV_ENC_LCD240x160_dsi   = 22,
    TV_ENC_LCD240x160_slow   = 24,
    TV_ENC_LCD3840x2160p_vic01 = 23,
       TV_ENC_LCD_TYPE_MAX
} tv_enc_lcd_type_t;   /* tv encoder output format */

extern void config_tv_enc ( tv_enc_type_t output_type );
extern void config_tv_enci ( tv_enc_type_t output_type );
extern int set_tv_enc (tv_enc_type_t output_type);
extern int set_tv_enci (tv_enc_type_t output_type);
extern void config_tv_enc_lcd( tv_enc_lcd_type_t output_type );
extern int set_tv_enc_lcd( tv_enc_lcd_type_t output_type );
extern void venc_set_lcd(int w, int h, int w1, int h1);
extern void set_tv_enc_720x480i (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_720x576i (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_1920x1080p (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_1920x2205p (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_1280x720p (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_1920x1080i (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_3840x2160p_vic03 (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_enc_4096x2160p_vic04 (int viu1_sel, int viu2_sel, int enable);
extern void set_tv_encl (tv_enc_lcd_type_t output_type, int viu1_sel, int viu2_sel, int enable);
extern void set_tv_encl_480x234 (int viu1_sel, int viu2_sel, int enable);
extern void venc_set_lvds_and_sel_viu (int w, int h, int w1, int h1, int viu1_sel, int viu2_sel);

#endif /* TV_ENC_H */
