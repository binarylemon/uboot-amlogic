#include <config.h>
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/dmc.h>
#include <asm/arch/ddr.h>
#include <asm/arch/memtest.h>
#include <asm/arch/pctl.h>

#define dbg_out(s,v) f_serial_puts(s);serial_put_hex(v,32);f_serial_puts("\n");wait_uart_empty();
#define dbg_puts(s) f_serial_puts(s);wait_uart_empty();

#if 0
void __udelay(int n)
{	
	unsigned base= get_tick(0);
	while(get_tick(base) < n);
}
#else
void __udelay(int n)
{	
	int i;
	for(i=0;i<n;i++)
	{
	    asm("mov r0,r0");
	}
}
#endif

void disable_mmc_req(void)
{
	APB_Wr(MMC_REQ_CTRL,0X0);
  	while(APB_Rd(MMC_CHAN_STS) == 0){
		__udelay(10);	
	}
}

void reset_mmc(void)
{
   APB_Wr(MMC_SOFT_RST, 0x17ff);
   while(APB_Rd(MMC_SOFT_RST) != 0);
}

void enable_mmc_req(void)
{
	// Next, we enable all requests
	APB_Wr(MMC_REQ_CTRL, 0xff);
	while(APB_Rd(MMC_CHAN_STS) == 0){
		__udelay(10);	
	}
}
void mmc_sleep(void)
{
	int stat;
	do
	{
		stat = APB_Rd(UPCTL_STAT_ADDR);
		stat &= 0x7;
		if(stat == UPCTL_STAT_INIT) {
			APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_CONFIG);
		}
		else if(stat == UPCTL_STAT_CONFIG) {
			APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_GO);
		}
		else if(stat == UPCTL_STAT_ACCESS) {
				APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_SLEEP);
		}
	}while(stat != UPCTL_STAT_LOW_POWER);
}

void mmc_wakeup(void)
{
	int stat;
	do
	{
		stat = APB_Rd(UPCTL_STAT_ADDR);
		stat &= 0x7;
		if(stat == UPCTL_STAT_LOW_POWER) {
			APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_WAKEUP);
			//while(stat != UPCTL_STAT_LOW_POWER);
		}
		else if(stat == UPCTL_STAT_INIT) {
			APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_CONFIG);
			//while(stat != UPCTL_STAT_CONFIG);
		}
		else if(stat == UPCTL_STAT_CONFIG) {
			APB_Wr(UPCTL_SCTL_ADDR, SCTL_CMD_GO);
			//while(stat != UPCTL_STAT_ACCESS);
		}
	}while(stat != UPCTL_STAT_LOW_POWER);
}
/*
#define DDR_RSLR_LEN 6
#define DDR_RDGR_LEN 4
#define PHYS_MEMORY_START 0x80000000
#define DDR3_2Gbx16
#define DDR_RANK  1   // 2'b11 means 2 rank.
#define NOP_CMD  0
#define PREA_CMD  1
#define REF_CMD  2
#define MRS_CMD  3
#define ZQ_SHORT_CMD 4
#define ZQ_LONG_CMD  5
#define SFT_RESET_CMD 6
*/

#define MMC_Wr(addr,v) writel((v),(APB_REG_ADDR(addr)))
#define MMC_Rd(addr) readl((APB_REG_ADDR(addr)))

void disp_pctl(void)
{
#if 1
#if 0
	dbg_out("DDR_PLL_CNTL:",readl(P_HHI_DDR_PLL_CNTL));
	dbg_out("DDR_PLL_CNTL2:",readl(P_HHI_DDR_PLL_CNTL2));
	dbg_out("DDR_PLL_CNTL3:",readl(P_HHI_DDR_PLL_CNTL3));
	dbg_out("DDR_PLL_CNTL4:", readl(P_HHI_DDR_PLL_CNTL4));
	
	dbg_out("MC_DDR_CTRL:",readl(P_MMC_DDR_CTRL));
	dbg_out("MMC_PHY_CTRL:",readl(P_MMC_PHY_CTRL));
	dbg_out("MMC_CLK_CNTL:",MMC_Rd(MMC_CLK_CNTL));
		
 	 dbg_out("U_TOGCNT1U_ADDR ",MMC_Rd(UPCTL_TOGCNT1U_ADDR )); 
 	 dbg_out("U_TOGCNT100N_ADDR ",MMC_Rd(UPCTL_TOGCNT100N_ADDR ));
 	 dbg_out("U_TINIT_ADDR ",MMC_Rd(UPCTL_TINIT_ADDR )); 
 	 dbg_out("U_TREFI:",MMC_Rd(UPCTL_TREFI_ADDR )); 
 
   dbg_out("U_TMRD:",MMC_Rd(UPCTL_TMRD_ADDR )); 
 	 dbg_out("U_TRFC:",MMC_Rd(UPCTL_TRFC_ADDR )); 
 	 dbg_out("U_TRP:",MMC_Rd(UPCTL_TRP_ADDR )); 
 	 dbg_out("U_TAL:",MMC_Rd(UPCTL_TAL_ADDR )); 
 	 dbg_out("U_TCL:",MMC_Rd(UPCTL_TCL_ADDR )); 
 	 dbg_out("U_TRAS:",MMC_Rd(UPCTL_TRAS_ADDR )); 
 	 dbg_out("U_TRC:",MMC_Rd(UPCTL_TRC_ADDR )); 
 	 dbg_out("U_TRCD:",MMC_Rd(UPCTL_TRCD_ADDR )); 
 	 dbg_out("U_TRRD:",MMC_Rd(UPCTL_TRRD_ADDR ));
 	 dbg_out("U_TRTP:",MMC_Rd(UPCTL_TRTP_ADDR ));
 	 dbg_out("U_TWR:",MMC_Rd(UPCTL_TWR_ADDR ));
 	 dbg_out("U_TWTR:",MMC_Rd(UPCTL_TWTR_ADDR )); 
 	 dbg_out("U_TEXSR:",MMC_Rd(UPCTL_TEXSR_ADDR )); 
 	 dbg_out("U_TXP:",MMC_Rd(UPCTL_TXP_ADDR ));
 	 dbg_out("U_TDQS:",MMC_Rd(UPCTL_TDQS_ADDR ));
 	 dbg_out("U_TRTW:",MMC_Rd(UPCTL_TRTW_ADDR ));
 	 dbg_out("U_TMOD:",MMC_Rd(UPCTL_TMOD_ADDR ));
 	 dbg_out("U_TCWL:",MMC_Rd(UPCTL_TCWL_ADDR ));
	
	
		dbg_out("U_MCFG:",MMC_Rd(UPCTL_MCFG_ADDR));
		dbg_out("U_TZQCL:",MMC_Rd(UPCTL_TZQCL_ADDR));
		
		dbg_out("U_TRSTH:",MMC_Rd(UPCTL_TRSTH_ADDR));
		dbg_out("U_TRSTL:",MMC_Rd(UPCTL_TRSTL_ADDR));
		dbg_out("U_TCKE:",MMC_Rd(UPCTL_TCKE_ADDR));
		dbg_out("U_TCKSRE:",MMC_Rd(UPCTL_TCKSRE_ADDR));
		dbg_out("U_TCKSRX:",MMC_Rd(UPCTL_TCKSRX_ADDR));
		
		dbg_out("PUB_DTPR0:",MMC_Rd(PUB_DTPR0_ADDR));
		dbg_out("PUB_DTPR1:",MMC_Rd(PUB_DTPR1_ADDR));
		dbg_out("PUB_DTPR2:",MMC_Rd(PUB_DTPR2_ADDR));
		dbg_out("PUB_PTR0:",MMC_Rd(PUB_PTR0_ADDR));
		dbg_out("PUB_PTR1:",MMC_Rd(PUB_PTR1_ADDR));
		dbg_out("PUB_PTR2:",MMC_Rd(PUB_PTR2_ADDR));
		
		dbg_out("PUB_MR0:",MMC_Rd( PUB_MR0_ADDR));
		dbg_out("PUB_MR1:",MMC_Rd( PUB_MR1_ADDR));
		dbg_out("PUB_MR2:",MMC_Rd( PUB_MR2_ADDR));
		dbg_out("PUB_MR3:",MMC_Rd( PUB_MR3_ADDR));	

    dbg_out("U_PPCFG:",MMC_Rd( UPCTL_PPCFG_ADDR));
    dbg_out("U_DFISTCFG0:",MMC_Rd( UPCTL_DFISTCFG0_ADDR));
    dbg_out("U_DFITPHYWRLAT:",MMC_Rd( UPCTL_DFITPHYWRLAT_ADDR));
    dbg_out("U_DFITRDDATAEN:",MMC_Rd( UPCTL_DFITRDDATAEN_ADDR));
    dbg_out("U_DFITPHYWRDATA:",MMC_Rd( UPCTL_DFITPHYWRDATA_ADDR));
    dbg_out("U_DFITPHYRDLAT:",MMC_Rd( UPCTL_DFITPHYRDLAT_ADDR));
    dbg_out("U_DFITDRAMCLKDIS:",MMC_Rd( UPCTL_DFITDRAMCLKDIS_ADDR));
    dbg_out("U_DFITDRAMCLKEN:",MMC_Rd( UPCTL_DFITDRAMCLKEN_ADDR));
    dbg_out("U_DFITCTRLDELAY:",MMC_Rd( UPCTL_DFITCTRLDELAY_ADDR));
    dbg_out("U_DFITCTRLUPDMIN:",MMC_Rd( UPCTL_DFITCTRLUPDMIN_ADDR));
    dbg_out("U_DFILPCFG0:",MMC_Rd( UPCTL_DFILPCFG0_ADDR)); 
    dbg_out("PUB_PIR:",MMC_Rd( PUB_PIR_ADDR)); 
    dbg_out("PUB_PGCR:",MMC_Rd( PUB_PGCR_ADDR)); 
	
		dbg_out("U_LPDDR2ZQCFG:",MMC_Rd(UPCTL_LPDDR2ZQCFG_ADDR));
	  dbg_out("U_ZQCR:",MMC_Rd(UPCTL_ZQCR_ADDR));
	  
	  
	  
	dbg_out("PUB_ACIOCR:",MMC_Rd( PUB_ACIOCR_ADDR));
	dbg_out("PUB_DSGCR:",MMC_Rd( PUB_DSGCR_ADDR)); 

  dbg_out("PUB_ZQ0CR1:",MMC_Rd( PUB_ZQ0CR1_ADDR));
    
	dbg_out("U_TZQCS:",MMC_Rd(UPCTL_TZQCS_ADDR));
	dbg_out("U_TZQCL:",MMC_Rd(UPCTL_TZQCL_ADDR));

	dbg_out("U_TXPDLL:",MMC_Rd(UPCTL_TXPDLL_ADDR));

	dbg_out("U_TZQCSI:",MMC_Rd(UPCTL_TZQCSI_ADDR));

	dbg_out("U_SCFG:",MMC_Rd(UPCTL_SCFG_ADDR));
	dbg_out("PUB_DTAR:",MMC_Rd( PUB_DTAR_ADDR)); 

	dbg_out("MMC_REQ_CTRL:",MMC_Rd(MMC_REQ_CTRL)); 
	
	dbg_out("U_DLLCR9:",MMC_Rd(UPCTL_DLLCR9_ADDR)); //2a8	
	dbg_out("U_IOCR:",MMC_Rd(UPCTL_IOCR_ADDR)); //248
#endif
	
  dbg_out("U_DLLCR0:",MMC_Rd(UPCTL_DLLCR0_ADDR)); //284
  dbg_out("U_DLLCR1:",MMC_Rd(UPCTL_DLLCR1_ADDR)); //288
  dbg_out("U_DLLCR2:",MMC_Rd(UPCTL_DLLCR2_ADDR)); //28c
  dbg_out("U_DLLCR3:",MMC_Rd(UPCTL_DLLCR3_ADDR)); //290
  dbg_out("U_DQSTR:",MMC_Rd(UPCTL_DQSTR_ADDR));   //2e4
  dbg_out("U_DQSNTR:",MMC_Rd(UPCTL_DQSNTR_ADDR)); //2e8
  dbg_out("U_DQTR0:",MMC_Rd(UPCTL_DQTR0_ADDR));     //2c0
  dbg_out("U_DQTR1:",MMC_Rd(UPCTL_DQTR1_ADDR));     //2c4
  dbg_out("U_DQTR2:",MMC_Rd(UPCTL_DQTR2_ADDR));     //2c8
  dbg_out("U_DQTR3:",MMC_Rd(UPCTL_DQTR3_ADDR));     //2cc

	 dbg_out("U_RDGR0_ADDR      ",MMC_Rd(UPCTL_RDGR0_ADDR));
	 dbg_out("U_RSLR0_ADDR     ",MMC_Rd(UPCTL_RSLR0_ADDR));


  dbg_out("PUB_DX0GSR0:", MMC_Rd(PUB_DX0GSR0_ADDR)); 
	dbg_out("PUB_DX0GSR1:", MMC_Rd(PUB_DX0GSR1_ADDR)); 
	dbg_out("PUB_DX0DQSTR:",MMC_Rd(PUB_DX0DQSTR_ADDR)); 

  dbg_out("PUB_DX1GSR0:", MMC_Rd(PUB_DX1GSR0_ADDR)); 
	dbg_out("PUB_DX1GSR1:", MMC_Rd(PUB_DX1GSR1_ADDR)); 
	dbg_out("PUB_DX1DQSTR:",MMC_Rd(PUB_DX1DQSTR_ADDR)); 

  dbg_out("PUB_DX2GSR0:", MMC_Rd(PUB_DX2GSR0_ADDR)); 
	dbg_out("PUB_DX2GSR1:", MMC_Rd(PUB_DX2GSR1_ADDR)); 
	dbg_out("PUB_DX2DQSTR:",MMC_Rd(PUB_DX2DQSTR_ADDR)); 

  dbg_out("PUB_DX3GSR0:", MMC_Rd(PUB_DX3GSR0_ADDR)); 
	dbg_out("PUB_DX3GSR1:", MMC_Rd(PUB_DX3GSR1_ADDR)); 
	dbg_out("PUB_DX3DQSTR:",MMC_Rd(PUB_DX3DQSTR_ADDR)); 

  dbg_out("PUB_DX4GSR0:", MMC_Rd(PUB_DX4GSR0_ADDR)); 
	dbg_out("PUB_DX4GSR1:", MMC_Rd(PUB_DX4GSR1_ADDR)); 
	dbg_out("PUB_DX4DQSTR:",MMC_Rd(PUB_DX4DQSTR_ADDR)); 

  dbg_out("PUB_DX5GSR0:", MMC_Rd(PUB_DX5GSR0_ADDR)); 
	dbg_out("PUB_DX5GSR1:", MMC_Rd(PUB_DX5GSR1_ADDR)); 
	dbg_out("PUB_DX5DQSTR:",MMC_Rd(PUB_DX5DQSTR_ADDR)); 

  dbg_out("PUB_DX6GSR0:", MMC_Rd(PUB_DX6GSR0_ADDR)); 
	dbg_out("PUB_DX6GSR1:", MMC_Rd(PUB_DX6GSR1_ADDR)); 
	dbg_out("PUB_DX6DQSTR:",MMC_Rd(PUB_DX6DQSTR_ADDR)); 

  dbg_out("PUB_DX7GSR0:", MMC_Rd(PUB_DX7GSR0_ADDR)); 
	dbg_out("PUB_DX7GSR1:", MMC_Rd(PUB_DX7GSR1_ADDR)); 
	dbg_out("PUB_DX7DQSTR:",MMC_Rd(PUB_DX7DQSTR_ADDR)); 

  dbg_out("PUB_DX8GSR0:", MMC_Rd(PUB_DX8GSR0_ADDR)); 
	dbg_out("PUB_DX8GSR1:", MMC_Rd(PUB_DX8GSR1_ADDR)); 
	dbg_out("PUB_DX8DQSTR:",MMC_Rd(PUB_DX8DQSTR_ADDR)); 

#endif
}

unsigned ddr_settings[DDR_SETTING_COUNT];
void save_ddr_settings()
{
	v_ddr_pll_cntl =  readl(P_HHI_DDR_PLL_CNTL);
	v_ddr_pll_cntl2 = readl(P_HHI_DDR_PLL_CNTL2);
	v_ddr_pll_cntl3 = readl(P_HHI_DDR_PLL_CNTL3);
	v_ddr_pll_cntl4 = readl(P_HHI_DDR_PLL_CNTL4);
	
	v_mmc_ddr_ctrl = readl(P_MMC_DDR_CTRL);
	v_mmc_phy_ctrl = readl(P_MMC_PHY_CTRL);
	v_mmc_clk_cntl = MMC_Rd(MMC_CLK_CNTL);
	
	v_rdgr0 = MMC_Rd(UPCTL_RDGR0_ADDR);
	v_rslr0 = MMC_Rd(UPCTL_RSLR0_ADDR);
	
 	v_t_1us_pck      = MMC_Rd(UPCTL_TOGCNT1U_ADDR ); 
 	v_t_100ns_pck    = MMC_Rd(UPCTL_TOGCNT100N_ADDR );
 	v_t_init_us      = MMC_Rd(UPCTL_TINIT_ADDR ); 
 	v_t_refi_100ns   = MMC_Rd(UPCTL_TREFI_ADDR ); 
 
  v_t_mrd          = MMC_Rd(UPCTL_TMRD_ADDR ); 
 	v_t_rfc          = MMC_Rd(UPCTL_TRFC_ADDR ); 
 	v_t_rp           = MMC_Rd(UPCTL_TRP_ADDR ); 
 	v_t_al           = MMC_Rd(UPCTL_TAL_ADDR ); 
 	v_t_cl           = MMC_Rd(UPCTL_TCL_ADDR ); 
 	v_t_ras          = MMC_Rd(UPCTL_TRAS_ADDR ); 
 	v_t_rc           = MMC_Rd(UPCTL_TRC_ADDR ); 
 	v_t_rcd          = MMC_Rd(UPCTL_TRCD_ADDR ); 
 	v_t_rrd          = MMC_Rd(UPCTL_TRRD_ADDR );
 	v_t_rtp          = MMC_Rd(UPCTL_TRTP_ADDR );
 	v_t_wr           = MMC_Rd(UPCTL_TWR_ADDR );
 	v_t_wtr          = MMC_Rd(UPCTL_TWTR_ADDR ); 
 	v_t_exsr         = MMC_Rd(UPCTL_TEXSR_ADDR ); 
 	v_t_xp           = MMC_Rd(UPCTL_TXP_ADDR );
 	v_t_dqs          = MMC_Rd(UPCTL_TDQS_ADDR );
 	v_t_trtw         = MMC_Rd(UPCTL_TRTW_ADDR );
 	v_t_mod          = MMC_Rd(UPCTL_TMOD_ADDR );
 	v_t_cwl          = MMC_Rd(UPCTL_TCWL_ADDR );
 	
 	v_mcfg           = MMC_Rd(UPCTL_MCFG_ADDR);
 	v_t_zqcl         = MMC_Rd(UPCTL_TZQCL_ADDR);

  v_t_rsth_us  = MMC_Rd(UPCTL_TRSTH_ADDR);
  v_t_rstl_us  = MMC_Rd(UPCTL_TRSTL_ADDR);
  v_t_cke      = MMC_Rd(UPCTL_TCKE_ADDR);
  v_t_cksre    = MMC_Rd(UPCTL_TCKSRE_ADDR);
  v_t_cksrx    = MMC_Rd(UPCTL_TCKSRX_ADDR);

  v_pub_dtpr0   = MMC_Rd(PUB_DTPR0_ADDR);
  v_pub_dtpr1   = MMC_Rd(PUB_DTPR1_ADDR);
  v_pub_dtpr2   = MMC_Rd(PUB_DTPR2_ADDR);
  v_pub_ptr0    = MMC_Rd(PUB_PTR0_ADDR);
  v_pub_ptr1    = MMC_Rd(PUB_PTR1_ADDR);
  v_pub_ptr2    = MMC_Rd(PUB_PTR2_ADDR);

	v_msr0 = MMC_Rd( PUB_MR0_ADDR);
	v_msr1 = MMC_Rd( PUB_MR1_ADDR);
	v_msr2 = MMC_Rd( PUB_MR2_ADDR);
	v_msr3 = MMC_Rd( PUB_MR3_ADDR);	

	v_odtcfg = MMC_Rd( UPCTL_LPDDR2ZQCFG_ADDR);	
	v_zqcr = MMC_Rd( UPCTL_ZQCR_ADDR);	
	
	v_dllcr9 = MMC_Rd(UPCTL_DLLCR9_ADDR); //2a8	
	v_iocr = MMC_Rd(UPCTL_IOCR_ADDR); //248
	
  v_dllcr0 = MMC_Rd(UPCTL_DLLCR0_ADDR); //284
  v_dllcr1 = MMC_Rd(UPCTL_DLLCR1_ADDR); //288
  v_dllcr2 = MMC_Rd(UPCTL_DLLCR2_ADDR); //28c
  v_dllcr3 = MMC_Rd(UPCTL_DLLCR3_ADDR); //290
  v_dqscr = MMC_Rd(UPCTL_DQSTR_ADDR);   //2e4
  v_dqsntr = MMC_Rd(UPCTL_DQSNTR_ADDR); //2e8
  v_tr0 = MMC_Rd(UPCTL_DQTR0_ADDR);     //2c0
  v_tr1 = MMC_Rd(UPCTL_DQTR1_ADDR);     //2c4
  v_tr2 = MMC_Rd(UPCTL_DQTR2_ADDR);     //2c8
  v_tr3 = MMC_Rd(UPCTL_DQTR3_ADDR);     //2cc
  
  
  v_dx0gsr0  = MMC_Rd(PUB_DX0GSR0_ADDR); 
	v_dx0gsr1  = MMC_Rd(PUB_DX0GSR1_ADDR); 
	v_dx0dqstr = MMC_Rd(PUB_DX0DQSTR_ADDR); 
	
  v_dx1gsr0  = MMC_Rd(PUB_DX1GSR0_ADDR); 
	v_dx1gsr1  = MMC_Rd(PUB_DX1GSR1_ADDR); 
	v_dx1dqstr = MMC_Rd(PUB_DX1DQSTR_ADDR); 

  v_dx2gsr0  = MMC_Rd(PUB_DX2GSR0_ADDR); 
	v_dx2gsr1  = MMC_Rd(PUB_DX2GSR1_ADDR); 
	v_dx2dqstr = MMC_Rd(PUB_DX2DQSTR_ADDR); 

  v_dx3gsr0  = MMC_Rd(PUB_DX3GSR0_ADDR); 
	v_dx3gsr1  = MMC_Rd(PUB_DX3GSR1_ADDR); 
	v_dx3dqstr = MMC_Rd(PUB_DX3DQSTR_ADDR); 

  v_dx4gsr0  = MMC_Rd(PUB_DX4GSR0_ADDR); 
	v_dx4gsr1  = MMC_Rd(PUB_DX4GSR1_ADDR); 
	v_dx4dqstr = MMC_Rd(PUB_DX4DQSTR_ADDR); 

  v_dx5gsr0  = MMC_Rd(PUB_DX5GSR0_ADDR); 
	v_dx5gsr1  = MMC_Rd(PUB_DX5GSR1_ADDR); 
	v_dx5dqstr = MMC_Rd(PUB_DX5DQSTR_ADDR); 

  v_dx6gsr0  = MMC_Rd(PUB_DX6GSR0_ADDR); 
	v_dx6gsr1  = MMC_Rd(PUB_DX6GSR1_ADDR); 
	v_dx6dqstr = MMC_Rd(PUB_DX6DQSTR_ADDR); 

  v_dx7gsr0  = MMC_Rd(PUB_DX7GSR0_ADDR); 
	v_dx7gsr1  = MMC_Rd(PUB_DX7GSR1_ADDR); 
	v_dx7dqstr = MMC_Rd(PUB_DX7DQSTR_ADDR); 

  v_dx8gsr0  = MMC_Rd(PUB_DX8GSR0_ADDR); 
	v_dx8gsr1  = MMC_Rd(PUB_DX8GSR1_ADDR); 
	v_dx8dqstr = MMC_Rd(PUB_DX8DQSTR_ADDR); 
	
}

#if 0
#define sec_mmc_wr(addr,v) writel(v,addr)
void init_dmc(void)
{
		dbg_puts("init dmc\n");
		MMC_Wr(MMC_DDR_CTRL, v_mmc_ddr_ctrl);
		dbg_out("p0:",readl(DMC_SEC_PORT0_RANGE0));
		dbg_out("p1:",readl(DMC_SEC_PORT1_RANGE0));
		dbg_out("p2:",readl(DMC_SEC_PORT2_RANGE0));
		dbg_out("p3:",readl(DMC_SEC_PORT3_RANGE0));
		dbg_out("p4:",readl(DMC_SEC_PORT4_RANGE0));
		dbg_out("p5:",readl(DMC_SEC_PORT5_RANGE0));
		dbg_out("p6:",readl(DMC_SEC_PORT6_RANGE0));
		dbg_out("p7:",readl(DMC_SEC_PORT7_RANGE0));
		dbg_out("sectrl:",readl(DMC_SEC_CTRL));
	  sec_mmc_wr(DMC_SEC_PORT0_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT1_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT2_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT3_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT4_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT5_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT6_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT7_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_CTRL,         0x80000000);
	
		dbg_puts("init dmc\n");

		//APB_Wr(MMC_REQ_CTRL,0xff); //hisun 2012.02.08
		MMC_Wr(MMC_REQ_CTRL,0xff);   //hisun 2012.02.08
		dbg_puts("init dmc\n");
}
#endif

void init_pctl(void)
{
	int nTempVal;
  
  MMC_Wr(MMC_DDR_CTRL,v_mmc_ddr_ctrl);
	MMC_Wr(MMC_PHY_CTRL,v_mmc_phy_ctrl);
	MMC_Wr(UPCTL_DLLCR9_ADDR, v_dllcr9); //2a8	
	MMC_Wr(UPCTL_IOCR_ADDR, v_iocr); //248

	
  //wait to DDR PLL lock.
    while (!(MMC_Rd(MMC_CLK_CNTL) & (1<<29)) ) {}
  //Enable DDR DLL clock input from PLL.
     MMC_Wr(MMC_CLK_CNTL, 0xc0000080);  //  @@@ select the final mux from PLL output directly.
     MMC_Wr(MMC_CLK_CNTL, 0xc00000c0);    
    //enable the clock.
     MMC_Wr(MMC_CLK_CNTL, v_mmc_clk_cntl);
     
    // release the DDR DLL reset pin.
    MMC_Wr( MMC_SOFT_RST,  0xffff);
	
	//UPCTL memory timing registers
	MMC_Wr(UPCTL_TOGCNT1U_ADDR, v_t_1us_pck);	 //1us = nn cycles.
	MMC_Wr(UPCTL_TOGCNT100N_ADDR, v_t_100ns_pck);//100ns = nn cycles.
	MMC_Wr(UPCTL_TINIT_ADDR, v_t_init_us);  //200us.
	
//  MMC_Wr(UPCTL_TRSTH_ADDR, 2);      // 0 for ddr2; 500 for ddr3. 2 for simulation.
	MMC_Wr(UPCTL_TRSTH_ADDR, v_t_rsth_us);  // 0 for ddr2;  2 for simulation; 500 for ddr3.
	MMC_Wr(UPCTL_TRSTL_ADDR, v_t_rstl_us);
	
	MMC_Wr(UPCTL_MCFG_ADDR,v_mcfg);
	 
	//configure DDR PHY PUBL registers.
	//  2:0   011: DDR3 mode.	 100:	LPDDR2 mode.
	//  3:    8 bank. 
	MMC_Wr(PUB_DCR_ADDR, 0x3 | (1 << 3));
	MMC_Wr(PUB_PGCR_ADDR, 0x01842e04); //PUB_PGCR_ADDR: c8001008

	MMC_Wr( PUB_MR0_ADDR,v_msr0);
	MMC_Wr( PUB_MR1_ADDR,v_msr1);
	MMC_Wr( PUB_MR2_ADDR,v_msr2);
	MMC_Wr( PUB_MR3_ADDR,v_msr3);	
	
	MMC_Wr(PUB_DTPR0_ADDR,v_pub_dtpr0);
	MMC_Wr(PUB_DTPR1_ADDR,v_pub_dtpr1);
	MMC_Wr(PUB_DTPR2_ADDR,v_pub_dtpr2);
	MMC_Wr(PUB_PTR0_ADDR,v_pub_ptr0);
	
	
	//wait PHY DLL LOCK
	while(!(MMC_Rd( PUB_PGSR_ADDR) & 1)) {}

	// configure DDR3_rst pin.
	MMC_Wr( PUB_ACIOCR_ADDR, MMC_Rd( PUB_ACIOCR_ADDR) & 0xdfffffff );
	MMC_Wr( PUB_DSGCR_ADDR,	MMC_Rd(PUB_DSGCR_ADDR) & 0xffffffef); 

  MMC_Wr( PUB_ZQ0CR1_ADDR, 0x18);
   

	//for simulation to reduce the init time.
	MMC_Wr(PUB_PTR1_ADDR,v_pub_ptr1);
	MMC_Wr(PUB_PTR2_ADDR,v_pub_ptr2);


   __udelay(10);
	//wait DDR3_ZQ_DONE: 
	while( !(MMC_Rd( PUB_PGSR_ADDR) & (1<< 2))) {}
	
 	
	// wait DDR3_PHY_INIT_WAIT : 
	while (!(MMC_Rd(PUB_PGSR_ADDR) & 1 )) {}
	

	// Monitor DFI initialization status.
	while(!(MMC_Rd(UPCTL_DFISTSTAT0_ADDR) & 1)) {} 

	MMC_Wr(UPCTL_POWCTL_ADDR, 1);
	while(!(MMC_Rd( UPCTL_POWSTAT_ADDR & 1) )) {}

	// initial upctl ddr timing.
	MMC_Wr(UPCTL_TREFI_ADDR, v_t_refi_100ns);  // 7800ns to one refresh command.
	// wr_reg UPCTL_TREFI_ADDR, 78

	MMC_Wr(UPCTL_TMRD_ADDR, v_t_mrd);
	//wr_reg UPCTL_TMRD_ADDR, 4

	MMC_Wr(UPCTL_TRFC_ADDR, v_t_rfc);	//64: 400Mhz.  86: 533Mhz. 
	// wr_reg UPCTL_TRFC_ADDR, 86

	MMC_Wr(UPCTL_TRP_ADDR, v_t_rp);	// 8 : 533Mhz.
	//wr_reg UPCTL_TRP_ADDR, 8

	MMC_Wr(UPCTL_TAL_ADDR,	v_t_al);
	//wr_reg UPCTL_TAL_ADDR, 0

//  MMC_Wr(UPCTL_TCWL_ADDR,  v_t_cl-1 + v_t_al);
  MMC_Wr(UPCTL_TCWL_ADDR,  v_t_cwl);
	//wr_reg UPCTL_TCWL_ADDR, 6

	MMC_Wr(UPCTL_TCL_ADDR, v_t_cl);	 //6: 400Mhz. 8 : 533Mhz.
	// wr_reg UPCTL_TCL_ADDR, 8

	MMC_Wr(UPCTL_TRAS_ADDR, v_t_ras); //16: 400Mhz. 20: 533Mhz.
	//  wr_reg UPCTL_TRAS_ADDR, 20 

	MMC_Wr(UPCTL_TRC_ADDR, v_t_rc);  //24 400Mhz. 28 : 533Mhz.
	//wr_reg UPCTL_TRC_ADDR, 28

	MMC_Wr(UPCTL_TRCD_ADDR, v_t_rcd);	//6: 400Mhz. 8: 533Mhz.
	// wr_reg UPCTL_TRCD_ADDR, 8

	MMC_Wr(UPCTL_TRRD_ADDR, v_t_rrd); //5: 400Mhz. 6: 533Mhz.
	//wr_reg UPCTL_TRRD_ADDR, 6

	MMC_Wr(UPCTL_TRTP_ADDR, v_t_rtp);
	// wr_reg UPCTL_TRTP_ADDR, 4

	MMC_Wr(UPCTL_TWR_ADDR, v_t_wr);
	// wr_reg UPCTL_TWR_ADDR, 8

	MMC_Wr(UPCTL_TWTR_ADDR, v_t_wtr);
	//wr_reg UPCTL_TWTR_ADDR, 4

	MMC_Wr(UPCTL_TEXSR_ADDR, v_t_exsr);
	//wr_reg UPCTL_TEXSR_ADDR, 200

	MMC_Wr(UPCTL_TXP_ADDR, v_t_xp);
	//wr_reg UPCTL_TXP_ADDR, 4

	MMC_Wr(UPCTL_TDQS_ADDR, v_t_dqs);
	// wr_reg UPCTL_TDQS_ADDR, 2

	MMC_Wr(UPCTL_TRTW_ADDR, v_t_trtw);
	//wr_reg UPCTL_TRTW_ADDR, 2

	//MMC_Wr(UPCTL_TCKSRE_ADDR, 5);
	MMC_Wr(UPCTL_TCKSRE_ADDR, v_t_cksre);
	//wr_reg UPCTL_TCKSRE_ADDR, 5 

	//MMC_Wr(UPCTL_TCKSRX_ADDR, 5);
	MMC_Wr(UPCTL_TCKSRX_ADDR, v_t_cksrx);
	//wr_reg UPCTL_TCKSRX_ADDR, 5 

	MMC_Wr(UPCTL_TMOD_ADDR, v_t_mod);
	//wr_reg UPCTL_TMOD_ADDR, 8

	//MMC_Wr(UPCTL_TCKE_ADDR, 4);
	MMC_Wr(UPCTL_TCKE_ADDR, v_t_cke);
	//wr_reg UPCTL_TCKE_ADDR, 4 

	MMC_Wr(UPCTL_TZQCS_ADDR, 64);
	//wr_reg UPCTL_TZQCS_ADDR , 64 

//	MMC_Wr(UPCTL_TZQCL_ADDR, v_t_zqcl);
	MMC_Wr(UPCTL_TZQCL_ADDR, 512);
	//wr_reg UPCTL_TZQCL_ADDR , 512 

	MMC_Wr(UPCTL_TXPDLL_ADDR, 10);
	// wr_reg UPCTL_TXPDLL_ADDR, 10  

	MMC_Wr(UPCTL_TZQCSI_ADDR, 1000);
	// wr_reg UPCTL_TZQCSI_ADDR, 1000  

	MMC_Wr(UPCTL_SCFG_ADDR, 0xf00);
	// wr_reg UPCTL_SCFG_ADDR, 0xf00 
	
 	MMC_Wr(UPCTL_LPDDR2ZQCFG_ADDR,v_odtcfg);
	MMC_Wr(UPCTL_ZQCR_ADDR,v_zqcr);
 	

 	MMC_Wr( UPCTL_SCTL_ADDR, 1);
	while (!( MMC_Rd(UPCTL_STAT_ADDR) & 1))  {
		MMC_Wr(UPCTL_SCTL_ADDR, 1);
	}
	
	//config the DFI interface.
	MMC_Wr( UPCTL_PPCFG_ADDR, (0xf0 << 1) );
	MMC_Wr( UPCTL_DFITCTRLDELAY_ADDR, 2 );
	MMC_Wr( UPCTL_DFITPHYWRDATA_ADDR,  0x1 );
	MMC_Wr( UPCTL_DFITPHYWRLAT_ADDR, v_t_cwl -1  );    //CWL -1
	MMC_Wr( UPCTL_DFITRDDATAEN_ADDR, v_t_cl - 2  );    //CL -2
	MMC_Wr( UPCTL_DFITPHYRDLAT_ADDR, 15 );
	MMC_Wr( UPCTL_DFITDRAMCLKDIS_ADDR, 2 );
	MMC_Wr( UPCTL_DFITDRAMCLKEN_ADDR, 2 );
	MMC_Wr( UPCTL_DFISTCFG0_ADDR, 0x4  );
	MMC_Wr( UPCTL_DFITCTRLUPDMIN_ADDR, 0x4000 );
	MMC_Wr( UPCTL_DFILPCFG0_ADDR, ( 1 | (7 << 4) | (1 << 8) | (10 << 12) | (12 <<16) | (1 <<24) | ( 7 << 28)));

	MMC_Wr( UPCTL_CMDTSTATEN_ADDR, 1);
	while (!(MMC_Rd(UPCTL_CMDTSTAT_ADDR) & 1 )) {}


	MMC_Wr( PUB_DTAR_ADDR, (0x0 | (0x0 <<12) | (7 << 28))); 
	
	//start trainning.
	// DDR PHY initialization 
	MMC_Wr( PUB_PIR_ADDR, 0x1e9);

	//DDR3_SDRAM_INIT_WAIT : 
	while( !(MMC_Rd(PUB_PGSR_ADDR & 1))) {}

 	MMC_Wr(UPCTL_SCTL_ADDR, 2); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4
	while ((MMC_Rd(UPCTL_STAT_ADDR) & 0x7 ) != 3 ) {}


	MMC_Wr(MMC_DDR_CTRL,v_mmc_ddr_ctrl);
	MMC_Wr(MMC_PHY_CTRL,v_mmc_phy_ctrl);
	MMC_Wr(UPCTL_PHYCR_ADDR, 2);
  MMC_Wr(MMC_REQ_CTRL, 0xff ); 
//	udelay(50);	
	//traning result
/*
  MMC_Wr(UPCTL_DLLCR0_ADDR, v_dllcr0); //284
  MMC_Wr(UPCTL_DLLCR1_ADDR, v_dllcr1); //288
  MMC_Wr(UPCTL_DLLCR2_ADDR, v_dllcr2); //28c
  MMC_Wr(UPCTL_DLLCR3_ADDR, v_dllcr3); //290
  MMC_Wr(UPCTL_DQSTR_ADDR, v_dqscr);   //2e4
  MMC_Wr(UPCTL_DQSNTR_ADDR, v_dqsntr); //2e8
  MMC_Wr(UPCTL_DQTR0_ADDR, v_tr0);     //2c0
  MMC_Wr(UPCTL_DQTR1_ADDR, v_tr1);     //2c4
  MMC_Wr(UPCTL_DQTR2_ADDR, v_tr2);     //2c8
  MMC_Wr(UPCTL_DQTR3_ADDR, v_tr3);     //2cc


	MMC_Wr(UPCTL_DLLCR9_ADDR, v_dllcr9); //2a8	
	MMC_Wr(UPCTL_IOCR_ADDR, v_iocr); //248


	MMC_Wr(UPCTL_RDGR0_ADDR,v_rdgr0); 
	MMC_Wr(UPCTL_RSLR0_ADDR,v_rslr0); 

	MMC_Wr(PUB_DX0GSR0_ADDR,v_dx0gsr0); 
	MMC_Wr(PUB_DX0GSR1_ADDR,v_dx0gsr1); 
	MMC_Wr(PUB_DX0DQSTR_ADDR,v_dx0dqstr); 
	
	MMC_Wr(PUB_DX1GSR0_ADDR,v_dx1gsr0); 
	MMC_Wr(PUB_DX1GSR1_ADDR,v_dx1gsr1); 
	MMC_Wr(PUB_DX1DQSTR_ADDR,v_dx1dqstr); 

	MMC_Wr(PUB_DX2GSR0_ADDR,v_dx2gsr0); 
	MMC_Wr(PUB_DX2GSR1_ADDR,v_dx2gsr1); 
	MMC_Wr(PUB_DX2DQSTR_ADDR,v_dx2dqstr); 

	MMC_Wr(PUB_DX3GSR0_ADDR,v_dx3gsr0); 
	MMC_Wr(PUB_DX3GSR1_ADDR,v_dx3gsr1); 
	MMC_Wr(PUB_DX3DQSTR_ADDR,v_dx3dqstr); 

	MMC_Wr(PUB_DX4GSR0_ADDR,v_dx4gsr0); 
	MMC_Wr(PUB_DX4GSR1_ADDR,v_dx4gsr1); 
	MMC_Wr(PUB_DX4DQSTR_ADDR,v_dx4dqstr); 

	MMC_Wr(PUB_DX5GSR0_ADDR,v_dx5gsr0); 
	MMC_Wr(PUB_DX5GSR1_ADDR,v_dx5gsr1); 
	MMC_Wr(PUB_DX5DQSTR_ADDR,v_dx5dqstr); 

	MMC_Wr(PUB_DX6GSR0_ADDR,v_dx6gsr0); 
	MMC_Wr(PUB_DX6GSR1_ADDR,v_dx6gsr1); 
	MMC_Wr(PUB_DX6DQSTR_ADDR,v_dx6dqstr); 

	MMC_Wr(PUB_DX7GSR0_ADDR,v_dx7gsr0); 
	MMC_Wr(PUB_DX7GSR1_ADDR,v_dx7gsr1); 
	MMC_Wr(PUB_DX7DQSTR_ADDR,v_dx7dqstr); 

	MMC_Wr(PUB_DX8GSR0_ADDR,v_dx8gsr0); 
	MMC_Wr(PUB_DX8GSR1_ADDR,v_dx8gsr1); 
	MMC_Wr(PUB_DX8DQSTR_ADDR,v_dx8dqstr); 
	*/
	return 0;
}


void init_ddr_pll(void)
{
	//reset pll
	writel(readl(P_HHI_DDR_PLL_CNTL)|(1<<29),P_HHI_DDR_PLL_CNTL);
	
	writel(v_ddr_pll_cntl2,P_HHI_DDR_PLL_CNTL2);
	writel(v_ddr_pll_cntl3,P_HHI_DDR_PLL_CNTL3);
	writel(v_ddr_pll_cntl4,P_HHI_DDR_PLL_CNTL4);
	writel(v_ddr_pll_cntl&0x7FFFFFFF,P_HHI_DDR_PLL_CNTL);
	do{
		__udelay(1000);
	}while((readl(P_HHI_DDR_PLL_CNTL)&0x80000000) == 0);
}

void enable_retention(void)
{
	unsigned v;
//	dbg_out("MCFG:",MMC_Rd(UPCTL_MCFG_ADDR));
//	dbg_out("MCFG1:",MMC_Rd(UPCTL_MCFG1_ADDR));
//   v = readl(P_AO_RTI_PULL_UP_REG);
 //  v |= (1<<12);
 //  writel(v,P_AO_RTI_PULL_UP_REG);
   writel(readl(P_AO_RTI_PIN_MUX_REG)|(1<<20),P_AO_RTI_PIN_MUX_REG);
/*	v = MMC_Rd(UPCTL_MCFG1_ADDR);
	v |= 0xd;
	MMC_Wr(UPCTL_MCFG1_ADDR,v);
	v = MMC_Rd(UPCTL_MCFG_ADDR);
	v |= (0xd<<24) | (0xd<<8);
	MMC_Wr(UPCTL_MCFG1_ADDR,v);*/
}

void disable_retention(void)
{
/*	unsigned v;
	v = MMC_Rd(UPCTL_MCFG1_ADDR);
	v &= 0xFFFFFF00;
	MMC_Wr(UPCTL_MCFG1_ADDR,v);
	v = MMC_Rd(UPCTL_MCFG_ADDR);
	v &=~((0xF<<24) | (0xF<<8));
	MMC_Wr(UPCTL_MCFG1_ADDR,v);*/

  writel(readl(P_AO_RTI_PIN_MUX_REG)&(~(1<<20)),P_AO_RTI_PIN_MUX_REG);
}
