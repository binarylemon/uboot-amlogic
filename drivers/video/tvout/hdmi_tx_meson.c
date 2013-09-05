/*
 * Amlogic M3
 * frame buffer driver-----------HDMI_TX
 * Copyright (C) 2010 Amlogic, Inc.
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
 */
#include "ubi_uboot.h"
#include <common.h>
#include <asm/arch/io.h>
#include <amlogic/aml_tv.h>
#include "enc_clk_config.h"

#include "hdmi_info_global.h"
#include "hdmi_tx_module.h"
#include "hdmi_tx_reg.h"
#include "tvenc_conf.h"

//#define XTAL_24MHZ
#ifdef Wr
#undef Wr
#endif
#ifdef Rd
#undef Rd
#endif
#ifdef Wr_reg_bits
#undef Wr_reg_bits
#endif

#define Wr(reg,val) WRITE_MPEG_REG(reg,val)
#define Rd(reg)   READ_MPEG_REG(reg)
#define Wr_reg_bits(reg, val, start, len) \
  Wr(reg, (Rd(reg) & ~(((1L<<(len))-1)<<(start)))|((unsigned int)(val) << (start)))

static void hdmi_suspend(void);
static void hdmi_wakeup(void);


#define HDMI_M1A 'a'
#define HDMI_M1B 'b'
#define HDMI_M1C 'c'
static unsigned char hdmi_chip_type = 0;
unsigned char hdmi_pll_mode = 0; /* 1, use external clk as hdmi pll source */

#define HSYNC_POLARITY      1                       // HSYNC polarity: active high 
#define VSYNC_POLARITY      1                       // VSYNC polarity: active high
#define TX_INPUT_COLOR_DEPTH    0                   // Pixel bit width: 0=24-bit; 1=30-bit; 2=36-bit; 3=48-bit.
#define TX_INPUT_COLOR_FORMAT   1                   // Pixel format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
#define TX_INPUT_COLOR_RANGE    0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.


#define TX_OUTPUT_COLOR_RANGE   0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.

#if 1
//spdif
#define TX_I2S_SPDIF        0                       // 0=SPDIF; 1=I2S.
#define TX_I2S_8_CHANNEL    0                       // 0=I2S 2-channel; 1=I2S 4 x 2-channel.
#else
//i2s 8 channel
#define TX_I2S_SPDIF        1                       // 0=SPDIF; 1=I2S.
#define TX_I2S_8_CHANNEL    1                       // 0=I2S 2-channel; 1=I2S 4 x 2-channel.
#endif

//static struct tasklet_struct EDID_tasklet;
static unsigned delay_flag = 0;
static unsigned color_depth_f=0;
static unsigned color_space_f=0;
static unsigned char power_mode=1;
static unsigned char power_off_vdac_flag=0;
    /* 0, do not use fixed tvenc val for all mode; 1, use fixed tvenc val mode for 480i; 2, use fixed tvenc val mode for all modes */

//static unsigned char cur_vout_index = 1; //CONFIG_AM_TV_OUTPUT2

#define HPD_DEBUG_IGNORE_UNPLUG   1

static unsigned long modulo(unsigned long a, unsigned long b)
{
    if (a >= b) {
        return(a-b);
    } else {
        return(a);
    }
}
        
static signed int to_signed(unsigned int a)
{
    if (a <= 7) {
        return(a);
    } else {
        return(a-16);
    }
}

static void delay_us (int us)
{
    udelay(us);
} /* delay_us */

static void hdmi_tvenc1080i_set(Hdmi_tx_video_para_t* param)
{
    unsigned long VFIFO2VD_TO_HDMI_LATENCY = 2; // Annie 01Sep2011: Change value from 3 to 2, due to video encoder path delay change.
    unsigned long TOTAL_PIXELS, PIXEL_REPEAT_HDMI, PIXEL_REPEAT_VENC, ACTIVE_PIXELS;
    unsigned FRONT_PORCH, HSYNC_PIXELS, ACTIVE_LINES, INTERLACE_MODE, TOTAL_LINES, SOF_LINES, VSYNC_LINES;
    unsigned LINES_F0, LINES_F1,BACK_PORCH, EOF_LINES, TOTAL_FRAMES;

    unsigned long total_pixels_venc ;
    unsigned long active_pixels_venc;
    unsigned long front_porch_venc  ;
    unsigned long hsync_pixels_venc ;

    unsigned long de_h_begin, de_h_end;
    unsigned long de_v_begin_even, de_v_end_even, de_v_begin_odd, de_v_end_odd;
    unsigned long hs_begin, hs_end;
    unsigned long vs_adjust;
    unsigned long vs_bline_evn, vs_eline_evn, vs_bline_odd, vs_eline_odd;
    unsigned long vso_begin_evn, vso_begin_odd;
    
    if(param->VIC==HDMI_1080i60){
         INTERLACE_MODE     = 1;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS  =     (1920*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES   =     (1080/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 562;                 
         LINES_F1           = 563;                 
         FRONT_PORCH        = 88;                  
         HSYNC_PIXELS       = 44;                  
         BACK_PORCH         = 148;                  
         EOF_LINES          = 2;                   
         VSYNC_LINES        = 5;                   
         SOF_LINES          = 15;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if(param->VIC==HDMI_1080i50){
         INTERLACE_MODE     = 1;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS  =     (1920*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES   =     (1080/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 562;                 
         LINES_F1           = 563;                 
         FRONT_PORCH        = 528;                  
         HSYNC_PIXELS       = 44;                  
         BACK_PORCH         = 148;                  
         EOF_LINES          = 2;                   
         VSYNC_LINES        = 5;                   
         SOF_LINES          = 15;                  
         TOTAL_FRAMES       = 4;                   
    }
    TOTAL_PIXELS =(FRONT_PORCH+HSYNC_PIXELS+BACK_PORCH+ACTIVE_PIXELS); // Number of total pixels per line.
    TOTAL_LINES  =(LINES_F0+(LINES_F1*INTERLACE_MODE));                // Number of total lines per frame.

    total_pixels_venc = (TOTAL_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 2200 / 1 * 2 = 4400
    active_pixels_venc= (ACTIVE_PIXELS / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 1920 / 1 * 2 = 3840
    front_porch_venc  = (FRONT_PORCH   / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 88   / 1 * 2 = 176
    hsync_pixels_venc = (HSYNC_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 44   / 1 * 2 = 88

    Wr(ENCP_VIDEO_MODE, Rd(ENCP_VIDEO_MODE)|(1<<14)); // cfg_de_v = 1

    // Program DE timing
    de_h_begin = modulo(Rd(ENCP_VIDEO_HAVON_BEGIN) + VFIFO2VD_TO_HDMI_LATENCY,  total_pixels_venc); // (383 + 3) % 4400 = 386
    de_h_end   = modulo(de_h_begin + active_pixels_venc,                        total_pixels_venc); // (386 + 3840) % 4400 = 4226
    Wr(ENCP_DE_H_BEGIN, de_h_begin);    // 386
    Wr(ENCP_DE_H_END,   de_h_end);      // 4226
    // Program DE timing for even field
    de_v_begin_even = Rd(ENCP_VIDEO_VAVON_BLINE);       // 20
    de_v_end_even   = de_v_begin_even + ACTIVE_LINES;   // 20 + 540 = 560
    Wr(ENCP_DE_V_BEGIN_EVEN,de_v_begin_even);   // 20
    Wr(ENCP_DE_V_END_EVEN,  de_v_end_even);     // 560
    // Program DE timing for odd field if needed
    if (INTERLACE_MODE) {
        // Calculate de_v_begin_odd according to enc480p_timing.v:
        //wire[10:0]	cfg_ofld_vavon_bline	= {{7{ofld_vavon_ofst1 [3]}},ofld_vavon_ofst1 [3:0]} + cfg_video_vavon_bline	+ ofld_line;
        de_v_begin_odd  = to_signed((Rd(ENCP_VIDEO_OFLD_VOAV_OFST) & 0xf0)>>4) + de_v_begin_even + (TOTAL_LINES-1)/2; // 1 + 20 + (1125-1)/2 = 583
        de_v_end_odd    = de_v_begin_odd + ACTIVE_LINES;    // 583 + 540 = 1123
        Wr(ENCP_DE_V_BEGIN_ODD, de_v_begin_odd);// 583
        Wr(ENCP_DE_V_END_ODD,   de_v_end_odd);  // 1123
    }

    // Program Hsync timing
    if (de_h_end + front_porch_venc >= total_pixels_venc) {
        hs_begin    = de_h_end + front_porch_venc - total_pixels_venc; // 4226 + 176 - 4400 = 2
        vs_adjust   = 1;
    } else {
        hs_begin    = de_h_end + front_porch_venc;
        vs_adjust   = 0;
    }
    hs_end  = modulo(hs_begin + hsync_pixels_venc,   total_pixels_venc); // (2 + 88) % 4400 = 90
    Wr(ENCP_DVI_HSO_BEGIN,  hs_begin);  // 2
    Wr(ENCP_DVI_HSO_END,    hs_end);    // 90
    
    // Program Vsync timing for even field
    if (de_v_begin_even >= SOF_LINES + VSYNC_LINES + (1-vs_adjust)) {
        vs_bline_evn = de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust); // 20 - 15 - 5 - 0 = 0
    } else {
        vs_bline_evn = TOTAL_LINES + de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust);
    }
    vs_eline_evn = modulo(vs_bline_evn + VSYNC_LINES, TOTAL_LINES); // (0 + 5) % 1125 = 5
    Wr(ENCP_DVI_VSO_BLINE_EVN, vs_bline_evn);   // 0
    Wr(ENCP_DVI_VSO_ELINE_EVN, vs_eline_evn);   // 5
    vso_begin_evn = hs_begin; // 2
    Wr(ENCP_DVI_VSO_BEGIN_EVN, vso_begin_evn);  // 2
    Wr(ENCP_DVI_VSO_END_EVN,   vso_begin_evn);  // 2
    // Program Vsync timing for odd field if needed
    if (INTERLACE_MODE) {
        vs_bline_odd = de_v_begin_odd-1 - SOF_LINES - VSYNC_LINES;  // 583-1 - 15 - 5   = 562
        vs_eline_odd = de_v_begin_odd-1 - SOF_LINES;                // 583-1 - 15       = 567
        vso_begin_odd   = modulo(hs_begin + (total_pixels_venc>>1), total_pixels_venc); // (2 + 4400/2) % 4400 = 2202
        Wr(ENCP_DVI_VSO_BLINE_ODD, vs_bline_odd);   // 562
        Wr(ENCP_DVI_VSO_ELINE_ODD, vs_eline_odd);   // 567
        Wr(ENCP_DVI_VSO_BEGIN_ODD, vso_begin_odd);  // 2202
        Wr(ENCP_DVI_VSO_END_ODD,   vso_begin_odd);  // 2202
    }

    // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
    Wr(VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                         (0                                 << 1) | // [    1] src_sel_encp
                         (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                         (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                         (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                         (((TX_INPUT_COLOR_FORMAT==0)?1:0)  << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
                                                                    //                          0=output CrYCb(BRG);
                                                                    //                          1=output YCbCr(RGB);
                                                                    //                          2=output YCrCb(RBG);
                                                                    //                          3=output CbCrY(GBR);
                                                                    //                          4=output CbYCr(GRB);
                                                                    //                          5=output CrCbY(BGR);
                                                                    //                          6,7=Rsrv.
#ifdef DOUBLE_CLK_720P_1080I
                         (0                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
#else                         
                         (1                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
#endif                         
                         (0                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
    );
    Wr_reg_bits(VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI

}    

static void hdmi_tvenc480i_set(Hdmi_tx_video_para_t* param)
{
    unsigned long VFIFO2VD_TO_HDMI_LATENCY = 1; // Annie 01Sep2011: Change value from 2 to 1, due to video encoder path delay change.
    unsigned long TOTAL_PIXELS, PIXEL_REPEAT_HDMI, PIXEL_REPEAT_VENC, ACTIVE_PIXELS;
    unsigned FRONT_PORCH, HSYNC_PIXELS, ACTIVE_LINES, INTERLACE_MODE, TOTAL_LINES, SOF_LINES, VSYNC_LINES;
    unsigned LINES_F0, LINES_F1,BACK_PORCH, EOF_LINES, TOTAL_FRAMES;

    unsigned long total_pixels_venc ;
    unsigned long active_pixels_venc;
    unsigned long front_porch_venc  ;
    unsigned long hsync_pixels_venc ;

    unsigned long de_h_begin, de_h_end;
    unsigned long de_v_begin_even, de_v_end_even, de_v_begin_odd, de_v_end_odd;
    unsigned long hs_begin, hs_end;
    unsigned long vs_adjust;
    unsigned long vs_bline_evn, vs_eline_evn, vs_bline_odd, vs_eline_odd;
    unsigned long vso_begin_evn, vso_begin_odd;

    if((param->VIC==HDMI_480i60)||(param->VIC==HDMI_480i60_16x9)){
         INTERLACE_MODE     = 1;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 1;                   
         ACTIVE_PIXELS  =     (720*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES   =     (480/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 262;                 
         LINES_F1           = 263;                 
         FRONT_PORCH        = 38;                  
         HSYNC_PIXELS       = 124;                  
         BACK_PORCH         = 114;                  
         EOF_LINES          = 4;                   
         VSYNC_LINES        = 3;                   
         SOF_LINES          = 15;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if((param->VIC==HDMI_576i50)||(param->VIC==HDMI_576i50_16x9)){
         INTERLACE_MODE     = 1;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 1;                   
         ACTIVE_PIXELS  =     (720*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES   =     (576/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 312;                 
         LINES_F1           = 313;                 
         FRONT_PORCH        = 24;                  
         HSYNC_PIXELS       = 126;                  
         BACK_PORCH         = 138;                  
         EOF_LINES          = 2;                   
         VSYNC_LINES        = 3;                   
         SOF_LINES          = 19;                  
         TOTAL_FRAMES       = 4;                   
    }
    TOTAL_PIXELS =(FRONT_PORCH+HSYNC_PIXELS+BACK_PORCH+ACTIVE_PIXELS); // Number of total pixels per line.
    TOTAL_LINES  =(LINES_F0+(LINES_F1*INTERLACE_MODE));                // Number of total lines per frame.

    total_pixels_venc = (TOTAL_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 1716 / 2 * 2 = 1716
    active_pixels_venc= (ACTIVE_PIXELS / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 1440 / 2 * 2 = 1440
    front_porch_venc  = (FRONT_PORCH   / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 38   / 2 * 2 = 38
    hsync_pixels_venc = (HSYNC_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 124  / 2 * 2 = 124

    // Annie 01Sep2011: Comment out the following 2 lines. Because ENCP is not used for 480i and 576i.
    //Wr(ENCP_VIDEO_MODE,Rd(ENCP_VIDEO_MODE)|(1<<14)); // cfg_de_v = 1

    // Program DE timing
    // Annie 01Sep2011: for 480/576i, replace VFIFO2VD_PIXEL_START with ENCI_VFIFO2VD_PIXEL_START.
    de_h_begin = modulo(Rd(ENCI_VFIFO2VD_PIXEL_START) + VFIFO2VD_TO_HDMI_LATENCY,   total_pixels_venc); // (233 + 2) % 1716 = 235
    de_h_end   = modulo(de_h_begin + active_pixels_venc,                            total_pixels_venc); // (235 + 1440) % 1716 = 1675
    Wr(ENCI_DE_H_BEGIN, de_h_begin);    // 235
    Wr(ENCI_DE_H_END,   de_h_end);      // 1675

    // Annie 01Sep2011: for 480/576i, replace VFIFO2VD_LINE_TOP/BOT_START with ENCI_VFIFO2VD_LINE_TOP/BOT_START.
    de_v_begin_even = Rd(ENCI_VFIFO2VD_LINE_TOP_START);      // 17
    de_v_end_even   = de_v_begin_even + ACTIVE_LINES;   // 17 + 240 = 257
    de_v_begin_odd  = Rd(ENCI_VFIFO2VD_LINE_BOT_START);      // 18
    de_v_end_odd    = de_v_begin_odd + ACTIVE_LINES;    // 18 + 480/2 = 258
    Wr(ENCI_DE_V_BEGIN_EVEN,de_v_begin_even);   // 17
    Wr(ENCI_DE_V_END_EVEN,  de_v_end_even);     // 257
    Wr(ENCI_DE_V_BEGIN_ODD, de_v_begin_odd);    // 18
    Wr(ENCI_DE_V_END_ODD,   de_v_end_odd);      // 258

    // Program Hsync timing
    if (de_h_end + front_porch_venc >= total_pixels_venc) {
        hs_begin    = de_h_end + front_porch_venc - total_pixels_venc;
        vs_adjust   = 1;
    } else {
        hs_begin    = de_h_end + front_porch_venc; // 1675 + 38 = 1713
        vs_adjust   = 0;
    }
    hs_end  = modulo(hs_begin + hsync_pixels_venc,   total_pixels_venc); // (1713 + 124) % 1716 = 121
    Wr(ENCI_DVI_HSO_BEGIN,  hs_begin);  // 1713
    Wr(ENCI_DVI_HSO_END,    hs_end);    // 121
    
    // Program Vsync timing for even field
    if (de_v_end_odd-1 + EOF_LINES + vs_adjust >= LINES_F1) {
        vs_bline_evn = de_v_end_odd-1 + EOF_LINES + vs_adjust - LINES_F1;
        vs_eline_evn = vs_bline_evn + VSYNC_LINES;
        Wr(ENCI_DVI_VSO_BLINE_EVN, vs_bline_evn);
        //vso_bline_evn_reg_wr_cnt ++;
        Wr(ENCI_DVI_VSO_ELINE_EVN, vs_eline_evn);
        //vso_eline_evn_reg_wr_cnt ++;
        Wr(ENCI_DVI_VSO_BEGIN_EVN, hs_begin);
        Wr(ENCI_DVI_VSO_END_EVN,   hs_begin);
    } else {
        vs_bline_odd = de_v_end_odd-1 + EOF_LINES + vs_adjust; // 258-1 + 4 + 0 = 261
        Wr(ENCI_DVI_VSO_BLINE_ODD, vs_bline_odd); // 261
        //vso_bline_odd_reg_wr_cnt ++;
        Wr(ENCI_DVI_VSO_BEGIN_ODD, hs_begin);  // 1713
        if (vs_bline_odd + VSYNC_LINES >= LINES_F1) {
            vs_eline_evn = vs_bline_odd + VSYNC_LINES - LINES_F1; // 261 + 3 - 263 = 1
            Wr(ENCI_DVI_VSO_ELINE_EVN, vs_eline_evn);   // 1
            //vso_eline_evn_reg_wr_cnt ++;
            Wr(ENCI_DVI_VSO_END_EVN,   hs_begin);       // 1713
        } else {
            vs_eline_odd = vs_bline_odd + VSYNC_LINES;
            Wr(ENCI_DVI_VSO_ELINE_ODD, vs_eline_odd);
            //vso_eline_odd_reg_wr_cnt ++;
            Wr(ENCI_DVI_VSO_END_ODD,   hs_begin);
        }
    }
    // Program Vsync timing for odd field
    if (de_v_end_even-1 + EOF_LINES + 1 >= LINES_F0) {
        vs_bline_odd = de_v_end_even-1 + EOF_LINES + 1 - LINES_F0;
        vs_eline_odd = vs_bline_odd + VSYNC_LINES;
        Wr(ENCI_DVI_VSO_BLINE_ODD, vs_bline_odd);
        //vso_bline_odd_reg_wr_cnt ++;
        Wr(ENCI_DVI_VSO_ELINE_ODD, vs_eline_odd);
        //vso_eline_odd_reg_wr_cnt ++;
        vso_begin_odd   = modulo(hs_begin + (total_pixels_venc>>1), total_pixels_venc);
        Wr(ENCI_DVI_VSO_BEGIN_ODD, vso_begin_odd);
        Wr(ENCI_DVI_VSO_END_ODD,   vso_begin_odd);
    } else {
        vs_bline_evn = de_v_end_even-1 + EOF_LINES + 1; // 257-1 + 4 + 1 = 261
        Wr(ENCI_DVI_VSO_BLINE_EVN, vs_bline_evn); // 261
        //vso_bline_evn_reg_wr_cnt ++;
        vso_begin_evn   = modulo(hs_begin + (total_pixels_venc>>1), total_pixels_venc);   // (1713 + 1716/2) % 1716 = 855
        Wr(ENCI_DVI_VSO_BEGIN_EVN, vso_begin_evn);  // 855
        if (vs_bline_evn + VSYNC_LINES >= LINES_F0) {
            vs_eline_odd = vs_bline_evn + VSYNC_LINES - LINES_F0; // 261 + 3 - 262 = 2
            Wr(ENCI_DVI_VSO_ELINE_ODD, vs_eline_odd);   // 2
            //vso_eline_odd_reg_wr_cnt ++;
            Wr(ENCI_DVI_VSO_END_ODD,   vso_begin_evn);  // 855
        } else {
            vs_eline_evn = vs_bline_evn + VSYNC_LINES;
            Wr(ENCI_DVI_VSO_ELINE_EVN, vs_eline_evn);
            //vso_eline_evn_reg_wr_cnt ++;
            Wr(ENCI_DVI_VSO_END_EVN,   vso_begin_evn);
        }
    }

    // Check if there are duplicate or missing timing settings
    //if ((vso_bline_evn_reg_wr_cnt != 1) || (vso_bline_odd_reg_wr_cnt != 1) ||
    //    (vso_eline_evn_reg_wr_cnt != 1) || (vso_eline_odd_reg_wr_cnt != 1)) {
        //stimulus_print("[TEST.C] Error: Multiple or missing timing settings on reg ENCI_DVI_VSO_B(E)LINE_EVN(ODD)!\n");
        //stimulus_finish_fail(1);
    //}

    // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
    Wr(VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                         (0                                 << 1) | // [    1] src_sel_encp
                         (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                         (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                         (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                         (((TX_INPUT_COLOR_FORMAT==0)?1:0)  << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
                                                                    //                          0=output CrYCb(BRG);
                                                                    //                          1=output YCbCr(RGB);
                                                                    //                          2=output YCrCb(RBG);
                                                                    //                          3=output CbCrY(GBR);
                                                                    //                          4=output CbYCr(GRB);
                                                                    //                          5=output CrCbY(BGR);
                                                                    //                          6,7=Rsrv.
                         (1                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
                         (1                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
    );
    Wr_reg_bits(VPU_HDMI_SETTING, 1, 0, 1);  // [    0] src_sel_enci: Enable ENCI output to HDMI

}    

#ifndef AVOS
static 
#endif
void hdmi_tvenc_set(Hdmi_tx_video_para_t *param)
{
    unsigned long VFIFO2VD_TO_HDMI_LATENCY = 2; // Annie 01Sep2011: Change value from 3 to 2, due to video encoder path delay change.
    unsigned long TOTAL_PIXELS, PIXEL_REPEAT_HDMI, PIXEL_REPEAT_VENC, ACTIVE_PIXELS;
    unsigned FRONT_PORCH, HSYNC_PIXELS, ACTIVE_LINES, INTERLACE_MODE, TOTAL_LINES, SOF_LINES, VSYNC_LINES;
    unsigned LINES_F0, LINES_F1,BACK_PORCH, EOF_LINES, TOTAL_FRAMES;

    unsigned long total_pixels_venc ;
    unsigned long active_pixels_venc;
    unsigned long front_porch_venc  ;
    unsigned long hsync_pixels_venc ;

    unsigned long de_h_begin, de_h_end;
    unsigned long de_v_begin_even, de_v_end_even, de_v_begin_odd, de_v_end_odd;
    unsigned long hs_begin, hs_end;
    unsigned long vs_adjust;
    unsigned long vs_bline_evn, vs_eline_evn, vs_bline_odd, vs_eline_odd;
    unsigned long vso_begin_evn, vso_begin_odd;

    if((param->VIC==HDMI_480p60)||(param->VIC==HDMI_480p60_16x9)){
         INTERLACE_MODE     = 0;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS      = (720*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES       = (480/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 525;                 
         LINES_F1           = 525;                 
         FRONT_PORCH        = 16;                  
         HSYNC_PIXELS       = 62;                  
         BACK_PORCH         = 60;                  
         EOF_LINES          = 9;                   
         VSYNC_LINES        = 6;                   
         SOF_LINES          = 30;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if((param->VIC==HDMI_576p50)||(param->VIC==HDMI_576p50_16x9)){
         INTERLACE_MODE     = 0;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS      = (720*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES       = (576/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 625;                 
         LINES_F1           = 625;                 
         FRONT_PORCH        = 12;                  
         HSYNC_PIXELS       = 64;                  
         BACK_PORCH         = 68;                  
         EOF_LINES          = 5;                   
         VSYNC_LINES        = 5;                   
         SOF_LINES          = 39;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if(param->VIC==HDMI_720p60){
         INTERLACE_MODE     = 0;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS      = (1280*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES       = (720/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 750;                 
         LINES_F1           = 750;                 
         FRONT_PORCH        = 110;                  
         HSYNC_PIXELS       = 40;                  
         BACK_PORCH         = 220;                  
         EOF_LINES          = 5;                   
         VSYNC_LINES        = 5;                   
         SOF_LINES          = 20;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if(param->VIC==HDMI_720p50){
         INTERLACE_MODE     = 0;                   
         PIXEL_REPEAT_VENC  = 1;                   
         PIXEL_REPEAT_HDMI  = 0;                   
         ACTIVE_PIXELS      = (1280*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES       = (720/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0           = 750;                 
         LINES_F1           = 750;                 
         FRONT_PORCH        = 440;                  
         HSYNC_PIXELS       = 40;                  
         BACK_PORCH         = 220;                  
         EOF_LINES          = 5;                   
         VSYNC_LINES        = 5;                   
         SOF_LINES          = 20;                  
         TOTAL_FRAMES       = 4;                   
    }
    else if(param->VIC==HDMI_1080p50){
         INTERLACE_MODE      =0;              
         PIXEL_REPEAT_VENC   =0;              
         PIXEL_REPEAT_HDMI   =0;              
         ACTIVE_PIXELS       =(1920*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES        =(1080/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0            =1125;           
         LINES_F1            =1125;           
         FRONT_PORCH         =528;             
         HSYNC_PIXELS        =44;             
         BACK_PORCH          =148;            
         EOF_LINES           =4;              
         VSYNC_LINES         =5;              
         SOF_LINES           =36;             
         TOTAL_FRAMES        =4;              
    }
    else{ //HDMI_1080p60, HDMI_1080p30
         INTERLACE_MODE      =0;              
         PIXEL_REPEAT_VENC   =0;              
         PIXEL_REPEAT_HDMI   =0;              
         ACTIVE_PIXELS       =(1920*(1+PIXEL_REPEAT_HDMI)); // Number of active pixels per line.
         ACTIVE_LINES        =(1080/(1+INTERLACE_MODE));    // Number of active lines per field.
         LINES_F0            =1125;           
         LINES_F1            =1125;           
         FRONT_PORCH         =88;             
         HSYNC_PIXELS        =44;             
         BACK_PORCH          =148;            
         EOF_LINES           =4;              
         VSYNC_LINES         =5;              
         SOF_LINES           =36;             
         TOTAL_FRAMES        =4;              
    }

    TOTAL_PIXELS       = (FRONT_PORCH+HSYNC_PIXELS+BACK_PORCH+ACTIVE_PIXELS); // Number of total pixels per line.
    TOTAL_LINES        = (LINES_F0+(LINES_F1*INTERLACE_MODE));                // Number of total lines per frame.

    total_pixels_venc = (TOTAL_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 858 / 1 * 2 = 1716
    active_pixels_venc= (ACTIVE_PIXELS / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 720 / 1 * 2 = 1440
    front_porch_venc  = (FRONT_PORCH   / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 16   / 1 * 2 = 32
    hsync_pixels_venc = (HSYNC_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC); // 62   / 1 * 2 = 124

    Wr(ENCP_VIDEO_MODE,Rd(ENCP_VIDEO_MODE)|(1<<14)); // cfg_de_v = 1
    // Program DE timing
    de_h_begin = modulo(Rd(ENCP_VIDEO_HAVON_BEGIN) + VFIFO2VD_TO_HDMI_LATENCY,  total_pixels_venc); // (217 + 3) % 1716 = 220
    de_h_end   = modulo(de_h_begin + active_pixels_venc,                        total_pixels_venc); // (220 + 1440) % 1716 = 1660
    Wr(ENCP_DE_H_BEGIN, de_h_begin);    // 220
    Wr(ENCP_DE_H_END,   de_h_end);      // 1660
    // Program DE timing for even field
    de_v_begin_even = Rd(ENCP_VIDEO_VAVON_BLINE);       // 42
    de_v_end_even   = de_v_begin_even + ACTIVE_LINES;   // 42 + 480 = 522
    Wr(ENCP_DE_V_BEGIN_EVEN,de_v_begin_even);   // 42
    Wr(ENCP_DE_V_END_EVEN,  de_v_end_even);     // 522
    // Program DE timing for odd field if needed
    if (INTERLACE_MODE) {
        // Calculate de_v_begin_odd according to enc480p_timing.v:
        //wire[10:0]    cfg_ofld_vavon_bline    = {{7{ofld_vavon_ofst1 [3]}},ofld_vavon_ofst1 [3:0]} + cfg_video_vavon_bline    + ofld_line;
        de_v_begin_odd  = to_signed((Rd(ENCP_VIDEO_OFLD_VOAV_OFST) & 0xf0)>>4) + de_v_begin_even + (TOTAL_LINES-1)/2;
        de_v_end_odd    = de_v_begin_odd + ACTIVE_LINES;
        Wr(ENCP_DE_V_BEGIN_ODD, de_v_begin_odd);
        Wr(ENCP_DE_V_END_ODD,   de_v_end_odd);
    }

    // Program Hsync timing
    if (de_h_end + front_porch_venc >= total_pixels_venc) {
        hs_begin    = de_h_end + front_porch_venc - total_pixels_venc;
        vs_adjust   = 1;
    } else {
        hs_begin    = de_h_end + front_porch_venc; // 1660 + 32 = 1692
        vs_adjust   = 0;
    }
    hs_end  = modulo(hs_begin + hsync_pixels_venc,   total_pixels_venc); // (1692 + 124) % 1716 = 100
    Wr(ENCP_DVI_HSO_BEGIN,  hs_begin);  // 1692
    Wr(ENCP_DVI_HSO_END,    hs_end);    // 100
    
    // Program Vsync timing for even field
    if (de_v_begin_even >= SOF_LINES + VSYNC_LINES + (1-vs_adjust)) {
        vs_bline_evn = de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust); // 42 - 30 - 6 - 1 = 5
    } else {
        vs_bline_evn = TOTAL_LINES + de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust);
    }
    vs_eline_evn = modulo(vs_bline_evn + VSYNC_LINES, TOTAL_LINES); // (5 + 6) % 525 = 11
    Wr(ENCP_DVI_VSO_BLINE_EVN, vs_bline_evn);   // 5
    Wr(ENCP_DVI_VSO_ELINE_EVN, vs_eline_evn);   // 11
    vso_begin_evn = hs_begin; // 1692
    Wr(ENCP_DVI_VSO_BEGIN_EVN, vso_begin_evn);  // 1692
    Wr(ENCP_DVI_VSO_END_EVN,   vso_begin_evn);  // 1692
    // Program Vsync timing for odd field if needed
    if (INTERLACE_MODE) {
        vs_bline_odd = de_v_begin_odd-1 - SOF_LINES - VSYNC_LINES;
        vs_eline_odd = de_v_begin_odd-1 - SOF_LINES;
        vso_begin_odd   = modulo(hs_begin + (total_pixels_venc>>1), total_pixels_venc);
        Wr(ENCP_DVI_VSO_BLINE_ODD, vs_bline_odd);
        Wr(ENCP_DVI_VSO_ELINE_ODD, vs_eline_odd);
        Wr(ENCP_DVI_VSO_BEGIN_ODD, vso_begin_odd);
        Wr(ENCP_DVI_VSO_END_ODD,   vso_begin_odd);
    }
    // Annie 01Sep2011: Remove the following line as register VENC_DVI_SETTING_MORE is no long valid, use VPU_HDMI_SETTING instead.
    //Wr(VENC_DVI_SETTING_MORE, (TX_INPUT_COLOR_FORMAT==0)? 1 : 0); // [0] 0=Map data pins from Venc to Hdmi Tx as CrYCb mode;
    
    switch(param->VIC)
    {
        case HDMI_480p60:
        case HDMI_480p60_16x9:
        case HDMI_576p50:
        case HDMI_576p50_16x9:
//Note: Hsync & Vsync polarity should be negative.
//Refer to HDMI CTS 1.4A Page 169
            // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
            Wr(VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                                 (0                                 << 1) | // [    1] src_sel_encp
                                 (0                                 << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                                 (0                                 << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                                 (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                                 (((TX_INPUT_COLOR_FORMAT==0)?1:0)  << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
                                                                            //                          0=output CrYCb(BRG);
                                                                            //                          1=output YCbCr(RGB);
                                                                            //                          2=output YCrCb(RBG);
                                                                            //                          3=output CbCrY(GBR);
                                                                            //                          4=output CbYCr(GRB);
                                                                            //                          5=output CrCbY(BGR);
                                                                            //                          6,7=Rsrv.
                                 (1                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
                                 (0                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
            );
            break;
        case HDMI_720p60:
        case HDMI_720p50:
            // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
            Wr(VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                                 (0                                 << 1) | // [    1] src_sel_encp
                                 (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                                 (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                                 (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                                 (((TX_INPUT_COLOR_FORMAT==0)?1:0)  << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
                                                                            //                          0=output CrYCb(BRG);
                                                                            //                          1=output YCbCr(RGB);
                                                                            //                          2=output YCrCb(RBG);
                                                                            //                          3=output CbCrY(GBR);
                                                                            //                          4=output CbYCr(GRB);
                                                                            //                          5=output CrCbY(BGR);
                                                                            //                          6,7=Rsrv.
#ifdef DOUBLE_CLK_720P_1080I
                                 (0                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
#else
                                 (1                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
#endif                             
                                 (0                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
            );
            break;
        default:
            // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
            Wr(VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                                 (0                                 << 1) | // [    1] src_sel_encp
                                 (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                                 (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                                 (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                                 (((TX_INPUT_COLOR_FORMAT==0)?1:0)  << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
                                                                            //                          0=output CrYCb(BRG);
                                                                            //                          1=output YCbCr(RGB);
                                                                            //                          2=output YCrCb(RBG);
                                                                            //                          3=output CbCrY(GBR);
                                                                            //                          4=output CbYCr(GRB);
                                                                            //                          5=output CrCbY(BGR);
                                                                            //                          6,7=Rsrv.
                                 (0                                 << 8) | // [11: 8] wr_rate. 0=A write every clk1; 1=A write every 2 clk1; ...; 15=A write every 16 clk1.
                                 (0                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
            );
    }

    // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
    Wr_reg_bits(VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
}    

/*
hdmi on/off
*/
static int is_hpd_muxed(void)
{
    int ret;
    ret = !!(Rd(PERIPHS_PIN_MUX_1)&(1<<22));
    return ret; 
}    

static void mux_hpd(void)
{
    Wr(PERIPHS_PIN_MUX_1, Rd(PERIPHS_PIN_MUX_1)|(1 << 22));
}

static void unmux_hpd(void)
{
    Wr(PERIPHS_PIN_MUX_1, Rd(PERIPHS_PIN_MUX_1)&~(1 << 22));
    //GPIOC_10 0x2012[10]
    Wr(PREG_PAD_GPIO2_EN_N, Rd(PREG_PAD_GPIO2_EN_N)|(1<<10)); //GPIOC_10 as input
}    

static int read_hpd_gpio(void)
{
    int level;
//    level = Rd(PREG_PAD_GPIO2_I)&0x1; //read GPIOA_0
    level = !!(Rd(PREG_PAD_GPIO2_I)&(1<<10)); //read GPIOC_10
    return level;
}

static void digital_clk_off(unsigned char flag)
{
//    clk81_resume();
    if(flag&1){
    }

    if(flag&2){
        /* off hdmi pixel clock */
//        Wr(HHI_GCLK_MPEG2, Rd(HHI_GCLK_MPEG2)&(~(1<<4))); //disable pixel clock, set cbus reg HHI_GCLK_MPEG2 bit [4] = 0
        Wr(HHI_GCLK_OTHER, Rd(HHI_GCLK_OTHER)&(~(1<<17))); //disable VCLK1_HDMI GATE, set cbus reg HHI_GCLK_OTHER bit [17] = 0
        Wr(VENC_DVI_SETTING, (Rd(VENC_DVI_SETTING)&(~(7<<4)))|(5<<4)); //set cbus reg VENC_DVI_SETTING bit[6:4] = 0x5
        Wr(HHI_VID_PLL_CNTL, (Rd(HHI_VID_PLL_CNTL) | (1<<30)));     //0x105c[30]PD, 1: PowerDown
        Wr(HHI_VID_PLL_CNTL3, (Rd(HHI_VID_PLL_CNTL3) & ~((1<<5)|(1<<3))));  //0x1058[5]VBG_PU [3]IREXT_PU
#if 1
    // Second turn off gate.
        Wr(HHI_GCLK_MPEG2, Rd(HHI_GCLK_MPEG2) & (~(1<<4)));     //Disable HDMI PCLK
#endif 
 
    }
    if(flag&4){
        /* off hdmi sys clock */
        Wr(HHI_HDMI_CLK_CNTL, Rd(HHI_HDMI_CLK_CNTL)&(~(1<<8))); // off hdmi sys clock gate
    }
}

static void digital_clk_on(unsigned char flag)
{
    int i;
//    clk81_set();
    if(flag&4){
        /* on hdmi sys clock */
//#ifdef AML_A3
        // -----------------------------------------
        // HDMI (90Mhz)
        // -----------------------------------------
        //         .clk_div            ( hi_hdmi_clk_cntl[6:0] ),
        //         .clk_en             ( hi_hdmi_clk_cntl[8]   ),
        //         .clk_sel            ( hi_hdmi_clk_cntl[11:9]),
        Wr_reg_bits(HHI_HDMI_CLK_CNTL, 0, 0, 7);    // Divide the "other" PLL output by 1
        Wr_reg_bits(HHI_HDMI_CLK_CNTL, 0, 9, 3);    // select "XTAL" PLL
        Wr_reg_bits(HHI_HDMI_CLK_CNTL, 1, 8, 1);    // Enable gated clock
    }
    if(flag&2){
        /* on hdmi pixel clock */
#if 1
        Wr(HHI_GCLK_MPEG2, Rd(HHI_GCLK_MPEG2) | (1<<4));     //Enable HDMI PCLK
        Wr(HHI_VID_PLL_CNTL3, (Rd(HHI_VID_PLL_CNTL3) | ((1<<5)|(1<<3))));  //0x1058[5]VBG_PU [3]IREXT_PU
        i=100;
        while(i--);     //delay some time and then PowerUp HHI_VID_PLL
        Wr(HHI_VID_PLL_CNTL, (Rd(HHI_VID_PLL_CNTL) & ~(1<<30)));     //0x105c[30]PD, 0: Power Up
#endif        
//        Wr(HHI_GCLK_MPEG2, Rd(HHI_GCLK_MPEG2)|(1<<4)); //enable pixel clock, set cbus reg HHI_GCLK_MPEG2 bit [4] = 1
        Wr(HHI_GCLK_OTHER, Rd(HHI_GCLK_OTHER)|(1<<17)); //enable VCLK1_HDMI GATE, set cbus reg HHI_GCLK_OTHER bit [17] = 1
    }
    if(flag&1){
    }  
}

static void phy_pll_off(void)
{
    hdmi_suspend();
}

/**/
void hdmi_hw_set_powermode( int power_mode, int vic)
{
    power_mode = 1;
    hdmi_wakeup();
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG0, 0xfe << HDMI_COMMON_b7_b0);    //0x10
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG1, (0xf  <<HDMI_CTL_REG_b3_b0) |
                                     (0xe  << HDMI_COMMON_b11_b8));    //0x10
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG2, 0xff << HDMI_CTL_REG_b11_b4);   //0xf7
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG3, (0xf << HDMI_MDR_PU)|
                                     (0x7 << HDMI_L2H_CTL));    //0x16
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG4, (0x2 << HDMI_PREM_CTL) |
                                     (0x0 << HDMI_MODE) |
                                     (0x1 << HDMI_PHY_CLK_EN) |
                                     (0x0 << HDMI_LF_PD)
                                     );      //0x14 Prem
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG5, (0x7 << HDMI_VCM_CTL) | 
                                     (0x7 << HDMI_PREFCTL));         //0x15 Slew
    hdmi_wr_reg(TX_HDMI_PHY_CONFIG6, (0xa << HDMI_SWING_CTL) | 
                                     (0x0 << HDMI_RTERM_CTL) );
}

void hdmi_hw_init(hdmitx_dev_t* hdmitx_device)
{
    unsigned int tmp_add_data;
    
    digital_clk_on(7);
    Wr(HHI_HDMI_AFC_CNTL, Rd(HHI_HDMI_AFC_CNTL) | 0x3);
    hdmi_wr_reg(0x01a, 0xfb);   //bit[2:0]=011 ,CK channel output TMDS CLOCK ,bit[2:0]=101 ,ck channel output PHYCLCK 

    hdmi_hw_set_powermode(power_mode, 0);

    hdmi_wr_reg(0x0F7, 0x0F);   // Termination resistor calib value
  //hdmi_wr_reg(0x014, 0x07);   // This register is for pre-emphasis control ,we need test different TMDS Clcok speed then write down the suggested     value for each one ;
  //hdmi_wr_reg(0x014, 0x01);   // This is a sample for Pre-emphasis setting ,recommended for 225MHz's TMDS Setting & ** meters HDMI Cable  

    // --------------------------------------------------------
    // Program core_pin_mux to enable HDMI pins
    // --------------------------------------------------------
    //wire            pm_hdmi_cec_en              = pin_mux_reg0[2];
    //wire            pm_hdmi_hpd_5v_en           = pin_mux_reg0[1];
    //wire            pm_hdmi_i2c_5v_en           = pin_mux_reg0[0];
#ifdef AML_A3
    // --------------------------------------------------------
    // Program core_pin_mux to enable HDMI pins
    // --------------------------------------------------------
    //wire            pm_gpioA_3_hdmi_cec         = pin_mux_reg0[3];
    //wire            pm_gpioA_2_hdmi_scl         = pin_mux_reg0[2];
    //wire            pm_gpioA_1_hdmi_sda         = pin_mux_reg0[1];
    //wire            pm_gpioA_0_hdmi_hpd         = pin_mux_reg0[0];
    Wr(PERIPHS_PIN_MUX_0, Rd(PERIPHS_PIN_MUX_0)|((0 << 3) | // pm_gpioA_3_hdmi_cec
                               (1 << 2) | // pm_gpioA_2_hdmi_scl
                               (1 << 1) | // pm_gpioA_1_hdmi_sda
                               (0 << 0 ))); // pm_gpioA_0_hdmi_hpd , enable this signal after all init done to ensure fist HPD rising ok

#else
    Wr(PERIPHS_PIN_MUX_1, Rd(PERIPHS_PIN_MUX_1)|((1 << 25) | // pm_hdmi_cec_en
                               (1 << 22) | // pm_hdmi_hpd_5v_en , enable this signal after all init done to ensure fist HPD rising ok
                               (1 << 23) | // pm_hdmi_i2c_sda_en
                               (1 << 24))); // pm_hdmi_i2c_scl_en
#endif

    // Enable these interrupts: [2] tx_edit_int_rise [1] tx_hpd_int_fall [0] tx_hpd_int_rise
    hdmi_wr_reg(OTHER_BASE_ADDR + HDMI_OTHER_INTR_MASKN, 0x7);
    // HPD glitch filter
    hdmi_wr_reg(TX_HDCP_HPD_FILTER_L, 0x00);
    hdmi_wr_reg(TX_HDCP_HPD_FILTER_H, 0xaf);

//#ifdef AML_A3
#if 1
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x90); //bit5,6 is converted
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x60);
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0xff);
    delay_us(10);
#else
    //new reset sequence, 2010Sep09, rain
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0xf0);
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x00);
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0xff);
    delay_us(10);
#endif    
    /**/

    // Enable software controlled DDC transaction
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7]   = 1'b0 ;  // forced_sys_trigger
    //tmp_add_data[6]   = 1'b0 ;  // sys_trigger_config
    //tmp_add_data[5]   = 1'b0 ;  // mem_acc_seq_mode
    //tmp_add_data[4]   = 1'b0 ;  // mem_acc_seq_start
    //tmp_add_data[3]   = 1'b1 ;  // forced_mem_copy_done
    //tmp_add_data[2]   = 1'b1 ;  // mem_copy_done_config
    //tmp_add_data[1]   = 1'b1 ;  // sys_trigger_config_semi_manu
    //tmp_add_data[0]   = 1'b0 ;  // Rsrv
    hdmi_wr_reg(TX_HDCP_EDID_CONFIG, 0x0c); //// for hdcp, can not use 0x0e
    
    hdmi_wr_reg(TX_HDCP_CONFIG0,      1<<3);  //set TX rom_encrypt_off=1
    hdmi_wr_reg(TX_HDCP_MEM_CONFIG,   0<<3);  //set TX read_decrypt=0
    hdmi_wr_reg(TX_HDCP_ENCRYPT_BYTE, 0);     //set TX encrypt_byte=0x00
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b0;       // Force packet timing
    //tmp_add_data[6] = 1'b0;       // PACKET ALLOC MODE
    //tmp_add_data[5:0] = 6'd47 ;   // PACKET_START_LATENCY
    //tmp_add_data = 47;
    tmp_add_data = 58;
    hdmi_wr_reg(TX_PACKET_CONTROL_1, tmp_add_data); //this register should be set to ensure the first hdcp succeed

    //tmp_add_data[7] = 1'b0;      // cp_desired
    //tmp_add_data[6] = 1'b0;      // ess_config
    //tmp_add_data[5] = 1'b0;      // set_avmute
    //tmp_add_data[4] = 1'b1;      // clear_avmute
    //tmp_add_data[3] = 1'b0;      // hdcp_1_1
    //tmp_add_data[2] = 1'b0;      // Vsync/Hsync forced_polarity_select
    //tmp_add_data[1] = 1'b0;      // forced_vsync_polarity
    //tmp_add_data[0] = 1'b0;      // forced_hsync_polarity
    //tmp_add_data = 0x10;
    tmp_add_data = 0x0; //rain
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);
    //config_hdmi(1);

    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7:0]   = 0xa ; // time_divider[7:0] for DDC I2C bus clock
    //tmp_add_data = 0xa; //800k
    //tmp_add_data = 0x3f; //190k
    tmp_add_data = 0x18; //100k     // hdmi system clock change to XTAL 24MHz
    hdmi_wr_reg(TX_HDCP_CONFIG3, tmp_add_data);

    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7]   = 8'b1 ;  //cp_desired 
    //tmp_add_data[6]   = 8'b1 ;  //ess_config 
    //tmp_add_data[5]   = 8'b0 ;  //set_avmute 
    //tmp_add_data[4]   = 8'b0 ;  //clear_avmute 
    //tmp_add_data[3]   = 8'b1 ;  //hdcp_1_1 
    //tmp_add_data[2]   = 8'b0 ;  //forced_polarity 
    //tmp_add_data[1]   = 8'b0 ;  //forced_vsync_polarity 
    //tmp_add_data[0]   = 8'b0 ;  //forced_hsync_polarity
    tmp_add_data = 0x40;
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);

    hdmi_hw_set_powermode(power_mode, 0);

    // --------------------------------------------------------
    // Release TX out of reset
    // --------------------------------------------------------
    //new reset sequence, 2010Sep09, rain
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 1<<6); // Release resets all other TX digital clock domain, except tmds_clk
        delay_us(10);
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0x00); // Final release reset on tmds_clk domain
        delay_us(10);        
//#ifdef AML_A3
#if 1
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x68);        
        delay_us(10);
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x60);        
        delay_us(10);
#else
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x08);        
        delay_us(10);
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x00);        
        delay_us(10);
#endif        
    /**/
}    

static void hdmi_hw_reset(Hdmi_tx_video_para_t *param)
{
    unsigned int tmp_add_data;
    unsigned long TX_OUTPUT_COLOR_FORMAT;

    digital_clk_on(7);

    if(param->color==COLOR_SPACE_YUV444){
        TX_OUTPUT_COLOR_FORMAT=1;
    }
    else if(param->color==COLOR_SPACE_YUV422){
        TX_OUTPUT_COLOR_FORMAT=3;
    }
    else{
        TX_OUTPUT_COLOR_FORMAT=0;
    }
#ifndef AML_A3
    // Configure HDMI PLL
    if((hdmi_chip_type == HDMI_M1A)||(hdmi_pll_mode == 1)){
        Wr(HHI_VID_PLL_CNTL3, 0x50e8);
    }
    else{
        Wr(HHI_VID_PLL_CNTL3, 0x40e8);
    }

        Wr(HHI_VID_PLL_CNTL2, 0x00040003); 
#endif
    if(delay_flag&2)
        delay_us(1000*100);
    //printf("delay 100ms\n");

    Wr(HHI_HDMI_AFC_CNTL, Rd(HHI_HDMI_AFC_CNTL) | 0x3);
    // Configure HDMI TX serializer:
    //hdmi_wr_reg(0x011, 0x0f);   //Channels Power Up Setting ,"1" for Power-up ,"0" for Power-down,Bit[3:0]=CK,Data2,data1,data1,data0 Channels ;
  //hdmi_wr_reg(0x015, 0x03);   //slew rate

    hdmi_hw_set_powermode(power_mode, param->VIC);

    hdmi_wr_reg(0x0F7, 0x0F);   // Termination resistor calib value
  //hdmi_wr_reg(0x014, 0x07);   // This register is for pre-emphasis control ,we need test different TMDS Clcok speed then write down the suggested     value for each one ;
  //hdmi_wr_reg(0x014, 0x01);   // This is a sample for Pre-emphasis setting ,recommended for 225MHz's TMDS Setting & ** meters HDMI Cable  
    
    // delay 1000uS, then check HPLL_LOCK
    delay_us(1000);
    //while ( (Rd(HHI_VID_PLL_CNTL3) & (1<<31)) != (1<<31) );
 
//////////////////////////////reset    
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x90);
        delay_us(10);
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x60);
        delay_us(10);
        hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0xff);
        delay_us(10);
    // Enable software controlled DDC transaction
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7]   = 1'b0 ;  // forced_sys_trigger
    //tmp_add_data[6]   = 1'b0 ;  // sys_trigger_config
    //tmp_add_data[5]   = 1'b0 ;  // mem_acc_seq_mode
    //tmp_add_data[4]   = 1'b0 ;  // mem_acc_seq_start
    //tmp_add_data[3]   = 1'b1 ;  // forced_mem_copy_done
    //tmp_add_data[2]   = 1'b1 ;  // mem_copy_done_config
    //tmp_add_data[1]   = 1'b1 ;  // sys_trigger_config_semi_manu
    //tmp_add_data[0]   = 1'b0 ;  // Rsrv

    tmp_add_data = 0x0c; // for hdcp, can not use 0x0e 
    hdmi_wr_reg(TX_HDCP_EDID_CONFIG, tmp_add_data);
    
    hdmi_wr_reg(TX_HDCP_CONFIG0,      1<<3);  //set TX rom_encrypt_off=1
    hdmi_wr_reg(TX_HDCP_MEM_CONFIG,   0<<3);  //set TX read_decrypt=0
    hdmi_wr_reg(TX_HDCP_ENCRYPT_BYTE, 0);     //set TX encrypt_byte=0x00
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b0;      // Force DTV timing (Auto)
    //tmp_add_data[6] = 1'b0;      // Force Video Scan, only if [7]is set
    //tmp_add_data[5] = 1'b0 ;     // Force Video field, only if [7]is set
    //tmp_add_data[4:0] = 5'b00 ;  // Rsrv
    tmp_add_data = 0;
    hdmi_wr_reg(TX_VIDEO_DTV_TIMING, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 7; // [7]   forced_default_phase
    tmp_add_data |= 0                       << 2; // [6:2] Rsrv
    tmp_add_data |= param->color_depth      << 0; // [1:0] Color_depth:0=24-bit pixel; 1=30-bit pixel; 2=36-bit pixel; 3=48-bit pixel
    hdmi_wr_reg(TX_VIDEO_DTV_MODE, tmp_add_data); // 0x00
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b0;       // Force packet timing
    //tmp_add_data[6] = 1'b0;       // PACKET ALLOC MODE
    //tmp_add_data[5:0] = 6'd47 ;   // PACKET_START_LATENCY
    //tmp_add_data = 47;
    tmp_add_data = 58;
    hdmi_wr_reg(TX_PACKET_CONTROL_1, tmp_add_data);

    // For debug: disable packets of audio_request, acr_request, deep_color_request, and avmute_request
    //hdmi_wr_reg(TX_PACKET_CONTROL_2, hdmi_rd_reg(TX_PACKET_CONTROL_2) | 0x0f);
    
    //HDMI CT 7-19 GCP PB1 through PB6 not equal to 0 | 720 3 0 37 72 16367911819.90 31822 General Control Packet (GCP) 
    //PACKET_CONTROL[~deep_color_request_enable]
    //0: horizontal GC packet transport enabled
    //1: horizontal GC packet masked
    hdmi_wr_reg(TX_PACKET_CONTROL_2, hdmi_rd_reg(TX_PACKET_CONTROL_2) | (0xf<<0));
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7:6] = 2'b0;     // audio_source_select[1:0]
    //tmp_add_data[5] = 1'b0;       // external_packet_enable
    //tmp_add_data[4] = 1'b1 ;      // internal_packet_enable
    //tmp_add_data[3:2] = 2'b0;     // afe_fifo_source_select_lane_1[1:0]
    //tmp_add_data[1:0] = 2'b0 ;    // afe_fifo_source_select_lane_0[1:0]
    tmp_add_data = 0x10;
    hdmi_wr_reg(TX_CORE_DATA_CAPTURE_2, tmp_add_data);
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7]   = 1'b0;     // monitor_lane_1
    //tmp_add_data[6:4] = 3'd0;     // monitor_select_lane_1[2:0]
    //tmp_add_data[3]   = 1'b1 ;    // monitor_lane_0
    //tmp_add_data[2:0] = 3'd7;     // monitor_select_lane_0[2:0]
    tmp_add_data = 0xf;
    hdmi_wr_reg(TX_CORE_DATA_MONITOR_1, tmp_add_data);
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7:3] = 5'b0;     // Rsrv
    //tmp_add_data[2:0] = 3'd2;     // monitor_select[2:0]
    tmp_add_data = 0x2;
    hdmi_wr_reg(TX_CORE_DATA_MONITOR_2, tmp_add_data);
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b1;     // forced_hdmi
    //tmp_add_data[6] = 1'b1;     // hdmi_config
    //tmp_add_data[5:4] = 2'b0;   // Rsrv
    //tmp_add_data[3] = 1'b0;     // bit_swap.
    //tmp_add_data[2:0] = 3'd0;   // channel_swap[2:0]
    tmp_add_data = 0xc0;
    hdmi_wr_reg(TX_TMDS_MODE, tmp_add_data);
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b0;  // Rsrv
    //tmp_add_data[6] = 1'b0;  // TX_CONNECT_SEL: 0=use lower channel data[29:0]; 1=use upper channel data[59:30]
    //tmp_add_data[5:0] = 'h0;  // Rsrv
    tmp_add_data = 0x0;
    hdmi_wr_reg(TX_SYS4_CONNECT_SEL_1, tmp_add_data);
    
    // Normally it makes sense to synch 3 channel output with clock channel's rising edge,
    // as HDMI's serializer is LSB out first, invert tmds_clk pattern from "1111100000" to
    // "0000011111" actually enable data synch with clock rising edge.
    //if((param->VIC==HDMI_1080p30)||(param->VIC==HDMI_720p60)||(param->VIC==HDMI_1080i60)){
    //    hdmi_wr_reg(TX_SYS4_CK_INV_VIDEO, 0xf0);
    //}
    //else{
        tmp_add_data = 1 << 4; // Set tmds_clk pattern to be "0000011111" before being sent to AFE clock channel
        hdmi_wr_reg(TX_SYS4_CK_INV_VIDEO, tmp_add_data);
    //}            
    
    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7] = 1'b0;  // Rsrv
    //tmp_add_data[6] = 1'b0;  // TX_AFE_FIFO channel 2 bypass=0
    //tmp_add_data[5] = 1'b0;  // TX_AFE_FIFO channel 1 bypass=0
    //tmp_add_data[4] = 1'b0;  // TX_AFE_FIFO channel 0 bypass=0
    //tmp_add_data[3] = 1'b1;  // output enable of clk channel (channel 3)
    //tmp_add_data[2] = 1'b1;  // TX_AFE_FIFO channel 2 enable
    //tmp_add_data[1] = 1'b1;  // TX_AFE_FIFO channel 1 enable
    //tmp_add_data[0] = 1'b1;  // TX_AFE_FIFO channel 0 enable
    tmp_add_data = 0x7f;
    hdmi_wr_reg(TX_SYS5_FIFO_CONFIG, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= TX_OUTPUT_COLOR_FORMAT  << 6; // [7:6] output_color_format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= TX_INPUT_COLOR_FORMAT   << 4; // [5:4] input_color_format:  0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= param->color_depth   << 2; // [3:2] output_color_depth:  0=24-b; 1=30-b; 2=36-b; 3=48-b.
    tmp_add_data |= TX_INPUT_COLOR_DEPTH    << 0; // [1:0] input_color_depth:   0=24-b; 1=30-b; 2=36-b; 3=48-b.
    hdmi_wr_reg(TX_VIDEO_DTV_OPTION_L, tmp_add_data); // 0x50
    
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 4; // [7:4] Rsrv
    tmp_add_data |= TX_OUTPUT_COLOR_RANGE   << 2; // [3:2] output_color_range:  0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    tmp_add_data |= TX_INPUT_COLOR_RANGE    << 0; // [1:0] input_color_range:   0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    hdmi_wr_reg(TX_VIDEO_DTV_OPTION_H, tmp_add_data); // 0x00
    
        hdmi_wr_reg(TX_AUDIO_PACK, 0x00); // disable audio sample packets
    //tmp_add_data[7] = 1'b0;      // cp_desired
    //tmp_add_data[6] = 1'b0;      // ess_config
    //tmp_add_data[5] = 1'b0;      // set_avmute
    //tmp_add_data[4] = 1'b1;      // clear_avmute
    //tmp_add_data[3] = 1'b0;      // hdcp_1_1
    //tmp_add_data[2] = 1'b0;      // Vsync/Hsync forced_polarity_select
    //tmp_add_data[1] = 1'b0;      // forced_vsync_polarity
    //tmp_add_data[0] = 1'b0;      // forced_hsync_polarity
    //tmp_add_data = 0x10;
    tmp_add_data = 0x0; //rain
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);
    //config_hdmi(1);

    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7:0]   = 0xa ; // time_divider[7:0] for DDC I2C bus clock
    
    //tmp_add_data = 0xa; //800k
    //tmp_add_data = 0x3f; //190k
    tmp_add_data = 0x18; //100k     // hdmi system clock change to XTAL 24MHz
    hdmi_wr_reg(TX_HDCP_CONFIG3, tmp_add_data);

    //tmp_add_data[15:8] = 0;
    //tmp_add_data[7]   = 8'b1 ;  //cp_desired 
    //tmp_add_data[6]   = 8'b1 ;  //ess_config 
    //tmp_add_data[5]   = 8'b0 ;  //set_avmute 
    //tmp_add_data[4]   = 8'b0 ;  //clear_avmute 
    //tmp_add_data[3]   = 8'b1 ;  //hdcp_1_1 
    //tmp_add_data[2]   = 8'b0 ;  //forced_polarity 
    //tmp_add_data[1]   = 8'b0 ;  //forced_vsync_polarity 
    //tmp_add_data[0]   = 8'b0 ;  //forced_hsync_polarity
    tmp_add_data = 0x40;
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);
    if(param->cc == CC_ITU709){
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CB0, 0xf2);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CB1, 0x2f);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CR0, 0xd4);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CR1, 0x77);        
    }
    else{
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CB0, 0x18);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CB1, 0x58);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CR0, 0xd0);        
        hdmi_wr_reg(TX_VIDEO_CSC_COEFF_CR1, 0x66);        
    }    

    hdmi_hw_set_powermode(power_mode, param->VIC);
    
// --------------------------------------------------------
// Release TX out of reset
// --------------------------------------------------------
    //new reset sequence, 2010Sep09, rain
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 1<<6); // Release resets all other TX digital clock domain, except tmds_clk
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0x00); // Final release reset on tmds_clk domain
    delay_us(10);        
//#ifdef AML_A3
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x68);        
    delay_us(10);
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x60);        
    delay_us(10);
}

/************************************
*    hdmitx hardware level interface
*************************************/
static unsigned char hdmitx_m3_getediddata(hdmitx_dev_t* hdmitx_device)
{
    return 0;
}    

static void hdmitx_set_pll(Hdmi_tx_video_para_t *param)
{
//    Wr_reg_bits(VPU_HDMI_SETTING, 2, 0, 2);     //[ 1: 0] src_sel. 0=Disable output to HDMI; 1=Select VENC_I output to HDMI; 2=Select VENC_P output.
    
    //reset HHI_VID_DIVIDER_CNTL
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)|(1<<7));    //0x1066[7]:SOFT_RESET_POST
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)|(1<<3));    //0x1066[3]:SOFT_RESET_PRE
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)&(~(1<<1)));    //0x1066[1]:RESET_N_POST
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)&(~(1<<0)));    //0x1066[0]:RESET_N_PRE
//    msleep(2);
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)&(~((1<<7)|(1<<3))));
//    Wr(HHI_VID_DIVIDER_CNTL, Rd(HHI_VID_DIVIDER_CNTL)|((1<<1)|(1<<0)));
//
//    Wr(HHI_VID_DIVIDER_CNTL, 0x10843);          //0x1066, set vid_pll_clk = HPLL_CLK_OUT_DIG / 5
//
//    if(cur_vout_index !=2 ){
//        Wr_reg_bits(HHI_VID_CLK_CNTL, 0, 16, 3);    //0x105f    0: vid_pll_clk
//        Wr_reg_bits(HHI_VID_CLK_CNTL, 0x1f, 0, 5);     //0x105f    1: DIV1_EN
//        Wr_reg_bits(HHI_VID_CLK_CNTL, 1, 19, 1);    //0x105f    1: CLK_EN0
//    }
        printf("%s[%d]\n", __func__, __LINE__);
    M6_PLL_RESET(HHI_VID_PLL_CNTL);
	Wr(HHI_VID_PLL_CNTL2, M6_VID_PLL_CNTL_2 );
	Wr(HHI_VID_PLL_CNTL3, M6_VID_PLL_CNTL_3 );
	Wr(HHI_VID_PLL_CNTL4, M6_VID_PLL_CNTL_4 );
        printf("%s[%d]\n", __func__, __LINE__);
    switch(param->VIC)
    {
        case HDMI_480p60:
        case HDMI_480p60_16x9:
            set_vmode_clk(VMODE_480P);
            break;
        case HDMI_576p50:
        case HDMI_576p50_16x9:
            set_vmode_clk(VMODE_576P);
            break;
        case HDMI_480i60:
        case HDMI_480i60_16x9:
            set_vmode_clk(VMODE_480I);
            break;
        case HDMI_576i50:
        case HDMI_576i50_16x9:
//            //Wr(HHI_VID_PLL_CNTL, (3<<18)|(2<<10)|(90<<0));    //27MHz=24MHz*45/4/10
//            Wr(HHI_VID_PLL_CNTL, 0xc042d);//use value 0x42d calculated by "m3_pll_video.pl 1080 24 pll_out hpll"
//            if(cur_vout_index !=2 ){
//                Wr(HHI_VID_CLK_DIV, 3);      //0x1059
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 1, 16, 4);   //cts_hdmi_tx_pixel_clk from clk_div2
//            }
//            else{
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 9, 16, 4);   //cts_hdmi_tx_pixel_clk from v2_clk_div2
//            }
            set_vmode_clk(VMODE_576I);
            break;
        case HDMI_1080p30:
        case HDMI_1080p24:
        case HDMI_720p60:
        case HDMI_720p50:
            set_vmode_clk(VMODE_720P);
            break;
        case HDMI_1080i60:
        case HDMI_1080i50:
//            //Wr(HHI_VID_PLL_CNTL, (12<<10)|(371<<0));    //74.2MHz=24MHz*371/12/10
//            Wr(HHI_VID_PLL_CNTL, 0x5043e);//use value 0x15043e (0x15043e does not work in some TV, use 0x5043e) calculated by "m3_pll_video.pl 742.5 24 pll_out hpll", 
//            if(cur_vout_index !=2 ){
//                Wr(HHI_VID_CLK_DIV, 0);      //0x1059
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 1, 16, 4);   //cts_hdmi_tx_pixel_clk from clk_div2
//            }
//            else{
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 9, 16, 4);   //cts_hdmi_tx_pixel_clk from v2_clk_div2
//            }
            set_vmode_clk(VMODE_1080I);
            break;
        case HDMI_1080p60:
        case HDMI_1080p50:
//            //Wr(HHI_VID_PLL_CNTL, (6<<10)|(371<<0));    //148.4MHz=24MHz*371/6/10
//            Wr(HHI_VID_PLL_CNTL, 0x43e);//use value calculated by "m3_pll_video.pl 1485 24 pll_out hpll"
//            if(cur_vout_index !=2 ){
//                Wr(HHI_VID_CLK_DIV, 1);      //0x1059
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 0, 16, 4);   //0x1073, cts_hdmi_tx_pixel_clk from clk_div1
//            }
//            else{
//                Wr_reg_bits(HHI_HDMI_CLK_CNTL, 8, 16, 4);   //cts_hdmi_tx_pixel_clk from v2_clk_div1
//            }
            set_vmode_clk(VMODE_1080P);
            break;
        default:
            break;
    }
        printf("%s[%d]\n", __func__, __LINE__);

//    M6_PLL_WAIT_FOR_LOCK(HHI_VID_PLL_CNTL);
        printf("%s[%d]\n", __func__, __LINE__);

}

static int hdmitx_m3_set_dispmode(Hdmi_tx_video_para_t *param)
{
    if(param == NULL){ //disable HDMI
        return 0;
    }
    else if((param->VIC!=HDMI_480p60)&&(param->VIC!=HDMI_480p60_16x9)
        &&(param->VIC!=HDMI_576p50)&&(param->VIC!=HDMI_576p50_16x9)
        &&(param->VIC!=HDMI_480i60)&&(param->VIC!=HDMI_480i60_16x9)
        &&(param->VIC!=HDMI_576i50)&&(param->VIC!=HDMI_576i50_16x9)
        &&(param->VIC!=HDMI_1080p30)
        &&(param->VIC!=HDMI_1080p24)
        &&(param->VIC!=HDMI_1080p60)&&(param->VIC!=HDMI_1080p50)
        &&(param->VIC!=HDMI_720p60)&&(param->VIC!=HDMI_720p50)
        &&(param->VIC!=HDMI_1080i60)&&(param->VIC!=HDMI_1080i50)){
        return -1;
    }

    if((hdmi_chip_type == HDMI_M1B || hdmi_chip_type == HDMI_M1C)&&(color_depth_f != 0)){
        if(color_depth_f==24)
            param->color_depth = COLOR_24BIT;
        else if(color_depth_f==30)
            param->color_depth = COLOR_30BIT;
        else if(color_depth_f==36)
            param->color_depth = COLOR_36BIT;
        else if(color_depth_f==48)
            param->color_depth = COLOR_48BIT;
    }
    printf("set mode VIC: %d\n",param->VIC);
    if(color_space_f != 0){
        param->color = color_space_f;
    }

    hdmi_hw_reset(param);    
    hdmitx_set_pll(param);

    if((param->VIC==HDMI_720p60)||(param->VIC==HDMI_720p50)||
        (param->VIC==HDMI_1080i60)||(param->VIC==HDMI_1080i50)){
        Wr(ENCP_VIDEO_HAVON_BEGIN,  Rd(ENCP_VIDEO_HAVON_BEGIN)-1);     
        Wr(ENCP_VIDEO_HAVON_END,  Rd(ENCP_VIDEO_HAVON_END)-1);     
    }

    switch(param->VIC){
        case HDMI_480i60:
        case HDMI_480i60_16x9:
        case HDMI_576i50:
        case HDMI_576i50_16x9:
            hdmi_tvenc480i_set(param);
            break;
        case HDMI_1080i60:
        case HDMI_1080i50:
            hdmi_tvenc1080i_set(param);
            break;
        default:
            hdmi_tvenc_set(param);
        }
    
#ifndef AVOS
    if(power_off_vdac_flag){
        //video_dac_disable();
        SET_CBUS_REG_MASK(VENC_VDAC_SETTING, 0x1f);
    }
#endif

    hdmi_wr_reg(TX_HDMI_PHY_CONFIG1, 0xfe);   //Channels Power Up Setting ,"1" for Power-up ,"0" for Power-down,Bit[3:0]=CK,Data2,data1,data1,data0 Channels ;
    printf("%s[%d]\n", __func__, __LINE__);
    return 0;
}    


static void hdmitx_m3_set_packet(int type, unsigned char* DB, unsigned char* HB)
{
    // AVI frame
    int i ;
    unsigned char ucData ;
    unsigned int pkt_reg_base=TX_PKT_REG_AVI_INFO_BASE_ADDR;
    int pkt_data_len=0;
    
    switch(type)
    {
        case HDMI_PACKET_AVI:
            pkt_reg_base=TX_PKT_REG_AVI_INFO_BASE_ADDR; 
            pkt_data_len=13;
            break;
        default:
            break;
    }
    
    if(DB){
        for(i=0;i<pkt_data_len;i++){
            hdmi_wr_reg(pkt_reg_base+i+1, DB[i]);  
        }
    
        for(i = 0,ucData = 0; i < pkt_data_len ; i++)
        {
            ucData -= DB[i] ;
        }
        for(i=0; i<3; i++){
            ucData -= HB[i];
        }
        hdmi_wr_reg(pkt_reg_base+0x00, ucData);  
    
        hdmi_wr_reg(pkt_reg_base+0x1C, HB[0]);        
        hdmi_wr_reg(pkt_reg_base+0x1D, HB[1]);        
        hdmi_wr_reg(pkt_reg_base+0x1E, HB[2]);        
        hdmi_wr_reg(pkt_reg_base+0x1F, 0x00ff);        // Enable packet generation
    }
    else{
        hdmi_wr_reg(pkt_reg_base+0x1F, 0x0);        // disable packet generation
    }
}


static void hdmitx_m3_setaudioinfoframe(unsigned char* AUD_DB, unsigned char* CHAN_STAT_BUF)
{
    int i ;
    unsigned char AUD_HB[3]={0x84, 0x1, 0xa};
    hdmitx_m3_set_packet(HDMI_AUDIO_INFO, AUD_DB, AUD_HB);    
    //channel status
    if(CHAN_STAT_BUF){
        for(i=0;i<24;i++){
            hdmi_wr_reg(TX_IEC60958_SUB1_OFFSET+i, CHAN_STAT_BUF[i]);        
            hdmi_wr_reg(TX_IEC60958_SUB2_OFFSET+i, CHAN_STAT_BUF[24+i]);
        }
    }
}

static int hdmitx_m3_set_audmode(struct hdmi_tx_dev_s* hdmitx_device, Hdmi_tx_audio_para_t* audio_param)
{
    return 0;
}    
    
static void hdmitx_m3_setupirq(hdmitx_dev_t* hdmitx_device)
{
}    


static void hdmitx_m3_uninit(hdmitx_dev_t* hdmitx_device)
{
}    

static void hdmitx_m3_cntl(hdmitx_dev_t* hdmitx_device, int cmd, unsigned argv)
{
    if(cmd == HDMITX_HWCMD_POWERMODE_SWITCH){
        power_mode=argv;
        hdmi_hw_set_powermode(power_mode, hdmitx_device->cur_VIC);
    }
#ifndef AVOS
    else if(cmd == HDMITX_HWCMD_VDAC_OFF){
        power_off_vdac_flag=1;
        //video_dac_disable();
        SET_CBUS_REG_MASK(VENC_VDAC_SETTING, 0x1f);
    }
#endif 
    else if(cmd == HDMITX_HWCMD_MUX_HPD_IF_PIN_HIGH){
        /* turnon digital module if gpio is high */
        if(is_hpd_muxed() == 0){
            if(read_hpd_gpio()){
                digital_clk_on(4);
                delay_us(1000*100);
                mux_hpd();
            }
        }
    } 
    else if(cmd == HDMITX_HWCMD_MUX_HPD){
         mux_hpd();
    } 
// For test only. 
    else if(cmd == HDMITX_HWCMD_TURNOFF_HDMIHW){
        int unmux_hpd_flag = argv;
//        WRITE_MPEG_REG(VENC_DVI_SETTING, READ_MPEG_REG(VENC_DVI_SETTING)&(~(1<<13))); //bit 13 is used by HDMI only
//        digital_clk_on(4); //enable sys clk so that hdmi registers can be accessed when calling phy_pll_off/digit_clk_off
        if(unmux_hpd_flag){
            phy_pll_off();
            digital_clk_off(7); //off sys clk
            unmux_hpd();
        }
        else{
            digital_clk_on(6);
            phy_pll_off();      //should call digital_clk_on(), otherwise hdmi_rd/wr_reg will hungup
            digital_clk_off(3); //do not off sys clk
        }
    }                
}

static void hdmitx_m3_debug(hdmitx_dev_t* hdmitx_device, const char* buf)
{
}

void HDMITX_M1B_Init(hdmitx_dev_t* hdmitx_device)
{
    printf("%s\n", __func__);
    hdmitx_device->HWOp.SetPacket = hdmitx_m3_set_packet;
    hdmitx_device->HWOp.SetAudioInfoFrame = hdmitx_m3_setaudioinfoframe;
    hdmitx_device->HWOp.GetEDIDData = hdmitx_m3_getediddata;
    hdmitx_device->HWOp.SetDispMode = hdmitx_m3_set_dispmode;
    hdmitx_device->HWOp.SetAudMode = hdmitx_m3_set_audmode;
    hdmitx_device->HWOp.SetupIRQ = hdmitx_m3_setupirq;
    hdmitx_device->HWOp.DebugFun = hdmitx_m3_debug;
    hdmitx_device->HWOp.UnInit = hdmitx_m3_uninit;
    hdmitx_device->HWOp.Cntl = hdmitx_m3_cntl;
                                                                  //     1=Map data pins from Venc to Hdmi Tx as RGB mode.
    // --------------------------------------------------------
    // Configure HDMI TX analog, and use HDMI PLL to generate TMDS clock
    // --------------------------------------------------------
    // Enable APB3 fail on error
//    WRITE_APB_REG(HDMI_CNTL_PORT, READ_APB_REG(HDMI_CNTL_PORT)|(1<<15)); //APB3 err_en
    Wr_reg_bits(PAD_PULL_UP_REG2, 1, 10, 1);       // Disable GPIOC_10 internal pull-up register  --jihongzhou
    Wr(HHI_HDMI_CLK_CNTL, Rd(HHI_HDMI_CLK_CNTL)| (1 << 8));
    Wr(HDMI_CNTL_PORT, Rd(HDMI_CNTL_PORT)|(1<<15)); //APB3 err_en
    hdmi_wr_reg(0x10, 0xff);

    /**/    
    hdmi_hw_init(hdmitx_device);
}    

// The following two functions should move to 
// static struct platform_driver amhdmitx_driver.suspend & .wakeup
// For tempelet use only.
// Later will change it.
typedef struct 
{
    unsigned long reg;
    unsigned long val_sleep;
    unsigned long val_save;
}hdmi_phy_t;

static char hdmi_phy_reg_save_flag = 0;

#define HDMI_PHY_REG_NUM    7
static hdmi_phy_t hdmi_phy_reg [HDMI_PHY_REG_NUM] = {
                         {0x10, 0xc0, 0x0},
                         {0x11, 0xf1, 0x0},
                         {0x12, 0xff, 0x0},
                         {0x13, 0x07, 0x0},
                         {0x14, 0x00, 0x0},
                         {0x15, 0x04, 0x0},
                         {0x16, 0x30, 0x00},
                        };
                        
static void hdmi_suspend(void)
{
    // First backup HDMI PHY register according to Chao Shi.
    int i;
    for(i = 0; i < HDMI_PHY_REG_NUM; i++)
    {
        hdmi_phy_reg[i].val_save = hdmi_rd_reg(hdmi_phy_reg[i].reg);
    }
    for(i = 0; i < HDMI_PHY_REG_NUM; i++)
    {   
        hdmi_wr_reg(hdmi_phy_reg[i].reg, hdmi_phy_reg[i].val_sleep);
    }
    hdmi_phy_reg_save_flag = 1;
    printf("Hdmi phy suspend\n");
}

static void hdmi_wakeup(void)
{
    int i;
    if(hdmi_phy_reg_save_flag){
        for(i = 0; i < HDMI_PHY_REG_NUM; i++)
        {
            hdmi_wr_reg(hdmi_phy_reg[i].reg, hdmi_phy_reg[i].val_save);
        }
        printf("Hdmi phy wakeup\n");
        hdmi_phy_reg_save_flag = 0;
    }
}
