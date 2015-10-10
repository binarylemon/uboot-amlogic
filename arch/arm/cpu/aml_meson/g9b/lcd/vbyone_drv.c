#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/lcdoutc.h>
#include <asm/arch/vinfo.h>

#include "lcd_common.h"

#define VBO_ENABLE_BIT  0
#define VBO_EBABLE_WID  1

#define VBO_CTL_MODE_BIT      11
#define VBO_CTL_MODE_WID      5

#define VBO_CTL_MODE2_BIT     0
#define VBO_CTL_MODE2_WID     4

#define VBO_VIN2ENC_HVSYNC_DLY_BIT  9
#define VBO_VIN2ENC_HVSYNC_DLY_WID  1

#define  VBO_LANE_NUM_BIT      0
#define  VBO_LANE_NUM_WID      3

#define  VBO_LANE_REGION_BIT   4
#define  VBO_LANE_REGION_WID   2

#define  VBO_SUBLANE_NUM_BIT   8
#define  VBO_SUBLANE_NUM_WID   3

#define  VBO_BYTE_MODE_BIT     11
#define  VBO_BYTE_MODE_WID     2

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

#define  VBO_PXL_CTR0_BIT     0
#define  VBO_PXL_CTR0_WID     4

//set VX1_LOCKN && VX1_HTPDN
static void set_vbyone_pinmux(void)
{
	aml_set_reg32_bits(P_PERIPHS_PIN_MUX_7, 3, 18, 2);
	aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_4, 1<<9);
	aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_3, 1<<26);
	aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_7, 1<<29);
	aml_clr_reg32_mask(P_PERIPHS_PIN_MUX_4, 1<<8);
}

static void set_tcon_vbyone(Lcd_Config_t *pConf)
{
	vpp_set_matrix_ycbcr2rgb(2, 0);
	aml_write_reg32(P_L_RGB_BASE_ADDR, 0);
	aml_write_reg32(P_L_RGB_COEFF_ADDR, 0x400);
	aml_write_reg32(P_VPP_MISC, aml_read_reg32(P_VPP_MISC) & ~(VPP_OUT_SATURATE));
}

static void init_vbyone_phy(Lcd_Config_t *pConf)
{
	//aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	//aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	//aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);

	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6e0ec918);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x00000a7c);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x00ff0800);
	//od   clk 2970 / 5 = 594
	//aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0xfff00c0);
	//clear lvds fifo od (/2)
	//aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1, 0xc1000000);
}

void set_vbyone_ctlbits(int p3d_en, int p3d_lr, int mode)
{
	if (mode == 0)  //insert at the first pixel
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

static void set_vbyone_clk_util(Lcd_Config_t *pConf)
{
	unsigned int lcd_bits;
	unsigned int div_sel, phy_div;

	lcd_bits = 10;
	switch (lcd_bits) {
	case 6:
		div_sel = 0;
		break;
	case 8:
		div_sel = 2;
		break;
	case 10:
		div_sel = 3;
		break;
	default:
		div_sel = 3;
		break;
	}
	/* set fifo_clk_sel */
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, (div_sel << 6));
	/* set cntl_ser_en:  8-channel to 1 */
	aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL0, 0xfff, 16, 12);

	phy_div = pConf->lcd_control.vbyone_config->phy_div;
	/* decoupling fifo enable, gated clock enable */
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1,
		(1 << 30) | ((phy_div - 1) << 25) | (1 << 24));
	/* decoupling fifo write enable after fifo enable */
	aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL1, 1, 31, 1);
}

static int set_vbyone_lanes(int lane_num, int byte_mode, int region_num,
		int hsize, int vsize)
{
	int sublane_num;
	int region_size[4];
	int tmp;

	switch (lane_num) {
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		return -1;
	}
	switch (region_num) {
	case 1:
	case 2:
	case 4:
		break;
	default:
		return -1;
	}
	if (lane_num % region_num)
		return -1;
	switch (byte_mode) {
	case 3:
	case 4:
		break;
	default:
		return -1;
	}
	printf("lcd: byte_mode=%d, lane_num=%d, region_num=%d\n",
		byte_mode, lane_num, region_num);

	sublane_num = lane_num / region_num; /* lane num in each region */
	aml_set_reg32_bits(P_VBO_LANES, lane_num - 1, VBO_LANE_NUM_BIT, 3);
	aml_set_reg32_bits(P_VBO_LANES, region_num - 1, VBO_LANE_REGION_BIT, 2);
	aml_set_reg32_bits(P_VBO_LANES, sublane_num - 1, VBO_SUBLANE_NUM_BIT, 3);
	aml_set_reg32_bits(P_VBO_LANES, byte_mode - 1, VBO_BYTE_MODE_BIT, 2);

	if (region_num > 1) {
		region_size[3] = (hsize / lane_num) * sublane_num;
		tmp = (hsize % lane_num);
		region_size[0] = region_size[3] + (((tmp / sublane_num) > 0) ?
			sublane_num : (tmp % sublane_num));
		region_size[1] = region_size[3] + (((tmp / sublane_num) > 1) ?
			sublane_num : (tmp % sublane_num));
		region_size[2] = region_size[3] + (((tmp / sublane_num) > 2) ?
			sublane_num : (tmp % sublane_num));
		aml_write_reg32(P_VBO_REGION_00, region_size[0]);
		aml_write_reg32(P_VBO_REGION_01, region_size[1]);
		aml_write_reg32(P_VBO_REGION_02, region_size[2]);
		aml_write_reg32(P_VBO_REGION_03, region_size[3]);
	}
	aml_write_reg32(P_VBO_ACT_VSIZE, vsize);
	//aml_set_reg32_bits(P_VBO_CTRL_H,0x80,VBO_CTL_MODE_BIT,VBO_CTL_MODE_WID);  // different from FBC code!!!
	aml_set_reg32_bits(P_VBO_CTRL_H, 0x0, VBO_CTL_MODE2_BIT, VBO_CTL_MODE2_WID); // different from simulation code!!!
	aml_set_reg32_bits(P_VBO_CTRL_H, 0x1, VBO_VIN2ENC_HVSYNC_DLY_BIT, VBO_VIN2ENC_HVSYNC_DLY_WID);
	//aml_set_reg32_bits(P_VBO_CTRL_L,enable,VBO_ENABLE_BIT,VBO_EBABLE_WID);

	return 0;
}

static void set_control_vbyone(Lcd_Config_t *pConf)
{
	int lane, byte, region, hsize, vsize, color_fmt;
	int vin_color, vin_bpp;

	hsize = pConf->lcd_basic.h_active;
	vsize = pConf->lcd_basic.v_active;
	lane = pConf->lcd_control.vbyone_config->lane_count; //lane;8
	byte = pConf->lcd_control.vbyone_config->byte;       //byte;4
	region = pConf->lcd_control.vbyone_config->region;   //region_num;2
	color_fmt = pConf->lcd_control.vbyone_config->color_fmt;  //color_fmt;4

	set_vbyone_clk_util(pConf);

	switch (color_fmt) {
	case 0://SDVT_VBYONE_18BPP_RGB
		vin_color = 4;
		vin_bpp   = 2;
		break;
	case 1://SDVT_VBYONE_18BPP_YCBCR444
		vin_color = 0;
		vin_bpp   = 2;
		break;
	case 2://SDVT_VBYONE_24BPP_RGB
		vin_color = 4;
		vin_bpp   = 1;
		break;
	case 3://SDVT_VBYONE_24BPP_YCBCR444
		vin_color = 0;
		vin_bpp   = 1;
		break;
	case 4://SDVT_VBYONE_30BPP_RGB
		vin_color = 4;
		vin_bpp   = 0;
		break;
	case 5://SDVT_VBYONE_30BPP_YCBCR444
		vin_color = 0;
		vin_bpp   = 0;
		break;
	default:
		printf("lcd error: vbyone COLOR_FORMAT unsupport\n");
		return;
	}
	// clock seting for VX1
	//vclk_set_encl_vx1(vfromat, lane, byte);

	//PIN_MUX for VX1 need to add this to dtd

	// printf("Set VbyOne PIN MUX ......\n");
	// aml_set_reg32_bits(P_PERIPHS_PIN_MUX_3,3,8,2);

	// set Vbyone
	//printf("lcd: VbyOne Configuration ......\n");
	//set_vbyone_vfmt(vin_color,vin_bpp);
	aml_set_reg32_bits(P_VBO_VIN_CTRL, vin_color, VBO_VIN_PACK_BIT,VBO_VIN_PACK_WID);
	aml_set_reg32_bits(P_VBO_VIN_CTRL, vin_bpp,   VBO_VIN_BPP_BIT,VBO_VIN_BPP_WID);

	set_vbyone_lanes(lane, byte, region, hsize, vsize);

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
	//aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL0,3,6,2);

	//PAD select:
	if ((lane == 1) || (lane == 2)) {
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
}

static void vbyone_wait_stable(void)
{
	int i = 1000;

	while ((aml_read_reg32(P_VBO_STATUS_L) & 0x3f) != 0x20) {
		udelay(5);
		if (i-- == 0)
			break;
	}
	printf("lcd: %s status: 0x%x\n", __func__, aml_read_reg32(P_VBO_STATUS_L));
}

static void venc_set_vbyone(Lcd_Config_t *pConf)
{
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);

	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL,	(0<<0)|(3<<2));	 // viu1 select encl  | viu2 select encl
	aml_write_reg32(P_ENCL_VIDEO_MODE,			40);	//0); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	aml_write_reg32(P_ENCL_VIDEO_MODE_ADV,		0x18);//0x0418); // Sampling rate: 1

	// bypass filter
	aml_write_reg32(P_ENCL_VIDEO_FILT_CTRL,		0x1000);

	aml_write_reg32(P_ENCL_VIDEO_MAX_PXCNT,		pConf->lcd_basic.h_period - 1);
	aml_write_reg32(P_ENCL_VIDEO_MAX_LNCNT,		pConf->lcd_basic.v_period - 1);

	aml_write_reg32(P_ENCL_VIDEO_HAVON_BEGIN, 	pConf->lcd_basic.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_HAVON_END, 	pConf->lcd_basic.h_active - 1 + pConf->lcd_basic.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_BLINE,	pConf->lcd_basic.video_on_line);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_ELINE,	pConf->lcd_basic.v_active - 1  + pConf->lcd_basic.video_on_line);

	aml_write_reg32(P_ENCL_VIDEO_HSO_BEGIN,		pConf->lcd_timing.sth1_hs_addr);
	aml_write_reg32(P_ENCL_VIDEO_HSO_END,		pConf->lcd_timing.sth1_he_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BEGIN, 	pConf->lcd_timing.stv1_hs_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_END,		pConf->lcd_timing.stv1_he_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BLINE, 	pConf->lcd_timing.stv1_vs_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_ELINE, 	pConf->lcd_timing.stv1_ve_addr);

	aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 3);
	// enable encl
	aml_write_reg32(P_ENCL_VIDEO_EN, 1);
}

#if 0
//clock seting for VX1
static void set_pll_vbyone(Lcd_Config_t *pConf)
{
	int  lane, byte;//vfromat,
	//int hdmi_clk_out;
	//int hdmi_vx1_clk_od1;
	int pclk_div;
	int phy_div;
	int xd = 0;
	int minlane;

	lane = pConf->lcd_control.vbyone_config->lane_count;//lane_num;8
	byte = pConf->lcd_control.vbyone_config->byte;//byte_num;4

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
		printf("lcd error: VX1 cannot support this configuration\n");
		return;
	}

	phy_div = lane/minlane; //1,2,4,8
	if (phy_div == 8) {
		phy_div = phy_div/2;
		//if(hdmi_vx1_clk_od1 != 0) {
		//	printf("VX1 cannot support this configuration!\n");
		//	return;
		//}
		// hdmi_vx1_clk_od1=1;
	}
	//need check whether we need to set this dpll !!!!!!!
	//if (set_hdmi_dpll(hdmi_clk_out,hdmi_vx1_clk_od1)){
	//   printf("Unsupported HDMI_DPLL out frequency!\n");
	//    return;
	//}

	pclk_div = (((byte == 3) ? 30:40)*100)/minlane;
	//printf("lcd: vbyone byte=%d, lane=%d, pclk=%d, phy_div=%d \n", byte, lane, pclk_div, phy_div);
	//configure vid_clk_div_top
	if (byte == 3) {
		if (pclk_div == 375) {
			clocks_set_vid_clk_div(CLK_DIV_SEL_3p75);
			xd = 1;
		} else if (pclk_div == 750) {
			clocks_set_vid_clk_div(CLK_DIV_SEL_7p5);
			xd = 1;
		} else {
			clocks_set_vid_clk_div(CLK_DIV_SEL_15);
			xd = pclk_div/100/15;
		}
	} else if (byte == 4) {
		clocks_set_vid_clk_div(CLK_DIV_SEL_5);
		xd = pclk_div/100/5;
	}

	//for lvds phy clock and enable decoupling FIFO
	//aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1,((3<<6)|((phy_div-1)<<1)|1)<<24);

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
#endif

static unsigned int vbyone_lane_num[] = {
	1,
	2,
	4,
	8,
	8,
};

#define VBYONE_BIT_RATE_MAX		2970 //MHz
#define VBYONE_BIT_RATE_MIN		600
static void set_vbyone_config(Lcd_Config_t *pConf)
{
	unsigned int band_width, bit_rate, pclk, phy_div;
	unsigned int byte_mode, lane_count, minlane;
	unsigned int lcd_bits;
	unsigned int temp, i;

	//auto calculate bandwidth, clock
	lane_count = pConf->lcd_control.vbyone_config->lane_count;
	lcd_bits = 10;
	byte_mode = (lcd_bits == 10) ? 4 : 3;
	/* byte_mode * byte2bit * 8/10_encoding * pclk =
	   byte_mode * 8 * 10 / 8 * pclk */
	pclk = pConf->lcd_timing.lcd_clk / 1000; /* kHz */
	band_width = byte_mode * 10 * pclk;

	temp = VBYONE_BIT_RATE_MAX * 1000;
	temp = (band_width + temp - 1) / temp;
	for (i = 0; i < 4; i++) {
		if (temp <= vbyone_lane_num[i])
			break;
	}
	minlane = vbyone_lane_num[i];
	if (lane_count < minlane) {
		printf("lcd error: vbyone lane_num(%d) is less than min requirement(%d)\n",
			lane_count, minlane);
		lane_count = minlane;
		pConf->lcd_control.vbyone_config->lane_count = lane_count;
		printf("lcd: change to min lane_num %d\n", minlane);
	}

	bit_rate = band_width / minlane;//band_width / lane_count;
	phy_div = lane_count / minlane;
	if (phy_div == 8) {
		phy_div/= 2;
		bit_rate /= 2;
	}
	if (bit_rate > (VBYONE_BIT_RATE_MAX * 1000)) {
		printf("lcd error: vbyone bit rate(%dKHz) is out of max range(%dKHz)\n",
			bit_rate, (VBYONE_BIT_RATE_MAX * 1000));
	}
	if (bit_rate < (VBYONE_BIT_RATE_MIN * 1000)) {
		printf("lcd error: vbyone bit rate(%dKHz) is out of min range(%dKHz)\n",
			bit_rate, (VBYONE_BIT_RATE_MIN * 1000));
	}
	bit_rate = bit_rate * 1000; /* Hz */

	pConf->lcd_control.vbyone_config->phy_div = phy_div;
	pConf->lcd_control.vbyone_config->bit_rate = bit_rate;
	//printf("lcd: lane_count=%u, bit_rate = %uMHz, pclk=%u.%03uMhz\n",
	//	lane_count, (bit_rate / 1000000), (pclk / 1000), (pclk % 1000));
}

#define VX1_PLL_ADJUST		1
static void set_clk_vbyone(Lcd_Config_t *pConf)
{
	if (pConf->lcd_timing.clk_auto) {
		generate_clk_parameter(pConf);
		set_vclk_lcd(pConf);
	}
#if 0
	set_pll_vbyone(pConf);
#endif
}

unsigned int vbyone_init(Lcd_Config_t *pConf)
{
	//printf("lcd: vx1 mode is selected\n");
	set_vbyone_config(pConf);
	set_clk_vbyone(pConf);
	venc_set_vbyone(pConf);
	set_control_vbyone(pConf);
	init_vbyone_phy(pConf);
	set_tcon_vbyone(pConf);
	set_vbyone_pinmux();
	vbyone_wait_stable();

	return 0;
}

