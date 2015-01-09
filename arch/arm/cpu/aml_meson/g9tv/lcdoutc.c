/*
 * AMLOGIC timing controller driver.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Elvis Yu <elvis.yu@amlogic.com>
 *
 */
#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/aml_lcd.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include <asm/arch/gpio.h>

#define BL_MAX_LEVEL 0x100
#define PANEL_NAME	"panel"

#define VPP_OUT_SATURATE            (1 << 0)
#ifdef PRINT_DEBUG_INFO
#define PRINT_INFO(...)        printf(__VA_ARGS__)
#else
#define PRINT_INFO(...)
#endif

/* defines for bits&width */
#define VBO_VIN_YUV_GBR 0
#define VBO_VIN_YVU_GRB 1
#define VBO_VIN_UYV_BGR 2
#define VBO_VIN_UVY_BRG 3
#define VBO_VIN_VYU_RGB 4
#define VBO_VIN_VUY_RBG 5

#define VBO_VIN_30BPP	0
#define VBO_VIN_24BPP   1
#define VBO_VIN_18BPP   2

//VBYONE_CTRL_L
//`define VBO_CTRL_L             8'h60
    #define VBO_ENABLE_BIT  0
    #define VBO_EBABLE_WID  1

    #define VBO_ALN_SHIFT_BIT     1
    #define VBO_ALN_SHIFT_WID     1

    #define VBO_LFSR_BYPASS_BIT   2
    #define VBO_LFSR_BYPASS_WID   1

    #define VBO_VDE_EXTEND_BIT    3
    #define VBO_VDE_EXTEND_WID    1

    #define VBO_HSYNC_SYNC_MODE_BIT   4
    #define VBO_HSYNC_SYNC_MODE_WID   2

    #define VBO_VSYNC_SYNC_MODE_BIT   6
    #define VBO_VSYNC_SYNC_MODE_WID   2

    #define VBO_FSM_CTRL_BIT      8
    #define VBO_FSM_CTRL_WID      3

    #define VBO_CTL_MODE_BIT      11
    #define VBO_CTL_MODE_WID      5

//`define VBO_CTRL_H             8'h61
    #define VBO_CTL_MODE2_BIT     0
    #define VBO_CTL_MODE2_WID     4

    #define VBO_B8B10_CTRL_BIT    4
    #define VBO_B8B10_CTRL_WID    3

    #define VBO_VIN2ENC_TMSYNC_MODE_BIT 8
    #define VBO_VIN2ENC_TMSYNC_MODE_WID 1

    #define VBO_VIN2ENC_HVSYNC_DLY_BIT  9
    #define VBO_VIN2ENC_HVSYNC_DLY_WID  1

    #define VBO_POWER_ON_BIT  12
    #define VBO_POWER_ON_WID  1

    #define VBO_PLL_LOCK_BIT  13
    #define VBO_PLL_LOCK_WID  1

//`define VBO_SOFT_RST           8'h62
   #define  VBO_SOFT_RST_BIT      0
   #define  VBO_SOFT_RST_WID      9

//`define VBO_LANES              8'h63
   #define  VBO_LANE_NUM_BIT      0
   #define  VBO_LANE_NUM_WID      3

   #define  VBO_LANE_REGION_BIT   4
   #define  VBO_LANE_REGION_WID   2

   #define  VBO_SUBLANE_NUM_BIT   8
   #define  VBO_SUBLANE_NUM_WID   3

   #define  VBO_BYTE_MODE_BIT     11
   #define  VBO_BYTE_MODE_WID     2

//`define VBO_VIN_CTRL           8'h64
   #define  VBO_VIN_SYNC_CTRL_BIT 0
   #define  VBO_VIN_SYNC_CTRL_WID 2

   #define  VBO_VIN_HSYNC_POL_BIT 4
   #define  VBO_VIN_HSYNC_POL_WID 1

   #define  VBO_VIN_VSYNC_POL_BIT 5
   #define  VBO_VIN_VSYNC_POL_WID 1

   #define  VBO_VOUT_HSYNC_POL_BIT 6
   #define  VBO_VOUT_HSYNC_POL_WID 1

   #define  VBO_VOUT_VSYNC_POL_BIT 7
   #define  VBO_VOUT_VSYNC_POL_WID 1

   #define  VBO_VIN_PACK_BIT      8
   #define  VBO_VIN_PACK_WID      3

   #define  VBO_VIN_BPP_BIT      11
   #define  VBO_VIN_BPP_WID       2

//`define VBO_ACT_VSIZE          8'h65
//`define VBO_REGION_00          8'h66
//`define VBO_REGION_01          8'h67
//`define VBO_REGION_02          8'h68
//`define VBO_REGION_03          8'h69
//`define VBO_VBK_CTRL_0         8'h6a
//`define VBO_VBK_CTRL_1         8'h6b
//`define VBO_HBK_CTRL           8'h6c
//`define VBO_PXL_CTRL           8'h6d
    #define  VBO_PXL_CTR0_BIT     0
    #define  VBO_PXL_CTR0_WID     4
    #define  VBO_PXL_CTR1_BIT     4
    #define  VBO_PXL_CTR1_WID     4

//`define VBO_LANE_SKEW_L        8'h6e
//`define VBO_LANE_SKEW_H        8'h6f
//`define VBO_GCLK_LANE_CTRL_L   8'h70
//`define VBO_GCLK_LANE_CTRL_H   8'h71
//`define VBO_GCLK_MAIN_CTRL     8'h72
//`define VBO_STATUS_L           8'h73
//`define VBO_STATUS_H           8'h74

typedef struct {
	Lcd_Config_t conf;
	vinfo_t lcd_info;
} lcd_dev_t;

static lcd_dev_t *pDev = NULL;

static int bit_num = 1;
static int pn_swap = 0;
static int dual_port = 1;
static int lvds_repack = 1;
static int port_reverse = 1;
static int bit_num_flag = 1;
static int lvds_repack_flag = 1;
static int port_reverse_flag = 1;

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
#define CLK_UTIL_VID_PLL_DIV_2p5    14
// --------------------------------------------------
//              clocks_set_vid_clk_div
// --------------------------------------------------
// wire            clk_final_en    = control[19];
// wire            clk_div1        = control[18];
// wire    [1:0]   clk_sel         = control[17:16];
// wire            set_preset      = control[15];
// wire    [14:0]  shift_preset    = control[14:0];
void clocks_set_vid_clk_div(int div_sel)
{
	int shift_val = 0;
	int shift_sel = 0;

	printf("%s[%d] div = %d\n", __func__, __LINE__, div_sel);
	// Disable the output clock
	aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 19, 1);
	aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);

	switch(div_sel) {
		case CLK_UTIL_VID_PLL_DIV_1:      shift_val = 0xFFFF; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_2:      shift_val = 0x0aaa; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_3:      shift_val = 0x0db6; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_3p5:    shift_val = 0x36cc; shift_sel = 1; break;
		case CLK_UTIL_VID_PLL_DIV_3p75:   shift_val = 0x6666; shift_sel = 2; break;
		case CLK_UTIL_VID_PLL_DIV_4:      shift_val = 0x0ccc; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_5:      shift_val = 0x739c; shift_sel = 2; break;
		case CLK_UTIL_VID_PLL_DIV_6:      shift_val = 0x0e38; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_6p25:   shift_val = 0x0000; shift_sel = 3; break;
		case CLK_UTIL_VID_PLL_DIV_7:      shift_val = 0x3c78; shift_sel = 1; break;
		case CLK_UTIL_VID_PLL_DIV_7p5:    shift_val = 0x78f0; shift_sel = 2; break;
		case CLK_UTIL_VID_PLL_DIV_12:     shift_val = 0x0fc0; shift_sel = 0; break;
		case CLK_UTIL_VID_PLL_DIV_14:     shift_val = 0x3f80; shift_sel = 1; break;
		case CLK_UTIL_VID_PLL_DIV_15:     shift_val = 0x7f80; shift_sel = 2; break;
		case CLK_UTIL_VID_PLL_DIV_2p5:    shift_val = 0x5294; shift_sel = 2; break;
		default:
		printf("Error: clocks_set_vid_clk_div:  Invalid parameter\n");
		break;
	}

	if(shift_val == 0xffff ) {      // if divide by 1
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 18, 1);
	} else {
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 16, 2);
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 0, 14);

		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, shift_sel, 16, 2);
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 15, 1);
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, shift_val, 0, 14);
		aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);
	}
	// Enable the final output clock
	aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 19, 1);
}

static int sHDMI_DPLL_DATA[][3] = {  //frequency(M)    HHI_HDMI_PLL_CNTL   HHI_HDMI_PLL_CNTL2: (bit18: OD1 is 1)
                                    {   399.840,         0x60000663,         0x00520f5b},
                                    {   378.000,         0x6000023e,         0x00920fff},
                                    {  2079.000,         0x500404ad,         0x00414400},
				//{  2079.000,         0x60000681,         0x00110eff},
                                    {   810.000,         0x60000886,         0x00120fff},
                                    {  1080.000,         0x6000022c,         0x00120fff},
                                    {  2227.500,         0x6000068b,         0x00110380},
                                    {  4455.000,         0x6000068b,         0x00100380},
                                    {  2970.000,         0x6000023d,         0x00110dff},
                                    {  5940.000,         0x6000023d,         0x00100dff},
                                    {   540.000,         0x6000022c,         0x00520fff},
                                    {   576.000,         0x6000022f,         0x00520fff},
                                    {   594.000,         0x60000462,         0x00520fff},
                                    {  1188.000,         0x60000462,         0x00120fff},
                                    {   742.500,         0x6000023d,         0x00520dff},
                                    {  1485.000,         0x6000023d,         0x00120dff},
                                    {   928.125,         0x60000674,         0x00120040},
                                    {  1856.250,         0x60000674,         0x00110040},
                                    {  1039.500,         0x60000681,         0x00120eff},
                                    {  2702.002,         0x60000470,         0x00110955},
                                    {   337.500,         0x600008e0,         0x00920fff},
                                    {   270.000,         0x6000022c,         0x00920fff},
                                    {        0,                   0,                  0}
                                  };

int set_hdmi_dpll(int freq, int od1)
{
	int i;
	i=0;
	while(sHDMI_DPLL_DATA[i][0]!=0) {
		if(sHDMI_DPLL_DATA[i][0] == freq)
			break;
		i++;
	}

	if(sHDMI_DPLL_DATA[i][0]==0)
		return 1;
	else {
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL, sHDMI_DPLL_DATA[i][1]);
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL2,sHDMI_DPLL_DATA[i][2]);
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL, sHDMI_DPLL_DATA[i][1] & (~(1<<28)));
		//aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2,od1,18,2); //no used ,bit[18:19] is od3
	}
	printf("Wait 10us for phy_clk stable!\n");
	udelay(10); // delay 10uS to wait clock is stable 

	return 0;
}

void set_crt_video_enc(int vIdx, int inSel, int DivN)
{
	if(vIdx==0) //V1
	{
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0

		udelay(2);  //delay 2uS

		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, inSel,   16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VID_CLK_DIV, (DivN-1), 0, 8); // [7:0]   - cntl_xd0
		
		udelay(5);  // delay 5uS

		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0

	} else { //V2
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0

		udelay(2);  //delay 2uS

		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, inSel,  16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, (DivN-1),0, 8); // [7:0]   - cntl_xd0
		
		udelay(5);  // delay 5uS 

		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0
	}

	udelay(5);  //delay 5uS
}

void enable_crt_video_encl(int enable, int inSel)
{
	aml_set_reg32_bits(P_HHI_VIID_CLK_DIV,inSel,  12, 4); //encl_clk_sel:hi_viid_clk_div[15:12]

	if(inSel<=4) //V1
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL,1, inSel, 1);
	else
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL,1, (inSel-5),1);

	aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2,enable, 3, 1); //gclk_encl_clk:hi_vid_clk_cntl2[3]

#ifndef NO_EDP_DSI
	aml_set_reg32_bits(P_VPU_MISC_CTRL, 1, 0, 1);    // vencl_clk_en_force: vpu_misc_ctrl[0]
#endif
}
 
void vpp_set_matrix_ycbcr2rgb (int vd1_or_vd2_or_post, int mode)
{
	if (vd1_or_vd2_or_post == 0){ //vd1
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 5, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 8, 2);
	}else if (vd1_or_vd2_or_post == 1){ //vd2
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 4, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 2, 8, 2);
	}else{
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 0, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 0, 8, 2);
		if (mode == 0){
			aml_set_reg32_bits(P_VPP_MATRIX_CTRL, 1, 1, 2);
		}else if (mode == 1){
			aml_set_reg32_bits(P_VPP_MATRIX_CTRL, 0, 1, 2);
		}
	}

	if (mode == 0){ //ycbcr not full range, 601 conversion
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET0_1, 0x0064C8FF);
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET2, 0x006400C8);

		//1.164     0       1.596
		//1.164   -0.392    -0.813
		//1.164   2.017     0
		aml_write_reg32(P_VPP_MATRIX_COEF00_01, 0x04000000);
		aml_write_reg32(P_VPP_MATRIX_COEF02_10, 0x059C0400);
		aml_write_reg32(P_VPP_MATRIX_COEF11_12, 0x1EA01D24);
		aml_write_reg32(P_VPP_MATRIX_COEF20_21, 0x04000718);
		aml_write_reg32(P_VPP_MATRIX_COEF22, 0x00000000);
		aml_write_reg32(P_VPP_MATRIX_OFFSET0_1, 0x00000000);
		aml_write_reg32(P_VPP_MATRIX_OFFSET2, 0x00000000);
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET0_1, 0x00000E00);
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET2, 0x00000E00);
	}else if (mode == 1) {//ycbcr full range, 601 conversion
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET0_1, 0x0000600);
		aml_write_reg32(P_VPP_MATRIX_PRE_OFFSET2, 0x0600);

		//1     0       1.402
		//1   -0.34414  -0.71414
		//1   1.772     0
		aml_write_reg32(P_VPP_MATRIX_COEF00_01, (0x400 << 16) |0);
		aml_write_reg32(P_VPP_MATRIX_COEF02_10, (0x59c << 16) |0x400);
		aml_write_reg32(P_VPP_MATRIX_COEF11_12, (0x1ea0 << 16) |0x1d25);
		aml_write_reg32(P_VPP_MATRIX_COEF20_21, (0x400 << 16) |0x717);
		aml_write_reg32(P_VPP_MATRIX_COEF22, 0x0);
		aml_write_reg32(P_VPP_MATRIX_OFFSET0_1, 0x0);
		aml_write_reg32(P_VPP_MATRIX_OFFSET2, 0x0);
	}
}

static void set_tcon_lvds(Lcd_Config_t *pConf)
{
	vpp_set_matrix_ycbcr2rgb(2, 0);
	aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 3);
	aml_write_reg32(P_L_RGB_BASE_ADDR, 0);
	aml_write_reg32(P_L_RGB_COEFF_ADDR, 0x400);

	if(pConf->lcd_basic.lcd_bits == 8)
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x400);
	else if(pConf->lcd_basic.lcd_bits == 6)
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x600);
	else
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0);

	aml_write_reg32(P_VPP_MISC, aml_read_reg32(P_VPP_MISC) & ~(VPP_OUT_SATURATE));
}



//new lvd_vx1_phy config
void lvds_phy_config(int lvds_vx1_sel)
{
	//debug
	aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);
	//debug

	if(lvds_vx1_sel == 0){ //lvds
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6c6cca80);
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x0000006c);
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x0fff0800);
		//od   clk 1039.5 / 2 = 519.75 = 74.25*7
		aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0x0fff0040);
	}else{

	}

}

void vclk_set_encl_lvds(vmode_t vmode, int lvds_ports)
{
	int hdmi_clk_out;
	//int hdmi_vx1_clk_od1;
	int vx1_phy_div;
	int encl_div;
	unsigned int xd;
	//no used, od2 must >= od3.
	//hdmi_vx1_clk_od1 = 1; //OD1 always 1

	if(lvds_ports<2){
		//pll_video.pl 3500 pll_out
		switch(vmode) {
			case VMODE_LVDS_1080P: //total: 2200x1125 pixel clk = 148.5MHz,phy_clk(s)=(pclk*7)= 1039.5 = 2079/2
				hdmi_clk_out = 2079;
				vx1_phy_div  = 2/2;
				encl_div     = vx1_phy_div*7;
				break;
			default:
				printf("Error video format!\n");
				return;
		}
		//if(set_hdmi_dpll(hdmi_clk_out,hdmi_vx1_clk_od1)) {
		if(set_hdmi_dpll(hdmi_clk_out,0)) {
			printf("Unsupported HDMI_DPLL out frequency!\n");
			return;
		}

		if(lvds_ports==1) //dual port
			vx1_phy_div = vx1_phy_div*2;
	}else if(lvds_ports>=2) {
		printf("Quad-LVDS is not supported!\n");
		return;
	}

	//configure vid_clk_div_top
	if((encl_div%14)==0){//7*even
		clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_14);
		xd = encl_div/14;
	}else if((encl_div%7)==0){ //7*odd
		clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_7);
		xd = encl_div/7;
	}else{ //3.5*odd
		clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_3p5);
		xd = encl_div/3.5;
	}
	//for lvds phy clock and enable decoupling FIFO
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1,((3<<6)|((vx1_phy_div-1)<<1)|1)<<24);

	//config lvds phy
	lvds_phy_config(0);
	//configure crt_video
	set_crt_video_enc(0,0,xd);  //configure crt_video V1: inSel=vid_pll_clk(0),DivN=xd)
	enable_crt_video_encl(1,0); //select and enable the output
}

static void set_pll_lvds(Lcd_Config_t *pConf)
{
	PRINT_INFO("%s\n", __FUNCTION__);

	vclk_set_encl_lvds(pDev->lcd_info.mode, pConf->lvds_mlvds_config.lvds_config->dual_port);
	aml_write_reg32( P_HHI_VIID_DIVIDER_CNTL, ((aml_read_reg32(P_HHI_VIID_DIVIDER_CNTL) & ~(0x7 << 8)) | (2 << 8) | (0<<10)) );
	aml_write_reg32(P_LVDS_GEN_CNTL, (aml_read_reg32(P_LVDS_GEN_CNTL)| (1 << 3) | (3<< 0)));
}

static void venc_set_lvds(Lcd_Config_t *pConf)
{
	PRINT_INFO("%s.\n",__FUNCTION__);
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON6TV
	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL, (0<<0) |    // viu1 select encl
											(0<<2) );    // viu2 select encl
#endif
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);
	//int havon_begin = 80;
	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL, (0<<0) |    // viu1 select encl
											(0<<2) );     // viu2 select encl
	aml_write_reg32(P_ENCL_VIDEO_MODE, 0); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	aml_write_reg32(P_ENCL_VIDEO_MODE_ADV,     0x0418); // Sampling rate: 1

	// bypass filter
	aml_write_reg32(P_ENCL_VIDEO_FILT_CTRL, 0x1000);

	aml_write_reg32(P_ENCL_VIDEO_MAX_PXCNT, pConf->lcd_basic.h_period - 1);
		aml_write_reg32(P_ENCL_VIDEO_MAX_LNCNT, pConf->lcd_basic.v_period - 1);

	aml_write_reg32(P_ENCL_VIDEO_HAVON_BEGIN, pConf->lcd_timing.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_HAVON_END, pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_BLINE,	pConf->lcd_timing.video_on_line);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_ELINE,	pConf->lcd_basic.v_active - 1  + pConf->lcd_timing.video_on_line);

	aml_write_reg32(P_ENCL_VIDEO_HSO_BEGIN,	pConf->lcd_timing.sth1_hs_addr);//10);
	aml_write_reg32(P_ENCL_VIDEO_HSO_END,	pConf->lcd_timing.sth1_he_addr);//20);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BEGIN,	pConf->lcd_timing.stv1_hs_addr);//10);
	aml_write_reg32(P_ENCL_VIDEO_VSO_END,	pConf->lcd_timing.stv1_he_addr);//20);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BLINE,	pConf->lcd_timing.stv1_vs_addr);//2);
	aml_write_reg32(P_ENCL_VIDEO_VSO_ELINE,	pConf->lcd_timing.stv1_ve_addr);//4);
		aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 0);

		// enable encl
		aml_write_reg32(P_ENCL_VIDEO_EN, 1);
	}


static void venc_set_vx1(Lcd_Config_t *pConf)
{
	PRINT_INFO("%s\n", __FUNCTION__);

	aml_write_reg32(P_ENCL_VIDEO_EN, 0);
	//int havon_begin = 80;
	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL, (0<<0) |    // viu1 select encl
											(3<<2) );     // viu2 select encl
	aml_write_reg32(P_ENCL_VIDEO_MODE, 40);//0); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	aml_write_reg32(P_ENCL_VIDEO_MODE_ADV,     0x18);//0x0418); // Sampling rate: 1

	// bypass filter
	aml_write_reg32(P_ENCL_VIDEO_FILT_CTRL, 0x1000); //??

	aml_write_reg32(P_ENCL_VIDEO_MAX_PXCNT, 3840+560-1);//pConf->lcd_basic.h_period - 1);
	aml_write_reg32(P_ENCL_VIDEO_MAX_LNCNT, 2160+90-1);//pConf->lcd_basic.v_period - 1);

	aml_write_reg32(P_ENCL_VIDEO_HAVON_BEGIN, 560-3);//pConf->lcd_timing.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_HAVON_END, 3839+560-3);//pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_BLINE,	90);//pConf->lcd_timing.video_on_line);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_ELINE,	2159+90);//pConf->lcd_basic.v_active - 1  + pConf->lcd_timing.video_on_line);

	aml_write_reg32(P_ENCL_VIDEO_HSO_BEGIN,48-1);//	pConf->lcd_timing.sth1_hs_addr);//10);
	aml_write_reg32(P_ENCL_VIDEO_HSO_END,	48-1+32);//pConf->lcd_timing.sth1_he_addr);//20);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BEGIN,	48-1);//pConf->lcd_timing.stv1_hs_addr);//10);
	aml_write_reg32(P_ENCL_VIDEO_VSO_END,	48-1);//pConf->lcd_timing.stv1_he_addr);//20);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BLINE,	3);//pConf->lcd_timing.stv1_vs_addr);//2);
	aml_write_reg32(P_ENCL_VIDEO_VSO_ELINE,	9);//pConf->lcd_timing.stv1_ve_addr);//4);

	aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 0);

	// enable encl
	aml_write_reg32(P_ENCL_VIDEO_EN, 1);
}

static void set_control_lvds(Lcd_Config_t *pConf)
{
	PRINT_INFO("%s\n", __FUNCTION__);

	if(lvds_repack_flag)
		lvds_repack = (pConf->lvds_mlvds_config.lvds_config->lvds_repack) & 0x1;
	pn_swap = (pConf->lvds_mlvds_config.lvds_config->pn_swap) & 0x1;
	dual_port = (pConf->lvds_mlvds_config.lvds_config->dual_port) & 0x1;
	if(port_reverse_flag)
		port_reverse = (pConf->lvds_mlvds_config.lvds_config->port_reverse) & 0x1;

	if(bit_num_flag){
		switch(pConf->lcd_basic.lcd_bits) {
			case 10: bit_num=0; break;
			case 8: bit_num=1; break;
			case 6: bit_num=2; break;
			case 4: bit_num=3; break;
			default: bit_num=1; break;
		}
	}

	aml_write_reg32(P_MLVDS_CONTROL,  (aml_read_reg32(P_MLVDS_CONTROL) & ~(1 << 0)));  //disable mlvds
	aml_write_reg32(P_LVDS_PACK_CNTL_ADDR,
					( lvds_repack<<0 ) | // repack
					( port_reverse?(0<<2):(1<<2)) | // odd_even
					( 0<<3 ) | // reserve
					( 0<<4 ) | // lsb first
					( pn_swap<<5 ) | // pn swap
					( dual_port<<6 ) | // dual port
					( 0<<7 ) | // use tcon control
					( bit_num<<8 ) | // 0:10bits, 1:8bits, 2:6bits, 3:4bits.
					( 0<<10 ) | //r_select  //0:R, 1:G, 2:B, 3:0
					( 1<<12 ) | //g_select  //0:R, 1:G, 2:B, 3:0
					( 2<<14 ));  //b_select  //0:R, 1:G, 2:B, 3:0;
}

static void init_lvds_phy(Lcd_Config_t *pConf)
{
	//debug
	aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);
	//debug
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6c6cca80);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x0000006c);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x0fff0800);
	//od   clk 1039.5 / 2 = 519.75 = 74.25*7
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0x0fff0040);
}

int config_vbyone(int lane, int byte, int region, int hsize, int vsize)
{
   int sublane_num;
   int region_size[4];
   int tmp;

   if((lane == 0) || (lane == 3) || (lane == 5) || (lane == 6) || (lane == 7) || (lane>8))
      return 1;
   if((region ==0) || (region==3) || (region>4))
      return 1;
   if(lane%region)
      return 1;
   if((byte<3) || (byte>4))
      return 1;

   sublane_num = lane/region;
   aml_set_reg32_bits(P_VBO_LANES,lane-1,  VBO_LANE_NUM_BIT,    VBO_LANE_NUM_WID);
   aml_set_reg32_bits(P_VBO_LANES,region-1,VBO_LANE_REGION_BIT, VBO_LANE_REGION_WID);
   aml_set_reg32_bits(P_VBO_LANES,sublane_num-1,VBO_SUBLANE_NUM_BIT, VBO_SUBLANE_NUM_WID);
   aml_set_reg32_bits(P_VBO_LANES,byte-1,VBO_BYTE_MODE_BIT, VBO_BYTE_MODE_WID);

   if(region>1)
   {
       region_size[3] = (hsize/lane)*sublane_num;
       tmp = (hsize%lane);
       region_size[0] = region_size[3] + (((tmp/sublane_num)>0) ? sublane_num : (tmp%sublane_num));
       region_size[1] = region_size[3] + (((tmp/sublane_num)>1) ? sublane_num : (tmp%sublane_num));
       region_size[2] = region_size[3] + (((tmp/sublane_num)>2) ? sublane_num : (tmp%sublane_num));
       aml_write_reg32(P_VBO_REGION_00,region_size[0]);
       aml_write_reg32(P_VBO_REGION_01,region_size[1]);
       aml_write_reg32(P_VBO_REGION_02,region_size[2]);
       aml_write_reg32(P_VBO_REGION_03,region_size[3]);
   }
   aml_write_reg32(P_VBO_ACT_VSIZE,vsize);
   //aml_set_reg32_bits(P_VBO_CTRL_H,0x80,VBO_CTL_MODE_BIT,VBO_CTL_MODE_WID);  // different from FBC code!!!
   aml_set_reg32_bits(P_VBO_CTRL_H,0x0,VBO_CTL_MODE2_BIT,VBO_CTL_MODE2_WID); // different from simulation code!!!
   aml_set_reg32_bits(P_VBO_CTRL_H,0x1,VBO_VIN2ENC_HVSYNC_DLY_BIT,VBO_VIN2ENC_HVSYNC_DLY_WID);
   //aml_set_reg32_bits(P_VBO_CTRL_L,enable,VBO_ENABLE_BIT,VBO_EBABLE_WID);

   return 0;
}

void set_vbyone_ctlbits(int p3d_en, int p3d_lr, int mode)
{
	if(mode==0)  //insert at the first pixel
		aml_set_reg32_bits(P_VBO_PXL_CTRL,(1<<p3d_en)|(p3d_lr&0x1),VBO_PXL_CTR0_BIT,VBO_PXL_CTR0_WID);
	else
		aml_set_reg32_bits(P_VBO_VBK_CTRL_0,(1<<p3d_en)|(p3d_lr&0x1),0,2);
}

void set_vbyone_sync_pol(int hsync_pol, int vsync_pol)
{
    aml_set_reg32_bits(P_VBO_VIN_CTRL,hsync_pol,VBO_VIN_HSYNC_POL_BIT,VBO_VIN_HSYNC_POL_WID);
    aml_set_reg32_bits(P_VBO_VIN_CTRL,vsync_pol,VBO_VIN_VSYNC_POL_BIT,VBO_VIN_VSYNC_POL_WID);

    aml_set_reg32_bits(P_VBO_VIN_CTRL,hsync_pol,VBO_VOUT_HSYNC_POL_BIT,VBO_VOUT_HSYNC_POL_WID);
    aml_set_reg32_bits(P_VBO_VIN_CTRL,vsync_pol,VBO_VOUT_VSYNC_POL_BIT,VBO_VOUT_VSYNC_POL_WID);
}

static void set_control_vbyone(Lcd_Config_t *pConf)
{
    int lane, byte, region,  hsize, vsize;//color_fmt,
    int vin_color, vin_bpp;

    hsize = 3840;//pConf->lcd_basic.h_active;
	vsize = 2160;//pConf->lcd_basic.v_active;
    lane = 8;//byte_num;
	byte = 4;//byte_num;
	region = 2;//region_num;
	vin_color = 4;
	vin_bpp   = 0;
    //switch (color_fmt) {
    //    case 0:   //SDVT_VBYONE_18BPP_RGB
    //              vin_color = 4;
    //              vin_bpp   = 2;
    //              break;
    //    case 1:   //SDVT_VBYONE_18BPP_YCBCR444
    //              vin_color = 0;
    //              vin_bpp   = 2;
    //              break;
    //    case 2:   //SDVT_VBYONE_24BPP_RGB
    //              vin_color = 4;
    //              vin_bpp   = 1;
    //              break;
    //    case 3:   //SDVT_VBYONE_24BPP_YCBCR444
    //              vin_color = 0;
    //              vin_bpp   = 1;
    //              break;
    //    case 4:   //SDVT_VBYONE_30BPP_RGB
                  //vin_color = 4;
                  //vin_bpp   = 0;
    //              break;
    //    case 5:   //SDVT_VBYONE_30BPP_YCBCR444
    //              vin_color = 0;
    //              vin_bpp   = 0;
    //              break;
    //    default:
    //        printf( "Error VBYONE_COLOR_FORMAT!\n");
    //              return;
    //}
    // clock seting for VX1
    //vclk_set_encl_vx1(vfromat, lane, byte);

    // set encl format
    //set_tv_encl (TV_ENC_LCD3840x2160p_vic03,1,0,0);

    // vpu clock setting
    //aml_set_reg32(P_HHI_VPU_CLK_CNTL,   (0 << 9)    |   // vpu   clk_sel
    //                        (0 << 0) );     // vpu   clk_div
    //aml_set_reg32(P_HHI_VPU_CLK_CNTL, (Rd(HHI_VPU_CLK_CNTL) | (1 << 8)) );

    //PIN_MUX for VX1 need to add this to dtd
    printf("Set VbyOne PIN MUX ......\n");
    aml_set_reg32_bits(P_PERIPHS_PIN_MUX_3,3,8,2);

    // set Vbyone
    printf("VbyOne Configuration ......\n");
    //set_vbyone_vfmt(vin_color,vin_bpp);
    aml_set_reg32_bits(P_VBO_VIN_CTRL, vin_color, VBO_VIN_PACK_BIT,VBO_VIN_PACK_WID);
    aml_set_reg32_bits(P_VBO_VIN_CTRL,vin_bpp,  VBO_VIN_BPP_BIT,VBO_VIN_BPP_WID);
    config_vbyone(lane, byte, region, hsize, vsize);
    set_vbyone_sync_pol(0, 0); //set hsync/vsync polarity to let the polarity is low active inside the VbyOne

    // below line copy from simulation
    aml_set_reg32_bits(P_VBO_VIN_CTRL, 1, 0, 2); //gate the input when vsync asserted
    ///aml_set_reg32(P_VBO_VBK_CTRL_0,0x13);
    //aml_set_reg32(P_VBO_VBK_CTRL_1,0x56);
    //aml_set_reg32(P_VBO_HBK_CTRL,0x3478);
    //aml_set_reg32_bits(P_VBO_PXL_CTRL,0x2,VBO_PXL_CTR0_BIT,VBO_PXL_CTR0_WID);
    //aml_set_reg32_bits(P_VBO_PXL_CTRL,0x3,VBO_PXL_CTR1_BIT,VBO_PXL_CTR1_WID);
    //set_vbyone_ctlbits(1,0,0);
    //set fifo_clk_sel: 3 for 10-bits
    aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL0,3,6,2);

    //PAD select:
	if((lane == 1) || (lane == 2)) {
		aml_set_reg32_bits(P_LCD_PORT_SWAP,1,9,2);
    } else if(lane == 4) {
    	aml_set_reg32_bits(P_LCD_PORT_SWAP,2,9,2);
    } else {
    	aml_set_reg32_bits(P_LCD_PORT_SWAP,0,9,2);
    }
    //aml_set_reg32_bits(P_LCD_PORT_SWAP, 1, 8, 1);//reverse lane output order

    // Mux pads in combo-phy: 0 for dsi; 1 for lvds or vbyone; 2 for edp
    aml_write_reg32(P_HHI_DSI_LVDS_EDP_CNTL0, 0x1); // Select vbyone in combo-phy
    aml_set_reg32_bits(P_VBO_CTRL_L, 1, VBO_ENABLE_BIT, VBO_EBABLE_WID);

    //force vencl clk enable, otherwise, it might auto turn off by mipi DSI
    //WRITE_VCBUS_REG_BITS(VPU_MISC_CTRL, 1, 0, 1);

	printf("VbyOne is In Normal Status ......\n");
}

static void init_vbyone_phy(Lcd_Config_t *pConf)
{
	printf("%s\n", __FUNCTION__);
	//debug
	aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);
	//debug
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6e0ec918);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x00000a7c);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x00ff0800);
	//od   clk 2970 / 5 = 594
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0xfff00c0);
	//clear lvds fifo od (/2)
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1, 0xc1000000);

}
static void set_tcon_vbyone(Lcd_Config_t *pConf)
{
    //Lcd_Timing_t *tcon_adr = &(pConf->lcd_timing);

    vpp_set_matrix_ycbcr2rgb(2, 0);
    aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 3);
    aml_write_reg32(P_L_RGB_BASE_ADDR, 0);
    aml_write_reg32(P_L_RGB_COEFF_ADDR, 0x400);
    //aml_write_reg32(P_L_POL_CNTL_ADDR,  3);
    //aml_write_reg32(P_L_DUAL_PORT_CNTL_ADDR, (0x1 << LCD_TTL_SEL));
//	if(pConf->lcd_basic.lcd_bits == 8)
//		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x400);
//	else if(pConf->lcd_basic.lcd_bits == 6)
//		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x600);
//	else
//		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0);
    //PRINT_INFO("final LVDS_FIFO_CLK = %d\n", clk_util_clk_msr(24));
	//PRINT_INFO("final cts_encl_clk = %d\n", clk_util_clk_msr(9));
    aml_write_reg32(P_VPP_MISC, aml_read_reg32(P_VPP_MISC) & ~(VPP_OUT_SATURATE));
}

//clock seting for VX1
static void set_pll_vbyone(Lcd_Config_t *pConf)
{
    int  lane, byte;//vfromat,
	//int hdmi_clk_out;
	int hdmi_vx1_clk_od1;
	int pclk_div;
	int phy_div;
	int xd;
	int minlane;

    lane = 8;//lane_num;
    byte = 4;//byte_num;
	//phy_clk = pixel_clk*10*byte_num/lane_num;
	//                                   lane_num      byte_num      phy_clk
	//TV_ENC_LCD720x480:(pclk=27M) 858x525  8            3            101.25      (pclk*3.75)
	//                                      4            3            202.5       (pclk*7.5)
	//                                      2            3            405         (pclk*15)
	//                                      1            3            810         (pclk*30)
	//                                      8            4            135         (pclk*5)
	//                                      4            4            270         (pclk*10)
	//                                      2            4            540         (pclk*20)
	//                                      1            4            1080        (pclk*40)
	//                                   lane_num      byte_num      phy_clk
	//TV_ENC_LCD1920x1080:(pclk=148.5M)     8            3            556.875     (pclk*3.75)
	//                      2200x1125       4            3            1113.75     (pclk*7.5)
	//                                      2            3            2227.5      (pclk*15)
	//                                      1            3            4455        (pclk*30)
	//                                      8            4            742.5       (pclk*5)
	//                                      4            4            1485        (pclk*10)
	//                                      2            4            2970        (pclk*20)
	//                                      1            4            5940        (pclk*40)
	//                                   lane_num      byte_num      phy_clk
	//TV_ENC_LCD3840x2160p:(pclk=594M)      8            3            2227.5      (pclk*3.75)
	//                      4400x2250       4            3            4455        (pclk*7.5)
	//                                      2            3            8910        (pclk*15)
	//                                      1            3           17820        (pclk*30)
	//                                      8            4            2970        (pclk*5)
	//                                      4            4            5940        (pclk*10)
	//                                      2            4           11880        (pclk*20)
	//                                      1            4           23760        (pclk*40)
    //if (byte_num == 3)
    //	hdmi_clk_out = 2227.5*2;  //OD1 = 1
    //else //4
    //	hdmi_clk_out = 2970*2;    //OD1 = 1
    minlane = 8;
    //hdmi_vx1_clk_od1 = 1;

	if (lane < minlane) {
		printf("VX1 cannot support this configuration!\n");
		return;
	}

	phy_div = lane/minlane; //1,2,4,8
	if (phy_div == 8) {
    	phy_div = phy_div/2;
    	if(hdmi_vx1_clk_od1 != 0) {
    		printf("VX1 cannot support this configuration!\n");
    		return;
    	}
    //	hdmi_vx1_clk_od1=1;
	}
	//need check whether we need to set this dpll !!!!!!!
	//if (set_hdmi_dpll(hdmi_clk_out,hdmi_vx1_clk_od1)){
    //   printf("Unsupported HDMI_DPLL out frequency!\n");
    //    return;
    //}

   pclk_div = (((byte==3) ? 30:40)*100)/minlane;
   printf("vbyone byte:%d, lane:%d, pclk:%d, phy_div:%d \n", byte, lane, pclk_div, phy_div);
   //configure vid_clk_div_top
   if (byte == 3) {
	   if (pclk_div == 375) {
		   clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_3p75);
		   xd = 1;
	   } else if (pclk_div == 750) {
		   clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_7p5);
		   xd = 1;
	   } else {
		   clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_15);
		   xd = pclk_div/100/15;
	   }
	} else if (byte == 4) {
		clocks_set_vid_clk_div(CLK_UTIL_VID_PLL_DIV_5);
		xd = pclk_div/100/5;
	}

	//for lvds phy clock and enable decoupling FIFO
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1,((3<<6)|((phy_div-1)<<1)|1)<<24);

	//configure crt_video
	//set_crt_video_enc(0, 0, xd);  //configure crt_video V1: inSel=vid_pll_clk(0),DivN=xd)
	//if (vidx == 0)
	{ //V1
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0
		udelay(2);  //delay 2uS
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0,   16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VID_CLK_DIV, (xd-1), 0, 8); // [7:0]   - cntl_xd0
		udelay(5);  // delay 5uS
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0
	}
    //else { //V2
	//	aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0
	//	udelay(2);  //delay 2uS
	//	aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, insel,  16, 3); // [18:16] - cntl_clk_in_sel
	//	aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, (divn-1),0, 8); // [7:0]   - cntl_xd0
	//	udelay(5);  // delay 5uS
	//	aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0
	//}
	udelay(5);  // delay 5uS
	//enable_crt_video_encl(1, 0); //select and enable the output
	aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, 0,  12, 4); //encl_clk_sel:hi_viid_clk_div[15:12]
	//if(inSel<=4) //V1
	aml_set_reg32_bits(P_HHI_VID_CLK_CNTL,1, 0, 1);
	//else
	//	aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL,1, (inSel-5),1);

	aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2, 1, 3, 1); //gclk_encl_clk:hi_vid_clk_cntl2[3]
}

void set_vx1_pinmux(void)
{
	//VX1_LOCKN && VX1_HTPDN
	aml_set_reg32_bits(P_PERIPHS_PIN_MUX_7, 3, 18, 2);
}

static inline void _init_display_driver(Lcd_Config_t *pConf)
{
	int lcd_type;

	const char* lcd_type_table[]={
		"NULL",
		"TTL",
		"LVDS",
		"MINILVDS",
		"VBYONE",
		"invalid",
	};

	lcd_type = pDev->conf.lcd_basic.lcd_type;
	printf("\nInit LCD type: %s.\n", lcd_type_table[lcd_type]);

	switch(lcd_type) {
		case LCD_DIGITAL_LVDS:
			printf("lvds mode is selected!\n");
			set_pll_lvds(pConf);
			venc_set_lvds(pConf);
			set_control_lvds(pConf);
			init_lvds_phy(pConf);
			set_tcon_lvds(pConf);
			break;
		case LCD_DIGITAL_VBYONE:
			printf("vx1 mode is selected!\n");
			set_pll_vbyone(pConf);
			venc_set_vx1(pConf);
			set_control_vbyone(pConf);
			init_vbyone_phy(pConf);
			set_tcon_vbyone(pConf);
			set_vx1_pinmux();
			break;
		default:
			printf("Invalid LCD type.\n");
			break;
	}
}

static inline void _disable_display_driver(Lcd_Config_t *pConf)
{
	int vclk_sel;

	vclk_sel = 0;//((pConf->lcd_timing.clk_ctrl) >>4) & 0x1;

	aml_set_reg32_bits(P_HHI_VIID_DIVIDER_CNTL, 0, 11, 1);	//close lvds phy clk gate: 0x104c[11]

	aml_write_reg32(P_ENCT_VIDEO_EN, 0);	//disable enct
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);	//disable encl

	if (vclk_sel)
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 0, 5);		//close vclk2 gate: 0x104b[4:0]
	else
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 0, 5);		//close vclk1 gate: 0x105f[4:0]

	printf("disable lcd display driver.\n");
}

static inline void _enable_vsync_interrupt(void)
{
	if ((aml_read_reg32(P_ENCT_VIDEO_EN) & 1) || (aml_read_reg32(P_ENCL_VIDEO_EN) & 1)) {
		aml_write_reg32(P_VENC_INTCTRL, 0x200);
	}else{
		aml_write_reg32(P_VENC_INTCTRL, 0x2);
	}
}

extern void mdelay(unsigned long msec);
static void _lcd_module_enable(void)
{
	BUG_ON(pDev==NULL);   
	panel_oper.power_on();//panel power 12v
	udelay(50);
	_init_display_driver(&pDev->conf);//TX_clock
	udelay(50);
	////pull up pwm
	aml_set_reg32_bits(P_PREG_PAD_GPIO1_O,0,30,1);
	aml_set_reg32_bits(P_PREG_PAD_GPIO1_EN_N,1,30,1);
	////pull up pwm
	mdelay(160);
	//_enable_backlight(BL_MAX_LEVEL);	//disable backlight at pannel init
	_enable_vsync_interrupt();
}

static int lcd_set_current_vmode(vmode_t mode)
{
	_lcd_module_enable();
	if (VMODE_INIT_NULL == pDev->lcd_info.mode)
		pDev->lcd_info.mode = VMODE_LVDS_1080P;
	return 0;
}

static void _init_vout(lcd_dev_t *pDev)
{
	pDev->lcd_info.name = PANEL_NAME;

	if((pDev->lcd_info.mode == VMODE_LVDS_1080P)||
		(pDev->lcd_info.mode == VMODE_LVDS_1080P_50HZ))
	{
		pDev->lcd_info.width = pDev->conf.lcd_basic.h_active;
		pDev->lcd_info.height = pDev->conf.lcd_basic.v_active;
		pDev->lcd_info.field_height = pDev->conf.lcd_basic.v_active;
		pDev->lcd_info.aspect_ratio_num = pDev->conf.lcd_basic.screen_ratio_width;
		pDev->lcd_info.aspect_ratio_den = pDev->conf.lcd_basic.screen_ratio_height;
		pDev->lcd_info.screen_real_width= pDev->conf.lcd_basic.screen_actual_width;
		pDev->lcd_info.screen_real_height= pDev->conf.lcd_basic.screen_actual_height;
		pDev->lcd_info.sync_duration_num = pDev->conf.lcd_timing.sync_duration_num;
		pDev->lcd_info.sync_duration_den = pDev->conf.lcd_timing.sync_duration_den;
		pDev->conf.lcd_basic.lcd_type = LCD_DIGITAL_LVDS;
	}
	else if(pDev->lcd_info.mode == VMODE_VX1_4K2K_60HZ)
	{
		pDev->conf.lcd_basic.lcd_type = LCD_DIGITAL_VBYONE;
	}
}

static void _lcd_init(Lcd_Config_t *pConf)
{
	_init_vout(pDev);
	lcd_set_current_vmode(VMODE_LCD);
}

static vmode_t lcd_outputmode(void)
{
	char *mode = getenv("outputmode");

	if(!strcmp(mode,"lvds1080p")){
		return VMODE_LVDS_1080P;
	}else  if(!strcmp(mode,"lvds1080p50hz")){
	        return VMODE_LVDS_1080P_50HZ;
	}else  if(!strcmp(mode,"vx14k2k60hz")){
		return VMODE_VX1_4K2K_60HZ;
	}else{
		printf("the output mode is not support!\n");
		return VMODE_INIT_NULL;
	}
}

int lcd_probe(void)
{
	pDev = (lcd_dev_t *)malloc(sizeof(lcd_dev_t));
	if (!pDev) {
		printf("[tcon]: Not enough memory.\n");
		return -1;
	}
	pDev->lcd_info.mode =  lcd_outputmode();
	pDev->conf = lcd_config;
	_lcd_init(&pDev->conf);
	return 0;
}

int lcd_remove(void)
{
	_disable_display_driver(&pDev->conf);
	free(pDev);
	return 0;
}

