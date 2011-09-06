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
#include <asm/arch/io.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/tcon.h>

 
#define BL_MAX_LEVEL 0x100
//#define PANEL_NAME	"panel"
static tcon_conf_t *ptcon_conf = NULL;

static void _tcon_init(void) ;
static void set_lcd_gamma_table(u16 *data, u32 rgb_mask)
{
    int i;

    while (!(READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x0 << HADR));
    for (i=0;i<256;i++) {
        while (!( READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        WRITE_MPEG_REG(GAMMA_DATA_PORT, data[i]);
    }
    while (!(READ_MPEG_REG(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
    WRITE_MPEG_REG(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                                    (0x1 << rgb_mask)   |
                                    (0x23 << HADR));
}

static inline void _init_tcon(tcon_conf_t *ptcon_conf)
{
   set_lcd_gamma_table(ptcon_conf->GammaTableR, H_SEL_R);
    set_lcd_gamma_table(ptcon_conf->GammaTableG, H_SEL_G);
    set_lcd_gamma_table(ptcon_conf->GammaTableB, H_SEL_B);

    WRITE_MPEG_REG(GAMMA_CNTL_PORT, ptcon_conf->gamma_cntl_port);
    WRITE_MPEG_REG(GAMMA_VCOM_HSWITCH_ADDR, ptcon_conf->gamma_vcom_hswitch_addr);

    WRITE_MPEG_REG(RGB_BASE_ADDR,   ptcon_conf->rgb_base_addr);
    WRITE_MPEG_REG(RGB_COEFF_ADDR,  ptcon_conf->rgb_coeff_addr);
    WRITE_MPEG_REG(POL_CNTL_ADDR,   ptcon_conf->pol_cntl_addr);
    WRITE_MPEG_REG(DITH_CNTL_ADDR,  ptcon_conf->dith_cntl_addr);

    WRITE_MPEG_REG(STH1_HS_ADDR,    ptcon_conf->sth1_hs_addr);
    WRITE_MPEG_REG(STH1_HE_ADDR,    ptcon_conf->sth1_he_addr);
    WRITE_MPEG_REG(STH1_VS_ADDR,    ptcon_conf->sth1_vs_addr);
    WRITE_MPEG_REG(STH1_VE_ADDR,    ptcon_conf->sth1_ve_addr);

    WRITE_MPEG_REG(STH2_HS_ADDR,    ptcon_conf->sth2_hs_addr);
    WRITE_MPEG_REG(STH2_HE_ADDR,    ptcon_conf->sth2_he_addr);
    WRITE_MPEG_REG(STH2_VS_ADDR,    ptcon_conf->sth2_vs_addr);
    WRITE_MPEG_REG(STH2_VE_ADDR,    ptcon_conf->sth2_ve_addr);

    WRITE_MPEG_REG(OEH_HS_ADDR,     ptcon_conf->oeh_hs_addr);
    WRITE_MPEG_REG(OEH_HE_ADDR,     ptcon_conf->oeh_he_addr);
    WRITE_MPEG_REG(OEH_VS_ADDR,     ptcon_conf->oeh_vs_addr);
    WRITE_MPEG_REG(OEH_VE_ADDR,     ptcon_conf->oeh_ve_addr);

    WRITE_MPEG_REG(VCOM_HSWITCH_ADDR, ptcon_conf->vcom_hswitch_addr);
    WRITE_MPEG_REG(VCOM_VS_ADDR,    ptcon_conf->vcom_vs_addr);
    WRITE_MPEG_REG(VCOM_VE_ADDR,    ptcon_conf->vcom_ve_addr);

    WRITE_MPEG_REG(CPV1_HS_ADDR,    ptcon_conf->cpv1_hs_addr);
    WRITE_MPEG_REG(CPV1_HE_ADDR,    ptcon_conf->cpv1_he_addr);
    WRITE_MPEG_REG(CPV1_VS_ADDR,    ptcon_conf->cpv1_vs_addr);
    WRITE_MPEG_REG(CPV1_VE_ADDR,    ptcon_conf->cpv1_ve_addr);

    WRITE_MPEG_REG(CPV2_HS_ADDR,    ptcon_conf->cpv2_hs_addr);
    WRITE_MPEG_REG(CPV2_HE_ADDR,    ptcon_conf->cpv2_he_addr);
    WRITE_MPEG_REG(CPV2_VS_ADDR,    ptcon_conf->cpv2_vs_addr);
    WRITE_MPEG_REG(CPV2_VE_ADDR,    ptcon_conf->cpv2_ve_addr);

    WRITE_MPEG_REG(STV1_HS_ADDR,    ptcon_conf->stv1_hs_addr);
    WRITE_MPEG_REG(STV1_HE_ADDR,    ptcon_conf->stv1_he_addr);
    WRITE_MPEG_REG(STV1_VS_ADDR,    ptcon_conf->stv1_vs_addr);
    WRITE_MPEG_REG(STV1_VE_ADDR,    ptcon_conf->stv1_ve_addr);

    WRITE_MPEG_REG(STV2_HS_ADDR,    ptcon_conf->stv2_hs_addr);
    WRITE_MPEG_REG(STV2_HE_ADDR,    ptcon_conf->stv2_he_addr);
    WRITE_MPEG_REG(STV2_VS_ADDR,    ptcon_conf->stv2_vs_addr);
    WRITE_MPEG_REG(STV2_VE_ADDR,    ptcon_conf->stv2_ve_addr);

    WRITE_MPEG_REG(OEV1_HS_ADDR,    ptcon_conf->oev1_hs_addr);
    WRITE_MPEG_REG(OEV1_HE_ADDR,    ptcon_conf->oev1_he_addr);
    WRITE_MPEG_REG(OEV1_VS_ADDR,    ptcon_conf->oev1_vs_addr);
    WRITE_MPEG_REG(OEV1_VE_ADDR,    ptcon_conf->oev1_ve_addr);

    WRITE_MPEG_REG(OEV2_HS_ADDR,    ptcon_conf->oev2_hs_addr);
    WRITE_MPEG_REG(OEV2_HE_ADDR,    ptcon_conf->oev2_he_addr);
    WRITE_MPEG_REG(OEV2_VS_ADDR,    ptcon_conf->oev2_vs_addr);
    WRITE_MPEG_REG(OEV2_VE_ADDR,    ptcon_conf->oev2_ve_addr);

    WRITE_MPEG_REG(OEV3_HS_ADDR,    ptcon_conf->oev3_hs_addr);
    WRITE_MPEG_REG(OEV3_HE_ADDR,    ptcon_conf->oev3_he_addr);
    WRITE_MPEG_REG(OEV3_VS_ADDR,    ptcon_conf->oev3_vs_addr);
    WRITE_MPEG_REG(OEV3_VE_ADDR,    ptcon_conf->oev3_ve_addr);

    WRITE_MPEG_REG(INV_CNT_ADDR,    ptcon_conf->inv_cnt_addr);
    WRITE_MPEG_REG(TCON_MISC_SEL_ADDR, 	ptcon_conf->tcon_misc_sel_addr);
    WRITE_MPEG_REG(DUAL_PORT_CNTL_ADDR, ptcon_conf->dual_port_cntl_addr);

    CLEAR_MPEG_REG_MASK(VPP_MISC, VPP_OUT_SATURATE);
}

//static void vclk_set_lcd( int pll_sel, int pll_div_sel, int vclk_sel, 
 //                  unsigned long pll_reg, unsigned long vid_div_reg, unsigned int xd)
static void vclk_set_lcd(tcon_conf_t *ptcon_conf) 
{
	int pll_sel = ptcon_conf->pll_sel;
	int pll_div_sel = ptcon_conf->pll_div_sel;
	int vclk_sel = ptcon_conf->clk_sel;
	unsigned long pll_reg = ptcon_conf->pll_ctrl;
	unsigned long vid_div_reg = ptcon_conf->div_ctrl;
	unsigned int xd = ptcon_conf->clk_div;	
	
    vid_div_reg |= (1 << 16) ; // turn clock gate on
    vid_div_reg |= (pll_sel << 15); // vid_div_clk_sel
    
   
    if(vclk_sel) {
      WRITE_MPEG_REG( HHI_VIID_CLK_CNTL, READ_MPEG_REG(HHI_VIID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0 
    }
    else {
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) & ~(1 << 19) );     //disable clk_div0 
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) & ~(1 << 20) );     //disable clk_div1 
    } 

    // delay 2uS to allow the sync mux to switch over
    WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 2 ) {}    


    if(pll_sel){
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL, pll_reg|(1<<30) );
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL2, 0x65e31ff );
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL3, 0x9649a941 );
        WRITE_MPEG_REG( HHI_VIID_PLL_CNTL, pll_reg );
    }    
    else{
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL, pll_reg|(1<<30) );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL2, 0x65e31ff );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL3, 0x9649a941 );
        WRITE_MPEG_REG( HHI_VID_PLL_CNTL, pll_reg );
    }
    if(pll_div_sel ) WRITE_MPEG_REG( HHI_VIID_DIVIDER_CNTL,   vid_div_reg);
    else WRITE_MPEG_REG( HHI_VID_DIVIDER_CNTL,   vid_div_reg);

    if(vclk_sel) WRITE_MPEG_REG( HHI_VIID_CLK_DIV, (READ_MPEG_REG(HHI_VIID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value
    else WRITE_MPEG_REG( HHI_VID_CLK_DIV, (READ_MPEG_REG(HHI_VID_CLK_DIV) & ~(0xFF << 0)) | (xd-1) );   // setup the XD divider value

    // delay 5uS
    WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 5 ) {}    

    if(vclk_sel) {
      if(pll_div_sel) WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      WRITE_MPEG_REG( HHI_VIID_CLK_CNTL, READ_MPEG_REG(HHI_VIID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0 
    }
    else {
      if(pll_div_sel) WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
      else WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 0, 16, 3);  // Bit[18:16] - cntl_clk_in_sel
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) |  (1 << 19) );     //enable clk_div0 
      WRITE_MPEG_REG( HHI_VID_CLK_CNTL, READ_MPEG_REG(HHI_VID_CLK_CNTL) |  (1 << 20) );     //enable clk_div1 
    }
    // delay 2uS

    WRITE_MPEG_REG( ISA_TIMERE, 0); while( READ_MPEG_REG(ISA_TIMERE) < 2 ) {}    

    // set tcon_clko setting
    WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 
                    (
                    (0 << 11) |     //clk_div1_sel
                    (1 << 10) |     //clk_inv
                    (0 << 9)  |     //neg_edge_sel
                    (0 << 5)  |     //tcon high_thresh
                    (0 << 1)  |     //tcon low_thresh
                    (1 << 0)        //cntl_clk_en1
                    ), 
                    20, 12);

    if(vclk_sel) {
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 1, 15, 1);  //soft reset
      WRITE_MPEG_REG_BITS (HHI_VIID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    else {
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 
                   (1<<0),  // Enable cntl_div1_en
                   0, 1    // cntl_div1_en
                   );
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 1, 15, 1);  //soft reset
      WRITE_MPEG_REG_BITS (HHI_VID_CLK_CNTL, 0, 15, 1);  //release soft reset
    }
    
//    printf("video pl1 clk = %d\n", clk_util_clk_msr(VID_PLL_CLK));
//    printf("video pll2 clk = %d\n", clk_util_clk_msr(VID2_PLL_CLK));
//    printf("cts_enct clk = %d\n", clk_util_clk_msr(CTS_ENCT_CLK));
}

static void venc_set_lcd(tcon_conf_t *ptcon_conf, int havon_begin)
{
//    printf("setup lcd tvencoder\n");
    WRITE_MPEG_REG(VPU_VIU_VENC_MUX_CTRL,
       (3<<0) |    // viu1 select enct
       (3<<2)      // viu2 select enct
    );
    WRITE_MPEG_REG(ENCT_VIDEO_MODE,        0);
    WRITE_MPEG_REG(ENCT_VIDEO_MODE_ADV,    0x0418);	

    WRITE_MPEG_REG(ENCT_VIDEO_MAX_PXCNT,    ptcon_conf->max_width - 1);
    WRITE_MPEG_REG(ENCT_VIDEO_MAX_LNCNT,    ptcon_conf->max_height - 1);

    WRITE_MPEG_REG(ENCT_VIDEO_HAVON_BEGIN,  havon_begin);
    WRITE_MPEG_REG(ENCT_VIDEO_HAVON_END,    ptcon_conf->width - 1 + havon_begin );
    WRITE_MPEG_REG(ENCT_VIDEO_VAVON_BLINE,  ptcon_conf->video_on_line);
    WRITE_MPEG_REG(ENCT_VIDEO_VAVON_ELINE,  ptcon_conf->height + 3  + ptcon_conf->video_on_line);

    WRITE_MPEG_REG(ENCT_VIDEO_HSO_BEGIN,    15);
    WRITE_MPEG_REG(ENCT_VIDEO_HSO_END,      31);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_BEGIN,    15);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_END,      31);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_BLINE,    0);
    WRITE_MPEG_REG(ENCT_VIDEO_VSO_ELINE,    2);

    // bypass filter
    WRITE_MPEG_REG(ENCT_VIDEO_FILT_CTRL,    0x1000);
    
    // enable enct
    WRITE_MPEG_REG(ENCT_VIDEO_EN,           1);

}

static inline void _init_tvenc(tcon_conf_t *ptcon_conf)
{   
	//vclk_set_lcd(1, 0, 0, 0x1021e, 0x0018803, 9); // TODO change to use ptcon_conf
	vclk_set_lcd(ptcon_conf); // TODO change to use ptcon_conf
    venc_set_lcd(ptcon_conf, 48);
 }

static inline void _enable_vsync_interrupt(void)
{
    if (READ_MPEG_REG(ENCP_VIDEO_EN) & 1) {
        WRITE_MPEG_REG(VENC_INTCTRL, 0x200);
    }
    else{
        WRITE_MPEG_REG(VENC_INTCTRL, 0x2);
    }
}

static void _enable_backlight(u32 brightness_level)
{
	u32 l = brightness_level;
	
	if (l > BL_MAX_LEVEL)
		l = BL_MAX_LEVEL;
		
    WRITE_MPEG_REG(LCD_PWM0_LO_ADDR, BL_MAX_LEVEL - l);
    WRITE_MPEG_REG(LCD_PWM0_HI_ADDR, l);
}
static void _lcd_module_enable(void)
{
	BUG_ON(ptcon_conf==NULL);
	ptcon_conf->power_on?ptcon_conf->power_on():0;
	_init_tvenc(ptcon_conf);
    _init_tcon(ptcon_conf);
   	_enable_backlight(BL_MAX_LEVEL);
    _enable_vsync_interrupt();
}

static int lcd_set_current_vmode(vmode_t mode)	//ÉèÖÃÆÁ¿í
{	
	if (mode != VMODE_LCD)
		return -1;
	BUG_ON(ptcon_conf==NULL);
	WRITE_MPEG_REG(VPP_POSTBLEND_H_SIZE, ptcon_conf->width);
	_lcd_module_enable();
	return 0;
}

/*static vmode_t lcd_validate_vmode(char *mode)
{
	if ((strncmp(mode, PANEL_NAME, strlen(PANEL_NAME))) == 0)
		return VMODE_LCD;
	
	return VMODE_MAX;
}*/
/*static int lcd_vmode_is_supported(vmode_t mode)
{
	if(mode == VMODE_LCD)
	    return 1;
	return 0;
}
static int lcd_module_disable(vmode_t cur_vmod)
{
	BUG_ON(ptcon_conf==NULL);
	ptcon_conf->power_off?ptcon_conf->power_off():0;
	return 0;
}*/

/*static void _init_vout(tcon_dev_t *pDev)
{
	pDev->lcd_info.name = PANEL_NAME;
	pDev->lcd_info.mode = VMODE_LCD;
	pDev->lcd_info.width = pDev->conf.width;
	pDev->lcd_info.height = pDev->conf.height;
	pDev->lcd_info.field_height = pDev->conf.height;
	pDev->lcd_info.aspect_ratio_num = pDev->conf.screen_width;
	pDev->lcd_info.aspect_ratio_den = pDev->conf.screen_height;
	pDev->lcd_info.sync_duration_num = pDev->conf.sync_duration_num;
	pDev->lcd_info.sync_duration_den = pDev->conf.sync_duration_den;
}*/

static void _tcon_init(void)
{
	//_init_vout(pDev);
    _lcd_module_enable();
	lcd_set_current_vmode(VMODE_LCD);
}

int tcon_probe(tcon_conf_t *tcon_cfg)
{
/*    pDev = (tcon_dev_t *)malloc(sizeof(tcon_dev_t));
    if (!pDev) {
        printf("[tcon]: Not enough memory.\n");
        return -1;
    }

    pDev->conf = tcon_cfg;
*/
	ptcon_conf = tcon_cfg;
    _tcon_init();
    return 0;
}

int tcon_remove(void)
{
/*	if(pDev == NULL)
	{
		return 1;
	}
	free(pDev);*/
	ptcon_conf=NULL;
    return 0;
}
