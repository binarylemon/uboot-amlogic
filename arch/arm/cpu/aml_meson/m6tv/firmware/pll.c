/*
 *  This file is for M6TV Only . If you want to implement your pll initial function
 *  please copy this file into the arch/$(ARCH)/$(CPU)/$SOC/firmware directory .
 */
#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/timing.h>
#include <asm/arch/romboot.h>
#include <asm/arch/uart.h>

#ifdef CONFIG_ENABLE_WATCHDOG
	#define PLL_TIMES 4
	int pll_times=0;
#endif

unsigned long    clk_util_clk_msr(unsigned long   clk_mux)
{

	unsigned long   measured_val;
	unsigned long   uS_gate_time=64;
	unsigned long   dummy_rd;

	//Clear all msr first
	writel(0,P_MSR_CLK_REG0);

	//Set the measurement gate to 64uS
	setbits_le32(P_MSR_CLK_REG0,(uS_gate_time-1));

	setbits_le32(P_MSR_CLK_REG0,(clk_mux<<20) |(1<<19)|(1<<16));

	//dummy read
	{ dummy_rd = readl(P_MSR_CLK_REG0); }

	//Wait for the measurement to be done
	while( (readl(P_MSR_CLK_REG0) & (1 << 31)) ) {}

	//Disable measuring
	clrbits_le32(P_MSR_CLK_REG0, 1<<16 );

	measured_val = readl(P_MSR_CLK_REG2)&0x000FFFFF;
	    
    //Return Mhz
    return (measured_val>>6);
}


static void wait_clock(unsigned clk,unsigned dest)
{
	char * pszCLK[] = {
		NULL,NULL,NULL,"DDR PLL","USB0 CLK","USB1 CLK",
		"VID PLL","CLK81",NULL,NULL,NULL,"ETH RMII","VID2 PLL",
		};
    unsigned cur;
    do{
        cur=clk_util_clk_msr(clk);
		serial_puts("\nSet [");
		if(clk < (sizeof(pszCLK)/sizeof(pszCLK[0])) &&
			pszCLK[clk])
			serial_puts(pszCLK[clk]);
		else
			serial_puts("N/A");
		
		serial_puts("] to ");
        serial_put_dec(dest);
        serial_puts("MHz now it is ");

		//tolerance +/- 1
		if((cur == dest-1) || (cur == dest+1))
			serial_put_dec(dest);
		else
			serial_put_dec(cur);
		
		serial_puts("MHz");
        __udelay(100);
    }while(cur<dest-1 || cur >dest+1);

	serial_puts(" --> OK!\n");
	
}

SPL_STATIC_FUNC void pll_init(struct pll_clk_settings * plls)
{

	//Enable PLLs pins
	//*P_AM_ANALOG_TOP_REG1 |= 0x1; // Enable DDR_PLL enable pin	
	//#define AM_ANALOG_TOP_REG1    0x206F  ->  0xC11081BC 	
	Wr(AM_ANALOG_TOP_REG1, Rd(AM_ANALOG_TOP_REG1)|1);

	//*P_HHI_MPLL_CNTL5   |= 0x1; // Enable Both MPLL and SYS_PLL enable pin
	//move to following SYS PLL init

	//switch a9 clock to  oscillator in the first.  This is sync mux.
    Wr( HHI_A9_CLK_CNTL, 0);
	__udelay(100);


	do{
		//BANDGAP reset for SYS_PLL,VIID_PLL,MPLL lock fail
		//Note: once SYS PLL is up, there is no need to
		//          use AM_ANALOG_TOP_REG1 for VIID, MPLL
		//          lock fail
		Wr_reg_bits(HHI_MPLL_CNTL5,0,0,1);
		__udelay(10);
		Wr_reg_bits(HHI_MPLL_CNTL5,1,0,1);
		__udelay(1000); //1ms for bandgap bootup

		M6_PLL_RESET(HHI_SYS_PLL_CNTL);
		Wr(HHI_SYS_PLL_CNTL2,M6_SYS_PLL_CNTL_2);
		Wr(HHI_SYS_PLL_CNTL3,M6_SYS_PLL_CNTL_3);
		Wr(HHI_SYS_PLL_CNTL4,M6_SYS_PLL_CNTL_4);
		Wr(HHI_SYS_PLL_CNTL, plls->sys_pll_cntl);
		//M6_PLL_WAIT_FOR_LOCK(HHI_SYS_PLL_CNTL);

		__udelay(500); //wait 100us for PLL lock
		
#ifdef CONFIG_ENABLE_WATCHDOG
		pll_times++;
		serial_puts("pll_times:");
		serial_put_dword(pll_times);
		if(pll_times>PLL_TIMES){
			serial_puts(__FILE__);
			serial_puts(__FUNCTION__);
			serial_put_dword(__LINE__);
			writel((1<<22) | (3<<24)|1000, P_WATCHDOG_TC);
		}
#endif
	}while((Rd(HHI_SYS_PLL_CNTL)&0x80000000)==0);

	//A9 clock setting
	Wr(HHI_A9_CLK_CNTL,(plls->sys_clk_cntl & (~(1<<7))));
	__udelay(1);
	//enable A9 clock
	Wr(HHI_A9_CLK_CNTL,(plls->sys_clk_cntl | (1<<7)));

	//VIID PLL
	M6_PLL_RESET(HHI_VIID_PLL_CNTL);
	Wr(HHI_VIID_PLL_CNTL2, M6_VIID_PLL_CNTL_2 );
	Wr(HHI_VIID_PLL_CNTL3, M6_VIID_PLL_CNTL_3 );
	Wr(HHI_VIID_PLL_CNTL4, M6_VIID_PLL_CNTL_4 );
	Wr(HHI_VIID_PLL_CNTL,  0x20242 );
	M6_PLL_WAIT_FOR_LOCK(HHI_VIID_PLL_CNTL);

	//FIXED PLL/Multi-phase PLL, fixed to 2GHz
	M6_PLL_RESET(HHI_MPLL_CNTL);
	Wr(HHI_MPLL_CNTL2, M6_MPLL_CNTL_2 );
	Wr(HHI_MPLL_CNTL3, M6_MPLL_CNTL_3 );
	Wr(HHI_MPLL_CNTL4, M6_MPLL_CNTL_4 );
	Wr(HHI_MPLL_CNTL5, M6_MPLL_CNTL_5 );
	Wr(HHI_MPLL_CNTL6, M6_MPLL_CNTL_6 );
	Wr(HHI_MPLL_CNTL7, M6_MPLL_CNTL_7 );
	Wr(HHI_MPLL_CNTL8, M6_MPLL_CNTL_8 );
	Wr(HHI_MPLL_CNTL9, M6_MPLL_CNTL_9 );
	Wr(HHI_MPLL_CNTL10,M6_MPLL_CNTL_10);
	Wr(HHI_MPLL_CNTL, 0x67d );
	M6_PLL_WAIT_FOR_LOCK(HHI_MPLL_CNTL);

	//clk81=fclk_div5 /2=400/2=200M
	Wr(HHI_MPEG_CLK_CNTL, plls->mpeg_clk_cntl );
#ifdef CONFIG_ENABLE_WATCHDOG
		pll_times=0;
#endif

	//VID PLL
	do{
		//BANDGAP reset for VID_PLL,DDR_PLL lock fail
		//Note: once VID PLL is up, there is no need to
		//          use AM_ANALOG_TOP_REG1 for DDR PLL
		//          lock fail
		Wr_reg_bits(AM_ANALOG_TOP_REG1,0,0,1);
		__udelay(10);
		Wr_reg_bits(AM_ANALOG_TOP_REG1,1,0,1);
		__udelay(1000); //1ms for bandgap bootup

		M6_PLL_RESET(HHI_VID_PLL_CNTL);
		Wr(HHI_VID_PLL_CNTL2, M6_VID_PLL_CNTL_2 );
		Wr(HHI_VID_PLL_CNTL3, M6_VID_PLL_CNTL_3 );
		Wr(HHI_VID_PLL_CNTL4, M6_VID_PLL_CNTL_4 );
		Wr(HHI_VID_PLL_CNTL,  0xb0442 );
		//M6_PLL_WAIT_FOR_LOCK(HHI_VID_PLL_CNTL);

		__udelay(500); //wait 100us for PLL lock
#ifdef CONFIG_ENABLE_WATCHDOG
		pll_times++;
		serial_puts("pll_times:");
		serial_put_dword(pll_times);
		if(pll_times>PLL_TIMES){
			serial_puts(__FILE__);
			serial_puts(__FUNCTION__);
			serial_put_dword(__LINE__);
			writel((1<<22) | (3<<24)|1000, P_WATCHDOG_TC);
		}
#endif
	}while((Rd(HHI_VID_PLL_CNTL)&0x80000000)==0);

 	__udelay(100);

}



STATIC_PREFIX void pll_clk_list(void)
{


    unsigned long   clk_freq;
    unsigned char clk_list[]={3,10,11};
    char *clk_list_name[]={"arm","ddr","other"};
    unsigned long  i;
	for(i=0;i<3;i++)
	{
		clk_freq = clk_util_clk_msr(clk_list[i]     // unsigned long   clk_mux,             // Measure A9 clock
								);   // unsigned long   uS_gate_time )       // 50us gate time
		serial_puts(clk_list_name[i]);
		serial_puts("_clk=");
		serial_put_dword(clk_freq);
		serial_puts("\n");
	}
}
#define CONFIG_AML_CLK_LIST_ENABLE 1
