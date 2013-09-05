#ifndef DISPLAY_H
#define DISPLAY_H
#include "f2v.h"

extern int vpp_set_display(
                    f2v_vphase_type_t top_conv_type, 
                    f2v_vphase_type_t bot_conv_type, 
                    unsigned char vert_bank_length,
                    int dst_horz_start, int dst_vert_start,
                    int src_w, int src_h, int dst_w, int dst_h,
                    int prehsc_en, int prevsc_en,
                    int prebld_vd1_en,
                    int postbld_vd1_en,
                    int postbld_en,
                    int prebld_en,
                    int postbld_w,
                    int prebld_w,

                    int vd2_en, 
                    int vd2_h_start,
                    int vd2_h_end,
                    int vd2_v_start,
                    int vd2_v_end,
                    int vd2_sel,    //0: preblend, 1: postblend
                    int vd2_alpha,
                    int osd1_en, 
                    int osd1_sel,   //0: preblend   1: postblend
                    int osd2_en,
                    int osd2_sel,   //0: preblend   1: postblend    
                    int osd2_foreground
                    ); 

//vf_sep_coef_en = (mode >> 15) & 1;
//vf_chro_coef_wren = (mode >> 14) & 1;
//vf_coef_wren = (mode >> 13) & 1;
//vf_bank_len = (mode >> 8) & 0x7; 
//vf_chro_coef_idx = (mode >> 4) & 0xf;
//vf_coef_idx = mode & 0xf;
extern int vpp_scale_set_coef (int mode);



extern int vpp_set_osd_scale (
                   int scale_en, 
                   int hf_en,
                   int vf_en,
                   int src_sel,  
                   int src_w,
                   int src_h,
                   int vd_alpha,
                   int dst_h_start,
                   int dst_h_end,
                   int dst_v_start,
                   int dst_v_end, 
                   int hf_mode,
                   int vf_mode
                   );

extern int vpp2_set_display(
                    f2v_vphase_type_t top_conv_type, 
                    f2v_vphase_type_t bot_conv_type, 
                    unsigned char vert_bank_length,
                    int dst_horz_start, int dst_vert_start,
                    int src_w, int src_h, int dst_w, int dst_h,
                    int prehsc_en, int prevsc_en,
                    int prebld_vd1_en,
                    int postbld_vd1_en,
                    int postbld_en,
                    int prebld_en,
                    int postbld_w,
                    int prebld_w,

                    int osd1_en, 
                    int osd1_sel,   //0: preblend   1: postblend
                    int osd2_en,
                    int osd2_sel,   //0: preblend   1: postblend    
                    int osd2_foreground
                    ); 


extern void set_vd1_fmt(
                int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length 
                );

extern void set_vd1_fmt_more (
                int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                );


extern void    set_vd1_if0_combined_simple(
               unsigned long   x_start,
               unsigned long   x_end,
               unsigned long   y_start,
               unsigned long   y_end,
               unsigned long   mode,    // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
               unsigned long   canvas_addr );      

extern void set_vd1_if0_combined(
        unsigned long   x_start,
        unsigned long   x_end,
        unsigned long   y_start,
        unsigned long   y_end,
        unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
        unsigned long   canvas_addr,
        unsigned long   pic_struct);     //00: frame, 10: top_field, 11: bot_field


#define   set_rmem_if0_separate_simple  set_vd1_if0_separate_simple

extern void    set_vd1_if0_separate_simple(
        unsigned long   luma_x_start,
        unsigned long   luma_x_end,
        unsigned long   luma_y_start,
        unsigned long   luma_y_end,
        unsigned long   chroma_x_start,
        unsigned long   chroma_x_end,
        unsigned long   chroma_y_start,
        unsigned long   chroma_y_end,
        unsigned long   canvas_addr0,
        unsigned long   canvas_addr1,
        unsigned long   canvas_addr2 );

// Allow fully configure VD1_RMEM module
extern void    set_vd1_if0 (
        unsigned long   luma_x_start0,
        unsigned long   luma_x_end0,
        unsigned long   luma_y_start0,
        unsigned long   luma_y_end0,
        unsigned long   chroma_x_start0,
        unsigned long   chroma_x_end0,
        unsigned long   chroma_y_start0,
        unsigned long   chroma_y_end0,
        unsigned long   luma_x_start1,
        unsigned long   luma_x_end1,
        unsigned long   luma_y_start1,
        unsigned long   luma_y_end1,
        unsigned long   chroma_x_start1,
        unsigned long   chroma_x_end1,
        unsigned long   chroma_y_start1,
        unsigned long   chroma_y_end1,
        unsigned long   mode, // 1=444, 0=422, only applicable to YCbCr stored together
        unsigned long   st_separate_en, // 1=Store YCbCr separately, 0=Stored together
        unsigned long   luma_fifo_size, // Size of FIFO for stored together mode only
        unsigned long   chro_rpt_lastl_ctrl,
        unsigned long   burst_size_y,  // 0=24x64-bit, 1=32x64-bit, 2=48x64-bit, 3=64x64-bit
        unsigned long   burst_size_cb, // Same as burst_size_y
        unsigned long   burst_size_cr, // Same as burst_size_y
        unsigned long   luma0_rpt_loop_start,
        unsigned long   luma0_rpt_loop_end,
        unsigned long   luma0_rpt_pat,
        unsigned long   chroma0_rpt_loop_start,
        unsigned long   chroma0_rpt_loop_end,
        unsigned long   chroma0_rpt_pat,
        unsigned long   luma1_rpt_loop_start,
        unsigned long   luma1_rpt_loop_end,
        unsigned long   luma1_rpt_pat,
        unsigned long   chroma1_rpt_loop_start,
        unsigned long   chroma1_rpt_loop_end,
        unsigned long   chroma1_rpt_pat,
        unsigned long   luma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   luma_psel_loop_start,
        unsigned long   luma_psel_loop_end,
        unsigned long   luma_psel_pat,
        unsigned long   chroma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   chroma_psel_loop_start,
        unsigned long   chroma_psel_loop_end,
        unsigned long   chroma_psel_pat,
        unsigned long   canvas0_addr0,
        unsigned long   canvas0_addr1,
        unsigned long   canvas0_addr2,
        unsigned long   canvas1_addr0,
        unsigned long   canvas1_addr1,
        unsigned long   canvas1_addr2);

// Allow fully configure VD2_RMEM module
extern void    set_vd2_if0 (
        unsigned long   luma_x_start0,
        unsigned long   luma_x_end0,
        unsigned long   luma_y_start0,
        unsigned long   luma_y_end0,
        unsigned long   chroma_x_start0,
        unsigned long   chroma_x_end0,
        unsigned long   chroma_y_start0,
        unsigned long   chroma_y_end0,
        unsigned long   luma_x_start1,
        unsigned long   luma_x_end1,
        unsigned long   luma_y_start1,
        unsigned long   luma_y_end1,
        unsigned long   chroma_x_start1,
        unsigned long   chroma_x_end1,
        unsigned long   chroma_y_start1,
        unsigned long   chroma_y_end1,
        unsigned long   mode,
        unsigned long   st_separate_en,
        unsigned long   luma_fifo_size,
        unsigned long   chro_rpt_lastl_ctrl,
        unsigned long   burst_size_y,
        unsigned long   burst_size_cb,
        unsigned long   burst_size_cr,
        unsigned long   luma0_rpt_loop_start,
        unsigned long   luma0_rpt_loop_end,
        unsigned long   luma0_rpt_pat,
        unsigned long   chroma0_rpt_loop_start,
        unsigned long   chroma0_rpt_loop_end,
        unsigned long   chroma0_rpt_pat,
        unsigned long   luma1_rpt_loop_start,
        unsigned long   luma1_rpt_loop_end,
        unsigned long   luma1_rpt_pat,
        unsigned long   chroma1_rpt_loop_start,
        unsigned long   chroma1_rpt_loop_end,
        unsigned long   chroma1_rpt_pat,
        unsigned long   luma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   luma_psel_loop_start,
        unsigned long   luma_psel_loop_end,
        unsigned long   luma_psel_pat,
        unsigned long   chroma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   chroma_psel_loop_start,
        unsigned long   chroma_psel_loop_end,
        unsigned long   chroma_psel_pat,
        unsigned long   canvas0_addr0,
        unsigned long   canvas0_addr1,
        unsigned long   canvas0_addr2,
        unsigned long   canvas1_addr0,
        unsigned long   canvas1_addr1,
        unsigned long   canvas1_addr2);

extern void set_vd2_fmt(
                int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length 
                );

extern void set_vd2_fmt_more (
                int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                );


extern void    set_vd2_if0_combined_simple(
               unsigned long   x_start,
               unsigned long   x_end,
               unsigned long   y_start,
               unsigned long   y_end,
               unsigned long   mode,    // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
               unsigned long   canvas_addr );      

extern void set_vd2_if0_combined(
        unsigned long   x_start,
        unsigned long   x_end,
        unsigned long   y_start,
        unsigned long   y_end,
        unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
        unsigned long   canvas_addr,
        unsigned long   pic_struct);     //00: frame, 10: top_field, 11: bot_field

extern void    set_vd2_if0_separate_simple(
        unsigned long   luma_x_start,
        unsigned long   luma_x_end,
        unsigned long   luma_y_start,
        unsigned long   luma_y_end,
        unsigned long   chroma_x_start,
        unsigned long   chroma_x_end,
        unsigned long   chroma_y_start,
        unsigned long   chroma_y_end,
        unsigned long   canvas_addr0,
        unsigned long   canvas_addr1,
        unsigned long   canvas_addr2 );

extern void set_viu2_vd1_fmt (int hfmt_en,
                              int hz_yc_ratio,     //2bit 
                              int hz_ini_phase,    //4bit
                              int vfmt_en,
                              int vt_yc_ratio,     //2bit
                              int vt_ini_phase,    //4bit
                              int y_length            
                              );

extern void set_viu2_vd1_fmt_more (
                int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                );

extern void    set_viu2_vd1_if0_combined_simple(
               unsigned long   x_start,
               unsigned long   x_end,
               unsigned long   y_start,
               unsigned long   y_end,
               unsigned long   mode,    // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
               unsigned long   canvas_addr );      

extern void set_viu2_vd1_if0 (
        unsigned long   luma_x_start0,
        unsigned long   luma_x_end0,
        unsigned long   luma_y_start0,
        unsigned long   luma_y_end0,
        unsigned long   chroma_x_start0,
        unsigned long   chroma_x_end0,
        unsigned long   chroma_y_start0,
        unsigned long   chroma_y_end0,
        unsigned long   luma_x_start1,
        unsigned long   luma_x_end1,
        unsigned long   luma_y_start1,
        unsigned long   luma_y_end1,
        unsigned long   chroma_x_start1,
        unsigned long   chroma_x_end1,
        unsigned long   chroma_y_start1,
        unsigned long   chroma_y_end1,
        unsigned long   mode,
        unsigned long   st_separate_en,
        unsigned long   luma_fifo_size,
        unsigned long   chro_rpt_lastl_ctrl,
        unsigned long   burst_size_y,
        unsigned long   burst_size_cb,
        unsigned long   burst_size_cr,
        unsigned long   luma0_rpt_loop_start,
        unsigned long   luma0_rpt_loop_end,
        unsigned long   luma0_rpt_pat,
        unsigned long   chroma0_rpt_loop_start,
        unsigned long   chroma0_rpt_loop_end,
        unsigned long   chroma0_rpt_pat,
        unsigned long   luma1_rpt_loop_start,
        unsigned long   luma1_rpt_loop_end,
        unsigned long   luma1_rpt_pat,
        unsigned long   chroma1_rpt_loop_start,
        unsigned long   chroma1_rpt_loop_end,
        unsigned long   chroma1_rpt_pat,
        unsigned long   luma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   luma_psel_loop_start,
        unsigned long   luma_psel_loop_end,
        unsigned long   luma_psel_pat,
        unsigned long   chroma_psel_mode, // 0=pic 0 only; 1=pic 1 only; 2=pic 0 and 1
        unsigned long   chroma_psel_loop_start,
        unsigned long   chroma_psel_loop_end,
        unsigned long   chroma_psel_pat,
        unsigned long   canvas0_addr0,
        unsigned long   canvas0_addr1,
        unsigned long   canvas0_addr2,
        unsigned long   canvas1_addr0,
        unsigned long   canvas1_addr1,
        unsigned long   canvas1_addr2
);


extern void vpp_set_ycbcr2rgb (int yc_full_range, int venc_no);
extern void vpp2_set_ycbcr2rgb (int yc_full_range, int venc_no);
extern void vpp_set_rgb2ycbcr (int yc_full_range);
extern void vpp_set_matrix_ycbcr2rgb (int vd1_or_vd2_or_post, int mode);
extern void vpp_set_matrix_rgb2ycbcr (int vd1_or_vd2_or_post, int mode);
extern void vpp_set_matrix_601to709 (int vd1_or_vd2_or_post, int mode);
extern void vpp_set_matrix_709to601 (int vd1_or_vd2_or_post, int mode);

// Programe Osd1's color palette
extern void osd1_write_color_lut (int           start_lut_addr,
                                  int           end_lut_addr,
                                  unsigned long lut_data[256]);
// Programe Osd2's color palette
extern void osd2_write_color_lut (int           start_lut_addr,
                                  int           end_lut_addr,
                                  unsigned long lut_data[256]);
// Configure Osd1 to display just one block, with color_matrix default to 0
extern void osd1_simple (unsigned long global_alpha, // range 0-256
                         unsigned long pix_x_start,
                         unsigned long pix_x_end,
                         unsigned long pix_y_start,
                         unsigned long pix_y_end,
                         unsigned long canvas_addr,
                         unsigned long display_h_start,
                         unsigned long display_v_start,
                         unsigned long interlace_en,
                         unsigned long interlace_sel_odd, // 1=Output odd lines,
                                                          // 0=Output even lines,
                                                          // Only valid if interlace_en=1
                         unsigned long tc_alpha_en, // 1=Enable alpha matching 4 alpha registers,
                                                    //   for 422,422 and 16-bit(color_matrix=0/1)
                                                    //   Note: C code is to turn white color into transparent
                                                    // 0=For 422,444 and 16-bit(color_matrix=0/1), output full alpha value
                         unsigned long rgb_en, // 1=Perform RGB-YUV conversion before output,
                                               // 0=No conversion performed
                         unsigned long osd_blk_mode // 0=4-color,
                                                    // 1=16-color,
                                                    // 2=256-color,
                                                    // 3=4:2:2
                                                    // 4=16-bit RGB655,
                                                    // 5=RGBA8888,
                                                    // 7=RGB888
                        );
// Configure Osd1 to display just one block, with color_matrix defined
extern void osd1_simple_more (unsigned long global_alpha, // range 0-256
                              unsigned long pix_x_start,
                              unsigned long pix_x_end,
                              unsigned long pix_y_start,
                              unsigned long pix_y_end,
                              unsigned long canvas_addr,
                              unsigned long display_h_start,
                              unsigned long display_v_start,
                              unsigned long interlace_en,
                              unsigned long interlace_sel_odd, // 1=Output odd lines,
                                                               // 0=Output even lines,
                                                               // Only valid if interlace_en=1
                              unsigned long tc_alpha_en, // 1=Enable alpha matching 4 alpha registers,
                                                         //   for 422,422 and 16-bit(color_matrix=0/1)
                                                         //   Note: C code is to turn white color into transparent
                                                         // 0=For 422,444 and 16-bit(color_matrix=0/1), output full alpha value
                              unsigned long rgb_en, // 1=Perform RGB-YUV conversion before output,
                                                    // 0=No conversion performed
                              unsigned long little_endian,
                              unsigned long color_matrix,
                              unsigned long osd_blk_mode // Together with color_matrix, define the input data format:
                                                         // osd_blk_mode=0: 4-color;
                                                         // osd_blk_mode=1: 16-color;
                                                         // osd_blk_mode=2: 256-color;
                                                         // osd_blk_mode=3: 4:2:2;
                                                         // osd_blk_mode=4, color_matrix=0: 16-bit RGB655;
                                                         // osd_blk_mode=4, color_matrix=1: 16-bit RGB844;
                                                         // osd_blk_mode=4, color_matrix=2: 16-bit RGBA6442;
                                                         // osd_blk_mode=4, color_matrix=3: 16-bit RGBA4444;
                                                         // osd_blk_mode=4, color_matrix=4: 16-bit RGB565;
                                                         // osd_blk_mode=4, color_matrix=5: 16-bit ARGB4444;
                                                         // osd_blk_mode=4, color_matrix=6: 16-bit ARGB1555;
                                                         // osd_blk_mode=5, color_matrix=0: 32-bit RGBA8888;
                                                         // osd_blk_mode=5, color_matrix=1: 32-bit ARGB8888;
                                                         // osd_blk_mode=5, color_matrix=2: 32-bit ABGR8888;
                                                         // osd_blk_mode=5, color_matrix=3: 32-bit BGRA8888;
                                                         // osd_blk_mode=7, color_matrix=0: 24-bit RGB888;
                                                         // osd_blk_mode=7, color_matrix=1: 24-bit RGBA5658;
                                                         // osd_blk_mode=7, color_matrix=2: 24-bit ARGB8565;
                                                         // osd_blk_mode=7, color_matrix=3: 24-bit RGBA6666;
                                                         // osd_blk_mode=7, color_matrix=4: 24-bit ARGB6666;
                                                         // osd_blk_mode=7, color_matrix=5: 24-bit BGR888.
                             );
// Configure Osd2 to display just one block, with color_matrix default to 0
extern void osd2_simple (unsigned long global_alpha, // range 0-256
                         unsigned long pix_x_start,
                         unsigned long pix_x_end,
                         unsigned long pix_y_start,
                         unsigned long pix_y_end,
                         unsigned long canvas_addr,
                         unsigned long display_h_start,
                         unsigned long display_v_start,
                         unsigned long interlace_en,
                         unsigned long interlace_sel_odd, // 1=Output odd lines,
                                                          // 0=Output even lines,
                                                          // Only valid if interlace_en=1
                         unsigned long tc_alpha_en, // 1=Enable alpha matching 4 alpha registers,
                                                    //   for 422,422 and 16-bit(color_matrix=0/1)
                                                    //   Note: C code is to turn white color into transparent
                                                    // 0=For 422,444 and 16-bit(color_matrix=0/1), output full alpha value
                         unsigned long rgb_en, // 1=Perform RGB-YUV conversion before output,
                                               // 0=No conversion performed
                         unsigned long osd_blk_mode // 0=4-color,
                                                    // 1=16-color,
                                                    // 2=256-color,
                                                    // 3=4:2:2
                                                    // 4=16-bit RGB655,
                                                    // 5=RGBA8888,
                                                    // 7=RGB888
                        );

// Configure Osd2 to display just one block, with color_matrix defined
extern void osd2_simple_more (unsigned long global_alpha, // range 0-256
                              unsigned long pix_x_start,
                              unsigned long pix_x_end,
                              unsigned long pix_y_start,
                              unsigned long pix_y_end,
                              unsigned long canvas_addr,
                              unsigned long display_h_start,
                              unsigned long display_v_start,
                              unsigned long interlace_en,
                              unsigned long interlace_sel_odd, // 1=Output odd lines,
                                                               // 0=Output even lines,
                                                               // Only valid if interlace_en=1
                              unsigned long tc_alpha_en, // 1=Enable alpha matching 4 alpha registers,
                                                         //   for 422,422 and 16-bit(color_matrix=0/1)
                                                         //   Note: C code is to turn white color into transparent
                                                         // 0=For 422,444 and 16-bit(color_matrix=0/1), output full alpha value
                              unsigned long rgb_en, // 1=Perform RGB-YUV conversion before output,
                                                    // 0=No conversion performed
                              unsigned long little_endian,
                              unsigned long color_matrix,
                              unsigned long osd_blk_mode // Together with color_matrix, define the input data format:
                                                         // osd_blk_mode=0: 4-color;
                                                         // osd_blk_mode=1: 16-color;
                                                         // osd_blk_mode=2: 256-color;
                                                         // osd_blk_mode=3: 4:2:2;
                                                         // osd_blk_mode=4, color_matrix=0: 16-bit RGB655;
                                                         // osd_blk_mode=4, color_matrix=1: 16-bit RGB844;
                                                         // osd_blk_mode=4, color_matrix=2: 16-bit RGBA6442;
                                                         // osd_blk_mode=4, color_matrix=3: 16-bit RGBA4444;
                                                         // osd_blk_mode=4, color_matrix=4: 16-bit RGB565;
                                                         // osd_blk_mode=4, color_matrix=5: 16-bit ARGB4444;
                                                         // osd_blk_mode=4, color_matrix=6: 16-bit ARGB1555;
                                                         // osd_blk_mode=5, color_matrix=0: 32-bit RGBA8888;
                                                         // osd_blk_mode=5, color_matrix=1: 32-bit ARGB8888;
                                                         // osd_blk_mode=5, color_matrix=2: 32-bit ABGR8888;
                                                         // osd_blk_mode=5, color_matrix=3: 32-bit BGRA8888;
                                                         // osd_blk_mode=7, color_matrix=0: 24-bit RGB888;
                                                         // osd_blk_mode=7, color_matrix=1: 24-bit RGBA5658;
                                                         // osd_blk_mode=7, color_matrix=2: 24-bit ARGB8565;
                                                         // osd_blk_mode=7, color_matrix=3: 24-bit RGBA6666;
                                                         // osd_blk_mode=7, color_matrix=4: 24-bit ARGB6666;
                                                         // osd_blk_mode=7, color_matrix=5: 24-bit BGR888.
                             );

extern void viu2_osd1_write_color_lut (int           start_lut_addr,
                                       int           end_lut_addr,
                                       unsigned long lut_data[256]);

extern void viu2_osd2_write_color_lut (int           start_lut_addr,
                                       int           end_lut_addr,
                                       unsigned long lut_data[256]);

extern void viu2_osd1_simple (unsigned long global_alpha,
                              unsigned long pix_x_start,
                              unsigned long pix_x_end,
                              unsigned long pix_y_start,
                              unsigned long pix_y_end,
                              unsigned long canvas_addr,
                              unsigned long display_h_start,
                              unsigned long display_v_start,
                              unsigned long interlace_en,
                              unsigned long interlace_sel_odd,
                              unsigned long tc_alpha_en,
                              unsigned long rgb_en,
                              unsigned long osd_blk_mode);

extern void viu2_osd1_simple_more (unsigned long global_alpha,
                                   unsigned long pix_x_start,
                                   unsigned long pix_x_end,
                                   unsigned long pix_y_start,
                                   unsigned long pix_y_end,
                                   unsigned long canvas_addr,
                                   unsigned long display_h_start,
                                   unsigned long display_v_start,
                                   unsigned long interlace_en,
                                   unsigned long interlace_sel_odd,
                                   unsigned long tc_alpha_en,
                                   unsigned long rgb_en,
                                   unsigned long little_endian,
                                   unsigned long color_matrix,
                                   unsigned long osd_blk_mode);

extern void viu2_osd2_simple (unsigned long global_alpha,
                              unsigned long pix_x_start,
                              unsigned long pix_x_end,
                              unsigned long pix_y_start,
                              unsigned long pix_y_end,
                              unsigned long canvas_addr,
                              unsigned long display_h_start,
                              unsigned long display_v_start,
                              unsigned long interlace_en,
                              unsigned long interlace_sel_odd,
                              unsigned long tc_alpha_en,
                              unsigned long rgb_en,
                              unsigned long osd_blk_mode);

extern void viu2_osd2_simple_more (unsigned long global_alpha,
                                   unsigned long pix_x_start,
                                   unsigned long pix_x_end,
                                   unsigned long pix_y_start,
                                   unsigned long pix_y_end,
                                   unsigned long canvas_addr,
                                   unsigned long display_h_start,
                                   unsigned long display_v_start,
                                   unsigned long interlace_en,
                                   unsigned long interlace_sel_odd,
                                   unsigned long tc_alpha_en,
                                   unsigned long rgb_en,
                                   unsigned long little_endian,
                                   unsigned long color_matrix,
                                   unsigned long osd_blk_mode);

extern void ldim_set_region(
                    int resolution,     //0: auto calc by height/width/row_start/col_start
                                        //1: 720p
                                        //2: 1080p
                    int blk_height,
                    int blk_width,
                    int row_start,
                    int col_start
);
extern void ldim_stts_en(
					    int resolution,
					    int hist_16bit_mode, 	  //1: output 16bit hist to reduce cbus read times
					    int eol_en,
     					int hist_mode,            //hist mode: 0: comp0 hist only, 1: Max(comp0,1,2) for hist, 2: the hist of all comp0,1,2 are calculated
					    int lpf_en,         	  //1: 1,2,1 filter on before finding max& hist
					    int rd_idx_auto_inc_mode  //0: no self increase, 
												  //1: read index increase after read a 25/48 block, 
												  //2: increases every read and lock sub-idx
						);
extern void ldim_read_region(int nrow, int ncol);
extern void vdin0_ldim_set_region();
extern void vdin0_ldim_stts_en();
extern void vdin0_ldim_read_region();
extern void ldim_set_matrix_rgb2ycbcr(int mode);
extern void vdin_ldim_bypass_mtx();
#endif /*DISPLAY_H */
