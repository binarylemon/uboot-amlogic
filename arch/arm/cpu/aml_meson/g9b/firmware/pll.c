/*
 *  This file is for M8 Only . If you want to implement your pll initial function
 *  please copy this file into the $(uboot)/board/amlogic/$(board name)/firmware/ directory .
 */
#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/timing.h>
#include <asm/arch/romboot.h>
#include <asm/arch/uart.h>

static void hpll_load_en(void);
#define Write_reg_bits(reg, val, start, len) \
  Wr(reg, ((Rd(reg) & ~(((1L<<(len))-1)<<(start))) | ((unsigned int)(val) << (start))))
static unsigned int aml_read_reg32_d(unsigned int addr)
{
    unsigned int val = 0;
    val = readl(addr);
    return val;
}

unsigned long    clk_util_clk_msr(unsigned long   clk_mux)
{
	unsigned long   measured_val;
	unsigned long   uS_gate_time=64; //64us: 1024MHz, 32us:2048MHz
	unsigned long   dummy_rd;

	//set resolution
	writel((uS_gate_time-1),P_MSR_CLK_REG0);	

	//set mux and start measure
	writel(readl(P_MSR_CLK_REG0)|(clk_mux<<20) |(1<<19)|(1<<16),
		P_MSR_CLK_REG0);

	//dummy read
	{ dummy_rd = readl(P_MSR_CLK_REG0); }

	//Wait for the measurement to be done
	while( (readl(P_MSR_CLK_REG0) & (1 << 31)) ) {}

	//Disable measuring
	clrbits_le32(P_MSR_CLK_REG0, 1<<16 );

	measured_val = readl(P_MSR_CLK_REG2)&0xFFFFF; //20bit length
	    
    //Return Mhz, 64us:>>6,32us:>>5
    return (measured_val>>6);
}


static void wait_clock(unsigned clk,unsigned dest)
{
	char * pszCLK[] = {
		NULL,NULL,NULL,"A9_ring_osc",NULL,NULL,
		"VID PLL","CLK81","cts_encp","cts_encl",NULL,"ETH RMII",NULL,
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
	Wr_cbus(AM_ANALOG_TOP_REG1, Rd_cbus(AM_ANALOG_TOP_REG1)|1);

	//bandgap enable
	//SYS,MPLL
	Wr_cbus(HHI_MPLL_CNTL6, Rd_cbus(HHI_MPLL_CNTL6)|(1<<26));
	//VID,HDMI
	Wr_cbus(HHI_HDMI_PLL_CNTL5, Rd_cbus(HHI_HDMI_PLL_CNTL5)|(1<<30));
	//DDR
	writel(readl(0xc8000410)|(1<<12),0xc8000410);

	//MUST	
	//switch a9 clock to  oscillator in the first.  This is sync mux.
	Wr_cbus( HHI_A9_CLK_CNTL, Rd_cbus(HHI_A9_CLK_CNTL) & (~(1<<7)));
	__udelay(10);
	//Wr_cbus( HHI_A9_CLK_CNTL, 0);
	__udelay(10);
	
	//???
	Wr_cbus(HHI_MPEG_CLK_CNTL, Rd_cbus(HHI_MPEG_CLK_CNTL) & (~(1<<8)) );
	__udelay(100);

	serial_init(52|UART_CNTL_MASK_TX_EN|UART_CNTL_MASK_RX_EN);	//clk81 switch to 24M before MPLL init

	//PLL setup: bandgap enable -> 1ms delay -> PLL reset + PLL set ->
	//                 PLL enable -> 1ms delay -> PLL release from reset

	int n_pll_try_times = 0;

	if(readl(0xd9040004) != 0x18d7) //temp solution, for rev-A & rev-B compatible
	{
		//SYS PLL init
		do{
			//BANDGAP reset for SYS_PLL,MPLL lock fail		
			Wr_reg_bits(HHI_MPLL_CNTL6,0,26,1);
			__udelay(10);
			Wr_reg_bits(HHI_MPLL_CNTL6,1,26,1);
			__udelay(1000); //1ms for bandgap bootup

			PLL_ENTER_RESET_BIT29(HHI_SYS_PLL_CNTL);
			Wr_cbus(HHI_SYS_PLL_CNTL2,CFG_SYS_PLL_CNTL_2);
			Wr_cbus(HHI_SYS_PLL_CNTL3,CFG_SYS_PLL_CNTL_3);
			Wr_cbus(HHI_SYS_PLL_CNTL4,CFG_SYS_PLL_CNTL_4);
			Wr_cbus(HHI_MPLL_CNTL4, Rd_cbus(HHI_MPLL_CNTL4)|(0<<3));
			Wr_cbus(HHI_SYS_PLL_CNTL5,CFG_SYS_PLL_CNTL_5);
			PLL_SETUP(HHI_SYS_PLL_CNTL, plls->sys_pll_cntl);
			__udelay(200);
			PLL_RELEASE_RESET_BIT29(HHI_SYS_PLL_CNTL);
			__udelay(200);
			Wr_cbus(HHI_MPLL_CNTL4, Rd_cbus(HHI_MPLL_CNTL4)|(1<<3));

			PLL_LOCK_CHECK(n_pll_try_times,1);

		}while((Rd_cbus(HHI_SYS_PLL_CNTL)&(1<<31))==0);

		//A9 clock setting
		Wr_cbus(HHI_A9_CLK_CNTL,(plls->sys_clk_cntl & (~(1<<7))));
		__udelay(1);
		//enable A9 clock
		Wr_cbus(HHI_A9_CLK_CNTL,(plls->sys_clk_cntl | (1<<7)));
	}
	
	//MPLL init
	//FIXED PLL/Multi-phase PLL, fixed to 2.55GHz
	Wr_cbus(HHI_MPLL_CNTL3, 0x00010007);
	PLL_ENTER_RESET_BIT29(HHI_MPLL_CNTL);	//set reset bit to 1
	__udelay(200);
	Wr_cbus(HHI_MPLL_CNTL2, CFG_MPLL_CNTL_2 );
	Wr_cbus(HHI_MPLL_CNTL3, CFG_MPLL_CNTL_3 );
	Wr_cbus(HHI_MPLL_CNTL4, CFG_MPLL_CNTL_4 );
	Wr_cbus(HHI_MPLL_CNTL5, CFG_MPLL_CNTL_5 );
	Wr_cbus(HHI_MPLL_CNTL6, CFG_MPLL_CNTL_6 );
	Wr_cbus(HHI_MPLL_CNTL7, CFG_MPLL_CNTL_7 );
	Wr_cbus(HHI_MPLL_CNTL8, CFG_MPLL_CNTL_8 );
	Wr_cbus(HHI_MPLL_CNTL9, CFG_MPLL_CNTL_9 );
	PLL_SETUP(HHI_MPLL_CNTL, plls->mpll_cntl);	//2.55G, FIXED
	PLL_RELEASE_RESET_BIT29(HHI_MPLL_CNTL);	//set reset bit to 0

	__udelay(800);
	Wr_cbus(HHI_MPLL_CNTL4, Rd_cbus(HHI_MPLL_CNTL4)|(1<<14));
	//PLL_WAIT_FOR_LOCK(HHI_MPLL_CNTL); //need bandgap reset?
	n_pll_try_times=0;
	do{
		PLL_LOCK_CHECK(n_pll_try_times,5);
		if((Rd_cbus(HHI_MPLL_CNTL)&(1<<31))!=0)
			break;
		Wr_cbus(HHI_MPLL_CNTL,Rd_cbus(HHI_MPLL_CNTL) | (1<<29));
		__udelay(1000);
		PLL_RELEASE_RESET_BIT29(HHI_MPLL_CNTL);
		__udelay(1000);
	}while((Rd_cbus(HHI_MPLL_CNTL)&(1<<31))==0);

	//MPLL is fixed to 2.55GHz
	//clk81=fclk_div4 /2=637.5/4=159.375M
	Wr_cbus(HHI_MPEG_CLK_CNTL, plls->mpeg_clk_cntl);

	//here need do UART init, print MPLL LOCK CHECK info
	serial_init(__plls.uart);

	n_pll_try_times=0;
	//HDMI PLL init
	do{
		writel(plls->vid_pll_cntl, P_HHI_HDMI_PLL_CNTL);
		writel(CFG_HDMI_PLL_CNTL_2, P_HHI_HDMI_PLL_CNTL2);
		writel(CFG_HDMI_PLL_CNTL_3, P_HHI_HDMI_PLL_CNTL3);
		writel(CFG_HDMI_PLL_CNTL_4, P_HHI_HDMI_PLL_CNTL4);
		writel(CFG_HDMI_PLL_CNTL_5, P_HHI_HDMI_PLL_CNTL5);
		writel(CFG_HDMI_PLL_CNTL_6, P_HHI_HDMI_PLL_CNTL6);
		__udelay(10);
		writel((readl(P_HHI_HDMI_PLL_CNTL)&(~(1<<28))), P_HHI_HDMI_PLL_CNTL);
		__udelay(10);
		PLL_LOCK_CHECK(n_pll_try_times,3);
	}while((readl(P_HHI_HDMI_PLL_CNTL)&(1<<31))==0);

	__udelay(10);
}

static void hpll_load_en(void)
{
	Write_reg_bits(P_HHI_VID_CLK_CNTL,1,19,1);
	__udelay(1);
	Write_reg_bits(P_HHI_VID_CLK_CNTL,7,0,3);
	__udelay(1);
	Write_reg_bits(P_HHI_VID_CLK_CNTL,1,16,3);
	__udelay(1);
	writel(0x1, P_ENCL_VIDEO_EN);
	__udelay(100);
	writel(0x0, P_ENCL_VIDEO_EN);
	__udelay(100);
	Write_reg_bits(P_HHI_VID_CLK_CNTL,0,16,3);
    __udelay(10);
}
