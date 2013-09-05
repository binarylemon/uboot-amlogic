#include <common.h>

#include "register.h"
#include "c_arc_pointer_reg.h"
#include "f2v.h"
#include "canvas.h"
#include "display.h"
#include "c_stimulus.h"
#include "clk_util.h"
#include "tv_enc.h"
#include "gic.h"

#include "hdmi.h"
#include "hdmi_parameter.h"
#include "test_prm.h"

#define VFIFO2VD_TO_HDMI_LATENCY    2   // Latency in pixel clock from ENCP_VFIFO2VD request to data ready to HDMI
    #define NUM_INT_VSYNC   INT_VEC_VIU1_VSYNC

void hdmi_test_function(void);
unsigned long modulo(unsigned long a, unsigned long b);
signed int to_signed(unsigned int a);

//FILE: test.c

// --------------------------------------------------------
//                     C_Entry
// --------------------------------------------------------
unsigned char   field_n = 0;
unsigned int    src_w = 0, src_h = 0;

void C_Entry(void) 
{
    unsigned long data32;
    cav_con       canvas;

    unsigned long total_pixels_venc = (TOTAL_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC);
    unsigned long active_pixels_venc= (ACTIVE_PIXELS / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC);
    unsigned long front_porch_venc  = (FRONT_PORCH   / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC);
    unsigned long hsync_pixels_venc = (HSYNC_PIXELS  / (1+PIXEL_REPEAT_HDMI)) * (1+PIXEL_REPEAT_VENC);

    unsigned long de_h_begin, de_h_end;
    unsigned long de_v_begin_even, de_v_end_even, de_v_begin_odd, de_v_end_odd;
    unsigned long hs_begin, hs_end;
    unsigned long vs_adjust;
    unsigned long vs_bline_evn, vs_eline_evn, vs_bline_odd, vs_eline_odd;
    unsigned long vso_begin_evn, vso_begin_odd;

    // --------------------------------------------------------
    // Set Clocks
    // --------------------------------------------------------

    clocks_set_sys_defaults();    // set MPEG, audio and default video
    Wr_reg_bits (HHI_HDMI_CLK_CNTL, 0, 16, 4);  // Set [19:16] hdmi_tx_pixel_clk comes from clk_div1
    //                      pll_sel,    pll_div_sel,    vclk_sel,   upsample)
    vclk_set_encp_1920x1080(0,          0,              0,          0);

    // Configure HDMI AFE input clock
    // [19:18] Set HPLL_OD_HDMI: 0=video_pll.CLK_OUT; 1=video_pll.CLK_OUT/2; 2 or 3=video_pll.CLK_OUT/4
    //Wr_reg_bits(HHI_VID_PLL_CNTL, 2, 18, 2);
    
    // --------------------------------------------------------
    // Program core_pin_mux to enable HDMI pins
    // --------------------------------------------------------
    //wire            pm_gpioH_0_hdmi_hpd         = pin_mux_reg1[26];
    //wire            pm_gpioH_1_hdmi_sda         = pin_mux_reg1[25];
    //wire            pm_gpioH_2_hdmi_scl         = pin_mux_reg1[24];
    //wire            pm_gpioH_3_hdmi_cec         = pin_mux_reg1[23];
    (*P_PERIPHS_PIN_MUX_1) |= ((1 << 26)    | // pm_gpioH_0_hdmi_hpd
                               (1 << 25)    | // pm_gpioH_1_hdmi_sda
                               (1 << 24)    | // pm_gpioH_2_hdmi_scl
                               (1 << 23));    // pm_gpioH_3_hdmi_cec

    // --------------------------------------------------------
    // Set up HDMI
    // --------------------------------------------------------
    hdmi_test_function();

    // --------------------------------------------------------
    // Define Canvases
    // --------------------------------------------------------
    
    src_w = ACTIVE_PIXELS / (1 + PIXEL_REPEAT_HDMI);
    src_h = ACTIVE_LINES * (1 + INTERLACE_MODE);
    
    canvas.cav_width = ((src_w * 24 + (64*4-1)) / (64*4))*4;
    canvas.cav_hight = src_h;
    canvas.x_wrap_en = 0;
    canvas.y_wrap_en = 0;
    canvas.blk_mode = 0;

    // Picture to display before HDCP authentication
    canvas.start_addr = (0x400000 >> 3);
    config_cav_lut(0, canvas);

    // Picture to display after HDCP authentication
    canvas.start_addr = (0xa00000 >> 3);
    config_cav_lut(1, canvas);

    // --------------------------------------------------------
    // Set TV encoder for HDMI
    // --------------------------------------------------------
            //                      viu1_sel    viu2_sel    enable)
            set_tv_enc_1920x1080p(  1,          0,          0);

    // --------------------------------------------------------
    // Configure video format timing for HDMI:
    // Based on the corresponding settings in set_tv_enc.c, calculate
    // the register values to meet the timing requirements defined in CEA-861-D
    // --------------------------------------------------------

    // Program DE timing
    de_h_begin = modulo(Rd(ENCP_VIDEO_HAVON_BEGIN) + VFIFO2VD_TO_HDMI_LATENCY,  total_pixels_venc);
    de_h_end   = modulo(de_h_begin + active_pixels_venc,                        total_pixels_venc);
    Wr(ENCP_DE_H_BEGIN, de_h_begin);
    Wr(ENCP_DE_H_END,   de_h_end);
    // Program DE timing for even field
    de_v_begin_even = Rd(ENCP_VIDEO_VAVON_BLINE);
    de_v_end_even   = modulo(de_v_begin_even + ACTIVE_LINES, TOTAL_LINES);
    Wr(ENCP_DE_V_BEGIN_EVEN,de_v_begin_even);
    Wr(ENCP_DE_V_END_EVEN,  de_v_end_even);
    // Program DE timing for odd field if needed
    if (INTERLACE_MODE) {
        // Calculate de_v_begin_odd according to enc480p_timing.v:
        //wire[10:0]	cfg_ofld_vavon_bline	= {{7{ofld_vavon_ofst1 [3]}},ofld_vavon_ofst1 [3:0]} + cfg_video_vavon_bline	+ ofld_line;
        de_v_begin_odd  = to_signed((Rd(ENCP_VIDEO_OFLD_VOAV_OFST) & 0xf0)>>4) + de_v_begin_even + (TOTAL_LINES-1)/2;
        de_v_end_odd    = modulo(de_v_begin_odd + ACTIVE_LINES, TOTAL_LINES);
        Wr(ENCP_DE_V_BEGIN_ODD, de_v_begin_odd);
        Wr(ENCP_DE_V_END_ODD,   de_v_end_odd);
    }

    // Program Hsync timing
    if (de_h_end + front_porch_venc >= total_pixels_venc) {
        hs_begin    = de_h_end + front_porch_venc - total_pixels_venc;
        vs_adjust   = 1;
    } else {
        hs_begin    = de_h_end + front_porch_venc;
        vs_adjust   = 0;
    }
    hs_end  = modulo(hs_begin + hsync_pixels_venc,   total_pixels_venc);
    Wr(ENCP_DVI_HSO_BEGIN,  hs_begin);
    Wr(ENCP_DVI_HSO_END,    hs_end);
    
    // Program Vsync timing for even field
    if (de_v_begin_even >= SOF_LINES + VSYNC_LINES + (1-vs_adjust)) {
        vs_bline_evn = de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust);
    } else {
        vs_bline_evn = TOTAL_LINES + de_v_begin_even - SOF_LINES - VSYNC_LINES - (1-vs_adjust);
    }
    vs_eline_evn = modulo(vs_bline_evn + VSYNC_LINES, TOTAL_LINES);
    Wr(ENCP_DVI_VSO_BLINE_EVN, vs_bline_evn);
    Wr(ENCP_DVI_VSO_ELINE_EVN, vs_eline_evn);
    vso_begin_evn = hs_begin;
    Wr(ENCP_DVI_VSO_BEGIN_EVN, vso_begin_evn);
    Wr(ENCP_DVI_VSO_END_EVN,   vso_begin_evn);
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
    Wr_reg_bits(VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
    Wr(ENCP_VIDEO_EN, 1); // Enable VENC

    return;
}

void hdmi_test_function(void)
{
    unsigned int tmp_add_data;
    
    // Enable APB3 fail on error
    *((volatile unsigned long *) HDMI_CTRL_PORT) |= (1 << 15);        //APB3 err_en

    // Enable these interrupts: [2] tx_edid_int_rise [1] tx_hpd_int_fall [0] tx_hpd_int_rise
    hdmi_wr_reg(OTHER_BASE_ADDR + HDMI_OTHER_INTR_MASKN, 0x7);

    // HPD glitch filter
    hdmi_wr_reg(TX_HDCP_HPD_FILTER_L, 0x00);
    hdmi_wr_reg(TX_HDCP_HPD_FILTER_H, 0xa0);

    // Disable MEM power-down
    hdmi_wr_reg(TX_MEM_PD_REG0, 0);

    // Keep TX (except register I/F) in reset, while programming the registers:
    tmp_add_data  = 0;
    tmp_add_data |= 1   << 7; // [7] tx_pixel_rstn
    tmp_add_data |= 1   << 6; // [6] tx_tmds_rstn
    tmp_add_data |= 1   << 5; // [5] tx_audio_master_rstn
    tmp_add_data |= 1   << 4; // [4] tx_audio_sample_rstn
    tmp_add_data |= 1   << 3; // [3] tx_i2s_reset_rstn
    tmp_add_data |= 1   << 2; // [2] tx_dig_reset_n_ch2
    tmp_add_data |= 1   << 1; // [1] tx_dig_reset_n_ch1
    tmp_add_data |= 1   << 0; // [0] tx_dig_reset_n_ch0
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0   << 4; // [4] HDMI_CH0_RST_IN
    tmp_add_data |= 1   << 2; // [0] tx_ddc_hdcp_reset_n
    tmp_add_data |= 1   << 1; // [0] tx_ddc_edid_reset_n
    tmp_add_data |= 1   << 0; // [0] tx_dig_reset_n_ch3
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7] forced_sys_trigger
    tmp_add_data |= 0   << 6; // [6] sys_trigger_config
    tmp_add_data |= 0   << 5; // [5] mem_acc_seq_mode
    tmp_add_data |= 0   << 4; // [4] mem_acc_seq_start
    tmp_add_data |= 1   << 3; // [3] forced_mem_copy_done
    tmp_add_data |= 1   << 2; // [2] mem_copy_done_config
    tmp_add_data |= 0   << 1; // [1] edid_int_forced_clear
    tmp_add_data |= 0   << 0; // [0] edid_int_auto_clear
    hdmi_wr_reg(TX_HDCP_EDID_CONFIG, tmp_add_data);
    
    // SOFT_RESET [23:16]
    tmp_add_data  = 0;
    tmp_add_data |= 0 << 7; // [7]   ~rx_i2s_config_rstn
    tmp_add_data |= 0 << 6; // [6]   rsvd
    tmp_add_data |= 3 << 4; // [5:4] rx_acr_rst_config[1:0]
    tmp_add_data |= 0 << 3; // [3]   ~rx_config_eye_rstn_ch3
    tmp_add_data |= 0 << 2; // [2]   ~rx_config_eye_rstn_ch2
    tmp_add_data |= 0 << 1; // [1]   ~rx_config_eye_rstn_ch1
    tmp_add_data |= 0 << 0; // [0]   ~rx_config_eye_rstn_ch0
    ext_hdmi_wr_reg(RX_BASE_ADDR+0xe4, tmp_add_data);

    // Enable HDMI TX PHY clock
    tmp_add_data  = 0;
    tmp_add_data  |= (1   << 1);        // [1]      phy clock enable
    Wr(HHI_HDMI_PHY_CNTL1, tmp_add_data);

    // Div Pre
    tmp_add_data = 0xff;    // div_pre[7:0]
    ext_hdmi_wr_reg(RX_SYS3_RX_ACR_0, tmp_add_data);
    tmp_add_data  = 0;
    tmp_add_data |= 3   << 6;   // [7:6] acr_mode[1:0]
    tmp_add_data |= 1   << 5;   // [5]   ~force div_main
    tmp_add_data |= 1   << 4;   // [4]   ~force div_pre
    tmp_add_data |= 0xb << 0;   // [3:0] div_pre[11:8]
    ext_hdmi_wr_reg(RX_SYS3_RX_ACR_1, tmp_add_data);

    // SOFT_RESET [23:16]: need it to kick start ACR clocks
    tmp_add_data  = 0;
    tmp_add_data |= 0 << 7; // [7]   ~rx_i2s_config_rstn
    tmp_add_data |= 0 << 6; // [6]   Rsvd
    tmp_add_data |= 1 << 4; // [5:4] rx_acr_rst_config[1:0]
    tmp_add_data |= 0 << 3; // [3]   ~rx_config_eye_rstn_ch3
    tmp_add_data |= 0 << 2; // [2]   ~rx_config_eye_rstn_ch2
    tmp_add_data |= 0 << 1; // [1]   ~rx_config_eye_rstn_ch1
    tmp_add_data |= 0 << 0; // [0]   ~rx_config_eye_rstn_ch0
    ext_hdmi_wr_reg(RX_BASE_ADDR+0xe4, tmp_add_data);
    
    // SOFT_RESET [23:16]
    tmp_add_data  = 0;
    tmp_add_data |= 0 << 7; // [7]   ~rx_i2s_config_rstn
    tmp_add_data |= 0 << 6; // [6]   Rsvd
    tmp_add_data |= 0 << 4; // [5:4] rx_acr_rst_config[1:0]
    tmp_add_data |= 0 << 3; // [3]   ~rx_config_eye_rstn_ch3
    tmp_add_data |= 0 << 2; // [2]   ~rx_config_eye_rstn_ch2
    tmp_add_data |= 0 << 1; // [1]   ~rx_config_eye_rstn_ch1
    tmp_add_data |= 0 << 0; // [0]   ~rx_config_eye_rstn_ch0
    ext_hdmi_wr_reg(RX_BASE_ADDR+0xe4, tmp_add_data);
    // Set RX video/pixel/audio/packet source to DATA_PATH
    ext_hdmi_wr_reg(RX_CORE_DATA_CAPTURE_2, 0x00);

    tmp_add_data  = 0;
    tmp_add_data |= 0               << 7; // [7]   Force DTV timing (Auto)
    tmp_add_data |= 0               << 6; // [6]   Force Video Scan, only if [7]is set
    tmp_add_data |= 0               << 5; // [5]   Force Video field, only if [7]is set
    tmp_add_data |= ((VIC==39)?0:1) << 4; // [4]   disable_vic39_correction
    tmp_add_data |= 0               << 0; // [3:0] Rsrv
    hdmi_wr_reg(TX_VIDEO_DTV_TIMING, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 7; // [7]   forced_default_phase
    tmp_add_data |= 0                       << 2; // [6:2] Rsrv
    tmp_add_data |= TX_OUTPUT_COLOR_DEPTH   << 0; // [1:0] Color_depth:0=24-bit pixel; 1=30-bit pixel; 2=36-bit pixel; 3=48-bit pixel
    hdmi_wr_reg(TX_VIDEO_DTV_MODE, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 1                       << 7; // [7]   gc_pack_mode: 0=clear color_depth and pixel_phase when GC packet is transmitting AV_mute/clear info;
                                                  //                     1=do not clear.
    tmp_add_data |= 0                       << 0; // [6:0] forced_islands_per_period_active
    hdmi_wr_reg(TX_PACKET_ALLOC_ACTIVE_1, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   Force packet timing
    tmp_add_data |= 0   << 6; // [6]   PACKET ALLOC MODE
    tmp_add_data |= 58  << 0; // [5:0] PACKET_START_LATENCY
    hdmi_wr_reg(TX_PACKET_CONTROL_1, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 6; // [7:6] audio_source_select[1:0]
    tmp_add_data |= 0   << 5; // [5]   external_packet_enable
    tmp_add_data |= 1   << 4; // [4]   internal_packet_enable
    tmp_add_data |= 0   << 2; // [3:2] afe_fifo_source_select_lane_1[1:0]
    tmp_add_data |= 0   << 0; // [1:0] afe_fifo_source_select_lane_0[1:0]
    hdmi_wr_reg(TX_CORE_DATA_CAPTURE_2, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   monitor_lane_1
    tmp_add_data |= 0   << 4; // [6:4] monitor_select_lane_1[2:0]
    tmp_add_data |= 1   << 3; // [3]   monitor_lane_0
    tmp_add_data |= 7   << 0; // [2:0] monitor_select_lane_0[2:0]
    hdmi_wr_reg(TX_CORE_DATA_MONITOR_1, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 3; // [7:3] Rsrv
    tmp_add_data |= 2   << 0; // [2:0] monitor_select[2:0]
    hdmi_wr_reg(TX_CORE_DATA_MONITOR_2, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 1   << 7; // [7]   forced_hdmi
    tmp_add_data |= 1   << 6; // [6]   hdmi_config
    tmp_add_data |= 0   << 4; // [5:4] Rsrv
    tmp_add_data |= 0   << 3; // [3]   bit_swap.
    tmp_add_data |= 0   << 0; // [2:0] channel_swap[2:0]
    hdmi_wr_reg(TX_TMDS_MODE, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   Rsrv
    tmp_add_data |= 0   << 6; // [6]   TX_CONNECT_SEL: 0=use lower channel data[29:0]; 1=use upper channel data[59:30]
    tmp_add_data |= 0   << 0; // [5:0] Rsrv
    hdmi_wr_reg(TX_SYS4_CONNECT_SEL_1, tmp_add_data);
    
    // Normally it makes sense to synch 3 channel output with clock channel's rising edge,
    // as HDMI's serializer is LSB out first, invert tmds_clk pattern from "1111100000" to
    // "0000011111" actually enable data synch with clock rising edge.
    tmp_add_data = 1 << 4; // Set tmds_clk pattern to be "0000011111" before being sent to AFE clock channel
    hdmi_wr_reg(TX_SYS4_CK_INV_VIDEO, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7] Rsrv
    tmp_add_data |= 0   << 6; // [6] TX_AFE_FIFO channel 2 bypass=0
    tmp_add_data |= 0   << 5; // [5] TX_AFE_FIFO channel 1 bypass=0
    tmp_add_data |= 0   << 4; // [4] TX_AFE_FIFO channel 0 bypass=0
    tmp_add_data |= 1   << 3; // [3] output enable of clk channel (channel 3)
    tmp_add_data |= 1   << 2; // [2] TX_AFE_FIFO channel 2 enable
    tmp_add_data |= 1   << 1; // [1] TX_AFE_FIFO channel 1 enable
    tmp_add_data |= 1   << 0; // [0] TX_AFE_FIFO channel 0 enable
    hdmi_wr_reg(TX_SYS5_FIFO_CONFIG, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= TX_OUTPUT_COLOR_FORMAT  << 6; // [7:6] output_color_format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= TX_INPUT_COLOR_FORMAT   << 4; // [5:4] input_color_format:  0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= TX_OUTPUT_COLOR_DEPTH   << 2; // [3:2] output_color_depth:  0=24-b; 1=30-b; 2=36-b; 3=48-b.
    tmp_add_data |= TX_INPUT_COLOR_DEPTH    << 0; // [1:0] input_color_depth:   0=24-b; 1=30-b; 2=36-b; 3=48-b.
    hdmi_wr_reg(TX_VIDEO_DTV_OPTION_L, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 4; // [7:4] Rsrv
    tmp_add_data |= TX_OUTPUT_COLOR_RANGE   << 2; // [3:2] output_color_range:  0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    tmp_add_data |= TX_INPUT_COLOR_RANGE    << 0; // [1:0] input_color_range:   0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    hdmi_wr_reg(TX_VIDEO_DTV_OPTION_H, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7] cp_desired
    tmp_add_data |= 0   << 6; // [6] ess_config
    tmp_add_data |= 0   << 5; // [5] set_avmute
    tmp_add_data |= 1   << 4; // [4] clear_avmute
    tmp_add_data |= 0   << 3; // [3] hdcp_1_1
    tmp_add_data |= 0   << 2; // [2] Vsync/Hsync forced_polarity_select
    tmp_add_data |= 0   << 1; // [1] forced_vsync_polarity
    tmp_add_data |= 0   << 0; // [0] forced_hsync_polarity
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);

    // AVI frame
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x00, 0xCC);              // PB0: Checksum
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x01, 0x56);              // PB1 (Note: the value should be meaningful but is not!)
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x02, 0x34);              // PB2 (Note: the value should be meaningful but is not!)
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x03, 0x12);              // PB3 (Note: the value should be meaningful but is not!)
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x04, VIC);               // PB4: [7]    Rsrv
                                                                        //      [6:0]  VIC
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x05, PIXEL_REPEAT_HDMI); // PB5: [7:4]  Rsrv
                                                                        //      [3:0]  PixelRepeat
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x1C, 0x82);              // HB0: packet type=0x82
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x1D, 0x02);              // HB1: packet version =0x02
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x1E, 0x0D);              // HB2: payload bytes=13
    hdmi_wr_reg(TX_PKT_REG_AVI_INFO_BASE_ADDR+0x1F, 0x80);              // Enable AVI packet generation

    // Port Enable
    tmp_add_data  = 0;
    tmp_add_data |= 0           << 7; // [7]   cdr3_force_datafd_data
    tmp_add_data |= 0           << 6; // [6]   cdr2_force_datafd_data
    tmp_add_data |= 0           << 5; // [5]   cdr1_force_datafd_data
    tmp_add_data |= 0           << 4; // [4]   cdr0_force_datafd_data
    tmp_add_data |= 0           << 3; // [3]   rsvd
    tmp_add_data |= 1           << 0; // [2:0] port_en [2:0]
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x005, tmp_add_data);

    // Set CDR ch0 pixel_clk / tmds_clk ratio
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 5; // [7:5] hogg_adj ??
    tmp_add_data |= 0                       << 2; // [4:2] dp_div_cfg
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] hdmi_div_cfg: pixel_clk/tmds_clk
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x865, tmp_add_data);
    // Enable CDR ch0
    tmp_add_data  = 0;
    tmp_add_data |= 1   << 5; // [5] cdr0_en_clk_ch
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x866, tmp_add_data);
    // Set CDR ch1 pixel_clk / tmds_clk ratio
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 5; // [7:5] hogg_adj ??
    tmp_add_data |= 0                       << 2; // [4:2] dp_div_cfg
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] hdmi_div_cfg: pixel_clk/tmds_clk
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x86D, tmp_add_data);
    // Enable CDR ch1
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 5; // [5] cdr1_en_clk_ch
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x86E, tmp_add_data);
    // Set CDR ch2 pixel_clk / tmds_clk ratio
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 5; // [7:5] hogg_adj ??
    tmp_add_data |= 0                       << 2; // [4:2] dp_div_cfg
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] hdmi_div_cfg: pixel_clk/tmds_clk
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x875, tmp_add_data);
    // Enable CDR ch2
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 5; // [5] cdr2_en_clk_ch
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x876, tmp_add_data);
    // Set CDR ch3 pixel_clk / tmds_clk ratio
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 5; // [7:5] hogg_adj ??
    tmp_add_data |= 0                       << 2; // [4:2] dp_div_cfg
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] hdmi_div_cfg: pixel_clk/tmds_clk
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x87D, tmp_add_data);
    // Enable CDR ch3
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 5; // [5] cdr3_en_clk_ch
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x87E, tmp_add_data);
    
    // Enable AFE FIFO
    tmp_add_data  = 0;
    tmp_add_data |= 0xf << 0; // [3:0] hdmidp_rx_afe_connect.fifo_enable
    ext_hdmi_wr_reg(RX_BASE_ADDR+0xA1, tmp_add_data);

    // CHANNEL_SWITCH A4
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 0; // [0] polarity_0
    tmp_add_data |= 0   << 1; // [1] polarity_1
    tmp_add_data |= 0   << 2; // [2] polarity_2
    tmp_add_data |= 0   << 3; // [3] polarity_3
    tmp_add_data |= 0   << 4; // [4] bitswap_0 
    tmp_add_data |= 0   << 5; // [5] bitswap_1 
    tmp_add_data |= 0   << 6; // [6] bitswap_2 
    tmp_add_data |= 0   << 7; // [7] bitswap_3
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x0A4, tmp_add_data);
    
    // CHANNEL_SWITCH A5
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 0; // [1:0] source_0 
    tmp_add_data |= 1   << 2; // [3:2] source_1 
    tmp_add_data |= 2   << 4; // [5:4] source_2 
    tmp_add_data |= 3   << 6; // [7:6] source_3
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x0A5, tmp_add_data);
    
    // CHANNEL_SWITCH A6
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 0; // [2:0] skew_0 
    tmp_add_data |= 0   << 3; // [3]   enable_0 
    tmp_add_data |= 0   << 4; // [6:4] skew_1 
    tmp_add_data |= 1   << 7; // [7]   enable_1
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x0A6, tmp_add_data);
    
    // CHANNEL_SWITCH A7
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 0; // [2:0] skew_2 
    tmp_add_data |= 1   << 3; // [3]   enable_2 
    tmp_add_data |= 0   << 4; // [6:4] skew_3 
    tmp_add_data |= 1   << 7; // [7]   enable_3 
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x0A7, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   Force DTV timing
    tmp_add_data |= 0   << 6; // [6]   Force Video Scan
    tmp_add_data |= 0   << 5; // [5]   Force Video field
    tmp_add_data |= 0   << 0; // [4:0] Rsrv
    ext_hdmi_wr_reg(RX_VIDEO_DTV_TIMING, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 7; // [7]   forced_default_phase
    tmp_add_data |= 1                       << 3; // [3]   forced_color_depth
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] color_depth_config
    ext_hdmi_wr_reg(RX_VIDEO_DTV_MODE, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= RX_OUTPUT_COLOR_FORMAT  << 6; // [7:6] output_color_format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= RX_INPUT_COLOR_FORMAT   << 4; // [5:4] input_color_format:  0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
    tmp_add_data |= RX_OUTPUT_COLOR_DEPTH   << 2; // [3:2] output_color_depth:  0=24-b; 1=30-b; 2=36-b; 3=48-b.
    tmp_add_data |= RX_INPUT_COLOR_DEPTH    << 0; // [1:0] input_color_depth:   0=24-b; 1=30-b; 2=36-b; 3=48-b.
    ext_hdmi_wr_reg(RX_VIDEO_DTV_OPTION_L, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0                       << 4; // [7:4] Rsrv
    tmp_add_data |= RX_OUTPUT_COLOR_RANGE   << 2; // [3:2] output_color_range:  0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    tmp_add_data |= RX_INPUT_COLOR_RANGE    << 0; // [1:0] input_color_range:   0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
    ext_hdmi_wr_reg(RX_VIDEO_DTV_OPTION_H, tmp_add_data);

    tmp_add_data  = 0;
    tmp_add_data |= 0   << 6; // [7:6] hdcp_source_select[1:0]
    tmp_add_data |= 0   << 4; // [5:4] tmds_decode_source_select[1:0]
    tmp_add_data |= 0   << 2; // [3:2] tmds_align_source_select[1:0]
    tmp_add_data |= 0   << 0; // [1:0] tmds_channel_source_select[1:0]
    ext_hdmi_wr_reg(RX_CORE_DATA_CAPTURE_1, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   forced_hdmi
    tmp_add_data |= 0   << 6; // [6]   hdmi_config
    tmp_add_data |= 0   << 5; // [5]   hdmi_reset_enable
    tmp_add_data |= 0   << 4; // [4]   1 rsvd
    tmp_add_data |= 0   << 3; // [3]   bit_swap
    tmp_add_data |= 0   << 0; // [2:0] channel_swap[2:0]
    ext_hdmi_wr_reg(RX_TMDS_MODE, tmp_add_data);
    
    tmp_add_data = 0x00; // tmds_clock_meter.ref_cycles[7:0]
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x64, tmp_add_data);

    tmp_add_data = 0x10; // tmds_clock_meter.ref_cycles[15:8]
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x65, tmp_add_data);

    tmp_add_data = 0; // tmds_clock_meter.ref_cycles[23:16]
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x66, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 7; // [7]   forced_tmds_clock_int
    tmp_add_data |= 0   << 6; // [6]   tmds_clock_int_config
    tmp_add_data |= 0   << 5; // [5]   tmds_clock_int_forced_clear
    tmp_add_data |= 1   << 4; // [4]   tmds_clock_int_auto_clear
    tmp_add_data |= 0x9 << 0; // [3:0] tmds_clock_meter.meas_tolerance[3:0]
    ext_hdmi_wr_reg(RX_BASE_ADDR+0x67, tmp_add_data);
    
    tmp_add_data = 0xa; // time_divider[7:0] for DDC I2C bus clock
    hdmi_wr_reg(TX_HDCP_CONFIG3, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 1   << 7; // [7] cp_desired 
    tmp_add_data |= 1   << 6; // [6] ess_config 
    tmp_add_data |= 0   << 5; // [5] set_avmute 
    tmp_add_data |= 0   << 4; // [4] clear_avmute 
    tmp_add_data |= 1   << 3; // [3] hdcp_1_1 
    tmp_add_data |= 0   << 2; // [2] forced_polarity 
    tmp_add_data |= 0   << 1; // [1] forced_vsync_polarity 
    tmp_add_data |= 0   << 0; // [0] forced_hsync_polarity
    hdmi_wr_reg(TX_HDCP_MODE, tmp_add_data);
    
    tmp_add_data = 9; // time_divider[7:0] for DDC I2C bus clock
    ext_hdmi_wr_reg(RX_HDCP_CONFIG3, tmp_add_data);
    
    tmp_add_data  = 0;
    tmp_add_data |= 0   << 6; // [7:6] feed_through_mode 
    tmp_add_data |= 0   << 5; // [5]   gated_hpd 
    tmp_add_data |= 0   << 4; // [4]   forced_hpd 
    tmp_add_data |= 0   << 3; // [3]   hpd_config 
    tmp_add_data |= 1   << 2; // [2]   forced_ksv:0=automatic read after hpd; 1=manually triggered read
    tmp_add_data |= 1   << 1; // [1]   ksv_config:0=disable; 1=enable
    tmp_add_data |= 0   << 0; // [0]   read_km
    ext_hdmi_wr_reg(RX_HDCP_CONFIG0, tmp_add_data);

    // --------------------------------------------------------
    // Release TX out of reset
    // --------------------------------------------------------
    Wr(HHI_HDMI_PLL_CNTL1, 0x00040000);         // turn off phy_clk
    Wr(HHI_HDMI_PLL_CNTL1, 0x00040003);         // turn on phy_clk
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_2, 0x00); // Release reset on TX digital clock channel
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 1<<6); // Release resets all other TX digital clock domain, except tmds_clk
    hdmi_wr_reg(TX_SYS5_TX_SOFT_RESET_1, 0x00); // Final release reset on tmds_clk domain
}

// Use this self-made function rather than %, because % appears to produce wrong
// value for divisor which are not 2's exponential.
unsigned long modulo(unsigned long a, unsigned long b)
{
    if (a >= b) {
        return(a-b);
    } else {
        return(a);
    }
}
        
signed int to_signed(unsigned int a)
{
    if (a <= 7) {
        return(a);
    } else {
        return(a-16);
    }
}

