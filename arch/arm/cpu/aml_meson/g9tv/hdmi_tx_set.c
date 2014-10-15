#include <common.h>
#include <asm/io.h>
#include <asm/arch/register.h>
#include <asm/arch/reg_addr.h>
#include <amlogic/aml_tv.h>
#include <amlogic/hdmitx/hdmi.h>
#include "hdmi_tx_reg.h"
#include "hdmi_tx_tvenc.h"

void set_vmode_clk(vmode_t mode);
static void hdmi_tvenc_set(HDMI_Video_Codes_t vic);

#define HSYNC_POLARITY      1                       // HSYNC polarity: active high 
#define VSYNC_POLARITY      1                       // VSYNC polarity: active high

#define TX_COLOR_DEPTH          HDMI_COLOR_DEPTH_24B    // Pixel bit width: 4=24-bit; 5=30-bit; 6=36-bit; 7=48-bit.
#define TX_INPUT_COLOR_FORMAT   HDMI_COLOR_FORMAT_444   // Pixel format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
#define TX_OUTPUT_COLOR_FORMAT  HDMI_COLOR_FORMAT_444   // Pixel format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
#define TX_INPUT_COLOR_RANGE    HDMI_COLOR_RANGE_LIM    // Pixel range: 0=limited; 1=full.
#define TX_OUTPUT_COLOR_RANGE   HDMI_COLOR_RANGE_LIM    // Pixel range: 0=limited; 1=full.

// TODO  Delete later
#define CLK_UTIL_VID_PLL_DIV_1      0
#define CLK_UTIL_VID_PLL_DIV_2      1
#define CLK_UTIL_VID_PLL_DIV_3      2
#define CLK_UTIL_VID_PLL_DIV_3p5    3
#define CLK_UTIL_VID_PLL_DIV_3p75   4
#define CLK_UTIL_VID_PLL_DIV_4      5
#define CLK_UTIL_VID_PLL_DIV_5      6
#define CLK_UTIL_VID_PLL_DIV_6      7
#define CLK_UTIL_VID_PLL_DIV_6p25   8
#define CLK_UTIL_VID_PLL_DIV_7      9
#define CLK_UTIL_VID_PLL_DIV_7p5    10
#define CLK_UTIL_VID_PLL_DIV_12     11
#define CLK_UTIL_VID_PLL_DIV_14     12
#define CLK_UTIL_VID_PLL_DIV_15     13

#define hdmitx_print        printf

//TODO delete later
#define hdmitx_debug()      //hdmitx_print("hd: %s[%d]\n", __func__, __LINE__)

static int mode420 = 1;

static void hdelay(int us)
{
    int i;
    while(us--) {
        i = 10000;
        while(i--);
    }
}

#define mdelay(i)   hdelay(i)
#define msleep(i)   hdelay(i)

/*
 * HDMI reg read/write operation
 */
/**************************************  HDMI reg read/write operation start **************************************/

void aml_write_reg32_op(unsigned int _reg, const unsigned _value)
{
    __raw_writel(_value, _reg);
#ifdef HDMI_MINIBOOT_DEBUG
    hdmitx_print(" A:0x%08x 0x%04x  wD:0x%x ", _reg, (_reg & 0xffff)>>2, _value);
    if(_value == aml_read_reg32(_reg))
        hdmitx_print("\n");
    else
        hdmitx_print(" rD: 0x%08x\n", aml_read_reg32(_reg));
#endif
}

unsigned int aml_read_reg32_op(unsigned int _reg)
{
    return __raw_readl(_reg);
}

void aml_set_reg32_bits_op(uint32_t _reg, const uint32_t _val, const uint32_t _start, const uint32_t _len)
{
    unsigned int tmp;
    tmp = (__raw_readl(_reg) & ~(((1L<<(_len))-1)<<(_start))) | ((unsigned int)(_val) << (_start));
    aml_write_reg32_op(_reg, tmp);
}

static void C_Entry(HDMI_Video_Codes_t vic);

// Internal functions:
void hdmitx_csc_config (unsigned char input_color_format,
                        unsigned char output_color_format,
                        unsigned char color_depth);

static void dump_regs(void)
{
    unsigned int reg_adr;
    unsigned int reg_val;

    for(reg_adr = 0; reg_adr < 0x3000; reg_adr ++) {
        reg_val = aml_read_reg32(CBUS_REG_ADDR(reg_adr));
        if(reg_val) {
            printk("CBUS[0x%04x]=0x%08x\n", reg_adr, reg_val);
        }
    }
    for(reg_adr = 0; reg_adr < 0x3500; reg_adr ++) {
        reg_val = aml_read_reg32(VPU_REG_ADDR(reg_adr));
        if(reg_val) {
            printk("VCBUS[0x%04x]=0x%08x\n", reg_adr, reg_val);
        }
    }
    for(reg_adr = HDMITX_TOP_SW_RESET; reg_adr < HDMITX_TOP_STAT0 + 1; reg_adr ++) {
        reg_val = hdmitx_rd_reg(reg_adr);
        if(reg_val)
            printk("TOP[0x%x]: 0x%x\n", reg_adr, reg_val);
    }
    for (reg_adr = HDMITX_DWC_DESIGN_ID; reg_adr < HDMITX_DWC_I2CM_SCDC_UPDATE1 + 1; reg_adr ++) {
        if((reg_adr > HDMITX_DWC_HDCP_BSTATUS_0 -1) && (reg_adr < HDMITX_DWC_HDCPREG_BKSV0)) {
            //hdmitx_wr_reg(HDMITX_DWC_A_KSVMEMCTRL, 0x1);
//            hdmitx_poll_reg(HDMITX_DWC_A_KSVMEMCTRL, (1<<1), 2 * HZ);
            reg_val = 0;//hdmitx_rd_reg(reg_adr);
        }
        else {
            reg_val = hdmitx_rd_reg(reg_adr);
        }
        if(reg_val) {
            // excluse HDCP regisiters
            if((reg_adr < HDMITX_DWC_A_HDCPCFG0) || (reg_adr > HDMITX_DWC_CEC_CTRL))
                printk("DWC[0x%x]: 0x%x\n", reg_adr, reg_val);
        }
    }
}

void hdmi_tx_set(HDMI_Video_Codes_t vic, int pmode420, int pcolord)
{
    mode420 = pmode420;
    hdmitx_debug();
    C_Entry(vic);
    hdmitx_debug();
    return;
    dump_regs();

#if 0
    hdmi_tx_gate(vic);
    hdmi_tx_clk(vic);
    hdmi_tx_misc(vic);
    hdmi_tx_enc(vic);
    hdmi_tx_set_vend_spec_infofram(vic);
    hdmi_tx_phy(vic);
#endif
}

void hdmitx_wr_reg (unsigned long addr, unsigned long data)
{
    unsigned long offset = (addr >> 24);
    addr = addr & 0xffff;
    aml_write_reg32_op(HDMITX_ADDR_PORT + offset, addr);
    aml_write_reg32_op(HDMITX_ADDR_PORT + offset, addr);
    aml_write_reg32_op(HDMITX_DATA_PORT + offset, data);
    aml_write_reg32_op(HDMITX_DATA_PORT + offset, data);
} /* hdmitx_wr_reg */

unsigned long hdmitx_rd_reg (unsigned long addr)
{
    unsigned long offset = (addr >> 24);
    addr = addr & 0xffff;
    aml_write_reg32_op(HDMITX_ADDR_PORT + offset, addr);
    aml_write_reg32_op(HDMITX_ADDR_PORT + offset, addr);

    return aml_read_reg32_op(HDMITX_DATA_PORT + offset);
} /* hdmitx_rd_reg */

void hdmitx_set_reg_bits(unsigned int addr, unsigned int value, unsigned int offset, unsigned int len)
{
    unsigned int data32 = 0;

    data32 = hdmitx_rd_reg(addr);
    data32 &= ~(((1 << len) - 1) << offset);
    data32 |= (value & ((1 << len) - 1)) << offset;
    hdmitx_wr_reg(addr, data32);
}

void hdmitx_rd_check_reg (unsigned long addr, unsigned long exp_data, unsigned long mask)
{
    unsigned long rd_data;
    rd_data = hdmitx_rd_reg(addr);
    if ((rd_data | mask) != (exp_data | mask)) {
        hdmitx_print("HDMITX-DWC addr=0x%04x rd_data=0x%02x\n", (unsigned int)addr, (unsigned int)rd_data);
        hdmitx_print("Error: HDMITX-DWC exp_data=0x%02x mask=0x%02x\n", (unsigned int)exp_data, (unsigned int)mask);
    }
}

#define VFIFO2VD_TO_HDMI_LATENCY    2   // Latency in pixel clock from ENCP_VFIFO2VD request to data ready to HDMI
#define NUM_INT_VSYNC   INT_VEC_VIU1_VSYNC

unsigned long modulo(unsigned long a, unsigned long b);
signed int to_signed(unsigned int a);

void config_hdmi20_tx ( HDMI_Video_Codes_t vic, struct hdmi_format_para *para,
                        unsigned char   color_depth,            // Pixel bit width: 4=24-bit; 5=30-bit; 6=36-bit; 7=48-bit.
                        unsigned char   input_color_format,     // Pixel format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
                        unsigned char   input_color_range,      // Pixel range: 0=limited; 1=full.
                        unsigned char   output_color_format,    // Pixel format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
                        unsigned char   output_color_range     // Pixel range: 0=limited; 1=full.
                    )          // 0:TMDS_CLK_rate=TMDS_Character_rate; 1:TMDS_CLK_rate=TMDS_Character_rate/4, for TMDS_Character_rate>340Mcsc.
{
    struct hdmi_cea_timing *t = &para->timing;
    unsigned long   data32;
    unsigned char   vid_map;
    unsigned char   csc_en;
    unsigned char   default_phase = 0;

#define GET_TIMING(name)      (t->name)

    //--------------------------------------------------------------------------
    // Enable clocks and bring out of reset
    //--------------------------------------------------------------------------
    
    // Enable hdmitx_sys_clk
    //         .clk0               ( cts_oscin_clk         ),
    //         .clk1               ( fclk_div4             ),
    //         .clk2               ( fclk_div3             ),
    //         .clk3               ( fclk_div5             ),
    aml_set_reg32_bits(P_HHI_HDMI_CLK_CNTL, 0x100, 0, 16);   // [10: 9] clk_sel. select cts_oscin_clk=24MHz
                                                                // [    8] clk_en. Enable gated clock
                                                                // [ 6: 0] clk_div. Divide by 1. = 24/1 = 24 MHz
    
    aml_set_reg32_bits(P_HHI_GCLK_MPEG2, 1, 4, 1);       // Enable clk81_hdmitx_pclk
    // wire            wr_enable           = control[3];
    // wire            fifo_enable         = control[2];    
    // assign          phy_clk_en          = control[1];
    aml_set_reg32_bits(P_HHI_HDMI_PHY_CNTL1, 1, 1, 1);   // Enable tmds_clk
    aml_set_reg32_bits(P_HHI_HDMI_PHY_CNTL1, 1, 2, 1);   // Enable the decoupling FIFO
    aml_set_reg32_bits(P_HHI_HDMI_PHY_CNTL1, 1, 3, 1);   // Enable enable the write/read decoupling state machine 
    aml_set_reg32_bits(P_HHI_MEM_PD_REG0, 0, 8, 8);      // Bring HDMITX MEM output of power down

    // Enable APB3 fail on error
    aml_set_reg32_bits(P_HDMITX_CTRL_PORT, 1, 15, 1);
    aml_set_reg32_bits((P_HDMITX_CTRL_PORT + 0x10), 1, 15, 1);

    // Bring out of reset
    hdmitx_wr_reg(HDMITX_TOP_SW_RESET,  0);

    // Enable internal pixclk, tmds_clk, spdif_clk, i2s_clk, cecclk
    hdmitx_wr_reg(HDMITX_TOP_CLK_CNTL,  0x0000001f);
    hdmitx_wr_reg(HDMITX_DWC_MC_LOCKONCLOCK,   0xff);

    // But keep spdif_clk and i2s_clk disable until later enable by test.c
    data32  = 0;
    data32 |= (0    << 6);  // [  6] hdcpclk_disable
    data32 |= (0    << 5);  // [  5] cecclk_disable
    data32 |= (0    << 4);  // [  4] cscclk_disable
    data32 |= (0    << 3);  // [  3] audclk_disable
    data32 |= (0    << 2);  // [  2] prepclk_disable
    data32 |= (0    << 1);  // [  1] tmdsclk_disable
    data32 |= (0    << 0);  // [  0] pixelclk_disable
    hdmitx_wr_reg(HDMITX_DWC_MC_CLKDIS, data32);

    // Enable normal output to PHY

    switch(vic) {
    case HDMI_3840x2160p50_16x9:
    case HDMI_3840x2160p60_16x9:
        para->tmds_clk_div40 = 1;
        break;
    default:
        break;
    }

    data32  = 0;
    data32 |= (1    << 12); // [14:12] tmds_sel: 0=output 0; 1=output normal data; 2=output PRBS; 4=output shift pattern.
    data32 |= (0    << 8);  // [11: 8] shift_pttn
    data32 |= (0    << 0);  // [ 4: 0] prbs_pttn
    hdmitx_wr_reg(HDMITX_TOP_BIST_CNTL, data32);                        // 0x6

    //--------------------------------------------------------------------------
    // Configure video
    //--------------------------------------------------------------------------
    
    if (((input_color_format == HDMI_COLOR_FORMAT_420) || (output_color_format == HDMI_COLOR_FORMAT_420)) &&
        ((input_color_format != output_color_format) || (input_color_range != output_color_range))) {
        printk("Error: HDMITX input/output color combination not supported!\n");
    }

    // Configure video sampler

    vid_map = ( input_color_format == HDMI_COLOR_FORMAT_RGB )?  ((color_depth == HDMI_COLOR_DEPTH_24B)? 0x01    :
                                                                 (color_depth == HDMI_COLOR_DEPTH_30B)? 0x03    :
                                                                 (color_depth == HDMI_COLOR_DEPTH_36B)? 0x05    :
                                                                                                        0x07)   :
              ((input_color_format == HDMI_COLOR_FORMAT_444) ||
               (input_color_format == HDMI_COLOR_FORMAT_420))?  ((color_depth == HDMI_COLOR_DEPTH_24B)? 0x09    :
                                                                 (color_depth == HDMI_COLOR_DEPTH_30B)? 0x0b    :
                                                                 (color_depth == HDMI_COLOR_DEPTH_36B)? 0x0d    :
                                                                                                        0x0f)   :
                                                                ((color_depth == HDMI_COLOR_DEPTH_24B)? 0x16    :
                                                                 (color_depth == HDMI_COLOR_DEPTH_30B)? 0x14    :
                                                                                                        0x12);
    
    data32  = 0;
    data32 |= (0        << 7);  // [  7] internal_de_generator
    data32 |= (vid_map  << 0);  // [4:0] video_mapping
    hdmitx_wr_reg(HDMITX_DWC_TX_INVID0, data32);
    
    data32  = 0;
    data32 |= (0        << 2);  // [  2] bcbdata_stuffing
    data32 |= (0        << 1);  // [  1] rcrdata_stuffing
    data32 |= (0        << 0);  // [  0] gydata_stuffing
    hdmitx_wr_reg(HDMITX_DWC_TX_INSTUFFING, data32);
    hdmitx_wr_reg(HDMITX_DWC_TX_GYDATA0,    0x00);
    hdmitx_wr_reg(HDMITX_DWC_TX_GYDATA1,    0x00);
    hdmitx_wr_reg(HDMITX_DWC_TX_RCRDATA0,   0x00);
    hdmitx_wr_reg(HDMITX_DWC_TX_RCRDATA1,   0x00);
    hdmitx_wr_reg(HDMITX_DWC_TX_BCBDATA0,   0x00);
    hdmitx_wr_reg(HDMITX_DWC_TX_BCBDATA1,   0x00);

    // Configure Color Space Converter
    
    csc_en  = ((input_color_format != output_color_format) ||
               (input_color_range  != output_color_range))? 1 : 0;
    
    data32  = 0;
    data32 |= (csc_en   << 0);  // [  0] CSC enable
    hdmitx_wr_reg(HDMITX_DWC_MC_FLOWCTRL,   data32);

    data32  = 0;
    data32 |= ((((input_color_format ==HDMI_COLOR_FORMAT_422) &&
                 (output_color_format!=HDMI_COLOR_FORMAT_422))? 2 : 0 ) << 4);  // [5:4] intmode
    data32 |= ((((input_color_format !=HDMI_COLOR_FORMAT_422) &&
                 (output_color_format==HDMI_COLOR_FORMAT_422))? 2 : 0 ) << 0);  // [1:0] decmode
    hdmitx_wr_reg(HDMITX_DWC_CSC_CFG,       data32);

    hdmitx_csc_config(input_color_format, output_color_format, color_depth);
    
    // Configure video packetizer

    // Video Packet color depth and pixel repetition
    data32  = 0;
    data32 |= (((output_color_format==HDMI_COLOR_FORMAT_422)? HDMI_COLOR_DEPTH_24B : color_depth)   << 4);  // [7:4] color_depth
    data32 |= (0                                                                                    << 0);  // [3:0] desired_pr_factor
    hdmitx_wr_reg(HDMITX_DWC_VP_PR_CD,  data32);

    // Video Packet Stuffing
    data32  = 0;
    data32 |= (default_phase    << 5);  // [  5] default_phase
    data32 |= (0                << 2);  // [  2] ycc422_stuffing
    data32 |= (0                << 1);  // [  1] pp_stuffing
    data32 |= (0                << 0);  // [  0] pr_stuffing
    hdmitx_wr_reg(HDMITX_DWC_VP_STUFF,  data32);

    // Video Packet YCC color remapping
    data32  = 0;
    data32 |= (((color_depth == HDMI_COLOR_DEPTH_30B)? 1 :
                (color_depth == HDMI_COLOR_DEPTH_36B)? 2 : 0)   << 0);  // [1:0] ycc422_size
    hdmitx_wr_reg(HDMITX_DWC_VP_REMAP,  data32);

    // Video Packet configuration
    data32  = 0;
    data32 |= ((((output_color_format != HDMI_COLOR_FORMAT_422) &&
                 (color_depth         == HDMI_COLOR_DEPTH_24B))? 1 : 0) << 6);  // [  6] bypass_en
    data32 |= ((((output_color_format == HDMI_COLOR_FORMAT_422) ||
                 (color_depth         == HDMI_COLOR_DEPTH_24B))? 0 : 1) << 5);  // [  5] pp_en
    data32 |= (0                                                        << 4);  // [  4] pr_en
    data32 |= (((output_color_format == HDMI_COLOR_FORMAT_422)?  1 : 0) << 3);  // [  3] ycc422_en
    data32 |= (1                                                        << 2);  // [  2] pr_bypass_select
    data32 |= (((output_color_format == HDMI_COLOR_FORMAT_422)? 1 :
                (color_depth         == HDMI_COLOR_DEPTH_24B)?  2 : 0)  << 0);  // [1:0] output_selector: 0=pixel packing; 1=YCC422 remap; 2/3=8-bit bypass
    hdmitx_wr_reg(HDMITX_DWC_VP_CONF,   data32);

    data32  = 0;
    data32 |= (1    << 7);  // [  7] mask_int_full_prpt
    data32 |= (1    << 6);  // [  6] mask_int_empty_prpt
    data32 |= (1    << 5);  // [  5] mask_int_full_ppack
    data32 |= (1    << 4);  // [  4] mask_int_empty_ppack
    data32 |= (1    << 3);  // [  3] mask_int_full_remap
    data32 |= (1    << 2);  // [  2] mask_int_empty_remap
    data32 |= (1    << 1);  // [  1] mask_int_full_byp
    data32 |= (1    << 0);  // [  0] mask_int_empty_byp
    hdmitx_wr_reg(HDMITX_DWC_VP_MASK,   data32);

    //--------------------------------------------------------------------------
    // Configure audio
    //--------------------------------------------------------------------------

    //I2S Sampler config

    data32  = 0;
    data32 |= (1    << 3);  // [  3] fifo_empty_mask: 0=enable int; 1=mask int.
    data32 |= (1    << 2);  // [  2] fifo_full_mask: 0=enable int; 1=mask int.
    hdmitx_wr_reg(HDMITX_DWC_AUD_INT,   data32);

    data32  = 0;
    data32 |= (1    << 4);  // [  4] fifo_overrun_mask: 0=enable int; 1=mask int. Enable it later when audio starts.
    hdmitx_wr_reg(HDMITX_DWC_AUD_INT1,  data32);

    data32  = 0;
    data32 |= (0    << 5);  // [7:5] i2s_mode: 0=standard I2S mode
    data32 |= (24   << 0);  // [4:0] i2s_width
    hdmitx_wr_reg(HDMITX_DWC_AUD_CONF1, data32);

    //spdif sampler config

    data32  = 0;
    data32 |= (1    << 3);  // [  3] SPDIF fifo_empty_mask: 0=enable int; 1=mask int.
    data32 |= (1    << 2);  // [  2] SPDIF fifo_full_mask: 0=enable int; 1=mask int.
    hdmitx_wr_reg(HDMITX_DWC_AUD_SPDIFINT,  data32);

    data32  = 0;
    data32 |= (0    << 4);  // [  4] SPDIF fifo_overrun_mask: 0=enable int; 1=mask int.
    hdmitx_wr_reg(HDMITX_DWC_AUD_SPDIFINT1, data32);

    data32  = 0;
    data32 |= (0    << 7);  // [  7] sw_audio_fifo_rst
    hdmitx_wr_reg(HDMITX_DWC_AUD_SPDIF0,    data32);

    //--------------------------------------------------------------------------
    // Frame Composer configuration
    //--------------------------------------------------------------------------

    // Video definitions, as per output video (for packet gen/schedulling)

    data32  = 0;
//    data32 |= (((hdcp_on|scrambler_en)?1:0) << 7);  // [  7] HDCP_keepout
    data32 |= (1                            << 7);  // [  7] HDCP_keepout
    data32 |= (GET_TIMING(vsync_polarity)     << 6);  // [  6] vs_in_pol: 0=active low; 1=active high.
    data32 |= (GET_TIMING(hsync_polarity)     << 5);  // [  5] hs_in_pol: 0=active low; 1=active high.
    data32 |= (1                            << 4);  // [  4] de_in_pol: 0=active low; 1=active high.
    data32 |= (1                            << 3);  // [  3] dvi_modez: 0=dvi; 1=hdmi.
    data32 |= (!(para->progress_mode)         << 1);  // [  1] r_v_blank_in_osc
    data32 |= (!(para->progress_mode)         << 0);  // [  0] in_I_P: 0=progressive; 1=interlaced.
    hdmitx_wr_reg(HDMITX_DWC_FC_INVIDCONF,  data32);

    data32  = GET_TIMING(h_active)&0xff;       // [7:0] H_in_active[7:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_INHACTV0,   data32);
    data32  = (GET_TIMING(h_active)>>8)&0x3f;  // [5:0] H_in_active[13:8]
    hdmitx_wr_reg(HDMITX_DWC_FC_INHACTV1,   data32);

    data32  = GET_TIMING(h_blank)&0xff;        // [7:0] H_in_blank[7:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_INHBLANK0,  data32);
    data32  = (GET_TIMING(h_blank)>>8)&0x1f;   // [4:0] H_in_blank[12:8]
    hdmitx_wr_reg(HDMITX_DWC_FC_INHBLANK1,  data32);

    data32  = GET_TIMING(v_active)&0xff;        // [7:0] V_in_active[7:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_INVACTV0,   data32);
    data32  = (GET_TIMING(v_active)>>8)&0x1f;   // [4:0] V_in_active[12:8]
    hdmitx_wr_reg(HDMITX_DWC_FC_INVACTV1,   data32);

    data32  = GET_TIMING(v_blank)&0xff;         // [7:0] V_in_blank
    hdmitx_wr_reg(HDMITX_DWC_FC_INVBLANK,   data32);

    data32  = GET_TIMING(h_front)&0xff;         // [7:0] H_in_delay[7:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINDELAY0,  data32);
    data32  = (GET_TIMING(h_front)>>8)&0x1f;    // [4:0] H_in_delay[12:8]
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINDELAY1,  data32);

    data32  = GET_TIMING(h_sync)&0xff;        // [7:0] H_in_width[7:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINWIDTH0,  data32);
    data32  = (GET_TIMING(h_sync)>>8)&0x3;    // [1:0] H_in_width[9:8]
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINWIDTH1,  data32);

    data32  = GET_TIMING(v_front)&0xff;           // [7:0] V_in_delay
    hdmitx_wr_reg(HDMITX_DWC_FC_VSYNCINDELAY,   data32);

    data32  = GET_TIMING(v_sync)&0x3f;         // [5:0] V_in_width
    hdmitx_wr_reg(HDMITX_DWC_FC_VSYNCINWIDTH,   data32);

    //control period duration (typ 12 tmds periods)
    hdmitx_wr_reg(HDMITX_DWC_FC_CTRLDUR,    12);
    //extended control period duration (typ 32 tmds periods)
    hdmitx_wr_reg(HDMITX_DWC_FC_EXCTRLDUR,  32);
    //max interval betwen extended control period duration (typ 50)
    hdmitx_wr_reg(HDMITX_DWC_FC_EXCTRLSPAC, 1);     // ??
    //preamble filler
    hdmitx_wr_reg(HDMITX_DWC_FC_CH0PREAM,   0x0b);
    hdmitx_wr_reg(HDMITX_DWC_FC_CH1PREAM,   0x16);
    hdmitx_wr_reg(HDMITX_DWC_FC_CH2PREAM,   0x21);

    //write GCP packet configuration
    data32  = 0;
    data32 |= (default_phase    << 2);  // [  2] default_phase
    data32 |= (0                << 1);  // [  1] set_avmute
    data32 |= (0                << 0);  // [  0] clear_avmute
    hdmitx_wr_reg(HDMITX_DWC_FC_GCP,    data32);

    //write AVI Infoframe packet configuration
    
    data32  = 0;
    data32 |= (((output_color_format>>2)&0x1)   << 7);  // [  7] rgb_ycc_indication[2]
    data32 |= (1                                << 6);  // [  6] active_format_present
    data32 |= (0                                << 4);  // [5:4] scan_information
    data32 |= (0                                << 2);  // [3:2] bar_information
    data32 |= (0x2                              << 0);  // [1:0] rgb_ycc_indication[1:0]
    hdmitx_wr_reg(HDMITX_DWC_FC_AVICONF0,   data32);

    data32  = 0;
    data32 |= (0    << 6);  // [7:6] colorimetry
    data32 |= (0    << 4);  // [5:4] picture_aspect_ratio
    data32 |= (8    << 0);  // [3:0] active_aspect_ratio
    hdmitx_wr_reg(HDMITX_DWC_FC_AVICONF1,   data32);

    data32  = 0;
    data32 |= (0    << 7);  // [  7] IT_content
    data32 |= (0    << 4);  // [6:4] extended_colorimetry
    data32 |= (0    << 2);  // [3:2] quantization_range
    data32 |= (0    << 0);  // [1:0] non_uniform_picture_scaling
    hdmitx_wr_reg(HDMITX_DWC_FC_AVICONF2,   data32);

    data32  = 0;
    data32 |= (((output_color_range == HDMI_COLOR_RANGE_FUL)?1:0)   << 2);  // [3:2] YQ
    data32 |= (0                                                    << 0);  // [1:0] CN
    hdmitx_wr_reg(HDMITX_DWC_FC_AVICONF3,   data32);

    hdmitx_wr_reg(HDMITX_DWC_FC_AVIVID, para->vic);

    // the audio setting bellow are only used for I2S audio IEC60958-3 frame insertion

    //packet queue priority (auto mode)
    hdmitx_wr_reg(HDMITX_DWC_FC_CTRLQHIGH,  15);
    hdmitx_wr_reg(HDMITX_DWC_FC_CTRLQLOW,   3);

    //packet scheduller configuration for SPD, VSD, ISRC1/2, ACP.
    data32  = 0;
    data32 |= (0    << 4);  // [  4] spd_auto
    data32 |= (0    << 3);  // [  3] vsd_auto
    data32 |= (0    << 2);  // [  2] isrc2_auto
    data32 |= (0    << 1);  // [  1] isrc1_auto
    data32 |= (0    << 0);  // [  0] acp_auto
    hdmitx_wr_reg(HDMITX_DWC_FC_DATAUTO0,   data32);
    hdmitx_wr_reg(HDMITX_DWC_FC_DATAUTO1,   0);
    hdmitx_wr_reg(HDMITX_DWC_FC_DATAUTO2,   0);
    hdmitx_wr_reg(HDMITX_DWC_FC_DATMAN,     0);

    //packet scheduller configuration for AVI, GCP, AUDI, ACR.
    data32  = 0;
    data32 |= (1    << 3);  // [  3] avi_auto: insert on Vsync
    data32 |= (1    << 2);  // [  2] gcp_auto: insert on Vsync
    data32 |= (1    << 1);  // [  1] audi_auto: insert on Vsync
    data32 |= (0    << 0);  // [  0] acr_auto: insert on CTS update. Assert this bit later to avoid inital packets with false CTS value
    hdmitx_wr_reg(HDMITX_DWC_FC_DATAUTO3,   data32);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB0,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB1,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB2,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB3,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB4,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB5,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB6,      0);
    hdmitx_wr_reg(HDMITX_DWC_FC_RDRB7,      0);

    // Do not enable these interrupt below, we can check them at RX side.

    data32  = 0;
    data32 |= (1    << 7);  // [  7] AUDI_int_mask
    data32 |= (1    << 6);  // [  6] ACP_int_mask
    data32 |= (1    << 5);  // [  5] HBR_int_mask
    data32 |= (1    << 2);  // [  2] AUDS_int_mask
    data32 |= (1    << 1);  // [  1] ACR_int_mask
    data32 |= (1    << 0);  // [  0] NULL_int_mask
    hdmitx_wr_reg(HDMITX_DWC_FC_MASK0,      data32);

    data32  = 0;
    data32 |= (1    << 7);  // [  7] GMD_int_mask
    data32 |= (1    << 6);  // [  6] ISRC1_int_mask
    data32 |= (1    << 5);  // [  5] ISRC2_int_mask
    data32 |= (1    << 4);  // [  4] VSD_int_mask
    data32 |= (1    << 3);  // [  3] SPD_int_mask
    data32 |= (1    << 1);  // [  1] AVI_int_mask
    data32 |= (1    << 0);  // [  0] GCP_int_mask
    hdmitx_wr_reg(HDMITX_DWC_FC_MASK1,      data32);

    data32  = 0;
    data32 |= (1    << 1);  // [  1] LowPriority_fifo_full
    data32 |= (1    << 0);  // [  0] HighPriority_fifo_full
    hdmitx_wr_reg(HDMITX_DWC_FC_MASK2,      data32);

    // Pixel repetition ratio the input and output video
    data32  = 0;
    data32 |= ((para->pixel_repetition_factor+1) << 4);  // [7:4] incoming_pr_factor
    data32 |= (para->pixel_repetition_factor     << 0);  // [3:0] output_pr_factor
    hdmitx_wr_reg(HDMITX_DWC_FC_PRCONF, data32);

    // Scrambler control
    data32  = 0;
    data32 |= (0            << 4);  // [  4] scrambler_ucp_line
    data32 |= (para->scrambler_en << 0);  // [  0] scrambler_en. Only update this bit once we've sent SCDC message, in test.c
    hdmitx_wr_reg(HDMITX_DWC_FC_SCRAMBLER_CTRL, data32);

    //--------------------------------------------------------------------------
    // Configure HDCP
    //--------------------------------------------------------------------------

    data32  = 0;
    data32 |= (0    << 7);  // [  7] hdcp_engaged_int_mask
    data32 |= (0    << 6);  // [  6] hdcp_failed_int_mask
    data32 |= (0    << 4);  // [  4] i2c_nack_int_mask
    data32 |= (0    << 3);  // [  3] lost_arbitration_int_mask
    data32 |= (0    << 2);  // [  2] keepout_error_int_mask
    data32 |= (0    << 1);  // [  1] ksv_sha1_calc_int_mask
    data32 |= (1    << 0);  // [  0] ksv_access_int_mask
    hdmitx_wr_reg(HDMITX_DWC_A_APIINTMSK,   data32);

    data32  = 0;
    data32 |= (0    << 5);  // [6:5] unencryptconf
    data32 |= (1    << 4);  // [  4] dataenpol
    data32 |= (1    << 3);  // [  3] vsyncpol
    data32 |= (1    << 1);  // [  1] hsyncpol
    hdmitx_wr_reg(HDMITX_DWC_A_VIDPOLCFG,   data32);

    hdmitx_wr_reg(HDMITX_DWC_A_OESSWCFG,    0x40);

    data32  = 0;
    data32 |= (0                << 4);  // [  4] hdcp_lock
    data32 |= (0                << 3);  // [  3] dissha1check
    data32 |= (1                << 2);  // [  2] ph2upshiftenc
    data32 |= (1                << 1);  // [  1] encryptiondisable
    data32 |= (1                << 0);  // [  0] swresetn. Write 0 to activate, self-clear to 1.
    hdmitx_wr_reg(HDMITX_DWC_A_HDCPCFG1,    data32);

//    configure_hdcp_dpk(base_offset, 0xa938);

    //initialize HDCP, with rxdetect low
    data32  = 0;
    data32 |= (0                << 7);  // [  7] ELV_ena
    data32 |= (1                << 6);  // [  6] i2c_fastmode
    data32 |= (1                << 5);  // [  5] byp_encryption
    data32 |= (1                << 4);  // [  4] sync_ri_check
    data32 |= (0                << 3);  // [  3] avmute
    data32 |= (0                << 2);  // [  2] rxdetect
    data32 |= (1                << 1);  // [  1] en11_feature
    data32 |= (1                << 0);  // [  0] hdmi_dvi
    hdmitx_wr_reg(HDMITX_DWC_A_HDCPCFG0,    data32);

    //--------------------------------------------------------------------------
    // Interrupts
    //--------------------------------------------------------------------------

    // Clear interrupts
    hdmitx_wr_reg(HDMITX_DWC_IH_FC_STAT0,      0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_FC_STAT1,      0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_FC_STAT2,      0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_AS_STAT0,      0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_PHY_STAT0,     0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_I2CM_STAT0,    0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_CEC_STAT0,     0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_VP_STAT0,      0xff);
    hdmitx_wr_reg(HDMITX_DWC_IH_I2CMPHY_STAT0, 0xff);
    hdmitx_wr_reg(HDMITX_DWC_A_APIINTCLR,      0xff);
    // [2]      hpd_fall
    // [1]      hpd_rise
    // [0]      core_intr_rise
    hdmitx_wr_reg(HDMITX_TOP_INTR_STAT_CLR,    0x00000007);

    // Selectively enable/mute interrupt sources
    
    data32  = 0;
    data32 |= (1    << 7);  // [  7] mute_AUDI
    data32 |= (1    << 6);  // [  6] mute_ACP
    data32 |= (1    << 4);  // [  4] mute_DST
    data32 |= (1    << 3);  // [  3] mute_OBA
    data32 |= (1    << 2);  // [  2] mute_AUDS
    data32 |= (1    << 1);  // [  1] mute_ACR
    data32 |= (1    << 0);  // [  0] mute_NULL
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_FC_STAT0,  data32);

    data32  = 0;
    data32 |= (1    << 7);  // [  7] mute_GMD
    data32 |= (1    << 6);  // [  6] mute_ISRC1
    data32 |= (1    << 5);  // [  5] mute_ISRC2
    data32 |= (1    << 4);  // [  4] mute_VSD
    data32 |= (1    << 3);  // [  3] mute_SPD
    data32 |= (1    << 1);  // [  1] mute_AVI
    data32 |= (1    << 0);  // [  0] mute_GCP
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_FC_STAT1,  data32);

    data32  = 0;
    data32 |= (1    << 1);  // [  1] mute_LowPriority_fifo_full
    data32 |= (1    << 0);  // [  0] mute_HighPriority_fifo_full
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_FC_STAT2,  data32);

    data32  = 0;
    data32 |= (0    << 3);  // [  3] mute_aud_fifo_overrun
    data32 |= (1    << 2);  // [  2] mute_aud_fifo_underflow_thr. aud_fifo_underflow tied to 0.
    data32 |= (1    << 1);  // [  1] mute_aud_fifo_empty
    data32 |= (1    << 0);  // [  0] mute_aud_fifo_full
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_AS_STAT0,  data32);

    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_PHY_STAT0, 0x3f);

    data32  = 0;
    data32 |= (0    << 2);  // [  2] mute_scdc_readreq
    data32 |= (1    << 1);  // [  1] mute_edid_i2c_master_done
    data32 |= (0    << 0);  // [  0] mute_edid_i2c_master_error
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_I2CM_STAT0,data32);

    data32  = 0;
    data32 |= (0    << 6);  // [  6] cec_wakeup
    data32 |= (0    << 5);  // [  5] cec_error_follower
    data32 |= (0    << 4);  // [  4] cec_error_initiator
    data32 |= (0    << 3);  // [  3] cec_arb_lost
    data32 |= (0    << 2);  // [  2] cec_nack
    data32 |= (0    << 1);  // [  1] cec_eom
    data32 |= (0    << 0);  // [  0] cec_done
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_CEC_STAT0, data32);

    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_VP_STAT0,      0xff);

    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE_I2CMPHY_STAT0, 0x03);

    data32  = 0;
    data32 |= (0    << 1);  // [  1] mute_wakeup_interrupt
    data32 |= (0    << 0);  // [  0] mute_all_interrupt
    hdmitx_wr_reg(HDMITX_DWC_IH_MUTE,   data32);

    data32  = 0;
    data32 |= (1    << 2);  // [  2] hpd_fall_intr
    data32 |= (1    << 1);  // [  1] hpd_rise_intr
    data32 |= (1    << 0);  // [  0] core_intr
    hdmitx_wr_reg(HDMITX_TOP_INTR_MASKN,data32);

    //--------------------------------------------------------------------------
    // Reset pulse
    //--------------------------------------------------------------------------

    hdmitx_rd_check_reg(HDMITX_DWC_MC_LOCKONCLOCK, 0xff, 0x9f);
    hdmitx_wr_reg(HDMITX_DWC_MC_SWRSTZREQ, 0);
} /* config_hdmi20_tx */

// --------------------------------------------------------
//                     C_Entry
// --------------------------------------------------------

void hdmi_tx_enc(HDMI_Video_Codes_t vic)
{
    struct hdmi_format_para * para = NULL;
    struct hdmi_cea_timing * t = NULL;
    para = hdmi_get_fmt_paras(vic);
    if(para == NULL) {
        printk("error at %s[%d]\n", __func__, __LINE__);
        return;
    }
    printk("%s[%d] set VIC = %d\n", __func__, __LINE__, para->vic);
    t = &para->timing;

    // --------------------------------------------------------
    // Set TV encoder for HDMI
    // --------------------------------------------------------
    printk("Configure VENC\n");

    switch (vic) {
        case HDMI_1920x1080p60_16x9:
            config_tv_enc(TVMODE_1080P);
            break;
        case HDMI_1920x1080p50_16x9:
            config_tv_enc(TVMODE_1080P_50HZ);
            break;
        case HDMI_3840x2160p50_16x9:
            config_tv_enc(TVMODE_4K2K_50HZ);
            break;
        case HDMI_3840x2160p30_16x9:
        case HDMI_3840x2160p60_16x9:
            config_tv_enc(TVMODE_4K2K_30HZ);
            break;
        default :
            printk("Error: Unkown HDMI Video Identification Code (VIC)!\n");
            break;
    }
    hdmi_tvenc_set(vic);

//    aml_set_reg32_bits(P_VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.

    // --------------------------------------------------------
    // Configure video format timing for HDMI:
    // Based on the corresponding settings in set_tv_enc.c, calculate
    // the register values to meet the timing requirements defined in CEA-861-D
    // --------------------------------------------------------
    printk("Configure HDMI video format timing\n");

    // --------------------------------------------------------
    // Set up HDMI
    // --------------------------------------------------------
    config_hdmi20_tx(vic, para,                     // pixel_repeat,
                     TX_COLOR_DEPTH,                        // Pixel bit width: 4=24-bit; 5=30-bit; 6=36-bit; 7=48-bit.
                     TX_INPUT_COLOR_FORMAT,                 // input_color_format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
                     TX_INPUT_COLOR_RANGE,                  // input_color_range: 0=limited; 1=full.
                     TX_OUTPUT_COLOR_FORMAT,                // output_color_format: 0=RGB444; 1=YCbCr422; 2=YCbCr444; 3=YCbCr420.
                     TX_OUTPUT_COLOR_RANGE                 // output_color_range: 0=limited; 1=full.
                     );
    return;
}

static void hdmitx_set_pll(HDMI_Video_Codes_t vic)
{
    switch(vic)
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
            set_vmode_clk(VMODE_576I);
            break;
        case HDMI_1080p24://1080p24 support
            set_vmode_clk(VMODE_1080P_24HZ);
            break;
        case HDMI_1080p30:
        case HDMI_720p60:
        case HDMI_720p50:
            set_vmode_clk(VMODE_720P);
            break;
        case HDMI_1080i60:
        case HDMI_1080i50:
            set_vmode_clk(VMODE_1080I);
            break;
        case HDMI_1080p60:
        case HDMI_1080p50:
            set_vmode_clk(VMODE_1080P);
            break;
        case HDMI_4k2k_30:
        case HDMI_4k2k_25:
        case HDMI_4k2k_24:
        case HDMI_4k2k_smpte_24:
            set_vmode_clk(VMODE_4K2K_30HZ);
            break;
        case HDMI_3840x2160p60_16x9:
            if(mode420 == 1) {
                set_vmode_clk(VMODE_4K2K_60HZ_Y420);
            } else {
                set_vmode_clk(VMODE_4K2K_60HZ);
            };
            break;
        case HDMI_3840x2160p50_16x9:
            if(mode420 == 1) {
                set_vmode_clk(VMODE_4K2K_50HZ_Y420);
            } else {
                set_vmode_clk(VMODE_4K2K_50HZ);
            };
            break;
        default:
            break;
    }
}

static void hdmitx_set_phy(HDMI_Video_Codes_t vic)
{
    switch(vic) {
    case HDMI_3840x2160p50_16x9:
    case HDMI_3840x2160p60_16x9:
    case HDMI_4096x2160p50_256x135:
    case HDMI_4096x2160p60_256x135:
        aml_write_reg32(P_HHI_HDMI_PHY_CNTL0, 0x33b544ab);
        aml_write_reg32(P_HHI_HDMI_PHY_CNTL3, 0x303e0003);
        break;
    case HDMI_1080p60:
    case HDMI_4k2k_24:
    case HDMI_4k2k_25:
    case HDMI_4k2k_30:
    case HDMI_4k2k_smpte_24:
    default:
        aml_write_reg32(P_HHI_HDMI_PHY_CNTL0, 0x33b544ab);
        aml_write_reg32(P_HHI_HDMI_PHY_CNTL3, 0x303e005b);
        break;
    }
    if(mode420 == 1)
        aml_write_reg32(P_HHI_HDMI_PHY_CNTL3, 0x303e005b);
// P_HHI_HDMI_PHY_CNTL1     bit[1]: enable clock    bit[0]: soft reset
#define RESET_HDMI_PHY()                            \
    aml_write_reg32(P_HHI_HDMI_PHY_CNTL1, 0xf);     \
    mdelay(2);                                      \
    aml_write_reg32(P_HHI_HDMI_PHY_CNTL1, 0xe);     \
    mdelay(2)

    aml_write_reg32(P_HHI_HDMI_PHY_CNTL1, 0x0);
    RESET_HDMI_PHY();
    RESET_HDMI_PHY();
    RESET_HDMI_PHY();
#undef RESET_HDMI_PHY
    printk("phy setting done\n");
}

/*
 * mode: 1 means Progressive;  0 means interlaced
 */
static void enc_vpu_bridge_reset(int mode)
{
    unsigned int wr_clk = 0;

    printk("%s[%d]\n", __func__, __LINE__);
    wr_clk = (aml_read_reg32(P_VPU_HDMI_SETTING) & 0xf00) >> 8;
    if(mode) {
        aml_write_reg32(P_ENCP_VIDEO_EN, 0);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 0, 0, 2);  // [    0] src_sel_enci: Disable ENCP output to HDMI
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 0, 8, 4);  // [    0] src_sel_enci: Disable ENCP output to HDMI
        mdelay(1);
        aml_write_reg32(P_ENCP_VIDEO_EN, 1);
        mdelay(1);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, wr_clk, 8, 4);
        mdelay(1);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 2, 0, 2);  // [    0] src_sel_enci: Enable ENCP output to HDMI
    } else {
        aml_write_reg32(P_ENCI_VIDEO_EN, 0);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 0, 0, 2);  // [    0] src_sel_enci: Disable ENCI output to HDMI
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 0, 8, 4);  // [    0] src_sel_enci: Disable ENCP output to HDMI
        mdelay(1);
        aml_write_reg32(P_ENCI_VIDEO_EN, 1);
        mdelay(1);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, wr_clk, 8, 4);
        mdelay(1);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 0, 2);  // [    0] src_sel_enci: Enable ENCI output to HDMI
    }
}

static void hdmi_tvenc_set(HDMI_Video_Codes_t vic)
{
    config_hdmi_tvenc(vic);

    switch(vic) {
    case HDMI_480i60:
    case HDMI_480i60_16x9:
    case HDMI_576i50:
    case HDMI_576i50_16x9:
    case HDMI_480i60_16x9_rpt:
    case HDMI_576i50_16x9_rpt:
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
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
                             (1                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
        );
        if((vic == HDMI_480i60_16x9_rpt) || (vic == HDMI_576i50_16x9_rpt)) {
            aml_set_reg32_bits(P_VPU_HDMI_SETTING, 3, 12, 4);
        }
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 0, 1);  // [    0] src_sel_enci: Enable ENCI output to HDMI
        break;
    case HDMI_1080i60:
    case HDMI_1080i50:
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
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
                             (0                                 <<12)   // [15:12] rd_rate. 0=A read every clk2; 1=A read every 2 clk2; ...; 15=A read every 16 clk2.
        );
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
        break;
    case HDMI_4k2k_30:
    case HDMI_4k2k_25:
    case HDMI_4k2k_24:
    case HDMI_4k2k_smpte_24:
    case HDMI_3840x2160p50_16x9:
    case HDMI_3840x2160p60_16x9:
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                  << 0) | // [    0] src_sel_enci
                     (0                                 << 1) | // [    1] src_sel_encp
                     (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                     (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                     (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                     (4                                 << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
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
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
        aml_write_reg32(P_ENCP_VIDEO_EN, 1); // Enable VENC
        break;
    case HDMI_480p60_16x9_rpt:
    case HDMI_576p50_16x9_rpt:
    case HDMI_480p60:
    case HDMI_480p60_16x9:
    case HDMI_576p50:
    case HDMI_576p50_16x9:
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                             (0                                 << 1) | // [    1] src_sel_encp
                             (0                                 << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                             (0                                 << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                             (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                             (4                                 << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
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
        if((vic == HDMI_480p60_16x9_rpt) || (vic == HDMI_576p50_16x9_rpt)) {
            aml_set_reg32_bits(P_VPU_HDMI_SETTING, 3, 12, 4);
        }
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
        break;
    case HDMI_720p60:
    case HDMI_720p50:
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                             (0                                 << 1) | // [    1] src_sel_encp
                             (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                             (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                             (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                             (4                                 << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
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
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
        break;
    default:
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_write_reg32(P_VPU_HDMI_SETTING, (0                                 << 0) | // [    0] src_sel_enci
                             (0                                 << 1) | // [    1] src_sel_encp
                             (HSYNC_POLARITY                    << 2) | // [    2] inv_hsync. 1=Invert Hsync polarity.
                             (VSYNC_POLARITY                    << 3) | // [    3] inv_vsync. 1=Invert Vsync polarity.
                             (0                                 << 4) | // [    4] inv_dvi_clk. 1=Invert clock to external DVI, (clock invertion exists at internal HDMI).
                             (4                                 << 5) | // [ 7: 5] data_comp_map. Input data is CrYCb(BRG), map the output data to desired format:
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
        // Annie 01Sep2011: Register VENC_DVI_SETTING and VENC_DVI_SETTING_MORE are no long valid, use VPU_HDMI_SETTING instead.
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 1, 1);  // [    1] src_sel_encp: Enable ENCP output to HDMI
    }
}    

static void mode420_half_horizontal_para(void)
{
    unsigned int hactive = 0;
    unsigned int hblank = 0;
    unsigned int hfront = 0;
    unsigned int hsync = 0;

    printk("%s[%d]\n", __func__, __LINE__);
    hactive  =  hdmitx_rd_reg(HDMITX_DWC_FC_INHACTV0);
    hactive += (hdmitx_rd_reg(HDMITX_DWC_FC_INHACTV1) & 0x3f) << 8;
    hblank  =  hdmitx_rd_reg(HDMITX_DWC_FC_INHBLANK0);
    hblank += (hdmitx_rd_reg(HDMITX_DWC_FC_INHBLANK1) & 0x1f) << 8;
    hfront  =  hdmitx_rd_reg(HDMITX_DWC_FC_HSYNCINDELAY0);
    hfront += (hdmitx_rd_reg(HDMITX_DWC_FC_HSYNCINDELAY1) & 0x1f) << 8;
    hsync  =  hdmitx_rd_reg(HDMITX_DWC_FC_HSYNCINWIDTH0);
    hsync += (hdmitx_rd_reg(HDMITX_DWC_FC_HSYNCINWIDTH1) & 0x3) << 8;

    hactive = hactive / 2;
    hblank = hblank / 2;
    hfront = hfront / 2;
    hsync = hsync / 2;

    hdmitx_wr_reg(HDMITX_DWC_FC_INHACTV0, (hactive & 0xff));
    hdmitx_wr_reg(HDMITX_DWC_FC_INHACTV1, ((hactive >> 8) & 0x3f));
    hdmitx_wr_reg(HDMITX_DWC_FC_INHBLANK0, (hblank  & 0xff));
    hdmitx_wr_reg(HDMITX_DWC_FC_INHBLANK1, ((hblank >> 8) & 0x1f));
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINDELAY0, (hfront & 0xff));
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINDELAY1, ((hfront >> 8) & 0x1f));
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINWIDTH0, (hsync & 0xff));
    hdmitx_wr_reg(HDMITX_DWC_FC_HSYNCINWIDTH1, ((hsync >> 8) & 0x3));
}

static void set_tmds_clk_div40(unsigned int div40)
{
    if (div40 == 1) {
        hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_01, 0);          // [25:16] tmds_clk_pttn[19:10]  [ 9: 0] tmds_clk_pttn[ 9: 0]
        hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_23, 0x03ff03ff); // [25:16] tmds_clk_pttn[39:30]  [ 9: 0] tmds_clk_pttn[29:20]
    }
    else {
        hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_01, 0x001f001f);
        hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_23, 0x001f001f);
    }

    hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_CNTL, 0x1);            // 0xc
    msleep(10);
    hdmitx_wr_reg(HDMITX_TOP_TMDS_CLK_PTTN_CNTL, 0x2);            // 0xc
}

static void power_switch_to_vpu_hdmi(int pwr_ctrl)
{
    unsigned int i;
    if(pwr_ctrl == 1) {
        // Powerup VPU_HDMI
        aml_set_reg32_bits(P_AO_RTI_GEN_PWR_SLEEP0, 0, 8, 1);

        // power up memories
        for(i = 0; i < 32; i++) {
            aml_set_reg32_bits(P_HHI_VPU_MEM_PD_REG0, 0, i, 1);
            msleep(10);
        }
        for(i = 0; i < 32; i++) {
            aml_set_reg32_bits(P_HHI_VPU_MEM_PD_REG1, 0, i, 1);
            msleep(10);
        }
        for(i = 8; i < 16; i++) {
            aml_set_reg32_bits(P_HHI_MEM_PD_REG0, 0, i, 8); // MEM-PD
        }
        // Remove VPU_HDMI ISO
        aml_set_reg32_bits(P_AO_RTI_GEN_PWR_SLEEP0, 0, 9, 1);
    } else {
        // Add isolations
        aml_set_reg32_bits(P_AO_RTI_GEN_PWR_SLEEP0, 1, 9, 1);

        // Power off VPU_HDMI domain
        aml_write_reg32(P_HHI_VPU_MEM_PD_REG0, 0xffffffff );
        aml_write_reg32(P_HHI_VPU_MEM_PD_REG1, 0xffffffff );
        aml_write_reg32(P_HHI_MEM_PD_REG0, aml_read_reg32(HHI_MEM_PD_REG0) | (0xff << 8)); // HDMI MEM-PD
        aml_set_reg32_bits(P_AO_RTI_GEN_PWR_SLEEP0, 1, 8, 1);  //PDN
    }
}

void C_Entry(HDMI_Video_Codes_t vic)
{
    power_switch_to_vpu_hdmi(1);

    hdmitx_set_pll(vic);
    hdmitx_set_phy(vic);
    hdmi_tx_enc(vic);
    aml_write_reg32(P_VPU_HDMI_FMT_CTRL,(((TX_INPUT_COLOR_FORMAT==HDMI_COLOR_FORMAT_420)?2:0)  << 0) | // [ 1: 0] hdmi_vid_fmt. 0=444; 1=convert to 422; 2=convert to 420.
                         (2                                                     << 2) | // [ 3: 2] chroma_dnsmp. 0=use pixel 0; 1=use pixel 1; 2=use average.
                         (((TX_COLOR_DEPTH==HDMI_COLOR_DEPTH_24B)? 1:0)         << 4) | // [    4] dith_en. 1=enable dithering before HDMI TX input.
                         (0                                                     << 5) | // [    5] hdmi_dith_md: random noise selector.
                         (0                                                     << 6)); // [ 9: 6] hdmi_dith10_cntl.
    if(mode420 == 1) {
        aml_set_reg32_bits(P_VPU_HDMI_FMT_CTRL, 2, 0, 2);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 0, 4, 4);
        aml_set_reg32_bits(P_VPU_HDMI_SETTING, 1, 8, 1);
    }
    switch(vic) {
    case HDMI_480i60:
    case HDMI_480i60_16x9:
    case HDMI_576i50:
    case HDMI_576i50_16x9:
    case HDMI_480i60_16x9_rpt:
    case HDMI_576i50_16x9_rpt:
        enc_vpu_bridge_reset(0);
        break;
    default:
        enc_vpu_bridge_reset(1);
        break;
    }

    //hdmi_hw_reset(hdev, param);    
	// move hdmitx_set_pll() to the end of this function.
    // hdmitx_set_pll(param);

    if(mode420 == 1) {
        hdmitx_wr_reg(HDMITX_DWC_FC_AVICONF0, 0x43);    // change AVI packet
        mode420_half_horizontal_para();
    }
    if(((vic == HDMI_3840x2160p50_16x9) || (vic == HDMI_3840x2160p60_16x9))
       && (mode420 != 1)){
        set_tmds_clk_div40(1);
    } else {
        set_tmds_clk_div40(0);
    }
    hdmitx_set_reg_bits(HDMITX_DWC_FC_INVIDCONF, 0, 3, 1);
    msleep(1);
    hdmitx_set_reg_bits(HDMITX_DWC_FC_INVIDCONF, 1, 3, 1);
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

// TODO
void hdmitx_csc_config (unsigned char input_color_format,
                        unsigned char output_color_format,
                        unsigned char color_depth)
{
    unsigned char   conv_en;
    unsigned long   csc_coeff_a1, csc_coeff_a2, csc_coeff_a3, csc_coeff_a4;
    unsigned long   csc_coeff_b1, csc_coeff_b2, csc_coeff_b3, csc_coeff_b4;
    unsigned long   csc_coeff_c1, csc_coeff_c2, csc_coeff_c3, csc_coeff_c4;
    unsigned char   csc_scale;
    unsigned long   data32;

    conv_en = (((input_color_format  == HDMI_COLOR_FORMAT_RGB) ||
                (output_color_format == HDMI_COLOR_FORMAT_RGB)) &&
               ( input_color_format  != output_color_format))? 1 : 0;
    
    if (conv_en) {
        if (output_color_format == HDMI_COLOR_FORMAT_RGB) {
            csc_coeff_a1    = 0x2000;
            csc_coeff_a2    = 0x6926;
            csc_coeff_a3    = 0x74fd;
            csc_coeff_a4    = (color_depth==HDMI_COLOR_DEPTH_24B)? 0x010e :
                              (color_depth==HDMI_COLOR_DEPTH_30B)? 0x043b :
                              (color_depth==HDMI_COLOR_DEPTH_36B)? 0x10ee :
                              (color_depth==HDMI_COLOR_DEPTH_48B)? 0x10ee : 0x010e;
            csc_coeff_b1    = 0x2000;
            csc_coeff_b2    = 0x2cdd;
            csc_coeff_b3    = 0x0000;
            csc_coeff_b4    = (color_depth==HDMI_COLOR_DEPTH_24B)? 0x7e9a :
                              (color_depth==HDMI_COLOR_DEPTH_30B)? 0x7a65 :
                              (color_depth==HDMI_COLOR_DEPTH_36B)? 0x6992 :
                              (color_depth==HDMI_COLOR_DEPTH_48B)? 0x6992 : 0x7e9a;
            csc_coeff_c1    = 0x2000;
            csc_coeff_c2    = 0x0000;
            csc_coeff_c3    = 0x38b4;
            csc_coeff_c4    = (color_depth==HDMI_COLOR_DEPTH_24B)? 0x7e3b :
                              (color_depth==HDMI_COLOR_DEPTH_30B)? 0x78ea :
                              (color_depth==HDMI_COLOR_DEPTH_36B)? 0x63a6 :
                              (color_depth==HDMI_COLOR_DEPTH_48B)? 0x63a6 : 0x7e3b;
            csc_scale       = 1;
        } else {    // input_color_format == HDMI_COLOR_FORMAT_RGB
            csc_coeff_a1    = 0x2591;
            csc_coeff_a2    = 0x1322;
            csc_coeff_a3    = 0x074b;
            csc_coeff_a4    = 0x0000;
            csc_coeff_b1    = 0x6535;
            csc_coeff_b2    = 0x2000;
            csc_coeff_b3    = 0x7acc;
            csc_coeff_b4    = (color_depth==HDMI_COLOR_DEPTH_24B)? 0x0200 :
                              (color_depth==HDMI_COLOR_DEPTH_30B)? 0x0800 :
                              (color_depth==HDMI_COLOR_DEPTH_36B)? 0x2000 :
                              (color_depth==HDMI_COLOR_DEPTH_48B)? 0x2000 : 0x0200;
            csc_coeff_c1    = 0x6acd;
            csc_coeff_c2    = 0x7534;
            csc_coeff_c3    = 0x2000;
            csc_coeff_c4    = (color_depth==HDMI_COLOR_DEPTH_24B)? 0x0200 :
                              (color_depth==HDMI_COLOR_DEPTH_30B)? 0x0800 :
                              (color_depth==HDMI_COLOR_DEPTH_36B)? 0x2000 :
                              (color_depth==HDMI_COLOR_DEPTH_48B)? 0x2000 : 0x0200;
            csc_scale       = 0;
        }
    } else {
            csc_coeff_a1    = 0x2000;
            csc_coeff_a2    = 0x0000;
            csc_coeff_a3    = 0x0000;
            csc_coeff_a4    = 0x0000;
            csc_coeff_b1    = 0x0000;
            csc_coeff_b2    = 0x2000;
            csc_coeff_b3    = 0x0000;
            csc_coeff_b4    = 0x0000;
            csc_coeff_c1    = 0x0000;
            csc_coeff_c2    = 0x0000;
            csc_coeff_c3    = 0x2000;
            csc_coeff_c4    = 0x0000;
            csc_scale       = 1;
    }

    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A1_MSB,   (csc_coeff_a1>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A1_LSB,    csc_coeff_a1&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A2_MSB,   (csc_coeff_a2>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A2_LSB,    csc_coeff_a2&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A3_MSB,   (csc_coeff_a3>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A3_LSB,    csc_coeff_a3&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A4_MSB,   (csc_coeff_a4>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_A4_LSB,    csc_coeff_a4&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B1_MSB,   (csc_coeff_b1>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B1_LSB,    csc_coeff_b1&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B2_MSB,   (csc_coeff_b2>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B2_LSB,    csc_coeff_b2&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B3_MSB,   (csc_coeff_b3>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B3_LSB,    csc_coeff_b3&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B4_MSB,   (csc_coeff_b4>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_B4_LSB,    csc_coeff_b4&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C1_MSB,   (csc_coeff_c1>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C1_LSB,    csc_coeff_c1&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C2_MSB,   (csc_coeff_c2>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C2_LSB,    csc_coeff_c2&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C3_MSB,   (csc_coeff_c3>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C3_LSB,    csc_coeff_c3&0xff      );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C4_MSB,   (csc_coeff_c4>>8)&0xff  );
    hdmitx_wr_reg(HDMITX_DWC_CSC_COEF_C4_LSB,    csc_coeff_c4&0xff      );

    data32  = 0;
    data32 |= (color_depth  << 4);  // [7:4] csc_color_depth
    data32 |= (csc_scale    << 0);  // [1:0] cscscale
    hdmitx_wr_reg(HDMITX_DWC_CSC_SCALE,         data32);
}   /* hdmitx_csc_config */
