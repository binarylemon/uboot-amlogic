
//FILE: display.c
#include "register.h"
//#include "venc_regs_ucode.h"
#include "display.h"
//####################################################################################################
void set_vd1_fmt (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length            
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    int vfmt_w = (y_length >> hz_yc_ratio); 
            
    Wr(VIU_VD1_FMT_CTRL,      
                              (0 << 28)       |     //hz rpt pixel        
                              (hz_ini_phase << 24) |     //hz ini phase
                              (0 << 23)         |        //repeat p0 enable
                              (hz_yc_ratio << 21)  |     //hz yc ratio
                              (hfmt_en << 20)   |        //hz enable
                              (1 << 17)         |        //nrpt_phase0 enable
                              (0 << 16)         |        //repeat l0 enable
                              (0 << 12)         |        //skip line num
                              (vt_ini_phase << 8)  |     //vt ini phase
                              (vt_phase_step << 1) |     //vt phase step (3.4)
                              (vfmt_en << 0)             //vt enable
                              );
                    

    
    Wr(VIU_VD1_FMT_W,        (y_length << 16)        |        //hz format width
                             (vfmt_w << 0)                  //vt format width
                             );
        
} /* set_vd1_fmt */

//####################################################################################################
void set_vd1_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    Wr(VIU_VD1_FMT_CTRL,      
                              (hz_rpt << 28)       |     //hz rpt pixel        
                              (hz_ini_phase << 24) |     //hz ini phase
                              (0 << 23)         |        //repeat p0 enable
                              (hz_yc_ratio << 21)  |     //hz yc ratio
                              (hfmt_en << 20)   |        //hz enable
                              (1 << 17)         |        //nrpt_phase0 enable
                              (0 << 16)         |        //repeat l0 enable
                              (0 << 12)         |        //skip line num
                              (vt_ini_phase << 8)  |     //vt ini phase
                              (vt_phase_step << 1) |     //vt phase step (3.4)
                              (vfmt_en << 0)             //vt enable
                              );
    
    Wr(VIU_VD1_FMT_W,        (y_length << 16)        |        //hz format width
                             (c_length << 0)                  //vt format width
                             );
        
}


//####################################################################################################
void    set_vd1_if0_combined_simple(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr )      
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD1_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                |   // TODO: cntl_vt_yc_ratio
                                (0 << 2)                |   // TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD1_IF0_CANVAS0,         (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    // Double writing for decoding sync
    Wr(ASSIST_SPARE16_REG2,     (0 << 16)     | // cntl_canvas0_addr2
                                (0 << 8)      | // cntl_canvas0_addr1
                                (canvas_addr << 0)        // cntl_canvas0_addr0
    );

    Wr(VD1_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD1_IF0_LUMA_X0,         (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD1_IF0_LUMA_Y0,         (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD1_IF0_CHROMA_X0,     0);                            // unused
    Wr(VD1_IF0_CHROMA_Y0,     0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD1_IF0_LUMA_X1,       0);                            // unused
    Wr(VD1_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD1_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD1_IF0_CHROMA_Y1,     0);                            // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VD1_IF0_RPT_LOOP,        (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VD1_IF0_LUMA0_RPT_PAT,      0);                        // no skip /repeat
    Wr(VD1_IF0_CHROMA0_RPT_PAT,    0);                        // unused
    Wr(VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
    Wr(VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    Wr(VD1_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD1_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD1_IF0_DUMMY_PIXEL,   0x00808000);     

    // Enable VD1: vd_rmem_if0
    Wr(VD1_IF0_GEN_REG,      Rd(VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}


//####################################################################################################
void    set_vd1_if0_combined(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr,
unsigned long   pic_struct)     //00: frame, 10: top_field, 11: bot_field    
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD1_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (0 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD1_IF0_CANVAS0,         (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    // Double writing for decoding sync
    Wr(ASSIST_SPARE16_REG2,     (0 << 16)     | // cntl_canvas0_addr2
                                (0 << 8)      | // cntl_canvas0_addr1
                                (canvas_addr << 0)        // cntl_canvas0_addr0
    );

    Wr(VD1_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD1_IF0_LUMA_X0,         (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD1_IF0_LUMA_Y0,         (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD1_IF0_CHROMA_X0,     0);                            // unused
    Wr(VD1_IF0_CHROMA_Y0,     0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD1_IF0_LUMA_X1,       0);                            // unused
    Wr(VD1_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD1_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD1_IF0_CHROMA_Y1,     0);                            // unused

    if (pic_struct == 0) { //frame
        Wr(VD1_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VD1_IF0_LUMA0_RPT_PAT,      0x0);                        
        Wr(VD1_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    }
    else if (pic_struct == 3) { //bot_field
        Wr(VD1_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VD1_IF0_LUMA0_RPT_PAT,      0x8);                        
        Wr(VD1_IF0_CHROMA0_RPT_PAT,    0x0);                        // unused
        Wr(VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }
    else if (pic_struct == 2) //top_field
    {
       Wr(VD1_IF0_RPT_LOOP,     (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0x11 << 0)                 // cntl_luma0_rpt_loop
        ); 
        Wr(VD1_IF0_LUMA0_RPT_PAT,      0x80);                        
        Wr(VD1_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }

    Wr(VD1_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD1_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD1_IF0_DUMMY_PIXEL,   0x00808000);     

    // Enable VD1: vd_rmem_if0
    Wr(VD1_IF0_GEN_REG,      Rd(VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}


//####################################################################################################
void    set_vd1_if0_separate_simple(
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
        unsigned long   canvas_addr2
)
{
    // General register setup
    unsigned long   bytes_per_pixel = 0;    // 1Byte per  
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 1;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD1_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (0 << 16)               |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (1 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD1_IF0_CANVAS0,         (canvas_addr2 << 16)     |   // cntl_canvas0_addr2
                                (canvas_addr1 << 8)      |   // cntl_canvas0_addr1
                                (canvas_addr0 << 0)          // cntl_canvas0_addr0
    );

    // Double writing for decoding sync
    Wr(ASSIST_SPARE16_REG2,     (canvas_addr2 << 16)     | // cntl_canvas0_addr2
                                (canvas_addr1 << 8)      | // cntl_canvas0_addr1
                                (canvas_addr0 << 0)        // cntl_canvas0_addr0
    );

    Wr(VD1_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD1_IF0_LUMA_X0,         (luma_x_end << 16)           |   // cntl_luma_x_end0
                                (luma_x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD1_IF0_LUMA_Y0,         (luma_y_end << 16)           |   // cntl_luma_y_end0
                                (luma_y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD1_IF0_CHROMA_X0,     (chroma_x_end << 16)      |
                              (chroma_x_start << 0)
    );                           
    Wr(VD1_IF0_CHROMA_Y0,     (chroma_y_end << 16)      |
                              (chroma_y_start << 0)
    );                           

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD1_IF0_LUMA_X1,       0);                            // unused
    Wr(VD1_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD1_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD1_IF0_CHROMA_Y1,     0);                            // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VD1_IF0_RPT_LOOP,        (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VD1_IF0_LUMA0_RPT_PAT,      0);                        // no skip /repeat
    Wr(VD1_IF0_CHROMA0_RPT_PAT,    0);                        // unused
    Wr(VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
    Wr(VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    Wr(VD1_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD1_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD1_IF0_DUMMY_PIXEL,   0x00808000); 

    // Enable VD1: vd_rmem_if0
    Wr(VD1_IF0_GEN_REG,      Rd(VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}

//####################################################################################################
void    set_vd1_if0 (
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
)
{
    unsigned long bytes_per_pixel;  
    
    bytes_per_pixel = st_separate_en ? 0 : (mode ? 2 : 1);

    // ----------------------
    // General register
    // ----------------------

    Wr(VD1_IF0_GEN_REG,         (4 << 19)                  | // hold lines
                                (1 << 18)                  | // push dummy pixel
                                (mode << 16)               | // demux_mode
                                (bytes_per_pixel << 14)    | 
                                (burst_size_cr << 12)      |
                                (burst_size_cb << 10)      |
                                (burst_size_y << 8)        |
                                (chro_rpt_lastl_ctrl << 6) |
                                (st_separate_en << 1)      |
                                (0 << 0)                     // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD1_IF0_CANVAS0,         (canvas0_addr2 << 16)     | // cntl_canvas0_addr2
                                (canvas0_addr1 << 8)      | // cntl_canvas0_addr1
                                (canvas0_addr0 << 0)        // cntl_canvas0_addr0
    );

    // Double writing for decoding sync
    Wr(ASSIST_SPARE16_REG2,     (canvas0_addr2 << 16)     | // cntl_canvas0_addr2
                                (canvas0_addr1 << 8)      | // cntl_canvas0_addr1
                                (canvas0_addr0 << 0)        // cntl_canvas0_addr0
    );

    Wr(VD1_IF0_CANVAS1,         (canvas1_addr2 << 16)     | // cntl_canvas1_addr2
                                (canvas1_addr1 << 8)      | // cntl_canvas1_addr1
                                (canvas1_addr0 << 0)        // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD1_IF0_LUMA_X0,         (luma_x_end0 << 16)       | // cntl_luma_x_end0
                                (luma_x_start0 << 0)        // cntl_luma_x_start0
    );
    Wr(VD1_IF0_LUMA_Y0,         (luma_y_end0 << 16)       | // cntl_luma_y_end0
                                (luma_y_start0 << 0)        // cntl_luma_y_start0
    );
    Wr(VD1_IF0_CHROMA_X0,     (chroma_x_end0 << 16)      |
                              (chroma_x_start0 << 0)
    );                           
    Wr(VD1_IF0_CHROMA_Y0,     (chroma_y_end0 << 16)      |
                              (chroma_y_start0 << 0)
    );                           
                   
    // ----------------------
    // Picture 1 X/Y start,end
    // ----------------------
    Wr(VD1_IF0_LUMA_X1,         (luma_x_end1 << 16)       | // cntl_luma_x_end1
                                (luma_x_start1 << 0)        // cntl_luma_x_start1
    );
    Wr(VD1_IF0_LUMA_Y1,         (luma_y_end1 << 16)       | // cntl_luma_y_end1
                                (luma_y_start1 << 0)        // cntl_luma_y_start1
    );
    Wr(VD1_IF0_CHROMA_X1,     (chroma_x_end1 << 16)      |
                              (chroma_x_start1 << 0)
    );                           
    Wr(VD1_IF0_CHROMA_Y1,     (chroma_y_end1 << 16)      |
                              (chroma_y_start1 << 0)
    );

    // ----------------------
    // Repeat or skip
    // ----------------------
    Wr(VD1_IF0_RPT_LOOP,        (chroma1_rpt_loop_start << 28) |
                                (chroma1_rpt_loop_end   << 24) |
                                (luma1_rpt_loop_start   << 20) |
                                (luma1_rpt_loop_end     << 16) |
                                (chroma0_rpt_loop_start << 12) |
                                (chroma0_rpt_loop_end   << 8)  |
                                (luma0_rpt_loop_start   << 4)  |
                                (luma0_rpt_loop_end     << 0)
    ); 

    Wr(VD1_IF0_LUMA0_RPT_PAT,      luma0_rpt_pat);
    Wr(VD1_IF0_CHROMA0_RPT_PAT,    chroma0_rpt_pat);
    
    Wr(VD1_IF0_LUMA1_RPT_PAT,      luma1_rpt_pat);
    Wr(VD1_IF0_CHROMA1_RPT_PAT,    chroma1_rpt_pat);

    // ----------------------
    // Picture select/toggle
    // ----------------------
    Wr(VD1_IF0_LUMA_PSEL,       (luma_psel_mode       << 26) |
                                (0                    << 24) | // psel_last_line
                                (luma_psel_pat        << 8)  |
                                (luma_psel_loop_start << 4)  |
                                (luma_psel_loop_end   << 0)
    ); 
    Wr(VD1_IF0_CHROMA_PSEL,     (chroma_psel_mode       << 26) |
                                (0                      << 24) | // psel_last_line
                                (chroma_psel_pat        << 8)  |
                                (chroma_psel_loop_start << 4)  |
                                (chroma_psel_loop_end   << 0)
    ); 

    // Dummy pixel value
    Wr(VD1_IF0_DUMMY_PIXEL,   0x00808000); 
    
    // Depth of FIFO when stored together
    Wr(VD1_IF0_LUMA_FIFO_SIZE, luma_fifo_size);

    // Enable VD1: vd_rmem_if0
    Wr(VD1_IF0_GEN_REG,      Rd(VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable
} /* set_vd1_if0 */

//####################################################################################################
void    set_vd2_if0 (
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
)
{
    unsigned long bytes_per_pixel;  
    
    bytes_per_pixel = st_separate_en ? 0 : (mode ? 2 : 1);

    // ----------------------
    // General register
    // ----------------------

    Wr(VD2_IF0_GEN_REG,         (4 << 19)                  | // hold lines
                                (1 << 18)                  | // push dummy pixel
                                (mode << 16)               | // demux_mode
                                (bytes_per_pixel << 14)    | 
                                (burst_size_cr << 12)      |
                                (burst_size_cb << 10)      |
                                (burst_size_y << 8)        |
                                (chro_rpt_lastl_ctrl << 6) |
                                (st_separate_en << 1)      |
                                (0 << 0)                     // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD2_IF0_CANVAS0,         (canvas0_addr2 << 16)     | // cntl_canvas0_addr2
                                (canvas0_addr1 << 8)      | // cntl_canvas0_addr1
                                (canvas0_addr0 << 0)        // cntl_canvas0_addr0
    );

    Wr(VD2_IF0_CANVAS1,         (canvas1_addr2 << 16)     | // cntl_canvas1_addr2
                                (canvas1_addr1 << 8)      | // cntl_canvas1_addr1
                                (canvas1_addr0 << 0)        // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD2_IF0_LUMA_X0,         (luma_x_end0 << 16)       | // cntl_luma_x_end0
                                (luma_x_start0 << 0)        // cntl_luma_x_start0
    );
    Wr(VD2_IF0_LUMA_Y0,         (luma_y_end0 << 16)       | // cntl_luma_y_end0
                                (luma_y_start0 << 0)        // cntl_luma_y_start0
    );
    Wr(VD2_IF0_CHROMA_X0,     (chroma_x_end0 << 16)      |
                              (chroma_x_start0 << 0)
    );                           
    Wr(VD2_IF0_CHROMA_Y0,     (chroma_y_end0 << 16)      |
                              (chroma_y_start0 << 0)
    );                           
                   
    // ----------------------
    // Picture 1 X/Y start,end
    // ----------------------
    Wr(VD2_IF0_LUMA_X1,         (luma_x_end1 << 16)       | // cntl_luma_x_end1
                                (luma_x_start1 << 0)        // cntl_luma_x_start1
    );
    Wr(VD2_IF0_LUMA_Y1,         (luma_y_end1 << 16)       | // cntl_luma_y_end1
                                (luma_y_start1 << 0)        // cntl_luma_y_start1
    );
    Wr(VD2_IF0_CHROMA_X1,     (chroma_x_end1 << 16)      |
                              (chroma_x_start1 << 0)
    );                           
    Wr(VD2_IF0_CHROMA_Y1,     (chroma_y_end1 << 16)      |
                              (chroma_y_start1 << 0)
    );

    // ----------------------
    // Repeat or skip
    // ----------------------
    Wr(VD2_IF0_RPT_LOOP,        (chroma1_rpt_loop_start << 28) |
                                (chroma1_rpt_loop_end   << 24) |
                                (luma1_rpt_loop_start   << 20) |
                                (luma1_rpt_loop_end     << 16) |
                                (chroma0_rpt_loop_start << 12) |
                                (chroma0_rpt_loop_end   << 8)  |
                                (luma0_rpt_loop_start   << 4)  |
                                (luma0_rpt_loop_end     << 0)
    ); 

    Wr(VD2_IF0_LUMA0_RPT_PAT,      luma0_rpt_pat);
    Wr(VD2_IF0_CHROMA0_RPT_PAT,    chroma0_rpt_pat);
    
    Wr(VD2_IF0_LUMA1_RPT_PAT,      luma1_rpt_pat);
    Wr(VD2_IF0_CHROMA1_RPT_PAT,    chroma1_rpt_pat);

    // ----------------------
    // Picture select/toggle
    // ----------------------
    Wr(VD2_IF0_LUMA_PSEL,       (luma_psel_mode       << 26) |
                                (0                    << 24) | // psel_last_line
                                (luma_psel_pat        << 8)  |
                                (luma_psel_loop_start << 4)  |
                                (luma_psel_loop_end   << 0)
    ); 
    Wr(VD2_IF0_CHROMA_PSEL,     (chroma_psel_mode       << 26) |
                                (0                      << 24) | // psel_last_line
                                (chroma_psel_pat        << 8)  |
                                (chroma_psel_loop_start << 4)  |
                                (chroma_psel_loop_end   << 0)
    ); 

    // Dummy pixel value
    Wr(VD2_IF0_DUMMY_PIXEL,   0x00808000); 
    
    // Depth of FIFO when stored together
    Wr(VD2_IF0_LUMA_FIFO_SIZE, luma_fifo_size);

    // Enable VD2: vd_rmem_if0
    Wr(VD2_IF0_GEN_REG,      Rd(VD2_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable
} /* set_vd2_if0 */

//####################################################################################################
void set_vd2_fmt (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length            
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    int vfmt_w = (y_length >> hz_yc_ratio); 
            
    Wr(VIU_VD2_FMT_CTRL,      
                              (0 << 28)       |     //hz rpt pixel        
                              (hz_ini_phase << 24) |     //hz ini phase
                              (0 << 23)         |        //repeat p0 enable
                              (hz_yc_ratio << 21)  |     //hz yc ratio
                              (hfmt_en << 20)   |        //hz enable
                              (1 << 17)         |        //nrpt_phase0 enable
                              (0 << 16)         |        //repeat l0 enable
                              (0 << 12)         |        //skip line num
                              (vt_ini_phase << 8)  |     //vt ini phase
                              (vt_phase_step << 1) |     //vt phase step (3.4)
                              (vfmt_en << 0)             //vt enable
                              );
                    

    
    Wr(VIU_VD2_FMT_W,        (y_length << 16)        |        //hz format width
                             (vfmt_w << 0)                  //vt format width
                             );
        
}

//####################################################################################################
void set_vd2_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    Wr(VIU_VD2_FMT_CTRL,      
                              (hz_rpt << 28)       |     //hz rpt pixel        
                              (hz_ini_phase << 24) |     //hz ini phase
                              (0 << 23)         |        //repeat p0 enable
                              (hz_yc_ratio << 21)  |     //hz yc ratio
                              (hfmt_en << 20)   |        //hz enable
                              (1 << 17)         |        //nrpt_phase0 enable
                              (0 << 16)         |        //repeat l0 enable
                              (0 << 12)         |        //skip line num
                              (vt_ini_phase << 8)  |     //vt ini phase
                              (vt_phase_step << 1) |     //vt phase step (3.4)
                              (vfmt_en << 0)             //vt enable
                              );
    
    Wr(VIU_VD2_FMT_W,        (y_length << 16)        |        //hz format width
                             (c_length << 0)                  //vt format width
                             );
        
}


//####################################################################################################
void    set_vd2_if0_combined_simple(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr )      
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD2_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD2_IF0_CANVAS0,         (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    Wr(VD2_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD2_IF0_LUMA_X0,         (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD2_IF0_LUMA_Y0,         (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD2_IF0_CHROMA_X0,     0);                            // unused
    Wr(VD2_IF0_CHROMA_Y0,     0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD2_IF0_LUMA_X1,       0);                            // unused
    Wr(VD2_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD2_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD2_IF0_CHROMA_Y1,     0);                            // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VD2_IF0_RPT_LOOP,        (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VD2_IF0_LUMA0_RPT_PAT,      0);                        // no skip /repeat
    Wr(VD2_IF0_CHROMA0_RPT_PAT,    0);                        // unused
    Wr(VD2_IF0_LUMA1_RPT_PAT,      0);                        // unused
    Wr(VD2_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    Wr(VD2_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD2_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD2_IF0_DUMMY_PIXEL,   0x00808000);     

    // Enable VD2: vd_rmem_if0
    Wr(VD2_IF0_GEN_REG,      Rd(VD2_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}

//####################################################################################################
void    set_vd2_if0_combined(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr,
unsigned long   pic_struct)     //00: frame, 10: top_field, 11: bot_field    
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD2_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (0 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD2_IF0_CANVAS0,         (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    Wr(VD2_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD2_IF0_LUMA_X0,         (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD2_IF0_LUMA_Y0,         (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD2_IF0_CHROMA_X0,     0);                            // unused
    Wr(VD2_IF0_CHROMA_Y0,     0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD2_IF0_LUMA_X1,       0);                            // unused
    Wr(VD2_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD2_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD2_IF0_CHROMA_Y1,     0);                            // unused

    if (pic_struct == 0) { //frame
        Wr(VD2_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VD2_IF0_LUMA0_RPT_PAT,      0x0);                        
        Wr(VD2_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VD2_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD2_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    }
    else if (pic_struct == 3) { //bot_field
        Wr(VD2_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VD2_IF0_LUMA0_RPT_PAT,      0x8);                        
        Wr(VD2_IF0_CHROMA0_RPT_PAT,    0x0);                        // unused
        Wr(VD2_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD2_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }
    else if (pic_struct == 2) //top_field
    {
       Wr(VD2_IF0_RPT_LOOP,     (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0x11 << 0)                 // cntl_luma0_rpt_loop
        ); 
        Wr(VD2_IF0_LUMA0_RPT_PAT,      0x80);                        
        Wr(VD2_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VD2_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VD2_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }

    Wr(VD2_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD2_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD2_IF0_DUMMY_PIXEL,   0x00808000);     

    // Enable VD2: vd_rmem_if0
    Wr(VD2_IF0_GEN_REG,      Rd(VD2_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}


//####################################################################################################
void    set_vd2_if0_separate_simple(
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
        unsigned long   canvas_addr2
)
{
    // General register setup
    unsigned long   bytes_per_pixel = 0;    // 1Byte per  
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 1;

    // ----------------------
    // General register
    // ----------------------

    Wr(VD2_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (0 << 16)               |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (1 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VD2_IF0_CANVAS0,         (canvas_addr2 << 16)     |   // cntl_canvas0_addr2
                                (canvas_addr1 << 8)      |   // cntl_canvas0_addr1
                                (canvas_addr0 << 0)          // cntl_canvas0_addr0
    );

    Wr(VD2_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VD2_IF0_LUMA_X0,         (luma_x_end << 16)           |   // cntl_luma_x_end0
                                (luma_x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VD2_IF0_LUMA_Y0,         (luma_y_end << 16)           |   // cntl_luma_y_end0
                                (luma_y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VD2_IF0_CHROMA_X0,     (chroma_x_end << 16)      |
                              (chroma_x_start << 0)
    );                           
    Wr(VD2_IF0_CHROMA_Y0,     (chroma_y_end << 16)      |
                              (chroma_y_start << 0)
    );                           

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VD2_IF0_LUMA_X1,       0);                            // unused
    Wr(VD2_IF0_LUMA_Y1,       0);                            // unused
    Wr(VD2_IF0_CHROMA_X1,     0);                            // unused
    Wr(VD2_IF0_CHROMA_Y1,     0);                            // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VD2_IF0_RPT_LOOP,        (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VD2_IF0_LUMA0_RPT_PAT,      0);                        // no skip /repeat
    Wr(VD2_IF0_CHROMA0_RPT_PAT,    0);                        // unused
    Wr(VD2_IF0_LUMA1_RPT_PAT,      0);                        // unused
    Wr(VD2_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    Wr(VD2_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VD2_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VD2_IF0_DUMMY_PIXEL,   0x00808000); 

    // Enable VD2: vd_rmem_if0
    Wr(VD2_IF0_GEN_REG,      Rd(VD2_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}

//####################################################################################################
void set_viu2_vd1_fmt (int hfmt_en,
                       int hz_yc_ratio,     //2bit 
                       int hz_ini_phase,    //4bit
                       int vfmt_en,
                       int vt_yc_ratio,     //2bit
                       int vt_ini_phase,    //4bit
                       int y_length            
                       )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    int vfmt_w = (y_length >> hz_yc_ratio); 
            
    Wr(VIU2_VD1_FMT_CTRL,      
                              (0 << 28)             |   //hz rpt pixel        
                              (hz_ini_phase << 24)  |   //hz ini phase
                              (0 << 23)             |   //repeat p0 enable
                              (hz_yc_ratio << 21)   |   //hz yc ratio
                              (hfmt_en << 20)       |   //hz enable
                              (1 << 17)             |   //nrpt_phase0 enable
                              (0 << 16)             |   //repeat l0 enable
                              (0 << 12)             |   //skip line num
                              (vt_ini_phase << 8)   |   //vt ini phase
                              (vt_phase_step << 1)  |   //vt phase step (3.4)
                              (vfmt_en << 0)            //vt enable
                              );
                    

    
    Wr(VIU2_VD1_FMT_W,       (y_length << 16)       |   //hz format width
                             (vfmt_w << 0)              //vt format width
                             );
        
} /* set_viu2_vd1_fmt */

//####################################################################################################
void set_viu2_vd1_fmt_more (int hfmt_en,
                int hz_yc_ratio,        //2bit 
                int hz_ini_phase,       //4bit
                int vfmt_en,
                int vt_yc_ratio,        //2bit
                int vt_ini_phase,       //4bit
                int y_length,
                int c_length,
                int hz_rpt              //1bit
                )
{
    int vt_phase_step = (16 >> vt_yc_ratio);  
    Wr(VIU2_VD1_FMT_CTRL,      
                              (hz_rpt << 28)       |     //hz rpt pixel        
                              (hz_ini_phase << 24) |     //hz ini phase
                              (0 << 23)         |        //repeat p0 enable
                              (hz_yc_ratio << 21)  |     //hz yc ratio
                              (hfmt_en << 20)   |        //hz enable
                              (1 << 17)         |        //nrpt_phase0 enable
                              (0 << 16)         |        //repeat l0 enable
                              (0 << 12)         |        //skip line num
                              (vt_ini_phase << 8)  |     //vt ini phase
                              (vt_phase_step << 1) |     //vt phase step (3.4)
                              (vfmt_en << 0)             //vt enable
                              );
    
    Wr(VIU2_VD1_FMT_W,       (y_length << 16)        |        //hz format width
                             (c_length << 0)                  //vt format width
                             );
        
} /* set_viu2_vd1_fmt_more */

//####################################################################################################
void    set_viu2_vd1_if0_combined_simple(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr )      
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VIU2_VD1_IF0_GEN_REG,    (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (0 << 4)                |   // TODO: cntl_vt_yc_ratio
                                (0 << 2)                |   // TODO: cntl_hz_yc_ratio
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VIU2_VD1_IF0_CANVAS0,    (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    Wr(VIU2_VD1_IF0_CANVAS1,    (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X0,    (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VIU2_VD1_IF0_LUMA_Y0,    (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VIU2_VD1_IF0_CHROMA_X0,  0);                         // unused
    Wr(VIU2_VD1_IF0_CHROMA_Y0,  0);                         // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X1,    0);                         // unused
    Wr(VIU2_VD1_IF0_LUMA_Y1,    0);                         // unused
    Wr(VIU2_VD1_IF0_CHROMA_X1,  0);                         // unused
    Wr(VIU2_VD1_IF0_CHROMA_Y1,  0);                         // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VIU2_VD1_IF0_RPT_LOOP,   (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      0);                 // no skip /repeat
    Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    0);                 // unused
    Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      0);                 // unused
    Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    0);                 // unused

    Wr(VIU2_VD1_IF0_LUMA_PSEL,          0);                 // unused only one picture 
    Wr(VIU2_VD1_IF0_CHROMA_PSEL,        0);                 // unused only one picture 


    Wr(VIU2_VD1_IF0_DUMMY_PIXEL,        0x00808000);     

    // Enable VD1: vd_rmem_if0
    Wr(VIU2_VD1_IF0_GEN_REG,    Rd(VIU2_VD1_IF0_GEN_REG) |  (1 << 0));  // cntl_enable

} /* set_viu2_vd1_if0_combined_simple */

//####################################################################################################
void    set_viu2_vd1_if0_combined(
unsigned long   x_start,
unsigned long   x_end,
unsigned long   y_start,
unsigned long   y_end,
unsigned long   mode,           // 1 = RGB/YCBCR (3 bytes per pixel), 0 = 4:2:2 (2 bytes per pixel)
unsigned long   canvas_addr,
unsigned long   pic_struct)     //00: frame, 10: top_field, 11: bot_field    
{

    // General register setup
    unsigned long   demux_mode      = mode; // 0 = 4:2:2 demux, 1 = RGB demuxing from a single FIFO
    unsigned long   bytes_per_pixel = (mode  == 1) ? 2 : 1;     // RGB = 3 bytes per 
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 0;

    // ----------------------
    // General register
    // ----------------------

    Wr(VIU2_VD1_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (0 << 18)               |   // push pixel value
                                (demux_mode << 16)      |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (0 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VIU2_VD1_IF0_CANVAS0,         (0 << 16)               |   // cntl_canvas0_addr2
                                (0 << 8)                |   // cntl_canvas0_addr1
                                (canvas_addr << 0)          // cntl_canvas0_addr0
    );

    Wr(VIU2_VD1_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X0,         (x_end << 16)           |   // cntl_luma_x_end0
                                (x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VIU2_VD1_IF0_LUMA_Y0,         (y_end << 16)           |   // cntl_luma_y_end0
                                (y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VIU2_VD1_IF0_CHROMA_X0,     0);                            // unused
    Wr(VIU2_VD1_IF0_CHROMA_Y0,     0);                            // unused

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X1,       0);                            // unused
    Wr(VIU2_VD1_IF0_LUMA_Y1,       0);                            // unused
    Wr(VIU2_VD1_IF0_CHROMA_X1,     0);                            // unused
    Wr(VIU2_VD1_IF0_CHROMA_Y1,     0);                            // unused

    if (pic_struct == 0) { //frame
        Wr(VIU2_VD1_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      0x0);                        
        Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    }
    else if (pic_struct == 3) { //bot_field
        Wr(VIU2_VD1_IF0_RPT_LOOP,    (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
        ); 
        Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      0x8);                        
        Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    0x0);                        // unused
        Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }
    else if (pic_struct == 2) //top_field
    {
       Wr(VIU2_VD1_IF0_RPT_LOOP,     (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0x11 << 0)                 // cntl_luma0_rpt_loop
        ); 
        Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      0x80);                        
        Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    0x0);                      // unused
        Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
        Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused
    }

    Wr(VIU2_VD1_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VIU2_VD1_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VIU2_VD1_IF0_DUMMY_PIXEL,   0x00808000);     

    // Enable VIU2_VD1: vd_rmem_if0
    Wr(VIU2_VD1_IF0_GEN_REG,      Rd(VIU2_VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}


//####################################################################################################
void    set_viu2_vd1_if0 (
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
)
{
    unsigned long bytes_per_pixel;  
    
    bytes_per_pixel = st_separate_en ? 0 : (mode ? 2 : 1);

    // ----------------------
    // General register
    // ----------------------

    Wr(VIU2_VD1_IF0_GEN_REG,    (4 << 19)                  | // hold lines
                                (1 << 18)                  | // push dummy pixel
                                (mode << 16)               | // demux_mode
                                (bytes_per_pixel << 14)    | 
                                (burst_size_cr << 12)      |
                                (burst_size_cb << 10)      |
                                (burst_size_y << 8)        |
                                (chro_rpt_lastl_ctrl << 6) |
                                (st_separate_en << 1)      |
                                (0 << 0)                     // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VIU2_VD1_IF0_CANVAS0,    (canvas0_addr2 << 16)     | // cntl_canvas0_addr2
                                (canvas0_addr1 << 8)      | // cntl_canvas0_addr1
                                (canvas0_addr0 << 0)        // cntl_canvas0_addr0
    );

    Wr(VIU2_VD1_IF0_CANVAS1,    (canvas1_addr2 << 16)     | // cntl_canvas1_addr2
                                (canvas1_addr1 << 8)      | // cntl_canvas1_addr1
                                (canvas1_addr0 << 0)        // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X0,    (luma_x_end0 << 16)       | // cntl_luma_x_end0
                                (luma_x_start0 << 0)        // cntl_luma_x_start0
    );
    Wr(VIU2_VD1_IF0_LUMA_Y0,    (luma_y_end0 << 16)       | // cntl_luma_y_end0
                                (luma_y_start0 << 0)        // cntl_luma_y_start0
    );
    Wr(VIU2_VD1_IF0_CHROMA_X0,  (chroma_x_end0 << 16)     |
                                (chroma_x_start0 << 0)
    );                           
    Wr(VIU2_VD1_IF0_CHROMA_Y0,  (chroma_y_end0 << 16)     |
                                (chroma_y_start0 << 0)
    );                           
                   
    // ----------------------
    // Picture 1 X/Y start,end
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X1,    (luma_x_end1 << 16)       | // cntl_luma_x_end1
                                (luma_x_start1 << 0)        // cntl_luma_x_start1
    );
    Wr(VIU2_VD1_IF0_LUMA_Y1,    (luma_y_end1 << 16)       | // cntl_luma_y_end1
                                (luma_y_start1 << 0)        // cntl_luma_y_start1
    );
    Wr(VIU2_VD1_IF0_CHROMA_X1,  (chroma_x_end1 << 16)     |
                                (chroma_x_start1 << 0)
    );                           
    Wr(VIU2_VD1_IF0_CHROMA_Y1,  (chroma_y_end1 << 16)     |
                                (chroma_y_start1 << 0)
    );

    // ----------------------
    // Repeat or skip
    // ----------------------
    Wr(VIU2_VD1_IF0_RPT_LOOP,   (chroma1_rpt_loop_start << 28) |
                                (chroma1_rpt_loop_end   << 24) |
                                (luma1_rpt_loop_start   << 20) |
                                (luma1_rpt_loop_end     << 16) |
                                (chroma0_rpt_loop_start << 12) |
                                (chroma0_rpt_loop_end   << 8)  |
                                (luma0_rpt_loop_start   << 4)  |
                                (luma0_rpt_loop_end     << 0)
    ); 

    Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      luma0_rpt_pat);
    Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    chroma0_rpt_pat);
    
    Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      luma1_rpt_pat);
    Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    chroma1_rpt_pat);

    // ----------------------
    // Picture select/toggle
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_PSEL,  (luma_psel_mode       << 26) |
                                (0                    << 24) | // psel_last_line
                                (luma_psel_pat        << 8)  |
                                (luma_psel_loop_start << 4)  |
                                (luma_psel_loop_end   << 0)
    ); 
    Wr(VIU2_VD1_IF0_CHROMA_PSEL,(chroma_psel_mode       << 26) |
                                (0                      << 24) | // psel_last_line
                                (chroma_psel_pat        << 8)  |
                                (chroma_psel_loop_start << 4)  |
                                (chroma_psel_loop_end   << 0)
    ); 

    // Dummy pixel value
    Wr(VIU2_VD1_IF0_DUMMY_PIXEL,    0x00808000); 
    
    // Depth of FIFO when stored together
    Wr(VIU2_VD1_IF0_LUMA_FIFO_SIZE, luma_fifo_size);

    // Enable VD1: vd_rmem_if0
    Wr(VIU2_VD1_IF0_GEN_REG,    Rd(VIU2_VD1_IF0_GEN_REG) |  (1 << 0));   // cntl_enable
} /* set_viu2_vd1_if0 */


//####################################################################################################

void vpp_set_ycbcr2rgb (int yc_full_range, int venc_no)
{
   if (!yc_full_range)
   {
        //Wr(VPP_MATRIX_CTRL,    1 << 7 |
        //                       1 << 6 |
        //                       0 << 5 |
        //                       0 << 4 |
        //                       0 << 3 |
        //                       1 << 2 |
        //                       1 << 1 |
        //                       1);

        Wr_reg_bits (VPP_MATRIX_CTRL, 3, 0, 3);
        Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);

        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x7c00600);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0600);
        
        Wr(VPP_MATRIX_COEF00_01, (0x4a8 << 16) |
                            0);
        Wr(VPP_MATRIX_COEF02_10, (0x662 << 16) |
                            0x4a8);
        Wr(VPP_MATRIX_COEF11_12, (0x1e6f << 16) |
                            0x1cbf);
        Wr(VPP_MATRIX_COEF20_21, (0x4a8 << 16) | 
                            0x811);
        Wr(VPP_MATRIX_COEF22, 0x0);
        Wr(VPP_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_OFFSET2, 0x0);

        if(venc_no == 3) Wr(ENCT_VIDEO_RGBIN_CTRL, 3);
        else Wr(ENCP_VIDEO_RGBIN_CTRL, 3);

        Wr(RGB_BASE_ADDR, 0);
        Wr(RGB_COEFF_ADDR, 0x400);
   }
   else 
   {
   }
   
} /* vpp_set_ycbcr2rgb */

void vpp2_set_ycbcr2rgb (int yc_full_range, int venc_no)
{
   if (!yc_full_range)
   {
        //Wr(VPP2_MATRIX_CTRL,   1 << 7 |
        //                       1 << 6 |
        //                       0 << 5 |
        //                       0 << 4 |
        //                       0 << 3 |
        //                       1 << 2 |
        //                       1 << 1 |
        //                       1);

        Wr_reg_bits (VPP2_MATRIX_CTRL, 3, 0, 3);
        Wr_reg_bits (VPP2_MATRIX_CTRL, 0, 8, 2);

        Wr(VPP2_MATRIX_PRE_OFFSET0_1, 0x7c00600);
        Wr(VPP2_MATRIX_PRE_OFFSET2, 0x0600);
        
        Wr(VPP2_MATRIX_COEF00_01, (0x4a8 << 16) |
                            0);
        Wr(VPP2_MATRIX_COEF02_10, (0x662 << 16) |
                            0x4a8);
        Wr(VPP2_MATRIX_COEF11_12, (0x1e6f << 16) |
                            0x1cbf);
        Wr(VPP2_MATRIX_COEF20_21, (0x4a8 << 16) | 
                            0x811);
        Wr(VPP2_MATRIX_COEF22, 0x0);
        Wr(VPP2_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP2_MATRIX_OFFSET2, 0x0);

        if(venc_no == 3) Wr(ENCT_VIDEO_RGBIN_CTRL, 3);
        else Wr(ENCP_VIDEO_RGBIN_CTRL, 3);

        Wr(RGB_BASE_ADDR, 0);
        Wr(RGB_COEFF_ADDR, 0x400);
   }
   else 
   {
   }
   
} /* vpp2_set_ycbcr2rgb */

void vpp_set_rgb2ycbcr (int yc_full_range)
{
   if (!yc_full_range)
   {
        //Wr(VPP_MATRIX_CTRL,    0 << 7 |
        //                       0 << 6 |
        //                       0 << 5 |
        //                       0 << 4 |
        //                       0 << 3 |
        //                       0 << 2 |
        //                       0 << 1 |
        //                       1);

        Wr_reg_bits (VPP_MATRIX_CTRL, 5, 0, 3);
        Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);

        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0);
        
        //0.257     0.504   0.098
        //-0.148    -0.291  0.439
        //0.439     -0.368 -0.071
        Wr(VPP_MATRIX_COEF00_01, (0x107 << 16) |
                               0x204);
        Wr(VPP_MATRIX_COEF02_10, (0x64 << 16) |
                               0x1f68);
        Wr(VPP_MATRIX_COEF11_12, (0x1ed6 << 16) |
                                  0x1c2);
        Wr(VPP_MATRIX_COEF20_21, (0x1c2 << 16) | 
                              0x1e87);
        Wr(VPP_MATRIX_COEF22, 0x1fb7);
        Wr(VPP_MATRIX_OFFSET0_1, (0x40 << 16) | 0x0200);
        Wr(VPP_MATRIX_OFFSET2, 0x0200);
   } 
   else
   {
   }

}

void vpp_set_matrix_ycbcr2rgb (int vd1_or_vd2_or_post, int mode)
{
   if (vd1_or_vd2_or_post == 0) //vd1
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 5, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 8, 2);
   }
   else if (vd1_or_vd2_or_post == 1) //vd2 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 4, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 2, 8, 2);
   }
   else 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 0, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);
      if (mode == 0)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 1, 1, 2);  
      }
      else if (mode == 1)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 0, 1, 2);  
      }
   }

   if (mode == 0) //ycbcr not full range, 601 conversion 
   {
        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x7c00600);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0600);

        //1.164     0       1.596
        //1.164   -0.392    -0.813
        //1.164   2.017     0
        Wr(VPP_MATRIX_COEF00_01, (0x4a8 << 16) |
                            0);
        Wr(VPP_MATRIX_COEF02_10, (0x662 << 16) |
                            0x4a8);
        Wr(VPP_MATRIX_COEF11_12, (0x1e6f << 16) |
                            0x1cbf);
        Wr(VPP_MATRIX_COEF20_21, (0x4a8 << 16) | 
                            0x811);
        Wr(VPP_MATRIX_COEF22, 0x0);
        Wr(VPP_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_OFFSET2, 0x0);
   } 
   else if (mode == 1) //ycbcr full range, 601 conversion
   {

        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x0000600);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0600);

        //1     0       1.402
        //1   -0.34414  -0.71414
        //1   1.772     0
        Wr(VPP_MATRIX_COEF00_01, (0x400 << 16) |
                            0);
        Wr(VPP_MATRIX_COEF02_10, (0x59c << 16) |
                            0x400);
        Wr(VPP_MATRIX_COEF11_12, (0x1ea0 << 16) |
                            0x1d25);
        Wr(VPP_MATRIX_COEF20_21, (0x400 << 16) | 
                            0x717);
        Wr(VPP_MATRIX_COEF22, 0x0);
        Wr(VPP_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_OFFSET2, 0x0);
   }
}

void vpp_set_matrix_rgb2ycbcr (int vd1_or_vd2_or_post, int mode)
{

   if (vd1_or_vd2_or_post == 0) //vd1
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 5, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 8, 2);
   }
   else if (vd1_or_vd2_or_post == 1) //vd2 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 4, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 2, 8, 2);
   }
   else 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 0, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);
      if (mode == 0)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 2, 1, 2);  
      }
      else if (mode == 1)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 0, 1, 2);  
      }
   }

   if (mode == 0) //ycbcr not full range, 601 conversion 
   {

        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0);
        
        //0.257     0.504   0.098
        //-0.148    -0.291  0.439
        //0.439     -0.368 -0.071
        Wr(VPP_MATRIX_COEF00_01, (0x107 << 16) |
                               0x204);
        Wr(VPP_MATRIX_COEF02_10, (0x64 << 16) |
                               0x1f68);
        Wr(VPP_MATRIX_COEF11_12, (0x1ed6 << 16) |
                                  0x1c2);
        Wr(VPP_MATRIX_COEF20_21, (0x1c2 << 16) | 
                              0x1e87);
        Wr(VPP_MATRIX_COEF22, 0x1fb7);
        Wr(VPP_MATRIX_OFFSET0_1, (0x40 << 16) | 0x0200);
        Wr(VPP_MATRIX_OFFSET2, 0x0200);
   } 
   else if (mode == 1) //ycbcr full range, 601 conversion
   {
   }

}

void vpp_set_matrix_601to709 (int vd1_or_vd2_or_post, int mode)
{
   if (vd1_or_vd2_or_post == 0) //vd1
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 5, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 8, 2);
   }
   else if (vd1_or_vd2_or_post == 1) //vd2 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 4, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 2, 8, 2);
   }
   else 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 0, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);
      if (mode == 0)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 3, 1, 2);  
      }
      else if (mode == 1)
      {
      }
   }

    
   if (mode == 0)  //601 219 Y to 709 219Y
   {
        //1   -0.115550     -0.207938               //1024   -118     -213         
        //0    1.018640      0.114618               //0      1043      117
        //0    0.075049      1.025327               //0      77       1050
           
        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0);
        
        Wr(VPP_MATRIX_COEF00_01, (0x0400 << 16) |
                               0x1f8a);
        Wr(VPP_MATRIX_COEF02_10, (0x1f2b << 16) |
                               0x0);
        Wr(VPP_MATRIX_COEF11_12, (0x413 << 16) |
                                  0x75);
        Wr(VPP_MATRIX_COEF20_21, (0x0 << 16) | 
                              0x4d);
        Wr(VPP_MATRIX_COEF22, 0x41a);
        Wr(VPP_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_OFFSET2, 0x0);
   }
   else if (mode == 1)
   {
   }
}

void vpp_set_matrix_709to601 (int vd1_or_vd2_or_post, int mode)
{
   if (vd1_or_vd2_or_post == 0) //vd1
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 5, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 8, 2);
   }
   else if (vd1_or_vd2_or_post == 1) //vd2 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 4, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 2, 8, 2);
   }
   else 
   {
      Wr_reg_bits (VPP_MATRIX_CTRL, 1, 0, 1);
      Wr_reg_bits (VPP_MATRIX_CTRL, 0, 8, 2);
      if (mode == 0)
      {
        Wr_reg_bits(VPP_MATRIX_CTRL, 3, 1, 2);  
      }
      else if (mode == 1)
      {
      }
   }

    
   if (mode == 0)  //709 219 Y to 601 219Y
   {
    
    //1   0.099312      0.191700            //1024    102     196
    //0   0.989854     -0.110654            //0       1014   -113
    //0  -0.072453      0.983398            //0       -74     1007
           
        Wr(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_PRE_OFFSET2, 0x0);
        
        Wr(VPP_MATRIX_COEF00_01, (0x0400 << 16) |
                               0x66);
        Wr(VPP_MATRIX_COEF02_10, (0xc4 << 16) |
                               0x0);
        Wr(VPP_MATRIX_COEF11_12, (0x3f6 << 16) |
                                  0x1f8f);
        Wr(VPP_MATRIX_COEF20_21, (0x0 << 16) | 
                              0x1fb6);
        Wr(VPP_MATRIX_COEF22, 0x3ef);
        Wr(VPP_MATRIX_OFFSET0_1, 0x0);
        Wr(VPP_MATRIX_OFFSET2, 0x0);
   }
   else if (mode == 1)
   {
   }
}



//####################################################################################################

// Programe Osd1's color palette
void osd1_write_color_lut (int           start_lut_addr,
                           int           end_lut_addr,
                           unsigned long lut_data[256])
{
    unsigned long data32;
    int i;

    // Program reg VIU_OSD1_COLOR_ADDR
    data32   = 0;
    data32 |= start_lut_addr << 0; // Start addr
    data32 |= 0              << 8; // Write LUT
    Wr(VIU_OSD1_COLOR_ADDR, data32);
    
    for (i = start_lut_addr; i <= end_lut_addr; i ++) {
        // Program reg VIU_OSD1_COLOR
        Wr(VIU_OSD1_COLOR, lut_data[i]);
    }
} /* osd1_write_color_lut */

// Programe Osd2's color palette
void osd2_write_color_lut (int           start_lut_addr,
                           int           end_lut_addr,
                           unsigned long lut_data[256])
{
    unsigned long data32;
    int i;

    // Program reg VIU_OSD2_COLOR_ADDR
    data32  = 0;
    data32 |= start_lut_addr << 0; // Start addr
    data32 |= 0              << 8; // Write LUT
    Wr(VIU_OSD2_COLOR_ADDR, data32);
    
    for ( i = start_lut_addr; i <= end_lut_addr; i ++) {
        // Program reg VIU_OSD1_COLOR
        Wr(VIU_OSD2_COLOR, lut_data[i]);
    }
} /* osd2_write_color_lut */

// Configure Osd1 to display just one block, with color_matrix default to 0
void osd1_simple (unsigned long global_alpha,
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
                  unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD1_TCOLOR_AG<0-3>
    Wr(VIU_OSD1_TCOLOR_AG0, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG1, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG2, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD1_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= canvas_addr            << 16;
    Wr(VIU_OSD1_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU_OSD1_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU_OSD1_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU_OSD1_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU_OSD1_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD1_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 1   << 10; // burst_len_sel: 31, 11:10.  3'b101 = 128. 
    data32 |= 1   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU_OSD1_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD1_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU_OSD1_CTRL_STAT, data32);
} /* osd1_simple */

// Configure Osd1 to display just one block, with color_matrix defined
void osd1_simple_more (unsigned long global_alpha,
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
                       unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD1_TCOLOR_AG<0-3>
    Wr(VIU_OSD1_TCOLOR_AG0, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG1, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG2, 0xffffff00);
    Wr(VIU_OSD1_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD1_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= color_matrix           << 2;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= little_endian          << 15;
    data32 |= canvas_addr            << 16;
    Wr(VIU_OSD1_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU_OSD1_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU_OSD1_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU_OSD1_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU_OSD1_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD1_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 1   << 10; // burst_len_sel: 31, 11:10 = 3'b101 :  128.
    data32 |= 1   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU_OSD1_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD1_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU_OSD1_CTRL_STAT, data32);
} /* osd1_simple_more */

// Configure Osd2 to display just one block, with color_matrix default to 0
void osd2_simple (unsigned long global_alpha,
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
                  unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD2_TCOLOR_AG<0-3>
    Wr(VIU_OSD2_TCOLOR_AG0, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG1, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG2, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD2_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= canvas_addr            << 16;
    Wr(VIU_OSD2_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU_OSD2_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU_OSD2_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU_OSD2_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU_OSD2_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD2_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 3   << 10; // burst_len_sel: 3=64
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU_OSD2_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD2_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU_OSD2_CTRL_STAT, data32);
} /* osd2_simple */

// Configure Osd2 to display just one block, with color_matrix defined
void osd2_simple_more (unsigned long global_alpha,
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
                       unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD2_TCOLOR_AG<0-3>
    Wr(VIU_OSD2_TCOLOR_AG0, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG1, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG2, 0xffffff00);
    Wr(VIU_OSD2_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD2_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= color_matrix           << 2;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= little_endian          << 15;
    data32 |= canvas_addr            << 16;
    Wr(VIU_OSD2_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU_OSD2_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU_OSD2_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU_OSD2_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU_OSD2_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD2_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 1   << 10; // burst_len_sel: 31, 11:10 = 3'b101 :  128.
    data32 |= 1   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU_OSD2_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD2_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU_OSD2_CTRL_STAT, data32);
} /* osd2_simple_more */

// Programe VIU2 Osd1's color palette
void viu2_osd1_write_color_lut (int           start_lut_addr,
                                int           end_lut_addr,
                                unsigned long lut_data[256])
{
    unsigned long data32;
    int i;

    // Program reg VIU2_OSD1_COLOR_ADDR
    data32   = 0;
    data32 |= start_lut_addr << 0; // Start addr
    data32 |= 0              << 8; // Write LUT
    Wr(VIU2_OSD1_COLOR_ADDR, data32);
    
    for (i = start_lut_addr; i <= end_lut_addr; i ++) {
        // Program reg VIU2_OSD1_COLOR
        Wr(VIU2_OSD1_COLOR, lut_data[i]);
    }
} /* viu2_osd1_write_color_lut */

// Programe VIU2 Osd2's color palette
void viu2_osd2_write_color_lut (int           start_lut_addr,
                                int           end_lut_addr,
                                unsigned long lut_data[256])
{
    unsigned long data32;
    int i;

    // Program reg VIU2_OSD2_COLOR_ADDR
    data32  = 0;
    data32 |= start_lut_addr << 0; // Start addr
    data32 |= 0              << 8; // Write LUT
    Wr(VIU2_OSD2_COLOR_ADDR, data32);
    
    for ( i = start_lut_addr; i <= end_lut_addr; i ++) {
        // Program reg VIU2_OSD1_COLOR
        Wr(VIU2_OSD2_COLOR, lut_data[i]);
    }
} /* viu2_osd2_write_color_lut */

// Configure VIU2's Osd1 to display just one block, with color_matrix default to 0
void viu2_osd1_simple (unsigned long global_alpha,
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
                       unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD1_TCOLOR_AG<0-3>
    Wr(VIU2_OSD1_TCOLOR_AG0, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG1, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG2, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD1_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= canvas_addr            << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD1_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD1_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 3   << 10; // burst_len_sel: 31, 11:10.  3'b101 = 128. 
    data32 |= 0   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU2_OSD1_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD1_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU2_OSD1_CTRL_STAT, data32);
} /* viu2_osd1_simple */

// Configure VIU2's Osd1 to display just one block, with color_matrix defined
void viu2_osd1_simple_more (unsigned long global_alpha,
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
                            unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU2_OSD1_TCOLOR_AG<0-3>
    Wr(VIU2_OSD1_TCOLOR_AG0, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG1, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG2, 0xffffff00);
    Wr(VIU2_OSD1_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU2_OSD1_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= color_matrix           << 2;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= little_endian          << 15;
    data32 |= canvas_addr            << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W0, data32);
    // Program reg VIU2_OSD1_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W1, data32);
    // Program reg VIU2_OSD1_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W2, data32);
    // Program reg VIU2_OSD1_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W3, data32);
    // Program reg VIU2_OSD1_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU2_OSD1_BLK0_CFG_W4, data32);
    // Program reg VIU2_OSD1_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 3   << 10; // burst_len_sel: 31, 11:10.  3'b101 = 128. 
    data32 |= 0   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU2_OSD1_FIFO_CTRL_STAT, data32);
    // Program reg VIU2_OSD1_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU2_OSD1_CTRL_STAT, data32);
} /* viu2_osd1_simple_more */

// Configure VIU2 Osd2 to display just one block, with color_matrix default to 0
void viu2_osd2_simple (unsigned long global_alpha,
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
                       unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU_OSD2_TCOLOR_AG<0-3>
    Wr(VIU2_OSD2_TCOLOR_AG0, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG1, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG2, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU_OSD2_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= canvas_addr            << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W0, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W1, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W2, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W3, data32);
    // Program reg VIU_OSD2_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W4, data32);
    // Program reg VIU_OSD2_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 3   << 10; // burst_len_sel: 3=64
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU2_OSD2_FIFO_CTRL_STAT, data32);
    // Program reg VIU_OSD2_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU2_OSD2_CTRL_STAT, data32);
} /* viu2_osd2_simple */

// Configure VIU2's Osd2 to display just one block, with color_matrix defined
void viu2_osd2_simple_more (unsigned long global_alpha,
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
                            unsigned long osd_blk_mode)
{
    unsigned long data32;
    
    unsigned long display_h_end, display_v_end, i;
    
    display_h_end = display_h_start + (pix_x_end - pix_x_start);
    display_v_end = display_v_start - 1;
    for (i = pix_y_start; i <= pix_y_end; i ++) {
        if ((interlace_en == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 0 && (i & 0x1) == 0) ||
            (interlace_en == 1 && interlace_sel_odd == 1 && (i & 0x1) == 1)) {
            display_v_end ++;
        }
    }

    // Program reg VIU2_OSD2_TCOLOR_AG<0-3>
    Wr(VIU2_OSD2_TCOLOR_AG0, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG1, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG2, 0xffffff00);
    Wr(VIU2_OSD2_TCOLOR_AG3, 0xffffff00);
    // Program reg VIU2_OSD2_BLK0_CFG_W0
    data32  = interlace_sel_odd      << 0;
    data32 |= interlace_en           << 1;
    data32 |= color_matrix           << 2;
    data32 |= tc_alpha_en            << 6;
    data32 |= rgb_en                 << 7;
    data32 |= osd_blk_mode           << 8;
    data32 |= little_endian          << 15;
    data32 |= canvas_addr            << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W0, data32);
    // Program reg VIU2_OSD2_BLK0_CFG_W1
    data32  = pix_x_start            << 0;
    data32 |= pix_x_end              << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W1, data32);
    // Program reg VIU2_OSD2_BLK0_CFG_W2
    data32  = pix_y_start            << 0;
    data32 |= pix_y_end              << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W2, data32);
    // Program reg VIU2_OSD2_BLK0_CFG_W3
    data32  = display_h_start        << 0;
    data32 |= display_h_end          << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W3, data32);
    // Program reg VIU2_OSD2_BLK0_CFG_W4
    data32  = display_v_start        << 0;
    data32 |= display_v_end          << 16;
    Wr(VIU2_OSD2_BLK0_CFG_W4, data32);
    // Program reg VIU2_OSD2_FIFO_CTRL_STAT
    data32  = 4   << 5;  // hold_fifo_lines
    data32 |= 3   << 10; // burst_len_sel: 31, 11:10.  3'b101 = 128. 
    data32 |= 0   << 31; // burst_len_sel: 
    data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    Wr(VIU2_OSD2_FIFO_CTRL_STAT, data32);
    // Program reg VIU2_OSD2_CTRL_STAT
    data32  = 0x1          << 0; // osd_blk_enable
    data32 |= global_alpha << 12;
    Wr(VIU2_OSD2_CTRL_STAT, data32);
} /* viu2_osd2_simple_more */


//####################################################################################################
void    set_viu2_vd1_if0_separate_simple(
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
        unsigned long   canvas_addr2
)
{
    // General register setup
    unsigned long   bytes_per_pixel = 0;    // 1Byte per  
    unsigned long   burst_size_cr   = 0;    // unused
    unsigned long   burst_size_cb   = 0;    // unused
    unsigned long   burst_size_y    = 3;    // 64x64 burst size
    unsigned long   st_separate_en  = 1;

    // ----------------------
    // General register
    // ----------------------

    Wr(VIU2_VD1_IF0_GEN_REG,         (4 << 19)               |   //hold lines
                                (1 << 18)               |   // push pixel value
                                (0 << 16)               |
                                (bytes_per_pixel << 14) | 
                                (burst_size_cr << 12)   |
                                (burst_size_cb << 10)   |
                                (burst_size_y << 8)     |
                                (1 << 6)                |   // TODO: cntl_chro_rpt_lastl_ctrl
                                (st_separate_en << 1)   |
                                (0 << 0)                    // cntl_enable (don't enable just yet)
      );
                            
    // ----------------------
    // Canvas
    // ----------------------
    Wr(VIU2_VD1_IF0_CANVAS0,         (canvas_addr2 << 16)     |   // cntl_canvas0_addr2
                                (canvas_addr1 << 8)      |   // cntl_canvas0_addr1
                                (canvas_addr0 << 0)          // cntl_canvas0_addr0
    );

    Wr(VIU2_VD1_IF0_CANVAS1,         (0 << 16)               |   // cntl_canvas1_addr2
                                (0 << 8)                |   // cntl_canvas1_addr1
                                (0 << 0)                    // cntl_canvas1_addr0
    );

    // ----------------------
    // Picture 0 X/Y start,end
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X0,         (luma_x_end << 16)           |   // cntl_luma_x_end0
                                (luma_x_start << 0)              // cntl_luma_x_start0
    );
    Wr(VIU2_VD1_IF0_LUMA_Y0,         (luma_y_end << 16)           |   // cntl_luma_y_end0
                                (luma_y_start << 0)              // cntl_luma_y_start0
    );
    Wr(VIU2_VD1_IF0_CHROMA_X0,     (chroma_x_end << 16)      |
                              (chroma_x_start << 0)
    );                           
    Wr(VIU2_VD1_IF0_CHROMA_Y0,     (chroma_y_end << 16)      |
                              (chroma_y_start << 0)
    );                           

    // ----------------------
    // Picture 1 unused
    // ----------------------
    Wr(VIU2_VD1_IF0_LUMA_X1,       0);                            // unused
    Wr(VIU2_VD1_IF0_LUMA_Y1,       0);                            // unused
    Wr(VIU2_VD1_IF0_CHROMA_X1,     0);                            // unused
    Wr(VIU2_VD1_IF0_CHROMA_Y1,     0);                            // unused

    // ----------------------
    // No Repeat or skip
    // ----------------------
    Wr(VIU2_VD1_IF0_RPT_LOOP,        (0 << 24)               |   // cntl_chroma1_rpt_loop
                                (0 << 16)               |   // cntl_luma1_rpt_loop
                                (0 << 8)                |   // cntl_chroma0_rpt_loop
                                (0 << 0)                    // cntl_luma0_rpt_loop
    ); 

    Wr(VIU2_VD1_IF0_LUMA0_RPT_PAT,      0);                        // no skip /repeat
    Wr(VIU2_VD1_IF0_CHROMA0_RPT_PAT,    0);                        // unused
    Wr(VIU2_VD1_IF0_LUMA1_RPT_PAT,      0);                        // unused
    Wr(VIU2_VD1_IF0_CHROMA1_RPT_PAT,    0);                        // unused

    Wr(VIU2_VD1_IF0_LUMA_PSEL,          0);                        // unused only one picture 
    Wr(VIU2_VD1_IF0_CHROMA_PSEL,        0);                        // unused only one picture 


    Wr(VIU2_VD1_IF0_DUMMY_PIXEL,   0x00808000); 

    // Enable VIU2_VD1: vd_rmem_if0
    Wr(VIU2_VD1_IF0_GEN_REG,      Rd(VIU2_VD1_IF0_GEN_REG) |  (1 << 0));                  // cntl_enable

}

//####################################################################################################
void ldim_stts_en(
	int resolution,
	int hist_16bit_mode, //1: output 16bit hist to reduce cbus read times
	int eol_en,
    int hist_mode,       //hist mode: 0: comp0 hist only, 1: Max(comp0,1,2) for hist, 2: the hist of all comp0,1,2 are calculated
	int lpf_en,			//1: 1,2,1 filter on before finding max& hist
	int rd_idx_auto_inc_mode  //0: no self increase, 1: read index increase after read a 25/48 block, 2: increases every read and lock sub-idx
)
{
	int data32;
    Wr(LDIM_STTS_GCLK_CTRL0, 0x0);
    Wr(LDIM_STTS_WIDTHM1_HEIGHTM1, resolution);

	data32 = 0x80000000 | ((hist_16bit_mode & 0x1) << 29);
	data32 = data32 | ((eol_en & 0x1) << 28);
	data32 = data32 | ((hist_mode & 0x3) << 21);
	data32 = data32 | ((lpf_en & 0x1) << 20);
	data32 = data32 | ((rd_idx_auto_inc_mode & 0x3) << 14);
    Wr(LDIM_STTS_HIST_REGION_IDX, data32);
}

void ldim_set_region(
					int resolution, 	//0: auto calc by height/width/row_start/col_start
										//1: 720p
										//2: 1080p
					int blk_height,
					int blk_width,
					int row_start,
					int col_start

)
{
  int hend0, hend1, hend2, hend3, hend4, hend5, hend6, hend7, hend8, hend9;
  int vend0, vend1, vend2, vend3, vend4, vend5, vend6, vend7, vend8, vend9;
  int data32;

  if (resolution == 0) {
    hend0 = col_start + blk_width - 1;
    hend1 = hend0 + blk_width;
    hend2 = hend1 + blk_width;
    hend3 = hend2 + blk_width;
    hend4 = hend3 + blk_width;
    hend5 = hend4 + blk_width;
    hend6 = hend5 + blk_width;
    hend7 = hend6 + blk_width;
    hend8 = hend7 + blk_width;
    hend9 = hend8 + blk_width;
    vend0 = col_start + blk_height - 1;
    vend1 = vend0 + blk_height;
    vend2 = vend1 + blk_height;
    vend3 = vend2 + blk_height;
    vend4 = vend3 + blk_height;
    vend5 = vend4 + blk_height;
    vend6 = vend5 + blk_height;
    vend7 = vend6 + blk_height;
    vend8 = vend7 + blk_height;
    vend9 = vend8 + blk_height;
	
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    Wr(LDIM_STTS_HIST_SET_REGION, ((((row_start & 0x1fff) << 16)& 0xffff0000) | (col_start & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((hend1 & 0x1fff) << 16) | (hend0 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((vend1 & 0x1fff) << 16) | (vend0 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((hend3 & 0x1fff) << 16) | (hend2 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((vend3 & 0x1fff) << 16) | (vend2 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((hend5 & 0x1fff) << 16) | (hend4 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((vend5 & 0x1fff) << 16) | (vend4 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((hend7 & 0x1fff) << 16) | (hend6 & 0x1fff)));
	Wr(LDIM_STTS_HIST_SET_REGION, (((vend7 & 0x1fff) << 16) | (vend6 & 0x1fff)));
	//Wr(LDIM_STTS_HIST_SET_REGION, (((hend9 & 0x1fff) << 16) | (hend8 & 0x1fff)));
	//Wr(LDIM_STTS_HIST_SET_REGION, (((vend9 & 0x1fff) << 16) | (vend8 & 0x1fff)));
  }
  else if (resolution == 1) {
	int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);

    Wr(LDIM_STTS_HIST_SET_REGION, 0x0010010);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x1000080);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0800040);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2000180);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x10000c0);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x3000280);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x1800140);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4000380);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x20001c0);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4ff0480);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x2cf0260);
  }
  else if (resolution == 2) {
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);

    Wr(LDIM_STTS_HIST_SET_REGION, 0x0000000);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x17f00bf);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0d7006b);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2ff023f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x1af0143);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x47f03bf);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x287021b);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x5ff053f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x35f02f3);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x77e06bf);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x43703cb);
  } 
  else if (resolution == 3) {
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);

    Wr(LDIM_STTS_HIST_SET_REGION, 0x0000000);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x1df00ef);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x10d0086);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x3bf02cf);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x21b0194);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x59f04af);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x32902a2);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x77f068f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x43703b0);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 4) { //5x6
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0040001);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x27f0136);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x1af00d7);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4ff03bf);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x35f0287);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x77f063f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380437);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 5) { //8x2
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0030002);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x31f02bb);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0940031);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x233012b);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x30b0243);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x42d03d3);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 6) { //2x1
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0030002);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x78002bb);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0940031);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 7) { //2x2
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0000000);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x77f03bf);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x437021b);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 8) { //3x5
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0000000);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2ff017f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2cf0167);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x5ff047f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380437);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x780077f);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 9) { //4x3
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0010001);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4560333);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2220180);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800666);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4000338);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }
  else if (resolution == 10) { //6x8
    int data32;
    data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);
    
    Wr(LDIM_STTS_HIST_SET_REGION, 0x0010001);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2430167);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x2220180);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4000350);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4000338);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x6000510);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4370410);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x77f0700);
    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x7800780);
//    Wr(LDIM_STTS_HIST_SET_REGION, 0x4380438);
  }

}

void ldim_read_region(int nrow, int ncol)
{
	int i,j,k;
	int data32;
	data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
    Wr(LDIM_STTS_HIST_REGION_IDX, 0xfffff000 & data32);
	for(i=0;i<nrow;i++) {
	  data32 = Rd(LDIM_STTS_HIST_REGION_IDX);
      Wr(LDIM_STTS_HIST_REGION_IDX, (0xfffff000 & data32)|i*8);
	  for(j=0;j<ncol;j++)
		for (k=0;k<16;k++)
		  data32 = Rd(LDIM_STTS_HIST_READ_REGION);
    }
}

void  vdin0_ldim_stts_en()
{
    Wr(VDIN0_LDIM_STTS_HIST_REGION_IDX, 0x81008000);
}

void vdin0_ldim_set_region()
{
    int data32;
    data32 = Rd(VDIN0_LDIM_STTS_HIST_REGION_IDX);
    Wr(VDIN0_LDIM_STTS_HIST_REGION_IDX, 0xfff0ffff & data32);

    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x0010010);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x1000080);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x0800040);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x2000180);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x10000c0);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x3000280);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x1800140);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x4000380);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x20001c0);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x4ff0480);
    Wr(VDIN0_LDIM_STTS_HIST_SET_REGION, 0x2cf0260);

}

void vdin0_ldim_read_region()
{
    int i,j;
    int data32;
    data32 = Rd(VDIN0_LDIM_STTS_HIST_REGION_IDX);
    Wr(VDIN0_LDIM_STTS_HIST_REGION_IDX, 0xffffc000 & data32);
    for(i=0;i<100;i++)
      for(j=0;j<48;j++)
        data32 = Rd(VDIN0_LDIM_STTS_HIST_READ_REGION);
}

void ldim_set_matrix_rgb2ycbcr (int mode)
{
   Wr_reg_bits (LDIM_STTS_CTRL0, 1, 2, 1); 

   if (mode == 0) //ycbcr not full range, 601 conversion 
   {

        Wr(LDIM_STTS_MATRIX_PRE_OFFSET0_1, 0x0); 
        Wr(LDIM_STTS_MATRIX_PRE_OFFSET2, 0x0);

        //0.257     0.504   0.098
        //-0.148    -0.291  0.439
        //0.439     -0.368 -0.071
        Wr(LDIM_STTS_MATRIX_COEF00_01, (0x107 << 16) |
                               0x204);
        Wr(LDIM_STTS_MATRIX_COEF02_10, (0x64 << 16) |
                               0x1f68);
        Wr(LDIM_STTS_MATRIX_COEF11_12, (0x1ed6 << 16) |
                                  0x1c2);
        Wr(LDIM_STTS_MATRIX_COEF20_21, (0x1c2 << 16) |
                              0x1e87);
        Wr(LDIM_STTS_MATRIX_COEF22, 0x1fb7);
        
        Wr(LDIM_STTS_MATRIX_OFFSET2, 0x0200);
   }
   else if (mode == 1) //ycbcr full range, 601 conversion
   {
   }

}  

void vdin_ldim_bypass_mtx()
{
	Wr_reg_bits (VDIN0_MATRIX_CTRL, 3, 0, 2); 
	Wr_reg_bits (VDIN0_MATRIX_CTRL, 3, 8, 2); 
}

