#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/lcdoutc.h>
#include <asm/arch/vinfo.h>

#include "lcd_common.h"

static void set_tcon_lvds(Lcd_Config_t *pConf)
{
	vpp_set_matrix_ycbcr2rgb(2, 0);
	aml_write_reg32(P_L_RGB_BASE_ADDR, 0);
	aml_write_reg32(P_L_RGB_COEFF_ADDR, 0x400);

	if (pConf->lcd_control.lvds_config->lvds_bits == 8)
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x400);
	else if (pConf->lcd_control.lvds_config->lvds_bits == 6)
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0x600);
	else
		aml_write_reg32(P_L_DITH_CNTL_ADDR,  0);

	aml_write_reg32(P_VPP_MISC, aml_read_reg32(P_VPP_MISC) & ~(VPP_OUT_SATURATE));
}

static void init_lvds_phy(Lcd_Config_t *pConf)
{
	// aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	// aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	// aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);

	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6c6cca80);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x0000006c);
	aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x0fff0800);
	//od   clk 1039.5 / 2 = 519.75 = 74.25*7
	//aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0x0fff0040);
}

static void set_lvds_clk_util(Lcd_Config_t *pConf)
{
	unsigned int fifo_mode, phy_div;

	if (pConf->lcd_control.lvds_config->dual_port) {
		fifo_mode = 0x3;
		phy_div = 2;
	} else {
		fifo_mode = 0x1;
		phy_div = 1;
	}

	aml_write_reg32(P_LVDS_GEN_CNTL, (aml_read_reg32(P_LVDS_GEN_CNTL)| (1 << 4) | (fifo_mode << 0)));
	aml_set_reg32_bits(P_LVDS_GEN_CNTL, 1, 3, 1);

	/* set fifo_clk_sel: div 7 */
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, (1 << 6));
	/* set cntl_ser_en:  8-channel to 1 */
	aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL0, 0xfff, 16, 12);

	/* decoupling fifo enable, gated clock enable */
	aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL1,
		(1 << 30) | ((phy_div - 1) << 25) | (1 << 24));
	/* decoupling fifo write enable after fifo enable */
	aml_set_reg32_bits(P_HHI_LVDS_TX_PHY_CNTL1, 1, 31, 1);
}

static void set_control_lvds(Lcd_Config_t *pConf)
{
	unsigned int bit_num = 1;
	unsigned int pn_swap = 0;
	unsigned int dual_port = 1;
	unsigned int lvds_repack = 1;
	unsigned int port_swap = 0;

	set_lvds_clk_util(pConf);

	lvds_repack = (pConf->lcd_control.lvds_config->lvds_repack) & 0x1;
	pn_swap   = (pConf->lcd_control.lvds_config->pn_swap) & 0x1;
	dual_port = (pConf->lcd_control.lvds_config->dual_port) & 0x1;
	port_swap = (pConf->lcd_control.lvds_config->port_swap) & 0x1;

	switch (pConf->lcd_control.lvds_config->lvds_bits) {
	case 10:
		bit_num=0;
		break;
	case 8:
		bit_num=1;
		break;
	case 6:
		bit_num=2;
		break;
	case 4:
		bit_num=3;
		break;
	default:
		bit_num=1;
		break;
	}

	aml_write_reg32(P_LVDS_PACK_CNTL_ADDR,
			( lvds_repack<<0 ) | // repack
			( port_swap<<2) | // odd_even
			( 0<<3 ) |		// reserve
			( 0<<4 ) |		// lsb first
			( pn_swap<<5 ) |	// pn swap
			( dual_port<<6 ) |	// dual port
			( 0<<7 ) |		// use tcon control
			( bit_num<<8 ) |	// 0:10bits, 1:8bits, 2:6bits, 3:4bits.
			( 0<<10 ) |		//r_select  //0:R, 1:G, 2:B, 3:0
			( 1<<12 ) |		//g_select  //0:R, 1:G, 2:B, 3:0
			( 2<<14 ));		//b_select  //0:R, 1:G, 2:B, 3:0;
}

static void venc_set_lvds(Lcd_Config_t *pConf)
{
//	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL, (0<<0)|(0<<2));     // viu1 select encl  | viu2 select encl
	aml_write_reg32(P_ENCL_VIDEO_EN, 0);
	//int havon_begin = 80;
	aml_write_reg32(P_VPU_VIU_VENC_MUX_CTRL, (0<<0)|(0<<2) );    // viu1 select encl | viu2 select encl
	aml_write_reg32(P_ENCL_VIDEO_MODE, 0); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	aml_write_reg32(P_ENCL_VIDEO_MODE_ADV, 0x0418); // Sampling rate: 1

	// bypass filter
	aml_write_reg32(P_ENCL_VIDEO_FILT_CTRL, 	0x1000);
	aml_write_reg32(P_ENCL_VIDEO_MAX_PXCNT, 	pConf->lcd_basic.h_period - 1);
	aml_write_reg32(P_ENCL_VIDEO_MAX_LNCNT,		pConf->lcd_basic.v_period - 1);
	aml_write_reg32(P_ENCL_VIDEO_HAVON_BEGIN,	pConf->lcd_basic.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_HAVON_END,		pConf->lcd_basic.h_active - 1 + pConf->lcd_basic.video_on_pixel);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_BLINE,	pConf->lcd_basic.video_on_line);
	aml_write_reg32(P_ENCL_VIDEO_VAVON_ELINE,	pConf->lcd_basic.v_active - 1  + pConf->lcd_basic.video_on_line);

	aml_write_reg32(P_ENCL_VIDEO_HSO_BEGIN,		pConf->lcd_timing.sth1_hs_addr);
	aml_write_reg32(P_ENCL_VIDEO_HSO_END,		pConf->lcd_timing.sth1_he_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BEGIN,		pConf->lcd_timing.stv1_hs_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_END,		pConf->lcd_timing.stv1_he_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_BLINE,		pConf->lcd_timing.stv1_vs_addr);
	aml_write_reg32(P_ENCL_VIDEO_VSO_ELINE,		pConf->lcd_timing.stv1_ve_addr);
	aml_write_reg32(P_ENCL_VIDEO_RGBIN_CTRL,    3);

	// enable encl
	aml_write_reg32(P_ENCL_VIDEO_EN, 1);
}

#if 0
//new lvd_vx1_phy config
static void lvds_phy_config(int lvds_vx1_sel)
{
	aml_write_reg32(P_VPU_VLOCK_GCLK_EN, 7);
	aml_write_reg32(P_VPU_VLOCK_ADJ_EN_SYNC_CTRL, 0x108010ff);
	aml_write_reg32(P_VPU_VLOCK_CTRL, 0xe0f50f1b);

	if (lvds_vx1_sel == 0) { //lvds
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL1, 0x6c6cca80);
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL2, 0x0000006c);
		aml_write_reg32(P_HHI_DIF_CSI_PHY_CNTL3, 0x00000800);
		//od   clk 1039.5 / 2 = 519.75 = 74.25*7
		aml_write_reg32(P_HHI_LVDS_TX_PHY_CNTL0, 0x0fff0040);
	}
}
#endif

static void set_clk_lvds(Lcd_Config_t *pConf)
{
	//unsigned int sw_port   = (pConf->lcd_control.lvds_config->lvds_fifo_wr_mode)&0x100;
	//unsigned int lvds_fifo = (pConf->lcd_control.lvds_config->lvds_fifo_wr_mode)&0xff;

	if (pConf->lcd_timing.clk_auto) {
		generate_clk_parameter(pConf);
		set_vclk_lcd(pConf);
	} else {
		printf("lcd: lvds clk: hpll_clk=%x hpll_od%x \n",
			pConf->lcd_timing.hpll_clk, pConf->lcd_timing.hpll_od);
		set_hpll_pll(pConf->lcd_timing.hpll_clk, pConf->lcd_timing.hpll_od, pConf->lcd_timing.hdmi_pll_cntl5);
		clocks_set_vid_clk_div(CLK_DIV_SEL_7);
		set_crt_video_enc(0, 0, 1);  //configure crt_video V1: inSel=vid_pll_clk(0),DivN=xd)
		enable_crt_video_encl(1, 0); //select and enable the output
	}
	//vclk_set_encl_lvds(pConf->lcd_control.lvds_config->dual_port);
	// if ( sw_port == 0x100 )
		// aml_write_reg32( P_HHI_VIID_DIVIDER_CNTL, ((aml_read_reg32(P_HHI_VIID_DIVIDER_CNTL) & ~(0x7 << 8)) | (1 << 8) | (0<<10)));
	// else
		// aml_write_reg32( P_HHI_VIID_DIVIDER_CNTL, ((aml_read_reg32(P_HHI_VIID_DIVIDER_CNTL) & ~(0x7 << 8)) | (2 << 8) | (0<<10)));
	//aml_write_reg32(P_LVDS_GEN_CNTL, (aml_read_reg32(P_LVDS_GEN_CNTL)| (1 << 3) | (lvds_fifo << 0)));
}

unsigned int lvds_init(Lcd_Config_t *pConf)
{
	printf("lcd: lvds mode is selected\n");
	set_clk_lvds(pConf);
	venc_set_lvds(pConf);
	set_control_lvds(pConf);
	set_tcon_lvds(pConf);
	init_lvds_phy(pConf);

	return 0;
}
