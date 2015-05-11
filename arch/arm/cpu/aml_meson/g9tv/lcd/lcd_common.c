#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include "lcd_common.h"

int set_hpll_pll(int hpll_clk, int hpll_od,int hdmi_pll_cntl5)
{
//===============================
//out off pll clock
//===============================
	aml_write_reg32(P_HHI_HDMI_PLL_CNTL, hpll_clk);
	aml_write_reg32(P_HHI_HDMI_PLL_CNTL2,hpll_od);
	aml_write_reg32(P_HHI_HDMI_PLL_CNTL5,hdmi_pll_cntl5);

	aml_write_reg32(P_HHI_HDMI_PLL_CNTL, hpll_clk & (~(1<<28)));
	//aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2,od1,18,2); //no used ,bit[18:19]

	printf("Wait 10us for phy_clk stable!\n");
	udelay(10); // delay 10uS to wait clock is stable

	return 0;
}


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

	switch (div_sel) {
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

	if (shift_val == 0xffff ) {      // if divide by 1
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

void set_crt_video_enc(int vIdx, int inSel, int DivN)
{
	if (vIdx == 0) //V1
	{
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0
		udelay(2);
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, inSel,   16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VID_CLK_DIV, (DivN-1), 0, 8); // [7:0]   - cntl_xd0
		udelay(5);
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 1, 19, 1);//[19] -enable clk_div0
	} else { //V2
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0
		udelay(2);
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, inSel,  16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, (DivN-1),0, 8); // [7:0]   - cntl_xd0
		udelay(5);
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0
	}
	udelay(5);
}

void enable_crt_video_encl(int enable, int inSel)
{
	aml_set_reg32_bits(P_HHI_VIID_CLK_DIV,inSel,  12, 4); //encl_clk_sel:hi_viid_clk_div[15:12]

	if (inSel <= 4) //V1
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
	if (vd1_or_vd2_or_post == 0) { //vd1
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 5, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 8, 2);
	}else if (vd1_or_vd2_or_post == 1) { //vd2
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 4, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 2, 8, 2);
	}else{
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 1, 0, 1);
		aml_set_reg32_bits (P_VPP_MATRIX_CTRL, 0, 8, 2);
		if (mode == 0) {
			aml_set_reg32_bits(P_VPP_MATRIX_CTRL, 1, 1, 2);
		}else if (mode == 1) {
			aml_set_reg32_bits(P_VPP_MATRIX_CTRL, 0, 1, 2);
		}
	}

	if (mode == 0) { //ycbcr not full range, 601 conversion
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
		//	1	0			1.402
		//	1	-0.34414	-0.71414
		//	1	1.772		0
		aml_write_reg32(P_VPP_MATRIX_COEF00_01, (0x400 << 16) |0);
		aml_write_reg32(P_VPP_MATRIX_COEF02_10, (0x59c << 16) |0x400);
		aml_write_reg32(P_VPP_MATRIX_COEF11_12, (0x1ea0 << 16) |0x1d25);
		aml_write_reg32(P_VPP_MATRIX_COEF20_21, (0x400 << 16) |0x717);
		aml_write_reg32(P_VPP_MATRIX_COEF22, 0x0);
		aml_write_reg32(P_VPP_MATRIX_OFFSET0_1, 0x0);
		aml_write_reg32(P_VPP_MATRIX_OFFSET2, 0x0);
	}
}
