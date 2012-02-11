/*
 *  This file is for M1 Only . If you want to implement your pll initial function 
 *  please copy this file into the arch/$(ARCH)/$(CPU)/$SOC/firmware directory .
 */
#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/timing.h>
#include <asm/arch/romboot.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>

unsigned long    clk_util_clk_msr(unsigned long   clk_mux);

static void wait_pll(unsigned clk,unsigned dest)
{
    unsigned cur;
    do{
        cur=clk_util_clk_msr(clk);
		
        //asm volatile ("wfi");		
		
        serial_puts("wait pll-0x");
        serial_put_hex(clk,8);
        serial_puts(" target is ");
        serial_put_hex(dest,16);
        serial_puts(" now it is ");
        serial_put_dword(cur);
        __udelay(100);
    }while(cur<dest-1 || cur >dest+1);
}

SPL_STATIC_FUNC void pll_init(struct pll_clk_settings * plls) 
{

	/**
	 * Enable PLLs pins
	 */
	//*P_AM_ANALOG_TOP_REG1 |= 0x1; // Enable DDR_PLL enable pin
	//*P_HHI_MPLL_CNTL5	  |= 0x1; // Enable Both MPLL and SYS_PLL enable pin  
//	0xc11081bc
	writel(readl(0xc11081bc)|1,0xc11081bc);
	
	/**
	 * SYS_PLL setting:
	 * 24MHz: [30]:PD=0, [29]:RESET=0, [17:16]OD=1, [13:9]N=1, [8:0]M=50, PLL_FOUT= (24*(50/1))/2 = 600MHz
	 * 25MHz: [30]:PD=0, [29]:RESET=0, [17:16]OD=1, [13:9]N=1, [8:0]M=48, PLL_FOUT= (25*(48/1))/2 = 600MHz
	 */
	 #if 0
	*P_HHI_SYS_PLL_CNTL = (1 << 29); // RESET
	if (crystal)
		sys_pll_cntl = (1 << 16) | (1 << 9) | (50 << 0);
	else
		sys_pll_cntl = (1 << 16) | (1 << 9) | (48 << 0);
	*P_HHI_SYS_PLL_CNTL2 = 0x814d3928;
	*P_HHI_SYS_PLL_CNTL3 = 0x6b425012;
	*P_HHI_SYS_PLL_CNTL4 = 0x101;
	*P_HHI_SYS_PLL_CNTL = sys_pll_cntl; // TRUE ENABLE
	#endif
	writel((1<<29), 0xc1104260); //reset
	writel(0x814d3928,0xc1104264);//P_HHI_SYS_PLL_CNTL2
	writel(0x6b425012,0xc1104268);//P_HHI_SYS_PLL_CNTL3
	writel(0x101,0xc110426c);//P_HHI_SYS_PLL_CNTL4
	writel(((1 << 16) | (1 << 9) | (50 << 0)),0xc1104260); //SYS pll clk: 600M
	
	writel(plls->sys_clk_cntl,P_HHI_A9_CLK_CNTL); //300M
	
	///clk81=200M
	writel((7 << 12) | // 0:socin 1:ddr_pll 2:mp0_clko 3:mp1_clko 4:mp2_clko 5:fclk_div2 6:fclk_div3 7:fclk_div5
                               (1 << 8)  | // 0:oscin 1:pll
                               (1 << 7)  | // 0:oscin 1:div enable
                               (1 << 0),P_HHI_MPEG_CLK_CNTL);   // 0:div1, n:div(n+1)
		//**************************************************************//

 	__udelay(1000);
	
}
SPL_STATIC_FUNC void ddr_pll_init(struct ddr_set * ddr_setting) 
{
    writel(ddr_setting->ddr_pll_cntl,P_HHI_DDR_PLL_CNTL);    
}

unsigned long    clk_util_clk_msr(unsigned long   clk_mux)
{
	
    unsigned long   measured_val;
    unsigned long   uS_gate_time=64;
    unsigned long dummy_rd;
    writel(0,P_MSR_CLK_REG0);
    // Set the measurement gate to 64uS
    clrsetbits_le32(P_MSR_CLK_REG0,0xffff,(uS_gate_time-1));
    // Disable continuous measurement
    // disable interrupts
    clrbits_le32(P_MSR_CLK_REG0,(1 << 18) | (1 << 17));
//    Wr(MSR_CLK_REG0, (Rd(MSR_CLK_REG0) & ~((1 << 18) | (1 << 17))) );
    clrsetbits_le32(P_MSR_CLK_REG0,
        (0xf << 20)|(1<<19)|(1<<16),
        (clk_mux<<20) |(1<<19)|(1<<16));
    { dummy_rd = readl(P_MSR_CLK_REG0); }
    // Wait for the measurement to be done
    while( (readl(P_MSR_CLK_REG0) & (1 << 31)) ) {} 
    // disable measuring
    clrbits_le32(P_MSR_CLK_REG0, 1<<16 );

    measured_val = readl(P_MSR_CLK_REG2)&0x000FFFFF;
    // Return value in Hz*measured_val
    // Return Mhz
    return (measured_val>>6);
    // Return value in Hz*measured_val
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
