
//FILE: clk_util.c
// #define _GATESIM_
//---------------------------------------------------
//       clocks_set_sys_defaults
//---------------------------------------------------

#include "register.h"
#include "c_stimulus.h"
#include "clk_util.h"

#define NO_ENCT
//#define NO_ENCP
//#define NO_2ND_PLL

void    clocks_set_sys_defaults( )
{
    // unsigned long   rd_data;
    // -----------------------------------------
    // HDMI (90Mhz)
    // -----------------------------------------
    //         .clk0               ( cts_oscin_clk         ),
    //         .clk1               ( fclk_div4             ),
    //         .clk2               ( fclk_div3             ),
    //         .clk3               ( fclk_div5             ),
    Wr( HHI_HDMI_CLK_CNTL,  ((1 << 9)  |   // select "fclk_div4" PLL
                             (1 << 8)  |   // Enable gated clock
                             (6 << 0)) );  // Divide by 7
    // -----------------------------------------
    // HDMI PLL
    // -----------------------------------------
    //        .HDMI_DPLL_EN               ( hi_vid_pll_cntl[30]       ), // input           
    //        .HDMI_DPLL_RESET            ( hi_vid_pll_cntl[29]       ),
    //        .HDMI_DPLL_OD_HDMI          ( hi_vid_pll_cntl[19:18]    ), 
    //        .HDMI_DPLL_OD_LVDS          ( hi_vid_pll_cntl[17:16]    ), 
    //        .HDMI_DPLL_N                ( hi_vid_pll_cntl[14:10]    ), 
    //        .HDMI_DPLL_M                ( hi_vid_pll_cntl[8:0]      ),
    //
    //        .HDMI_DPLL_DIV_FRAC         ( hi_vid_pll_cntl2[11:0]    ),
    // chris[seven-up.amlogic.com]:~/work/project_m8/top/test/test502x > pll_video.pl 2970 24 pll_out hpll
    //                                                                
    //                                                                
    //  +----------------------------------------------------------+  
    //  |         Multi-Phase PLL                            Final |  
    //  |   FIn   |   N    M   Frac    fVCO        od     Pll Out  |  
    //  +---------+------------------------------------------------+  
    //  | 24.0000 |   4  495      0    2970.0000    1 |  2970.0000 |  
    Wr( HHI_VID_PLL_CNTL,   (1    << 30)    | 
                            (0    << 29)    |
                            (0    << 18)    |   // HDMI direct divide by 1
                            (1    << 16)    |   // HDMI (lvds divider) divide by 2
                            (4    << 10)    | 
                            (495  << 0) );
    // -----------------------------------------
    // VPU clock
    // -----------------------------------------

    //        .clk_div            ( hi_vpu_clk_cntl[6:0]  ),
    //        .clk_en             ( hi_vpu_clk_cntl[8]    ),
    //        .clk_sel            ( hi_vpu_clk_cntl[11:9] ),
    Wr( HHI_VPU_CLK_CNTL,   (1 << 9)    |   // vpu   clk_sel
                            (3 << 0) );     // vpu   clk_div
    Wr( HHI_VPU_CLK_CNTL, (Rd(HHI_VPU_CLK_CNTL) | (1 << 8)) );
    //
    // VPU
    //
    // Moved to hiu: Wr( HHI_VPU_MEM_PD_REG0, 0x00000000 );
    // Moved to hiu: Wr( HHI_VPU_MEM_PD_REG1, 0x00000000 );

    Wr( HHI_VPU_MEM_PD_REG0, 0x00000000 );
    Wr( HHI_VPU_MEM_PD_REG1, 0x00000000 );

    Wr( VPU_MEM_PD_REG0, 0x00000000 );
    Wr( VPU_MEM_PD_REG1, 0x00000000 );
}

//-------------------------------------
// setting for progressive only
void vclk_set_encp_1920x1080( int pll_sel, int pll_div_sel, int vclk_sel, int upsample)
{
    unsigned long pll_reg;
    unsigned long vid_div_reg;
    unsigned int xd; 
    unsigned long data32;


#ifdef NO_2ND_PLL
    pll_sel = 0;
#endif
    //add by juncheng.xiong to fix test1502 vpu clk open
    Wr( HHI_VPU_CLK_CNTL,   (2 << 9)    |   // vpu   clk_sel
                            (1 << 0) );     // vpu   clk_div
    Wr( HHI_VPU_CLK_CNTL, (Rd(HHI_VPU_CLK_CNTL) | (1 << 8)) );

    if (pll_sel) { // Setting for VID2_PLL
        pll_reg = 0x00020863;
        vid_div_reg = 0x00010803;
        xd = 2; 
    } else { // Setting for VID_PLL, consideration for supporting HDMI clock
        pll_reg = 0x400611ef;
        vid_div_reg = 0x00010843;
        xd = 1; 
    }

    vid_div_reg |= (1 << 16) ; // turn clock gate on
    vid_div_reg |= (pll_sel << 15); // vid_div_clk_sel
    
   
    if(vclk_sel) {
      Wr( HHI_VIID_CLK_CNTL, Rd(HHI_VIID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0 
    }
    else {
      Wr( HHI_VID_CLK_CNTL, Rd(HHI_VID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0 
      Wr( HHI_VID_CLK_CNTL, Rd(HHI_VID_CLK_CNTL) & ~(1 << 20) );     //disable clk_div1 
    } 

    // delay 2uS to allow the sync mux to switch over
    Wr( ISA_TIMERE, 0); while( Rd(ISA_TIMERE) < 2 ) {}    


    if(pll_sel) Wr( HHI_VID2_PLL_CNTL,       pll_reg );    
    else Wr( HHI_VID_PLL_CNTL,       pll_reg );

    if(pll_div_sel ) {
      Wr( HHI_VIID_DIVIDER_CNTL,   vid_div_reg);
    }
    else {
      Wr( HHI_VID_DIVIDER_CNTL,   vid_div_reg);
    }

    if(vclk_sel) Wr( HHI_VIID_CLK_DIV, (Rd(HHI_VIID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value
    else Wr( HHI_VID_CLK_DIV, (Rd(HHI_VID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value

    // delay 5uS
    Wr( ISA_TIMERE, 0); while( Rd(ISA_TIMERE) < 5 ) {}    

    if(vclk_sel) {
      if(pll_div_sel) Wr_reg_bits (HHI_VIID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else Wr_reg_bits (HHI_VIID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      Wr( HHI_VIID_CLK_CNTL, Rd(HHI_VIID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0 
    }
    else {
      Wr_reg_bits (HHI_VID_CLK_DIV,
                        1,              //divide 2 for clk_div1
                        8, 8);
    
      if(pll_div_sel) Wr_reg_bits (HHI_VID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else Wr_reg_bits (HHI_VID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      Wr( HHI_VID_CLK_CNTL, Rd(HHI_VID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0 
    }
    // delay 2uS

    Wr( ISA_TIMERE, 0); while( Rd(ISA_TIMERE) < 2 ) {}    


    if(vclk_sel) {
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      Wr_reg_bits (HHI_VID_CLK_DIV, 
                   8,      // select clk_div1 
                   24, 4); // [23:20] encp_clk_sel 
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 1, 15, 1);  //soft reset
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    else {
      Wr_reg_bits (HHI_VID_CLK_CNTL, 
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      Wr_reg_bits (HHI_VID_CLK_DIV, 
                   0,      // select clk_div1 
                   24, 4); // [23:20] encp_clk_sel 
      Wr_reg_bits (HHI_VID_CLK_CNTL, 1, 15, 1);  //soft reset
      Wr_reg_bits (HHI_VID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    
    // delay 2uS
    Wr( ISA_TIMERE, 0); while( Rd(ISA_TIMERE) < 2 ) {}    


//    Wr_reg_bits (HHI_VID_CLK_DIV, 
//                   1|(vclk_sel<<3),      // select clk_div2 
//                   24, 4); // [23:20] encp_clk_sel 

    if(upsample) {
        Wr_reg_bits (HHI_VIID_CLK_DIV, 
                     0|(vclk_sel<<3)|(vclk_sel<<7)|(vclk_sel<<11),      // select clk_div1 
                     20, 12); // [31:20] dac0_clk_sel, dac1_clk_sel, dac2_clk_sel
    }
    else {
        data32 = Rd(HHI_VID_CLK_CNTL);  // Save HHI_VID_CLK_CNTL value
        Wr_reg_bits (HHI_VID_CLK_CNTL, 0, 0, 1);  // Disable cntl_div1_en first before select other clock, to avoid glitch
        Wr_reg_bits (HHI_VIID_CLK_DIV, 
                     0x111|(vclk_sel<<3)|(vclk_sel<<7)|(vclk_sel<<11),      // select clk_div2 
                     20, 12); // [31:20] dac0_clk_sel, dac1_clk_sel, dac2_clk_sel
        Wr(HHI_VID_CLK_CNTL, data32);   // Recover HHI_VID_CLK_CNTL value
    }

    if(vclk_sel) {
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 
                   (3<<0),  // Enable cntl_div1_en and cntl_div2_en
                   0, 2    // cntl_div1_en
                   );
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 1, 15, 1);  //soft reset
      Wr_reg_bits (HHI_VIID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    else {
      Wr_reg_bits (HHI_VID_CLK_CNTL, 
                   (3<<0),  // Enable cntl_div1_en and cntl_div2_en
                   0, 2    // cntl_div1_en
                   );
      Wr_reg_bits (HHI_VID_CLK_CNTL, 1, 15, 1);  //soft reset
      Wr_reg_bits (HHI_VID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
        // -----------------------------------------
    // Power up memories
    // -----------------------------------------
    //
    // VPU
    //
#if 0
    Wr( HHI_VPU_MEM_PD_REG0, 0x00000000 );
    Wr( HHI_VPU_MEM_PD_REG1, 0x00000000 );
#endif
   stimulus_print("vpu mem power up !\n");
} /* vclk_set_encp_1920x1080 */

