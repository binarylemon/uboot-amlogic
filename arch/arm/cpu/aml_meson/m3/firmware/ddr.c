
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/timing.h>
#include <asm/arch/reg_addr.h>
#define DDR_RSLR_LEN 6
#define DDR_RDGR_LEN 4
#define PHYS_MEMORY_START 0x80000000
#define DDR3_2Gbx16
#define DDR_RANK  3   // 2'b11 means 2 rank.
//#define DDR3_533Mhz
////#define DDR2_400Mhz
//#ifdef DDR2_400Mhz
//#define init_pctl init_pctl_ddr2
//#else
//#define init_pctl init_pctl_ddr3
//#endif
#define NOP_CMD  0
#define PREA_CMD  1
#define REF_CMD  2
#define MRS_CMD  3
#define ZQ_SHORT_CMD 4
#define ZQ_LONG_CMD  5
#define SFT_RESET_CMD 6



extern int  ddr_phy_data_training(void);


extern void load_nop(void);
extern void load_prea(void);
extern void load_mrs(int mrs_num, int mrs_value);
extern void load_ref(void);
extern void load_zqcl(int zqcl_value);
extern void set_ddr_clock_333(void);
extern void set_ddr_clock_533(void);
void init_dmc (struct ddr_set * timing_reg);

#define APB_Wr(addr, data) WRITE_APB_REG(addr,data)
#define APB_Rd(addr) READ_APB_REG(addr)
#define Wr           WRITE_CBUS_REG
//#define PHYS_MEMORY_SIZE 0x8000000

#define DDR_MAX_WIN_LEN (DDR_RSLR_LEN*DDR_RDGR_LEN)
static unsigned ddr_start_again=1;
//#define serial_puts(a) //printf("%s",a)
//#define serial_put_hex(a,b) //printf("%08x",a)
//#define serial_put_dword(a)


#ifdef DDR2_1Gbx16
//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
#define ddr2_row_size 1 
//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
#define ddr2_col_size 2
#else
//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
#define ddr2_row_size 1 
//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
#define ddr2_col_size 2
#endif

#ifdef DDR3_2Gbx16

//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
#define ddr3_row_size 2 
//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
#define ddr3_col_size 2
#else
#define DDR3_2Gbx16
//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
#define ddr3_row_size 2 
//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
#define ddr3_col_size 2
#endif

//
#if 0

static struct ddr_set ddr3_timing={
                    .cl             =   8,
                    .t_faw          =  30,
                    .t_mrd          =   4,
                    .t_1us_pck      = 400,//533,
                    .t_100ns_pck    =  40,//53,
                    .t_init_us      = 511,
                    .t_ras          =  24,//20,
                    .t_rc           =  33,//28,
                    .t_rcd          =   9,
                    .t_refi_100ns   =  78,
                    .t_rfc          = 110,
                    .t_rp           =   9,
                    .t_rrd          =   5,
                    .t_rtp          =   5,
                    .t_wr           =  10,//8,
                    .t_wtr          =   5,
                    .t_xp           =   5,//4,
                    .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                    .t_xsnr         =   0,
                    .t_exsr         = 512,
                    .t_al           =   0,   // Additive Latency
                    .t_clr          =   8,   // cas_latency for DDR2 (nclk cycles)
                    .t_dqs          =   2,   // distance between data phases to different ranks
                    .t_cwl          =   6,
                    .t_mod          =   12,//8,
                    .t_zqcl         = 512,
                    .t_cksrx        =   7,
                    .t_cksre        =   7,
                    .t_cke          =   4,
         .ddr_pll_cntl=0x00110664,
         .ddr_ctrl= (1 << 24 ) |    //pctl_brst 4,
                    (0xff << 16) |  //reorder en for the 8 channel.
                    (0 << 15 ) |     // pctl16 mode = 0.  pctl =   32bits data pins
                    (0 << 14 ) |     // page policy = 0. 
                    (1 << 13 ) |     // command reorder enabled. 
                    (0 << 12 ) |     // bank map = 0, bank sweep between 4 banks. 
                    (0 << 11 ) |     // Block size.  0 = 32x32 bytes.  1 = 64x32 bytes.
                    (0 << 9 )  |     // ddr burst 0 = 8 burst. 1 = 4 burst.
                    (3 << 8 )  |      // ddr type.  2 = DDR2 SDRAM.  3 = DDR3 SDRAM.
                    (0 << 7 )  |     // ddr 16 bits mode.  0 = 32bits mode. 
                    (1 << 6 )  |     // 1 = 8 banks.  0 = 4 banks.
                    (0 << 4 )  |     // rank size.   0= 1 rank.   1 = 2 rank. 
                    (ddr3_row_size << 2) |  
                    (ddr3_col_size),
         .init_pctl=init_pctl_ddr3
};
#endif

void load_nop(void)
{
  APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
                         (DDR_RANK << 20) |   //rank select
                          NOP_CMD );
  while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 ) {}
}
void load_prea(void)
{
  APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
                         (DDR_RANK << 20) |   //rank select
                         PREA_CMD );
  while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 ) {}
}

void load_mrs(int mrs_num, int mrs_value)
{
  APB_Wr(PCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                   (mrs_num << 17) |
                  (mrs_value << 4) |
                           MRS_CMD );
  while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 ) {};
}

void load_ref(void )
{
  APB_Wr(PCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                          REF_CMD );
  while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 ) {}
}

void load_zqcl(int zqcl_value )
{
  APB_Wr(PCTL_MCMD_ADDR, (1 << 31) | 
                         (DDR_RANK << 20) |   //rank select
                  (zqcl_value << 4 ) |
                          ZQ_LONG_CMD );
  while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 ) {}
}

unsigned get_mrs0(struct ddr_set * timing_reg)
{
    unsigned mmc_ctrl=timing_reg->ddr_ctrl;
    unsigned ret=1<<12;
    //bl 2==4 0==8
    ret|=mmc_ctrl&(1<<10)?2:0;
    //cl
    ret|=((timing_reg->cl-4)&0x7)<<4;
    //wr: write recovery 
    ret|=((timing_reg->t_wr-4)&0x7)<<9;
    return ret&0x1fff;
}

unsigned get_mrs1(struct ddr_set * timing_reg)
{
    unsigned ret=(1<<6)|(1<<2);//rtt_nominal;      //(A9, A6, A2) 000 : disabled. 001 : RZQ/4   (A6:A2)
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

   int mrs0_value;
   int mrs1_value;
   int mrs3_value;
   int i;
//    timing_reg=ddr3_timing;
    Wr(RESET1_REGISTER,1<<3);
    //for(i=0;i<0x1000;i++);
#if 0    
     mrs0_value =           (1 << 12 ) |   // 1 fast exit from power down (tXARD), 0 slow (txARDS).
                            (4 <<9 )   |   //wr recovery   4 means write recovery = 8 cycles..
                            (0  << 8 ) |   //DLL reset.
                            (0  << 7 ) |   //0= Normal 1=Test.
                            (4  << 4 ) |   //cas latency high 3 bits (A6,A5, A4, A2) = 8 
                            (0  << 3 ) |   //burst type,  0:sequential 1 Interleave.
                            (0  << 2 ) |   //cas latency bit 0.
                                   0 ;     //burst length  :  2'b00 fixed BL8 

      mrs1_value =       (0 << 12 )  | // qoff 0=op buf enabled : 1=op buf disabled  (A12)
                         (0 << 11 )  | // rdqs enable 0=disabled : 1=enabled                (A11)
                         (0 << 7  )  |  // write leveling enable 0 = disable.               (A7) 
                  (timing_reg->t_al << 3) | //additive_latency; // 0 - 4                                 (A4-A3)

                         ( 0 << 9)   |
                         ( 0 << 6)   |
                         ( 1 << 2)   | //rtt_nominal;      //(A9, A6, A2) 000 : disabled. 001 : RZQ/4   (A6:A2)
                         ( 0 << 5)   |
                         ( 0 << 1 )  | //ocd_impedence;    (A5, A1) 00 : RZQ/6. 01: RZQ/7. 10, 11: TBD  (A5,A1)
                         ( 0 << 0 ) ;  // dll_enable;       // 0=enable : 1=disable                     (A0)

   // load_mrs(2);
   // PASR = 0 : full Array self-Refresh.   A2 ~ A0.
   // CWL cas write latency = 5     A5~A3.
   // ASR  0: Manual Self-refrsh.  1: Auto Self_refresh. A6
   // SRT.   self-Frfresh temperature range. 0 : normal operationg temperature range. A7
   // Rtt_wr A10~A9:   2'b00:   Synamic ODT off. 2'b01: RZQ/4.  2'b10: RZQ/2.  2'b11 : reserved.
   //                 CWL : 0 for 400Mhz(CWL=5).  1 for 1.876 clock period.(CWL=6) 2 :  1.5ns <= tck <1.875ns(CWL=7).
       mrs2_value =  1 << 3;

       mrs3_value = 0;
#endif
      //configure basic DDR PHY parameter.
      APB_Wr(PCTL_TOGCNT1U_ADDR, timing_reg->t_1us_pck);
      APB_Wr(PCTL_TOGCNT100N_ADDR, timing_reg->t_100ns_pck);
      APB_Wr(PCTL_TINIT_ADDR, timing_reg->t_init_us);

      // to disalbe cmd lande receiver current. 
      // we don't need receive data from command lane.
      APB_Wr(MMC_PHY_CTRL,   1 );  
//0xcc00030d
      //configure DDR PHY power down/self refresh power saving mode.
      APB_Wr(PCTL_IOCR_ADDR, 0xcc0c0ffd);
      

      if ( 1 ) {
         APB_Wr(PCTL_TRSTH_ADDR, 500);       // 500us  to hold reset high before assert CKE. change it to 50 for fast simulation time.
      } else {
         APB_Wr(PCTL_TRSTH_ADDR, 5);       
      }
      APB_Wr(PCTL_TRSTL_ADDR, 100);        //  100 clock cycles for reset low 

      while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 2)) {} // wait for dll lock
      APB_Wr(PCTL_POWCTL_ADDR, 1);            // start memory power up sequence
      while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 1)) {} // wait for memory power up
      APB_Wr(PCTL_ODTCFG_ADDR, 8);         //configure ODT
#if 1

      //ZQ calibration 
      APB_Wr(PCTL_ZQCR_ADDR, ( 1 << 31) | (0x7b <<16) );
      //wait for ZQ calibration.
      while ( APB_Rd(PCTL_ZQCR_ADDR ) & ( 1<<31) ) {}
      if ( APB_Rd(PCTL_ZQSR_ADDR) & (1 << 30) ) {
        return -1;
      } 
#else
    APB_Wr(PCTL_ZQCR_ADDR, ( 1 << 24) | (0x2277) );
#endif      

      //configure DDR3 SDRAM parameter.
      APB_Wr(PCTL_TREFI_ADDR, timing_reg->t_refi_100ns);
      APB_Wr(PCTL_TMRD_ADDR,  timing_reg->t_mrd);
      APB_Wr(PCTL_TRFC_ADDR,  timing_reg->t_rfc);
      APB_Wr(PCTL_TRP_ADDR,   timing_reg->t_rp);
      APB_Wr(PCTL_TAL_ADDR,   timing_reg->t_al);
      APB_Wr(PCTL_TCWL_ADDR,  timing_reg->t_cwl);
      APB_Wr(PCTL_TCL_ADDR,   timing_reg->cl);
      APB_Wr(PCTL_TRAS_ADDR,  timing_reg->t_ras);
      APB_Wr(PCTL_TRC_ADDR,   timing_reg->t_rc);
      APB_Wr(PCTL_TRCD_ADDR,  timing_reg->t_rcd);
      APB_Wr(PCTL_TRRD_ADDR,  timing_reg->t_rrd);
      APB_Wr(PCTL_TRTP_ADDR,  timing_reg->t_rtp);
      APB_Wr(PCTL_TWR_ADDR,   timing_reg->t_wr);
      APB_Wr(PCTL_TWTR_ADDR,  timing_reg->t_wtr);
      APB_Wr(PCTL_TEXSR_ADDR, timing_reg->t_exsr);
      APB_Wr(PCTL_TXP_ADDR,   timing_reg->t_xp);
      APB_Wr(PCTL_TDQS_ADDR,  timing_reg->t_dqs);
      APB_Wr(PCTL_TMOD_ADDR,  timing_reg->t_mod);
      APB_Wr(PCTL_TZQCL_ADDR, timing_reg->t_zqcl);
      APB_Wr(PCTL_TCKSRX_ADDR, timing_reg->t_cksrx);
      APB_Wr(PCTL_TCKSRE_ADDR, timing_reg->t_cksre);
      APB_Wr(PCTL_TCKE_ADDR,   timing_reg->t_cke);
     
       //configure the PCTL for DDR3 SDRAM burst length = 8
       
      for(i=4;(i)*timing_reg->t_rrd<timing_reg->t_faw&&i<6;i++);
      APB_Wr(PCTL_MCFG_ADDR,     1 |            // burst length 0 = 4; 1 = 8
                                (0 << 2) |     // bl8int_en.   enable bl8 interrupt function.
                                (1 << 5) |     // 1: ddr3 protocal; 0 : ddr2 protocal
                                ((i-4) <<18) |      // tFAW.
                               (1 << 17) |      // power down exit which fast exit.
                                (0xf << 8)      // 0xf cycle empty will entry power down mode.
                                        );

    APB_Wr(PCTL_PHYCR_ADDR, APB_Rd(PCTL_PHYCR_ADDR) & 0xfffffeff );   //disable auto data training when PCTL exit low power state.

      // initialize DDR3 SDRAM
        load_nop();
        load_mrs(2, get_mrs2(timing_reg));
        mrs3_value=0;
        load_mrs(3, mrs3_value);
        mrs1_value = get_mrs1(timing_reg) & 0xfffffffe; //dll enable 
        load_mrs(1, mrs1_value);
        mrs0_value = get_mrs0(timing_reg) | (1 << 8);    // dll reset.
        load_mrs(0, mrs0_value);
        load_zqcl(0);     // send ZQ calibration long command.
        return 0;

}

static inline int start_ddr_config(void)
{
    unsigned timeout = -1;
    APB_Wr(PCTL_SCTL_ADDR, 0x1);
    while((APB_Rd(PCTL_STAT_ADDR) != 0x1) && timeout)
        --timeout;

    return timeout;
}

static inline int end_ddr_config(void)
{
    unsigned timeout = 10000;
    APB_Wr(PCTL_SCTL_ADDR, 0x2);
    while((APB_Rd(PCTL_STAT_ADDR) != 0x3) && timeout)
        --timeout;

    return timeout;
}

static inline void dtu_enable(void)
{
    APB_Wr(PCTL_DTUECTL_ADDR, 0x1);  // start wr/rd
}

static  unsigned char check_dtu(void)
{
    unsigned char r_num = 0;
    volatile char *pr, *pw;
    unsigned char i;
    volatile unsigned * rd;
    volatile unsigned * wd;
    pr = (volatile char *)APB_REG_ADDR(PCTL_DTURD0_ADDR);
    pw = (volatile char *)APB_REG_ADDR(PCTL_DTUWD0_ADDR);
    for(i=0;i<16;i++)
    {
        if(*(pr+i) == *(pw+i))
            ++r_num;
    }
    rd = (volatile unsigned *)APB_REG_ADDR(PCTL_DTURD0_ADDR);
    wd = (volatile unsigned *)APB_REG_ADDR(PCTL_DTUWD0_ADDR);
    if(ddr_start_again)
    {
        serial_puts("rd=");
        for(i=0;i<4;i++)
            serial_put_hex(*rd++,32);
        serial_puts(" wd=");
        for(i=0;i<4;i++)
            serial_put_hex(*wd++,32);
    }
    return r_num;
}

static unsigned char get_best_dtu(unsigned char* p, unsigned char* best)
{
    unsigned char i;

    for(i=0;i<=DDR_MAX_WIN_LEN -DDR_RDGR_LEN +1;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) == 48)
            goto next;
    }
    return 1;

next:

    for(i=0;i<=DDR_MAX_WIN_LEN -DDR_RDGR_LEN;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) + *(p+i+3) == 64)
        {
            if(!i)
                *best = 2;
            else if(i == 8)
                *best = 9;
            else
            {
                if(*(p+i-1)>*(p+i+4))
                    *best = i + 1;
                else
                    *best = i + 2;
            }

            return 0;
        }
    }

    for(i=0;i<=DDR_MAX_WIN_LEN -DDR_RDGR_LEN +1;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) == 48)
        {
            *best = i + 1;
            return 0;
        }
    }

    return 2;
}

static void set_result(unsigned char* res)
{
    unsigned rslr0=((res[0]>>2)&3) | (((res[1]>>2)&3)<<3) | (((res[2]>>2)&3)<<6) | (((res[3]>>2)&3)<<9);
    unsigned rdgr0=(res[0]&3)      | ((res[1]&3)     <<2) | ((res[2]&3)     <<4) | ((res[3]&3)<<6);
    APB_Wr(PCTL_RSLR0_ADDR,rslr0);
    APB_Wr(PCTL_RDGR0_ADDR,rdgr0);
    
    
}

unsigned ddr_sw_trainning (struct ddr_set * ddr)
{
	unsigned char Tra[4];
	unsigned char chk[DDR_RSLR_LEN*DDR_RDGR_LEN];
	
//	writel(RESET_DDR,P_RESET1_REGISTER);
	int i,j,k;

	//Start manual data trainning
//	init_dmc(ddr_setting);
    init_pctl_ddr3(ddr);
	for (k = 0; k < (0x4); k++) {

		for (i = 0; i < DDR_RSLR_LEN; i++) {
			for (j = 0; j < DDR_RDGR_LEN; j++) {
				init_pctl_ddr3(ddr);

				if (!start_ddr_config()) {
					return 1;
				}
#if 1				
                APB_Wr(PCTL_DTUWD0_ADDR, 0xdd22ee11);
        APB_Wr(PCTL_DTUWD1_ADDR, 0x7788bb44);
        APB_Wr(PCTL_DTUWD2_ADDR, 0xdd22ee11);
        APB_Wr(PCTL_DTUWD3_ADDR, 0x7788bb44);
#else        
				// add for DTU
				APB_Wr(PCTL_DTUWD0_ADDR, 0x55aa55aa);
				APB_Wr(PCTL_DTUWD1_ADDR, 0xaa55aa55);
				APB_Wr(PCTL_DTUWD2_ADDR, 0xcc33cc33);
				APB_Wr(PCTL_DTUWD3_ADDR, 0x33cc33cc);
#endif				
				APB_Wr(PCTL_DTUWACTL_ADDR, 0);
//				    0x300 | // col addr
//						(0x7<<10) | // bank addr
//						(0x1fff <<13) | // row addr
//						(0 <<30 )); // rank addr
				APB_Wr(PCTL_DTURACTL_ADDR, 0);
//				         0x300 | // col addr
//						(0x7<<10) | // bank addr
//						(0x1fff <<13) | // row addr
//						(0 <<30 )); // rank addr

				APB_Wr(PCTL_DTUCFG_ADDR, (k << 10) | 0x01); // select byte lane, & enable DTU

				APB_Wr(PCTL_RSLR0_ADDR, i | (i << 3) | (i << 6) | (i << 9));
				APB_Wr(PCTL_RDGR0_ADDR, j | (j << 2) | (j << 4) | (j << 6));

				dtu_enable();

				if (!end_ddr_config()) {
					return 1;
				}
				if(ddr_start_again){
				    serial_puts("\ndtu windows:");
				    serial_put_hex(i * 4 + j,8);
				    
				    chk[i * DDR_RDGR_LEN + j] = check_dtu();
				    serial_puts("result windows :");
				    serial_put_hex(chk[i * 4 + j],8);
			    }else{
				    chk[i * DDR_RDGR_LEN + j] = check_dtu();
				}
			}
		}

		if (get_best_dtu(chk, &Tra[k])) {
			Tra[k]=0;
			serial_puts("\nlane");
			serial_put_hex(k,8);
				    
			serial_puts(" Fail\n");
				    
			
		}else{
			serial_puts("\nlane");
			serial_put_hex(k,8);
				    
			serial_puts(" Success");
			serial_put_hex(Tra[k],8);
			
		}

	}

    init_pctl_ddr3(ddr);
	if (!start_ddr_config()) {
    	return 1;
	}

	set_result(Tra);

	if (!end_ddr_config()) {

		return 1;
	}
	
	init_dmc(ddr);
    return 0;

}
int ddr_phy_data_traning(void)
{
    int  data_temp;

        APB_Wr(PCTL_DTUWD0_ADDR, 0xdd22ee11);
        APB_Wr(PCTL_DTUWD1_ADDR, 0x7788bb44);
        APB_Wr(PCTL_DTUWD2_ADDR, 0xdd22ee11);
        APB_Wr(PCTL_DTUWD3_ADDR, 0x7788bb44);
        APB_Wr(PCTL_DTUWACTL_ADDR, 0x300 |    // col addr
                                   (0x7<<10) |  //bank addr
                                   (0x1fff <<13) |  // row addr
                                   (0 <<30 ));    // rank addr
        APB_Wr(PCTL_DTURACTL_ADDR, 0x300 |    // col addr
                                   (0x7<<10) |  //bank addr
                                   (0x1fff <<13) |  // row addr
                                   (0 <<30 ));    // rank addr

     if (0) {

        APB_Wr(PCTL_RSLR0_ADDR, (0 << 0) |       // system additinal latency.
                                (0 << 3) |    
                                (0 << 6) |
                                (0 << 9) );

        APB_Wr(PCTL_RDGR0_ADDR, (0 << 0) |        //DQS GATing phase.
                                (0 << 2) |
                                (0 << 4) |
                                (0 << 6) );

       APB_Wr(PCTL_DLLCR0_ADDR, (APB_Rd(PCTL_DLLCR0_ADDR) & 0xfffc3fff) | 
                                (3 << 14 ));
    
       APB_Wr(PCTL_DLLCR1_ADDR, (APB_Rd(PCTL_DLLCR1_ADDR) & 0xfffc3fff) | 
                                (3 << 14 ));
    
       APB_Wr(PCTL_DLLCR2_ADDR, (APB_Rd(PCTL_DLLCR2_ADDR) & 0xfffc3fff) | 
                                (3 << 14 ));
            
       APB_Wr(PCTL_DLLCR3_ADDR, (APB_Rd(PCTL_DLLCR3_ADDR) & 0xfffc3fff) | 
                                (3 << 14 ));

       APB_Wr(PCTL_DQSTR_ADDR,  (0 << 0 ) |
                                (0 << 3 ) |
                                (0 << 6 ) |
                                (0 << 9 ));
     
       APB_Wr(PCTL_DQSNTR_ADDR, (0 << 0 ) |
                                (0 << 3 ) |
                                (0 << 6 ) |
                                (0 << 9 ));

       APB_Wr(PCTL_DQTR0_ADDR,  (0 << 0 ) |
                                (0 << 4 ) |
                                (0 << 8 ) |
                                (0 << 12 ) |
                                (0 << 16) |
                                (0 << 20) |
                                (0 << 24) |
                                (0 << 28) );

       APB_Wr(PCTL_DQTR1_ADDR,  (0 << 0 ) |
                                (0 << 4 ) |
                                (0 << 8 ) |
                                (0 << 12 ) |
                                (0 << 16) |
                                (0 << 20) |
                                (0 << 24) |
                                (0 << 28) );

       APB_Wr(PCTL_DQTR2_ADDR,  (0 << 0 ) |
                                (0 << 4 ) |
                                (0 << 8 ) |
                                (0 << 12 ) |
                                (0 << 16) |
                                (0 << 20) |
                                (0 << 24) |
                                (0 << 28) );

       APB_Wr(PCTL_DQTR3_ADDR,  (0 << 0 ) |
                                (0 << 4 ) |
                                (0 << 8 ) |
                                (0 << 12 ) |
                                (0 << 16) |
                                (0 << 20) |
                                (0 << 24) |
                                (0 << 28) );
       }
 
       // hardware build in  data training.
       APB_Wr(PCTL_PHYCR_ADDR, APB_Rd(PCTL_PHYCR_ADDR) | (1<<31));
       APB_Wr(PCTL_SCTL_ADDR, 1); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4         rddata = 1;        // init: 0, cfg: 1, cfg_req: 2, access: 3, access_req: 4, low_power: 5, low_power_enter_req: 6, low_power_exit_req: 7
       while ((APB_Rd(PCTL_STAT_ADDR) & 0x7 ) != 1 ) {}
       APB_Wr(PCTL_SCTL_ADDR, 2); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4
       data_temp = APB_Rd(PCTL_PHYCR_ADDR);
       while (data_temp & 0x80000000 ) {
          data_temp = APB_Rd(PCTL_PHYCR_ADDR);
       }  // waiting the data trainning finish.  
       data_temp = APB_Rd(PCTL_PHYSR_ADDR);
       //printf("PCTL_PHYSR_ADDR=%08x\n",data_temp& 0x00340000);
       serial_put_dword(data_temp);
       if ( data_temp & 0x00340000 ) {       // failed.
           return (1); 
       } else {
           return (0);                      //passed.
       }
} 

void init_dmc (struct ddr_set * timing_reg)
{
    APB_Wr(MMC_DDR_CTRL, timing_reg->ddr_ctrl);
    APB_Wr(MMC_REQ_CTRL, 0xff );   // 
}

//void set_ddr_clock_333(void)
//{
// int i;
//    Wr(HHI_DDR_PLL_CNTL, 0x0030067d|0x8000);
//    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
//    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
//    Wr(HHI_DDR_PLL_CNTL, 0x0030067d);
//    // wait for PLL stable.
//    for ( i = 0; i <= 1000; i ++) {          
//       APB_Wr(MMC_DDR_CTRL, timing_reg->ddr_ctrl );   // 
//   }
//   
//}
//
//void set_ddr_clock_533(void)
//{
// int i;
////    Wr(HHI_DDR_PLL_CNTL,0x00110459|0x8000);
////    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
////    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
////    Wr(HHI_DDR_PLL_CNTL,0x00110459);
//    
//    
//    Wr(HHI_DDR_PLL_CNTL, 0x0030067d |0x8000);
//    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
//    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
//    Wr(HHI_DDR_PLL_CNTL, 0x0030067d);
//    for ( i = 0; i <= 1000; i ++) {          
//       APB_Wr(MMC_DDR_CTRL, timing_reg->ddr_ctrl );   // 
//   }
////    Wr(HHI_DDR_PLL_CNTL, 0x00110459);
////    Wr(HHI_DDR_PLL_CNTL, 0x110220);//384
////    0x00310232
////    Wr(HHI_DDR_PLL_CNTL, 0x00300232);//200
//}
void set_ddr_clock(struct ddr_set * timing_reg)
{
 int i;
//    Wr(HHI_DDR_PLL_CNTL,0x00110459|0x8000);
//    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
//    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
//    Wr(HHI_DDR_PLL_CNTL,0x00110459);
    
//    timing_reg=ddr3_timing;
    Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl |0x8000);
    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
    Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl);
    for ( i = 0; i <= 1000; i ++) {          
       APB_Wr(MMC_DDR_CTRL, timing_reg->ddr_ctrl );   // 
   }
//    Wr(HHI_DDR_PLL_CNTL, 0x00110459);
//    Wr(HHI_DDR_PLL_CNTL, 0x110220);//384
//    0x00310232
//    Wr(HHI_DDR_PLL_CNTL, 0x00300232);//200
}
int testmem(unsigned i)
{
    int ret=0;
    
    if(memTestDataBus((volatile datum *) 0x80000000))
        return -1;
    if(memTestAddressBus((volatile datum *) 0x80000000, 0x20000000))return -2;
    if(i&2)
        return 0;
#ifdef CONFIG_ENABLE_MEM_DEVICE_TEST    
    if(memTestDevice((volatile datum *) 0x80000000, 0x20000000))
        return -3;
#endif    
    return 0;
}
static void m3_ddr_init_test(unsigned i)
{
    serial_put_dword(i);
    set_ddr_clock(&__ddr_setting);
    init_pctl_ddr3(&__ddr_setting);
    if(i&1)
        ddr_sw_trainning(&__ddr_setting);
    else
    {
        ddr_phy_data_traning();
        init_dmc(&__ddr_setting);
    }
    serial_puts("\nRSLR0=");
    serial_put_hex(readl(P_PCTL_RSLR0_ADDR),32);
    serial_puts(" RDGR0=");
    serial_put_dword(readl(P_PCTL_RDGR0_ADDR));
    testmem(i);
}

void ddr_init_test(void)
{
		m3_ddr_init_test(0);
}
	