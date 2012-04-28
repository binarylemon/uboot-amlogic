/*
 * Amlogic Ethernet Driver
 *
 * Copyright (C) 2012 Amlogic, Inc.
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
 * Author: Platform-BJ@amlogic.com
 *
 */

#include <linux/types.h>
#include <config.h>
#include <malloc.h>
#include <common.h>
#include <net.h>

#include <asm/u-boot.h>
#include <asm/cache.h>

/**
 * Amlogic mason headers
 *
 * arch/arm/include/asm/arch-m*
 */
#include <asm/arch/io.h>
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/aml_emac_lan8700.h>

/**
 * arch/arm/cpu/aml_meson
 *
 */
extern unsigned __net_dma_start;
extern unsigned __net_dma_end;


static unsigned char g_bi_enetaddr[6] = {0, 0, 0, 0, 0, 0};
static struct _gStruct*	gS = NULL;
static struct _rx_desc*	g_rx = NULL;
static struct _tx_desc*	g_tx = NULL;
static struct _rx_desc*	g_current_rx = NULL;
static struct _tx_desc*	g_current_tx = NULL;
static int g_nInitialized = 0 ;
static unsigned long g_phy_Identifier = 0;
#define PHY_SMSC_8700			0x7c0c4
#define PHY_SMSC_8720			0x7c0f1
#define PHY_ATHEROS_8032		0x004dd023
#define PHY_ATHEROS_8035		0x004dd072

#define MAC_MODE_RMII_CLK_EXTERNAL       0
#define MAC_MODE_RMII_CLK_INTERNAL       1
#define MAC_MODE_RGMII                   2
static int g_mac_mode = MAC_MODE_RMII_CLK_EXTERNAL;

static int g_debug = 0;
//#define ET_DEBUG

static void phy_reg_wr(int phyad, unsigned int reg, unsigned int val)
{
	unsigned long busy = 0, tmp = 0;
	unsigned int phyaddr;
	unsigned int phyreg;
	unsigned long reg4;

	phyaddr = phyad << ETH_MAC_4_GMII_Addr_PA_P;
	phyreg  = reg << ETH_MAC_4_GMII_Addr_GR_P;

	reg4 = phyaddr | phyreg | ETH_MAC_4_GMII_Addr_CR_100_150 | ETH_MAC_4_GMII_Addr_GW | ETH_MAC_4_GMII_Addr_GB;
	writel(val, ETH_MAC_5_GMII_Data);
	writel(reg4, ETH_MAC_4_GMII_Addr);
	busy = 1;
	while (busy) {
		tmp = readl(ETH_MAC_4_GMII_Addr);
		busy = tmp & 1;
	}
}

static unsigned int phy_reg_rd(int phyad, unsigned int reg)
{
	unsigned long busy = 0, tmp = 0;
	unsigned int phyaddr;
	unsigned int phyreg;
	unsigned long reg4;

	phyaddr = phyad << ETH_MAC_4_GMII_Addr_PA_P;
	phyreg  = reg << ETH_MAC_4_GMII_Addr_GR_P;
	reg4 = phyaddr | phyreg | ETH_MAC_4_GMII_Addr_CR_100_150 | ETH_MAC_4_GMII_Addr_GB;

	writel(reg4, ETH_MAC_4_GMII_Addr);

	busy = 1;
	while (busy) {
		tmp = readl(ETH_MAC_4_GMII_Addr);
		busy = tmp & 1;
	}

	tmp = readl(ETH_MAC_5_GMII_Data);

	return tmp;
}

static inline void _dcache_flush_range_for_net(unsigned startAddr, unsigned endAddr)
{
	dcache_clean_range(startAddr, endAddr - startAddr + 1);
	return;
}

static inline void _dcache_inv_range_for_net(unsigned startAddr, unsigned endAddr)
{
	dcache_flush_range(startAddr, endAddr - startAddr + 1);
	dcache_invalid_range(startAddr, endAddr - startAddr + 1);
	return;
}

static unsigned int detect_phyad(void)
{
	unsigned int testval = 0;
	int i;
	static int s_phyad = -1;

	if (s_phyad != -1) {
		return s_phyad;
	}
	for (i = 0; i < 32; i++) {
		testval = phy_reg_rd(i, PHY_SR);	//read the SR register..
		if (testval != 0x0000 && testval != 0xffff) {
			s_phyad = i;
			return s_phyad;
		}
	}
	return 0xffff;
}

static void set_mac_mode()
{
	printf("set_mac_mode(%d)\n", g_mac_mode);
	if (g_mac_mode == 2) {
		/* RGMII */
		writel((ETH_MAC_0_Configuration_PS_GMII | ETH_MAC_0_Configuration_DM| ETH_MAC_0_Configuration_RE | ETH_MAC_0_Configuration_TE), ETH_MAC_0_Configuration);
	} else {
		/* RMII */
		writel((ETH_MAC_0_Configuration_PS_MII | ETH_MAC_0_Configuration_FES_100M | ETH_MAC_0_Configuration_DM
				| ETH_MAC_0_Configuration_RE | ETH_MAC_0_Configuration_TE), ETH_MAC_0_Configuration);
	}

	writel((ETH_MAC_1_Frame_Filter_PM | ETH_MAC_1_Frame_Filter_RA), ETH_MAC_1_Frame_Filter);
}

static void set_mac_addrs(void *ptr)
{
	unsigned int mac_filter = 0;
	unsigned char * p = (unsigned char *)ptr;

	mac_filter = (p[5] << 8) | p[4];
	writel(mac_filter, ETH_MAC_Addr0_High);
	mac_filter = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
	writel(mac_filter, ETH_MAC_Addr0_Low);
}

static void netdev_chk(void)
{
	unsigned int rint, rint2;
	static unsigned int old_rint = -1;
	unsigned int id;
	int speed, full;

	speed = full = 0;
	id = detect_phyad();
	rint2 = 3000;
	do {
		rint = phy_reg_rd(id, PHY_SR);
		if ((rint & PHY_SR_ANCOMPLETE)) {
			break;
		}
		udelay(1000);
	} while (rint2-- > 0);
	if (!(rint & PHY_SR_ANCOMPLETE)) {
		printf("phy auto link failed\n");
	}

	if (old_rint != rint) {
		if (g_debug > 1)
			printf("netdev_chk() g_phy_Identifier: 0x%x\n", g_phy_Identifier);
		switch (g_phy_Identifier) {
		case PHY_ATHEROS_8032:
			rint2 = phy_reg_rd(id, 17);
			speed = (rint2 & (1 << 14)) >> 14;
			full = ((rint2) & (1 << 13));
			gS->linked = rint2 & (1 << 10);
			break;
		case PHY_ATHEROS_8035:
			rint2 = phy_reg_rd(id, 17);
			speed = (rint2 & (3 << 14)) >> 14;
			full = ((rint2) & (1 << 13));
			gS->linked = rint2 & (1 << 10);
			break;
		case PHY_SMSC_8700:
		case PHY_SMSC_8720:
		default:
			rint2 = phy_reg_rd(id, 31);
			speed = (rint2 & (1 << 3)) >> 3;
			full = ((rint2 >> 4) & 1);
			gS->linked = rint2 & (1 << 2);
			break;
		}
		/* phy_auto_negotiation_set */
		if (full) {
			printf("duplex\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		} else {
			printf("half duplex\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		}
		if (speed == 0) {
			printf("10m\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);
#ifndef CONFIG_M6
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 20
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 20
			writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
#endif
		} else if (speed == 1) {
			printf("100m\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);	// program mac
#ifndef CONFIG_M6
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 2
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 2
			writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
#endif
		} else {
			printf("1000m\n");
#ifdef CONFIG_M6
			writel(readl(ETH_MAC_0_Configuration) & ~ETH_MAC_0_Configuration_PS_MII, ETH_MAC_0_Configuration);	// program mac
#endif
		}

		/* link_changed */
#if defined(ET_DEBUG)
		unsigned int regs = 0, val;
		for (regs = 0; regs <= 31; regs++) {
			val = phy_reg_rd(id, regs);
			printf("reg[%d]=%x\n", regs, (unsigned)val);
		}
#endif
		old_rint = rint;
	}
}

static void set_phy_mode()
{
	unsigned int phyad = -1;
	unsigned int val;

	phyad = detect_phyad();
	if (phyad > 32 || phyad < 0) {
		return;
	}

	if (g_debug > 1)
		printf("set_phy_mode() g_phy_Identifier: 0x%x\n", g_phy_Identifier);
	switch (g_phy_Identifier) {
	case PHY_ATHEROS_8032:
	case PHY_ATHEROS_8035:
		break;
	case PHY_SMSC_8700:
	case PHY_SMSC_8720:
	default:
		val = PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE << PHY_SPMD_MODE_P) | (phyad << PHY_SPMD_PHYAD_P);
		phy_reg_wr(phyad, PHY_SPMD, val);
		break;
	}
}

/* Reset and idle the chip, putting all registers into
 * a reasonable state */
static int eth_reset(struct _gStruct* emac_config)
{
	int i, k, phyad;
	unsigned int val;
	struct _gStruct* m = emac_config;

#ifdef CONFIG_M6
	/* make sure PHY power-on */
	set_phy_mode();
#endif
#define NET_MAX_RESET_TEST 1000
	for (i = 0; i < NET_MAX_RESET_TEST; i++) {
		/* Software Reset MAC */
		writel(ETH_DMA_0_Bus_Mode_SWR, ETH_DMA_0_Bus_Mode);
		for (k = 0; k < NET_MAX_RESET_TEST; k++) {
			udelay(100);

			if (!(readl(ETH_DMA_0_Bus_Mode)&ETH_DMA_0_Bus_Mode_SWR)) {
				break;
			}
		}
		if (k >= NET_MAX_RESET_TEST) {
			printf("Error: Fail to reset mac!(%d)\n", k);
			return -1;
		} else {
			printf("Success: reset mac OK!(%d)\n", k);
		}

		phyad = detect_phyad();
		if (phyad > 32 || phyad < 0) {
			continue;
		}
		/* set phy work mode */
		val = PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE << PHY_SPMD_MODE_P) | (phyad << PHY_SPMD_PHYAD_P);
		phy_reg_wr(phyad, PHY_SPMD, val);

		/* get phy_Identifier */
		val = phy_reg_rd(phyad, 2);
		g_phy_Identifier = val << 16;
		val = phy_reg_rd(phyad, 3);
		g_phy_Identifier |= val;
		printf("find net phy id=0x%x, phyad=%d\n", (unsigned int)g_phy_Identifier, phyad);

		/* Software Reset PHY */
		phy_reg_wr(phyad, PHY_CR, PHY_CR_RST);
		for (k = 0; k < NET_MAX_RESET_TEST; k++) {
			udelay(1000);
			val = phy_reg_rd(phyad, PHY_CR);
			if (!(val & PHY_CR_RST)) {
				break;
			}
		}
		if (k >= NET_MAX_RESET_TEST) {
			continue;
		} else {
			break;
		}
	}
	if (i >= NET_MAX_RESET_TEST) {
		printf("Error to detected phy\n");
		return -1;
	}

	val = PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE << PHY_SPMD_MODE_P) | (phyad << PHY_SPMD_PHYAD_P);
	phy_reg_wr(phyad, PHY_SPMD, val);

	val = PHY_CR_AN | PHY_CR_RSTAN;
	phy_reg_wr(phyad, PHY_CR, val);

	udelay(10);

	set_mac_mode();

	writel((~0), ETH_DMA_5_Status);							/* clear all status flag */
	writel(0, ETH_DMA_5_Status);
	writel(0, ETH_DMA_6_Operation_Mode);					/* stop RX and TX */
	val = readl(ETH_DMA_8_Missed_Frame_and_Overflow);		/* read to clean */

	writel(0, ETH_DMA_7_Interrupt_Enable);					/* disable all interrupt */
	writel((8 << ETH_DMA_0_Bus_Mode_PBL_P) | ETH_DMA_0_Bus_Mode_FB, ETH_DMA_0_Bus_Mode);

	printf("final_addr[rx-tx]: 0x%x-0x%x\n", m->rx, m->tx);
	writel((long)m->rx, ETH_DMA_3_Re_Descriptor_List_Addr);
	writel((long)m->tx, ETH_DMA_4_Tr_Descriptor_List_Addr);

	/* config the interrupt */
	writel(ETH_DMA_7_Interrupt_Enable_TUE | ETH_DMA_7_Interrupt_Enable_TJE
	       | ETH_DMA_7_Interrupt_Enable_OVE | ETH_DMA_7_Interrupt_Enable_UNE | ETH_DMA_7_Interrupt_Enable_RIE
	       | ETH_DMA_7_Interrupt_Enable_RUE | ETH_DMA_7_Interrupt_Enable_RSE | ETH_DMA_7_Interrupt_Enable_FBE
	       | ETH_DMA_7_Interrupt_Enable_AIE | ETH_DMA_7_Interrupt_Enable_NIE, ETH_DMA_7_Interrupt_Enable);
	writel(0, ETH_MAC_Interrupt_Mask);

	printf("Ethernet reset OK\n");
	return 0;
}

static void DMARXStart(void)
{
	writel(readl(ETH_DMA_6_Operation_Mode) | ETH_DMA_6_Operation_Mode_SR, ETH_DMA_6_Operation_Mode);
}

static void DMATXStart(void)
{
	writel(readl(ETH_DMA_6_Operation_Mode) | ETH_DMA_6_Operation_Mode_ST, ETH_DMA_6_Operation_Mode);
}

static void GetDMAStatus(unsigned int* mask, unsigned  int* status)
{
	*mask = readl(ETH_DMA_7_Interrupt_Enable);
	*status = readl(ETH_DMA_5_Status);
}

static void eth_data_dump(unsigned char *p, int len)
{
	int i, j;
	char s[20];

	for (i = 0; i < len; i += 16) {
		printf("0x%08x:", (unsigned int)p);
		for (j = 0; j < 16 && j < (len - i); j++) {
			s[j] = (p[j] > 15 && p[j] < 128) ? p[j] : '.';
			printf(" %02x", p[j]);
		}
		s[j] = 0;
		printf(" |%s|\n", s);
		p = p + 16;
	}
}

static void eth_tx_dump(unsigned char *p, int len)
{
	if ((g_debug == 2) || (g_debug == 4)) {
		printf("=====>\n");
		eth_data_dump(p, len);
	}
}

static void eth_rx_dump(unsigned char *p, int len)
{
	if ((g_debug == 3) || (g_debug == 4)) {
		printf("<=====\n");
		eth_data_dump(p, len);
	}
}

static void aml_eth_halt(struct eth_device * net_current)
{
	return;
}

static int aml_eth_send(struct eth_device *net_current, volatile void *packet, int length)
{
	unsigned int mask;
	unsigned int status;

	if (!g_nInitialized) {
		return -1;
	}

	eth_tx_dump(packet, length);
	netdev_chk();

	struct _tx_desc* pTx = g_current_tx;
	struct _tx_desc* pDma = (struct _tx_desc*)readl(ETH_DMA_18_Curr_Host_Tr_Descriptor);

	if (pDma != NULL) {
		_dcache_inv_range_for_net((unsigned long)pDma, (unsigned long)(pDma) + sizeof(struct _tx_desc) - 1);
	}
	if (pDma != NULL && !(pDma->tdes0 & TDES0_OWN) && pTx != pDma) {
		//this may not happend,if all the hardware work well...
		//to fixed a bug of the dma maybe lost setting some tx buf to own by host,..;
		//start the current_tx at pDMA
		pTx = pDma;
	}
	if (pDma != pTx) {
		_dcache_inv_range_for_net((unsigned long)pTx, (unsigned long)(pTx + 1) - 1);
	}

	if (length > ETH_MTU) {
		goto err;
	}

	if (length < 14) {
		printf("pbuf len error, len=%d\n", length);
		goto err;
	}

	if (pTx->tdes0 & TDES0_OWN) {
#if 1
		volatile unsigned long Cdma, Dstatus, status;
		Cdma = readl(ETH_DMA_18_Curr_Host_Tr_Descriptor);
		Dstatus = readl(Cdma);
		status = readl(ETH_DMA_5_Status);
		printf("Current DMA=0x%x, Dstatus=0x%x\n", (unsigned int)Cdma, (unsigned int)Dstatus);
		printf("Current status=0x%x\n", (unsigned int)status);
		printf("no buffer to send\n");
#endif
		goto err;
	}

	if (!(unsigned char*)pTx->tdes2) {
		goto err;
	}
	g_current_tx = (struct _tx_desc*)pTx->tdes3;
	memcpy((unsigned char*)pTx->tdes2, (unsigned char*)packet, length);
	_dcache_flush_range_for_net((unsigned)packet, (unsigned)(packet + 1500));
	_dcache_inv_range_for_net((unsigned)packet, (unsigned)(packet + 1500)); //this is the uboot's problem
	//change for add to 60 bytes..
	//by zhouzhi
	if (length < 60) {
		memset((unsigned char*)(pTx->tdes2 + length), 0, 60 - length);
		length = 60;
	}
	//pTx->tdes1 &= DescEndOfRing;
	_dcache_flush_range_for_net((unsigned long)pTx->tdes2, (unsigned long)pTx->tdes2 + length - 1);
	pTx->tdes1 = ((length << TDES1_TBS1_P) & TDES1_TBS1_MASK) | TDES1_FS | TDES1_LS | TDES1_TCH | TDES1_IC;
	pTx->tdes0 = TDES0_OWN;
	_dcache_flush_range_for_net((unsigned long)pTx, (unsigned long)(pTx + 1) - 1);

	GetDMAStatus(&mask, &status);
	if (status & ETH_DMA_5_Status_TS_SUSP) {
		writel(1, ETH_DMA_1_Tr_Poll_Demand);
	} else {
		DMATXStart();
	}

#ifdef ET_DEBUG
	printf("Transfer starting...\n");
	GetDMAStatus(&mask, &status);
	printf("Current status=%x\n", status);
#endif

	/* wait for transfer to succeed */
	//unsigned tmo = get_timer (0) + 5 * CONFIG_SYS_HZ;		//5S time out
	unsigned tmo = 0;
	do {
		udelay(100);
		GetDMAStatus(&mask, &status);
		if (tmo++ >= 50000) {
			printf("\ntransmission error %#x\n", status);
			break;
		}
	} while (!((status & ETH_DMA_5_Status_NIS) && (status & ETH_DMA_5_Status_TI)));

	if (status & ETH_DMA_5_Status_NIS) {
		if (status & ETH_DMA_5_Status_TI) {
			writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_TI, ETH_DMA_5_Status);
		}
		if (status & ETH_DMA_5_Status_TU) {
			writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_TU, ETH_DMA_5_Status);
		}
	}

#ifdef ET_DEBUG
	//test
	GetDMAStatus(&mask, &status);
	printf("Current status=%x\n", status);
#endif
	return 0;
err:
	return -1;
}

/*
 * each time receive a whole packet
 */
static int aml_eth_rx(struct eth_device * net_current)
{
	unsigned int mask;
	unsigned int status;
	int rxnum = 0;
	int len = 0;
	struct _rx_desc* pRx;

	if (!g_nInitialized) {
		return -1;
	}

	netdev_chk();

	/* Check packet ready or not */
	GetDMAStatus(&mask, &status);
	if (!((status & ETH_DMA_5_Status_NIS) && (status & ETH_DMA_5_Status_RI))) {
		return 0;
	}
	writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_RI, ETH_DMA_5_Status);	//clear the int flag

	if (!g_current_rx) {
		g_current_rx = gS->rx;
	}
	pRx = g_current_rx;
	_dcache_inv_range_for_net((unsigned long)pRx, (unsigned long)(pRx + 1) - 1);
	while (!(pRx->rdes0 & RDES0_OWN)) {
		len = (pRx->rdes0 & RDES0_FL_MASK) >> RDES0_FL_P;
		if (14 >= len) {
			printf("err len=%d\n", len);
			goto NEXT_BUF;
		}
		// pbuf_header(pb, -sizeof(short));
		_dcache_inv_range_for_net((unsigned long)pRx->rdes2, (unsigned long)pRx->rdes2 + len - 1);

		if (!memcpy((unsigned char*)NetRxPackets[0], (unsigned char*)pRx->rdes2, len)) {
			printf("memcp error\n");
			goto NEXT_BUF;
		}
NEXT_BUF:
		pRx->rdes0 = RDES0_OWN;
		_dcache_flush_range_for_net((unsigned long)pRx, (unsigned long)(pRx + 1) - 1);
		pRx = (struct _rx_desc*)g_current_rx->rdes3;
		_dcache_inv_range_for_net((unsigned long)pRx, (unsigned long)(pRx + 1) - 1);
		g_current_rx = pRx;
		rxnum++;
		NetReceive(NetRxPackets[0], len);
		eth_rx_dump(NetRxPackets[0], len);
	}

	return len;
}

static int aml_ethernet_init(struct eth_device * net_current, bd_t *bd)
{
#ifdef CONFIG_M6
	unsigned net_dma_start_addr = (unsigned)0x8f880000;
	unsigned net_dma_end_addr   = (unsigned)0x8f8a0000;
#else
	unsigned net_dma_start_addr = (unsigned)&__net_dma_start;
	unsigned net_dma_end_addr   = (unsigned)&__net_dma_end;
#endif
	unsigned tx_start, rx_start;
	struct _rx_desc * pRDesc;
	struct _tx_desc * pTDesc;
	unsigned char * bufptr;
	int i;

	if (g_nInitialized) {
		return 0;
	}
	printf("Amlogic Ethernet Init\n");

	/* init the dma descriptor 128k */
	gS = (struct _gStruct*)malloc(sizeof(struct _gStruct));
	if (net_dma_start_addr != 0 &&
	    (net_dma_end_addr - net_dma_start_addr) > (CTX_BUFFER_NUM + CRX_BUFFER_NUM)*CBUFFER_SIZE +
	    CRX_BUFFER_NUM * sizeof(struct _rx_desc) + CTX_BUFFER_NUM * sizeof(struct _tx_desc)) {

		g_rx = (struct _rx_desc*)((unsigned long)net_dma_start_addr + (CTX_BUFFER_NUM + CRX_BUFFER_NUM) * CBUFFER_SIZE);
		g_tx = (struct _tx_desc*)((char *)g_rx + CRX_BUFFER_NUM * sizeof(struct _rx_desc));
	} else {
		printf("Error!! Ethernet DMA size is smaller");
		goto error_out;
	}
	gS->rx = g_rx;
	gS->tx = g_tx;
	tx_start = net_dma_start_addr;
	rx_start = net_dma_start_addr + CTX_BUFFER_NUM * CBUFFER_SIZE;
	gS->rx_buf_addr = rx_start;
	gS->tx_buf_addr = tx_start;

	/* init mStruct */
	gS->current_rx_des = 0;
	gS->current_tx_des = 0;
	gS->rx_len = CRX_BUFFER_NUM;
	gS->tx_len = CTX_BUFFER_NUM;
	gS->buffer_len = CBUFFER_SIZE;
	gS->rx_frame_num = 0;
	gS->current_tx_ready = 0;
	gS->last_tx_sent = 0;
	gS->last_tx_desc_num = 0;
	gS->irq_handle = -1;
	gS->linked = 0;

	if (g_debug > 1) {
		printf("netdma:[0x%x-0x%x]\n", net_dma_start_addr, net_dma_end_addr);
		printf("tx_buf_num:%d \t rx_buf_num:%d buf_size:%d\n", CTX_BUFFER_NUM, CRX_BUFFER_NUM, CBUFFER_SIZE);
		printf("[===dma_tx] 0x%x-0x%x\n", tx_start, rx_start);
		printf("[===dma_rx] 0x%x-0x%x\n", rx_start, g_rx);
	}
	/* init RX desc */
	pRDesc = gS->rx;
	bufptr = (unsigned char *) gS->rx_buf_addr;
	for (i = 0; i < gS->rx_len - 1; i++) {
		if (g_debug > 1) {
			printf("[rx-descriptor%d] 0x%x\n", i, bufptr);
		}
		pRDesc->rdes0 = RDES0_OWN;
		pRDesc->rdes1 = RDES1_RCH | (gS->buffer_len & RDES1_RBS1_MASK);
		pRDesc->rdes2 = (unsigned long)bufptr;
		pRDesc->rdes3 = (unsigned long)pRDesc + sizeof(struct _rx_desc);
		pRDesc->reverse[0] = 0;
		pRDesc->reverse[1] = 0;
		pRDesc->reverse[2] = 0;
		pRDesc->reverse[3] = 0;
		bufptr += gS->buffer_len;
		pRDesc = pRDesc + 1;

	}
	pRDesc->rdes0 = RDES0_OWN;
	pRDesc->rdes1 = RDES1_RCH | RDES1_RER | (gS->buffer_len & RDES1_RBS1_MASK); //chain buf
	pRDesc->rdes2 = (unsigned long)bufptr;
	pRDesc->rdes3 = (unsigned long)gS->rx; 		//circle
	_dcache_flush_range_for_net((unsigned)gS->rx, (unsigned)gS->rx + sizeof(struct _rx_desc)*gS->rx_len);

	/* init TX desc */
	pTDesc = (struct _tx_desc *)gS->tx;
	bufptr = (unsigned char *)gS->tx_buf_addr;
	for (i = 0; i < gS->tx_len - 1; i++) {
		if (g_debug > 1) {
			printf("[tx-descriptor%d] 0x%x\n", i, bufptr);
		}
		pTDesc->tdes0 = 0;
		pTDesc->tdes1 = TDES1_TCH | TDES1_IC;
		pTDesc->tdes2 = (unsigned long)bufptr;
		pTDesc->tdes3 = (unsigned long)pTDesc + sizeof(struct _tx_desc);
		pTDesc->reverse[0] = 0;
		pTDesc->reverse[1] = 0;
		pTDesc->reverse[2] = 0;
		pTDesc->reverse[3] = 0;
		bufptr += gS->buffer_len;
		pTDesc = pTDesc + 1;
	}
	pTDesc->tdes0 = 0;
	pTDesc->tdes1 = TDES1_TCH | TDES1_TER | TDES1_IC; 	//chain buf, enable complete interrupt
	pTDesc->tdes2 = (unsigned long)bufptr;
	pTDesc->tdes3 = (unsigned long)gS->tx; 		//circle
	g_current_tx = gS->tx;
	_dcache_flush_range_for_net((unsigned)gS->tx, (unsigned)gS->tx + sizeof(struct _tx_desc)*gS->tx_len);

	/* mac and phy reset */
	eth_reset(gS);

#ifndef CONFIG_RANDOM_MAC_ADDR
	/* set mac addr */
	eth_getenv_enetaddr("ethaddr", g_bi_enetaddr);
	set_mac_addrs(g_bi_enetaddr);

	/* get the mac and ip */
	printf("MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", g_bi_enetaddr[0], g_bi_enetaddr[1],
	           g_bi_enetaddr[2], g_bi_enetaddr[3], g_bi_enetaddr[4], g_bi_enetaddr[5]);
#endif
	/* start the dma para, but don't start the receive dma */
	writel(ETH_DMA_6_Operation_Mode_EFC | ETH_DMA_6_Operation_Mode_TTC_16 | ETH_DMA_6_Operation_Mode_RSF | ETH_DMA_6_Operation_Mode_DT, ETH_DMA_6_Operation_Mode);
	// | ETH_DMA_6_Operation_Mode_RTC_32 | ETH_DMA_6_Operation_Mode_FUF

	netdev_chk();
	DMARXStart();

	g_nInitialized = 1;
	return 0;

error_out:
	return -1;
}

int aml_eth_init(bd_t *bis)
{
	struct eth_device *dev;
	dev = (struct eth_device *)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "Meson_Ethernet");
	dev->init	= aml_ethernet_init;
	dev->halt 	= aml_eth_halt;
	dev->send	= aml_eth_send;
	dev->recv	= aml_eth_rx;
	return eth_register(dev);
}

static int do_phyreg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int phyad, reg, value;
	unsigned char *cmd = NULL;
	unsigned int i;

	if (argc  < 2) {
		return cmd_usage(cmdtp);
	}

	phyad = detect_phyad();
	if (phyad > 32 || phyad < 0) {
		return -1;
	}

	cmd = argv[1];
	switch (*cmd) {
	case 'd':
		printf("=== ethernet phy register dump:\n");
		for (i = 0; i < 32; i++)
			printf("[reg_%d] 0x%x\n", i, phy_reg_rd(phyad, i));
		break;
	case 'r':
		if (argc != 3) {
			return cmd_usage(cmdtp);
		}
		printf("=== ethernet phy register read:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		printf("[reg_%d] 0x%x\n", reg, phy_reg_rd(phyad, reg));

		break;
	case 'w':
		if (argc != 4) {
			return cmd_usage(cmdtp);
		}
		printf("=== ethernet phy register write:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		value = simple_strtoul(argv[3], NULL, 16);
		phy_reg_wr(phyad, reg, value);
		printf("[reg_%d] 0x%x\n", reg, phy_reg_rd(phyad, reg));
		break;

	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

static int do_macreg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int reg, value;
	unsigned char *cmd = NULL;
	unsigned int i;

	if (argc  < 2) {
		return cmd_usage(cmdtp);
	}

	cmd = argv[1];
	switch (*cmd) {
	case 'd':
		printf("=== ETH_MAC register dump:\n");
		//for (i = 0x0000; i <= 0x00FC; i += 0x4)
		for (i = 0x0000; i <= 0x004C; i += 0x4)
			printf("[0x%04x] 0x%x\n", i, readl(ETH_BASE + i));
#if 0
		printf("=== ETH_MMC register dump:\n");
		for (i = 0x0100; i <= 0x0284; i += 0x4)
			printf("[0x%04x] 0x%x\n", i, readl(ETH_BASE + i));
#endif
		printf("=== ETH_DMA register dump:\n");
		for (i = 0x1000; i <= 0x1054; i += 0x4)
			printf("[0x%04x] 0x%x\n", i, readl(ETH_BASE + i));

		printf("=== ethernet board config register dump:\n");
		printf("[0x1076] 0x%x\n", READ_CBUS_REG(0x1076));
		printf("[0x2032] 0x%x\n", READ_CBUS_REG(0x2032));
		printf("[0x2042] 0x%x\n", READ_CBUS_REG(0x2042));
		break;
	case 'r':
		if (argc != 3) {
			return cmd_usage(cmdtp);
		}
		printf("=== ethernet mac register read:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		printf("[0x%04x] 0x%x\n", i, readl(ETH_BASE + reg));

		break;
	case 'w':
		if (argc != 4) {
			return cmd_usage(cmdtp);
		}
		printf("=== ethernet mac register write:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		value = simple_strtoul(argv[3], NULL, 16);
		writel(value, ETH_BASE + reg);
		printf("[0x%04x] 0x%x\n", reg, value);
		break;

	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

static int do_cbusreg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int reg, value;
	unsigned char *cmd = NULL;
	unsigned int i;

	if (argc < 3) {
		return cmd_usage(cmdtp);
	}

	cmd = argv[1];
	switch (*cmd) {
	case 'r':
		if (argc != 3) {
			return cmd_usage(cmdtp);
		}
		printf("=== cbus register read:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		printf("[0x%04x] 0x%x\n", reg, READ_CBUS_REG(reg));

		break;
	case 'w':
		if (argc != 4) {
			return cmd_usage(cmdtp);
		}
		printf("=== cbus register write:\n");
		reg = simple_strtoul(argv[2], NULL, 10);
		value = simple_strtoul(argv[3], NULL, 16);
		WRITE_CBUS_REG(reg, value);
		printf("[0x%04x] 0x%x\n", reg, READ_CBUS_REG(reg));
		break;

	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

//loopback test.
static int do_autoping(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int phyad = -1;
	unsigned int value;
	char buffer[40];

	if (argc < 2) {
		return cmd_usage(cmdtp);
	}

	phyad = detect_phyad();
	if (phyad > 32 || phyad < 0) {
		return -1;
	}

	value = phy_reg_rd(phyad, PHY_CR);
	phy_reg_wr(phyad, PHY_CR, value | (1 << 14)); //phy loopback
	while (1) {
		if (had_ctrlc()) {
			printf("Quit autoping...\n");
			return 0;
		}
		sprintf(buffer, "ping %s ", argv[1]);
		run_command(buffer, 0);
	}
	return 0;
}

int do_ethchk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	netdev_chk();
	return 0;
}

#ifdef CONFIG_M6
static void hardware_reset(void)
{
	/* PHY hardware reset */
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO5_O, 1 << 15);
	udelay(500);
	SET_CBUS_REG_MASK(PREG_PAD_GPIO5_O, 1 << 15);
	udelay(500);
	printf("ETH PHY hardware reset OK\n");

	return;
}
#endif

static int do_ethrst(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_M6
	hardware_reset();
#endif
	eth_reset(gS);

	return 0;
}

static int do_ethmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int reg, value;
	unsigned char *cmd = NULL;
	unsigned int i;

	if (argc  < 2) {
		return cmd_usage(cmdtp);
	}

	cmd = argv[1];
	switch (*cmd) {
	case '0':
		g_mac_mode = 0;
		printf("set MAC mode: RMII 100/10 Mbps (external clk).\n");
		break;
	case '1':
		g_mac_mode = 1;
		printf("set MAC mode: RMII 100/10 Mbps (internal clk).\n");
		break;
	case '2':
		g_mac_mode = 2;
		printf("set MAC mode: RGMII 1000/100/10 Mbps.\n");
		break;

	default:
		return cmd_usage(cmdtp);
	}

#ifdef CONFIG_M6
	/* config ethernet mode */
	switch (g_mac_mode) {
	case MAC_MODE_RMII_CLK_EXTERNAL:
		WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x130); //clock phase invert
		WRITE_CBUS_REG(PERIPHS_PIN_MUX_6, 0x8007ffe0);
		WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x241);
		break;
	case MAC_MODE_RMII_CLK_INTERNAL:
		WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x702);
		WRITE_CBUS_REG(PERIPHS_PIN_MUX_6, 0x4007ffe0);
		WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x241);
		break;
	case MAC_MODE_RGMII:
		WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x309);
		WRITE_CBUS_REG(PERIPHS_PIN_MUX_6, 0x4007ffe0);
		WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x211);
		break;
	default:
		break;
	}

	udelay(1000);
	hardware_reset();
	eth_reset(gS);
#endif

	return 0;
}

static int do_ethdbg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char *cmd = NULL;

	if (argc  < 2) {
		return cmd_usage(cmdtp);
	}

	cmd = argv[1];
	switch (*cmd) {
	case '0':
		g_debug = 0;
		break;
	case '1':
		g_debug = 1;
		break;
	case '2':
		g_debug = 2;
		break;
	case '3':
		g_debug = 3;
		break;
	case '4':
		g_debug = 4;
		break;

	default:
		return cmd_usage(cmdtp);
	}
	printf("set ethernet debug: %d\n", g_debug);

	return 0;
}

static unsigned int clk_util_clk_msr(unsigned int clk_mux)
{
	unsigned int regval = 0;
	WRITE_CBUS_REG(MSR_CLK_REG0, 0);
	//Set the measurement gate to 64uS
	CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, 0xffff);
	SET_CBUS_REG_MASK(MSR_CLK_REG0, (64 - 1)); //64uS is enough for measure the frequence?
	//Disable continuous measurement
	//Disable interrupts
	CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, ((1 << 18) | (1 << 17)));
	CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, (0x1f << 20));
	SET_CBUS_REG_MASK(MSR_CLK_REG0, (clk_mux << 20) | // Select MUX
						(1 << 19) |       // enable the clock
						(1 << 16));       //enable measuring
	//Wait for the measurement to be done
	regval = READ_CBUS_REG(MSR_CLK_REG0);
	do {
		regval = READ_CBUS_REG(MSR_CLK_REG0);
	} while (regval & (1 << 31));

	//Disable measuring
	CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, (1 << 16));
	regval = (READ_CBUS_REG(MSR_CLK_REG2) + 31) & 0x000FFFFF;

	return (regval >> 6);
}

static int do_clkmsr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char* clk_table[] = {
		" CTS_PWM_A_CLK(45)",
		" CTS_PWM_B_CLK(44)",
		" CTS_PWM_C_CLK(43)",
		" CTS_PWM_D_CLK(42)",
		" CTS_ETH_RX_TX (41)",
		" CTS_PCM_MCLK(40)",
		" CTS_PCM_SCLK(39)",
		" CTS_VDIN_MEAS_CLK(38)",
		" CTS_VDAC_CLK1(37)",
		" CTS_HDMI_TX_PIXEL_CLK(36)",
		" CTS_MALI_CLK (35)",
		" CTS_SDHC_CLK1(34)",
		" CTS_SDHC_CLK0(33)",
		" CTS_AUDAC_CLKPI(32)",
		" CTS_A9_CLK(31)",
		" CTS_DDR_CLK(30)",
		" CTS_VDAC_CLK0(29)",
		" CTS_SAR_ADC_CLK (28)",
		" CTS_ENCI_CL(27)",
		" SC_CLK_INT(26)",
		" USB_CLK_12MHZ (25)",
		" LVDS_FIFO_CLK (24)",
		" HDMI_CH3_TMDSCLK(23)",
		" MOD_ETH_CLK50_I (22)",
		" MOD_AUDIN_AMCLK_I  (21)",
		" CTS_BTCLK27 (20)",
		" CTS_HDMI_SYS_CLK(19)",
		" CTS_LED_PLL_CLK(18)",
		" CTS_VGHL_PLL_CLK (17)",
		" CTS_FEC_CLK_2(16)",
		" CTS_FEC_CLK_1 (15)",
		" CTS_FEC_CLK_0 (14)",
		" CTS_AMCLK(13)",
		" VID2_PLL_CLK(12)",
		" CTS_ETH_RMII(11)",
		" CTS_ENCT_CLK(10)",
		" CTS_ENCL_CLK(9)",
		" CTS_ENCP_CLK(8)",
		" CLK81(7)",
		" VID_PLL_CLK(6)",
		" AUD_PLL_CLK(5)",
		" MISC_PLL_CLK(4)",
		" DDR_PLL_CLK(3)",
		" SYS_PLL_CLK(2)",
		" AM_RING_OSC_CLK_OUT1(1)",
		" AM_RING_OSC_CLK_OUT0(0)",
	};
	int i;
	int index = 0xff;

	if (argc ==  2) {
		index = simple_strtoul(argv[1], NULL, 10);
	}

	if (index == 0xff) {
		for (i = 0; i < sizeof(clk_table) / sizeof(char *); i++) {
			printf("[%4d MHz] %s\n", clk_util_clk_msr(i), clk_table[45-i]);
		}
		return 0;
	}
	printf("[%4d MHz] %s\n", clk_util_clk_msr(index), clk_table[45-index]);

	return 0;
}

U_BOOT_CMD(
	phyreg, 4, 1, do_phyreg,
	"ethernet phy register read/write/dump",
	"d            - dump phy registers\n"
	"       r reg        - read phy register\n"
	"       w reg val    - write phy register"
);

U_BOOT_CMD(
	macreg, 4, 1, do_macreg,
	"ethernet mac register read/write/dump",
	"d            - dump mac registers\n"
	"       r reg        - read mac register\n"
	"       w reg val    - write mac register"
);

U_BOOT_CMD(
	cbusreg, 4, 1, do_cbusreg,
	"cbus register read/write",
	"r reg        - read cbus register\n"
	"        w reg val    - write cbus register"
);

U_BOOT_CMD(
	autoping, 2, 1, do_autoping,
	"do auto ping test",
	"ip"
);

U_BOOT_CMD(
	ethchk, 1, 1, do_ethchk,
	"check ethernet status",
	""
);

U_BOOT_CMD(
	ethrst, 1, 1, do_ethrst,
	"reset ethernet phy",
	"             - reset etherent phy\n"
);

U_BOOT_CMD(
	ethmode, 2, 1, do_ethmode,
	"set ethernet mac mode",
	"0         - set mac mode RMII (external clk).\n"
	"        1         - set mac mode RMII (internal clk).\n"
	"        2         - set mac mode RGMII.\n"
);

U_BOOT_CMD(
	ethdbg, 2, 1, do_ethdbg,
	"set ethernet debug level",
	"0         - disable ethernet debug\n"
	"       1         - ethernet basic info\n"
	"       2         - ethernet TX debug\n"
	"       3         - ethernet RX debug\n"
	"       4         - ethernet TX/RX debug\n"
);

U_BOOT_CMD(
	clkmsr, 2, 1, do_clkmsr,
	"measure PLL clock",
	"             - measure PLL clock.\n"
);
