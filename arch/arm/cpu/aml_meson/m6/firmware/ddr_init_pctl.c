void load_nop(void)
{
  APB_Wr(UPCTL_MCMD_ADDR, (1 << 31) |
                         (DDR_RANK << 20) |   //rank select
                          NOP_CMD );
  __udelay(1000);
  while ( APB_Rd(UPCTL_MCMD_ADDR) & 0x80000000 ) {}
}
void load_prea(void)
{
  APB_Wr(UPCTL_MCMD_ADDR, (1 << 31) |
                         (DDR_RANK << 20) |   //rank select
                         PREA_CMD );
  while ( APB_Rd(UPCTL_MCMD_ADDR) & 0x80000000 ) {}
}

void load_mrs(int mrs_num, int mrs_value)
{
  APB_Wr(UPCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                   (mrs_num << 17) |
                  (mrs_value << 4) |
                           MRS_CMD );
  __udelay(1000);
  while ( APB_Rd(UPCTL_MCMD_ADDR) & 0x80000000 ) {};
}

void load_ref(void )
{
  APB_Wr(UPCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                          REF_CMD );
  while ( APB_Rd(UPCTL_MCMD_ADDR) & 0x80000000 ) {}
}

void load_zqcl(int zqcl_value )
{
  APB_Wr(UPCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                  (zqcl_value << 4 ) |
                          ZQ_LONG_CMD );
  while ( APB_Rd(UPCTL_MCMD_ADDR) & 0x80000000 ) {}
}

unsigned get_mrs0(struct ddr_set * timing_reg)
{
    unsigned mmc_ctrl=timing_reg->ddr_ctrl;
    unsigned ret=(1<<12)|(0<<3);
    //bl 2==4 0==8
    ret|=mmc_ctrl&(1<<10)?2:0;
    //cl
    ret|=((timing_reg->cl-4)&0x7)<<4;
    //wr: write recovery 
    if(timing_reg->t_wr <= 8)
        ret|=((timing_reg->t_wr-4)&0x7)<<9;
    else
    	  ret|=((timing_reg->t_wr>>1)&7)<<9;
    return ret&0x1fff;
}

unsigned get_mrs1(struct ddr_set * timing_reg)
{
	unsigned ret = 0;
    ret|=timing_reg->mrs[1]&((1<<9)|(1<<6)|(1<<2)| //rtt_nominal
                             (1<<5)|(1<<1) |       //(A5 A1),Output driver impedance control 00:RZQ/6,01:RZQ/7,10£ºRZQ/5 11:Reserved
                             (0<<12)|              //Qoff output buffer
                             (0<<7) );             //Write level  

    //cl
    if(timing_reg->t_al)
    {
        ret|=((timing_reg->cl-timing_reg->t_al)&3)<<3;
    }
    return ret&0x1fff;
}
unsigned get_mrs2(struct ddr_set * timing_reg)
{
    unsigned ret=((timing_reg->t_cwl-5)&7)<<3;
    return ret&0x1fff;
}


int init_pctl_ddr3(struct ddr_set * timing_reg)
{
	//timing_reg=ddr3_timing;
	//set_ddr_clock.
	//	+------------------------------------------------------------+	 +----------------------------------------+
	//	|					   <<< PLL >>>							 |	 |		   <<< Clock Reset Test >>> 	  |
	//	+---------+-----------+----------------------+---------------+	 +------+-----------+---------------------+  +------------
	//	|		  | PLL Ports | 	 PLL		PLL  | PLL	PLL 	 |	 | CRT	|	 Final	|				Ideal |  |	HIU REG 
	//	|	Fin   |  N	  M   | 	Fref		VCO  |	OD	FOUT	 |	 |	XD	|	 Clock	|	 Error		Clock |  |	0x1068
	//	+---------+-----------+----------------------+---------------+	 +------+-----------+---------------------+  +------------
	//	| 24.0000 |  3	 133  |   8.0000  1064.0000  |	 1	532.0000 |	 |	 1	|  532.0000 |  -0.188% ( 533.000) |  | 0x00010685 
	//	| 24.0000 |  3	 125  |   8.0000  1000.0000  |	 1	500.0000 |	 |	 1	|  500.0000 |	0.000% ( 500.000) |  | 0x0001067d

	//write memory timing registers
	MMC_Wr(UPCTL_TOGCNT1U_ADDR, timing_reg->t_1us_pck);	 //1us =  cycles.

	MMC_Wr(UPCTL_TOGCNT100N_ADDR, timing_reg->t_100ns_pck); //100ns = cycles.

	MMC_Wr(UPCTL_TINIT_ADDR, timing_reg->t_init_us);  //200us.

	MMC_Wr(UPCTL_TRSTH_ADDR, 500);	  // 0 for ddr2;  2 for simulation; 500 for ddr3.

	//configure the PCTL for DDR3 SDRAM burst length = 8
	MMC_Wr(UPCTL_MCFG_ADDR, 	1		  |   // burst length 0 = 4; 1 = 8
							   (0 << 2)   |   // bl8int_en.   enable bl8 interrupt function.
							   //(1 << 3)   |   //2T mode by hisun2012.02.10
							   (1 << 5)   |   // 1: ddr3 protocal; 0 : ddr2 protocal
							   (0x0 << 8) |   // 0xf cycle empty will entry power down mode.
							   (1 << 17)  |   // power down exit which fast exit.
							   (1 <<18) 	  // tFAW. 1 : 5*tRRD.
									   );
	  
	//configure DDR PHY PUBL registers.
	//  2:0   011: DDR3 mode.	 100:	LPDDR2 mode.
	//  3:    8 bank. 
	MMC_Wr(PUB_DCR_ADDR, 0x3 | (1 << 3));
	MMC_Wr(PUB_PGCR_ADDR, 0x01842e04); //PUB_PGCR_ADDR: c8001008

	// program PUB MRx registers.
	MMC_Wr( PUB_MR0_ADDR,	(1 << 12 ) |   // 1 fast exit from power down (tXARD), 0 slow (txARDS).
							(4 <<  9 ) |   //wr recovery   4 means write recovery = 8 cycles..
							(0	<< 8 ) |   //DLL reset.
							(0	<< 7 ) |   //0= Normal 1=Test.
							(5	<< 4 ) |   //cas latency high 3 bits (A6,A5, A4, A2) 4 : for cl=8. 5 : for cl=9
							(0	<< 3 ) |   //burst type,  0:sequential 1 Interleave.
							(0	<< 2 ) |   //cas latency bit 0.
								   0 ); 	//burst length	:  2'b00 fixed BL8 
	//MMC_Wr( PUB_MR0_ADDR, 0x940); 
	MMC_Wr( PUB_MR1_ADDR, 0x6); //Rtt = RZQ/4 = 60(M5, M1);	OUTPUT Drive = RZQ/7 = 34(M9,M6,M2); 
	MMC_Wr( PUB_MR2_ADDR, 0x8); //CWL = 6 ( M5, M4, M3).   2.5ns > tCK >= 1.875ns). 
	MMC_Wr( PUB_MR3_ADDR, 0x0); 


	//program DDR SDRAM timing parameter.
	MMC_Wr( PUB_DTPR0_ADDR, (0x0 |		//tMRD.
						(timing_reg->t_rtp <<2) |		//tRTP
						( timing_reg->t_wtr << 5) |		//tWTR
						(timing_reg->t_rp << 8) |		//tRP
						(timing_reg->t_rcd << 12) |		//tRCD
						(timing_reg->t_ras <<16) |		//tRAS
						( timing_reg->t_rrd <<21 ) |		//tRRD
						(timing_reg->t_rc <<25) |		//tRC
						( 0 <<31) ));	//tCCD

	MMC_Wr( PUB_DTPR1_ADDR, ((timing_reg->t_faw << 3) |   //tFAW
							(timing_reg->t_mod << 9) |   //tMOD
							(0 << 11) |   //tRTODT
						   ( timing_reg->t_rfc << 16) |   //tRFC
						   ( 0 << 24) |   //tDQSCK
						   ( 0 << 27) )); //tDQSCKmax

	MMC_Wr( PUB_DTPR2_ADDR, ( 512 |		 //tXS
					 ( timing_reg->t_xp << 10) |		 //tXP
					  ( timing_reg->t_cke << 15) |		 //tCKE
					( 512 << 19) ));	 //tDLLK

	// initialization PHY.
	MMC_Wr( PUB_PTR0_ADDR,  ( 27 |	  //tDLL_SRST 
					(2750 << 6) |	  //tDLLLOCK 
					   (8 <<18)));	   //tITMSRST 

	//wait PHY DLL LOCK
	while(!(MMC_Rd( PUB_PGSR_ADDR) & 1)) {}

	// configure DDR3_rst pin.
	MMC_Wr( PUB_ACIOCR_ADDR, MMC_Rd( PUB_ACIOCR_ADDR) & 0xdfffffff );
	MMC_Wr( PUB_DSGCR_ADDR,	MMC_Rd(PUB_DSGCR_ADDR) & 0xffffffef); 

	//for simulation to reduce the init time.
	//MMC_Wr(PUB_PTR1_ADDR,  (20000 | 		  // Tdinit0   DDR3 : 500us.  LPDDR2 : 200us.
	//					  (192 << 19)));	  //tdinit1    DDR3 : tRFC + 10ns. LPDDR2 : 100ns.
	//MMC_Wr(PUB_PTR2_ADDR,  (10000 | 		  //tdinit2    DDR3 : 200us for power up. LPDDR2 : 11us.  
	//					(40 << 17)));		  //tdinit3    LPDDR2 : 1us. 

	//wait DDR3_ZQ_DONE: 
	while( !(MMC_Rd( PUB_PGSR_ADDR) & (1<< 2))) {}

	// wait DDR3_PHY_INIT_WAIT : 
	while (!(MMC_Rd(PUB_PGSR_ADDR) & 1 )) {}

	// Monitor DFI initialization status.
	while(!(MMC_Rd(UPCTL_DFISTSTAT0_ADDR) & 1)) {} 

	MMC_Wr(UPCTL_POWCTL_ADDR, 1);
	while(!(MMC_Rd( UPCTL_POWSTAT_ADDR & 1) )) {}




	// initial upctl ddr timing.
	MMC_Wr(UPCTL_TREFI_ADDR, timing_reg->t_refi_100ns);  // 7800ns to one refresh command.
	// wr_reg UPCTL_TREFI_ADDR, 78

	MMC_Wr(UPCTL_TMRD_ADDR, timing_reg->t_mrd);
	//wr_reg UPCTL_TMRD_ADDR, 4

	MMC_Wr(UPCTL_TRFC_ADDR, timing_reg->t_rfc);	//64: 400Mhz.  86: 533Mhz. 
	// wr_reg UPCTL_TRFC_ADDR, 86

	MMC_Wr(UPCTL_TRP_ADDR, timing_reg->t_rp);	// 8 : 533Mhz.
	//wr_reg UPCTL_TRP_ADDR, 8

	MMC_Wr(UPCTL_TAL_ADDR,	timing_reg->t_al);
	//wr_reg UPCTL_TAL_ADDR, 0

	MMC_Wr(UPCTL_TCWL_ADDR,  timing_reg->t_cwl);
	//wr_reg UPCTL_TCWL_ADDR, 6

	MMC_Wr(UPCTL_TCL_ADDR, timing_reg->cl);	 //6: 400Mhz. 8 : 533Mhz.
	// wr_reg UPCTL_TCL_ADDR, 8

	MMC_Wr(UPCTL_TRAS_ADDR, timing_reg->t_ras); //16: 400Mhz. 20: 533Mhz.
	//  wr_reg UPCTL_TRAS_ADDR, 20 

	MMC_Wr(UPCTL_TRC_ADDR, timing_reg->t_rc);  //24 400Mhz. 28 : 533Mhz.
	//wr_reg UPCTL_TRC_ADDR, 28

	MMC_Wr(UPCTL_TRCD_ADDR, timing_reg->t_rcd);	//6: 400Mhz. 8: 533Mhz.
	// wr_reg UPCTL_TRCD_ADDR, 8

	MMC_Wr(UPCTL_TRRD_ADDR, timing_reg->t_rrd); //5: 400Mhz. 6: 533Mhz.
	//wr_reg UPCTL_TRRD_ADDR, 6

	MMC_Wr(UPCTL_TRTP_ADDR, timing_reg->t_rtp);
	// wr_reg UPCTL_TRTP_ADDR, 4

	MMC_Wr(UPCTL_TWR_ADDR, timing_reg->t_wr);
	// wr_reg UPCTL_TWR_ADDR, 8

	MMC_Wr(UPCTL_TWTR_ADDR, timing_reg->t_wtr);
	//wr_reg UPCTL_TWTR_ADDR, 4


	MMC_Wr(UPCTL_TEXSR_ADDR, timing_reg->t_exsr);
	//wr_reg UPCTL_TEXSR_ADDR, 200

	MMC_Wr(UPCTL_TXP_ADDR, timing_reg->t_xp);
	//wr_reg UPCTL_TXP_ADDR, 4

	MMC_Wr(UPCTL_TDQS_ADDR, timing_reg->t_dqs);
	// wr_reg UPCTL_TDQS_ADDR, 2

	MMC_Wr(UPCTL_TRTW_ADDR, timing_reg->t_rtw);
	//wr_reg UPCTL_TRTW_ADDR, 2

	MMC_Wr(UPCTL_TCKSRE_ADDR, timing_reg->t_cksre);
	//wr_reg UPCTL_TCKSRE_ADDR, 5 

	MMC_Wr(UPCTL_TCKSRX_ADDR, timing_reg->t_cksrx);
	//wr_reg UPCTL_TCKSRX_ADDR, 5 

	MMC_Wr(UPCTL_TMOD_ADDR, timing_reg->t_mod);
	//wr_reg UPCTL_TMOD_ADDR, 8

	MMC_Wr(UPCTL_TCKE_ADDR, timing_reg->t_cke);
	//wr_reg UPCTL_TCKE_ADDR, 4 

	MMC_Wr(UPCTL_TZQCS_ADDR, 64);
	//wr_reg UPCTL_TZQCS_ADDR , 64 

	MMC_Wr(UPCTL_TZQCL_ADDR, timing_reg->t_zqcl);
	//wr_reg UPCTL_TZQCL_ADDR , 512 

	MMC_Wr(UPCTL_TXPDLL_ADDR, 10);
	// wr_reg UPCTL_TXPDLL_ADDR, 10  

	MMC_Wr(UPCTL_TZQCSI_ADDR, 1000);
	// wr_reg UPCTL_TZQCSI_ADDR, 1000  

	MMC_Wr(UPCTL_SCFG_ADDR, 0xf00);

	// wr_reg UPCTL_SCFG_ADDR, 0xf00 

	MMC_Wr( UPCTL_SCTL_ADDR, 1);
	while (!( MMC_Rd(UPCTL_STAT_ADDR) & 1))  {}

	//config the DFI interface.
	MMC_Wr( UPCTL_PPCFG_ADDR, (0xf0 << 1) );
	MMC_Wr( UPCTL_DFITCTRLDELAY_ADDR, 2 );
	MMC_Wr( UPCTL_DFITPHYWRDATA_ADDR,  0x1 );
	MMC_Wr( UPCTL_DFITPHYWRLAT_ADDR, timing_reg->t_cwl -1  );    //CWL -1
	MMC_Wr( UPCTL_DFITRDDATAEN_ADDR, timing_reg->cl - 2  );    //CL -2
	MMC_Wr( UPCTL_DFITPHYRDLAT_ADDR, 15 );
	MMC_Wr( UPCTL_DFITDRAMCLKDIS_ADDR, 2 );
	MMC_Wr( UPCTL_DFITDRAMCLKEN_ADDR, 2 );
	MMC_Wr( UPCTL_DFISTCFG0_ADDR, 0x4  );
	MMC_Wr( UPCTL_DFITCTRLUPDMIN_ADDR, 0x4000 );
	MMC_Wr( UPCTL_DFILPCFG0_ADDR, ( 1 | (7 << 4) | (1 << 8) | (10 << 12) | (12 <<16) | (1 <<24) | ( 7 << 28)));

	MMC_Wr( UPCTL_CMDTSTATEN_ADDR, 1);
	while (!(MMC_Rd(UPCTL_CMDTSTAT_ADDR) & 1 )) {}

	MMC_Wr( PUB_DTAR_ADDR, (0x0 | (0x0 <<12) | (7 << 28))); 

	// DDR PHY initialization 
	MMC_Wr( PUB_PIR_ADDR, 0x1e1); 
	//DDR3_SDRAM_INIT_WAIT : 
	while( !(MMC_Rd(PUB_PGSR_ADDR & 1))) {}

	MMC_Wr(UPCTL_SCTL_ADDR, 2); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4

	while ((MMC_Rd(UPCTL_STAT_ADDR) & 0x7 ) != 3 ) {}

	return 0;

}
