#include <config.h>
#include <asm/arch/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/timing.h>
#include <asm/arch/uart.h>
#include <ddr_types.h>


#ifndef FIRMWARE_IN_ONE_FILE
#define STATIC_PREFIX_DATA
#else
#define STATIC_PREFIX_DATA static
#endif

static int init_pctl_ddr3(struct ddr_set * ddr_setting);

#if defined(M6_DDR3_1GB)
	#define DDR3_4Gbx16
#elif defined(M6_DDR3_512M)
	#define DDR3_2Gbx16
#else
	#error "Please define DDR3 memory capacity first in file aml_tv_m2c.h!\n"
#endif


#ifdef DDR3_2Gbx16
	//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
	#define ddr3_row_size 2 
	//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
	#define ddr3_col_size 2
#elif defined DDR3_4Gbx16
	//row_size 00 : A0~A15.  01 : A0~A12, 10 : A0~A13, 11 : A0~A14. 
	#define ddr3_row_size 3
	//col size 00 : A0~A7,   01 : A0~A8, 10: A0 ~A9.  11, A0~A9, A11. 
	#define ddr3_col_size 2
#endif

static struct ddr_set __ddr_setting={

                    .cl             =   9,
                    .t_faw          =  30,   //8bit:20, 16bit:30
                    .t_mrd          =   4,
                    .t_1us_pck      = 500,
                    .t_100ns_pck    =  50,
                    .t_init_us      = 512,
                    .t_rsth_us      = 500,  // 0 for ddr2;  2 for simulation; 500 for ddr3.
                    .t_ras          =  24,
                    .t_rc           =  33,
                    .t_rcd          =   9,
                    .t_refi_100ns   =  78,
                    .t_rfc          =  139,   //4Gb:139, 2Gb:86, 1Gb:59
                    .t_rp           =   9,
                    .t_rrd          =   5,   //8bit:4, 16bit:6
                    .t_rtp          =   4,
                    .t_wr           =   8,
                    .t_wtr          =   4,
                    .t_xp           =   4,
                    .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                    .t_xsnr         =   0,
                    .t_exsr         = 512,
                    .t_al           =   0,   // Additive Latency
                    .t_clr          =   8,   // cas_latency for DDR2 (nclk cycles)
                    .t_dqs          =   2,   // distance between data phases to different ranks
                    .t_cwl          =   6,
                    .t_mod          =  12,
                    .t_zqcl         = 512,
                    .t_rtw          =   2,
                    .t_cksrx        =   6,
                    .t_cksre        =   6,
                    .t_cke          =   4,
                    .mrs={  [0]=(1 << 12) |   // 1 fast exit from power down (tXARD), 0 slow (txARDS).
                    			(4 <<  9) |   //wr recovery. It will be calcualted by get_mrs0()@ddr_init_pctl.c
                    			(0 <<  8) |   //DLL reset.
                    			(0 <<  7) |   //0= Normal 1=Test.
                    			(5 <<  4) |   //cas latency high 3 bits (A6,A5, A4, A2=0).
                    						  //It will be calcualted by get_mrs0()@ddr_init_pctl.c
                    			(0 << 3 ) |   //burst type,  0:sequential; 1:Interleave.
                    			(0 << 2 ) |   //cas latency bit 0.
								(0 << 0 ),    //burst length	:  2'b00 fixed BL8
                    			                    						      
                            [1]=(0 << 9)|(0 << 6)|(1 << 2)|	//RTT (A9,A6,A2) RZQ/4(001) = 60
                                (0 << 5)|(1 << 1) |			//DIC(M5, M1) RZQ/7(01) = 34
                                (0 <<3 ),					//AL: It will be calcualted by get_mrs1()@ddr_init_pctl.c
                                
                                                                	
                            [2]=0,	//CWL:(A5,A4,A3)000 = 5 (tCK = 2.5ns) 
                            		//001 = 6 (2.5ns > tCK = 1.875ns)
                            		//010 = 7 (1.875ns > tCK = 1.5ns)
                            		//011 = 8 (1.5ns > tCK = 1.25ns)
                            [3]=0
                        },
                    .mcfg = {  1 |				// burst length: 0 for 4; 1 for 8
                    		  (0 << 2) |		// bl8int_en.   enable bl8 interrupt function.Only valid for DDR2
                    		  					// and is ignored for mDDR/LPDDR2 and DDR3
                              (1 << 5) |      	// 1: ddr3 protocal; 0 : ddr2 protocal
                              //(1 << 3) |    	//2T mode, default is disable
                              //(tFAW <<18) | 	//tFAW will be set according to the calculation with t_rrd and t_faw
                                              	// in file /firmware/ddr_init_pctl.c
                              (1 << 17) |     	// power down exit which fast exit.
                              (0 << 8)      	// 0xf cycle empty will entry power down mode.
                           },
                    .zqcr  = (( 1 << 24) | 0x11dd),   //0x11dd->22 ohm;0x1155->0 ohm
         .ddr_pll_cntl=0x1067d, //500MHz
         .ddr_clk=500,
         .ddr_ctrl= (0 << 24 ) |    //pctl_brst 4,
                    (0xff << 16) |  //reorder en for the 8 channel.
                    (0 << 15 ) |     // pctl16 mode = 0.  pctl =   32bits data pins
                    (0 << 14 ) |     // page policy = 0.
                    (1 << 13 ) |     // command reorder enabled.
                    (0 << 12 ) |     // bank map = 0, bank sweep between 4 banks.
                    (0 << 11 ) |     // Block size.  0 = 32x32 bytes.  1 = 64x32 bytes.
                    (0 << 10 )  |     // ddr burst 0 = 8 burst. 1 = 4 burst.
                    (3 << 8 )  |      // ddr type.  2 = DDR2 SDRAM.  3 = DDR3 SDRAM.
                    (0 << 7 )  |     // ddr 16 bits mode.  0 = 32bits mode.
                    (1 << 6 )  |     // 1 = 8 banks.  0 = 4 banks.
                    (0 << 4 )  |     // rank size.   0= 1 rank.   1 = 2 rank.
                    (ddr3_row_size << 2) |
                    (ddr3_col_size),
         .init_pctl=init_pctl_ddr3        
};

STATIC_PREFIX_DATA struct pll_clk_settings __plls __attribute__((section(".setting")))
={
    .sys_pll_cntl=0x232,//1200M
    .sys_clk_cntl=(1<<0)    |  //select sys pll for sys cpu
	        (0<<2)    |  // divided 2
	        (1<<4)    |  //APB_en
	        (1<<5)    |  //AT en
	        (1<<7)    |(1<<8),  // send to sys cpu
    .other_pll_cntl=0x00000219,//0x19*24/1=600M
    .mpeg_clk_cntl= (1 << 12) |                     // select other PLL
                   ((3- 1) << 0 ) |    // div1
                    (1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
                    (1 << 8 ) |(1<<15),                    // Connect clk81 to the PLL divider output
    .demod_pll400m_cntl=(1<<9)  | //n 1200=xtal*m/n 
            (50<<0),    //m 50*24
    .clk81=200000000,   //750/4=180M
    .a9_clk=1200000000/2,
    .spi_setting=0xea949,
    .nfc_cfg=(((0)&0xf)<<10) | (0<<9) | (0<<5) | 5,
    .sdio_cmd_clk_divide=5,
    .sdio_time_short=(250*180000)/(2*(12)),
    .uart=
        (200000000/(CONFIG_BAUDRATE*4) -1)
        | UART_STP_BIT 
        | UART_PRTY_BIT
        | UART_CHAR_LEN 
        //please do not remove these setting , jerry.yu
        | UART_CNTL_MASK_TX_EN
        | UART_CNTL_MASK_RX_EN
        | UART_CNTL_MASK_RST_TX
        | UART_CNTL_MASK_RST_RX
        | UART_CNTL_MASK_CLR_ERR,
};
#define DEBUGROM_CMD_BUF_SIZE ((0x1b0-0xc0-sizeof(__ddr_setting)-sizeof(__plls))&~0x1f)

STATIC_PREFIX_DATA char init_script[DEBUGROM_CMD_BUF_SIZE] __attribute__((section(".setting")))
="r c1107d54";
