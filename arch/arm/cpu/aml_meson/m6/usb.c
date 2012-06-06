/******************************************************************************
 *
 *  Copyright (C) 2011 AMLOGIC, INC.
 *
 *
 * remark: copy from @ trunk/arch/arm/cpu/aml_meson/m3 @Rev2634
 *              by Haixiang.Bao 2011.10.17
 *              haixiang.bao@amlogic.com
 *		  change to M6 clock setting
 *		   by Victor Wan
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *********************************************************************************/
//#include <common.h>
//#include <asm/cache.h>
#include <asm/arch/usb.h>

#ifdef CONFIG_USB_DWC_OTG_HCD
static amlogic_usb_config_t * g_usb_cfg = 0;
static char * g_clock_src_name_m6[]={
    "XTAL input",
    "XTAL input divided by 2",
    "DDR PLL",
    "MPLL OUT0"
    "MPLL OUT1",
    "MPLL OUT2",
    "FCLK / 2",
    "FCLK / 3"
};

extern void udelay(unsigned long usec);

static int reset_count = 0;
//int set_usb_phy_clk(struct lm_device * plmdev,int is_enable)
//{
static int set_usb_phy_clock(amlogic_usb_config_t * usb_cfg)
{
	
	int port_idx;
	usb_peri_reg_t * peri;
	usb_config_data_t config;
	usb_dbg_uart_data_t uart;
	usb_ctrl_data_t control;
	int clk_sel,clk_div,clk_src;
	unsigned int port = usb_cfg->base_addr & USB_PHY_PORT_MSK;
	int time_dly = 500; //usec
	
	if(!usb_cfg)
		return -1;


	if(port == USB_PHY_PORT_A){
		port_idx = 0;
		peri = (usb_peri_reg_t*)CBUS_REG_ADDR(PREI_USB_PHY_REG_A);
	}else if(port == USB_PHY_PORT_B){
		port_idx = 1;
		peri = (usb_peri_reg_t*)CBUS_REG_ADDR(PREI_USB_PHY_REG_B);
	}else{
		printf("usb base address error: %p\n",usb_cfg->base_addr);
		return -1;
	}
	writel((1 << 2),P_RESET1_REGISTER);	
	printf("USB (%d) peri reg base: %x\n",port_idx,(uint32_t)peri);

	clk_sel = usb_cfg->clk_selecter;
	clk_div = usb_cfg->pll_divider;

	config.d32 = peri->config;
	config.b.clk_sel = clk_sel;	
	config.b.clk_div = clk_div; 
  	config.b.clk_en = 1;
	peri->config = config.d32;

	printf("USB (%d) use clock source: %s\n",port_idx,g_clock_src_name_m6[clk_sel]);

	control.d32 = peri->ctrl;
	control.b.fsel = 2;	/* PHY default is 24M (5), change to 12M (2) */
	control.b.por = 1;  /* power off default*/
	peri->ctrl = control.d32;
	udelay(time_dly);

	return 0;
}
//call after set clock
void set_usb_phy_power(amlogic_usb_config_t * usb_cfg,int is_on)
{
	unsigned long delay = 1000;
	int port_idx;
	unsigned int port = usb_cfg->base_addr & USB_PHY_PORT_MSK;
	usb_peri_reg_t *peri_a,*peri_b,*peri;
	usb_ctrl_data_t control;

	peri_a = (usb_peri_reg_t*)CBUS_REG_ADDR(PREI_USB_PHY_REG_A);
	peri_b = (usb_peri_reg_t*)CBUS_REG_ADDR(PREI_USB_PHY_REG_B);

	if(port == USB_PHY_PORT_A){
		peri = peri_a;
		port_idx = 0;
	}else{
		peri = peri_b;
		port_idx = 1;
	}
	
	if(is_on){
		control.d32 = peri_a->ctrl;
		control.b.por = 0;
		peri_a->ctrl = control.d32;

		control.d32 = peri_b->ctrl;
		control.b.por = 0;
		peri_b->ctrl = control.d32;

		/* read back clock detected flag*/
		control.d32 = peri->ctrl;
		if(!control.b.clk_detected){
			printf("USB (%d) PHY Clock not detected!\n",port_idx);
		}

	}else{
		control.d32 = peri_a->ctrl;
		control.b.por = 1;
		peri_a->ctrl = control.d32;

		control.d32 = peri_b->ctrl;
		control.b.por = 1;
		peri_b->ctrl = control.d32;
	}
	udelay(delay);

}
amlogic_usb_config_t * board_usb_start(void)
{
	if(!g_usb_cfg)
		return 0;


	set_usb_phy_clock(g_usb_cfg);
	set_usb_phy_power(g_usb_cfg,1);//on
	return g_usb_cfg;
}

int board_usb_stop(void)
{
	printf("board_usb_stop!\n");
	set_usb_phy_power(g_usb_cfg,0);//off

	return 0;
}
void board_usb_init(amlogic_usb_config_t * usb_cfg)
{
	g_usb_cfg = usb_cfg;
}
#endif //CONFIG_USB_DWC_OTG_HCD
