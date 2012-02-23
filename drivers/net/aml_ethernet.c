/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: Enthernet driver for nike.
 *
 *  Author: Min Chu
 *  Created: 2009-3-18 
 *  
 *  Remark: 2011.07.22 merged from trunk by Hisun Bao
 *
 *******************************************************************/
#include <linux/types.h>
#include <config.h>				
#include <malloc.h>
#include <common.h>
#include <net.h>

#include <asm/u-boot.h>
#include <asm/cache.h>

//following header file exist @ arch/arm/include/asm/arch-m1/m2/m3...
#include <asm/arch/io.h> 
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/aml_emac_lan8700.h>


//flollowing lable locate @ arch/arm/cpu/aml_meson
extern unsigned __net_dma_start;
extern unsigned __net_dma_end;

static unsigned char g_bi_enetaddr[6]={0,0,0,0,0,0};
static struct _gStruct* 	gS = NULL;
static struct _rx_desc* 	g_rx = NULL;
static struct _tx_desc* 	g_tx = NULL;
static struct _rx_desc* 	g_current_rx = NULL;
static struct _tx_desc* 	g_current_tx = NULL;
static int g_nInitialized = 0 ;
static unsigned long phy_Identifier=0;
#define PHY_SMSC_8700			0x7c0c4
#define PHY_ATHEROS_8032		0x004dd023
#define PHY_ATHEROS_8035		
#define RMII_CLK_OUT	1
#define RMII_CLK_IN		0
static  int debug_enable=0;
static char   rmii_clk_out =RMII_CLK_IN;
#define aml_printf   debug_enable&&printf
//#define ET_DEBUG

/*
  * phyid [in]:
  * reg [in]: physical layer chip id
  * val [in]: 
  */
void phy_reg_wr(int phyid, unsigned int reg, unsigned int val)
{
	unsigned long busy = 0, tmp = 0;
	unsigned int phyaddr;
	unsigned int phyreg;
	unsigned long reg4;

	phyaddr = phyid << ETH_MAC_4_GMII_Addr_PA_P;
	phyreg  = reg << ETH_MAC_4_GMII_Addr_GR_P;

	reg4 = phyaddr | phyreg | ETH_MAC_4_GMII_Addr_CR_100_150 | ETH_MAC_4_GMII_Addr_GW | ETH_MAC_4_GMII_Addr_GB;
	writel(val, ETH_MAC_5_GMII_Data);
	writel(reg4, ETH_MAC_4_GMII_Addr);
	busy = 1;
	while (busy) 
	{
		tmp = readl(ETH_MAC_4_GMII_Addr);
		busy = tmp&1;
	}
}

/*
  * phyid [in]:
  * reg [in]: physical layer chip id
  * val [out]: 
  */
unsigned int phy_reg_rd(int phyid, unsigned int reg)
{
	unsigned long busy = 0, tmp = 0;
	unsigned int phyaddr;
	unsigned int phyreg;
	unsigned long reg4;

	phyaddr = phyid << ETH_MAC_4_GMII_Addr_PA_P;
	phyreg  = reg << ETH_MAC_4_GMII_Addr_GR_P;
	reg4 = phyaddr | phyreg | ETH_MAC_4_GMII_Addr_CR_100_150 | ETH_MAC_4_GMII_Addr_GB;
	
	writel(reg4, ETH_MAC_4_GMII_Addr);
	
	busy = 1;
	while (busy) 
	{
		tmp = readl(ETH_MAC_4_GMII_Addr);
		busy = tmp&1;
	}

	tmp = readl(ETH_MAC_5_GMII_Data);

	return tmp;
}



static inline void _dcache_flush_range_for_net(unsigned startAddr, unsigned endAddr)
{
    dcache_clean_range(startAddr,endAddr-startAddr+1);
    return;
}

static inline void _dcache_inv_range_for_net(unsigned startAddr, unsigned endAddr)
{
    dcache_flush_range(startAddr,endAddr-startAddr+1);
    dcache_invalid_range(startAddr,endAddr-startAddr+1);
    return;
}

int eth_io_init(void)
{
	return 0;
}

static unsigned int detect_phyid(void)
{
	unsigned int testval = 0;
	int i;
	static int phy_id=-1;

	if(phy_id!=-1)
		return phy_id;
	for (i = 0; i < 32; i++) 
	{
		testval = phy_reg_rd(i, PHY_SR);	//read the SR register..
		if (testval !=0x0000 && testval !=0xffff) 
		{
			phy_id=i;
			return phy_id;
		}
	}
	return 0xffff;
}

void set_mac_addrs(void *ptr)
{
	unsigned int mac_filter = 0;
	unsigned char * p = (unsigned char *)ptr;
	writel((ETH_MAC_0_Configuration_PS_MII | ETH_MAC_0_Configuration_FES_100M | ETH_MAC_0_Configuration_DM
			| ETH_MAC_0_Configuration_RE | ETH_MAC_0_Configuration_TE), ETH_MAC_0_Configuration);
	writel((ETH_MAC_1_Frame_Filter_PM |ETH_MAC_1_Frame_Filter_RA), ETH_MAC_1_Frame_Filter);

	mac_filter = (p[5]<<8) | p[4];
	writel(mac_filter, ETH_MAC_Addr0_High);
	mac_filter = (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
	writel(mac_filter, ETH_MAC_Addr0_Low);
}
static void cmd_netdev_chk(void)
{
	unsigned int rint, rint2;
	static unsigned int old_rint=-1;
	unsigned int id;
	int s100,full;
	int  an_enable=1;
	id=detect_phyid();
	an_enable=phy_reg_rd(id,0)&(1<<12);
	if(an_enable)
	{
		rint2 = 3000;
		do {
			rint = phy_reg_rd(id,PHY_SR);
			if((rint & PHY_SR_ANCOMPLETE))
			break;
			udelay(1000);
		} while (rint2-->0);
		if(!(rint & PHY_SR_ANCOMPLETE) )
		aml_printf("phy auto link failed\n");
	}else{
		rint = phy_reg_rd(id,PHY_SR);
	}	
	if(1)
	{
	    switch(phy_Identifier)
	    	{
	    	case PHY_ATHEROS_8032:
				rint2 = phy_reg_rd(id,17);
				s100 = rint2 & (1<<14);
				full = ((rint2) & (1<<13));
				gS->linked = rint2&(1<<10);
				break;
		case PHY_SMSC_8700:
			default:
				rint2 = phy_reg_rd(id,31);
				s100 = rint2 & (1<<3);
				full = ((rint2 >>4) & 1);
				gS->linked = rint2&(1<<2);
				break;
		//case  PHY_
	    	}
		/* phy_auto_negotiation_set */
		if(full)
		{
			aml_printf("duplex\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		}
		else
		{
			aml_printf("half duplex\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		}
		if(!s100)
		{
			aml_printf("10m\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks     
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 20			
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 20
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
		}
		else
		{
			aml_printf("100m\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);	// program mac
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks     
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 2		
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 2
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
		}
		
		/* link_changed */
#if defined(ET_DEBUG)
		unsigned int regs=0,val;		
		for(regs=0; regs<=31; regs++)
		{
			val = phy_reg_rd(id,regs);
			aml_printf("reg[%d]=%x\n",regs,(unsigned)val);
		}
#endif		
		old_rint=rint;
	}
}
static void netdev_chk(void)
{
	unsigned int rint, rint2;
	static unsigned int old_rint=-1;
	unsigned int id;
	int s100,full;
	int  an_enable=1;
	id=detect_phyid();
	an_enable=phy_reg_rd(id,0)&(1<<12);
	if(an_enable)
	{
		rint2 = 3000;
		do {
			rint = phy_reg_rd(id,PHY_SR);
			if((rint & PHY_SR_ANCOMPLETE))
			break;
			udelay(1000);
		} while (rint2-->0);
		if(!(rint & PHY_SR_ANCOMPLETE) )
		aml_printf("phy auto link failed\n");
	}else{
		rint = phy_reg_rd(id,PHY_SR);
	}	

	if(old_rint!=rint)
	{
	    switch(phy_Identifier)
	    	{
	    	case PHY_ATHEROS_8032:
				rint2 = phy_reg_rd(id,17);
				s100 = rint2 & (1<<14);
				full = ((rint2) & (1<<13));
				gS->linked = rint2&(1<<10);
				break;
			case PHY_SMSC_8700:
			default:
				rint2 = phy_reg_rd(id,31);
				s100 = rint2 & (1<<3);
				full = ((rint2 >>4) & 1);
				gS->linked = rint2&(1<<2);
				break;
	    	}
		/* phy_auto_negotiation_set */
		if(full)
		{
			aml_printf("duplex\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		}
		else
		{
			aml_printf("half duplex\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_DM, ETH_MAC_0_Configuration);
		}
		if(!s100)
		{
			aml_printf("10m\n");
			writel(readl(ETH_MAC_0_Configuration) & ~ ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks     
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 20			
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 20
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
		}
		else
		{
			aml_printf("100m\n");
			writel(readl(ETH_MAC_0_Configuration) | ETH_MAC_0_Configuration_FES_100M, ETH_MAC_0_Configuration);	// program mac
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// Disable the Ethernet clocks     
			// ---------------------------------------------
			// Test 50Mhz Input Divide by 2
			// ---------------------------------------------
			// Select divide by 2		
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DESEND, ETH_PLL_CNTL);  	// desc endianess "same order"
			//writel(readl(ETH_PLL_CNTL) & ~ETH_PLL_CNTL_DATEND, ETH_PLL_CNTL); 	// data endianess "little"
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_MACSPD, ETH_PLL_CNTL);	// divide by 2
			//writel(readl(ETH_PLL_CNTL) | ETH_PLL_CNTL_DIVEN, ETH_PLL_CNTL);		// enable Ethernet clocks
		}
		
		/* link_changed */
#if defined(ET_DEBUG)
		unsigned int regs=0,val;		
		for(regs=0; regs<=31; regs++)
		{
			val = phy_reg_rd(id,regs);
			aml_printf("reg[%d]=%x\n",regs,(unsigned)val);
		}
#endif		
		old_rint=rint;
	}
}

static void  hardware_reset(void)
{
	aml_printf("net clock %s\n",(rmii_clk_out?"out":"in"));
	switch (rmii_clk_out)
	{
		case RMII_CLK_OUT:
		WRITE_CBUS_REG(0x1076,0x303);
		WRITE_CBUS_REG(0x2032,0x4007ffe0);

		break;
		case RMII_CLK_IN:
		WRITE_CBUS_REG(0x1076,0x130); //clock phase invert  
		WRITE_CBUS_REG(0x2032,0x8007ffe0);
		CLEAR_CBUS_REG_MASK(0x201c,1<<15); //phy reset
	 	udelay(500);
		SET_CBUS_REG_MASK(0x201c,1<<15);
		udelay(1);//skip reset config read.
		break;
	}
}
/* Reset and idle the chip, putting all registers into
 * a reasonable state */
static int eth_reset(struct _gStruct* emac_config)
{
	int i, k,phyad;
	unsigned int val;
	struct _gStruct* m=emac_config;
	hardware_reset();
	
	if(rmii_clk_out==RMII_CLK_IN) //in clock in mode. after reset ,you need turn on phy clock by quit power down mode.
	{
		phyad=detect_phyid();
		val = 1<<13|PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE<<PHY_SPMD_MODE_P)  | (phyad<<PHY_SPMD_PHYAD_P);
		do{
		phy_reg_wr(1, PHY_SPMD, val);
		}while(phy_reg_rd(1, PHY_SPMD)!=val);
	}
#define NET_MAX_RESET_TEST	20	

	 for(i=0;i<NET_MAX_RESET_TEST;i++)
	{
		writel(ETH_DMA_0_Bus_Mode_SWR, ETH_DMA_0_Bus_Mode);
		
		for (k= 0;k<NET_MAX_RESET_TEST; k++) 
		{
			udelay(100);
			
			//asm("ldr r1,=0xc9011000");
			//asm("ldr r0,[r1]");
			//asm("bl disp_r0");
			//asm("WFI");
			
			if(!(readl(ETH_DMA_0_Bus_Mode)&ETH_DMA_0_Bus_Mode_SWR))
				break;			
		}
		if(k>=NET_MAX_RESET_TEST)
		{
			aml_printf("Error! Fail to reset mac!\n");
			return -1;
		}else{
			aml_printf("Success: reset mac OK!\n");		
		}
		
	 	phyad=detect_phyid();
		if(phyad>32 || phyad<0)
		{
			continue;
		}
	   val = 1<<13|PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE<<PHY_SPMD_MODE_P)  | (phyad<<PHY_SPMD_PHYAD_P);
		phy_reg_wr(phyad, PHY_SPMD, val);
        val = phy_reg_rd(phyad,2);
		phy_Identifier=val<<16;
		val = phy_reg_rd(phyad,3);
		phy_Identifier|=val;
		aml_printf("find net phy id=0x%x, phyad=%d\n",(unsigned int)phy_Identifier,phyad);
	    // soft reset
		phy_reg_wr(phyad, PHY_CR, PHY_CR_RST);
	    // may be smaller value??  Ask SMSC
		for (k=0; k<1000; k++) 
		{
	    	udelay(1000);
			val = phy_reg_rd(phyad, PHY_CR);
			if (!(val & PHY_CR_RST) )
				break;
		}
		if(k>=1000)
			continue;
		else
			break;
	} 
	if(i>=NET_MAX_RESET_TEST)
	{
		aml_printf("Error to detected phy\n");
		return -1;
	}
	
	val = 1<<13|PHY_SPMD_MIIMODE_RMII | (PHY_MODE_BUS_ALL_AE<<PHY_SPMD_MODE_P)  | (phyad<<PHY_SPMD_PHYAD_P);
	phy_reg_wr(phyad, PHY_SPMD, val);
	
	val = PHY_CR_AN | PHY_CR_RSTAN;
	phy_reg_wr(phyad, PHY_CR, val);

	udelay(10);
	writel((~0), ETH_DMA_5_Status);							/* clear all status flag */
	writel(0, ETH_DMA_5_Status);
	writel(0, ETH_DMA_6_Operation_Mode);					/* stop RX and TX */
	val=readl(ETH_DMA_8_Missed_Frame_and_Overflow);		/* read to clean */

	writel(0, ETH_DMA_7_Interrupt_Enable);					/* disable all interrupt */
	writel((8<<ETH_DMA_0_Bus_Mode_PBL_P) | ETH_DMA_0_Bus_Mode_FB, ETH_DMA_0_Bus_Mode);

	aml_printf("final_addr[rx-tx]:0x%x-0x%x\n",m->rx,m->tx);
	writel((long)m->rx, ETH_DMA_3_Re_Descriptor_List_Addr);
	writel((long)m->tx, ETH_DMA_4_Tr_Descriptor_List_Addr);

	/* config the interrupt */
	writel(ETH_DMA_7_Interrupt_Enable_TUE | ETH_DMA_7_Interrupt_Enable_TJE
					| ETH_DMA_7_Interrupt_Enable_OVE | ETH_DMA_7_Interrupt_Enable_UNE | ETH_DMA_7_Interrupt_Enable_RIE
					| ETH_DMA_7_Interrupt_Enable_RUE | ETH_DMA_7_Interrupt_Enable_RSE | ETH_DMA_7_Interrupt_Enable_FBE
					| ETH_DMA_7_Interrupt_Enable_AIE | ETH_DMA_7_Interrupt_Enable_NIE, ETH_DMA_7_Interrupt_Enable);
	writel(0, ETH_MAC_Interrupt_Mask);
	aml_printf("eth reset ok\n");
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

//static void DMARXStop(void)
//{
//	writel(readl(ETH_DMA_6_Operation_Mode) & ~ETH_DMA_6_Operation_Mode_SR, ETH_DMA_6_Operation_Mode);
//}

/*static void DMATXStop(void)
{
	writel(readl(ETH_DMA_6_Operation_Mode) & ~ETH_DMA_6_Operation_Mode_ST, ETH_DMA_6_Operation_Mode);
}*/

static void GetDMAStatus(unsigned int* mask,unsigned  int* status)
{
	*mask = readl(ETH_DMA_7_Interrupt_Enable);
	*status = readl(ETH_DMA_5_Status);
}

/*static void chk_dma_stat(void)
{
	unsigned int mask;
	unsigned int status;

	GetDMAStatus(&mask,&status);
	
   if(status&NOR_INTR_EN)  //Normal Interrupts Process
   {
   		if(status&TX_INTR_EN)  //Transmit Interrupt Process
   		{
   			writel(1<<0|1<<16, ETH_DMA_5_Status);
			//AVSemPost(tx_sem);
  		}
		if(status&RX_INTR_EN) //Receive Interrupt Process
		{
			writel(1<<6|1<<16, ETH_DMA_5_Status);
		}
		if(status&EARLY_RX_INTR_EN)
		{
			writel(EARLY_RX_INTR_EN|NOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&TX_BUF_UN_EN){
		
#if defined(ET_DEBUG)
        volatile unsigned long Cdma,Dstatus,mstatus,buf_addr,count;
        Cdma = readl(ETH_DMA_18_Curr_Host_Tr_Descriptor);

        Dstatus=*(volatile unsigned long *)Cdma;
        count=*(volatile unsigned long *)(Cdma+4);
        buf_addr=*(volatile unsigned long *)(Cdma+8);
        mstatus = readl(ETH_DMA_5_Status);
        if((Dstatus==0x80000000) && (buf_addr!=0)&& count!=0){
			//the apollo can go here.............
			aml_printf("Txdescriptor is invalid Current DMA=%x,Dstatus=%x\n",Cdma,Dstatus);
			aml_printf("Current status=%x,%x\n",mstatus);
			//low_level_resume_output(); 
			}
#endif
		writel(1<<2|1<<16, ETH_DMA_5_Status);
		}
	}
	else if(status&ANOR_INTR_EN){ //Abnormal Interrupts Process
		if(status&RX_BUF_UN){
			writel(RX_BUF_UN|ANOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&RX_STOP_EN){
			writel(RX_STOP_EN|ANOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&RX_WATCH_TIMEOUT){
			writel(RX_WATCH_TIMEOUT|ANOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&FATAL_BUS_ERROR){
			writel(FATAL_BUS_ERROR|ANOR_INTR_EN, ETH_DMA_5_Status);
			//to do: bus error, reset GMAC and DMA
			aml_printf("NET DMA FATAL_BUS_ERROR\n");
			aml_printf("I restart the network....\n");
			eth_reset(gS);
		}
		if(status&EARLY_TX_INTR_EN){
			writel(EARLY_TX_INTR_EN|ANOR_INTR_EN, ETH_DMA_5_Status);			
		}
		if(status&TX_STOP_EN){
			writel(TX_STOP_EN|ANOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&TX_JABBER_TIMEOUT){
			writel(TX_JABBER_TIMEOUT|ANOR_INTR_EN, ETH_DMA_5_Status);
		}
		if(status&RX_FIFO_OVER){
			writel(RX_FIFO_OVER|ANOR_INTR_EN, ETH_DMA_5_Status);	
			DMARXStart();
		}
		if(status&TX_UNDERFLOW){
			writel(TX_UNDERFLOW|ANOR_INTR_EN, ETH_DMA_5_Status);
			DMATXStart();
		}
	}
}*/

static void aml_eth_halt(struct eth_device * net_current)
{

}
static void  dump_tx_pkt(uchar *inpkt,int  len)
{
#define  LINE_LEN  	16

	char line=0;
	int i;
	aml_printf("=====>\n");
	return ;
	while(len > LINE_LEN)
	{
		for(i=0;i<LINE_LEN;i++)
		aml_printf("%02x ",inpkt[line*LINE_LEN+i]);
		aml_printf("\n");
		line++;
		len-=LINE_LEN;
	}
	for(i=0;i<len;i++)
	aml_printf("%02x ",inpkt[line*LINE_LEN+i]);
	aml_printf("\n");
}
static int aml_eth_send(struct eth_device *net_current, volatile void *packet, int length)
{
	unsigned int mask;
   	unsigned int status;
	if(!g_nInitialized)
		return -1;
	dump_tx_pkt(packet,length);
	netdev_chk();
	
	struct _tx_desc* pTx = g_current_tx;
	struct _tx_desc* pDma = (struct _tx_desc* )readl(ETH_DMA_18_Curr_Host_Tr_Descriptor);

	if(pDma!=NULL)
		_dcache_inv_range_for_net((unsigned long)pDma,(unsigned long)(pDma)+sizeof(struct _tx_desc)-1);
	if(pDma!=NULL && !(pDma->tdes0 & TDES0_OWN) && pTx !=pDma)
	{
		//this may not happend,if all the hardware work well...
		//to fixed a bug of the dma maybe lost setting some tx buf to own by host,..;
		//start the current_tx at pDMA
		pTx=pDma;	
	}
	if(pDma!=pTx)
		_dcache_inv_range_for_net((unsigned long)pTx,(unsigned long)(pTx+1)-1);

	if (length > ETH_MTU)
	{
		goto err;
	}
	
	if(length<14)
	{
		aml_printf("pbuf len error,len=%d\n", length);
		goto err;
	}

	if(pTx->tdes0 & TDES0_OWN)
	{
#if 1
		volatile unsigned long Cdma,Dstatus,status;
		Cdma = readl(ETH_DMA_18_Curr_Host_Tr_Descriptor);
		Dstatus = readl(Cdma);
		status = readl(ETH_DMA_5_Status);
		aml_printf("Current DMA=%x,Dstatus=%x\n", (unsigned int)Cdma, (unsigned int)Dstatus);
		aml_printf("Current status=%x\n", (unsigned int)status);
		aml_printf("no buffer to send\n");
#endif
		goto err;
	}

	if(!(unsigned char*)pTx->tdes2)
		goto err;
	g_current_tx=(struct _tx_desc*)pTx->tdes3; 
	memcpy((unsigned char*)pTx->tdes2, (unsigned char*)packet, length);
	_dcache_flush_range_for_net((unsigned)packet,(unsigned)(packet+1500));
	_dcache_inv_range_for_net((unsigned)packet,(unsigned)(packet+1500));//this is the uboot's problem
	//change for add to 60 bytes..
	//by zhouzhi
	if(length<60){
		memset((unsigned char*)(pTx->tdes2 + length), 0, 60-length);
		length=60;
	}
	//pTx->tdes1 &= DescEndOfRing;     
	_dcache_flush_range_for_net((unsigned long)pTx->tdes2, (unsigned long)pTx->tdes2+length-1);	
	pTx->tdes1 = ((length<< TDES1_TBS1_P) & TDES1_TBS1_MASK) | TDES1_FS | TDES1_LS |TDES1_TCH | TDES1_IC;
	pTx->tdes0 = TDES0_OWN;
	_dcache_flush_range_for_net((unsigned long)pTx,(unsigned long)(pTx+1)-1);

	GetDMAStatus(&mask, &status);
	if(status & ETH_DMA_5_Status_TS_SUSP)
		writel(1, ETH_DMA_1_Tr_Poll_Demand);
	else
		DMATXStart();
	
#ifdef ET_DEBUG
	aml_printf("Transfer starting...\n");
	GetDMAStatus(&mask, &status);
	aml_printf("Current status=%x\n",status);	
#endif

	/* wait for transfer to succeed */
	//unsigned tmo = get_timer (0) + 5 * CONFIG_SYS_HZ;		//5S time out
	unsigned tmo = 0;	
	do{
		udelay(100);
		GetDMAStatus(&mask, &status);
		if (tmo++ >= 50000){
			aml_printf ("\ntransmission error %#x\n", status);
			break;
		}
	}while(!((status & ETH_DMA_5_Status_NIS) && (status & ETH_DMA_5_Status_TI)));

	if(status & ETH_DMA_5_Status_NIS){
		if(status & ETH_DMA_5_Status_TI)
			writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_TI, ETH_DMA_5_Status);
		if(status & ETH_DMA_5_Status_TU)
			writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_TU, ETH_DMA_5_Status);
	}

#ifdef ET_DEBUG
	//test
	GetDMAStatus(&mask, &status);
	aml_printf("Current status=%x\n",status);
#endif
	return 0;
err:	   
	return -1;
}

/*
 * each time receive a whole packet
 */
static int aml_eth_rx (struct eth_device * net_current)
{
	unsigned int mask;
   	unsigned int status;
	int rxnum=0;
	int len=0;
	struct _rx_desc* pRx; 
	
	if(!g_nInitialized)
		return -1;

	netdev_chk();

	/* Check packet ready or not */
	GetDMAStatus(&mask, &status);
	if(!((status & ETH_DMA_5_Status_NIS) && (status & ETH_DMA_5_Status_RI)))
	{
		return 0;
	}
	writel(ETH_DMA_5_Status_NIS | ETH_DMA_5_Status_RI, ETH_DMA_5_Status);	//clear the int flag

	if(!g_current_rx)
		g_current_rx = gS->rx;		
	pRx = g_current_rx;
	_dcache_inv_range_for_net((unsigned long)pRx,(unsigned long)(pRx+1)-1);
	while(!(pRx->rdes0 & RDES0_OWN))
	{    	         
		len = (pRx->rdes0 & RDES0_FL_MASK)>>RDES0_FL_P;
		if(14>=len)
		{
			aml_printf("err len=%d\n",len);
			goto NEXT_BUF;
		}
		// pbuf_header(pb, -sizeof(short));
		_dcache_inv_range_for_net((unsigned long)pRx->rdes2,(unsigned long)pRx->rdes2+len-1);	

		if(!memcpy((unsigned char*)NetRxPackets[0], (unsigned char*)pRx->rdes2, len))
		{
			aml_printf("memcp error\n");
			goto NEXT_BUF;
		}
		NEXT_BUF:
		pRx->rdes0 = RDES0_OWN;
		_dcache_flush_range_for_net((unsigned long)pRx,(unsigned long)(pRx+1)-1);
		pRx = (struct _rx_desc*)g_current_rx->rdes3;
		_dcache_inv_range_for_net((unsigned long)pRx,(unsigned long)(pRx+1)-1);
		g_current_rx = pRx; 
		rxnum++;
		NetReceive (NetRxPackets[0], len);
	}
	
//	NetReceive (NetRxPackets[0], len);
	return len;
}

static int aml_ethernet_init(struct eth_device * net_current, bd_t *bd)
{
	//unsigned net_dma_start_addr = (unsigned)&__net_dma_start;
	//unsigned net_dma_end_addr   = (unsigned)&__net_dma_end;
	unsigned net_dma_start_addr = (unsigned)0x8f880000;
	unsigned net_dma_end_addr   = (unsigned)0x8f8a0000;
	unsigned tx_start,rx_start;
	struct _rx_desc * pRDesc;
	struct _tx_desc * pTDesc;
	unsigned char * bufptr;
	
	int i;
	aml_printf("Amlogic eth init\n");
	if(g_nInitialized)
		return 0;

	/* init the dma descriptor 128k */
	gS = (struct _gStruct*)malloc(sizeof(struct _gStruct));
	if(net_dma_start_addr!=0 && 
	(net_dma_end_addr - net_dma_start_addr) > (CTX_BUFFER_NUM+CRX_BUFFER_NUM)*CBUFFER_SIZE+
		CRX_BUFFER_NUM*sizeof(struct _rx_desc)+CTX_BUFFER_NUM*sizeof(struct _tx_desc)){

		g_rx = (struct _rx_desc*)((unsigned long)net_dma_start_addr + (CTX_BUFFER_NUM+CRX_BUFFER_NUM)*CBUFFER_SIZE);
		g_tx = (struct _tx_desc*)((char *)g_rx+CRX_BUFFER_NUM*sizeof(struct _rx_desc));		
	}
	else{
		aml_printf("Error!! Ethernet DMA size is smaller");
		goto error_out;
	}
	gS->rx = g_rx;
	gS->tx = g_tx;		
	tx_start = net_dma_start_addr;
	rx_start = net_dma_start_addr + CTX_BUFFER_NUM*CBUFFER_SIZE;
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
	aml_printf("netdma:[0x%x-0x%x]\n",net_dma_start_addr,net_dma_end_addr);
	aml_printf("tx_buf_num:%d \t rx_buf_num:%d \t buf_size:%d\n",CTX_BUFFER_NUM,CRX_BUFFER_NUM,CBUFFER_SIZE);
	aml_printf("[===dma_tx]0x%x-0x%x\n",tx_start,rx_start);
	aml_printf("[===dma_rx]0x%x-0x%x\n",rx_start,g_rx);
	/* init RX desc */
	pRDesc = gS->rx;
	bufptr = (unsigned char *) gS->rx_buf_addr;
	for (i = 0; i < gS->rx_len - 1; i++) 
	{
		aml_printf("[rx-descriptor%d]0x%x\n",i,bufptr);
		pRDesc->rdes0 = RDES0_OWN;
		pRDesc->rdes1 = RDES1_RCH | (gS->buffer_len & RDES1_RBS1_MASK);
		pRDesc->rdes2 = (unsigned long)bufptr;
		pRDesc->rdes3 = (unsigned long)pRDesc+sizeof(struct _rx_desc);
		pRDesc->reverse[0] = 0;
		pRDesc->reverse[1] = 0;
		pRDesc->reverse[2] = 0;
		pRDesc->reverse[3] = 0;
		bufptr += gS->buffer_len; 
		pRDesc = pRDesc+1;
		
	}
	pRDesc->rdes0 = RDES0_OWN;
	pRDesc->rdes1 = RDES1_RCH | RDES1_RER | (gS->buffer_len & RDES1_RBS1_MASK); //chain buf
	pRDesc->rdes2 = (unsigned long)bufptr;
	pRDesc->rdes3 = (unsigned long)gS->rx; 		//circle
	_dcache_flush_range_for_net((unsigned)gS->rx,(unsigned)gS->rx+sizeof(struct _rx_desc)*gS->rx_len);	

	/* init TX desc */
	pTDesc = (struct _tx_desc *)gS->tx;
	bufptr = (unsigned char *)gS->tx_buf_addr;
	for (i = 0; i < gS->tx_len - 1; i++) 
	{
		aml_printf("[tx-descriptor%d]0x%x\n",i,bufptr);
		pTDesc->tdes0 = 0;
		pTDesc->tdes1 = TDES1_TCH | TDES1_IC;
		pTDesc->tdes2 = (unsigned long)bufptr;
		pTDesc->tdes3 = (unsigned long)pTDesc+sizeof(struct _tx_desc);
		pTDesc->reverse[0] = 0;
		pTDesc->reverse[1] = 0;
		pTDesc->reverse[2] = 0;
		pTDesc->reverse[3] = 0;
		bufptr += gS->buffer_len;
		pTDesc = pTDesc+1;
	}
	pTDesc->tdes0 = 0;
	pTDesc->tdes1 =TDES1_TCH | TDES1_TER | TDES1_IC; 	//chain buf, enable complete interrupt 
	pTDesc->tdes2 = (unsigned long)bufptr;
	pTDesc->tdes3 = (unsigned long)gS->tx; 		//circle
	g_current_tx = gS->tx;
	_dcache_flush_range_for_net((unsigned)gS->tx,(unsigned)gS->tx+sizeof(struct _tx_desc)*gS->tx_len);		

	/* peripheral io init */
	eth_io_init();

	/* reset */
	eth_reset(gS);
	
#ifndef CONFIG_RANDOM_MAC_ADDR	

	/* set mac addr */
	eth_getenv_enetaddr("ethaddr", g_bi_enetaddr);

	set_mac_addrs(g_bi_enetaddr);

	/* get the mac and ip */
	aml_printf("MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", g_bi_enetaddr[0], g_bi_enetaddr[1],
	g_bi_enetaddr[2], g_bi_enetaddr[3], g_bi_enetaddr[4],g_bi_enetaddr[5]);	
	
#endif
	/* start the dma para, but don't start the receive dma */
	writel(ETH_DMA_6_Operation_Mode_EFC | ETH_DMA_6_Operation_Mode_TTC_16 | ETH_DMA_6_Operation_Mode_RSF | ETH_DMA_6_Operation_Mode_DT, ETH_DMA_6_Operation_Mode);
					// | ETH_DMA_6_Operation_Mode_RTC_32 | ETH_DMA_6_Operation_Mode_FUF
	
	netdev_chk();
	DMARXStart();

	g_nInitialized = 1;
	return (0);
error_out:
	return -1;
}

int aml_eth_init(bd_t *bis)
{
	struct eth_device *dev;
	dev = (struct eth_device *) malloc(sizeof(*dev)); 
	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name,"Apollo_EMAC");
	dev->init	= aml_ethernet_init;
	dev->halt 	= aml_eth_halt;
	dev->send	= aml_eth_send;
	dev->recv	= aml_eth_rx;
	return eth_register(dev);	
}

int do_phyread ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  reg_index;
	 unsigned int phyid;
	 
	 if (argc  == 1)
	 return cmd_usage(cmdtp);
	 phyid = simple_strtoul(argv[1], NULL, 16);
	 reg_index = simple_strtoul(argv[2], NULL, 10);
	 aml_printf("[reg_%d]	0x%x\n",reg_index,phy_reg_rd(phyid,reg_index));
	 return 0;
}
int do_phywrite ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  reg_index,value;
	 unsigned int phyid;
	 
	 if (argc  < 3)
	 return cmd_usage(cmdtp);
	 phyid = simple_strtoul(argv[1], NULL, 16);
	 reg_index = simple_strtoul(argv[2], NULL, 10);
	 value=simple_strtoul(argv[3], NULL, 16);

	 phy_reg_wr(phyid,reg_index,value);
	 aml_printf("[reg_%d]	0x%x\n",reg_index,phy_reg_rd(phyid,reg_index));
	 return 0;
}
int do_phydump ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  i;

	 
	 for (i=0;i<32;i++)
	 {
	 	 aml_printf("[reg_%d]	0x%x\n",i,phy_reg_rd(1,i));
	 }
	 return 0;
}
int do_phyreset ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  i;

	 eth_reset(gS);
	 return 0;
}
//loop back test.
int do_autoping ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  value;
	 char  buffer[40];
	 
	 if (argc  < 2)
	 return cmd_usage(cmdtp);
	 value = phy_reg_rd(1, PHY_CR);	

	 phy_reg_wr(1,PHY_CR,value|(1<<14)); //phy loop back
	 while(1)
	 {
	 	if(ctrlc())
		{	
			aml_printf("quit auto ping \n");
			goto out;
	 	}
		sprintf(buffer,"ping %s ",argv[1]);
		run_command(buffer,0);
	 }
out:	 
	 return 0;
}
int do_netchk ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  value;
	 char  buffer[40];
	 
	cmd_netdev_chk();
	 return 0;
}
int do_net_debug_control ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  value;
	 int  enable;
	 
	 if (argc  < 2)
	 return cmd_usage(cmdtp);
	 enable = simple_strtoul(argv[1], NULL, 10);
	 aml_printf("net debug %s\n",(enable?"enable":"disable"));
	debug_enable=(enable?1:0);
	 return 0;
}
int do_net_clk_type ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	 unsigned int  value;
	 int  in_out;
	 
	 if (argc  < 2)
	 return cmd_usage(cmdtp);
	 in_out = simple_strtoul(argv[1], NULL, 10);\
	 aml_printf("net clock %s\n",(in_out?"out":"in"));
	rmii_clk_out=(in_out?RMII_CLK_OUT:RMII_CLK_IN);
	 return 0;
}

U_BOOT_CMD(
	phyw,	4,	1,	do_phywrite,
	"phyw id  reg_index value",
	"index  value "
);

U_BOOT_CMD(
	phyr,	3,	1,	do_phyread,
	"phyr  id reg_index",
	"index "
);
U_BOOT_CMD(
	autoping,2,	1,	do_autoping,
	"autoping ip",
	"ip "
);
U_BOOT_CMD(
	phydump,1,	1,	do_phydump,
	"phydump ",
	"dump "
);
U_BOOT_CMD(
	phyreset,1,	1,	do_phyreset,
	"phydump ",
	"dump "
);
U_BOOT_CMD(
	netchk,1,	1,	do_netchk,
	"netchk ",
	"dump "
);
U_BOOT_CMD(
	net_dbg_ctl,2,	1,	do_net_debug_control,
	"net_dbg_ctl  [1]enable [0]disable",
	"dump "
);
U_BOOT_CMD(
	net_clk_type,2,	1,	do_net_clk_type,
	"net_clk_type  [1]clk_out [0]clk_in",
	"dump "
);
