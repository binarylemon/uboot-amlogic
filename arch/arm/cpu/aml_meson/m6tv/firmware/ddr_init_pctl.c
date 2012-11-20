
unsigned get_mrs0(struct ddr_set * timing_reg)
{    
    //only calculate CL (6:4) and WR (11:9)
    //all others load from timming.c          
    unsigned short nMask = ~((7<<4)|(7<<9));
    unsigned short nMR0 = timing_reg->mrs[0] & nMask;    
    
    //CL
    nMR0 |=  ((timing_reg->cl-4)&0x7)<<4;
    
    //WR
    if(timing_reg->t_wr <= 8)
        nMR0|=((timing_reg->t_wr-4)&0x7)<<9;
    else
    	nMR0|=((timing_reg->t_wr>>1)&7)<<9;
    	
    return nMR0;      
}

unsigned get_mrs1(struct ddr_set * timing_reg)
{   
    //only calculate AL (4:3)
    //all others load from timming.c          
    unsigned short nMask = ~((3<<3));
    unsigned short nMR1 = timing_reg->mrs[1] & nMask;
    
    
    //AL
    if(timing_reg->t_al)
    {
        nMR1|=((timing_reg->cl-timing_reg->t_al)&3)<<3;
    }
        	
    return nMR1;
}
unsigned get_mrs2(struct ddr_set * timing_reg)
{   
    //only calculate CWL (5:3)
    //all others load from timming.c          
    unsigned short nMask = ~((7<<3));  
    unsigned short nMR2 = timing_reg->mrs[2] & nMask;
    
    
    //CWL
    nMR2 |= (((timing_reg->t_cwl-5)&7)<<3);// | (1 << 6 );
            	
    return nMR2;
}

#ifdef CONFIG_CMD_DDR_TEST
static unsigned short zqcr = 0;
#endif
int init_pctl_ddr3(struct ddr_set * timing_reg)
{	
	int nTempVal = 0;
	int i;
//	int ret = 0;
	//asm volatile ("wfi"); //debug 11.20

	//UPCTL memory timing registers
	writel(timing_reg->t_1us_pck, P_UPCTL_TOGCNT1U_ADDR);	 //1us = nn cycles.

	writel(timing_reg->t_100ns_pck, P_UPCTL_TOGCNT100N_ADDR);//100ns = nn cycles.

	writel(timing_reg->t_init_us, P_UPCTL_TINIT_ADDR);  //200us.

	writel(timing_reg->t_rsth_us, P_UPCTL_TRSTH_ADDR);  // 0 for ddr2;  2 for simulation; 500 for ddr3.
	writel(timing_reg->t_rstl_us, P_UPCTL_TRSTL_ADDR);
	
    for(i=4;(i)*timing_reg->t_rrd<timing_reg->t_faw&&i<6;i++);

	writel(((i-4) <<18)|  // 0:tFAW=4*tRRD 1:tFAW=5*tRRD 2:tFAW=6*tRRD
		(timing_reg->mcfg & (~(3<<18)))
		, P_UPCTL_MCFG_ADDR);
    if((i)*timing_reg->t_rrd > timing_reg->t_faw)
    {
        nTempVal = ((i)*timing_reg->t_rrd - timing_reg->t_faw) > 7 ? 7 : ((i)*timing_reg->t_rrd - timing_reg->t_faw);
    }
	
	writel( 0x80000000 | (nTempVal << 8), P_UPCTL_MCFG1_ADDR );  //enable hardware c_active_in;
	  
	  
    writel(0x8,P_UPCTL_DFIODTCFG_ADDR);
	  
	//configure DDR PHY PUBL registers.
	//  2:0   011: DDR3 mode.	 100:	LPDDR2 mode.
	//  3:    8 bank. 
	writel(0x3 | (1 << 3)| (1 << 7), P_PUB_DCR_ADDR);
	//writel(0x01842e04, P_PUB_PGCR0_ADDR); //PUB_PGCR_ADDR: c8001008

	// program PUB MRx registers.	
	writel(get_mrs0(timing_reg), P_PUB_MR0_ADDR);
	writel(get_mrs1(timing_reg), P_PUB_MR1_ADDR);
	writel(get_mrs2(timing_reg), P_PUB_MR2_ADDR);
	writel(0x0, P_PUB_MR3_ADDR);	

	//program DDR SDRAM timing parameter.
	writel( (timing_reg->t_rtp << 0)  |		//tRTP
		(timing_reg->t_wtr << 4)  |		//tWTR
		(timing_reg->t_rp << 8)   |		//tRP
		(timing_reg->t_rcd << 12) |		//tRCD
		(timing_reg->t_ras <<16)  |		//tRAS
		(timing_reg->t_rrd <<22 ) |		//tRRD
		(timing_reg->t_rc <<26) 		//tRC
		, P_PUB_DTPR0_ADDR);

	writel(( (0 << 0) | 			//tMRD
        (timing_reg->t_mod << 2 ) | //tMOD.
        (timing_reg->t_faw << 5 ) | //tFAW.
		(timing_reg->t_rfc << 11) | //tRFC
		(40 << 20 )               | //tWLMRD
		(6 << 26)                 | //tWLO
        (0 << 30 ) )                //tAOND  DDR2 only.
		, P_PUB_DTPR1_ADDR);


	writel(( 512 					 | //tXS
		( timing_reg->t_xpdll << 10) | //tXP
		( timing_reg->t_cke << 15) 	 | //tCKE
		( 512 << 19)                 | //tDLLK
        ( 1 << 29  )  				 | //tRTODT,  1 : ODT may not be truned on until one clock after the read post-amble.  0 : ODT may be turned on immediately after read post-amble.  if set to 1 and odt is enabled, the read-to-write latency is increased by. 
        ( 1 << 30 )    				 | //1 : add 1 cycles read to write bus turn around delay. 0: normal turn around delay.
        ( 0 << 31 ) )     			   // read to read, write to write command delay.  0 : BL/2.  1: BL/2 + 1.
		, P_PUB_DTPR2_ADDR);

	// initialization PHY.
	writel(( 16 |	     //PHY RESET APB clock cycles.
		(800 << 6) |	 //tPLLGS    APB clock cycles. 4us
		(200 << 21))     //tPLLPD    APB clock cycles. 1us.
		, P_PUB_PTR0_ADDR);

	writel(( 1800 |	       //tPLLRST   APB clock cycles. 9us 
		(20000 << 16))     //TPLLLOCK  APB clock cycles. 100us. 
		, P_PUB_PTR1_ADDR);	


	//for calibration use all default value here. 
	writel(( 15 << 0 ) |      //tCALON
               ( 15 << 5 ) |  //tCALS
               ( 15 << 10) |  //TCALH
               ( 16 << 15)    //TWLDLYS
		, P_PUB_PTR2_ADDR);	

	

	
	// configure DDR3_rst pin.
	writel(readl(P_PUB_ACIOCR_ADDR) & 0xdfffffff, P_PUB_ACIOCR_ADDR);
		

	//configure for phy update request and ack.
    writel( 0x7f |	//enalbe controller update ack enable and DQS gate Extension.
			  (0x8 << 8 ) |  //PHY update ack delay.
			  (0xf0006 << 12),
			  P_PUB_DSGCR_ADDR );	//other bits.

	//for simulation to reduce the initial time.
	// real value should be based on the DDR3 SDRAM clock cycles.
	writel((299401   | 	  //tDINIT0  CKE low time with power and lock stable. 500us.
		 (136 << 20))	  //tDINIT1. CKE high time to first command(tRFC + 10ns).
		 ,P_PUB_PTR3_ADDR); 
	writel( (119760 |		//tDINIT2. RESET low time,	200us. 
		   (625 << 18))	 //tDINIT3. ZQ initialization command to first command. 1us.
		  ,P_PUB_PTR4_ADDR); 


	//wait PHY DLL LOCK
	while(!(readl(P_PUB_PGSR0_ADDR) & 1)) {}

	// Monitor DFI initialization status.
	while(!(readl(P_UPCTL_DFISTSTAT0_ADDR) & 1)) {} 

	writel(1, P_UPCTL_POWCTL_ADDR);
	while(!(readl(P_UPCTL_POWSTAT_ADDR) & 1) ) {}

	
#ifdef CONFIG_CMD_DDR_TEST
    if(zqcr)
	    writel(zqcr, P_PUB_ZQ0CR1_ADDR);
	else
#endif
    writel(timing_reg->zq0cr1, P_PUB_ZQ0CR1_ADDR);	//get from __ddr_setting

	//for simulation to reduce the init time.
	//MMC_Wr(PUB_PTR1_ADDR,  (20000 | 		  // Tdinit0   DDR3 : 500us.  LPDDR2 : 200us.
	//					  (192 << 19)));	  //tdinit1    DDR3 : tRFC + 10ns. LPDDR2 : 100ns.
	//MMC_Wr(PUB_PTR2_ADDR,  (10000 | 		  //tdinit2    DDR3 : 200us for power up. LPDDR2 : 11us.  
	//					(40 << 17)));		  //tdinit3    LPDDR2 : 1us. 

	
	// initial upctl ddr timing.
	writel(timing_reg->t_refi_100ns, P_UPCTL_TREFI_ADDR); 
	writel(timing_reg->t_mrd, P_UPCTL_TMRD_ADDR);
	writel(timing_reg->t_rfc, P_UPCTL_TRFC_ADDR);
	writel(timing_reg->t_rp, P_UPCTL_TRP_ADDR);	
	writel(timing_reg->t_al, P_UPCTL_TAL_ADDR);
	writel(timing_reg->t_cwl, P_UPCTL_TCWL_ADDR);
	writel(timing_reg->cl, P_UPCTL_TCL_ADDR);
	writel(timing_reg->t_ras, P_UPCTL_TRAS_ADDR); 
	writel(timing_reg->t_rc, P_UPCTL_TRC_ADDR);  
	writel(timing_reg->t_rcd, P_UPCTL_TRCD_ADDR);	
	writel(timing_reg->t_rrd, P_UPCTL_TRRD_ADDR); 
	writel(timing_reg->t_rtp, P_UPCTL_TRTP_ADDR);
	writel(timing_reg->t_wr, P_UPCTL_TWR_ADDR);
	writel(timing_reg->t_wtr, P_UPCTL_TWTR_ADDR);
	writel(timing_reg->t_exsr, P_UPCTL_TEXSR_ADDR);
	writel(timing_reg->t_xp, P_UPCTL_TXP_ADDR);
	writel(timing_reg->t_dqs, P_UPCTL_TDQS_ADDR);
	writel(timing_reg->t_rtw, P_UPCTL_TRTW_ADDR);
	writel(timing_reg->t_cksre, P_UPCTL_TCKSRE_ADDR);
	writel(timing_reg->t_cksrx, P_UPCTL_TCKSRX_ADDR);
	writel(timing_reg->t_mod, P_UPCTL_TMOD_ADDR);
	writel(timing_reg->t_cke, P_UPCTL_TCKE_ADDR);
	writel(64, P_UPCTL_TZQCS_ADDR);
	writel(timing_reg->t_zqcl, P_UPCTL_TZQCL_ADDR);
	writel(10, P_UPCTL_TXPDLL_ADDR);
	writel(1000, P_UPCTL_TZQCSI_ADDR);
	writel(0xf01, P_UPCTL_SCFG_ADDR);  //enable hardware low power interface.
	writel(1, P_UPCTL_SCTL_ADDR);
	while (!(readl(P_UPCTL_STAT_ADDR) & 1))  {}

	//config the DFI interface. 
        //in M6TV the UPCTL works as HDR mode. 
        //please check DDR3/2 SDRAM PHY Utility Block(PUB) Data book table4-13. DFI timming Parameters.
	writel((0xf0 << 1), P_UPCTL_PPCFG_ADDR);
	writel(0x4, P_UPCTL_DFISTCFG0_ADDR);
	writel(0x1, P_UPCTL_DFISTCFG1_ADDR);
	//writel(2, P_UPCTL_DFITCTRLDELAY_ADDR); //debug 11.20
	writel(3, P_UPCTL_DFITCTRLDELAY_ADDR); //debug 11.20// 0x240
	writel(0x1, P_UPCTL_DFITPHYWRDATA_ADDR); //0x250 debug 11.20

	//writel( 0x2, P_UPCTL_DFITPHYWRLAT_ADDR); //0x254   //(WL -4)/2,  2 if CWL = 8 //debug 11.20
	nTempVal = timing_reg->t_cwl+timing_reg->t_al;
	nTempVal = (nTempVal - ((nTempVal%2) ? 3:4))/2;
	writel(nTempVal , P_UPCTL_DFITPHYWRLAT_ADDR); //0x254   //(WL -4)/2,  2 if CWL = 8 //debug 11.20

	//writel(0x3, P_UPCTL_DFITRDDATAEN_ADDR);  //0x260    //(CL -4)/2,  3 if RL = 10. //debug 11.20
	nTempVal = timing_reg->cl+timing_reg->t_al; //debug 11.20
	nTempVal = (nTempVal - ((nTempVal%2) ? 3:4))/2; //debug 11.20
	writel(nTempVal, P_UPCTL_DFITRDDATAEN_ADDR);  //0x260    //(CL -4)/2,  3 if RL = 10. //debug 11.20
	
	writel(13, P_UPCTL_DFITPHYRDLAT_ADDR);   //0x264
	writel(1, P_UPCTL_DFITDRAMCLKDIS_ADDR);  //0x2d4
	writel(1, P_UPCTL_DFITDRAMCLKEN_ADDR);   //0x2d0
	writel(0x10, P_UPCTL_DFITCTRLUPDMIN_ADDR); //0x280
	writel(0x10, P_UPCTL_DFITPHYUPDTYPE0_ADDR); //0270
	writel(1600, P_UPCTL_DFITPHYUPDTYPE1_ADDR); //0x274

       

	writel(( 1 | (3 << 4) | (1 << 8) | (3 << 12) | (7 <<16) | (1 <<24) | ( 3 << 28))
		, P_UPCTL_DFILPCFG0_ADDR);

        writel( 5, P_UPCTL_DFITCTRLUPDI_ADDR); 

	writel(1, P_UPCTL_CMDTSTATEN_ADDR);
	while (!(readl(P_UPCTL_CMDTSTAT_ADDR) & 1 )) {}

        writel(readl(P_PUB_PGCR1_ADDR) & 0xffff83ff,    //set MDL LPF depth = 2.( bit 14:13 = 00). 
                   P_PUB_PGCR1_ADDR);

	writel( (56250-400) | 
                ( 0xf << 20 )  
                , P_PUB_PGCR2_ADDR); 
    
        writel( (0x0 | (0x0 <<12) | (7 << 28))
                ,P_PUB_DTAR0_ADDR );
        writel( (0x8 | (0x0 <<12) | (7 << 28))
                ,P_PUB_DTAR1_ADDR );
        writel( (0x10 | (0x0 <<12) | (7 << 28))
                ,P_PUB_DTAR2_ADDR ); 
        writel( (0x18 | (0x0 <<12) | (7 << 28)) 
                ,P_PUB_DTAR3_ADDR ); 

        writel( ((readl(P_PUB_DTCR_ADDR) & 0xf0ffffff) | (1 << 24) | (1 << 6) ) //debug 11.20
                ,P_PUB_DTCR_ADDR);

	//debug 11.20 begin
	/*
	// DDR PHY initialization 
#ifdef ENABLE_WRITE_LEVELING
    writel(0xf5f3, P_PUB_PIR_ADDR);
#else
	writel(0xfff3, P_PUB_PIR_ADDR);
	*/
	writel((1 |(1<<7)|(1<<8)|(1<<10)),
	P_PUB_PIR_ADDR);
	//debug 11.20 end

	//asm volatile ("wfi"); //debug 11.20

	while( !(readl(P_PUB_PGSR0_ADDR) & 1 ) ) {}

	nTempVal = readl(P_PUB_PGSR0_ADDR);
	
	writel(2, P_UPCTL_SCTL_ADDR); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4

	while ((readl(P_UPCTL_STAT_ADDR) & 0x7 ) != 3 ) {}

	//asm volatile ("wfi"); //debug 11.20

    if (( (nTempVal >> 20) & 0xfff ) != 0x800 ) {
	    return nTempVal;
    }
    else{
        return 0;
    }
}
