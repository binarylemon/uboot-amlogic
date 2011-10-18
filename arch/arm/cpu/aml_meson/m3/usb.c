/******************************************************************************
 *
 *  Copyright (C) 2011 AMLOGIC, INC.
 *
 *
 * remark: copy from @ trunk/arch/arm/cpu/aml_meson/m3 @Rev2634
 *              by Haixiang.Bao 2011.10.17
 *              haixiang.bao@amlogic.com
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
#include <common.h>
#include <asm/cache.h>
#include <asm/arch/usb.h>

#ifdef CONFIG_USB_DWC_OTG_HCD
static amlogic_usb_config_t * g_usb_cfg = 0;
static char * g_clock_src_name_m3[]={
		"XTAL input 24MHz",
		"XTAL input divided by 2,12MHz",
		"SYS PLL clock",
		"MISC PLL clock",
		"DDR PLL clock",
		"AUD pll clock",
		"VID PLL clock"
		"VID2 pll clock"
};

int set_usb_phy_id_mode(amlogic_usb_config_t * usb_cfg)
{
    int i;
    int time_dly = 50000;
    unsigned int reg;
    unsigned int port;
    unsigned int mode;

	mode = usb_cfg->id_mode;
	port = usb_cfg->base_addr & USB_PHY_PORT_MSK;
	
    if(port == USB_PHY_PORT_A)
    {
        reg = PREI_USB_PHY_A_REG3;
    }
    else if(port == USB_PHY_PORT_B)
    {
        reg = PREI_USB_PHY_B_REG4;
    }
    else
    {
        printf("id_mode this usb port is not exist!\n");
        return -1;
    }

    CLEAR_CBUS_REG_MASK(reg, PREI_USB_PHY_MODE_MASK);
    i=0;
    while(i++<time_dly){};
    
    switch(mode)
    {
        case USB_ID_MODE_M3_SW_HOST:
        		printf("usb id mode: SW_HOST\n");
            SET_CBUS_REG_MASK(reg, PREI_USB_PHY_MODE_SW_HOST);
            break;
			
        case USB_ID_MODE_M3_SW_DEVICE:
        		printf("usb id mode: SW_DEVICE\n");
            SET_CBUS_REG_MASK(reg, PREI_USB_PHY_MODE_SW_SLAVE);
            break;
			
        case USB_ID_MODE_M3_HARDWARE:
        default:
        		printf("usb id mode: HARDWARE\n");
            SET_CBUS_REG_MASK(reg, PREI_USB_PHY_MODE_HW);
            break;
    }
    
    i=0;
    while(i++<time_dly){};

    return 0;
}

void set_usb_phy_clock(amlogic_usb_config_t * usb_cfg)
{
    int clk_sel = usb_cfg->clk_selecter;
	int pll_div = usb_cfg->pll_divider;

	// ------------------------------------------------------------
	//  CLK_SEL: These bits select the source for the 12Mhz: 
	// 0 = XTAL input (24Mhz)
	// 1 = XTAL input divided by 2 (12MHz)
	// 2 = SYS PLL output (800MHz)
	// 3 = MISC pll clock(800MHz)
	// 4 = DDR pll clock (528MHz)

	//following not support yet
	// 5 = AUD pll clock ()
	// 6 = VID pll clock()
	// 7 = VID2 pll clock()
	
	CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_CLK_SEL);
	//clk_sel = 0; // 24M CLK 
	//clk_sel = 1; // 12M, Phy default setting is 12Mhz
	//clk_sel = 2; // SYS pll, 800M
	//clk_sel = 3; // MISC pll, 800M
	//clk_sel = 4; // DDR pll, 528M
	
	//following not support yet
	//clk_sel = 5; // AUD pll
	//clk_sel = 6; // VID pll 
	//clk_sel = 6; // VID2 pll
	
	printf("usb clk_sel: %s\n",g_clock_src_name_m3[clk_sel]);
	
	SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (clk_sel<<5 ));

	CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG,PREI_USB_PHY_CLK_DIV);
	switch(clk_sel)
	{
		case USB_PHY_CLOCK_SEL_M3_XTAL://24M
			//XTAL 24M, Divide by 2
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
			
		case USB_PHY_CLOCK_SEL_M3_XTAL_DIV2://12M
			//XTAL 24M, Divide by 1
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
		/*	
		//when 800MHz, divider set to 65,66 all fail
		case USB_PHY_CLOCK_SEL_M3_SYS_PLL://800M
			//SYS PLL running at 800M (800/(65+1)=12)
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
			
		case USB_PHY_CLOCK_SEL_M3_MISC_PLL://800M
			//MISC runing 800MHz (800/(65+1)=12)
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
		*/	
		case USB_PHY_CLOCK_SEL_M3_DDR_PLL:
			//DDR 528M (528/(43+1) = 12)
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
		/*	
		case USB_PHY_CLOCK_SEL_M3_AUD_PLL: 
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
			
		case USB_PHY_CLOCK_SEL_M3_VID_PLL: 
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
			
		case USB_PHY_CLOCK_SEL_M3_VID2_PLL:
			SET_CBUS_REG_MASK(PREI_USB_PHY_REG, (pll_div << 24));
			break;
		*/
	}

	// Open clock gate, to enable CLOCK to usb phy 
	SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_CLK_GATE);

}
void set_usb_phy_reset(amlogic_usb_config_t * usb_cfg,int is_on)
{
	int i;
    int time_dly = 50000;
    unsigned int port = usb_cfg->base_addr & USB_PHY_PORT_MSK;;

		if(port == USB_PHY_PORT_A){   
      if(is_on){
      	    /*  Reset USB PHY A  */
          SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_AHB_RSET);
          i=0;
          while(i++<time_dly){};  
          CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_AHB_RSET);
          i=0;
          while(i++<time_dly){};
          SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_CLK_RSET);
          i=0;
          while(i++<time_dly){};      
          CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_CLK_RSET);
          i=0;
          while(i++<time_dly){};
          SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_PLL_RSET);
          i=0;
          while(i++<time_dly){};
          CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_PLL_RSET);
          i=0;
          while(i++<time_dly){};
  		}
      // ------------------------------------------------------------ 
      // Reset the PHY A by setting POR high for 10uS.
      SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_POR);
      i=0;
      while(i++<time_dly){};
      // Set POR to the PHY high
      if(is_on){
        	CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_A_POR);
        	i=0;
        	while(i++<time_dly){};
    	}  
		}

		if(port == USB_PHY_PORT_B){		    
    		if(is_on){
            /* Reset USB PHY B */
            SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_AHB_RSET);
            i=0;
            while(i++<time_dly){};
            CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_AHB_RSET);
            i=0;
            while(i++<time_dly){};
            SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_CLK_RSET);
            i=0;
            while(i++<time_dly){};
            CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_CLK_RSET);
            i=0;
            while(i++<time_dly){};
            SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_PLL_RSET);
            i=0;
            while(i++<time_dly){};
            CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_PLL_RSET);
            i=0;
            while(i++<time_dly){};
    		}
        // ------------------------------------------------------------ 
        // Reset the PHY B by setting POR high for 10uS.
        SET_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_POR);
        i=0;
        while(i++<time_dly){};
    
        // Set POR to the PHY high
        if(is_on){
        	CLEAR_CBUS_REG_MASK(PREI_USB_PHY_REG, PREI_USB_PHY_B_POR);
        	i=0;
        	while(i++<time_dly){};
        }
  	}
}
amlogic_usb_config_t * board_usb_start(void)
{
	if(!g_usb_cfg)
		return 0;

	set_usb_phy_id_mode(g_usb_cfg);
	set_usb_phy_clock(g_usb_cfg);
	set_usb_phy_reset(g_usb_cfg,1);//on
	
	return g_usb_cfg;
}

int board_usb_stop(void)
{
	printf("board_usb_stop!\n");
	set_usb_phy_reset(g_usb_cfg,0);//off

	return 0;
}
void board_usb_init(amlogic_usb_config_t * usb_cfg)
{
	g_usb_cfg = usb_cfg;
}
#endif //CONFIG_USB_DWC_OTG_HCD