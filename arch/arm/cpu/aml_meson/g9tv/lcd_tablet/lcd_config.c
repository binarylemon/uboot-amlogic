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

#define VPP_OUT_SATURATE            (1 << 0)

static Lcd_Config_t *lcd_conf;
static unsigned char lcd_gamma_init_err = 0;

#define SS_LEVEL_MAX	1
static const char *lcd_ss_level_table[]={
	"0",
	"0.5%",
	"1%",
	"1.5%",
	"2%",
};

#define LVDS_VSWING_LEVEL_MAX  5
static unsigned int lvds_vswing_ctrl[] = {
/* vswing_ctrl   level   voltage */
	0x1,   /* 0      0.2V */
	0x3,   /* 1      0.4V */
	0x5,   /* 2      0.6V */
	0x6,   /* 3      0.7V */
	0x7,   /* 4      0.8V */
};

static void print_lcd_driver_version(void)
{
    printf("lcd driver version: %s%s\n\n", LCD_DRV_DATE, LCD_DRV_TYPE);
}

static void lcd_ports_ctrl_lvds(Bool_t status)
{
	unsigned int phy_reg, phy_bit, phy_width;
	unsigned int lane_cnt;
	LVDS_Config_t *lconf;

	lconf = lcd_conf->lcd_control.lvds_config;
	if (status) {
		WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 3, 1); /*enable lvds fifo*/
		phy_reg = HHI_DIF_CSI_PHY_CNTL3;
		phy_bit = BIT_PHY_LANE;
		phy_width = WIDTH_PHY_LANE;
		if (lconf->port_sel == LVDS_PORT_A)
			lane_cnt = LVDS_PORT_A;
		else if (lconf->port_sel == LVDS_PORT_B)
			lane_cnt = LVDS_PORT_B;
		else
			lane_cnt = LVDS_PORT_AB;
		WRITE_LCD_CBUS_REG_BITS(phy_reg, lane_cnt, phy_bit, phy_width);
	} else {
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, 0x0);
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x0);
		WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x0);
	}

	lcd_print("%s: %s\n", __FUNCTION__, (status ? "ON" : "OFF"));
}

static unsigned pinmux_data_set[][2]={
    {PERIPHS_PIN_MUX_0,0x00000015},//RGB[7:2]
    {PERIPHS_PIN_MUX_0,0x0000002a},//RGB[1:0]
};
static unsigned pinmux_data_clr[][2]={
    {PERIPHS_PIN_MUX_0,0x00000040},//RGB[7:2]
    {PERIPHS_PIN_MUX_1,0x0003005f},//RGB[7:2]
    {PERIPHS_PIN_MUX_2,0x00000008},//RGB[7:2]
    {PERIPHS_PIN_MUX_3,0x7844ffc0},//RGB[7:2]
    {PERIPHS_PIN_MUX_4,0x0003fc00},//RGB[7:2]
    {PERIPHS_PIN_MUX_5,0x00003000},//RGB[7:2]
    {PERIPHS_PIN_MUX_7,0x8000fcfc},//RGB[7:2]
    {PERIPHS_PIN_MUX_8,0x0000000c},//RGB[7:2]
    {PERIPHS_PIN_MUX_9,0x00180000},//RGB[7:2]
    {PERIPHS_PIN_MUX_3,0x00030000},//RGB[1:0]
    {PERIPHS_PIN_MUX_5,0x00004c00},//RGB[1:0]
    {PERIPHS_PIN_MUX_6,0x00030000},//RGB[1:0]
    {PERIPHS_PIN_MUX_7,0x00030303},//RGB[1:0]
    {PERIPHS_PIN_MUX_8,0x0e000033},//RGB[1:0]
    {PERIPHS_PIN_MUX_9,0x00064000},//RGB[1:0]
};
static unsigned pinmux_data_set_table[][3]={
    {0,0xff},  //6bit index
    {0,1,0xff},//8bit index
};
static unsigned pinmux_data_clr_table[][16]={
    {0,1,2,3,4,5,6,7,8,0xff},                 //6bit index
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,0xff},//8bit index
};

static unsigned pinmux_tcon_set[][2]={
    {PERIPHS_PIN_MUX_8,0x00400000},//clk
    {PERIPHS_PIN_MUX_8,0x10000000},//DE
    {PERIPHS_PIN_MUX_8,0x01800000},//sync
};
static unsigned pinmux_tcon_clr[][2]={
    {PERIPHS_PIN_MUX_0,0x00000080},//clk
    {PERIPHS_PIN_MUX_3,0x02000000},//clk
    {PERIPHS_PIN_MUX_4,0x00080000},//clk
    {PERIPHS_PIN_MUX_7,0x40000000},//clk
    {PERIPHS_PIN_MUX_0,0x00000400},//DE
    {PERIPHS_PIN_MUX_4,0x00040000},//DE
    {PERIPHS_PIN_MUX_8,0x00080000},//DE
    {PERIPHS_PIN_MUX_0,0x000c0300},//sync
    {PERIPHS_PIN_MUX_1,0x000003a0},//sync
    {PERIPHS_PIN_MUX_4,0x00300000},//sync
};
static unsigned pinmux_tcon_set_table[][4]={
    {0,1,0xff},  //DE index
    {0,2,0xff},  //Sync index
    {0,1,2,0xff},//DE+Sync index
};
static unsigned pinmux_tcon_clr_table[][11]={
    {0,1,2,3,4,5,6,0xff},      //DE index
    {0,1,2,3,7,8,9,0xff},      //Sync index
    {0,1,2,3,4,5,6,7,8,9,0xff},//DE+Sync index
};

static unsigned gpio_data_set[][2]={
    {PREG_PAD_GPIO4_EN_N, 0x00fcfcfc},//RGB[7:2]
    {PREG_PAD_GPIO4_EN_N, 0x00030303},//RGB[1:0]
};
static unsigned gpio_tcon_set[][2]={
    {PREG_PAD_GPIO4_EN_N, 0x04000000},//clk
    {PREG_PAD_GPIO4_EN_N, 0x08000000},//DE
    {PREG_PAD_GPIO4_EN_N, 0x03000000},//Sync
};
static unsigned gpio_data_set_table[][3]={
    {0,0xff},  //6bit index
    {0,1,0xff},//8bit index
};
static unsigned gpio_tcon_set_table[][4]={
    {0,1,0xff},  //DE index
    {0,2,0xff},  //Sync index
    {0,1,2,0xff},//DE+Sync index
};

static void lcd_ports_ctrl_ttl(Bool_t status)
{
	unsigned *pin_tcon_set, *pin_data_set;
	unsigned *pin_tcon_clr, *pin_data_clr;
	unsigned *gpio_tcon, *gpio_data;
	unsigned pin_reg;
	int i;

	//tcon pin
	if ((lcd_conf->lcd_timing.de_valid == 1) && (lcd_conf->lcd_timing.hvsync_valid == 0)) {
		pin_tcon_set = &pinmux_tcon_set_table[0][0];
		pin_tcon_clr = &pinmux_tcon_clr_table[0][0];
		gpio_tcon = &gpio_tcon_set_table[0][0];
	}
	else if ((lcd_conf->lcd_timing.de_valid == 0) && (lcd_conf->lcd_timing.hvsync_valid == 1)) {
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
	if (lcd_conf->lcd_basic.lcd_bits == 8) {
		pin_data_set = &pinmux_data_set_table[1][0];
		pin_data_clr = &pinmux_data_clr_table[1][0];
		gpio_data = &gpio_data_set_table[1][0];
	} else {
		pin_data_set = &pinmux_data_set_table[0][0];
		pin_data_clr = &pinmux_data_clr_table[0][0];
		gpio_data = &gpio_data_set_table[0][0];
	}
	if (status) {
		i = 0;
		while (i < 0xff) {//pinmux_tcon_set
			if (pin_tcon_set[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_tcon_set[pin_tcon_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | pinmux_tcon_set[pin_tcon_set[i]][1]));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//pinmux_tcon_clr
			if (pin_tcon_clr[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_tcon_clr[pin_tcon_clr[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_tcon_clr[pin_tcon_clr[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//pinmux_data_set
			if (pin_data_set[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_data_set[pin_data_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | pinmux_data_set[pin_data_set[i]][1]));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//pinmux_data_clr
			if (pin_data_clr[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_data_clr[pin_data_clr[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_data_clr[pin_data_clr[i]][1])));
				i++;
			}
		}
	} else {
		i = 0;
		while (i < 0xff) {//pinmux_data_set
			if (pin_data_set[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_data_set[pin_data_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_data_set[pin_data_set[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//pinmux_tcon_set
			if (pin_tcon_set[i] == 0xff) {
				break;
			} else {
				pin_reg = pinmux_tcon_set[pin_tcon_set[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) & ~(pinmux_tcon_set[pin_tcon_set[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//gpio_data_set
			if (gpio_data[i] == 0xff) {
				break;
			} else {
				pin_reg = gpio_data_set[gpio_data[i]][0];
				WRITE_LCD_CBUS_REG(pin_reg, (READ_LCD_CBUS_REG(pin_reg) | (gpio_data_set[gpio_data[i]][1])));
				i++;
			}
		}
		i = 0;
		while (i < 0xff) {//gpio_tcon_set
			if (gpio_tcon[i] == 0xff) {
				break;
			} else {
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
	switch (lcd_conf->lcd_basic.lcd_type) {
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
	} else {
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
	write_gamma_table(lcd_conf->lcd_effect.GammaTableR, GAMMA_SEL_R, lcd_conf->lcd_effect.gamma_r_coeff, ((lcd_conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));
	write_gamma_table(lcd_conf->lcd_effect.GammaTableG, GAMMA_SEL_G, lcd_conf->lcd_effect.gamma_g_coeff, ((lcd_conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));
	write_gamma_table(lcd_conf->lcd_effect.GammaTableB, GAMMA_SEL_B, lcd_conf->lcd_effect.gamma_b_coeff, ((lcd_conf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_REVERSE) & 1));

	if (lcd_gamma_init_err) {
		WRITE_LCD_REG_BITS(L_GAMMA_CNTL_PORT, 0, 0, 1);
		printf("[warning]: write gamma table error, gamma table disabled\n");
	} else
		WRITE_LCD_REG_BITS(L_GAMMA_CNTL_PORT, gamma_en, 0, 1);
}

static void set_tcon_lcd(Lcd_Config_t *pconf)
{
	Lcd_Timing_t *tcon_adr = &(pconf->lcd_timing);
	unsigned hs_pol_adj, vs_pol_adj;

	lcd_print("%s\n", __FUNCTION__);

	set_gamma_table_lcd(((pconf->lcd_effect.gamma_ctrl >> GAMMA_CTRL_EN) & 1));

	WRITE_LCD_REG(L_RGB_BASE_ADDR,  pconf->lcd_effect.rgb_base_addr);
	WRITE_LCD_REG(L_RGB_COEFF_ADDR, pconf->lcd_effect.rgb_coeff_addr);
	if (pconf->lcd_effect.dith_user) {
		WRITE_LCD_REG(L_DITH_CNTL_ADDR,  pconf->lcd_effect.dith_cntl_addr);
	} else {
		if (pconf->lcd_basic.lcd_bits == 8)
			WRITE_LCD_REG(L_DITH_CNTL_ADDR,  0x400);
		else
			WRITE_LCD_REG(L_DITH_CNTL_ADDR,  0x600);
	}

	WRITE_LCD_REG(L_POL_CNTL_ADDR, (((pconf->lcd_timing.pol_ctrl >> POL_CTRL_CLK) & 1) << LCD_CPH1_POL));

	hs_pol_adj = (((pconf->lcd_timing.pol_ctrl >> POL_CTRL_HS) & 1) ? 0 : 1); //1 for low active, 0 for high active.
	vs_pol_adj = (((pconf->lcd_timing.pol_ctrl >> POL_CTRL_VS) & 1) ? 0 : 1); //1 for low active, 0 for high active
	WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((0 << LCD_DE_POL) | (vs_pol_adj << LCD_VS_POL) | (hs_pol_adj << LCD_HS_POL)))); //adjust hvsync pol
	WRITE_LCD_REG(L_POL_CNTL_ADDR, (READ_LCD_REG(L_POL_CNTL_ADDR) | ((1 << LCD_TCON_DE_SEL) | (1 << LCD_TCON_VS_SEL) | (1 << LCD_TCON_HS_SEL)))); //enable tcon DE, Hsync, Vsync

	//DE signal
	WRITE_LCD_REG(L_DE_HS_ADDR,    tcon_adr->de_hs_addr);
	WRITE_LCD_REG(L_DE_HE_ADDR,    tcon_adr->de_he_addr);
	WRITE_LCD_REG(L_DE_VS_ADDR,    tcon_adr->de_vs_addr);
	WRITE_LCD_REG(L_DE_VE_ADDR,    tcon_adr->de_ve_addr);
	/* DE signal for TTL */
	WRITE_LCD_REG(L_OEV1_HS_ADDR,  tcon_adr->de_hs_addr);
	WRITE_LCD_REG(L_OEV1_HE_ADDR,  tcon_adr->de_he_addr);
	WRITE_LCD_REG(L_OEV1_VS_ADDR,  tcon_adr->de_vs_addr);
	WRITE_LCD_REG(L_OEV1_VE_ADDR,  tcon_adr->de_ve_addr);
	WRITE_LCD_REG(L_OEH_HS_ADDR,  tcon_adr->de_hs_addr);
	WRITE_LCD_REG(L_OEH_HE_ADDR,  tcon_adr->de_he_addr);
	WRITE_LCD_REG(L_OEH_VS_ADDR,  tcon_adr->de_vs_addr);
	WRITE_LCD_REG(L_OEH_VE_ADDR,  tcon_adr->de_ve_addr);

	//Hsync signal
	WRITE_LCD_REG(L_HSYNC_HS_ADDR,  tcon_adr->hs_hs_addr);
	WRITE_LCD_REG(L_HSYNC_HE_ADDR,  tcon_adr->hs_he_addr);
	WRITE_LCD_REG(L_HSYNC_VS_ADDR,  tcon_adr->hs_vs_addr);
	WRITE_LCD_REG(L_HSYNC_VE_ADDR,  tcon_adr->hs_ve_addr);
	if (hs_pol_adj == 0) {
		WRITE_LCD_REG(L_STH1_HS_ADDR, tcon_adr->hs_hs_addr);
		WRITE_LCD_REG(L_STH1_HE_ADDR, tcon_adr->hs_he_addr);
	} else {
		WRITE_LCD_REG(L_STH1_HS_ADDR, tcon_adr->hs_he_addr);
		WRITE_LCD_REG(L_STH1_HE_ADDR, tcon_adr->hs_hs_addr);
	}
	WRITE_LCD_REG(L_STH1_VS_ADDR, tcon_adr->hs_vs_addr);
	WRITE_LCD_REG(L_STH1_VE_ADDR, tcon_adr->hs_ve_addr);

	//Vsync signal
	WRITE_LCD_REG(L_VSYNC_HS_ADDR,  tcon_adr->vs_hs_addr);
	WRITE_LCD_REG(L_VSYNC_HE_ADDR,  tcon_adr->vs_he_addr);
	WRITE_LCD_REG(L_VSYNC_VS_ADDR,  tcon_adr->vs_vs_addr);
	WRITE_LCD_REG(L_VSYNC_VE_ADDR,  tcon_adr->vs_ve_addr);
	WRITE_LCD_REG(L_STV1_HS_ADDR, tcon_adr->vs_hs_addr);
	WRITE_LCD_REG(L_STV1_HE_ADDR, tcon_adr->vs_he_addr);
	if (vs_pol_adj == 0) {
		WRITE_LCD_REG(L_STV1_VS_ADDR, tcon_adr->vs_vs_addr);
		WRITE_LCD_REG(L_STV1_VE_ADDR, tcon_adr->vs_ve_addr);
	} else {
		WRITE_LCD_REG(L_STV1_VS_ADDR, tcon_adr->vs_ve_addr);
		WRITE_LCD_REG(L_STV1_VE_ADDR, tcon_adr->vs_vs_addr);
	}

	WRITE_LCD_REG(L_INV_CNT_ADDR,       0);
	WRITE_LCD_REG(L_TCON_MISC_SEL_ADDR, ((1 << LCD_STV1_SEL) | (1 << LCD_STV2_SEL)));

	WRITE_LCD_REG(VPP_MISC, READ_LCD_REG(VPP_MISC) & ~(VPP_OUT_SATURATE));
}

static void lcd_set_pll(unsigned int pll_reg, unsigned int clk_ctrl_reg)
{
	unsigned m, n, od1, od2, od3, frac;
	int wait_loop = PLL_WAIT_LOCK_CNT;
	unsigned pll_lock = 0;
	unsigned pll_ctrl, pll_ctrl2;

	lcd_print("%s\n", __func__);

	m = (pll_reg >> PLL_CTRL_M) & 0x1ff;
	n = (pll_reg >> PLL_CTRL_N) & 0x1f;
	od1 = (pll_reg >> PLL_CTRL_OD1) & 0x3;
	od2 = (pll_reg >> PLL_CTRL_OD2) & 0x3;
	od3 = (pll_reg >> PLL_CTRL_OD3) & 0x3;
	frac = (clk_ctrl_reg >> CLK_CTRL_FRAC) & 0xfff;

	pll_ctrl = ((1 << 30) | (n << 9) | (m << 0));
	pll_ctrl2 = ((od1 << 16) | (od2 << 22) | (od3 << 18));
	if (frac > 0)
		pll_ctrl2 |= ((1 << 14) | (frac << 0));

	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL, pll_ctrl | (1 << 28));
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL2, pll_ctrl2);
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL3, 0x135c5091);
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL4, 0x801da72c);
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL5, 0x71486900);
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL6, 0x00000a55);
	WRITE_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL, pll_ctrl);

	do {
		udelay(50);
		pll_lock = (READ_LCD_CBUS_REG(HHI_HDMI_PLL_CNTL) >> 31) & 0x1;
		wait_loop--;
	} while ((pll_lock == 0) && (wait_loop > 0));
	if (wait_loop == 0)
		printf("[error]: hdmi pll lock failed\n");
}

static unsigned int lcd_clk_div_table[][3] = {
	/* divider,        shift_val,  shift_sel */
	{CLK_DIV_SEL_1,    0xffff,     0,},
	{CLK_DIV_SEL_2,    0x0aaa,     0,},
	{CLK_DIV_SEL_3,    0x0db6,     0,},
	{CLK_DIV_SEL_3p5,  0x36cc,     1,},
	{CLK_DIV_SEL_3p75, 0x6666,     2,},
	{CLK_DIV_SEL_4,    0x0ccc,     0,},
	{CLK_DIV_SEL_5,    0x739c,     2,},
	{CLK_DIV_SEL_6,    0x0e38,     0,},
	{CLK_DIV_SEL_6p25, 0x0000,     3,},
	{CLK_DIV_SEL_7,    0x3c78,     1,},
	{CLK_DIV_SEL_7p5,  0x78f0,     2,},
	{CLK_DIV_SEL_12,   0x0fc0,     0,},
	{CLK_DIV_SEL_14,   0x3f80,     1,},
	{CLK_DIV_SEL_15,   0x7f80,     2,},
	{CLK_DIV_SEL_2p5,  0x5294,     2,},
	{CLK_DIV_SEL_MAX,  0xffff,     0,},
};

static void lcd_set_clk_div(unsigned long vid_div_reg)
{
	unsigned int  clk_div;
	unsigned int shift_val, shift_sel;
	int i;
	lcd_print("%s\n", __func__);

	clk_div = (vid_div_reg >> DIV_CTRL_CLK_DIV) & 0xf;

	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 19, 1);
	udelay(5);

	/* Disable the div output clock */
	WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 19, 1);
	WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 15, 1);

	i = 0;
	while (lcd_clk_div_table[i][0] != CLK_DIV_SEL_MAX) {
		if (clk_div == lcd_clk_div_table[i][0])
			break;
		i++;
	}
	if (lcd_clk_div_table[i][0] == CLK_DIV_SEL_MAX)
		printf("invalid clk divider\n");
	shift_val = lcd_clk_div_table[i][1];
	shift_sel = lcd_clk_div_table[i][2];

	if (shift_val == 0xffff) { /* if divide by 1 */
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 1, 18, 1);
	} else {
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 16, 2);
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 15, 1);
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 0, 14);

		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, shift_sel, 16, 2);
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 1, 15, 1);
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, shift_val, 0, 14);
		WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 0, 15, 1);
	}
	/* Enable the final output clock */
	WRITE_LCD_CBUS_REG_BITS(HHI_VID_PLL_CLK_DIV, 1, 19, 1);
}

static void lcd_set_vclk_crt(unsigned int clk_ctrl_reg)
{
	unsigned int xd;
	lcd_print("%s\n", __func__);

	xd = (clk_ctrl_reg >> CLK_CTRL_XD) & 0xff;
	/* setup the XD divider value */
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, (xd-1), 0, 8);
	udelay(5);
	/* Bit[18:16] - v2_cntl_clk_in_sel */
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 16, 3);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 19, 1);
	udelay(2);

	/* [15:12] encl_clk_sel, select vclk2_div1 */
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, 8, 12, 4);
	/* release vclk2_div_reset and enable vclk2_div */
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_DIV, 1, 16, 2);
	udelay(5);

	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 0, 1);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 1, 15, 1);
	udelay(10);
	WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 15, 1);
	udelay(5);

	WRITE_LCD_CBUS_REG_BITS(HHI_VID_CLK_CNTL2, 1, 3, 1);
}

static void set_vclk_lcd(Lcd_Config_t *pconf)
{
	unsigned pll_reg, div_reg, clk_reg;
	int xd;
	int lcd_type;
	unsigned long flags = 0;

	lcd_print("%s\n", __func__);

	pll_reg = pconf->lcd_timing.pll_ctrl;
	div_reg = pconf->lcd_timing.div_ctrl;
	clk_reg = pconf->lcd_timing.clk_ctrl;
	xd = (clk_reg >> CLK_CTRL_XD) & 0xff;

	lcd_type = pconf->lcd_basic.lcd_type;

	switch (lcd_type) {
	case LCD_DIGITAL_LVDS:
		xd = 1;
		break;
	default:
		break;
	}
	clk_reg = (pconf->lcd_timing.clk_ctrl & ~(0xff << CLK_CTRL_XD)) |
		(xd << CLK_CTRL_XD);

	lcd_set_pll(pll_reg, clk_reg);
	lcd_set_clk_div(div_reg);
	lcd_set_vclk_crt(clk_reg);
}

static void set_venc_lcd(Lcd_Config_t *pconf)
{
	lcd_print("%s\n",__FUNCTION__);

	WRITE_LCD_REG(ENCL_VIDEO_EN,          0);

	WRITE_LCD_REG_BITS(VPU_VIU_VENC_MUX_CTRL, 0, 0, 4); //viu1, viu2 select encl

	WRITE_LCD_REG(ENCL_VIDEO_MODE,        0);
	WRITE_LCD_REG(ENCL_VIDEO_MODE_ADV,    0x8); // Sampling rate: 1

	WRITE_LCD_REG(ENCL_VIDEO_FILT_CTRL,   0x1000); // bypass filter

	WRITE_LCD_REG(ENCL_VIDEO_MAX_PXCNT,   pconf->lcd_basic.h_period - 1);
	WRITE_LCD_REG(ENCL_VIDEO_MAX_LNCNT,   pconf->lcd_basic.v_period - 1);

	WRITE_LCD_REG(ENCL_VIDEO_HAVON_BEGIN, pconf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_VIDEO_HAVON_END,   pconf->lcd_basic.h_active - 1 + pconf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_VIDEO_VAVON_BLINE, pconf->lcd_timing.video_on_line);
	WRITE_LCD_REG(ENCL_VIDEO_VAVON_ELINE, pconf->lcd_basic.v_active - 1  + pconf->lcd_timing.video_on_line);

	WRITE_LCD_REG(ENCL_VIDEO_HSO_BEGIN,   10);//pconf->lcd_timing.hs_hs_addr);
	WRITE_LCD_REG(ENCL_VIDEO_HSO_END,     16);//pconf->lcd_timing.hs_he_addr);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_BEGIN,   pconf->lcd_timing.vso_hstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_END,     pconf->lcd_timing.vso_hstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_BLINE,   pconf->lcd_timing.vso_vstart);
	WRITE_LCD_REG(ENCL_VIDEO_VSO_ELINE,   pconf->lcd_timing.vso_vstart + 2);

	WRITE_LCD_REG(ENCL_VIDEO_RGBIN_CTRL,  (1 << 0));//(1 << 1) | (1 << 0)); //bit[0] 1:RGB, 0:YUV

	WRITE_LCD_REG(ENCL_VIDEO_EN,          1); // enable encl
}

static void clk_util_lvds_set_clk_div(Lcd_Config_t *pconf)
{
	unsigned int phy_div2, wr_mode;

	if (pconf->lcd_control.lvds_config->dual_port == 0) {
		phy_div2 = 0;
		wr_mode = 1;
	} else {
		phy_div2 = 1;
		wr_mode = 3;
	}

	/* ---------------------------------------------
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
	// enable all serializers, divide by 7 */
	WRITE_LCD_CBUS_REG(HHI_LVDS_TX_PHY_CNTL0, (0xfff << 16) | (0x1 << 6));
	WRITE_LCD_CBUS_REG(HHI_LVDS_TX_PHY_CNTL1,
			(1 << 30) | (phy_div2 << 25) | (1 << 24));

	/*    lvds_gen_cntl       <= {10'h0,     // [15:4] unused
	//                            2'h1,      // [5:4] divide by 7 in the PHY
	//                            1'b0,      // [3] fifo_en
	//                            1'b0,      // [2] wr_bist_gate
	//                            2'b00};    // [1:0] fifo_wr mode
	//FIFO_CLK_SEL = 1; // div7 */
	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 4, 2); /* lvds fifo clk div 7 */
	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, wr_mode, 0, 2);  /* fifo wr mode */

	/* lvds phy div reset */
	WRITE_LCD_CBUS_REG_BITS(HHI_LVDS_TX_PHY_CNTL0, 1, 0, 1);
	udelay(5);
	/* Release lvds div reset */
	WRITE_LCD_CBUS_REG_BITS(HHI_LVDS_TX_PHY_CNTL0, 0, 0, 1);
}

static void set_control_lvds(Lcd_Config_t *pconf)
{
	unsigned int lvds_repack, pn_swap, bit_num;
	unsigned int dual_port, port_swap;
	unsigned int data32;

	lcd_print("%s\n", __FUNCTION__);
	clk_util_lvds_set_clk_div(pconf);

	WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1); // disable fifo

	data32 = (0x00 << LVDS_blank_data_r) |
		(0x00 << LVDS_blank_data_g) |
		(0x00 << LVDS_blank_data_b) ;
	WRITE_LCD_REG(LVDS_BLANK_DATA_HI, (data32 >> 16));
	WRITE_LCD_REG(LVDS_BLANK_DATA_LO, (data32 & 0xffff));

	dual_port = pconf->lcd_control.lvds_config->dual_port;
	port_swap = pconf->lcd_control.lvds_config->port_swap;
	lvds_repack = (pconf->lcd_control.lvds_config->lvds_repack) & 0x1;
	pn_swap = (pconf->lcd_control.lvds_config->pn_swap) & 0x1;

	switch (pconf->lcd_basic.lcd_bits) {
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

	WRITE_LCD_REG(LVDS_PACK_CNTL_ADDR,
			(lvds_repack << 0) | /* repack */
			(port_swap << 2) | /* odd_even */
			(0 << 3) | /* reserve */
			(0 << 4) | /* lsb first */
			(pn_swap << 5) | /* pn swap */
			(dual_port << 6) | /* dual port */
			(0 << 7) | /* use tcon control */
			(bit_num << 8) | /* 0:10bits, 1:8bits, 2:6bits, 3:4bits. */
			(0 << 10) | /*r_select  //0:R, 1:G, 2:B, 3:0 */
			(1 << 12) | /*g_select  //0:R, 1:G, 2:B, 3:0 */
			(2 << 14));  /*b_select  //0:R, 1:G, 2:B, 3:0;  */

	/* WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 1, 3, 1);  //enable fifo */
}

static void set_control_ttl(Lcd_Config_t *pconf)
{
	unsigned rb_port_swap, rgb_bit_swap;

	rb_port_swap = (unsigned)(pconf->lcd_control.ttl_config->rb_swap);
	rgb_bit_swap = (unsigned)(pconf->lcd_control.ttl_config->bit_swap);

	WRITE_LCD_REG(L_DUAL_PORT_CNTL_ADDR, (rb_port_swap << LCD_RGB_SWP) | (rgb_bit_swap << LCD_BIT_SWP));
}

static void init_phy_lvds(Lcd_Config_t *pconf)
{
	unsigned int swing_level;
	unsigned int temp;
	lcd_print("%s\n", __FUNCTION__);

	WRITE_LCD_REG(LVDS_SER_EN, 0xfff); /* Enable the serializers */

	WRITE_LCD_REG(LVDS_PHY_CNTL0, 0xffff);
	WRITE_LCD_REG(LVDS_PHY_CNTL1, 0xff00);
	WRITE_LCD_REG(LVDS_PHY_CNTL4, 0x007f);

	swing_level = pconf->lcd_control.lvds_config->lvds_vswing;
	swing_level = (swing_level >= LVDS_VSWING_LEVEL_MAX) ?
		(LVDS_VSWING_LEVEL_MAX - 1) : swing_level;

	temp = 0x606cca80 | (lvds_vswing_ctrl[swing_level] << 26);
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL1, temp);
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL2, 0x0000006c);
	WRITE_LCD_CBUS_REG(HHI_DIF_CSI_PHY_CNTL3, 0x00000800);
}

static void init_dphy(Lcd_Config_t *pconf)
{
	unsigned lcd_type = (unsigned)(pconf->lcd_basic.lcd_type);

	switch (lcd_type) {
	case LCD_DIGITAL_LVDS:
		WRITE_LCD_CBUS_REG(HHI_DSI_LVDS_EDP_CNTL0, lcd_type);
		init_phy_lvds(pconf);
		break;
	default:
		break;
	}
}

static void _init_lcd_driver(Lcd_Config_t *pconf)
{
    int lcd_type = pconf->lcd_basic.lcd_type;
    unsigned char ss_level = (pconf->lcd_timing.clk_ctrl >> CLK_CTRL_SS) & 0xf;

    printf("\nInit LCD mode: %s, %s(%u) %ubit, %ux%u@%u.%uHz, ss_level=%u(%s)\n",
	pconf->lcd_basic.model_name, lcd_type_table[lcd_type], lcd_type,
	pconf->lcd_basic.lcd_bits, pconf->lcd_basic.h_active, pconf->lcd_basic.v_active,
	(pconf->lcd_timing.sync_duration_num / 10),
	(pconf->lcd_timing.sync_duration_num % 10),
	ss_level, lcd_ss_level_table[ss_level]);

    set_vclk_lcd(pconf);
    set_venc_lcd(pconf);
    set_tcon_lcd(pconf);
    switch (lcd_type) {
        case LCD_DIGITAL_LVDS:
            set_control_lvds(pconf);
            init_dphy(pconf);
            break;
        case LCD_DIGITAL_TTL:
            set_control_ttl(pconf);
            break;
        default:
            printf("Invalid LCD type.\n");
            break;
    }
    printf("%s finished.\n", __FUNCTION__);
}

static void _disable_lcd_driver(Lcd_Config_t *pconf)
{
    WRITE_LCD_REG_BITS(LVDS_GEN_CNTL, 0, 3, 1); //disable lvds fifo

    WRITE_LCD_REG(ENCL_VIDEO_EN, 0);	//disable encl
    WRITE_LCD_CBUS_REG_BITS(HHI_VID_CLK_CNTL2, 0, 3, 1);	//disable CTS_ENCL

    WRITE_LCD_CBUS_REG_BITS(HHI_VIID_CLK_CNTL, 0, 0, 5);	//close vclk2 gate: 0x104b[4:0]

    WRITE_LCD_CBUS_REG_BITS(HHI_VIID_DIVIDER_CNTL, 0, 16, 1);	//close vid2_pll gate: 0x104c[16]

    WRITE_LCD_CBUS_REG_BITS(HHI_HDMI_PLL_CNTL, 0, 30, 1);	//disable vid_pll: 0x10e0[30]
    printf("disable lcd display driver.\n");
}

static void _enable_vsync_interrupt(void)
{
	WRITE_LCD_REG(VENC_INTCTRL, 0x200);
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
	WRITE_LCD_REG(ENCL_TST_CLRBAR_STRT, lcd_conf->lcd_timing.video_on_pixel);
	WRITE_LCD_REG(ENCL_TST_CLRBAR_WIDTH, (lcd_conf->lcd_basic.h_active / 9));
	WRITE_LCD_REG(ENCL_TST_EN, lcd_enc_tst[num][4]);
	WRITE_LCD_REG_BITS(ENCL_VIDEO_MODE_ADV, lcd_enc_tst[num][5], 3, 1);

	if (num > 0)
		printf("show test pattern %d: %s\n", num, lcd_enc_tst_str[num]);
	else
		printf("disable test pattern\n");
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
	_init_lcd_driver(lcd_conf);
	lcd_conf->lcd_power_ctrl.power_ctrl(ON);
	_enable_vsync_interrupt();
}

static void lcd_module_disable(void)
{
	lcd_conf->lcd_power_ctrl.power_ctrl(OFF);
	_disable_lcd_driver(lcd_conf);
}

static unsigned int clk_div_calc(unsigned int clk, unsigned int div_sel, int dir)
{
	unsigned int clk_ret;

	switch (div_sel) {
	case CLK_DIV_SEL_1:
		clk_ret = clk;
		break;
	case CLK_DIV_SEL_2:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 2;
		else
			clk_ret = clk * 2;
		break;
	case CLK_DIV_SEL_3:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 3;
		else
			clk_ret = clk * 3;
		break;
	case CLK_DIV_SEL_3p5:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk * 2 / 7;
		else
			clk_ret = clk * 7 / 2;
		break;
	case CLK_DIV_SEL_3p75:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk * 4 / 15;
		else
			clk_ret = clk * 15 / 4;
		break;
	case CLK_DIV_SEL_4:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 4;
		else
			clk_ret = clk * 4;
		break;
	case CLK_DIV_SEL_5:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 5;
		else
			clk_ret = clk * 5;
		break;
	case CLK_DIV_SEL_6:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 6;
		else
			clk_ret = clk * 6;
		break;
	case CLK_DIV_SEL_6p25:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk * 4 / 25;
		else
			clk_ret = clk * 25 / 4;
		break;
	case CLK_DIV_SEL_7:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 7;
		else
			clk_ret = clk * 7;
		break;
	case CLK_DIV_SEL_7p5:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk * 2 / 15;
		else
			clk_ret = clk * 15 / 2;
		break;
	case CLK_DIV_SEL_12:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 12;
		else
			clk_ret = clk * 12;
		break;
	case CLK_DIV_SEL_14:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 14;
		else
			clk_ret = clk * 14;
		break;
	case CLK_DIV_SEL_15:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk / 15;
		else
			clk_ret = clk * 15;
		break;
	case CLK_DIV_SEL_2p5:
		if (dir == CLK_DIV_I2O)
			clk_ret = clk * 2 / 5;
		else
			clk_ret = clk * 5 / 2;
		break;
	default:
		clk_ret = clk;
		printf("[Error]clk_div_sel:  Invalid parameter\n");
		break;
	}

	return clk_ret;
}

struct pll_para_s {
	unsigned int m;
	unsigned int n;
	unsigned int frac;
	unsigned int od1_sel;
	unsigned int od2_sel;
	unsigned int od3_sel;
};

static int check_pll(struct pll_para_s *pll, unsigned int pll_fout)
{
	unsigned int fin, m, n;
	unsigned int od1_sel, od2_sel, od3_sel, od1, od2, od3;
	unsigned int pll_fod2_in, pll_fod3_in, pll_fvco;
	unsigned int od_fb = 0, pll_frac;
	int done;

	done = 0;
	fin = FIN_FREQ; /* kHz */
	for (od3_sel = OD_SEL_MAX; od3_sel > 0; od3_sel--) {
		od3 = od_table[od3_sel - 1];
		pll_fod3_in = pll_fout * od3;
		for (od2_sel = od3_sel; od2_sel > 0; od2_sel--) {
			od2 = od_table[od2_sel - 1];
			pll_fod2_in = pll_fod3_in * od2;
			for (od1_sel = od2_sel; od1_sel > 0; od1_sel--) {
				od1 = od_table[od1_sel - 1];
				pll_fvco = pll_fod2_in * od1;
				if ((pll_fvco < PLL_VCO_MIN) ||
					(pll_fvco > PLL_VCO_MAX)) {
					continue;
				}
				pll->od1_sel = od1_sel - 1;
				pll->od2_sel = od2_sel - 1;
				pll->od3_sel = od3_sel - 1;
				lcd_print("od1_sel=%d, od2_sel=%d, od3_sel=%d,",
					(od1_sel - 1), (od2_sel - 1),
					(od3_sel - 1));
				lcd_print(" pll_fvco=%d\n", pll_fvco);
				n = 1;
				od_fb = 0; /* pll default */
				pll_fvco = pll_fvco / ((od_fb + 1) * 2);
				m = pll_fvco / fin;
				pll_frac = (pll_fvco % fin) * 4096 / fin;
				pll->m = m;
				pll->n = n;
				pll->frac = pll_frac;
				lcd_print("pll_m=%d, pll_n=%d, pll_frac=%d\n",
					m, n, pll_frac);
				done = 1;
			}
		}
	}
	return done;
}

static void generate_clk_parameter(Lcd_Config_t *pconf)
{
	struct pll_para_s pll;
	int ret = 0;

	unsigned clk_div_sel, crt_xd;
	unsigned crt_xd_max;
	unsigned fout_pll, clk_div_out;
	unsigned tmp;
	unsigned fout;

	fout = pconf->lcd_timing.lcd_clk / 1000; /* kHz */

	if (fout > ENCL_MAX_CLK_IN)
		goto generate_clk_done;

	switch (pconf->lcd_basic.lcd_type) {
	case LCD_DIGITAL_LVDS:
		clk_div_sel = CLK_DIV_SEL_1;//CLK_DIV_SEL_7;
		crt_xd_max = CRT_VID_DIV_MAX;
		crt_xd = 7;
		clk_div_out = fout * crt_xd;
		if (clk_div_out > CRT_VID_MAX_CLK_IN)
			goto generate_clk_done;
		lcd_print("fout=%d, crt_xd=%d, clk_div_out=%d\n",
			fout, crt_xd, clk_div_out);
		fout_pll = clk_div_calc(clk_div_out,
				clk_div_sel, CLK_DIV_O2I);
		if (fout_pll > CLK_DIV_MAX_CLK_IN)
			goto generate_clk_done;
		lcd_print("clk_div_sel=%d, fout_pll=%d\n",
			clk_div_sel, fout_pll);
		ret = check_pll(&pll, fout_pll);
		if (ret)
			goto generate_clk_done;
		break;
	case LCD_DIGITAL_TTL:
		clk_div_sel = CLK_DIV_SEL_1;
		crt_xd_max = CRT_VID_DIV_MAX;
		for (crt_xd = 1; crt_xd <= crt_xd_max; crt_xd++) {
			clk_div_out = fout * crt_xd;
			if (clk_div_out > CRT_VID_MAX_CLK_IN)
				continue;
			lcd_print("fout=%d, crt_xd=%d, clk_div_out=%d\n",
				fout, crt_xd, clk_div_out);
			fout_pll = clk_div_calc(clk_div_out,
					clk_div_sel, CLK_DIV_O2I);
			if (fout_pll > CLK_DIV_MAX_CLK_IN)
				continue;
			lcd_print("clk_div_sel=%d, fout_pll=%d\n",
				clk_div_sel, fout_pll);
			ret = check_pll(&pll, fout_pll);
			if (ret)
				goto generate_clk_done;
		}
		break;
	default:
		break;
	}

generate_clk_done:
	if (ret) {
		pconf->lcd_timing.pll_ctrl =
			(pll.od1_sel << PLL_CTRL_OD1) |
			(pll.od2_sel << PLL_CTRL_OD2) |
			(pll.od3_sel << PLL_CTRL_OD3) |
			(pll.n << PLL_CTRL_N) |
			(pll.m << PLL_CTRL_M);
		pconf->lcd_timing.div_ctrl = (clk_div_sel << DIV_CTRL_CLK_DIV);
		tmp = (pconf->lcd_timing.clk_ctrl &
			~((0xff << CLK_CTRL_XD) | (0xfff << CLK_CTRL_FRAC)));
		pconf->lcd_timing.clk_ctrl = (tmp |
			((crt_xd << CLK_CTRL_XD) |
			(pll.frac << CLK_CTRL_FRAC)));
	} else {
		pconf->lcd_timing.pll_ctrl = (0 << PLL_CTRL_OD1) |
			(1 << PLL_CTRL_OD2) | (1 << PLL_CTRL_OD3) |
			(1 << PLL_CTRL_N) | (65 << PLL_CTRL_M);
		pconf->lcd_timing.div_ctrl =
			(CLK_DIV_SEL_1 << DIV_CTRL_CLK_DIV);
		pconf->lcd_timing.clk_ctrl = (pconf->lcd_timing.clk_ctrl &
			~(0xff << CLK_CTRL_XD)) | (7 << CLK_CTRL_XD);
		printf("Out of clock range, reset to default setting!\n");
	}
}

static void lcd_sync_duration(Lcd_Config_t *pconf)
{
	unsigned m, n, od1, od2, od3, od_fb, frac, clk_div, xd;
	unsigned h_period, v_period, sync_duration_num, sync_duration_den;
	unsigned pll_out_clk, lcd_clk;

	m = ((pconf->lcd_timing.pll_ctrl) >> PLL_CTRL_M) & 0x1ff;
	n = ((pconf->lcd_timing.pll_ctrl) >> PLL_CTRL_N) & 0x1f;
	od1 = ((pconf->lcd_timing.pll_ctrl) >> PLL_CTRL_OD1) & 0x3;
	od2 = ((pconf->lcd_timing.pll_ctrl) >> PLL_CTRL_OD2) & 0x3;
	od3 = ((pconf->lcd_timing.pll_ctrl) >> PLL_CTRL_OD3) & 0x3;
	od1 = od_table[od1];
	od2 = od_table[od2];
	od3 = od_table[od3];
	frac = ((pconf->lcd_timing.clk_ctrl) >> CLK_CTRL_FRAC) & 0xfff;
	od_fb = 0;
	clk_div = ((pconf->lcd_timing.div_ctrl) >> DIV_CTRL_CLK_DIV) & 0xff;
	xd = ((pconf->lcd_timing.clk_ctrl) >> CLK_CTRL_XD) & 0xff;

	h_period = pconf->lcd_basic.h_period;
	v_period = pconf->lcd_basic.v_period;

	switch (pconf->lcd_basic.lcd_type) {
	case LCD_DIGITAL_LVDS:
		xd = 1;
		clk_div = CLK_DIV_SEL_7;
		break;
	case LCD_DIGITAL_TTL:
		clk_div = CLK_DIV_SEL_1;
		break;
	default:
		break;
	}

	od_fb = (od_fb + 1) * 2;
	pll_out_clk = (frac * od_fb * FIN_FREQ) / 4096;
	pll_out_clk = ((m * od_fb * FIN_FREQ + pll_out_clk) /
		(n * od1 * od2 * od3));
	lcd_clk = clk_div_calc(pll_out_clk, clk_div, CLK_DIV_I2O) / xd;
	pconf->lcd_timing.lcd_clk = lcd_clk * 1000;
	sync_duration_num = ((lcd_clk * 1000 / h_period) * 100) / v_period;
	sync_duration_num = (sync_duration_num + 5) / 10;
	sync_duration_den = 10;

	pconf->lcd_timing.sync_duration_num = sync_duration_num;
	pconf->lcd_timing.sync_duration_den = sync_duration_den;
	printf("lcd_clk=%u.%03uMHz, frame_rate=%u.%uHz\n\n",
		(lcd_clk / 1000), (lcd_clk % 1000),
		(sync_duration_num / sync_duration_den),
		((sync_duration_num * 10 / sync_duration_den) % 10));
}

static void lcd_tcon_config(Lcd_Config_t *pconf)
{
	unsigned short de_hstart, de_vstart;
	unsigned short hstart, hend, vstart, vend;
	unsigned short h_delay = 0;
	unsigned short h_offset = 0, v_offset = 0, vsync_h_phase=0;

	switch (pconf->lcd_basic.lcd_type) {
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

    pconf->lcd_timing.video_on_pixel = pconf->lcd_basic.h_period - pconf->lcd_basic.h_active - 1 -h_delay;
    pconf->lcd_timing.video_on_line = pconf->lcd_basic.v_period - pconf->lcd_basic.v_active;

    h_offset = (pconf->lcd_timing.h_offset & 0xffff);
    v_offset = (pconf->lcd_timing.v_offset & 0xffff);
    if ((pconf->lcd_timing.h_offset >> 31) & 1)
        de_hstart = (pconf->lcd_basic.h_period - pconf->lcd_basic.h_active - 1 + pconf->lcd_basic.h_period - h_offset) % pconf->lcd_basic.h_period;
    else
        de_hstart = (pconf->lcd_basic.h_period - pconf->lcd_basic.h_active - 1 + h_offset) % pconf->lcd_basic.h_period;
    if ((pconf->lcd_timing.v_offset >> 31) & 1)
        de_vstart = (pconf->lcd_basic.v_period - pconf->lcd_basic.v_active + pconf->lcd_basic.v_period - v_offset) % pconf->lcd_basic.v_period;
    else
        de_vstart = (pconf->lcd_basic.v_period - pconf->lcd_basic.v_active + v_offset) % pconf->lcd_basic.v_period;

    hstart = (de_hstart + pconf->lcd_basic.h_period - pconf->lcd_timing.hsync_bp - pconf->lcd_timing.hsync_width) % pconf->lcd_basic.h_period;
    hend = (de_hstart + pconf->lcd_basic.h_period - pconf->lcd_timing.hsync_bp) % pconf->lcd_basic.h_period;
    pconf->lcd_timing.hs_hs_addr = hstart;
    pconf->lcd_timing.hs_he_addr = hend;
    pconf->lcd_timing.hs_vs_addr = 0;
    pconf->lcd_timing.hs_ve_addr = pconf->lcd_basic.v_period - 1;

    vsync_h_phase = (pconf->lcd_timing.vsync_h_phase & 0xffff);
    if ((pconf->lcd_timing.vsync_h_phase >> 31) & 1) //negative
        vsync_h_phase = (hstart + pconf->lcd_basic.h_period - vsync_h_phase) % pconf->lcd_basic.h_period;
    else //positive
        vsync_h_phase = (hstart + pconf->lcd_basic.h_period + vsync_h_phase) % pconf->lcd_basic.h_period;
    pconf->lcd_timing.vs_hs_addr = vsync_h_phase;
    pconf->lcd_timing.vs_he_addr = vsync_h_phase;
    vstart = (de_vstart + pconf->lcd_basic.v_period - pconf->lcd_timing.vsync_bp - pconf->lcd_timing.vsync_width) % pconf->lcd_basic.v_period;
    vend = (de_vstart + pconf->lcd_basic.v_period - pconf->lcd_timing.vsync_bp) % pconf->lcd_basic.v_period;
    pconf->lcd_timing.vs_vs_addr = vstart;
    pconf->lcd_timing.vs_ve_addr = vend;

    pconf->lcd_timing.de_hs_addr = de_hstart;
    pconf->lcd_timing.de_he_addr = (de_hstart + pconf->lcd_basic.h_active) % pconf->lcd_basic.h_period;
    pconf->lcd_timing.de_vs_addr = de_vstart;
    pconf->lcd_timing.de_ve_addr = (de_vstart + pconf->lcd_basic.v_active - 1) % pconf->lcd_basic.v_period;

    if (pconf->lcd_timing.vso_user == 0) {
        //pconf->lcd_timing.vso_hstart = pconf->lcd_timing.vs_hs_addr;
        pconf->lcd_timing.vso_vstart = pconf->lcd_timing.vs_vs_addr;
    }

    //lcd_print("hs_hs_addr=%d, hs_he_addr=%d, hs_vs_addr=%d, hs_ve_addr=%d\n", pconf->lcd_timing.hs_hs_addr, pconf->lcd_timing.hs_he_addr, pconf->lcd_timing.hs_vs_addr, pconf->lcd_timing.hs_ve_addr);
    //lcd_print("vs_hs_addr=%d, vs_he_addr=%d, vs_vs_addr=%d, vs_ve_addr=%d\n", pconf->lcd_timing.vs_hs_addr, pconf->lcd_timing.vs_he_addr, pconf->lcd_timing.vs_vs_addr, pconf->lcd_timing.vs_ve_addr);
    //lcd_print("de_hs_addr=%d, de_he_addr=%d, de_vs_addr=%d, de_ve_addr=%d\n", pconf->lcd_timing.de_hs_addr, pconf->lcd_timing.de_he_addr, pconf->lcd_timing.de_vs_addr, pconf->lcd_timing.de_ve_addr);
}

static void lcd_control_config_pre(Lcd_Config_t *pconf)
{
    unsigned ss_level;

    if (pconf->lcd_timing.lcd_clk < 200) {//prepare refer clock for frame_rate setting
        pconf->lcd_timing.lcd_clk = (pconf->lcd_timing.lcd_clk * pconf->lcd_basic.h_period * pconf->lcd_basic.v_period);
    }

    ss_level = ((pconf->lcd_timing.clk_ctrl >> CLK_CTRL_SS) & 0xf);
    ss_level = ((ss_level >= SS_LEVEL_MAX) ? (SS_LEVEL_MAX-1) : ss_level);

    switch (pconf->lcd_basic.lcd_type) {
        case LCD_DIGITAL_LVDS:
            if (pconf->lcd_control.lvds_config->lvds_repack_user == 0) {
                if (pconf->lcd_basic.lcd_bits == 6)
                    pconf->lcd_control.lvds_config->lvds_repack = 0;
                else
                    pconf->lcd_control.lvds_config->lvds_repack = 1;
            }
            if (pconf->lcd_control.lvds_config->dual_port == 0) {
                if (pconf->lcd_control.lvds_config->port_swap == 0)
                    pconf->lcd_control.lvds_config->port_sel = LVDS_PORT_A;
                else
                    pconf->lcd_control.lvds_config->port_sel = LVDS_PORT_B;
            } else {
                pconf->lcd_control.lvds_config->port_sel = LVDS_PORT_AB;
            }
            break;
        default:
            break;
    }
    pconf->lcd_timing.clk_ctrl = ((pconf->lcd_timing.clk_ctrl & (~(0xf << CLK_CTRL_SS))) | (ss_level << CLK_CTRL_SS));
}

//****************************************
static LVDS_Config_t lcd_lvds_config = {
    .lvds_vswing = 1,
    .lvds_repack_user = 0,
    .lvds_repack = 0,
    .pn_swap = 0,
    .dual_port = 0,
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
        .gamma_ctrl = ((0 << GAMMA_CTRL_REVERSE) | (1 << GAMMA_CTRL_EN)),
        .gamma_r_coeff = 100,
        .gamma_g_coeff = 100,
        .gamma_b_coeff = 100,
    },
    .lcd_control = {
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

static void lcd_config_assign(Lcd_Config_t *pconf)
{
    pconf->lcd_timing.vso_hstart = 10; //for video process
    pconf->lcd_timing.vso_vstart = 10; //for video process
    pconf->lcd_timing.vso_user = 0; //use default config

    pconf->lcd_power_ctrl.ports_ctrl = lcd_ports_ctrl;

    pconf->lcd_misc_ctrl.module_enable = lcd_module_enable;
    pconf->lcd_misc_ctrl.module_disable = lcd_module_disable;
    pconf->lcd_misc_ctrl.lcd_test = lcd_test;
    pconf->lcd_misc_ctrl.print_version = print_lcd_driver_version;
    pconf->lcd_misc_ctrl.print_clk = print_lcd_clk_info;
}

void lcd_config_init(Lcd_Config_t *pconf)
{
    lcd_control_config_pre(pconf);

    if ((pconf->lcd_timing.clk_ctrl >> CLK_CTRL_AUTO) & 1) {
        printf("Auto generate clock parameters.\n");
        generate_clk_parameter(pconf);
    }
    else {
        printf("Custome clock parameters.\n");
    }
    printf("pll_ctrl=0x%x, div_ctrl=0x%x, clk_ctrl=0x%x.\n", pconf->lcd_timing.pll_ctrl, pconf->lcd_timing.div_ctrl, pconf->lcd_timing.clk_ctrl);

    lcd_sync_duration(pconf);
    lcd_tcon_config(pconf);
}

void lcd_config_probe(Lcd_Config_t *pconf)
{
    lcd_conf = pconf;
    lcd_config_assign(pconf);
}

