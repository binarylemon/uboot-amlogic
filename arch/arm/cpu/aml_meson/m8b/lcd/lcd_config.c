/*
 * AMLOGIC lcd controller driver.
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
 * Modify:  Evoke Zhang <evoke.zhang@amlogic.com>
 * compatible dts
 *
 */
#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcd_reg.h>
#include <amlogic/lcdoutc.h>
#include <amlogic/aml_lcd_common.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include "lcd_config.h"
#include "mipi_dsi_util.h"

#define VPP_OUT_SATURATE            (1 << 0)

static Lcd_Config_t *lcd_Conf;
static unsigned char lcd_gamma_init_err = 0;

#define SS_LEVEL_MAX	5
static const char *lcd_ss_level_table[]={
	"0",
	"0.5%",
	"1%",
	"1.5%",
	"2%",
};

#define LVDS_VSWING_LEVEL_MAX  5
static unsigned int lvds_vswing_ctrl[] = {
//vswing_ctrl   level   voltage
    0x028,      //0      0.2V
    0x048,      //1      0.4V
    0x088,      //2      0.6V
    0x0c8,      //3      0.8V
    0x0f8,      //4      1.2V
};

static void print_lcd_driver_version(void)
{
    printf("lcd driver version: %s%s\n\n", LCD_DRV_DATE, LCD_DRV_TYPE);
}

static void lcd_ports_ctrl_lvds(Bool_t status)
{
	if (status) {
		WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 3, 1); //enable lvds fifo
		if (lcd_Conf->lcd_basic.lcd_bits == 6)
			WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, LVDS_LANE_COUNT_3, BIT_DPHY_LANE, 5);
		else
			WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, LVDS_LANE_COUNT_4, BIT_DPHY_LANE, 5);
	}
	else {
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x0);
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x00060000);
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x00200000);
	}

	lcd_print("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static void lcd_ports_ctrl_mipi(Bool_t status)
{
    if (status) {
        switch (lcd_Conf->lcd_control.mipi_config->lane_num) {
            case 1:
                WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, MIPI_DSI_LANE_COUNT_1, BIT_DPHY_LANE, 5);
                break;
            case 2:
                WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, MIPI_DSI_LANE_COUNT_2, BIT_DPHY_LANE, 5);
                break;
            case 3:
                WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, MIPI_DSI_LANE_COUNT_3, BIT_DPHY_LANE, 5);
                break;
            case 4:
                WRITE_LCD_CBUS_REG_BITS(HHI_DIF_CSI_PHY_CNTL3, MIPI_DSI_LANE_COUNT_4, BIT_DPHY_LANE, 5);
                break;
            default:
                break;
        }
    }
    else {
        WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x0);
        WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x00060000);
        WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x00200000);
    }

    lcd_print("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static unsigned pinmux_data_set[][2]={
    {PERIPHS_PIN_MUX_0,0x03c00000},//RGB[7:2]
    //{PERIPHS_PIN_MUX_8,0x00070000},//RGB[1:0]
};
static unsigned pinmux_data_clr[][2]={
    {PERIPHS_PIN_MUX_6,0x0000077f},//RGB[7:2]
    //{PERIPHS_PIN_MUX_4,0x0000000e},//RGB[1:0]
    //{PERIPHS_PIN_MUX_5,0x07000000},//RGB[1:0]
    //{PERIPHS_PIN_MUX_6,0x0000f800},//RGB[1:0]
    //{PERIPHS_PIN_MUX_7,0x00300000},//RGB[1:0]
    //{PERIPHS_PIN_MUX_9,0x00001c00},//RGB[1:0]
};
static unsigned pinmux_data_set_table[][3]={
    {0,0xf},  //6bit index
    //{0,1,0xf},//8bit index
};
static unsigned pinmux_data_clr_table[][7]={
    {0,0xf},          //6bit index
    //{0,1,2,3,4,5,0xf},//8bit index
};

static unsigned pinmux_tcon_set[][2]={
    {PERIPHS_PIN_MUX_1,0x00000004},//clk
    {PERIPHS_PIN_MUX_1,0x00000002},//DE
    {PERIPHS_PIN_MUX_1,0x00000011},//sync
};
static unsigned pinmux_tcon_clr[][2]={
    {PERIPHS_PIN_MUX_1,0x02000008},//clk
    {PERIPHS_PIN_MUX_8,0x00001000},//clk
    {PERIPHS_PIN_MUX_1,0x01000000},//DE
    {PERIPHS_PIN_MUX_1,0x04800000},//sync
    {PERIPHS_PIN_MUX_5,0x08000000},//sync
    {PERIPHS_PIN_MUX_8,0x00000800},//sync
    {PERIPHS_PIN_MUX_9,0x00020000},//sync
};
static unsigned pinmux_tcon_set_table[][4]={
    {0,1,0xf},  //DE index
    {0,2,0xf},  //Sync index
    {0,1,2,0xf},//DE+Sync index
};
static unsigned pinmux_tcon_clr_table[][8]={
    {0,1,2,0xf},        //DE index
    {0,1,3,4,5,6,0xf},  //Sync index
    {0,1,2,3,4,5,6,0xf},//DE+Sync index
};

static unsigned gpio_data_set[][2]={
    {PREG_PAD_GPIO4_EN_N, 0x3ffff000},//RGB[7:2]
    //{PREG_PAD_GPIO3_EN_N, 0x1f800000},//RGB[1:0]
};
static unsigned gpio_tcon_set[][2]={
    {PREG_PAD_GPIO3_EN_N, 0x00100000},//clk
    {PREG_PAD_GPIO3_EN_N, 0x00200000},//DE
    {PREG_PAD_GPIO3_EN_N, 0x00480000},//Sync
};
static unsigned gpio_data_set_table[][3]={
    {0,0xf},  //6bit index
    //{0,1,0xf},//8bit index
};
static unsigned gpio_tcon_set_table[][4]={
    {0,1,0xf},  //DE index
    {0,2,0xf},  //Sync index
    {0,1,2,0xf},//DE+Sync index
};

static void lcd_ports_ctrl_ttl(Bool_t status)
{
	unsigned *pin_tcon_set, *pin_data_set;
	unsigned *pin_tcon_clr, *pin_data_clr;
	unsigned *gpio_tcon, *gpio_data;
	unsigned pin_reg;
	int i;
	
	//tcon pin
	if ((lcd_Conf->lcd_timing.de_valid == 1) && (lcd_Conf->lcd_timing.hvsync_valid == 0)) {
		pin_tcon_set = &pinmux_tcon_set_table[0][0];
		pin_tcon_clr = &pinmux_tcon_clr_table[0][0];
		gpio_tcon = &gpio_tcon_set_table[0][0];
	}
	else if ((lcd_Conf->lcd_timing.de_valid == 0) && (lcd_Conf->lcd_timing.hvsync_valid == 1)) {
		pin_tcon_set = &pinmux_tcon_set_table[1][0];
		pin_tcon_clr = &pinmux_tcon_clr_table[1][0];
		gpio_tcon = &gpio_tcon_set_table[1][0];
	}
	else {
		pin_tcon_set = &pinmux_tcon_set_table[2][0];
		pin_tcon_clr = &pinmux_tcon_clr_table[2][0];
		gpio_tcon = &gpio_tcon_set_table[2][0];
	}
	//RGB data pin
	//if (lcd_Conf->lcd_basic.lcd_bits == 8) {
	//	pin_data_set = &pinmux_data_set_table[1][0];
	//	pin_data_clr = &pinmux_data_clr_table[1][0];
	//	gpio_data = &gpio_data_set_table[1][0];
	//}
	//else 
	//{
		pin_data_set = &pinmux_data_set_table[0][0];
		pin_data_clr = &pinmux_data_clr_table[0][0];
		gpio_data = &gpio_data_set_table[0][0];
	//}
	if (status) {
		i = 0;
		while (i < 0xf) {//pinmux_tcon_set
			if (pin_tcon_set[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_tcon_set[pin_tcon_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | pinmux_tcon_set[pin_tcon_set[i]][1]));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//pinmux_tcon_clr
			if (pin_tcon_clr[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_tcon_clr[pin_tcon_clr[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_tcon_clr[pin_tcon_clr[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//pinmux_data_set
			if (pin_data_set[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_data_set[pin_data_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | pinmux_data_set[pin_data_set[i]][1]));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//pinmux_data_clr
			if (pin_data_clr[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_data_clr[pin_data_clr[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_data_clr[pin_data_clr[i]][1])));
				i++;
			}
		}
 	}else {
		i = 0;
		while (i < 0xf) {//pinmux_data_set
			if (pin_data_set[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_data_set[pin_data_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_data_set[pin_data_set[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//pinmux_tcon_set
			if (pin_tcon_set[i] == 0xf) {
				break;
			}
			else {
				pin_reg = pinmux_tcon_set[pin_tcon_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_tcon_set[pin_tcon_set[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//gpio_data_set
			if (gpio_data[i] == 0xf) {
				break;
			}
			else {
				pin_reg = gpio_data_set[gpio_data[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | (gpio_data_set[gpio_data[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xf) {//gpio_tcon_set
			if (gpio_tcon[i] == 0xf) {
				break;
			}
			else {
				pin_reg = gpio_tcon_set[gpio_tcon[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | (gpio_tcon_set[gpio_tcon[i]][1])));
				i++;
			}
		}
	}
	lcd_print("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static void lcd_ports_ctrl(Bool_t status)
{	
	switch(lcd_Conf->lcd_basic.lcd_type){
		case LCD_DIGITAL_MIPI:
			lcd_ports_ctrl_mipi(status);
			break;
		case LCD_DIGITAL_LVDS:
			lcd_ports_ctrl_lvds(status);
			break;
		case LCD_DIGITAL_TTL:
			lcd_ports_ctrl_ttl(status);
			break;
		default:
			printf("Invalid LCD type.\n");
			break;
	}
}

static void set_control_mipi(Lcd_Config_t *pConf);
//for special interface
static int lcd_power_ctrl_video(Bool_t status)
{
    if (status) {
        switch(lcd_Conf->lcd_basic.lcd_type) {
            case LCD_DIGITAL_MIPI:
                set_control_mipi(lcd_Conf);
                break;
            default:
                break;
        }
    }
    else {
        switch(lcd_Conf->lcd_basic.lcd_type) {
            case LCD_DIGITAL_MIPI:
                mipi_dsi_link_off(lcd_Conf);  //link off command
                break;
            default:
                break;
        }
    }
    lcd_print("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
    return 0;
}

#define LCD_GAMMA_RETRY_CNT  1000
static void write_gamma_table(u16 *data, u32 rgb_mask, u16 gamma_coeff, u32 gamma_reverse)
{
	int i;
	int cnt = 0;
	
	rgb_mask = gamma_sel_table[rgb_mask];
	while ((!(READ_LCD_REG(L_GAMMA_CNTL_PORT) & (0x1 << LCD_ADR_RDY))) && (cnt < LCD_GAMMA_RETRY_CNT)) {
		udelay(10);
		cnt++;
	};
	WRITE_LCD_REG(L_GAMMA_ADDR_PORT, (0x1 << LCD_H_AUTO_INC) | (0x1 << rgb_mask) | (0x0 << LCD_HADR));
	if (gamma_reverse == 0) {
		for (i=0;i<256;i++) {
			cnt = 0;
			while ((!( READ_LCD_REG(L_GAMMA_CNTL_PORT) & (0x1 << LCD_WR_RDY))) && (cnt < LCD_GAMMA_RETRY_CNT)) {
				udelay(10);
				cnt++;
			};
			WRITE_LCD_REG(L_GAMMA_DATA_PORT, (data[i] * gamma_coeff / 100));
		}
	}
	else {
		for (i=0;i<256;i++) {
			cnt = 0;
			while ((!( READ_LCD_REG(L_GAMMA_CNTL_PORT) & (0x1 << LCD_WR_RDY))) && (cnt < LCD_GAMMA_RETRY_CNT)) {
				udelay(10);
				cnt++;
			};
			WRITE_LCD_REG(L_GAMMA_DATA_PORT, (data[255-i] * gamma_coeff / 100));
		}
	}
	cnt = 0;
	while ((!(READ_LCD_REG(L_GAMMA_CNTL_PORT) & (0x1 << LCD_ADR_RDY))) && (cnt < LCD_GAMMA_RETRY_CNT)) {
		udelay(10);
		cnt++;
	};
	WRITE_LCD_REG(L_GAMMA_ADDR_PORT, (0x1 << LCD_H_AUTO_INC) | (0x1 << rgb_mask) | (0x23 << LCD_HADR));
	
	if (cnt >= LCD_GAMMA_RETRY_CNT)
		lcd_gamma_init_err = 1;
}

static void set_gamma_table_lcd(unsigned int gamma_en)
{
	lcd_print("%s\n", __FUNCTION__);
	lcd_gamma_init_err = 0;
	write_gamma_table(lcd_Conf->lcd_effect.GammaTableR, GAMMA_SEL_R, lcd_Conf->lcd_effect.gamma_r_coeff, ((lcd_Conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));
	write_gamma_table(lcd_Conf->lcd_effect.GammaTableG, GAMMA_SEL_G, lcd_Conf->lcd_effect.gamma_g_coeff, ((lcd_Conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));
	write_gamma_table(lcd_Conf->lcd_effect.GammaTableB, GAMMA_SEL_B, lcd_Conf->lcd_effect.gamma_b_coeff, ((lcd_Conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));

	if (lcd_gamma_init_err) {
		WRITE_LCD_REG_BITS(L_GAMMA_CNTL_PORT, 0, 0, 1);
		printf("[warning]: write gamma table error, gamma table disabled\n");
	}
	else
		WRITE_LCD_REG_BITS(L_GAMMA_CNTL_PORT, gamma_en, 0, 1);
}

static void set_tcon_lcd(Lcd_Config_t *pConf)
{
	Lcd_Timing_t *tcon_adr = &(pConf->lcd_timing);
	unsigned hs_pol_adj, vs_pol_adj;

	lcd_print("%s\n", __FUNCTION__);
	
	set_gamma_table_lcd(((pConf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_EN) & 1));

	WRITE_LCD_REG(L_RGB_BASE_ADDR,  pConf->lcd_effect.rgb_base_addr);
	WRITE_LCD_REG(L_RGB_COEFF_ADDR, pConf->lcd_effect.rgb_coeff_addr);
	if (pConf->lcd_effect.dith_user) {
		WRITE_LCD_REG(L_DITH_CNTL_ADDR,  pConf->lcd_effect.dith_cntl_addr);
	}
	else {
		if(pConf->lcd_basic.lcd_bits == 8)
			WRITE_LCD_REG(L_DITH_CNTL_ADDR,  0x400);
		else
			WRITE_LCD_REG(L_DITH_CNTL_ADDR,  0x600);
	}
	
	WRITE_LCD_REG(L_POL_CNTL_ADDR,   (((pConf->lcd_timing.pol_ctrl >> POL_CTRL_CLK) & 1) << LCD_CPH1_POL));
	
	switch (pConf->lcd_basic.lcd_type) {
		case LCD_DIGITAL_MIPI:
			hs_pol_adj = 1; //1 for low active, 0 for high active.
			vs_pol_adj = 1; //1 for low active, 0 for high active
			WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((0 << LCD_DE_POL) | (vs_pol_adj << LCD_VS_POL) | (hs_pol_adj << LCD_HS_POL)))); //adjust hvsync pol
			WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((1 << LCD_TCON_DE_SEL) | (1 << LCD_TCON_VS_SEL) | (1 << LCD_TCON_HS_SEL)))); //enable tcon DE, Hsync, Vsync
			break;
		case LCD_DIGITAL_LVDS:
		case LCD_DIGITAL_TTL:
			hs_pol_adj = (((pConf->lcd_timing.pol_ctrl >> POL_CTRL_HS) & 1) ? 0 : 1); //1 for low active, 0 for high active.
			vs_pol_adj = (((pConf->lcd_timing.pol_ctrl >> POL_CTRL_VS) & 1) ? 0 : 1); //1 for low active, 0 for high active
			WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((0 << LCD_DE_POL) | (vs_pol_adj << LCD_VS_POL) | (hs_pol_adj << LCD_HS_POL)))); //adjust hvsync pol
			WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((1 << LCD_TCON_DE_SEL) | (1 << LCD_TCON_VS_SEL) | (1 << LCD_TCON_HS_SEL)))); //enable tcon DE, Hsync, Vsync 
			break;
		default:
			hs_pol_adj = 0;
			vs_pol_adj = 0;
			break;
	}
	
	//DE signal
	WRITE_LCD_REG(L_DE_HS_ADDR,    tcon_adr->de_hs_addr);
	WRITE_LCD_REG(L_DE_HE_ADDR,    tcon_adr->de_he_addr);
	WRITE_LCD_REG(L_DE_VS_ADDR,    tcon_adr->de_vs_addr);
	WRITE_LCD_REG(L_DE_VE_ADDR,    tcon_adr->de_ve_addr);
	
	WRITE_LCD_REG(L_OEV1_HS_ADDR,  tcon_adr->de_hs_addr);
	WRITE_LCD_REG(L_OEV1_HE_ADDR,  tcon_adr->de_he_addr);
	WRITE_LCD_REG(L_OEV1_VS_ADDR,  tcon_adr->de_vs_addr);
	WRITE_LCD_REG(L_OEV1_VE_ADDR,  tcon_adr->de_ve_addr);
	
	//Hsync signal
	WRITE_LCD_REG(L_HSYNC_HS_ADDR,  tcon_adr->hs_hs_addr);
	WRITE_LCD_REG(L_HSYNC_HE_ADDR,  tcon_adr->hs_he_addr);
	WRITE_LCD_REG(L_HSYNC_VS_ADDR,  tcon_adr->hs_vs_addr);
	WRITE_LCD_REG(L_HSYNC_VE_ADDR,  tcon_adr->hs_ve_addr);
	
	//Vsync signal
	WRITE_LCD_REG(L_VSYNC_HS_ADDR,  tcon_adr->vs_hs_addr);
	WRITE_LCD_REG(L_VSYNC_HE_ADDR,  tcon_adr->vs_he_addr);
	WRITE_LCD_REG(L_VSYNC_VS_ADDR,  tcon_adr->vs_vs_addr);
	WRITE_LCD_REG(L_VSYNC_VE_ADDR,  tcon_adr->vs_ve_addr);
	
	WRITE_LCD_REG(L_INV_CNT_ADDR,       0);
	WRITE_LCD_REG(L_TCON_MISC_SEL_ADDR, ((1 << LCD_STV1_SEL) | (1 << LCD_STV2_SEL)));
	
	WRITE_LCD_REG(VPP_MISC, READ_LCD_REG(VPP_MISC) & ~(VPP_OUT_SATURATE));
}

static void vclk_set_lcd(int lcd_type, unsigned long pll_reg, unsigned long vid_div_reg, unsigned int clk_ctrl_reg)
{
	unsigned xd = 0;
	unsigned pll_level = 0, pll_frac = 0;
	int wait_loop = PLL_WAIT_LOCK_CNT;
	unsigned pll_lock = 0;
	unsigned ss_level=0, pll_ctrl2, pll_ctrl3, pll_ctrl4, od_fb;
	
	lcd_print("%s\n", __FUNCTION__);
	
	vid_div_reg = ((vid_div_reg & 0x1ffff) | (1 << 16) | (0 << 15) | (0x3 << 0));	//select vid_pll and enable clk
	xd = (clk_ctrl_reg >> CLK_CTRL_XD) & 0xff;
	pll_level = (clk_ctrl_reg >> CLK_CTRL_LEVEL) & 0x7;
	pll_frac = (clk_ctrl_reg >> CLK_CTRL_FRAC) & 0xfff;
	ss_level = (clk_ctrl_reg >> CLK_CTRL_SS) & 0xf;
	pll_reg |= (1 << PLL_CTRL_EN);

	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 19, 1);	//disable vclk2_en
	udelay(2);
	
	if (pll_frac == 0)
		pll_ctrl2 = 0x59c88000;
	else
		pll_ctrl2 = (0x59c8c000 | pll_frac);
	
	pll_ctrl4 = (0x00238100 & ~((1<<9) | (0xf<<4) | (0xf<<0)));
	switch (ss_level) {
		case 1:	//0.5%
			pll_ctrl4 |= ((1<<9) | (2<<4) | (1<<0));
			break;
		case 2:	//1%
			pll_ctrl4 |= ((1<<9) | (1<<4) | (1<<0));
			break;
		case 3:	//1.5%
			pll_ctrl4 |= ((1<<9) | (8<<4) | (1<<0));
			break;
		case 4: //2%
			pll_ctrl4 |= ((1<<9) | (0<<4) | (1<<0));
			break;
		case 0:
		default:
			ss_level = 0;
			break;
	}
	
	switch (pll_level) {
		case 1: //<=1.7G
			pll_ctrl3 = (ss_level > 0) ? 0xca7e3823 : 0xca49b022;
			od_fb = 0;
			break;
		case 2: //1.7G~2.0G
			pll_ctrl2 |= (1<<13);//special adjust
			pll_ctrl3 = (ss_level > 0) ? 0xca7e3823 : 0xca493822;
			od_fb = 1;
			break;
		case 3: //2.0G~2.5G
			pll_ctrl3 = (ss_level > 0) ? 0xca7e3823 : 0xca493822;
			od_fb = 1;
			break;
		case 4: //>=2.5G
			pll_ctrl3 = (ss_level > 0) ? 0xca7e3823 : 0xce49c022;
			od_fb = 1;
			break;
		default:
			pll_ctrl3 = 0xca7e3823;
			od_fb = 0;
			break;
	}
	WRITE_LCD_CBUS_REG_BITS(HHI_VID2_PLL_CNTL2, 1, 16, 1);//enable ext LDO
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL2, pll_ctrl2);
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL3, pll_ctrl3);
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL4, (pll_ctrl4 | (od_fb << 24))); //[24] od_fb
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL5, 0x00012385);
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL, pll_reg | (1 << PLL_CTRL_RST));
	WRITE_LCD_CBUS_REG(HHI_VID_PLL_CNTL, pll_reg);
	
	do{
		udelay(50);
		pll_lock = (READ_LCD_CBUS_REG(HHI_VID_PLL_CNTL) >> PLL_CTRL_LOCK) & 0x1;
		if (wait_loop == 100) {
			if (pll_level == 2) {
				//change setting if can't lock
				WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CNTL2, 1, 12, 1);
				WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CNTL, 1, PLL_CTRL_RST, 1);
				WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CNTL, 0, PLL_CTRL_RST, 1);
				printf("change setting for vid pll stability\n");
			}
		}
		wait_loop--;
	}while((pll_lock == 0) && (wait_loop > 0));
	if (wait_loop == 0)
		printf("[error]: vid_pll lock failed\n");

	WRITE_LCD_CBUS_REG(HHI_VIID_DIVIDER_CNTL, vid_div_reg);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 1, 7, 1);   //0x104c[7]:SOFT_RESET_POST
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 1, 3, 1);   //0x104c[3]:SOFT_RESET_PRE
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 1, 1);   //0x104c[1]:RESET_N_POST
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 0, 1);   //0x104c[0]:RESET_N_PRE
	udelay(10);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 3, 1);   //release SOFT_RESET_PRE
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 7, 1);   //release SOFT_RESET_POST
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 3, 0, 2);   //release RESET_N_PRE, RESET_N_POST
	udelay(5);

	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, (xd-1), 0, 8);   // setup XD divider
	udelay(5);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 4, 16, 3);  // Bit[18:16] - v2_cntl_clk_in_sel
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 19, 1);  //vclk2_en0
	udelay(2);
	
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, 8, 12, 4); // [15:12] encl_clk_sel, select vclk2_div1
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, 1, 16, 2); // release vclk2_div_reset and enable vclk2_div
	udelay(2);
	
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 0, 1);   //enable v2_clk_div1
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 15, 1);  //soft reset
	udelay(5);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 15, 1);  //release soft reset
	udelay(5);
	
	WRITE_LCD_CBUS_REG_BITS(HHI_VID_CLK_CNTL2, 1, 3, 1);	//enable encl clk gate, new added in m8baby
}

static void clk_util_lvds_set_clk_div(unsigned long divn_sel, unsigned long divn_tcnt, unsigned long div2_en)
{
    // ---------------------------------------------
    // Configure the LVDS PHY
    // ---------------------------------------------
    // wire    [4:0]   cntl_ser_en         = control[20:16];
    // wire            cntl_prbs_en        = control[13];
    // wire            cntl_prbs_err_en    = control[12];
    // wire    [1:0]   cntl_mode_set_high  = control[11:10];
    // wire    [1:0]   cntl_mode_set_low   = control[9:8];
    // 
    // wire    [1:0]   fifo_clk_sel        = control[7;6]
    // 
    // wire            mode_port_rev       = control[4];
    // wire            mode_bit_rev        = control[3];
    // wire            mode_inv_p_n        = control[2];
    // wire            phy_clk_en          = control[1];
    // wire            soft_reset_int      = control[0];
    WRITE_LCD_CBUS_REG(HHI_LVDS_TX_PHY_CNTL0, (0x1f << 16) | (0x1 << 6) ); // enable all serializers, divide by 7
}

static void set_pll_lcd(Lcd_Config_t *pConf)
{
    unsigned pll_reg, div_reg, clk_reg;
    int xd;
    int lcd_type;
    unsigned pll_div_post = 0, phy_clk_div2 = 0;

    lcd_print("%s\n", __FUNCTION__);

    pll_reg = pConf->lcd_timing.pll_ctrl;
    div_reg = pConf->lcd_timing.div_ctrl;
    clk_reg = pConf->lcd_timing.clk_ctrl;
    xd = (clk_reg >> CLK_CTRL_XD) & 0xff;

    lcd_type = pConf->lcd_basic.lcd_type;

    switch(lcd_type){
        case LCD_DIGITAL_MIPI:
            break;
        case LCD_DIGITAL_LVDS:
            xd = 1;
            pll_div_post = 7;
            phy_clk_div2 = 0;
            div_reg = (div_reg | (1 << DIV_CTRL_POST_SEL) | (1 << DIV_CTRL_LVDS_CLK_EN) | ((pll_div_post-1) << DIV_CTRL_DIV_POST) | (phy_clk_div2 << DIV_CTRL_PHY_CLK_DIV2));
            break;
        case LCD_DIGITAL_TTL:
            break;
        default:
            break;
    }
    clk_reg = (pConf->lcd_timing.clk_ctrl & ~(0xff << CLK_CTRL_XD)) | (xd << CLK_CTRL_XD);

    vclk_set_lcd(lcd_type, pll_reg, div_reg, clk_reg);

    switch(lcd_type){
        case LCD_DIGITAL_MIPI:
            WRITE_LCD_REG(MIPI_DSI_TOP_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_CNTL) & ~(0x7<<4)) | (1 << 4) | (1 << 5) | (0 << 6));
            WRITE_LCD_REG(MIPI_DSI_TOP_SW_RESET, (READ_LCD_REG(MIPI_DSI_TOP_SW_RESET) | 0xf) );     // Release mipi_dsi_host's reset
            WRITE_LCD_REG(MIPI_DSI_TOP_SW_RESET, (READ_LCD_REG(MIPI_DSI_TOP_SW_RESET) & 0xfffffff0) );     // Release mipi_dsi_host's reset
            WRITE_LCD_REG(MIPI_DSI_TOP_CLK_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_CLK_CNTL) | 0x3) );            // Enable dwc mipi_dsi_host's clock 
            break;
        case LCD_DIGITAL_LVDS:
            clk_util_lvds_set_clk_div(1, pll_div_post, phy_clk_div2);
            //    lvds_gen_cntl       <= {10'h0,      // [15:4] unused
            //                            2'h1,       // [5:4] divide by 7 in the PHY
            //                            1'b0,       // [3] fifo_en
            //                            1'b0,       // [2] wr_bist_gate
            //                            2'b00};     // [1:0] fifo_wr mode
            //FIFO_CLK_SEL = 1; // div7
            WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 4, 2);	//lvds fifo clk div 7

            WRITE_LCD_REG_BITS(LVDS_PHY_CLK_CNTL, 0, 15, 1);	// lvds div reset
            udelay(5);
            WRITE_LCD_REG_BITS(LVDS_PHY_CLK_CNTL, 1, 15, 1);	// Release lvds div reset
            break;
        case LCD_DIGITAL_TTL:
            break;
        default:
            break;
    }
}

static void set_venc_lcd(Lcd_Config_t *pConf)
{
	lcd_print("%s\n",__FUNCTION__);

	WRITE_LCD_REG(ENCL_VIDEO_EN,          0);

	WRITE_LCD_REG_BITS(VPU_VIU_VENC_MUX_CTRL, 0, 0, 4);; //viu1, viu2 select encl
	
	WRITE_LCD_REG(ENCL_VIDEO_MODE,        0);
	WRITE_LCD_REG(ENCL_VIDEO_MODE_ADV,    0x8); // Sampling rate: 1

	WRITE_LCD_REG(ENCL_VIDEO_FILT_CTRL,   0x1000); // bypass filter

	WRITE_LCD_REG(ENCL_VIDEO_MAX_PXCNT,   pConf->lcd_basic.h_period - 1);
	WRITE_LCD_REG(ENCL_VIDEO_MAX_LNCNT,   pConf->lcd_basic.v_period - 1);

	WRITE_LCD_REG(ENCL_VIDEO_HAVON_BEGIN, pConf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_VIDEO_HAVON_END,   pConf->lcd_basic.h_active - 1 + pConf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_VIDEO_VAVON_BLINE, pConf->lcd_timing.video_on_line);
	WRITE_LCD_REG(ENCL_VIDEO_VAVON_ELINE, pConf->lcd_basic.v_active - 1  + pConf->lcd_timing.video_on_line);

	WRITE_LCD_REG(ENCL_VIDEO_HSO_BEGIN,   10);//pConf->lcd_timing.hs_hs_addr);
	WRITE_LCD_REG(ENCL_VIDEO_HSO_END,     16);//pConf->lcd_timing.hs_he_addr);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_BEGIN,   pConf->lcd_timing.vso_hstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_END,     pConf->lcd_timing.vso_hstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_BLINE,   pConf->lcd_timing.vso_vstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_ELINE,   pConf->lcd_timing.vso_vstart + 2);

	WRITE_LCD_REG(ENCL_VIDEO_RGBIN_CTRL,  (1 << 0));//(1 << 1) | (1 << 0));	//bit[0] 1:RGB, 0:YUV

	WRITE_LCD_REG(ENCL_VIDEO_EN,          1);	// enable encl
}

static void set_control_lvds(Lcd_Config_t *pConf)
{
	unsigned lvds_repack, pn_swap, bit_num;
	unsigned data32;
	
	lcd_print("%s\n", __FUNCTION__);

	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1); // disable fifo
	
    data32 = (0x00 << LVDS_blank_data_r) |
             (0x00 << LVDS_blank_data_g) |
             (0x00 << LVDS_blank_data_b) ;
    WRITE_LCD_REG(LVDS_BLANK_DATA_HI, (data32 >> 16));
    WRITE_LCD_REG(LVDS_BLANK_DATA_LO, (data32 & 0xffff));
	
	lvds_repack = (pConf->lcd_control.lvds_config->lvds_repack) & 0x1;
	pn_swap = (pConf->lcd_control.lvds_config->pn_swap) & 0x1;

	switch(pConf->lcd_basic.lcd_bits) {
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
	
	
	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 0, 1);  //[1:0]set single clock write mode
	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 3, 1); // enable fifo
	
	WRITE_LCD_REG(LVDS_PACK_CNTL_ADDR,
					( lvds_repack<<0 ) | // repack
					( 0<<2 ) | // odd_even
					( 0<<3 ) | // reserve
					( 0<<4 ) | // lsb first
					( pn_swap<<5 ) | // pn swap
					( 0<<6 ) | // dual port
					( 0<<7 ) | // use tcon control
					( bit_num<<8 ) | // 0:10bits, 1:8bits, 2:6bits, 3:4bits.
					( 0<<10 ) | //r_select  //0:R, 1:G, 2:B, 3:0
					( 1<<12 ) | //g_select  //0:R, 1:G, 2:B, 3:0
					( 2<<14 ));  //b_select  //0:R, 1:G, 2:B, 3:0;

	//lcd_print("lvds fifo clk = %d.\n", clk_util_clk_msr(LVDS_FIFO_CLK));
}

static void set_control_mipi(Lcd_Config_t *pConf)
{
    set_mipi_dsi_control(pConf);
}

static void set_control_ttl(Lcd_Config_t *pConf)
{
	unsigned rb_port_swap, rgb_bit_swap;
	
	rb_port_swap = (unsigned)(pConf->lcd_control.ttl_config->rb_swap);
	rgb_bit_swap = (unsigned)(pConf->lcd_control.ttl_config->bit_swap);
	
	WRITE_LCD_REG(L_DUAL_PORT_CNTL_ADDR, (rb_port_swap << LCD_RGB_SWP) | (rgb_bit_swap << LCD_BIT_SWP));
}

static void init_phy_lvds(Lcd_Config_t *pConf)
{
	unsigned int swing_level;
	lcd_print("%s\n", __FUNCTION__);
	
	WRITE_LCD_REG(LVDS_SER_EN, 0xfff);	//Enable the serializers

	WRITE_LCD_REG(LVDS_PHY_CNTL0, 0xffff);
	WRITE_LCD_REG(LVDS_PHY_CNTL1, 0xff00);
	WRITE_LCD_REG(LVDS_PHY_CNTL4, 0x007f);
	
	swing_level = (pConf->lcd_control.lvds_config->lvds_vswing >= LVDS_VSWING_LEVEL_MAX) ? (LVDS_VSWING_LEVEL_MAX - 1) : pConf->lcd_control.lvds_config->lvds_vswing;
	
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, lvds_vswing_ctrl[swing_level]);
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x000665b7);
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x84070000);
}

static void init_phy_mipi(Lcd_Config_t *pConf)
{
    lcd_print("%s\n", __FUNCTION__);

    WRITE_LCD_CBUS_REG_BITS(HHI_DSI_LVDS_EDP_CNTL1, 1, 4, 1);//swap mipi channels
    WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x8);//DIF_REF_CTL0
    WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, (0x3e << 16) | (0xa5b8 << 0));//DIF_REF_CTL2:31-16bit, DIF_REF_CTL1:15-0bit
    WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, (0x26e0 << 16) | (0x459 << 0));//DIF_TX_CTL1:31-16bit, DIF_TX_CTL0:15-0bit
}

static void init_dphy(Lcd_Config_t *pConf)
{
	unsigned lcd_type = (unsigned)(pConf->lcd_basic.lcd_type);

	switch (lcd_type) {
		case LCD_DIGITAL_MIPI:
			WRITE_LCD_CBUS_REG(HHI_DSI_LVDS_EDP_CNTL0, lcd_type);	//dphy select by interface
			init_phy_mipi(pConf);
			break;
		case LCD_DIGITAL_LVDS:
			WRITE_LCD_CBUS_REG(HHI_DSI_LVDS_EDP_CNTL0, lcd_type);	//dphy select by interface
			init_phy_lvds(pConf);
			break;
		default:
			break;
	}
}

static void set_video_adjust(Lcd_Config_t *pConf)
{
	lcd_print("vadj_brightness = 0x%x, vadj_contrast = 0x%x, vadj_saturation = 0x%x.\n", pConf->lcd_effect.vadj_brightness, pConf->lcd_effect.vadj_contrast, pConf->lcd_effect.vadj_saturation);
	WRITE_LCD_REG(VPP_VADJ2_Y, (pConf->lcd_effect.vadj_brightness << 8) | (pConf->lcd_effect.vadj_contrast << 0));
	WRITE_LCD_REG(VPP_VADJ2_MA_MB, (pConf->lcd_effect.vadj_saturation << 16));
	WRITE_LCD_REG(VPP_VADJ2_MC_MD, (pConf->lcd_effect.vadj_saturation << 0));
	WRITE_LCD_REG(VPP_VADJ_CTRL, 0xf);	//enable video adjust
}

static void _init_lcd_driver(Lcd_Config_t *pConf)
{
    int lcd_type = pConf->lcd_basic.lcd_type;
    unsigned char ss_level = (pConf->lcd_timing.clk_ctrl >> CLK_CTRL_SS) & 0xf;

	printf("\nInit LCD mode: %s, %s(%u) %ubit, %ux%u@%u.%uHz, ss_level=%u(%s)\n", pConf->lcd_basic.model_name, lcd_type_table[lcd_type], lcd_type, pConf->lcd_basic.lcd_bits, pConf->lcd_basic.h_active, pConf->lcd_basic.v_active, (pConf->lcd_timing.sync_duration_num / 10), (pConf->lcd_timing.sync_duration_num % 10), ss_level, lcd_ss_level_table[ss_level]);

    set_pll_lcd(pConf);
    set_venc_lcd(pConf);
    set_tcon_lcd(pConf);
    switch(lcd_type){
        case LCD_DIGITAL_MIPI:
            init_dphy(pConf);
            break;
        case LCD_DIGITAL_LVDS:
            set_control_lvds(pConf);
            init_dphy(pConf);
            break;
        case LCD_DIGITAL_TTL:
            set_control_ttl(pConf);
            break;
        default:
            printf("Invalid LCD type.\n");
            break;
    }
    set_video_adjust(pConf);
    printf("%s finished.\n", __FUNCTION__);
}

static void _disable_lcd_driver(Lcd_Config_t *pConf)
{
    switch(pConf->lcd_basic.lcd_type){
        case LCD_DIGITAL_MIPI:
            mipi_dsi_off();
            break;
        case LCD_DIGITAL_LVDS:
        case LCD_DIGITAL_TTL:
        default:
            break;
    }

    WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 11, 1);	//close lvds phy clk gate: 0x104c[11]
    WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1);	//disable lvds fifo

    WRITE_LCD_REG(ENCL_VIDEO_EN, 0);	//disable encl
    WRITE_LCD_CBUS_REG_BITS(HHI_VID_CLK_CNTL2, 0, 3, 1);	//disable CTS_ENCL

    WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 0, 5);	//close vclk2 gate: 0x104b[4:0]

    WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 16, 1);	//close vid2_pll gate: 0x104c[16]

    WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CNTL, 0, 30, 1);	//disable vid_pll: 0x10e0[30]
    printf("disable lcd display driver.\n");
}

static void _enable_vsync_interrupt(void)
{
	if (READ_LCD_REG(ENCL_VIDEO_EN) & 1) {
		WRITE_LCD_REG(VENC_INTCTRL, 0x200);
	}
	else{
		WRITE_LCD_REG(VENC_INTCTRL, 0x2);
	}
}

#define LCD_ENC_TST_NUM_MAX    8
static const char *lcd_enc_tst_str[] = {
	"None",        //0
	"Color Bar",   //1
	"Thin Line",   //2
	"Dot Grid",    //3
	"Gray",        //4
	"Red",         //5
	"Green",       //6
	"Blue",        //7
};

static unsigned int lcd_enc_tst[][6] = {
//tst_mode,  Y,       Cb,     Cr,    tst_en, vfifo_en
  {0,       0x200,   0x200,  0x200,   0,      1},  //0
  {1,       0x200,   0x200,  0x200,   1,      0},  //1
  {2,       0x200,   0x200,  0x200,   1,      0},  //2
  {3,       0x200,   0x200,  0x200,   1,      0},  //3
  {0,       0x200,   0x200,  0x200,   1,      0},  //4
  {0,       0x130,   0x153,  0x3fd,   1,      0},  //5
  {0,       0x256,   0x0ae,  0x055,   1,      0},  //6
  {0,       0x074,   0x3fd,  0x1ad,   1,      0},  //7
};

static void lcd_test(unsigned int num)
{
	num = (num >= LCD_ENC_TST_NUM_MAX) ? 0 : num;
	
	WRITE_LCD_REG(ENCL_TST_MDSEL, lcd_enc_tst[num][0]);
	WRITE_LCD_REG(ENCL_TST_Y, lcd_enc_tst[num][1]);
	WRITE_LCD_REG(ENCL_TST_CB, lcd_enc_tst[num][2]);
	WRITE_LCD_REG(ENCL_TST_CR, lcd_enc_tst[num][3]);
	WRITE_LCD_REG(ENCL_TST_CLRBAR_STRT, lcd_Conf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_TST_CLRBAR_WIDTH, (lcd_Conf->lcd_basic.h_active / 9));
	WRITE_LCD_REG(ENCL_TST_EN, lcd_enc_tst[num][4]);
	WRITE_LCD_REG_BITS(ENCL_VIDEO_MODE_ADV, lcd_enc_tst[num][5], 3, 1);
	
	if (num > 0)
		printf("show test pattern %d: %s (0: disable test pattern)\n", num, lcd_enc_tst_str[num]);
	else
		printf("disable test pattern (1~7: show different test pattern)\n");
}

static void print_lcd_clk_info(void)
{
    printf("vid pll clk       %uMHz\n"
           "lvds fifo clk     %uMHz\n"
           "cts encl clk      %uMHz\n\n",
           (unsigned int)clk_util_clk_msr(6), (unsigned int)clk_util_clk_msr(24), (unsigned int)clk_util_clk_msr(9));
}

static void lcd_module_enable(void)
{
	_init_lcd_driver(lcd_Conf);
	lcd_Conf->lcd_power_ctrl.power_ctrl(ON);
	_enable_vsync_interrupt();
}

static void lcd_module_disable(void)
{
	lcd_Conf->lcd_power_ctrl.power_ctrl(OFF);
	_disable_lcd_driver(lcd_Conf);
}

static void generate_clk_parameter(Lcd_Config_t *pConf)
{
    unsigned pll_n = 0, pll_m = 0, pll_od = 0, pll_frac = 0, pll_level = 0;
    unsigned vid_div_pre = 0, crt_xd = 0;

    unsigned m, n, od, div_pre, div_post, xd;
    unsigned od_sel, pre_div_sel;
    unsigned div_pre_sel_max, crt_xd_max;
    unsigned pll_vco, fout_pll, div_pre_out, div_post_out, iflogic_vid_clk_in_max;
    unsigned od_fb=0;
    unsigned int dsi_bit_rate_min=0, dsi_bit_rate_max=0;
    unsigned clk_num = 0;
    unsigned tmp;
    unsigned fin, fout;

    fin = FIN_FREQ; //kHz
    fout = pConf->lcd_timing.lcd_clk / 1000; //kHz

    switch (pConf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            div_pre_sel_max = DIV_PRE_SEL_MAX;
            div_post = 1;
            crt_xd_max = CRT_VID_DIV_MAX;
            dsi_bit_rate_min = pConf->lcd_control.mipi_config->bit_rate_min;
            dsi_bit_rate_max = pConf->lcd_control.mipi_config->bit_rate_max;
            iflogic_vid_clk_in_max = MIPI_MAX_VID_CLK_IN;
            break;
        case LCD_DIGITAL_LVDS:
            div_pre_sel_max = DIV_PRE_SEL_MAX;
            div_post = 7;
            crt_xd_max = 1;
            iflogic_vid_clk_in_max = LVDS_MAX_VID_CLK_IN;
            break;
        case LCD_DIGITAL_TTL:
            div_pre_sel_max = DIV_PRE_SEL_MAX;
            div_post = 1;
            crt_xd_max = CRT_VID_DIV_MAX;
            iflogic_vid_clk_in_max = TTL_MAX_VID_CLK_IN;
            break;
        default:
            div_pre_sel_max = DIV_PRE_SEL_MAX;
            div_post = 1;
            crt_xd_max = 1;
            iflogic_vid_clk_in_max = ENCL_MAX_CLK_IN;
            break;
    }

    switch (pConf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            if (fout < ENCL_MAX_CLK_IN) {
                for (xd = 1; xd <= crt_xd_max; xd++) {
                    div_post_out = fout * xd;
                    lcd_print("div_post_out=%d, xd=%d, fout=%d\n",div_post_out, xd, fout);
                    if (div_post_out <= CRT_VID_MAX_CLK_IN) {
                        div_pre_out = div_post_out * div_post;
                        if (div_pre_out <= DIV_POST_MAX_CLK_IN) {
                            for (pre_div_sel = 0; pre_div_sel < div_pre_sel_max; pre_div_sel++) {
                                div_pre = div_pre_table[pre_div_sel];
                                fout_pll = div_pre_out * div_pre;
                                lcd_print("pre_div_sel=%d, div_pre=%d, fout_pll=%d\n", pre_div_sel, div_pre, fout_pll);
                                if ((fout_pll <= dsi_bit_rate_max) && (fout_pll >= dsi_bit_rate_min)){
                                    for (od_sel = OD_SEL_MAX; od_sel > 0; od_sel--) {
                                        od = od_table[od_sel - 1];
                                        pll_vco = fout_pll * od;
                                        lcd_print("od_sel=%d, od=%d, pll_vco=%d\n", od_sel, od, pll_vco);
                                        if ((pll_vco >= PLL_VCO_MIN) && (pll_vco <= PLL_VCO_MAX)) {
                                            if ((pll_vco >= 2500000) && (pll_vco <= PLL_VCO_MAX)) {
                                                od_fb = 1;
                                                pll_level = 4;
                                            }
                                            else if ((pll_vco >= 2000000) && (pll_vco < 2500000)) {
                                                od_fb = 1;
                                                pll_level = 3;
                                            }
                                            else if ((pll_vco >= 1700000) && (pll_vco < 2000000)) {//special adjust
                                                od_fb = 1;
                                                pll_level = 2;
                                            }
                                            else if ((pll_vco >= PLL_VCO_MIN) && (pll_vco < 1700000)) {
                                                od_fb = 0;
                                                pll_level = 1;
                                            }
                                            n = 1;
                                            m = pll_vco / (fin * (od_fb + 1));
                                            pll_frac = (pll_vco % (fin * (od_fb + 1))) * 4096 / (fin * (od_fb + 1));
                                            pll_m = m;
                                            pll_n = n;
                                            pll_od = od_sel - 1;
                                            vid_div_pre = pre_div_sel;
                                            crt_xd = xd;
                                            clk_num = 1;
                                            lcd_print("pll_m=0x%x, pll_n=0x%x, pll_od=0x%x, vid_div_pre=0x%x, crt_xd=0x%x, pll_frac=0x%x, pll_level=%d\n",
                                                       pll_m, pll_n, pll_od, vid_div_pre, crt_xd, pll_frac, pll_level);
                                            goto generate_clk_done;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        case LCD_DIGITAL_LVDS:
        case LCD_DIGITAL_TTL:
            if (fout < ENCL_MAX_CLK_IN) {
                for (xd = 1; xd <= crt_xd_max; xd++) {
                    div_post_out = fout * xd;
                    lcd_print("div_post_out=%d, xd=%d, fout=%d\n",div_post_out, xd, fout);
                    if (div_post_out <= CRT_VID_MAX_CLK_IN) {
                        div_pre_out = div_post_out * div_post;
                        if (div_pre_out <= DIV_POST_MAX_CLK_IN) {
                            for (pre_div_sel = 0; pre_div_sel < div_pre_sel_max; pre_div_sel++) {
                                div_pre = div_pre_table[pre_div_sel];
                                fout_pll = div_pre_out * div_pre;
                                lcd_print("pre_div_sel=%d, div_pre=%d, fout_pll=%d\n", pre_div_sel, div_pre, fout_pll);
                                if (fout_pll <= DIV_PRE_MAX_CLK_IN) {
                                    for (od_sel = OD_SEL_MAX; od_sel > 0; od_sel--) {
                                        od = od_table[od_sel - 1];
                                        pll_vco = fout_pll * od;
                                        lcd_print("od_sel=%d, od=%d, pll_vco=%d\n", od_sel, od, pll_vco);
                                        if ((pll_vco >= PLL_VCO_MIN) && (pll_vco <= PLL_VCO_MAX)) {
                                            if ((pll_vco >= 2500000) && (pll_vco <= PLL_VCO_MAX)) {
                                                od_fb = 1;
                                                pll_level = 4;
                                            }
                                            else if ((pll_vco >= 2000000) && (pll_vco < 2500000)) {
                                                od_fb = 1;
                                                pll_level = 3;
                                            }
                                            else if ((pll_vco >= 1700000) && (pll_vco < 2000000)) {
                                                od_fb = 1;
                                                pll_level = 2;
                                            }
                                            else if ((pll_vco >= PLL_VCO_MIN) && (pll_vco < 1700000)) {
                                                od_fb = 0;
                                                pll_level = 1;
                                            }
                                            n = 1;
                                            m = pll_vco / (fin * (od_fb + 1));
                                            pll_frac = (pll_vco % (fin * (od_fb + 1))) * 4096 / (fin * (od_fb + 1));

                                            pll_m = m;
                                            pll_n = n;
                                            pll_od = od_sel - 1;
                                            vid_div_pre = pre_div_sel;
                                            crt_xd = xd;

                                            clk_num = 1;
                                            goto generate_clk_done;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
    }

generate_clk_done:
    if (clk_num > 0) {
        pConf->lcd_timing.pll_ctrl = (pll_od << PLL_CTRL_OD) | (pll_n << PLL_CTRL_N) | (pll_m << PLL_CTRL_M);
        pConf->lcd_timing.div_ctrl = 0x18803 | (vid_div_pre << DIV_CTRL_DIV_PRE);
        tmp = (pConf->lcd_timing.clk_ctrl & ~((0xff << CLK_CTRL_XD) | (0x7 << CLK_CTRL_LEVEL) | (0xfff << CLK_CTRL_FRAC)));
        pConf->lcd_timing.clk_ctrl = (tmp | ((crt_xd << CLK_CTRL_XD) | (pll_level << CLK_CTRL_LEVEL) | (pll_frac << CLK_CTRL_FRAC)));
    }
    else {
        pConf->lcd_timing.pll_ctrl = (1 << PLL_CTRL_OD) | (1 << PLL_CTRL_N) | (50 << PLL_CTRL_M);
        pConf->lcd_timing.div_ctrl = 0x18803 | (1 << DIV_CTRL_DIV_PRE);
        pConf->lcd_timing.clk_ctrl = (pConf->lcd_timing.clk_ctrl & ~(0xff << CLK_CTRL_XD)) | (7 << CLK_CTRL_XD);
        printf("Out of clock range, reset to default setting!\n");
    }
}

static void lcd_sync_duration(Lcd_Config_t *pConf)
{
	unsigned m, n, od, od_fb, frac, pre_div, xd, post_div;
	unsigned h_period, v_period, sync_duration;
	unsigned pll_out_clk, lcd_clk;

	m = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_M) & 0x1ff;
	n = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_N) & 0x1f;
	od = ((pConf->lcd_timing.pll_ctrl) >> PLL_CTRL_OD) & 0x3;
	od = od_table[od];
	frac = ((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_FRAC) & 0xfff;
	od_fb = ((((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_LEVEL) & 0x7) > 1) ? 1 : 0;
	pre_div = ((pConf->lcd_timing.div_ctrl) >> DIV_CTRL_DIV_PRE) & 0x7;
	pre_div = div_pre_table[pre_div];
	
	h_period = pConf->lcd_basic.h_period;
	v_period = pConf->lcd_basic.v_period;

	switch(pConf->lcd_basic.lcd_type) {
		case LCD_DIGITAL_MIPI:
			xd = ((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_XD) & 0xff;
			post_div = 1;
			break;
		case LCD_DIGITAL_LVDS:
			xd = 1;
			post_div = 7;
			break;
		case LCD_DIGITAL_TTL:
			xd = ((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_XD) & 0xff;
			post_div = 1;
			break;
		default:
			xd = ((pConf->lcd_timing.clk_ctrl) >> CLK_CTRL_XD) & 0xff;
			post_div = 1;
			break;
	}
	
	pll_out_clk = (frac * (od_fb + 1) * FIN_FREQ) / 4096;
	pll_out_clk = ((m * (od_fb + 1) * FIN_FREQ + pll_out_clk) / (n * od)) * 1000;
	lcd_clk = pll_out_clk  / (pre_div * post_div * xd);
	if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_MIPI) {
		pConf->lcd_control.mipi_config->bit_rate = pll_out_clk;
		printf("mipi-dsi bit rate: %d.%03dMHz\n", (pConf->lcd_control.mipi_config->bit_rate / 1000000), ((pConf->lcd_control.mipi_config->bit_rate / 1000) % 1000));
	}
	pConf->lcd_timing.lcd_clk = lcd_clk;
	sync_duration = ((lcd_clk / h_period) * 100) / v_period;
	sync_duration = (sync_duration + 5) / 10;
	
	pConf->lcd_timing.sync_duration_num = sync_duration;
	pConf->lcd_timing.sync_duration_den = 10;
	printf("lcd_clk=%u.%03uMHz, frame_rate=%u.%uHz.\n\n", (lcd_clk / 1000000), ((lcd_clk / 1000) % 1000), (sync_duration / pConf->lcd_timing.sync_duration_den), ((sync_duration * 10 / pConf->lcd_timing.sync_duration_den) % 10));
}

static void lcd_tcon_config(Lcd_Config_t *pConf)
{
	unsigned short de_hstart, de_vstart;
	unsigned short hstart, hend, vstart, vend;
	unsigned short h_delay = 0;
	unsigned short h_offset = 0, v_offset = 0, vsync_h_phase=0;
	
	switch (pConf->lcd_basic.lcd_type) {
		case LCD_DIGITAL_MIPI:
			h_delay = MIPI_DELAY;
			break;
		case LCD_DIGITAL_LVDS:
			h_delay = LVDS_DELAY;
			break;
		case LCD_DIGITAL_TTL:
			h_delay = TTL_DELAY;
			break;
		default:
			h_delay = 0;
			break;
	}
#if 0
	h_offset = (pConf->lcd_timing.h_offset & 0xffff);
	v_offset = (pConf->lcd_timing.v_offset & 0xffff);
	if ((pConf->lcd_timing.h_offset >> 31) & 1)
		de_hstart = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period + h_delay + h_offset) % pConf->lcd_basic.h_period;
	else
		de_hstart = (pConf->lcd_timing.video_on_pixel + pConf->lcd_basic.h_period + h_delay - h_offset) % pConf->lcd_basic.h_period;
	if ((pConf->lcd_timing.v_offset >> 31) & 1)
		de_vstart = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period + v_offset) % pConf->lcd_basic.v_period;
	else
		de_vstart = (pConf->lcd_timing.video_on_line + pConf->lcd_basic.v_period - v_offset) % pConf->lcd_basic.v_period;
	
	hstart = (de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp - pConf->lcd_timing.hsync_width) % pConf->lcd_basic.h_period;
	hend = (de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp) % pConf->lcd_basic.h_period;
	pConf->lcd_timing.hs_hs_addr = hstart;
	pConf->lcd_timing.hs_he_addr = hend;
	pConf->lcd_timing.hs_vs_addr = 0;
	pConf->lcd_timing.hs_ve_addr = pConf->lcd_basic.v_period - 1;
	
	vsync_h_phase = (pConf->lcd_timing.vsync_h_phase & 0xffff);
	if ((pConf->lcd_timing.vsync_h_phase >> 31) & 1) //negative
		vsync_h_phase = (hstart + pConf->lcd_basic.h_period - vsync_h_phase) % pConf->lcd_basic.h_period;
	else	//positive
		vsync_h_phase = (hstart + pConf->lcd_basic.h_period + vsync_h_phase) % pConf->lcd_basic.h_period;
	pConf->lcd_timing.vs_hs_addr = vsync_h_phase;
	pConf->lcd_timing.vs_he_addr = vsync_h_phase;
	vstart = (de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp - pConf->lcd_timing.vsync_width) % pConf->lcd_basic.v_period;
	vend = (de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp) % pConf->lcd_basic.v_period;
	pConf->lcd_timing.vs_vs_addr = vstart;
	pConf->lcd_timing.vs_ve_addr = vend;
	
	pConf->lcd_timing.de_hs_addr = de_hstart;
	pConf->lcd_timing.de_he_addr = (de_hstart + pConf->lcd_basic.h_active) % pConf->lcd_basic.h_period;
	pConf->lcd_timing.de_vs_addr = de_vstart;
	pConf->lcd_timing.de_ve_addr = (de_vstart + pConf->lcd_basic.v_active - 1) % pConf->lcd_basic.v_period;
#else
    pConf->lcd_timing.video_on_pixel = pConf->lcd_basic.h_period - pConf->lcd_basic.h_active - 1 -h_delay;
    pConf->lcd_timing.video_on_line = pConf->lcd_basic.v_period - pConf->lcd_basic.v_active;

    h_offset = (pConf->lcd_timing.h_offset & 0xffff);
    v_offset = (pConf->lcd_timing.v_offset & 0xffff);
    if ((pConf->lcd_timing.h_offset >> 31) & 1)
        de_hstart = (pConf->lcd_basic.h_period - pConf->lcd_basic.h_active - 1 + pConf->lcd_basic.h_period - h_offset) % pConf->lcd_basic.h_period;
    else
        de_hstart = (pConf->lcd_basic.h_period - pConf->lcd_basic.h_active - 1 + h_offset) % pConf->lcd_basic.h_period;
    if ((pConf->lcd_timing.v_offset >> 31) & 1)
        de_vstart = (pConf->lcd_basic.v_period - pConf->lcd_basic.v_active + pConf->lcd_basic.v_period - v_offset) % pConf->lcd_basic.v_period;
    else
        de_vstart = (pConf->lcd_basic.v_period - pConf->lcd_basic.v_active + v_offset) % pConf->lcd_basic.v_period;

    hstart = (de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp - pConf->lcd_timing.hsync_width) % pConf->lcd_basic.h_period;
    hend = (de_hstart + pConf->lcd_basic.h_period - pConf->lcd_timing.hsync_bp) % pConf->lcd_basic.h_period;
    pConf->lcd_timing.hs_hs_addr = hstart;
    pConf->lcd_timing.hs_he_addr = hend;
    pConf->lcd_timing.hs_vs_addr = 0;
    pConf->lcd_timing.hs_ve_addr = pConf->lcd_basic.v_period - 1;

    vsync_h_phase = (pConf->lcd_timing.vsync_h_phase & 0xffff);
    if ((pConf->lcd_timing.vsync_h_phase >> 31) & 1) //negative
        vsync_h_phase = (hstart + pConf->lcd_basic.h_period - vsync_h_phase) % pConf->lcd_basic.h_period;
    else //positive
        vsync_h_phase = (hstart + pConf->lcd_basic.h_period + vsync_h_phase) % pConf->lcd_basic.h_period;
    pConf->lcd_timing.vs_hs_addr = vsync_h_phase;
    pConf->lcd_timing.vs_he_addr = vsync_h_phase;
    vstart = (de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp - pConf->lcd_timing.vsync_width) % pConf->lcd_basic.v_period;
    vend = (de_vstart + pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp) % pConf->lcd_basic.v_period;
    pConf->lcd_timing.vs_vs_addr = vstart;
    pConf->lcd_timing.vs_ve_addr = vend;

    pConf->lcd_timing.de_hs_addr = de_hstart;
    pConf->lcd_timing.de_he_addr = (de_hstart + pConf->lcd_basic.h_active) % pConf->lcd_basic.h_period;
    pConf->lcd_timing.de_vs_addr = de_vstart;
    pConf->lcd_timing.de_ve_addr = (de_vstart + pConf->lcd_basic.v_active - 1) % pConf->lcd_basic.v_period;
#endif

    if (pConf->lcd_timing.vso_user == 0) {
        //pConf->lcd_timing.vso_hstart = pConf->lcd_timing.vs_hs_addr;
        pConf->lcd_timing.vso_vstart = pConf->lcd_timing.vs_vs_addr;
    }

    //lcd_print("hs_hs_addr=%d, hs_he_addr=%d, hs_vs_addr=%d, hs_ve_addr=%d\n", pConf->lcd_timing.hs_hs_addr, pConf->lcd_timing.hs_he_addr, pConf->lcd_timing.hs_vs_addr, pConf->lcd_timing.hs_ve_addr);
    //lcd_print("vs_hs_addr=%d, vs_he_addr=%d, vs_vs_addr=%d, vs_ve_addr=%d\n", pConf->lcd_timing.vs_hs_addr, pConf->lcd_timing.vs_he_addr, pConf->lcd_timing.vs_vs_addr, pConf->lcd_timing.vs_ve_addr);
    //lcd_print("de_hs_addr=%d, de_he_addr=%d, de_vs_addr=%d, de_ve_addr=%d\n", pConf->lcd_timing.de_hs_addr, pConf->lcd_timing.de_he_addr, pConf->lcd_timing.de_vs_addr, pConf->lcd_timing.de_ve_addr);
}

static void lcd_control_config_pre(Lcd_Config_t *pConf)
{
    unsigned ss_level;

    if (pConf->lcd_timing.lcd_clk < 200) {//prepare refer clock for frame_rate setting
        pConf->lcd_timing.lcd_clk = (pConf->lcd_timing.lcd_clk * pConf->lcd_basic.h_period * pConf->lcd_basic.v_period);
    }

    ss_level = ((pConf->lcd_timing.clk_ctrl >> CLK_CTRL_SS) & 0xf);
    ss_level = ((ss_level >= SS_LEVEL_MAX) ? (SS_LEVEL_MAX - 1) : ss_level);

    switch (pConf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            ss_level = ((ss_level > 0) ? 1 : 0);
            set_mipi_dsi_control_config(pConf);
            break;
        case LCD_DIGITAL_LVDS:
            if (pConf->lcd_control.lvds_config->lvds_repack_user == 0) {
                if (pConf->lcd_basic.lcd_bits == 6)
                    pConf->lcd_control.lvds_config->lvds_repack = 0;
                else
                    pConf->lcd_control.lvds_config->lvds_repack = 1;
            }
            break;
        case LCD_DIGITAL_TTL:
            if (pConf->lcd_basic.lcd_bits != 6) {
                pConf->lcd_basic.lcd_bits = 6;
                printf("lcd change to 6bit for ttl support!\n");
            }
            break;
        default:
            break;
    }
    pConf->lcd_timing.clk_ctrl = ((pConf->lcd_timing.clk_ctrl & (~(0xf << CLK_CTRL_SS))) | (ss_level << CLK_CTRL_SS));
}

//for special interface config after clk setting
static void lcd_control_config_post(Lcd_Config_t *pConf)
{
    switch (pConf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            set_mipi_dsi_control_config_post(pConf);
            break;
        default:
            break;
    }
}

//****************************************
static unsigned char dsi_init_on_table[DSI_INIT_ON_MAX]={0xff,0xff};
static unsigned char dsi_init_off_table[DSI_INIT_OFF_MAX]={0xff,0xff};
static DSI_Config_t lcd_mipi_config = {
    .lane_num = 4,
    .bit_rate_min = 0,
    .bit_rate_max = 0,
    .factor_numerator = 0,
    .factor_denominator = 10,
    .transfer_ctrl = 0,
    .dsi_init_on = &dsi_init_on_table[0],
    .dsi_init_off = &dsi_init_off_table[0],
};

static LVDS_Config_t lcd_lvds_config = {
    .lvds_vswing = 1,
    .lvds_repack_user = 0,
    .lvds_repack = 0,
    .pn_swap = 0,
};

static TTL_Config_t lcd_ttl_config = {
    .rb_swap = 0,
    .bit_swap = 0,
};

static Lcd_Config_t lcd_config = {
    .lcd_timing = {
        .lcd_clk = 40000000,
        .clk_ctrl = ((1 << CLK_CTRL_AUTO) | (0 << CLK_CTRL_SS)),
        .hvsync_valid = 1,
        .de_valid = 1,
        .pol_ctrl = ((0 << POL_CTRL_CLK) | (1 << POL_CTRL_DE) | (0 << POL_CTRL_VS) | (0 << POL_CTRL_HS)),
    },
    .lcd_effect = {
        .rgb_base_addr = 0xf0,
        .rgb_coeff_addr = 0x74a,
        .dith_user = 0,
        .vadj_brightness = 0x0,
        .vadj_contrast = 0x80,
        .vadj_saturation = 0x100,
        .gamma_ctrl = ((0 << GAMMA_CTRL_REVERSE) | (1 << LCD_GAMMA_EN)),
        .gamma_r_coeff = 100,
        .gamma_g_coeff = 100,
        .gamma_b_coeff = 100,
    },
    .lcd_control = {
        .mipi_config = &lcd_mipi_config,
        .lvds_config = &lcd_lvds_config,
        .ttl_config = &lcd_ttl_config,
    },
    .lcd_power_ctrl = {
        .power_on_step = 0,
        .power_off_step = 0,
        .power_ctrl = NULL,
    },
};

Lcd_Config_t* get_lcd_config(void)
{
    return &lcd_config;
}
//****************************************

static void lcd_config_assign(Lcd_Config_t *pConf)
{
    pConf->lcd_timing.vso_hstart = 10; //for video process
    pConf->lcd_timing.vso_vstart = 10; //for video process
    pConf->lcd_timing.vso_user = 0; //use default config

    pConf->lcd_power_ctrl.ports_ctrl = lcd_ports_ctrl;
    pConf->lcd_power_ctrl.power_ctrl_video = lcd_power_ctrl_video;

    pConf->lcd_misc_ctrl.module_enable = lcd_module_enable;
    pConf->lcd_misc_ctrl.module_disable = lcd_module_disable;
    pConf->lcd_misc_ctrl.lcd_test = lcd_test;
    pConf->lcd_misc_ctrl.print_version = print_lcd_driver_version;
    pConf->lcd_misc_ctrl.print_clk = print_lcd_clk_info;
}

void lcd_config_init(Lcd_Config_t *pConf)
{
    lcd_control_config_pre(pConf);

    if ((pConf->lcd_timing.clk_ctrl >> CLK_CTRL_AUTO) & 1) {
        printf("Auto generate clock parameters.\n");
        generate_clk_parameter(pConf);
    }
    else {
        printf("Custome clock parameters.\n");
    }
    printf("pll_ctrl=0x%x, div_ctrl=0x%x, clk_ctrl=0x%x.\n", pConf->lcd_timing.pll_ctrl, pConf->lcd_timing.div_ctrl, pConf->lcd_timing.clk_ctrl);

    lcd_sync_duration(pConf);
    lcd_tcon_config(pConf);

    lcd_control_config_post(pConf);
}

void lcd_config_probe(Lcd_Config_t *pConf)
{
    lcd_Conf = pConf;
    lcd_config_assign(pConf);

    switch (pConf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            dsi_probe(pConf);
            break;
        default:
            break;
    }
}

void lcd_config_remove(void)
{
    switch (lcd_Conf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_MIPI:
            dsi_remove();
            break;
        default:
            break;
    }
}

