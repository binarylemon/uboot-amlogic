//#include <linux/types.h>
//#include <mach/cpu.h>
//#include <plat/cpu.h>
//#include <linux/amlogic/vout/lcdoutc.h>
#if 1//(MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON8)
//#include <linux/kernel.h>
#include "mipi_dsi_util.h"
#include <asm/arch-m8/mipi_dsi_reg.h>
//#include <linux/amlogic/vout/lcd_reg.h>

#include <asm/arch/io.h>
#include <asm/arch/lcd_reg.h>
#if 0
#include <mach/io.h>
#include <plat/io.h>
#include <linux/delay.h>
#endif

#ifndef VPP_OUT_SATURATE
#define VPP_OUT_SATURATE            (1 << 0)
#endif

#ifdef CONFIG_AM_TV_OUTPUT2
extern unsigned int vpp2_sel; /*0,vpp; 1, vpp2 */
#endif
static unsigned int exp2_tbl[]={1,2,4,8};
#ifndef FIN_FREQ
#define FIN_FREQ        (24 * 1000)
#endif

void init_mipi_dsi_phy(Lcd_Config_t *p)
{
        unsigned int div;
        DSI_Config_t *cfg= p->lcd_control.mipi_config;

        div = cfg->dsi_clk_div;

      //  DPRINT("div=%d, clk_min=%d, clk_max=%d\n", div, clk_min, clk_max);

        DPRINT("MIPI_DSI_CHAN_CTRL=%x\n", LCD_DSI_ADDR(MIPI_DSI_CHAN_CTRL));
        switch (div){
                case 1:
                        div = 0;
                        break;
                case 2:
                        div = 2;
                        break;
                case 4:
                default:
                        div = 3;
                        break;
        }
        DPRINT("div=%d\n", div);
        // enable phy clock.
        //supose the VID2 PLL clock is locked and stable.
        WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1);          //enable DSI top clock.
        WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable the DSI PLL clock .
                        (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                        (1 << 8 )  |   //enable the clock divider counter
                        (0 << 9 )  |   //enable the divider clock out
                        (div << 10 ) |   //select the clock freq /2.
                        (0 << 12));    //enable the byte clock generateion.

        WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable DSI top clock.
                        (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                        (1 << 8 )  |   //enable the clock divider counter
                        (1 << 9 )  |   //enable the divider clock out
                        (div << 10 ) |   //select the clock freq /2.
                        (0 << 12));    //enable the byte clock generateion.

        WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable DSI top clock.
                        (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                        (1 << 8 )  |   //enable the clock divider counter
                        (1 << 9 )  |   //enable the divider clock out
                        (div << 10 ) |   //select the clock freq /2.
                        (1 << 12));    //enable the byte clock generateion.

        WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL,  1, 31, 1);
        WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL,  0, 31, 1);

        WRITE_DSI_REG(MIPI_DSI_CLK_TIM,  0x05210f08);//0x03211c08
        WRITE_DSI_REG(MIPI_DSI_CLK_TIM1, 0x8);//??
        WRITE_DSI_REG(MIPI_DSI_HS_TIM, 0x060f090d);//0x050f090d
        WRITE_DSI_REG(MIPI_DSI_LP_TIM, 0x4a370e0e);
        WRITE_DSI_REG(MIPI_DSI_ANA_UP_TIM, 0x0100); //?? //some number to reduce sim time.
        WRITE_DSI_REG(MIPI_DSI_INIT_TIM, 0x00d4); //0xe20   //30d4 -> d4 to reduce sim time.
        WRITE_DSI_REG(MIPI_DSI_WAKEUP_TIM, 0x48); //0x8d40  //1E848-> 48 to reduct sim time.
        WRITE_DSI_REG(MIPI_DSI_LPOK_TIM,  0x7C);   //wait for the LP analog ready.
        WRITE_DSI_REG(MIPI_DSI_ULPS_CHECK,  0x927C);   //1/3 of the tWAKEUP.
        WRITE_DSI_REG(MIPI_DSI_LP_WCHDOG,  0x1000);   // phy TURN watch dog.
        WRITE_DSI_REG(MIPI_DSI_TURN_WCHDOG,  0x1000);   // phy ESC command watch dog.

        // Powerup the analog circuit.
        WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0);
}

void powerup_mipi_dsi_dphy(void)
{
        // Power up DSI
        WRITE_LCD_REG( MIPI_DSI_DWC_PWR_UP_OS, 1);

        // Setup Parameters of DPHY
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL1_OS, 0x00010044);                            // testcode
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x2);
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x0);
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL1_OS, 0x00000074);                            // testwrite
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x2);
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x0);

        // Power up D-PHY
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_RSTZ_OS, 0xf);
}

void powerdown_mipi_dsi_dphy(void)
{
        // Power down DSI
        WRITE_LCD_REG( MIPI_DSI_DWC_PWR_UP_OS, 0);

        // Power down D-PHY, do not have to close dphy
        // WRITE_LCD_REG( MIPI_DSI_DWC_PHY_RSTZ_OS, (READ_LCD_REG( MIPI_DSI_DWC_PHY_RSTZ_OS ) & 0xc));
        // WRITE_LCD_REG( MIPI_DSI_DWC_PHY_RSTZ_OS, 0xc);
}

// -----------------------------------------------------------------------------
//                     Function: check_phy_st
// Check the status of the dphy: phylock and stopstateclklane
// -----------------------------------------------------------------------------
void check_phy_st(void)
{
        // Check the phylock/stopstateclklane to decide if the DPHY is ready
        while((( READ_LCD_REG(MIPI_DSI_DWC_PHY_STATUS_OS ) >> BIT_PHY_LOCK) & 0x1) == 0){
                delay_us(6);
        }
        while((( READ_LCD_REG(MIPI_DSI_DWC_PHY_STATUS_OS ) >> BIT_PHY_STOPSTATECLKLANE) & 0x1) == 0){
                DPRINT(" Waiting STOP STATE LANE\n");
                delay_us(6);
        }
}

// -----------------------------------------------------------------------------
//                     Function: set_mipi_dcs
// Configure relative registers in command mode
// -----------------------------------------------------------------------------
extern void set_mipi_dcs(int trans_type,                                       // 0: high speed, 1: low power
                int req_ack,                                          // 1: request ack, 0: do not need ack
                int tear_en                                           // 1: enable tear ack, 0: disable tear ack
                )
{
        WRITE_LCD_REG( MIPI_DSI_DWC_CMD_MODE_CFG_OS, (trans_type << BIT_MAX_RD_PKT_SIZE) | (trans_type << BIT_DCS_LW_TX)    |
                        (trans_type << BIT_DCS_SR_0P_TX)    | (trans_type << BIT_DCS_SW_1P_TX) |
                        (trans_type << BIT_DCS_SW_0P_TX)    | (trans_type << BIT_GEN_LW_TX)    |
                        (trans_type << BIT_GEN_SR_2P_TX)    | (trans_type << BIT_GEN_SR_1P_TX) |
                        (trans_type << BIT_GEN_SR_0P_TX)    | (trans_type << BIT_GEN_SW_2P_TX) |
                        (trans_type << BIT_GEN_SW_1P_TX)    | (trans_type << BIT_GEN_SW_0P_TX) |
                        (req_ack    << BIT_ACK_RQST_EN)     | (tear_en    << BIT_TEAR_FX_EN)  );

        if(tear_en == MIPI_DCS_ENABLE_TEAR) {
                // Enable Tear Interrupt if tear_en is valid
                WRITE_LCD_REG( MIPI_DSI_TOP_INTR_CNTL_STAT, (READ_LCD_REG(MIPI_DSI_TOP_INTR_CNTL_STAT) | (0x1<<BIT_EDPITE_INT_EN)) );
                // Enable Measure Vsync
                WRITE_LCD_REG( MIPI_DSI_TOP_MEAS_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_MEAS_CNTL) | (0x1<<BIT_VSYNC_MEAS_EN
                                                | (0x1<<BIT_TE_MEAS_EN))) );
        }
}
// -----------------------------------------------------------------------------
//                     Function: set_mipi_int
// Configure relative registers for mipi interrupt
// -----------------------------------------------------------------------------
extern void set_mipi_int(void)
{
        WRITE_LCD_REG( MIPI_DSI_DWC_INT_MSK0_OS, 0);
        WRITE_LCD_REG( MIPI_DSI_DWC_INT_MSK1_OS, 0);
}

// ----------------------------------------------------------------------------
//                     Function: set_mipi_dsi_host
// Configure the mipi dsi host according the trans type
// ----------------------------------------------------------------------------
void set_mipi_dsi_host_to_command_mode(int lane_num,                                           // lane number, from 1 to 4
                int vcid,                                               // virtual id
                int venc_color_code,                                    // VENC output data width
                int color_code,                                         // color code
                int chroma_subsample,                                   // chroma_subsample for YUV422 or YUV420 only
                int trans_mode,                                         // video mode/command mode
                tv_enc_lcd_type_t output_type,                          // video type, such as 1080x720
                int vid_mode_type,                                      // video mode : burst/non_burst
                int check_phy_status,                                   // enable/disable phy lock check, disable for multiple pic test
                Lcd_Config_t *p)
{
        int real_lane_num = lane_num+1;
        int pic_width, pic_height;
        int num_of_chunk;
        int pixel_per_chunk = 4;
        int byte_per_chunk = 0;
        int totol_bytes_per_chunk;
        int chunk_overhead;

        pic_width  = p->lcd_basic.h_active;
        pic_height = p->lcd_basic.v_active;
        totol_bytes_per_chunk = real_lane_num*pixel_per_chunk*3/4;

        // one lene has 8 bytes for 4 pixels
        // according to DSI spec line50
        switch(color_code) {
                case COLOR_24_BIT_YCBCR :
                case COLOR_24BIT        :
                        byte_per_chunk = 18;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        // byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        break;
                case COLOR_20BIT_LOOSE  :
                case COLOR_18BIT_CFG_2  :
                        byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        break;
                case COLOR_16BIT_YCBCR :
                case COLOR_16BIT_CFG_1 :
                case COLOR_16BIT_CFG_2 :
                case COLOR_16BIT_CFG_3 :
                        byte_per_chunk =  8;                                             // at least 3 lanes(8+6=14): 3*8-14=10>6
                        break;
                case COLOR_30BIT :
                        byte_per_chunk = 15;                                             // at least 4 lanes(15+6=21): 4*8-21=11>6
                        break;
                case COLOR_36BIT :
                        byte_per_chunk = 18;                                             // at least 4 leans(18+6=24): 4*8-24=8>6
                        break;
                case COLOR_12BIT :
                        byte_per_chunk =  6;                                             // at least 3 leans(6+6=12): 3*8-12=12>6
                        break;
                case COLOR_18BIT_CFG_1 :
                        byte_per_chunk =  9;                                             // at least 23lanes(9+6=15): 3*8-15=9>6
                        break;
                case COLOR_RGB_111 :
                case COLOR_RGB_332 :
                case COLOR_RGB_444 :
                        break;
                default :
                        DPRINT(" Error: Unsupport Color Format So Far, Please Add More\n");
                        break;
        }    /*switch(color_code)*/
        num_of_chunk = pic_width/pixel_per_chunk;
        chunk_overhead = totol_bytes_per_chunk-(byte_per_chunk+6);                 // byte_per_chunk+6=valid_payload

        // -----------------------------------------------------
        // Standard Configuration for Video Mode Transfer
        // -----------------------------------------------------
        // 1,    Configure Lane number and phy stop wait time
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (0x28 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (1 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
        }

        // 2.1,  Configure Virtual channel settings
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_VCID_OS, vcid );
        // 2.2,  Configure Color format
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_COLOR_CODING_OS, ((color_code == COLOR_18BIT_CFG_2 ? 1 : 0) << BIT_LOOSELY18_EN) |
                        (color_code << BIT_DPI_COLOR_CODING) );
        // 2.2.1 Configure Set color format for DPI register
        WRITE_LCD_REG( MIPI_DSI_TOP_CNTL, ((READ_LCD_REG(MIPI_DSI_TOP_CNTL) & ~(0xf<<BIT_DPI_COLOR_MODE) & ~(0x7<<BIT_IN_COLOR_MODE) & ~(0x3<<BIT_CHROMA_SUBSAMPLE)) |
                                (color_code         << BIT_DPI_COLOR_MODE)  |
                                (venc_color_code    << BIT_IN_COLOR_MODE)   |
                                (chroma_subsample   << BIT_CHROMA_SUBSAMPLE)) );
        // 2.3   Configure Signal polarity
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_CFG_POL_OS, (0x0 << BIT_COLORM_ACTIVE_LOW) |
                        (0x0 << BIT_SHUTD_ACTIVE_LOW)  |
                        (0x0 << BIT_HSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_VSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_DATAEN_ACTIVE_LOW));

        // -----------------------------------------------------
        // Finish Configuration
        // -----------------------------------------------------


        // Inner clock divider settings
        WRITE_LCD_REG( MIPI_DSI_DWC_CLKMGR_CFG_OS, (0x1 << BIT_TO_CLK_DIV) |
                        (0x1e << BIT_TX_ESC_CLK_DIV) );
        // Packet header settings
        WRITE_LCD_REG( MIPI_DSI_DWC_PCKHDL_CFG_OS, (1 << BIT_CRC_RX_EN) |
                        (1 << BIT_ECC_RX_EN) |
                        (0 << BIT_BTA_EN) |
                        (0 << BIT_EOTP_RX_EN) |
                        (0 << BIT_EOTP_TX_EN) );
        // transfer mode setting: video/command mode
        WRITE_LCD_REG( MIPI_DSI_DWC_MODE_CFG_OS, trans_mode );

        // Phy Timer
        // WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x036e0000);
        // WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x03370000);
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x03320000);
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x090f0000);
        }

        // Configure DPHY Parameters
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x870025);
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x260017);
        }


}
// ----------------------------------------------------------------------------
//                     Function: set_mipi_dsi_host
// Configure the mipi dsi host according the trans type
// ----------------------------------------------------------------------------
void set_mipi_dsi_host_to_video_mode(int lane_num,                                           // lane number, from 1 to 4
                int vcid,                                               // virtual id
                int venc_color_code,                                    // VENC output data width
                int color_code,                                         // color code
                int chroma_subsample,                                   // chroma_subsample for YUV422 or YUV420 only
                int trans_mode,                                         // video mode/command mode
                tv_enc_lcd_type_t output_type,                          // video type, such as 1080x720
                int vid_mode_type,                                      // video mode : burst/non_burst
                int check_phy_status,                                   // enable/disable phy lock check, disable for multiple pic test
                Lcd_Config_t *p)

{
        int real_lane_num = lane_num+1;
        int pic_width, pic_height;
        int num_of_chunk;
        int pixel_per_chunk = 4;
        int byte_per_chunk;
        int totol_bytes_per_chunk;
        int chunk_overhead;
        int vid_null_size = 0;

        pic_width  = p->lcd_basic.h_active;
        pic_height = p->lcd_basic.v_active;
        totol_bytes_per_chunk = real_lane_num*pixel_per_chunk*3/4;

        // one lene has 8 bytes for 4 pixels
        // according to DSI spec line50
        switch(color_code) {
                case COLOR_24_BIT_YCBCR :
                case COLOR_24BIT        :
                        byte_per_chunk = 18;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        // byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        break;
                case COLOR_20BIT_LOOSE  :
                case COLOR_18BIT_CFG_2  :
                        byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
                        break;
                case COLOR_16BIT_YCBCR :
                case COLOR_16BIT_CFG_1 :
                case COLOR_16BIT_CFG_2 :
                case COLOR_16BIT_CFG_3 :
                        byte_per_chunk =  8;                                             // at least 3 lanes(8+6=14): 3*8-14=10>6
                        break;
                case COLOR_30BIT :
                        byte_per_chunk = 15;                                             // at least 4 lanes(15+6=21): 4*8-21=11>6
                        break;
                case COLOR_36BIT :
                        byte_per_chunk = 18;                                             // at least 4 leans(18+6=24): 4*8-24=8>6
                        break;
                case COLOR_12BIT :
                        byte_per_chunk =  6;                                             // at least 3 leans(6+6=12): 3*8-12=12>6
                        break;
                case COLOR_18BIT_CFG_1 :
                        byte_per_chunk =  9;                                             // at least 23lanes(9+6=15): 3*8-15=9>6
                        break;
                case COLOR_RGB_111 :
                case COLOR_RGB_332 :
                case COLOR_RGB_444 :
                        break;
                default :
                        DPRINT(" Error: Unsupport Color Format So Far, Please Add More\n");
                        break;
        }    /*switch(color_code)*/
        num_of_chunk = pic_width/pixel_per_chunk;
        chunk_overhead = totol_bytes_per_chunk-(byte_per_chunk+6);                 // byte_per_chunk+6=valid_payload

        if(trans_mode == TRANS_VIDEO_MODE && vid_mode_type != BURST_MODE) {
                if(chunk_overhead >= 6) {                                              // if room for null_vid's head(4)+crc(2)
                        vid_null_size = chunk_overhead-6;                                  // chunk_overhead-null_vid's head(4)+crc(2) = null_vid's payload
                } else {
                        DPRINT(" No room for null, chunk_overhead is %d\n", chunk_overhead);
                }
        }
        // -----------------------------------------------------
        // Standard Configuration for Video Mode Transfer
        // -----------------------------------------------------
        // 1,    Configure Lane number and phy stop wait time
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (0x28 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (1 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
        }

        // 2.1,  Configure Virtual channel settings
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_VCID_OS, vcid );
        // 2.2,  Configure Color format
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_COLOR_CODING_OS, ((color_code == COLOR_18BIT_CFG_2 ? 1 : 0) << BIT_LOOSELY18_EN) |
                        (color_code << BIT_DPI_COLOR_CODING) );
        // 2.2.1 Configure Set color format for DPI register
        WRITE_LCD_REG( MIPI_DSI_TOP_CNTL, ((READ_LCD_REG(MIPI_DSI_TOP_CNTL) & ~(0xf<<BIT_DPI_COLOR_MODE) & ~(0x7<<BIT_IN_COLOR_MODE) & ~(0x3<<BIT_CHROMA_SUBSAMPLE)) |
                                (color_code         << BIT_DPI_COLOR_MODE)  |
                                (venc_color_code    << BIT_IN_COLOR_MODE)   |
                                (chroma_subsample   << BIT_CHROMA_SUBSAMPLE)) );
        // 2.3   Configure Signal polarity
        WRITE_LCD_REG( MIPI_DSI_DWC_DPI_CFG_POL_OS, (0x0 << BIT_COLORM_ACTIVE_LOW) |
                        (0x0 << BIT_SHUTD_ACTIVE_LOW)  |
                        (0x0 << BIT_HSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_VSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_DATAEN_ACTIVE_LOW));

        // 3.1   Configure Low power and video mode type settings
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_MODE_CFG_OS, (1 << BIT_LP_HFP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_HBP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VCAT_EN) |                  // enalbe lp
                        (1 << BIT_LP_VFP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VBP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VSA_EN)  |                  // enalbe lp
                        (1 << BIT_FRAME_BTA_ACK_EN) |            // enable BTA after one frame, TODO, need check
                        (vid_mode_type << BIT_VID_MODE_TYPE) );  // burst/non burst


        // 3.2   Configure video packet size settings
        if( vid_mode_type == BURST_MODE ) {                                        // burst mode
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_PKT_SIZE_OS, pic_width);                          // should be one line in pixels, such as 480/240...
        }
        else if(vid_mode_type == NON_BURST_SYNC_PULSE ||
                        vid_mode_type == NON_BURST_SYNC_EVENT) {                           // non-burst mode
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_PKT_SIZE_OS, pixel_per_chunk);                    // in unit of pixels, (pclk period/byte clk period)*num_of_lane should be integer
                // in our system, 16/8*num_of_lane is integer, so 6 pixel should be enough for 24bpp
                // Worst case: (16/8)*8(pixel)*1(lane) >= 6(pkt head+crc)+3(max 24bpp)
        }

        // 3.3   Configure number of chunks and null packet size for one line
        if( vid_mode_type == BURST_MODE ) {                                        // burst mode
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_NUM_CHUNKS_OS, 0);
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_NULL_SIZE_OS, 0);
        }
        else {                                                                     // non burst mode
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_NUM_CHUNKS_OS, num_of_chunk);                     // HACT/VID_PKT_SIZE
                WRITE_LCD_REG( MIPI_DSI_DWC_VID_NULL_SIZE_OS, vid_null_size);                     // video null size
                DPRINT(" ============== NON_BURST SETTINGS =============\n");
                DPRINT(" pixel_per_chunk       = %d\n", pixel_per_chunk);
                DPRINT(" num_of_chunk          = %d\n", num_of_chunk);
                DPRINT(" totol_bytes_per_chunk = %d\n", totol_bytes_per_chunk);
                DPRINT(" byte_per_chunk        = %d\n", byte_per_chunk);
                DPRINT(" chunk_overhead        = %d\n", chunk_overhead);
                DPRINT(" vid_null_size         = %d\n", vid_null_size);
                DPRINT(" ===============================================\n");
        }

        // 4     Configure the video relative parameters according to the output type
        //         include horizontal timing and vertical line
        config_video_para(p);

        // -----------------------------------------------------
        // Finish Configuration
        // -----------------------------------------------------


        // Inner clock divider settings
        WRITE_LCD_REG( MIPI_DSI_DWC_CLKMGR_CFG_OS, (0x1 << BIT_TO_CLK_DIV) |
                        (0x1e << BIT_TX_ESC_CLK_DIV) );
        // Packet header settings
        WRITE_LCD_REG( MIPI_DSI_DWC_PCKHDL_CFG_OS, (1 << BIT_CRC_RX_EN) |
                        (1 << BIT_ECC_RX_EN) |
                        (0 << BIT_BTA_EN) |
                        (0 << BIT_EOTP_RX_EN) |
                        (0 << BIT_EOTP_TX_EN) );
        // transfer mode setting: video/command mode
        WRITE_LCD_REG( MIPI_DSI_DWC_MODE_CFG_OS, trans_mode );

        // Phy Timer
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x03320000);
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x090f0000);
        }

        // Configure DPHY Parameters
        if ((output_type != TV_ENC_LCD240x160_dsi) &&
                        (output_type != TV_ENC_LCD1920x1200p) &&
                        (output_type != TV_ENC_LCD2560x1600) &&
                        (output_type != TV_ENC_LCD768x1024p)) {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x870025);
        } else {
                WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x260017);
        }

}
void mipi_dphy_init()
{

}

void startup_transfer_cmd(void)
{
        // Startup transfer
        WRITE_LCD_REG( MIPI_DSI_DWC_LPCLK_CTRL_OS, (0x1 << BIT_AUTOCLKLANE_CTRL) |
                        (0x1 << BIT_TXREQUESTCLKHS));
}
void startup_transfer_video(void)
{
        WRITE_LCD_REG( MIPI_DSI_DWC_LPCLK_CTRL_OS, (0x1 << BIT_TXREQUESTCLKHS));
}

void print_info(int lane_num,                                           // lane number, from 1 to 4
                int vcid,                                               // virtual id
                int venc_color_code,                                    // VENC output data width
                int color_code,                                         // color code
                int chroma_subsample,                                   // chroma_subsample for YUV422 or YUV420 only
                int trans_mode,                                         // video mode/command mode
                tv_enc_lcd_type_t output_type,                          // video type, such as 1080x720
                int vid_mode_type,                                      // video mode : burst/non_burst
                int check_phy_status                                   // enable/disable phy lock check, disable for multiple pic test
               )
{
        // Information display
        DPRINT(" ================================================\n");
        if(trans_mode == TRANS_COMMAND_MODE) {
                DPRINT(" CONFIG FOR COMMAND MODE\n");
        }
        else if(trans_mode == TRANS_VIDEO_MODE) {
                DPRINT(" CONFIG FOR VIDEO MODE\n");
        }

        if(vid_mode_type == NON_BURST_SYNC_PULSE) {
                DPRINT(" VIDEO MODE TYPE is 'NON_BURST_SYNC_PULSE'\n");
        }
        else if(vid_mode_type == NON_BURST_SYNC_EVENT) {
                DPRINT(" VIDEO MODE TYPE is 'NON_BURST_SYNC_EVENT'\n");
        }
        else if(vid_mode_type == BURST_MODE) {
                DPRINT(" VIDEO MODE TYPE is 'BURST_MODE'\n");
        }

        switch(color_code) {
                case COLOR_16BIT_CFG_1  :
                        DPRINT(" COLOR CODING is 'COLOR_16BIT_CFG_1'\n");
                        break;
                case COLOR_16BIT_CFG_2  :
                        DPRINT(" COLOR CODING is 'COLOR_16BIT_CFG_2'\n");
                        break;
                case COLOR_16BIT_CFG_3  :
                        DPRINT(" COLOR CODING is 'COLOR_16BIT_CFG_3'\n");
                        break;
                case COLOR_18BIT_CFG_1  :
                        DPRINT(" COLOR CODING is 'COLOR_18BIT_CFG_1'\n");
                        break;
                case COLOR_18BIT_CFG_2  :
                        DPRINT(" COLOR CODING is 'COLOR_18BIT_CFG_2'\n");
                        break;
                case COLOR_24BIT        :
                        DPRINT(" COLOR CODING is 'COLOR_24BIT'\n");
                        break;
                case COLOR_20BIT_LOOSE  :
                        DPRINT(" COLOR CODING is 'COLOR_20BIT_LOOSE'\n");
                        break;
                case COLOR_24_BIT_YCBCR :
                        DPRINT(" COLOR CODING is 'COLOR_24BIT_YCBCR'\n");
                        break;
                case COLOR_16BIT_YCBCR  :
                        DPRINT(" COLOR CODING is 'COLOR_16BIT_YCBCR'\n");
                        break;
                case COLOR_30BIT        :
                        DPRINT(" COLOR CODING is 'COLOR_30BIT'\n");
                        break;
                case COLOR_36BIT        :
                        DPRINT(" COLOR CODING is 'COLOR_36BIT'\n");
                        break;
                case COLOR_12BIT        :
                        DPRINT(" COLOR CODING is 'COLOR_12BIT'\n");
                        break;
                case COLOR_RGB_111      :
                        DPRINT(" COLOR CODING is 'COLOR_RGB_111'\n");
                        break;
                case COLOR_RGB_332      :
                        DPRINT(" COLOR CODING is 'COLOR_RGB332'\n");
                        break;
                case COLOR_RGB_444      :
                        DPRINT(" COLOR CODING is 'COLOR_RGB444'\n");
                        break;
                default            :
                        DPRINT(" Error: Unsupport Color Format, Please Check\n");
                        break;
        }
        DPRINT(" POLARITY is HIGH ACTIVE\n");
        DPRINT(" ENABLE CRC/ECC/BTA\n");
        DPRINT(" ================================================\n");

}

// -----------------------------------------------------------------------------
//                     Function: delay_us
// Delay by N us.
// -----------------------------------------------------------------------------
inline void delay_us (int us)
{
        udelay(us);
} /* delay_us */

// -----------------------------------------------------------------------------
//                     Function: config_video_para
// -----------------------------------------------------------------------------
void config_video_para(Lcd_Config_t *pConf)
{
				unsigned pre_div, xd, post_div;
        unsigned lcd_clk;
        unsigned pclk;
        unsigned lanebyteclk;
        unsigned int div;
        DSI_Config_t    *cfg= pConf->lcd_control.mipi_config;
        Lcd_Timing_t    *t = &pConf->lcd_timing;
        Lcd_Basic_t     *basic = &pConf->lcd_basic;

        if (0 == cfg->numerator){
                div = cfg->dsi_clk_div;
                post_div = 1;

                pre_div = ((t->div_ctrl) >> DIV_CTRL_DIV_PRE) & 0x7;
                pre_div = div_pre_table[pre_div];
                xd = ((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_XD) & 0xf;

                lcd_clk = t->lcd_clk * pre_div * post_div * xd;

                lanebyteclk = lcd_clk/div;
                lanebyteclk = lanebyteclk / 8;
                lanebyteclk = lanebyteclk/1000;
                DPRINT("pll out clk=%d Hz, lanebyteclk/1000=%d kHz\n", lcd_clk, lanebyteclk);

                pclk = pConf->lcd_timing.lcd_clk/1000;
                DPRINT("pixel clk=%d kHz\n", pclk);
                cfg->denominator = lanebyteclk/1000;
                cfg->numerator = pclk/1000;
                DPRINT("d=%d, n=%d, d/n=%d\n",
                cfg->denominator, cfg->numerator, cfg->denominator/cfg->numerator);
        }

        cfg->hline =(basic->h_period*cfg->denominator + cfg->numerator - 1) / cfg->numerator;  // Rounded. Applicable for Period(pixclk)/Period(bytelaneclk)=9/16
        cfg->hsa   = (t->hsync_width*cfg->denominator + cfg->numerator - 1) / cfg->numerator;
        cfg->hbp   = (t->hsync_bp * cfg->denominator + cfg->numerator - 1) / cfg->numerator;

        cfg->vsa   = t->vsync_width;
        cfg->vbp   = t->vsync_bp;
        cfg->vfp   = t->video_on_line - t->vsync_width - t->vsync_bp;
        cfg->vact  = basic->v_active;

        DPRINT(" ============= VIDEO TIMING SETTING =============\n");
        DPRINT(" HLINE        = %d\n", cfg->hline);
        DPRINT(" HSA          = %d\n", cfg->hsa);
        DPRINT(" HBP          = %d\n", cfg->hbp);
       // DPRINT(" HFP          = %d\n", hfp);
        DPRINT(" VBP          = %d\n", cfg->vbp);
        DPRINT(" VFP          = %d\n", cfg->vfp);
        DPRINT(" VACT         = %d\n", cfg->vact);
        DPRINT(" ================================================\n");
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_HLINE_TIME_OS,    cfg->hline);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_HSA_TIME_OS,      cfg->hsa);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_HBP_TIME_OS,      cfg->hbp);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_VSA_LINES_OS,     cfg->vsa);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_VBP_LINES_OS,     cfg->vbp);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_VFP_LINES_OS,     cfg->vfp);
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_VACTIVE_LINES_OS, cfg->vact);
}

// ----------------------------------------------------------------------------
// Function: wait_bta_ack
// Poll to check if the BTA ack is finished
// ----------------------------------------------------------------------------
extern void wait_bta_ack(void)
{
        unsigned int phy_status;

        // Check if phydirection is RX
        do{
                phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        } while(((phy_status & 0x2) >> BIT_PHY_DIRECTION) == 0x0);

        // Check if phydirection is return to TX
        do{
                phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        } while(((phy_status & 0x2) >> BIT_PHY_DIRECTION) == 0x1);

}

// ----------------------------------------------------------------------------
// Function: wait_cmd_fifo_empty
// Poll to check if the generic command fifo is empty
// ----------------------------------------------------------------------------
extern void wait_cmd_fifo_empty(void)
{
        unsigned int cmd_status;

        do {
                cmd_status = READ_LCD_REG( MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
        } while(((cmd_status >> BIT_GEN_CMD_EMPTY) & 0x1) != 0x1);
}

// ----------------------------------------------------------------------------
// Function: wait_for_generic_read_response
// Wait for generic read response
// ----------------------------------------------------------------------------
extern unsigned int wait_for_generic_read_response(void)
{
        unsigned int timeout, phy_status, data_out;

        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        for(timeout=0; timeout<50; timeout++) {
                if(((phy_status & 0x40)>> BIT_PHY_RXULPSESC0LANE) == 0x0)    { break; }
                phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
                delay_us(1);
        }
        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        for(timeout=0; timeout<50; timeout++) {
                if(((phy_status & 0x40)>> BIT_PHY_RXULPSESC0LANE) == 0x1)    { break; }
                phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
                delay_us(1);
        }

        data_out = READ_LCD_REG( MIPI_DSI_DWC_GEN_PLD_DATA_OS );
        return data_out;
}

// ----------------------------------------------------------------------------
// Function: generic_if_wr
// Generic interface write, address has to be MIPI_DSI_DWC_GEN_PLD_DATA_OS and
// MIPI_DSI_DWC_GEN_HDR_OS, MIPI_DSI_DWC_GEN_VCID_OS
// ----------------------------------------------------------------------------
extern unsigned int generic_if_wr(unsigned int address, unsigned int data_in)
{
        if(address != MIPI_DSI_DWC_GEN_HDR_OS && address != MIPI_DSI_DWC_GEN_PLD_DATA_OS) {
                DPRINT(" Error Address : %x\n", address);
        }

        DPRINT("MIPI_DSI_UTIL.C address is %x, data_in is %x\n", address, data_in);
        WRITE_LCD_REG(address, data_in);

        return 1;
}

// ----------------------------------------------------------------------------
//                           Function: generic_if_rd
// Generic interface read, address has to be MIPI_DSI_DWC_GEN_PLD_DATA_OS
// ----------------------------------------------------------------------------
extern unsigned int generic_if_rd(unsigned int address)
{
        unsigned int data_out;

        if(address != MIPI_DSI_DWC_GEN_PLD_DATA_OS) {
                DPRINT(" Error Address : %x\n", address);
        }

        data_out = READ_DSI_REG( address );

        return data_out;
}

// ----------------------------------------------------------------------------
// Function: DCS_write_short_packet_0_para
// DCS Write Short Packet 0 Parameter with Generic Interface
// Supported DCS Command: DCS_ENTER_SLEEP_MODE/DCS_EXIT_IDLE_MODE/DCS_EXIT_INVERT_MODE
//                        DCS_EXIT_SLEEP_MODE/DCS_SET_DISPLAY_OFF/DCS_SET_DISPLAY_ON
//                        DCS_SET_TEAR_OFF/DCS_SOFT_RESET/DCS_NOP
// ----------------------------------------------------------------------------
extern void DCS_write_short_packet_0_para(unsigned int data_type,
                unsigned int vc_id,
                unsigned int dcs_command,
                unsigned int req_ack)
{
        generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((0x0 << BIT_GEN_WC_MSBYTE) |
                                (dcs_command << BIT_GEN_WC_LSBYTE) |
                                (vc_id << BIT_GEN_VC)              |
                                (data_type << BIT_GEN_DT)));
        if( req_ack == MIPI_DSI_DCS_REQ_ACK ) {
                wait_bta_ack();
        }
        else if( req_ack == MIPI_DSI_DCS_NO_ACK ) {
                wait_cmd_fifo_empty();
        }
}

// ----------------------------------------------------------------------------
// Function: DCS_write_short_packet_1_para
// DCS Write Short Packet 1 Parameter with Generic Interface
// Supported DCS Command: DCS_SET_ADDRESS_MODE/DCS_SET_GAMMA_CURVE/
//                        DCS_SET_PIXEL_FORMAT/DCS_SET_TEAR_ON
// ----------------------------------------------------------------------------
extern void DCS_write_short_packet_1_para(unsigned int data_type,
                unsigned int vc_id,
                unsigned int dcs_command,
                unsigned int para,
                unsigned int req_ack)
{
        // DPRINT(" para is %x, dcs_command is %x\n", para, dcs_command);
        // DPRINT(" vc_id %x, data_type is %x\n", vc_id, data_type);
        generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((para << BIT_GEN_WC_MSBYTE)        |
                                (dcs_command << BIT_GEN_WC_LSBYTE) |
                                (vc_id << BIT_GEN_VC)              |
                                (data_type << BIT_GEN_DT)));
        if( req_ack == MIPI_DSI_DCS_REQ_ACK ) {
                wait_bta_ack();
        }
        else if( req_ack == MIPI_DSI_DCS_NO_ACK ) {
                wait_cmd_fifo_empty();
        }
}

// ----------------------------------------------------------------------------
//                           Function: DCS_read_packet_no_para
// DCS Write Short Packet 1 Parameter with Generic Interface
// Supported DCS Command: DCS_SET_ADDRESS_MODE/DCS_SET_GAMMA_CURVE/
//                        DCS_SET_PIXEL_FORMAT/DCS_SET_TEAR_ON
// ----------------------------------------------------------------------------
extern unsigned int DCS_read_packet_no_para(unsigned int data_type,
                unsigned int vc_id,
                unsigned int dcs_command)
{
        unsigned read_data;

        // DPRINT(" para is %x, dcs_command is %x\n", para, dcs_command);
        // DPRINT(" vc_id %x, data_type is %x\n", vc_id, data_type);
        generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((0x0 << BIT_GEN_WC_MSBYTE)         |
                                (dcs_command << BIT_GEN_WC_LSBYTE) |
                                (vc_id << BIT_GEN_VC)              |
                                (data_type << BIT_GEN_DT)));

        read_data = wait_for_generic_read_response();

        return read_data;
}

// ----------------------------------------------------------------------------
//                           Function: DCS_long_write_packet
// DCS Long Write Packet
// Supported DCS Command: DCS_SET_COLUMN_ADDRESS/DCS_SET_PAGE_ADDRESS
//                        DCS_WRITE_MEMORY_START/DCS_WRITE_MEMORY_CONTINUE
// ----------------------------------------------------------------------------
extern void DCS_long_write_packet(unsigned int data_type,                      // DSI data type, such as DCS Long Write Packet
                unsigned int vc_id,                          // Virtual Channel ID
                unsigned int dcs_command,                    // DCS Command, such as set_column_address/set_page_address
                unsigned char* payload,                      // Payload(including dcs_cmd+payload)
                unsigned int pld_size,                       // Payload size, from LSB to MSB, include dcs_cmd+payload
                unsigned int req_ack                         // if need check ack for bta
                )
{
        unsigned int payload_data, header_data;
        unsigned int cmd_status;
        unsigned int i;

        payload_data = 0;

        // Write Payload Register First
        for(i=0; i<pld_size; i++) {
                if(i%4 == 0)    { payload_data = 0; }
                payload_data = payload_data | (payload[i] << 8*(i%4));

                if(i%4 == 3 || i == pld_size-1) {                                      // when last byte
                        // Check the pld fifo status before write to it, do not need check every word
                        if(i == pld_size/3 || i == pld_size/2) {
                                do {
                                        cmd_status = READ_LCD_REG( MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
                                } while(((cmd_status >> BIT_GEN_PLD_W_FULL) & 0x1) == 0x1);
                        }
                        if(dcs_command == DCS_WRITE_MEMORY_CONTINUE) {
                                WRITE_LCD_REG( MIPI_DSI_DWC_GEN_PLD_DATA_OS, payload_data);               // Use direct memory write to save time when in WRITE_MEMORY_CONTINUE
                                // (*(MIPI_DSI_DWC_GEN_PLD_DATA_OS|0xc1100000)) = payload_data;
                                // *(volatile unsigned long *)((0xc1100000)|((MIPI_DSI_DWC_GEN_PLD_DATA_OS)<<2))=payload_data;
                        }
                        else {
                                generic_if_wr(MIPI_DSI_DWC_GEN_PLD_DATA_OS, payload_data);
                        }
                }
        }

        // Check cmd fifo status before write to it
        do {
                cmd_status = READ_LCD_REG( MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
        } while(((cmd_status >> BIT_GEN_CMD_FULL) & 0x1) == 0x1);
        // Write Header Register
        header_data = (((((pld_size) & 0xff00) >> 8) << BIT_GEN_WC_MSBYTE) |
                        (((pld_size) & 0x00ff) << BIT_GEN_WC_LSBYTE)        |
                        (vc_id << BIT_GEN_VC)                               |
                        (data_type << BIT_GEN_DT));
        generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, header_data);

        // Check BTA ACK
        if( req_ack == MIPI_DSI_DCS_REQ_ACK ) {
                wait_bta_ack();
        }
        else if( req_ack == MIPI_DSI_DCS_NO_ACK ) {
                wait_cmd_fifo_empty();
        }
}

#if 0
void set_pll_mipi(Lcd_Config_t *p)
{
        int pll_lock;
        int wait_loop = 100;

        Lcd_Control_Config_t *pConf = &p->lcd_control;

        pConf->mipi_config->venc_color_type = 2;
        pConf->mipi_config->dpi_color_type  = 4;

        pConf->mipi_config->trans_mode = 1;
        pConf->mipi_config->venc_fmt = TV_ENC_LCD768x1024p;
        pConf->mipi_config->lane_num = 3;
        pConf->mipi_config->dpi_chroma_subsamp = 0;

	// Configure VS/HS/DE polarity before mipi_dsi_host.pixclk starts,
	WRITE_LCD_REG(MIPI_DSI_TOP_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_CNTL) & ~(0x7<<4))   |
                          (1  << 4)               |
                          (1  << 5)               |
                          (0  << 6));

         WRITE_CBUS_REG(HHI_VID_PLL_CNTL5,  0x00012286);
         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL2, 0x0431aa1e);
         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL3, 0xca45b023);
         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL4, 0xd4000d67);

         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL5, 0x00700111);
         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL,  0x61000230);
         WRITE_CBUS_REG(HHI_VID2_PLL_CNTL,  0x41000230);

	do{
		udelay(100);
		pll_lock = (READ_CBUS_REG(HHI_VID2_PLL_CNTL) >> 31) & 0x1;
		if (wait_loop < 20){
			DPRINT("vid2_pll_locked=%u, wait_lock_loop=%d\n", pll_lock, (100 - wait_loop + 1));
                }
		wait_loop--;
	}while((pll_lock == 0) && (wait_loop > 0));

        WRITE_CBUS_REG(HHI_DSI_LVDS_EDP_CNTL1, READ_CBUS_REG(HHI_DSI_LVDS_EDP_CNTL1) & ~(1<<4)); //0x10d2
        //WRITE_CBUS_REG(HHI_VID_DIVIDER_CNTL, 0x0001d923); //0x1066
        WRITE_CBUS_REG(HHI_VIID_DIVIDER_CNTL, 0x0001d923); //0x104c
        WRITE_CBUS_REG( HHI_VID2_PLL_CNTL5, READ_CBUS_REG(HHI_VID2_PLL_CNTL5)  | //0x10e4
                         (1 << 23)               |   // Enable MIPI DSI PHY clock
                         (1 << 24));                 // Enable LVDS clock for ENCL pixel clock

        //WRITE_CBUS_REG( HHI_VID_CLK_CNTL, READ_CBUS_REG(HHI_VID_CLK_CNTL) |  (1 << 19) | (1 << 0));   //enable clk_div0, 0x105f
        //WRITE_CBUS_REG( HHI_VID_CLK_CNTL, READ_CBUS_REG(HHI_VID_CLK_CNTL) |  (1 << 20) | (1 << 0));   //enable clk_div1, 0x105f

        WRITE_CBUS_REG_BITS( HHI_VIID_CLK_CNTL, 4, 16, 3);//enable vid pll input, 0x104b
	WRITE_CBUS_REG_BITS( HHI_VIID_CLK_CNTL, 1, 19, 1);	//vclk2_en0
        //WRITE_CBUS_REG_BITS( HHI_VID_CLK_DIV,   0, 0, 8 ); //0x1059
        //WRITE_CBUS_REG_BITS( HHI_VID_CLK_CNTL, 1, 0, 1);   //0x105f
        //
        WRITE_CBUS_REG_BITS( HHI_VIID_CLK_DIV, 0, 0, 4);//0x104a[0:4], N=0, bypass
        WRITE_CBUS_REG_BITS( HHI_VIID_CLK_CNTL, 1, 0, 1);// 0x104b[0]=1 v2 clk div en
        WRITE_CBUS_REG_BITS( HHI_VIID_CLK_DIV, 1, 19, 1);//0x104a[19], enable

	WRITE_CBUS_REG_BITS( HHI_VIID_CLK_CNTL, 1, 15, 1);	//vclk2 reset
	WRITE_CBUS_REG_BITS( HHI_VIID_CLK_CNTL, 0, 15, 1);	//vclk2 reset

        WRITE_CBUS_REG_BITS( HHI_VIID_CLK_DIV,  8, 12, 4);//0x104a[12:15] v2_clk_div1
        DPRINT("%s, %d\n", __func__, __LINE__);

        //DPRINT("pll output stably, vid2 pll clk = %d", clk_util_clk_msr(6));
        //DPRINT("pll output stably, cts_encl_clk = %d", clk_util_clk_msr(9));

        //startup_mipi_dsi_host()
        WRITE_CBUS_REG( HHI_DSI_LVDS_EDP_CNTL0, 0x0);                                          // Select DSI as the output for u_dsi_lvds_edp_top
        WRITE_LCD_REG( MIPI_DSI_TOP_SW_RESET, (READ_LCD_REG(MIPI_DSI_TOP_SW_RESET) | 0xf) );     // Release mipi_dsi_host's reset
        WRITE_LCD_REG( MIPI_DSI_TOP_SW_RESET, (READ_LCD_REG(MIPI_DSI_TOP_SW_RESET) & 0xfffffff0) );     // Release mipi_dsi_host's reset
        WRITE_LCD_REG( MIPI_DSI_TOP_CLK_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_CLK_CNTL) | 0x3) );            // Enable dwc mipi_dsi_host's clock

}
#endif

void set_venc_mipi(Lcd_Config_t *pConf)
{
	WRITE_LCD_REG(ENCL_VIDEO_EN, 0);

#ifdef CONFIG_AM_TV_OUTPUT2
	if	(vpp2_sel) {
		WRITE_LCD_REG_BITS(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2);	//viu2 select encl
		WRITE_LCD_REG(VPU_VIU_VENC_MUX_CTRL, (READ_LCD_REG(VPU_VIU_VENC_MUX_CTRL)&(~(0x3<<2)))|(0x0<<2)); //viu2 select encl
	}
	else {
		WRITE_LCD_REG_BITS(VPU_VIU_VENC_MUX_CTRL, 0x88, 4, 8);//Select encl clock to VDIN, Enable VIU of ENC_l domain to VDIN
		WRITE_LCD_REG_BITS(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2);//viu1 select encl
	}
#else
	WRITE_LCD_REG(VPU_VIU_VENC_MUX_CTRL, (0<<0) | (0<<2));	// viu1 & viu2 select encl
#endif

        WRITE_LCD_REG(ENCL_VIDEO_MODE,		0);
        WRITE_LCD_REG(ENCL_VIDEO_MODE_ADV,	0x0008);	// Sampling rate: 1

        WRITE_LCD_REG(ENCL_VIDEO_FILT_CTRL,	0x1000);	// bypass filter

        WRITE_LCD_REG(ENCL_VIDEO_MAX_PXCNT,	pConf->lcd_basic.h_period - 1);
        WRITE_LCD_REG(ENCL_VIDEO_MAX_LNCNT,	pConf->lcd_basic.v_period - 1);

        WRITE_LCD_REG(ENCL_VIDEO_HAVON_BEGIN,	pConf->lcd_timing.video_on_pixel);
        WRITE_LCD_REG(ENCL_VIDEO_HAVON_END,	pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel);
        WRITE_LCD_REG(ENCL_VIDEO_VAVON_BLINE,	pConf->lcd_timing.video_on_line);
        WRITE_LCD_REG(ENCL_VIDEO_VAVON_ELINE,	pConf->lcd_basic.v_active - 1  + pConf->lcd_timing.video_on_line);

				WRITE_LCD_REG(ENCL_VIDEO_HSO_BEGIN,	pConf->lcd_timing.video_on_pixel - pConf->lcd_timing.hsync_width - pConf->lcd_timing.hsync_bp +3 -1);
				WRITE_LCD_REG(ENCL_VIDEO_HSO_END,	pConf->lcd_timing.video_on_pixel - pConf->lcd_timing.hsync_bp +3 -1);
				WRITE_LCD_REG(ENCL_VIDEO_VSO_BEGIN,	pConf->lcd_timing.video_on_pixel - pConf->lcd_timing.hsync_width - pConf->lcd_timing.hsync_bp +3 -1);
				WRITE_LCD_REG(ENCL_VIDEO_VSO_END,	pConf->lcd_timing.video_on_pixel - pConf->lcd_timing.hsync_width - pConf->lcd_timing.hsync_bp +3 -1);

				WRITE_LCD_REG(ENCL_VIDEO_VSO_BLINE,	pConf->lcd_timing.video_on_line  - pConf->lcd_timing.vsync_width - pConf->lcd_timing.vsync_bp);
				WRITE_LCD_REG(ENCL_VIDEO_VSO_ELINE,	pConf->lcd_timing.video_on_line  - pConf->lcd_timing.vsync_bp);

        WRITE_LCD_REG(ENCL_VIDEO_RGBIN_CTRL, 	(1 << 0));//(1 << 1) | (1 << 0));	//bit[0] 1:RGB, 0:YUV

        WRITE_LCD_REG(ENCL_VIDEO_EN,           1);

	// enable encl
}

void set_control_mipi(Lcd_Config_t *p)
{
        unsigned int        mipi_dsi_dpi_color_type;
        unsigned int        mipi_dsi_venc_color_type;
        tv_enc_lcd_type_t   venc_format;
        unsigned char       lane_num;
        unsigned char       chroma_subsamp;

        unsigned int        data32;
        unsigned int        is_rgb;
        unsigned int        dith10_en, dith8_en, dith5_en;
        unsigned char       trans_mode;


	//set VPU clk
        //WRITE_CBUS_REG(HHI_VPU_CLK_CNTL, 0x303); //vid pll clock, N = 0, enable //moved to vpu.c, default config by dts
	//WRITE_CBUS_REG(HHI_VPU_CLK_CNTL, 0x303);
        //WRITE_CBUS_REG_BITS(HHI_VPU_MEM_PD_REG1, 0, 22, 2);

        Lcd_Control_Config_t *pConf = &p->lcd_control;
        trans_mode  = pConf->mipi_config->trans_mode;
        venc_format = pConf->mipi_config->venc_fmt;
        mipi_dsi_venc_color_type= pConf->mipi_config->venc_color_type;
        mipi_dsi_dpi_color_type = pConf->mipi_config->dpi_color_type;
        lane_num                = pConf->mipi_config->lane_num;
        chroma_subsamp          = pConf->mipi_config->dpi_chroma_subsamp;

        DPRINT("%s, %d\n", __func__, __LINE__);
        print_info(lane_num,                        // Lane number
                        MIPI_DSI_VIRTUAL_CHAN_ID,                            // Virtual channel id
                        mipi_dsi_venc_color_type,                            // MIPI dsi venc color type
                        mipi_dsi_dpi_color_type,                             // MIPI dsi dpi color type
                        chroma_subsamp,                                      // Chroma sub sample for YUV 422 or 420, even or odd
                        trans_mode,                                          // DSI transfer mode, video or command
                        venc_format,                                         // Venc resolution format, eg, 240x160
                        MIPI_DSI_TRANS_VIDEO_MODE,                          //?????? // Video transfer mode, burst or non-burst
                        1);                                                   // If check the phy status, need check when first pic


        DPRINT("[TEST.C] Set mipi_dsi_host and mipi_dphy\n");
        set_mipi_dcs(1,                                  //0: high speed, 1: low power
                     0, //MIPI_DSI_DCS_ACK_TYPE,                                    // if need bta ack check
                     MIPI_DSI_TEAR_SWITCH                                      // enable tear ack
                     );

        DPRINT("%s, %d\n", __func__, __LINE__);
        set_mipi_dsi_host_to_command_mode(lane_num,                        // Lane number
                        MIPI_DSI_VIRTUAL_CHAN_ID,                            // Virtual channel id
                        mipi_dsi_venc_color_type,                            // MIPI dsi venc color type
                        mipi_dsi_dpi_color_type,                             // MIPI dsi dpi color type
                        chroma_subsamp,                                      // Chroma sub sample for YUV 422 or 420, even or odd
                        trans_mode,                                          // DSI transfer mode, video or command
                        venc_format,                                         // Venc resolution format, eg, 240x160
                        MIPI_DSI_TRANS_VIDEO_MODE,                          //?????? // Video transfer mode, burst or non-burst
                        1,                                                   // If check the phy status, need check when first pic
                        p);
}
void powerdown_mipi_analog(void)
{
    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x0);//DIF_REF_CTL0
    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x6 << 16);//DIF_REF_CTL2:31-16bit, DIF_REF_CTL1:15-0bit
    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x20<< 16);//DIF_TX_CTL1:31-16bit, DIF_TX_CTL0:15-0bit
}

void powerup_mipi_analog()
{

    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x8);//DIF_REF_CTL0
    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x3E << 16 |0xA5B8);//DIF_REF_CTL2:31-16bit, DIF_REF_CTL1:15-0bit
    WRITE_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3,  0x26E0 << 16 |0xFC59);//DIF_TX_CTL1:31-16bit, DIF_TX_CTL0:15-0bit
}

void init_phy_mipi(Lcd_Config_t *pConf)
{
    powerup_mipi_analog();
    DPRINT("%s, %d\n", __func__, __LINE__);
    mdelay( 10 );
    // Power up MIPI_DSI/DPHY, startup must be ahead of init
    powerup_mipi_dsi_dphy();

    init_mipi_dsi_phy(pConf);

    // Check the phylock/stopstateclklane to decide if the DPHY is ready
    check_phy_st();

    // Trigger a sync active for esc_clk
    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  READ_DSI_REG(MIPI_DSI_PHY_CTRL) | (1 << 1));
}

void set_tcon_mipi(Lcd_Config_t *p)
{

        unsigned int width;
        unsigned int height;
        int lcd_type;
        unsigned char lane_num;

        unsigned int        dith10_en, dith8_en, dith5_en;
        unsigned int        mipi_dsi_dpi_color_type;
        unsigned int        mipi_dsi_venc_color_type;
        unsigned int        is_rgb;
        unsigned int        data32;
        Lcd_Control_Config_t *pConf = &p->lcd_control;

        width                   = p->lcd_basic.h_active;
        height                  = p->lcd_basic.v_active;
        lcd_type                = p->lcd_basic.lcd_type;
        mipi_dsi_dpi_color_type = pConf->mipi_config->dpi_color_type;
        mipi_dsi_venc_color_type= pConf->mipi_config->venc_color_type;
        lane_num                = pConf->mipi_config->lane_num;


	Lcd_Timing_t *tcon_adr = &(p->lcd_timing);

	DPRINT("%s.\n", __FUNCTION__);

	WRITE_LCD_REG(L_RGB_BASE_ADDR,   p->lcd_effect.rgb_base_addr);
	WRITE_LCD_REG(L_RGB_COEFF_ADDR,  p->lcd_effect.rgb_coeff_addr);
        //============================================================

        is_rgb  = 1;    // RGB

        // Report error if VENC output width is configured to be < DPI data width
        //check_mipi_dsi_color_config (mipi_dsi_venc_color_type, mipi_dsi_dpi_color_type);

        switch (mipi_dsi_venc_color_type) {
                case MIPI_DSI_VENC_COLOR_24B    :
                        dith10_en = 1; dith8_en = 0; dith5_en = 0;
                        break;
                case MIPI_DSI_VENC_COLOR_18B    :
                        dith10_en = 1; dith8_en = 1; dith5_en = 0;
                        break;
                case MIPI_DSI_VENC_COLOR_16B    :
                        dith10_en = 1; dith8_en = 1; dith5_en = 1;
                        break;
                default :   // case MIPI_DSI_VENC_COLOR_30B :
                        dith10_en = 0; dith8_en = 0; dith5_en = 0;
                        break;
        }   /* switch (mipi_dsi_venc_color_type) */

        data32  = 0;
        //data32 |= (1        << 1);  // [1]  Set to 1 to bypass gain control ??????
        data32 |= (is_rgb   << 0);  // [0]  cfg_video_rgbin_zblk: 0=use blank data for YUV; 1=use blank data for RGB.
        WRITE_LCD_REG(ENCL_VIDEO_RGBIN_CTRL, data32);

        data32  = 0;
        data32 |= (dith5_en << 13); // [   13] dith_r5
        data32 |= (0        << 12); // [   12] dith_g5
        data32 |= (dith5_en << 11); // [   11] dith_b5
        data32 |= (dith10_en<< 10); // [   10] dith10_en
        data32 |= (dith8_en << 9);  // [    9] dith8_en
        data32 |= (0        << 8);  // [    8] dith_md: use default setting
        data32 |= (0        << 4);  // [ 7: 4] dith8_cntl: use default setting
        data32 |= (0        << 0);  // [ 3: 0] dith10_cntl: use default setting
        WRITE_LCD_REG(L_DITH_CNTL_ADDR, data32);

	WRITE_LCD_REG(VPP_MISC, READ_LCD_REG(VPP_MISC) & ~(VPP_OUT_SATURATE));
}

void lcd_ports_ctrl_mipi(Lcd_Config_t *p, Bool_t status)
{
        int vcid = MIPI_DSI_VIRTUAL_CHAN_ID;
        int req_ack = 0;
        unsigned int        mipi_dsi_dpi_color_type;
        unsigned int        mipi_dsi_venc_color_type;
        tv_enc_lcd_type_t   venc_format;
        unsigned char       lane_num;
        unsigned char       chroma_subsamp;

        unsigned int        data32;
        unsigned int        is_rgb;
        unsigned int        dith10_en, dith8_en, dith5_en;
        unsigned char       trans_mode;


        if(OFF == status){

                DPRINT("send display off\n");
                DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                                vcid,                                    // Virtual ID
                                DCS_SET_DISPLAY_OFF,                      // DCS Command Type
                                req_ack);                                // If need wait ack
                DPRINT("DCS_SET_DISPLAY_OFF Test Passed\n");
                mdelay(10);


                DPRINT("send exit sleep mode\n");
                DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                                vcid,                                    // Virtual ID
                                DCS_ENTER_SLEEP_MODE,                     // DCS Command Type
                                req_ack);                                // If need wait ack

                mdelay(20);

                DPRINT("poweroff dsi analog\n");

                powerdown_mipi_analog();

                DPRINT("poweroff dsi digital\n");

                powerdown_mipi_dsi_dphy();
                DPRINT("MIPI_DSI_PHY_CTRL=%x\n", READ_DSI_REG(MIPI_DSI_PHY_CTRL)); //read
                WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL, 0, 7, 1);

                return ;
        }

        Lcd_Control_Config_t *pConf = &p->lcd_control;
        trans_mode  = pConf->mipi_config->trans_mode;
        venc_format = pConf->mipi_config->venc_fmt;
        mipi_dsi_venc_color_type= pConf->mipi_config->venc_color_type;
        mipi_dsi_dpi_color_type = pConf->mipi_config->dpi_color_type;
        lane_num                = pConf->mipi_config->lane_num;
        chroma_subsamp          = pConf->mipi_config->dpi_chroma_subsamp;


        startup_transfer_video();

        DPRINT("send exit sleep mode\n");
        DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                        vcid,                                    // Virtual ID
                        DCS_EXIT_SLEEP_MODE,                     // DCS Command Type
                        req_ack);                                // If need wait ack

        mdelay(10);

        DPRINT("send display on\n");
        DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                        vcid,                                    // Virtual ID
                        DCS_SET_DISPLAY_ON,                      // DCS Command Type
                        req_ack);                                // If need wait ack
        DPRINT("DCS_SET_DISPLAY_ON Test Passed\n");
        mdelay(10);

        trans_mode = TRANS_VIDEO_MODE;
        set_mipi_dsi_host_to_video_mode(lane_num,                        // Lane number
                        MIPI_DSI_VIRTUAL_CHAN_ID,                            // Virtual channel id
                        mipi_dsi_venc_color_type,                            // MIPI dsi venc color type
                        mipi_dsi_dpi_color_type,                             // MIPI dsi dpi color type
                        chroma_subsamp,                                      // Chroma sub sample for YUV 422 or 420, even or odd
                        trans_mode,                                          // DSI transfer mode, video or command
                        venc_format,                                         // Venc resolution format, eg, 240x160
                        MIPI_DSI_TRANS_VIDEO_MODE,                          //?????? // Video transfer mode, burst or non-burst
                        1,                                                   // If check the phy status, need check when first pic
                        p);

}
#if 0
int do_dsi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, init_flag=0,dev, ret = 0;
	uint64_t addr;
	loff_t off=0, size=0;
	char *cmd, *s, *area;
	char	str[128];
        Lcd_Config_t *pConf = pDev->pConf;

        unsigned int data_type;
        unsigned int vc_id;
        unsigned int dcs_command;
        unsigned int req_ack;

	if (argc < 2){
	        DPRINT( "1.init\n"
                        "2.pll\n"
                        "3.ctrl\n"
                        "4.tcon\n"
                        "5.venc\n"
                        "6.phy\n"
                        " dsi cmdpkt dcs_command vc_id req_ack\n"
                        " dsi gen dcs_command vc_id\n"
                        "\nall\n");

		return 1;
        }

	cmd = argv[1];

	if (strcmp(cmd, "init") == 0){
                lcd_probe();
                DPRINT("init\n");
                DPRINT("MIPI_DSI_DWC_PWR_UP_OS=0x%x, P_MIPI_DSI_DWC_PWR_UP_OS=0x%x\n",
                                MIPI_DSI_DWC_PWR_UP_OS, P_MIPI_DSI_DWC_PWR_UP_OS);
        }else if(strcmp(cmd, "pll") == 0){
                set_pll_mipi(pConf); //1step

        }else if(strcmp(cmd, "ctrl") == 0){
                set_control_mipi(pConf);//2step

        }else if(strcmp(cmd, "tcon") == 0){
                set_tcon_mipi(pConf); //3step

        }else if(strcmp(cmd, "venc") == 0){
                set_venc_lcd(pConf); //4step

        }else if(strcmp(cmd, "phy") == 0){
		init_dphy(pConf);

        }else if(strcmp(cmd, "gen") == 0){
		if (argc < 3){
                        DPRINT(" para count =%d\n", argc);
                        DPRINT(" dsi gen dcs_command vc_id\n");
                }
		dcs_command = (ulong)simple_strtoul(argv[2], NULL, 16);
		vc_id = (ulong)simple_strtoul(argv[3], NULL, 16);
                data_type = DT_DCS_SHORT_WR_0;// DSI Data Type

                DPRINT("dcs_command=%x, vc_id=%d, data_type=DT_DCS_SHORT_WR_0\n", dcs_command, vc_id);

                generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((0x0 << BIT_GEN_WC_MSBYTE) |
                                (dcs_command << BIT_GEN_WC_LSBYTE) |
                                (vc_id << BIT_GEN_VC)              |
                                (data_type << BIT_GEN_DT)));

        }else if(strcmp(cmd, "ack") == 0){
                wait_bta_ack();

        }else if(strcmp(cmd, "noack") == 0){
                wait_cmd_fifo_empty();

        }else if(strcmp(cmd, "cmdpkt") == 0){
		if (argc < 3){
                        DPRINT(" para count =%d\n", argc);
                        DPRINT(" dsi cmdpkt dcs_command vc_id req_ack\n");
                }
		dcs_command = (ulong)simple_strtoul(argv[2], NULL, 16);
		vc_id = (ulong)simple_strtoul(argv[3], NULL, 16);
		req_ack = (ulong)simple_strtoul(argv[4], NULL, 16);

                DPRINT("dcs_command=%x, vc_id=%d req_ack=%d\n", dcs_command, vc_id, req_ack);

                DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                                  vc_id,                                    // Virtual ID
                                  dcs_command,                     // DCS Command Type
                                  req_ack);                                // If need wait ack

        }else if(strcmp(cmd, "poweron") == 0){
	        lcd_power_ctrl(ON);

        }else if(strcmp(cmd, "display") == 0){

                dcs_command = (ulong)simple_strtoul(argv[2], NULL, 16);
                vc_id = (ulong)simple_strtoul(argv[3], NULL, 16);
                req_ack = 0;
                DPRINT("STIMULUS_MIPI_DSI_DCS_PKT_TYPE | DCS_CMD_CODE_SET_DISPLAY_ON\n");
                DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0,                       // DSI Data Type
                                vc_id,                                    // Virtual ID
                                DCS_SET_DISPLAY_ON,                      // DCS Command Type
                                req_ack);                                // If need wait ack
                DPRINT("DCS_SET_DISPLAY_ON Test Passed\n");

        }else if(strcmp(cmd, "go") == 0){

            set_mipi_dsi_host_to_video_mode(3,                // Lane number
                                MIPI_DSI_VIRTUAL_CHAN_ID,     // Virtual channel id
                                2,                            // MIPI dsi venc color type
                                3,                             // MIPI dsi dpi color type
                                0,//chroma_subsamp,                                      // Chroma sub sample for YUV 422 or 420, even or odd
                                TRANS_VIDEO_MODE,               // DSI transfer mode, video or command
                                TV_ENC_LCD768x1024p,            // Venc resolution format, eg, 240x160
                                MIPI_DSI_TRANS_VIDEO_MODE,                          //?????? // Video transfer mode, burst or non-burst
                                1,                                                   // If check the phy status, need check when first pic
                                60);

        }else if(strcmp(cmd, "all") == 0){
		set_pll_mipi(pConf); //1step
                set_control_mipi(pConf);//2step
                set_tcon_mipi(pConf); //3step
                set_venc_lcd(pConf); //4step
                init_dphy(pConf);

        }else{
		DPRINT("cmd %s not support\n", cmd);
	}

        DPRINT("cmd=%s,exit\n", cmd);
        return 0;
}
U_BOOT_CMD(
		dsi, 7, 1, do_dsi,
		"enforce eth speed",
		"dsi init       - \n"
		"dsi host       - \n"
	  );
//****************************************
#endif
#else
void lcd_ports_ctrl_mipi(Lcd_Config_t *p, Bool_t status){}

void set_pll_mipi(Lcd_Config_t *p){}

void set_control_mipi(Lcd_Config_t *p){}

void set_venc_mipi(Lcd_Config_t *pConf){}

void set_tcon_mipi(Lcd_Config_t *p){}

void init_phy_mipi(Lcd_Config_t *pConf){}
#endif
