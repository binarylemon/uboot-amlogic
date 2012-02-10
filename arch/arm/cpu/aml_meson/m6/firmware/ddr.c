
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/timing.h>
#include <asm/arch/reg_addr.h>
#define DDR_RSLR_LEN 3
#define DDR_RDGR_LEN 4
#define DDR_MAX_WIN_LEN (DDR_RSLR_LEN*DDR_RDGR_LEN)
#define PHYS_MEMORY_START 0x80000000

#define DDR_RANK  3   // 2'b11 means 2 rank.
#define DDR_LANE  2

#define RESET_MMC_FOR_DTU_TEST

#define NOP_CMD  0
#define PREA_CMD  1
#define REF_CMD  2
#define MRS_CMD  3
#define ZQ_SHORT_CMD 4
#define ZQ_LONG_CMD  5
#define SFT_RESET_CMD 6


static unsigned ddr_start_again=1;

#define APB_Wr(addr, data) WRITE_APB_REG(addr,data)
#define APB_Rd(addr) READ_APB_REG(addr)
#define Wr           WRITE_CBUS_REG
#define Rd           READ_CBUS_REG

//#include "ddr_init_sw.c"
#include "ddr_init_hw.c"
#include "ddr_init_pctl.c"

static void wait_pll(unsigned clk,unsigned dest);

void set_ddr_clock(struct ddr_set * timing_reg)
{
	/*
#ifdef ENABLE_POWER_SAVING
    APB_Wr(PCTL_DLLCR_ADDR, APB_Rd(PCTL_DLLCR_ADDR)|(7<<2));
#endif
	*/
    //set_ddr_clock.
    //  +------------------------------------------------------------+   +----------------------------------------+
    //  |                      <<< PLL >>>                           |   |         <<< Clock Reset Test >>>       |
    //  +---------+-----------+----------------------+---------------+   +------+-----------+---------------------+  +------------
    //  |         | PLL Ports |      PLL        PLL  | PLL  PLL      |   | CRT  |    Final  |               Ideal |  |  HIU REG 
    //  |   Fin   |  N    M   |     Fref        VCO  |  OD  FOUT     |   |  XD  |    Clock  |    Error      Clock |  |  0x1068
    //  +---------+-----------+----------------------+---------------+   +------+-----------+---------------------+  +------------
    //  | 24.0000 |  3   133  |   8.0000  1064.0000  |   1  532.0000 |   |   1  |  532.0000 |  -0.188% ( 533.000) |  | 0x00010685 
    //  | 24.0000 |  3   125  |   8.0000  1000.0000  |   1  500.0000 |   |   1  |  500.0000 |   0.000% ( 500.000) |  | 0x0001067d
    Wr(HHI_DDR_PLL_CNTL, (1<<29) );
	
	Wr(HHI_DDR_PLL_CNTL2, 0x814d3928 );
	Wr(HHI_DDR_PLL_CNTL3, 0x6b425012 );
	//Wr(HHI_DDR_PLL_CNTL4, 0x101 ); 
	Wr(HHI_DDR_PLL_CNTL4, 0x110 ); //shi suggest PM14:23
	
	//Wr(HHI_DDR_PLL_CNTL, 0x0001067d);//500M
	//Wr(HHI_DDR_PLL_CNTL, 0x00010664);//400M
	Wr(HHI_DDR_PLL_CNTL, 0x0002067d);//250M
	
	
  	//wait to DDR PLL lock.
   	while (!(MMC_Rd(MMC_CLK_CNTL) & (1<<29)) ) {}

  	//Enable DDR DLL clock input from PLL.
    MMC_Wr(MMC_CLK_CNTL, 0xc0000080);  //  @@@ select the final mux from PLL output directly.
    MMC_Wr(MMC_CLK_CNTL, 0xc00000c0);

    //enable the clock.
    MMC_Wr(MMC_CLK_CNTL, 0x400000c0);
     
    // release the DDR DLL reset pin.
    MMC_Wr(MMC_SOFT_RST,  0xffff);
	
    //__udelay(1000);

	//wait_pll(3,500);
	wait_pll(3,250);
	//wait_pll(3,400);
	
	//asm volatile ("wfi");
	
    serial_puts("set ddr clock ok!\n");
}

static inline unsigned lowlevel_ddr(unsigned tag,struct ddr_set * timing_reg)
{
    set_ddr_clock(timing_reg);
    //if(tag)
    //    return ddr_init_sw(timing_reg);
    return ddr_init_hw(timing_reg);
}
static inline unsigned lowlevel_mem_test_device(unsigned tag,struct ddr_set * timing_reg)
{
    return tag&&(unsigned)memTestDevice((volatile datum *)PHYS_MEMORY_START,PHYS_MEMORY_SIZE);
}

static inline unsigned lowlevel_mem_test_data(unsigned tag,struct ddr_set * timing_reg)
{
    return tag&&(unsigned)memTestDataBus((volatile datum *) PHYS_MEMORY_START);
}

static inline unsigned lowlevel_mem_test_addr(unsigned tag,struct ddr_set * timing_reg)
{
	//asm volatile ("wfi");
    return tag&&(unsigned)memTestAddressBus((volatile datum *)PHYS_MEMORY_START, PHYS_MEMORY_SIZE);
}

static unsigned ( * mem_test[])(unsigned tag,struct ddr_set * timing_reg)={
    lowlevel_ddr,
    lowlevel_mem_test_addr,
    lowlevel_mem_test_data,
#ifdef CONFIG_ENABLE_MEM_DEVICE_TEST
    lowlevel_mem_test_device
#endif
};

#define MEM_DEVICE_TEST_ITEMS_BASE (sizeof(mem_test)/sizeof(mem_test[0]))

unsigned m6_ddr_init_test(int arg)
{
    int i; //,j;
    unsigned por_cfg=1;
    serial_putc('\n');
    por_cfg=0;
    for(i=0;i<MEM_DEVICE_TEST_ITEMS_BASE&&por_cfg==0;i++)
	{
        writel(0,P_WATCHDOG_RESET);
        por_cfg=mem_test[i](arg&(1<<i),&__ddr_setting);
        serial_puts("\nStage ");
        serial_put_hex(i,8);
        serial_puts(" Result ");
        serial_put_hex(por_cfg,32);
	}
	writel(0,P_WATCHDOG_TC);
	ddr_start_again=por_cfg?1:ddr_start_again;
	return por_cfg;
}
SPL_STATIC_FUNC unsigned ddr_init_test(void)
{
#define DDR_INIT_START  (1)
#define DDR_TEST_ADDR   (1<<1)
#define DDR_TEST_DATA   (1<<2)
#define DDR_TEST_DEVICE (1<<3)

#define DDR_TEST_BASEIC (DDR_INIT_START|DDR_TEST_ADDR|DDR_TEST_DATA)
#define DDR_TEST_ALL    (DDR_TEST_BASEIC|DDR_TEST_DEVICE)

	//m6_ddr_init_test(6);
	//m6_ddr_init_test(0x0E);
	m6_ddr_init_test(DDR_TEST_BASEIC);

	return 0;
}
	
