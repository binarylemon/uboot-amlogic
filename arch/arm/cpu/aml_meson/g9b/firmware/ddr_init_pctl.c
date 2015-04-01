//ddr pctl init code based on ucode from martin.jin, lpddr2 tuned by jiaxing.ye

/*remove warnings*/
static void ddr_udelay_nothing(unsigned long usec){
	return;
}

#if 0
  #define ddr_udelay __udelay
#else
  #define ddr_udelay ddr_udelay_nothing
#endif

static void delay_us(unsigned long us)
{
	int nStart = Rd_cbus(ISA_TIMERE);
	//need to check overflow?
	while((Rd_cbus(ISA_TIMERE) - nStart)< us){};
}

/*remove warnings*/
static void serial_puts_nothing(const char * s){
	return;
}
static void serial_put_hex_nothing(unsigned int data,unsigned bitlen){
	return;
}
static void serial_put_dec_nothing(unsigned int data){
	return;
}

#if 0
    #define hx_serial_puts serial_puts
	#define hx_serial_put_hex serial_put_hex
#else
    #define hx_serial_puts serial_puts_nothing
	#define hx_serial_put_hex serial_put_hex_nothing
#endif

#ifdef CONFIG_DDR_MODE_AUTO_DETECT
	#define pctl_serial_puts serial_puts_nothing
	#define pctl_serial_put_hex serial_put_hex_nothing
	#define pctl_serial_put_dec serial_put_dec_nothing
#else
	#define pctl_serial_puts serial_puts
	#define pctl_serial_put_hex serial_put_hex
	#define pctl_serial_put_dec serial_put_dec
#endif

#define DDR_INIT_WHILE_LOOP_MAX 1000	//all while loop share this number
#define DDR_INIT_WHILE_LOOP(counter) \
		__udelay(1);	\
		counter++; \
		if(counter > DDR_INIT_WHILE_LOOP_MAX){ \
			pctl_serial_puts("While loop "); \
			pctl_serial_put_dec(DDR_INIT_WHILE_LOOP_MAX); \
			pctl_serial_puts("times, error...\n"); \
			counter = 0;	\
			return -2;	\
		}	\
		else	\
			continue;

#define DDR_INIT_ERR_LOOP_MAX 10 //all error loop share this number
#define DDR_INIT_ERROR_LOOP(counter) \
		__udelay(1);	\
		counter++; \
		if(counter > DDR_INIT_ERR_LOOP_MAX){ \
			pctl_serial_puts("Error loop "); \
			pctl_serial_put_dec(DDR_INIT_ERR_LOOP_MAX); \
			pctl_serial_puts("times, error...\n"); \
			counter = 0;	\
			return -1;	\
		}

int init_pctl_ddr3(struct ddr_set * timing_set)
{
	int nRet = -1;
	int loop_whi_count = 0;	//all while loop share this number
	int loop_err_count = 0;

#define  UPCTL_STAT_MASK		(7)
#define  UPCTL_STAT_INIT        (0)
#define  UPCTL_STAT_CONFIG      (1)
#define  UPCTL_STAT_ACCESS      (3)
#define  UPCTL_STAT_LOW_POWER   (5)

#define UPCTL_CMD_INIT         (0) 
#define UPCTL_CMD_CONFIG       (1) 
#define UPCTL_CMD_GO           (2) 
#define UPCTL_CMD_SLEEP        (3) 
#define UPCTL_CMD_WAKEUP       (4) 

//PUB PIR setting
#define PUB_PIR_INIT  		(1<<0)
#define PUB_PIR_ZCAL  		(1<<1)
#define PUB_PIR_CA  		(1<<2)
#define PUB_PIR_PLLINIT 	(1<<4)
#define PUB_PIR_DCAL    	(1<<5)
#define PUB_PIR_PHYRST  	(1<<6)
#define PUB_PIR_DRAMRST 	(1<<7)
#define PUB_PIR_DRAMINIT 	(1<<8)
#define PUB_PIR_WL       	(1<<9)
#define PUB_PIR_QSGATE   	(1<<10)
#define PUB_PIR_WLADJ    	(1<<11)
#define PUB_PIR_RDDSKW   	(1<<12)
#define PUB_PIR_WRDSKW   	(1<<13)
#define PUB_PIR_RDEYE    	(1<<14)
#define PUB_PIR_WREYE    	(1<<15)
#define PUB_PIR_ICPC     	(1<<16)
#define PUB_PIR_PLLBYP   	(1<<17)
#define PUB_PIR_CTLDINIT 	(1<<18)
#define PUB_PIR_RDIMMINIT	(1<<19)
#define PUB_PIR_CLRSR    	(1<<27)
#define PUB_PIR_LOCKBYP  	(1<<28)
#define PUB_PIR_DCALBYP 	(1<<29)
#define PUB_PIR_ZCALBYP 	(1<<30)
#define PUB_PIR_INITBYP  	(1<<31)

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//HX DDR init code
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//PUB PGSR0
#define PUB_PGSR0_IDONE     (1<<0)
#define PUB_PGSR0_PLDONE    (1<<1)
#define PUB_PGSR0_DCDONE    (1<<2)
#define PUB_PGSR0_ZCDONE    (1<<3)
#define PUB_PGSR0_DIDONE    (1<<4)
#define PUB_PGSR0_WLDONE    (1<<5)
#define PUB_PGSR0_QSGDONE   (1<<6)
#define PUB_PGSR0_WLADONE   (1<<7)
#define PUB_PGSR0_RDDONE    (1<<8)
#define PUB_PGSR0_WDDONE    (1<<9)
#define PUB_PGSR0_REDONE    (1<<10)
#define PUB_PGSR0_WEDONE    (1<<11)
#define PUB_PGSR0_CADONE    (1<<12)

#define PUB_PGSR0_ZCERR     (1<<20)
#define PUB_PGSR0_WLERR     (1<<21)
#define PUB_PGSR0_QSGERR    (1<<22)
#define PUB_PGSR0_WLAERR    (1<<23)
#define PUB_PGSR0_RDERR     (1<<24)
#define PUB_PGSR0_WDERR     (1<<25)
#define PUB_PGSR0_REERR     (1<<26)
#define PUB_PGSR0_WEERR     (1<<27)
#define PUB_PGSR0_CAERR     (1<<28)
#define PUB_PGSR0_CAWERR    (1<<29)
#define PUB_PGSR0_VTDONE    (1<<30)
#define PUB_PGSR0_DTERR     (PUB_PGSR0_RDERR|PUB_PGSR0_WDERR|PUB_PGSR0_REERR|PUB_PGSR0_WEERR)

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//HX DDR init code
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define MMC_RESET_CNT_MAX       (8)

	int nTempVal = 0;
	int nRetryCnt = 0; //mmc reset try counter; max to try 8 times 

mmc_init_all:

	hx_serial_puts("\nAml log : mmc_init_all\n");
	nRetryCnt++;

	if(nRetryCnt > MMC_RESET_CNT_MAX )
		return nRet;

	unsigned int ddr_channel_0 = 0;
	//unsigned int ddr_channel_1 = 0;
	ddr_channel_0 = ((((timing_set->t_mmc_ddr_ctrl>>26)&0x3)==0x3)?(((timing_set->t_mmc_ddr_ctrl>>24)&0x1)?(0):(1)):(1));
	//ddr_channel_1 = ((((timing_set->t_mmc_ddr_ctrl>>26)&0x3)==0x3)?(((timing_set->t_mmc_ddr_ctrl>>24)&0x1)?(1):(0)):(1));
	//MMC/DMC reset
	writel(0, P_DMC_SOFT_RST);
	writel(0, P_DMC_SOFT_RST1);
	delay_us(10);
	writel(0xffffffff, P_DMC_SOFT_RST);
	writel(0xffffffff, P_DMC_SOFT_RST1);

	//for power down, init two phy and then close useless one.
	unsigned int ddr_channel_0_power = 0x1;
	//unsigned int ddr_channel_1_power = 0x1;
#if 0
	serial_puts("ddr_channel_0 : ");
	serial_put_hex(ddr_channel_0,32);
	serial_puts("\nddr_channel_1 : ");
	serial_put_hex(ddr_channel_1,32);
	serial_puts("\n");
#endif

/////////////////////////////////////////////////////
pub_init:
	loop_whi_count = 0;

	//enable UPCTL and PUB clock and reset.
	if(ddr_channel_0_power){
		writel(0x0, P_DDR0_SOFT_RESET);
		writel(0xf, P_DDR0_SOFT_RESET);
	}
	if(ddr_channel_0_power){
#if defined(CONFIG_DDR_LOW_POWER_DISABLE)
		writel((timing_set->t_ddr_apd_ctrl & (~0xFF)), P_DDR0_APD_CTRL);
#else
		writel(timing_set->t_ddr_apd_ctrl, P_DDR0_APD_CTRL);
#endif
#if (defined LPDDR2) || (defined LPDDR3)
		writel(0x12b,P_DDR0_CLK_CTRL);
#else
		writel(timing_set->t_ddr_clk_ctrl, P_DDR0_CLK_CTRL);
#endif
	}

	if(ddr_channel_0){
		writel(0x49494949,P_DDR0_PUB_IOVCR0);
		writel(0x49494949,P_DDR0_PUB_IOVCR1);
	}

	if(ddr_channel_0_power){
		//DDR0  timing registers
		writel(timing_set->t_pctl_1us_pck,   P_DDR0_PCTL_TOGCNT1U);	   //1us = nn cycles.
		writel(timing_set->t_pctl_100ns_pck, P_DDR0_PCTL_TOGCNT100N);  //100ns = nn cycles.
		writel(timing_set->t_pctl_init_us,   P_DDR0_PCTL_TINIT);       //200us.
		writel(timing_set->t_pctl_rsth_us,   P_DDR0_PCTL_TRSTH);       // 0 for ddr2;  2 for simulation; 500 for ddr3.
		writel(timing_set->t_pctl_rstl_us,   P_DDR0_PCTL_TRSTL);       // 0 for ddr2;  2 for simulation; 500 for ddr3.

		//configure the PCTL for DDR0
#if defined(CONFIG_DDR_LOW_POWER_DISABLE)
		writel((timing_set->t_pctl_mcfg & (~(0xFF<<8))),  P_DDR0_PCTL_MCFG );
#else
		writel(timing_set->t_pctl_mcfg,  P_DDR0_PCTL_MCFG );
#endif
		writel(timing_set->t_pctl_mcfg1, P_DDR0_PCTL_MCFG1);  //enable hardware c_active_in;
		//configure DDR PHY PUBL registers.
		writel(timing_set->t_pub_dcr, P_DDR0_PUB_DCR);
		//program PUB MRx registers.
		writel(timing_set->t_pub_mr[0], P_DDR0_PUB_MR0);
		writel(timing_set->t_pub_mr[1], P_DDR0_PUB_MR1);
		writel(timing_set->t_pub_mr[2], P_DDR0_PUB_MR2);
		writel(timing_set->t_pub_mr[3], P_DDR0_PUB_MR3);

		//program DDR SDRAM timing parameter.
		writel( timing_set->t_pub_dtpr[0], P_DDR0_PUB_DTPR0);
		writel( timing_set->t_pub_dtpr[1], P_DDR0_PUB_DTPR1);
		writel( timing_set->t_pub_dtpr[2], P_DDR0_PUB_DTPR2);

		//configure auto refresh when data training.
		writel( (readl(P_DDR0_PUB_PGCR2) & 0xfffc0000) | 0xc00 , P_DDR0_PUB_PGCR2 );
		//writel(((readl(P_DDR0_PUB_DTCR) & 0xf0ffffff) | (1 << 24)), P_DDR0_PUB_DTCR);
		/*mpr mode: set DTCR[bit6]=1, DCR[bit8]=1. or set to 0 to disable mpr*/
		//writel(((readl(P_DDR0_PUB_DTCR) & 0xffffffbf) | (1 << 6)), P_DDR0_PUB_DTCR);
		writel( ((readl(P_DDR0_PUB_DTCR) & 0x00ffffbf) | (1 << 28 ) | (1 << 24) | (1 << 6) |(1<<23) ), P_DDR0_PUB_DTCR);

		//for training gate extended gate
		writel(0x8, P_DDR0_PUB_DX0GCR3); //for pdr mode bug //for pdr mode bug this will cause some chip bit deskew  jiaxing add
		writel(0x8, P_DDR0_PUB_DX1GCR3);
		writel(0x8, P_DDR0_PUB_DX2GCR3);
		writel(0x8, P_DDR0_PUB_DX3GCR3);

#if ((defined LPDDR2) || (defined LPDDR3))
		writel((readl(P_DDR0_PUB_DSGCR))|(0x3<<06), P_DDR0_PUB_DSGCR); //eanble gate extension  dqs gate can help to bit deskew also
		//writel((((readl(P_DDR0_PUB_ZQCR))&0xff01fffc)|(1<<24)), P_DDR0_PUB_ZQCR);
#else
	//	writel((readl(P_DDR0_PUB_DSGCR))|(0x1<<06), P_DDR0_PUB_DSGCR); //eanble gate extension  dqs gate can help to bit deskew also
		writel((readl(P_DDR0_PUB_DSGCR))|(0x2<<06), P_DDR0_PUB_DSGCR); //eanble gate extension  dqs gate can help to bit deskew also
		//writel(((readl(P_DDR0_PUB_ZQCR))&0xff01fffc), P_DDR0_PUB_ZQCR); //for vt bug disable IODLMT
#endif
		writel(((readl(P_DDR0_PUB_ZQCR))&0xff01fffc), P_DDR0_PUB_ZQCR); //for vt bug disable IODLMT

#ifdef CONFIG_CMD_DDR_TEST
		if(zqcr)
			writel(zqcr, P_DDR0_PUB_ZQ0PR);
		else
#endif
		writel((timing_set->t_pub_zq0pr)|((readl(P_DDR0_PUB_ZQ0PR))&0xffffff00), P_DDR0_PUB_ZQ0PR);
if((zqcr>>19)==1)  //just for debug
{
writel((zqcr&0x7ffff), P_DDR0_PUB_ZQ0PR);
}

		writel((1<<1)|((readl(P_DDR0_PUB_ZQCR))&0xfffffffe), P_DDR0_PUB_ZQCR);
		writel(timing_set->t_pub_dxccr, P_DDR0_PUB_DXCCR);
		writel(readl(P_DDR0_PUB_ACBDLR0) | (timing_set->t_pub_acbdlr0), P_DDR0_PUB_ACBDLR0);  //place before write level

		//initialization PHY.
		writel(timing_set->t_pub_ptr[0]	, P_DDR0_PUB_PTR0);
		writel(timing_set->t_pub_ptr[1]	, P_DDR0_PUB_PTR1);

		//configure DDR0__rst pin.
		writel(readl(P_DDR0_PUB_ACIOCR0) & 0xdfffffff, P_DDR0_PUB_ACIOCR0);

		//configure for phy update request and ack.
		writel(((readl(P_DDR0_PUB_DSGCR) & 0xffffffef)  | (1 << 6)), P_DDR0_PUB_DSGCR );	 //other bits.

		writel(timing_set->t_pub_ptr[3]	, P_DDR0_PUB_PTR3);
		writel(timing_set->t_pub_ptr[4]	, P_DDR0_PUB_PTR4);
	}

	if(ddr_channel_0_power){
		while(!(readl(P_DDR0_PCTL_DFISTSTAT0) & 1)) {
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - DFI status check done\n");
		writel(1, P_DDR0_PCTL_POWCTL);
		while(!(readl(P_DDR0_PCTL_POWSTAT) & 1) ) {
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
	}

	if(ddr_channel_0_power){
		//ddr0 pctl
		writel(timing_set->t_pctl_trfc,		P_DDR0_PCTL_TRFC);
		writel(timing_set->t_pctl_trefi,	P_DDR0_PCTL_TREFI);
		writel(timing_set->t_pctl_trefi|(1<<31), P_DDR0_PCTL_TREFI);
		writel(timing_set->t_pctl_trefi_mem_ddr3, P_DDR0_PCTL_TREFI_MEM_DDR3);
		writel(timing_set->t_pctl_tmrd,	P_DDR0_PCTL_TMRD);
		writel(timing_set->t_pctl_trp,	P_DDR0_PCTL_TRP);
		writel(timing_set->t_pctl_tal,	P_DDR0_PCTL_TAL);
		writel(timing_set->t_pctl_tcwl,	P_DDR0_PCTL_TCWL);
		writel(timing_set->t_pctl_tcl,	P_DDR0_PCTL_TCL);
		writel(timing_set->t_pctl_tras,	P_DDR0_PCTL_TRAS);
		writel(timing_set->t_pctl_trc,	P_DDR0_PCTL_TRC);
		writel(timing_set->t_pctl_trcd,	P_DDR0_PCTL_TRCD);
		writel(timing_set->t_pctl_trrd,	P_DDR0_PCTL_TRRD);
		writel(timing_set->t_pctl_trtp,	P_DDR0_PCTL_TRTP);
		writel(timing_set->t_pctl_twr,	P_DDR0_PCTL_TWR);
		writel(timing_set->t_pctl_twtr,	P_DDR0_PCTL_TWTR);
		writel(timing_set->t_pctl_texsr,	P_DDR0_PCTL_TEXSR);
		writel(timing_set->t_pctl_txp,	P_DDR0_PCTL_TXP);
		writel(timing_set->t_pctl_tdqs,	P_DDR0_PCTL_TDQS);
		writel(timing_set->t_pctl_trtw,	P_DDR0_PCTL_TRTW);
		writel(timing_set->t_pctl_tcksre,	P_DDR0_PCTL_TCKSRE);
		writel(timing_set->t_pctl_tcksrx,	P_DDR0_PCTL_TCKSRX);
		writel(timing_set->t_pctl_tmod,	P_DDR0_PCTL_TMOD);
		writel(timing_set->t_pctl_tcke,	P_DDR0_PCTL_TCKE);
		writel(timing_set->t_pctl_tzqcs,	P_DDR0_PCTL_TZQCS);
		writel(timing_set->t_pctl_tzqcl,	P_DDR0_PCTL_TZQCL);
		writel(timing_set->t_pctl_txpdll,	P_DDR0_PCTL_TXPDLL);
		writel(timing_set->t_pctl_tzqcsi,	P_DDR0_PCTL_TZQCSI);
		writel(timing_set->t_pctl_scfg,		P_DDR0_PCTL_SCFG);
#if ((defined LPDDR2) || (defined LPDDR3))
		writel(timing_set->t_pctl_tckesr,	P_DDR0_PCTL_TCKESR);
		writel(timing_set->t_pctl_tdpd,		P_DDR0_PCTL_TDPD);
#endif
	}

	if(ddr_channel_0_power){
		writel(UPCTL_CMD_CONFIG, P_DDR0_PCTL_SCTL); //DDR 0
		while(!(readl(P_DDR0_PCTL_STAT) & 1)) {
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
	}

	if(ddr_channel_0_power){ //bus_width setting
		if(cfg_ddr_mode == CFG_DDR_BUS_WIDTH_32BIT){
			writel((0xf0 << 1),P_DDR0_PCTL_PPCFG);
		}
		else if(cfg_ddr_mode == CFG_DDR_BUS_WIDTH_16BIT_LANE01)
		{
			writel((0x1fc | 1), P_DDR0_PCTL_PPCFG);
			writel(0, P_DDR0_PUB_DX2GCR0);
			writel(0, P_DDR0_PUB_DX3GCR0);
			writel(0, P_DDR0_PUB_DX4GCR0);
			writel(0, P_DDR0_PUB_DX5GCR0);
		}
	}

	if(ddr_channel_0_power){
		//DDR 0 DFI
		writel(4,P_DDR0_PCTL_DFISTCFG0);
		writel(1,P_DDR0_PCTL_DFISTCFG1);
		writel(2,P_DDR0_PCTL_DFITCTRLDELAY);
		writel(1,P_DDR0_PCTL_DFITPHYWRDATA);

		//DDR 0 DFI
#ifdef LPDDR2
		writel(1,P_DDR0_PCTL_DFITPHYWRLAT);
		writel(4,P_DDR0_PCTL_DFITRDDATAEN); //use 1/ 2 /5 trainging will abort 3 /4 is better
#else
		nTempVal = timing_set->t_pctl_tcwl+timing_set->t_pctl_tal;
		nTempVal = (nTempVal - ((nTempVal%2) ? 3:4))/2;
		writel(nTempVal,P_DDR0_PCTL_DFITPHYWRLAT);
		nTempVal = timing_set->t_pctl_tcl+timing_set->t_pctl_tal;
		nTempVal = (nTempVal - ((nTempVal%2) ? 3:4))/2;
		writel(nTempVal,P_DDR0_PCTL_DFITRDDATAEN);
#endif
		//DDR 0 DFI
		writel(13,P_DDR0_PCTL_DFITPHYRDLAT);
		writel(1,P_DDR0_PCTL_DFITDRAMCLKDIS);
		writel(1,P_DDR0_PCTL_DFITDRAMCLKEN);
		writel(0x4000,P_DDR0_PCTL_DFITCTRLUPDMIN);
		writel(( 1 | (3 << 4) | (1 << 8) | (3 << 12) | (7 <<16) | (1 <<24) | ( 3 << 28)),
			P_DDR0_PCTL_DFILPCFG0);
		writel(0x200,P_DDR0_PCTL_DFITPHYUPDTYPE1);
#ifdef LPDDR2
		writel(0,P_DDR0_PCTL_DFIODTCFG);
#else
		writel(8,P_DDR0_PCTL_DFIODTCFG);
#endif
		writel(( 0x0 | (0x6 << 16)),P_DDR0_PCTL_DFIODTCFG1);

		//CONFIG_DDR0_DTAR_ADDR//0x8/0x10/0x18
		writel((0x0  + timing_set->t_pub0_dtar), P_DDR0_PUB_DTAR0);
		writel((0x08 + timing_set->t_pub0_dtar), P_DDR0_PUB_DTAR1);
		writel((0x10 + timing_set->t_pub0_dtar), P_DDR0_PUB_DTAR2);
		writel((0x18 + timing_set->t_pub0_dtar), P_DDR0_PUB_DTAR3);
	}

	if(ddr_channel_0_power){
		writel(1,P_DDR0_PCTL_CMDTSTATEN);
		while(!(readl(P_DDR0_PCTL_CMDTSTAT) & 0x1)) {
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
	}

	//===============================================
	//HX PUB INIT
	hx_serial_puts("Aml log : HX DDR PUB training begin:\n");

	//===============================================
	//PLL,DCAL,PHY RST,ZCAL
	if(ddr_channel_0_power){
		nTempVal =	PUB_PIR_ZCAL | PUB_PIR_PLLINIT | PUB_PIR_DCAL | PUB_PIR_PHYRST;
		writel(nTempVal, P_DDR0_PUB_PIR);
		ddr_udelay(1);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);
	}
	if(ddr_channel_0){
		while((readl(P_DDR0_PUB_PGSR0) != 0x8000000f) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC000000f))
		{
			ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) & PUB_PGSR0_ZCERR)
			{
				pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_ZCERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
				DDR_INIT_ERROR_LOOP(loop_err_count);
				goto pub_init;
			}
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - PLL,DCAL,RST,ZCAL done\n");
	}

	#ifdef CONFIG_DDR_FINE_TUNE_ADD_N303
	if (ddr_channel_0_power) {
writel(0x0, P_DDR0_PUB_ACBDLR0); //ck
writel((0x55<<16)|((readl(P_DDR0_PUB_PGCR3))&(~(0xff<<16))), P_DDR0_PUB_PGCR3);
writel(0x1a, P_DDR0_PUB_ACLCDLR); //720 1f  792 13  756 16

	}
	#else
		if (ddr_channel_0_power) {
//writel(0x1f, P_DDR0_PUB_ACBDLR0); //ck
writel((0xaa<<16)|((readl(P_DDR0_PUB_PGCR3))&(~(0xff<<16))), P_DDR0_PUB_PGCR3);
//writel(0x16, P_DDR0_PUB_ACLCDLR);

	}
#endif

	//===============================================
	//DRAM INIT
	if(ddr_channel_0_power){
		nTempVal =	PUB_PIR_DRAMRST | PUB_PIR_DRAMINIT ;
		writel(nTempVal, P_DDR0_PUB_PIR);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);
	}
	if(ddr_channel_0){
		while((readl(P_DDR0_PUB_PGSR0) != 0x8000001f) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC000001f))
		{
			//ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) & 1)
				break;
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - DRAM INIT done\n");
	}

#if !defined(CONFIG_VLSI_EMULATOR)
#if (!(defined LPDDR2))&& (!(defined LPDDR3))
	//===============================================	
	//WL init
	if(ddr_channel_0){
		nTempVal =	PUB_PIR_WL ;
		writel(nTempVal, P_DDR0_PUB_PIR);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);

		while((readl(P_DDR0_PUB_PGSR0) != 0x8000003f) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC000003f))
		{
			//ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) & PUB_PGSR0_WLERR)
			{
				pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_WLERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
				DDR_INIT_ERROR_LOOP(loop_err_count);
				goto pub_init;
			}
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - WL done\n");
	}
#endif //LPDDR2, LPDDR3
#endif //CONFIG_VLSI_EMULATOR

	//===============================================	
	//DQS Gate training
#if (!(defined LPDDR2))&& (!(defined LPDDR3))
	if(ddr_channel_0){
		nTempVal =	PUB_PIR_QSGATE ;
		writel(nTempVal, P_DDR0_PUB_PIR);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);

#if defined(CONFIG_VLSI_EMULATOR)
		while((readl(P_DDR0_PUB_PGSR0) != 0x8000005f) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC000005f))
#else
		while((readl(P_DDR0_PUB_PGSR0) != 0x8000007f) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC000007f))
#endif //CONFIG_VLSI_EMULATOR
		{
			//ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) & PUB_PGSR0_QSGERR)
			{
				pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_QSGERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
				DDR_INIT_ERROR_LOOP(loop_err_count);
				goto pub_init;
			}
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - DQS done\n");
	}
#endif //LPDDR2, LPDDR3

#if !defined(CONFIG_VLSI_EMULATOR)
#if (!(defined LPDDR2))&& (!(defined LPDDR3))
	//===============================================	
	//Write leveling ADJ
	if(ddr_channel_0){
		nTempVal =	PUB_PIR_WLADJ ;
		writel(nTempVal, P_DDR0_PUB_PIR);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);

		while((readl(P_DDR0_PUB_PGSR0) != 0x800000ff) &&
			(readl(P_DDR0_PUB_PGSR0) != 0xC00000ff))
		{
			//ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) & PUB_PGSR0_WLAERR)
			{
				pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_WLAERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
				DDR_INIT_ERROR_LOOP(loop_err_count);
				goto pub_init;
			}
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - WLADJ done\n");
	}
#endif //LPDDR2, LPDDR3

#if (!(defined LPDDR2))&& (!(defined LPDDR3))
	//===============================================	
	//Data bit deskew & data eye training
	if(ddr_channel_0){
		nTempVal =	PUB_PIR_RDDSKW | PUB_PIR_WRDSKW | PUB_PIR_RDEYE | PUB_PIR_WREYE;
		writel(nTempVal, P_DDR0_PUB_PIR);
		writel(nTempVal|PUB_PIR_INIT, P_DDR0_PUB_PIR);

		while((readl(P_DDR0_PUB_PGSR0) != 0x80000fff)&&
			(readl(P_DDR0_PUB_PGSR0) != 0xC0000fff))
		{
			ddr_udelay(1);
			if(readl(P_DDR0_PUB_PGSR0) & PUB_PGSR0_DTERR)
			{
			    pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_DTERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
			    goto pub_init;
			}
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - BIT deskew & data eye done\n");
	}
#endif //LPDDR2, LPDDR3
#endif //CONFIG_VLSI_EMULATOR

#ifdef LPDDR2
	if(ddr_channel_0){
		writel(0xf173, P_DDR0_PUB_PIR);
		while((readl(P_DDR0_PUB_PGSR0) != 0x80000f1f) && \
			(readl(P_DDR0_PUB_PGSR0) != 0xC0000f1f) && \
			(readl(P_DDR0_PUB_PGSR0) != 0x80000f5f) && \
			(readl(P_DDR0_PUB_PGSR0) != 0xC0000f5f)){
			//ddr_udelay(10);
			if(readl(P_DDR0_PUB_PGSR0) &( PUB_PGSR0_DTERR|PUB_PGSR0_ZCERR)){
				pctl_serial_puts("Aml log : DDR0 - PUB_PGSR0_ERR with [");
				pctl_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
				pctl_serial_puts("] retry...\n");
				goto pub_init;
			}
		}
		hx_serial_puts("Aml log : DDR0 - training done PGSR0 = 0x");
		hx_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
		hx_serial_puts("\n");
	}
#endif

	if(ddr_channel_0_power){
		writel(UPCTL_CMD_GO, P_DDR0_PCTL_SCTL);
		while ((readl(P_DDR0_PCTL_STAT) & UPCTL_STAT_MASK ) != UPCTL_STAT_ACCESS ) {
			DDR_INIT_WHILE_LOOP(loop_whi_count);
		}
		hx_serial_puts("Aml log : DDR0 - PCTL enter GO state\n");
	}
	if(ddr_channel_0){
		nTempVal = readl(P_DDR0_PUB_PGSR0);
#ifdef CONFIG_NO_DDR_PUB_VT_CHECK
		if ((( (nTempVal >> 20) & 0xfff ) != 0xC00 ) &&
			(( (nTempVal >> 20) & 0xfff ) != 0x800 ))
#else
		if (( (nTempVal >> 20) & 0xfff ) != 0xC00 )
#endif
	    {
			pctl_serial_puts("Aml log : DDR0 - PUB init fail with PGSR0 : 0x");
			pctl_serial_put_hex(nTempVal,32);
			pctl_serial_puts("Aml log : Try again with MMC reset...\n\n");
			DDR_INIT_ERROR_LOOP(loop_err_count);
			goto mmc_init_all;
	    }
	    else
	    {
	        hx_serial_puts("Aml log : DDR0 - init pass with  PGSR0 : 0x");
			hx_serial_put_hex(readl(P_DDR0_PUB_PGSR0),32);
			hx_serial_puts("\n");
			//serial_puts("\n  PGSR1 : 0x");
			//serial_put_hex(readl(P_DDR0_PUB_PGSR1),32);
			//serial_puts("\n");
	    }

#if defined(CONFIG_PUB_WLWDRDRGLVTWDRDBVT_DISABLE)
		writel((readl(P_DDR0_PUB_PGCR0) & (~0x3F)),P_DDR0_PUB_PGCR0);
#endif
	}

	nRet = 0;
	if(ddr_channel_0_power){
#if defined(CONFIG_GATEACDDRCLK_DISABLE)
		writel((0x7d<<9)|(readl(P_DDR0_PUB_PGCR3)), P_DDR0_PUB_PGCR3);
#else
		writel((0x7f<<9)|(readl(P_DDR0_PUB_PGCR3)), P_DDR0_PUB_PGCR3);
#endif
		writel(readl(P_DDR0_PUB_ZQCR) | (0x4), P_DDR0_PUB_ZQCR);
		__udelay(1);
		//writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);
		//__udelay(1);
	}
#ifdef CONFIG_DDR_BDL_DEBUG

//writel(0x1f, P_DDR0_PUB_ACBDLR0); //ck
/*
writel(0x0, P_DDR0_PUB_ACBDLR1);//ras cas we
writel(0x0, P_DDR0_PUB_ACBDLR2);//ba0 ba1 ba2
writel(0x0, P_DDR0_PUB_ACBDLR3);//cs0
writel(0x0, P_DDR0_PUB_ACBDLR4);//odt
writel(0x04, P_DDR0_PUB_ACBDLR5);//cke

writel(0x04040404, P_DDR0_PUB_ACBDLR6);//a0 a1 a2 a3
writel(0x04040404, P_DDR0_PUB_ACBDLR7);//a4 a5 a6 a7
writel(0x08080808, P_DDR0_PUB_ACBDLR8);//a8 a9 a10 a11
writel(0x08080808, P_DDR0_PUB_ACBDLR9);//a12 a13 a14 a15
*/
///*
if((timing_set->t_pctl_mcfg) & (1<<3)) 
{
writel(0x1a1a1a1a, P_DDR0_PUB_ACBDLR1);//ras cas we
writel(0x1f1f1f1f, P_DDR0_PUB_ACBDLR2);//ba0 ba1 ba2
writel(0x0, P_DDR0_PUB_ACBDLR3);//cs0
writel(0, P_DDR0_PUB_ACBDLR4);//odt
writel(0, P_DDR0_PUB_ACBDLR5);//cke

writel(0x04040404, P_DDR0_PUB_ACBDLR6);//a0 a1 a2 a3
writel(0x08080808, P_DDR0_PUB_ACBDLR7);//a4 a5 a6 a7
writel(0x12121212, P_DDR0_PUB_ACBDLR8);//a8 a9 a10 a11
writel(0x16161616, P_DDR0_PUB_ACBDLR9);//a12 a13 a14 a15
}
//*/
/*
writel(0x00000000, P_DDR0_PUB_DX0BDLR0);//d0 d1 d2 d3
writel(0x00000000, P_DDR0_PUB_DX0BDLR1);//d4 d5 d6 d7
writel(0x00000000, P_DDR0_PUB_DX0BDLR2);//dm ds oe

writel(0x03030303, P_DDR0_PUB_DX1BDLR0);//d0 d1 d2 d3
writel(0x03030303, P_DDR0_PUB_DX1BDLR1);//d4 d5 d6 d7
writel(0x03030303, P_DDR0_PUB_DX1BDLR2);//dm ds oe

writel(0x06060606, P_DDR0_PUB_DX2BDLR0);//d0 d1 d2 d3
writel(0x06060606, P_DDR0_PUB_DX2BDLR1);//d4 d5 d6 d7
writel(0x06060606, P_DDR0_PUB_DX2BDLR2);//dm ds oe

writel(0x09090909, P_DDR0_PUB_DX3BDLR0);//d0 d1 d2 d3
writel(0x09090909, P_DDR0_PUB_DX3BDLR1);//d4 d5 d6 d7
writel(0x09090909, P_DDR0_PUB_DX3BDLR2);//dm ds oe
*/

int temp_bdl=((zqcr&0xff00)>>8);
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
if((zqcr>>19)==1)  //just for debug
{

///*

writel(temp_bdl, P_DDR0_PUB_DX0BDLR3);//d0 d1 d2 d3 read
writel(temp_bdl, P_DDR0_PUB_DX0BDLR4);//d4 d5 d6 d7


writel(temp_bdl, P_DDR0_PUB_DX1BDLR3);//d0 d1 d2 d3
writel(temp_bdl, P_DDR0_PUB_DX1BDLR4);//d4 d5 d6 d7


writel(temp_bdl, P_DDR0_PUB_DX2BDLR3);//d0 d1 d2 d3
writel(temp_bdl, P_DDR0_PUB_DX2BDLR4);//d4 d5 d6 d7


writel(temp_bdl, P_DDR0_PUB_DX3BDLR3);//d0 d1 d2 d3
writel(temp_bdl, P_DDR0_PUB_DX3BDLR4);//d4 d5 d6 d7
//*/
temp_bdl=0;
writel(temp_bdl, P_DDR0_PUB_DX0BDLR5);//dm dsr dsrn
writel(temp_bdl, P_DDR0_PUB_DX1BDLR5);//dm dsr dsrn
writel(temp_bdl, P_DDR0_PUB_DX2BDLR5);//dm dsr dsrn
writel(temp_bdl, P_DDR0_PUB_DX3BDLR5);//dm dsr dsrn
//*/
/*
temp_bdl=(readl(P_DDR0_PUB_DX0LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX0LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX1LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX1LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX2LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX2LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX3LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX3LCDLR1);
*/
}
else
{
/*
writel(timing_set->t_pub_ddr0_dx0bdlr3, P_DDR0_PUB_DX0BDLR3);//d0 d1 d2 d3 read
writel(timing_set->t_pub_ddr0_dx0bdlr4, P_DDR0_PUB_DX0BDLR4);//d4 d5 d6 d7
writel(timing_set->t_pub_ddr0_dx0bdlr5, P_DDR0_PUB_DX0BDLR5);//dm dsr dsrn

writel(timing_set->t_pub_ddr0_dx1bdlr3, P_DDR0_PUB_DX1BDLR3);//d0 d1 d2 d3
writel(timing_set->t_pub_ddr0_dx1bdlr4, P_DDR0_PUB_DX1BDLR4);//d4 d5 d6 d7
writel(timing_set->t_pub_ddr0_dx1bdlr5, P_DDR0_PUB_DX1BDLR5);//dm dsr dsrn

writel(timing_set->t_pub_ddr0_dx2bdlr3, P_DDR0_PUB_DX2BDLR3);//d0 d1 d2 d3
writel(timing_set->t_pub_ddr0_dx2bdlr4, P_DDR0_PUB_DX2BDLR4);//d4 d5 d6 d7
writel(timing_set->t_pub_ddr0_dx2bdlr5, P_DDR0_PUB_DX2BDLR5);//dm dsr dsrn

writel(timing_set->t_pub_ddr0_dx3bdlr3, P_DDR0_PUB_DX3BDLR3);//d0 d1 d2 d3
writel(timing_set->t_pub_ddr0_dx3bdlr4, P_DDR0_PUB_DX3BDLR4);//d4 d5 d6 d7
writel(timing_set->t_pub_ddr0_dx3bdlr5, P_DDR0_PUB_DX3BDLR5);//dm dsr dsrn
*/
/*
temp_bdl=(readl(P_DDR0_PUB_DX0LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX0LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX1LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX1LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX2LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX2LCDLR1);

temp_bdl=(readl(P_DDR0_PUB_DX3LCDLR1) &0xff );
temp_bdl=((temp_bdl<<24)|(temp_bdl<<16)|(temp_bdl<<8)|(temp_bdl<<0));
writel(temp_bdl,P_DDR0_PUB_DX3LCDLR1);
*/
#if defined( CONFIG_DDR_DQS_TUNE_T826_SOC_SZ_N302)
temp_bdl=(readl(P_DDR0_PUB_DX0LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x050500; //0x040404
//temp_bdl=temp_bdl+0x040400; //0x040404
writel(temp_bdl,P_DDR0_PUB_DX0LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX1LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040401;
writel(temp_bdl,P_DDR0_PUB_DX1LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX2LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x050500;
writel(temp_bdl,P_DDR0_PUB_DX2LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX3LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040402;
writel(temp_bdl,P_DDR0_PUB_DX3LCDLR1);

#elif defined( CONFIG_DDR_DQS_TUNE_T828_SOC_SZ_N303)

temp_bdl=(readl(P_DDR0_PUB_DX0LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040405; //0x040404
//temp_bdl=temp_bdl+0x040400; //0x040404
writel(temp_bdl,P_DDR0_PUB_DX0LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX1LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040405;
writel(temp_bdl,P_DDR0_PUB_DX1LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX2LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040405;
writel(temp_bdl,P_DDR0_PUB_DX2LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX3LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040405;
writel(temp_bdl,P_DDR0_PUB_DX3LCDLR1);
#else

temp_bdl=(readl(P_DDR0_PUB_DX0LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040402; //0x040404
//temp_bdl=temp_bdl+0x040400; //0x040404
writel(temp_bdl,P_DDR0_PUB_DX0LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX1LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040402;
writel(temp_bdl,P_DDR0_PUB_DX1LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX2LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040402;
writel(temp_bdl,P_DDR0_PUB_DX2LCDLR1);
temp_bdl=(readl(P_DDR0_PUB_DX3LCDLR1) &0xffffff );
temp_bdl=temp_bdl-0x040402;
writel(temp_bdl,P_DDR0_PUB_DX3LCDLR1);

#endif





}

#endif

#ifdef CONFIG_DDR_DQS_TUNE_T828_SOC_SZ_N303
//writel(4|(0xf7<<5)|(3<<19), P_DDR0_PUB_DXCCR);// .t_pub_dxccr = 4|(0xf7<<5)|(3<<19), 2layer board
//writel(4|(0x01<<5)|(3<<19), P_DDR0_PUB_DXCCR);// .t_pub_dxccr = 4|(0xc4<<5)|(3<<19), 4layer board
//jiaxing debug use pcb 2layer should add resister improve dqs read noise.
#endif
//writel((readl(P_DDR0_PUB_DSGCR))&(~(0x3<<06)), P_DDR0_PUB_DSGCR); //eanble gate extension  dqs gate can help to bit deskew also
//writel((readl(P_DDR1_PUB_DSGCR))&(~(0x3<<06)), P_DDR1_PUB_DSGCR);
//	writel((readl(P_DDR0_PUB_DXCCR))&(~(0xFF<<05)), P_DDR0_PUB_DXCCR);
//	writel((readl(P_DDR1_PUB_DXCCR))&(~(0xFF<<05)), P_DDR1_PUB_DXCCR);

	if(0 == ddr_channel_0){ //ddr channel 0 disabled, power down it.
		writel(UPCTL_CMD_SLEEP, P_DDR0_PCTL_SCTL);
		while ((readl(P_DDR0_PCTL_STAT) & UPCTL_STAT_MASK ) != UPCTL_STAT_LOW_POWER ) {}
		writel(1<<29, P_DDR0_PUB_PLLCR);
		__udelay(1);
		writel((1<<2)|(1<<6)|(1<<4), P_DDR0_CLK_CTRL);
	}
	else{
		writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);
		__udelay(1);
	}

	hx_serial_puts("Aml log : DDR PCTL init done!\n");

	return nRet;
}

