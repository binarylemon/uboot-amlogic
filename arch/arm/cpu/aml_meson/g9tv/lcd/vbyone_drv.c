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
void set_vbyone_pinmux(void)
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
    aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL, 3);
    aml_write_reg32(P_L_RGB_BASE_ADDR, 0);
    aml_write_reg32(P_L_RGB_COEFF_ADDR, 0x400);
    aml_write_reg32(P_VPP_MISC, aml_read_reg32(P_VPP_MISC) & ~(VPP_OUT_SATURATE));
}

static void init_vbyone_phy(Lcd_Config_t *pConf)
{
	aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);

	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6e0ec918);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x00000a7c);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x00ff0800);
	//od   clk 2970 / 5 = 594
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0xfff00c0);
	//clear lvds fifo od (/2)
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1, 0xc1000000);
}

int config_vbyone(int lane, int byte, int region, int hsize, int vsize)
{
   int sublane_num;
   int region_size[4];
   int tmp;

   if ((lane == 0) || (lane == 3) || (lane == 5) || (lane == 6) || (lane == 7) || (lane>8))
      return 1;
   if ((region == 0) || (region== 3) || (region>4))
      return 1;
   if (lane%region)
      return 1;
   if ((byte<3) || (byte>4))
      return 1;

   sublane_num = lane/region;
   aml_set_reg32_bits(P_VBO_LANES,lane-1,  VBO_LANE_NUM_BIT,    VBO_LANE_NUM_WID);
   aml_set_reg32_bits(P_VBO_LANES,region-1,VBO_LANE_REGION_BIT, VBO_LANE_REGION_WID);
   aml_set_reg32_bits(P_VBO_LANES,sublane_num-1,VBO_SUBLANE_NUM_BIT, VBO_SUBLANE_NUM_WID);
   aml_set_reg32_bits(P_VBO_LANES,byte-1,VBO_BYTE_MODE_BIT, VBO_BYTE_MODE_WID);

   if (region>1) {
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

static void set_control_vbyone(Lcd_Config_t *pConf)
{
    int lane, byte, region,hsize, vsize,color_fmt;
    int vin_color, vin_bpp;

    hsize = pConf->lcd_basic.h_active;
	vsize = pConf->lcd_basic.v_active;
    lane = pConf->lcd_control.vbyone_config->lane_count;		//lane;8
	byte = pConf->lcd_control.vbyone_config->byte;		       //byte;4
	region = pConf->lcd_control.vbyone_config->region;		//region_num;2
	color_fmt = pConf->lcd_control.vbyone_config->color_fmt;  //color_fmt;4

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
            printf( "Error VBYONE_COLOR_FORMAT!\n");
                  return;
	}
    // clock seting for VX1
    //vclk_set_encl_vx1(vfromat, lane, byte);

    // set encl format
    //set_tv_encl (TV_ENC_LCD3840x2160p_vic03,1,0,0);

    // vpu clock setting
    //aml_set_reg32(P_HHI_VPU_CLK_CNTL,   (0 << 9)    |   // vpu   clk_sel
    //                       						 (0 << 0) );     // vpu   clk_div
    //aml_set_reg32(P_HHI_VPU_CLK_CNTL, (Rd(HHI_VPU_CLK_CNTL) | (1 << 8)) );

    //PIN_MUX for VX1 need to add this to dtd

   // printf("Set VbyOne PIN MUX ......\n");
   // aml_set_reg32_bits(P_PERIPHS_PIN_MUX_3,3,8,2);

    // set Vbyone
    printf("VbyOne Configuration ......\n");
    //set_vbyone_vfmt(vin_color,vin_bpp);
    aml_set_reg32_bits(P_VBO_VIN_CTRL, vin_color, VBO_VIN_PACK_BIT,VBO_VIN_PACK_WID);
    aml_set_reg32_bits(P_VBO_VIN_CTRL, vin_bpp,   VBO_VIN_BPP_BIT,VBO_VIN_BPP_WID);

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

	printf("VbyOne is In Normal Status ......\n");
}

static void venc_set_vbyone(Lcd_Config_t *pConf)
{
	printf("%s\n", __FUNCTION__);
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);

	//int havon_begin = 80;
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

	aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL,	0);
	// enable encl
	aml_write_reg32(P_ENCL_VIDEO_EN, 1);
}

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
		printf("VX1 cannot support this configuration!\n");
		return;
	}

	phy_div = lane/minlane; //1,2,4,8
	if (phy_div == 8) {
		phy_div = phy_div/2;
		//if(hdmi_vx1_clk_od1 != 0) {
		//	printf("VX1 cannot support this configuration!\n");
		//	return;
		//}
    //	hdmi_vx1_clk_od1=1;
	}
	//need check whether we need to set this dpll !!!!!!!
	//if (set_hdmi_dpll(hdmi_clk_out,hdmi_vx1_clk_od1)){
    //   printf("Unsupported HDMI_DPLL out frequency!\n");
    //    return;
    //}

   pclk_div = (((byte == 3) ? 30:40)*100)/minlane;
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

unsigned int vbyone_init(Lcd_Config_t *pConf)
{
	printf("vx1 mode is selected!\n");
	set_pll_vbyone(pConf);
	venc_set_vbyone(pConf);
	set_control_vbyone(pConf);
	init_vbyone_phy(pConf);
	set_tcon_vbyone(pConf);
	set_vbyone_pinmux();

	return 0;
}

