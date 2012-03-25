#include <config.h>
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/dmc.h>
#include <asm/arch/ddr.h>
#include <asm/arch/memtest.h>
#include <asm/arch/pctl.h>
#include "boot_code.c"

//----------------------------------------------------
unsigned UART_CONFIG_24M= (200000000/(115200*4)  );
unsigned UART_CONFIG= (32*1000/(300*4));
//#define EN_DEBUG
//----------------------------------------------------
//functions declare
void store_restore_plls(int flag);
#define clkoff_a9()

#define ISOLATE_RESET_N_TO_EE       clrbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,(1 << 5))
#define ISOLATE_TEST_MODE_FROM_EE   clrbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,(1 << 3))
#define ISOLATE_IRQ_FROM_EE         clrbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,(1 << 2))
#define ISOLATE_RESET_N_FROM_EE     clrbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,(1 << 1))
#define ISOLATE_AHB_BUS_FROM_EE     clrbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,(1 << 0))

#define ENABLE_RESET_N_TO_EE        setbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,  (1 << 5))
#define ENABLE_TEST_MODE_FROM_EE    setbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,  (1 << 3))
#define ENABLE_IRQ_FROM_EE          setbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,  (1 << 2))
#define ENABLE_RESET_N_FROM_EE      setbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,  (1 << 1))
#define ENABLE_AHB_BUS_FROM_EE      setbits_le32(P_AO_RTI_PWR_CNTL_REG0 ,  (1 << 0))

#define TICK_OF_ONE_SECOND 32000

#define dbg_out(s,v) f_serial_puts(s);serial_put_hex(v,32);f_serial_puts('\n');wait_uart_empty();

static void timer_init()
{
	//100uS stick timer a mode : periodic, timer a enable, timer e enable
    setbits_le32(P_AO_TIMER_REG,0x1f);
}

unsigned  get_tick(unsigned base)
{
    return readl(P_AO_TIMERE_REG)-base;
}

unsigned t_delay_tick(unsigned count)
{
    unsigned base=get_tick(0);
    if(readl(P_AO_RTI_PWR_CNTL_REG0)&(1<<8)){
        while(get_tick(base)<count);
    }else{
        while(get_tick(base)<count*100);
    }
    return 0;
}

unsigned delay_tick(unsigned count)
{
    unsigned i,j;
    for(i=0;i<count;i++)
    {
        for(j=0;j<1000;j++)
        {
            asm("mov r0,r0");
            asm("mov r0,r0");
        }
    }
}

void delay_ms(int ms)
{
		while(ms > 0){
		delay_tick(32);
		ms--;
	}
}

#define delay_1s() delay_tick(TICK_OF_ONE_SECOND);

//volatile unsigned * arm_base=(volatile unsigned *)0x8000;
void copy_reboot_code()
{
	int i;
	int code_size;
	volatile unsigned char* pcode = (volatile unsigned char*)arm_reboot;
  volatile unsigned char * arm_base = (volatile unsigned char *)0x0000;

	code_size = sizeof(arm_reboot);
	//copy new code for ARM restart
	for(i = 0; i < code_size; i++)
	{
/*	 	f_serial_puts("[ ");
		serial_put_hex(*arm_base,8);
	 	f_serial_puts(" , ");
		serial_put_hex(*pcode,8);
	 	f_serial_puts(" ]  ");
*/	 	
		
		if(i != 32 && i != 33 && i != 34 && i != 35) //skip firmware's reboot entry address.
				*arm_base = *pcode;
		pcode++;
		arm_base++;
	}
}

void disp_code()
{
#if 0
	int i;
	int code_size;
	volatile unsigned char * arm_base = (volatile unsigned char *)0x0000;
	unsigned addr;
	code_size = sizeof(arm_reboot);
	addr = 0;
	//copy new code for ARM restart
	for(i = 0; i < code_size; i++)
	{
	 	f_serial_puts(",");
		serial_put_hex(*arm_base,8);
		if(i == 32)
			addr |= *arm_base;
		else if(i == 33)
			addr |= (*arm_base)<<8;
		else if(i == 34)
			addr |= (*arm_base)<<16;
		else if(i == 35)
			addr |= (*arm_base)<<24;
			
		arm_base++;
	}
	
	f_serial_puts("\n addr:");
	serial_put_hex(addr,32);
	
	f_serial_puts(" value:");
	wait_uart_empty();
	serial_put_hex(readl(addr),32);
	wait_uart_empty();
#endif
}

void power_off_vddio();
//#define POWER_OFF_VDDIO
//#define POWER_OFF_HDMI_VCC
//#define POWER_OFF_AVDD33
//#define POWER_OFF_AVDD25
//#define POWER_DOWN_VCC12
//#define POWER_DOWN_DDR
//#define POWER_OFF_EE
static void enable_iso_ee()
{
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<4)),P_AO_RTI_PWR_CNTL_REG0);
}
static void disable_iso_ee()
{
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(1<<4),P_AO_RTI_PWR_CNTL_REG0);
}

static void cpu_off()
{
	writel(readl(P_HHI_SYS_CPU_CLK_CNTL)|(1<<19),P_HHI_SYS_CPU_CLK_CNTL);
}
static void switch_to_rtc()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(1<<8),P_AO_RTI_PWR_CNTL_REG0);
   udelay(100);
}
static void switch_to_81()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<8)),P_AO_RTI_PWR_CNTL_REG0);
   udelay(100);
}
static void enable_iso_ao()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(0xF<<0)),P_AO_RTI_PWR_CNTL_REG0);
}
static void disable_iso_ao()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(0xD<<0),P_AO_RTI_PWR_CNTL_REG0);
}
static void ee_off()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(0x1<<9)),P_AO_RTI_PWR_CNTL_REG0);
}
static void ee_on()
{
	 writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(0x1<<9),P_AO_RTI_PWR_CNTL_REG0);
}
void restart_arm()
{
	//------------------------------------------------------------------------
	// restart arm
		//0. make sure a9 reset
	setbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);
		
	//1. write flag
	writel(0x1234abcd,P_AO_RTI_STATUS_REG2);
	
	//2. remap AHB SRAM
//	writel(3,P_AO_REMAP_REG0);
	writel(1,P_AHB_ARBDEC_REG);
 
	//3. turn off romboot clock
	writel(readl(P_HHI_GCLK_MPEG1)&0x7fffffff,P_HHI_GCLK_MPEG1);
 
	//4. Release ISO for A9 domain.
	setbits_le32(P_AO_RTI_PWR_CNTL_REG0,1<<4);

	//reset A9
	writel(0xF,P_RESET4_REGISTER);// -- reset arm.ww
	writel(1<<14,P_RESET2_REGISTER);// -- reset arm.mali
	delay_ms(1);
	clrbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19); // release A9 reset
  
 //	f_serial_puts("arm restarted ...done\n");
//	wait_uart_empty();
}
#define v_outs(s,v) {f_serial_puts(s);serial_put_hex(v,32);f_serial_puts("\n"); wait_uart_empty();}

void test_ddr(int i)
{
#if 0
	f_serial_puts("test_ddr...\n");

	volatile unsigned addr = (volatile unsigned*)0x80000000;
	unsigned v;
	if(i == 0){
		for(i = 0; i < 100; i++){
//		v_outs("addr:",pAddr);
//		v_outs("value:",*pAddr);
			writel(i,addr);
			addr+=4;
		}
	}
	else if(i == 1){
			for(i = 0; i < 100; i++){
			//	writel(i,addr);
				v = readl(addr);
			//	if(v != i)
				{
			//		serial_put_hex(addr,32);
					f_serial_puts(" , ");
					serial_put_hex(v,32);
				}
				addr+=4;
			}
	}
	f_serial_puts("\n");
	wait_uart_empty();
#endif			
}
#define pwr_ddr_off 
//#define smp_test
#ifdef smp_test  // Just for temp solution for test flow
void enter_power_down()
{
	unsigned v;
	int i;
	unsigned addr;
	unsigned gate;
#ifdef smp_test
	//ignore ddr problems.
//	for(i = 0; i < 1000; i++)
//		udelay(1000);
//	restart_arm();
//	return;
#endif
//	disp_pctl();
//	test_ddr(0);
	 // First, we disable all memory accesses.
	f_serial_puts("step 1\n");
#ifdef pwr_ddr_off
  disable_mmc_req();
#endif	

  store_restore_plls(1);

#ifdef pwr_ddr_off
 	f_serial_puts("step 2\n");
 	wait_uart_empty();
  // Next, we sleep
  mmc_sleep();

    // save ddr power
  APB_Wr(MMC_PHY_CTRL, APB_Rd(MMC_PHY_CTRL)|(1<<0)|(1<<8)|(1<<13));
  APB_Wr(UPCTL_PHYCR_ADDR, APB_Rd(UPCTL_PHYCR_ADDR)|(1<<6));
  APB_Wr(UPCTL_DLLCR9_ADDR, APB_Rd(UPCTL_DLLCR9_ADDR)|(1<<31));
// 	  delay_ms(20);

   // enable retention
  enable_retention();	
  
  // power down DDR
 	writel(readl(P_HHI_DDR_PLL_CNTL)|(1<<15),P_HHI_DDR_PLL_CNTL);

 	f_serial_puts("step 3\n");
 	wait_uart_empty();

#endif

 	f_serial_puts("step 4\n");
 	wait_uart_empty();
  // turn off ee
//  enable_iso_ee();
//	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
 	
  
 	f_serial_puts("step 5\n");
 	wait_uart_empty();
  cpu_off();
  
  
 	f_serial_puts("step 6\n");
 	wait_uart_empty();
	//enable power_key int	
	writel(0x100,0xc1109860);//clear int
 	writel(readl(0xc1109868)|1<<8,0xc1109868);
	writel(readl(0xc8100080)|0x1,0xc8100080);


// ee use 32k, So interrup status can be accessed.
	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
	udelay(100);
	switch_to_rtc();
	udelay(1000);


 
//  enable_iso_ao();
 
//  ee_off();            
// gate off REMOTE, UART
  	writel(readl(P_AO_RTI_GEN_CNTL_REG0)&(~(0xF)),P_AO_RTI_GEN_CNTL_REG0);
 //  gate = readl(P_AO_RTI_GEN_CNTL_REG0);
//   writel(gate&(~(0xF)),P_AO_RTI_GEN_CNTL_REG0);
#if 1
	do{}while(!(readl(0xc1109860)&0x100));
#else
 	 for(i=0;i<64;i++)
   {
        udelay(1000);
        //udelay(1000);
   }
#endif
 //  writel(gate,P_AO_RTI_GEN_CNTL_REG0);
//	 udelay(100);
// gate on REMOTE, UART
	writel(readl(P_AO_RTI_GEN_CNTL_REG0)|0xF,P_AO_RTI_GEN_CNTL_REG0);

 
//  ee_on();
 
//  disable_iso_ao();

//#ifdef pwr_ddr_off
 // Next, we reset all channels 
//  reset_mmc();
//#endif

	//disable power_key int
	writel(readl(0xc1109868)&(~(1<<8)),0xc1109868);
	writel(readl(0xc8100080)&(~0x1),0xc8100080);
	writel(0x100,0xc1109860);//clear int


	switch_to_81();
	udelay(1000);
  // ee go back to clk81
	writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
	udelay(10000);

	
#ifdef pwr_ddr_off
	 // Next, we reset all channels 
	switch_to_rtc();
	udelay(1000);
	reset_mmc();
	switch_to_81();
	udelay(1000);
#endif

	//turn on ee
// 	writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
// 	writel(readl(P_HHI_GCLK_MPEG1)&(~(0x1<<31)),P_HHI_GCLK_MPEG1);
 	uart_reset();
 	
  f_serial_puts("step 7\n");
 	wait_uart_empty();
	store_restore_plls(0);
	
#ifdef pwr_ddr_off    
  f_serial_puts("step 8\n");
	wait_uart_empty();  
  init_ddr_pll();

  f_serial_puts("step 9\n");
 	wait_uart_empty();
  // initialize mmc and put it to sleep
  init_pctl();

// 	disp_pctl();

  f_serial_puts("step 10\n");
 	wait_uart_empty();
  mmc_sleep();


  f_serial_puts("step 11\n");
 	wait_uart_empty();
  // disable retention
  disable_retention();

  f_serial_puts("step 12\n");
 	wait_uart_empty();
  // Next, we wake up
  mmc_wakeup();

 
  f_serial_puts("step 13\n");
 	wait_uart_empty();
  // Next, we enable all requests
  enable_mmc_req();
#endif
	
//	disp_pctl();
	
//	test_ddr(1);
//	test_ddr(0);
//	test_ddr(1);
	
//	disp_code();	

	restart_arm();
  
}
#else// Just for temp solution for test flow
void enter_power_down()
{
	unsigned v;
	int i;
	unsigned addr;
	unsigned gate;

//	disp_pctl();
//	test_ddr(0);
	 // First, we disable all memory accesses.
	f_serial_puts("step 1\n");
#ifdef pwr_ddr_off
  disable_mmc_req();
#endif	

  store_restore_plls(1);

#ifdef pwr_ddr_off
 	f_serial_puts("step 2\n");
 	wait_uart_empty();
  // Next, we sleep
  mmc_sleep();

    // save ddr power
//  APB_Wr(MMC_PHY_CTRL, APB_Rd(MMC_PHY_CTRL)|(1<<0)|(1<<8)|(1<<13));
//  APB_Wr(UPCTL_PHYCR_ADDR, APB_Rd(UPCTL_PHYCR_ADDR)|(1<<6));
//  APB_Wr(UPCTL_DLLCR9_ADDR, APB_Rd(UPCTL_DLLCR9_ADDR)|(1<<31));
// 	  delay_ms(20);

   // enable retention
  enable_retention();	
  
  // power down DDR
// 	writel(readl(P_HHI_DDR_PLL_CNTL)|(1<<15),P_HHI_DDR_PLL_CNTL);

 	f_serial_puts("step 3\n");
 	wait_uart_empty();

#endif

 	f_serial_puts("step 4\n");
 	wait_uart_empty();
  // turn off ee
//  enable_iso_ee();
//	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
 	
  
 	f_serial_puts("step 5\n");
 	wait_uart_empty();
	cpu_off();
  
  
 	f_serial_puts("step 6\n");
 	wait_uart_empty();

	//enable power_key int	
	writel(0x100,0xc1109860);//clear int
 	writel(readl(0xc1109868)|1<<8,0xc1109868);
	writel(readl(0xc8100080)|0x1,0xc8100080);

	// ee use 32k, So interrup status can be accessed.
	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
	udelay(100);
	//ao to 32k
	switch_to_rtc();	
	udelay(1000);
 
//  enable_iso_ao();
 
//  ee_off();            
// gate off REMOTE, UART
//	writel(readl(P_AO_RTI_GEN_CNTL_REG0)&(~(0xF)),P_AO_RTI_GEN_CNTL_REG0);
 //  gate = readl(P_AO_RTI_GEN_CNTL_REG0);
//   writel(gate&(~(0xF)),P_AO_RTI_GEN_CNTL_REG0);
#if 1
	do{}while(!(readl(0xc1109860)&0x100));
#else
 	 for(i=0;i<64;i++)
   {
        udelay(1000);
        //udelay(1000);
   }
#endif


 //  writel(gate,P_AO_RTI_GEN_CNTL_REG0);
//	 udelay(100);
// gate on REMOTE, UART
//	writel(readl(P_AO_RTI_GEN_CNTL_REG0)|0xF,P_AO_RTI_GEN_CNTL_REG0);

 
//  ee_on();
 
//  disable_iso_ao();

	switch_to_81();
	
	// ee go back to clk81
	writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
	udelay(10000);

#ifdef pwr_ddr_off
 // Next, we reset all channels 
//	switch_to_rtc();
//	udelay(1000);
//	reset_mmc();
//	switch_to_81();
//	udelay(1000);
#endif

	//disable power_key int
	writel(readl(0xc1109868)&(~(1<<8)),0xc1109868);
	writel(readl(0xc8100080)&(~0x1),0xc8100080);
	writel(0x100,0xc1109860);//clear int


	//turn on ee
// 	writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
// 	writel(readl(P_HHI_GCLK_MPEG1)&(~(0x1<<31)),P_HHI_GCLK_MPEG1);
// 	uart_reset();
 	
	f_serial_puts("step 7\n");
 	wait_uart_empty();
	store_restore_plls(0);
	
#ifdef pwr_ddr_off    
	f_serial_puts("step 8\n");
	wait_uart_empty();  
//	init_ddr_pll();

	f_serial_puts("step 9\n");
 	wait_uart_empty();
  // initialize mmc and put it to sleep
//	init_pctl();

// 	disp_pctl();

	f_serial_puts("step 10\n");
 	wait_uart_empty();
//	mmc_sleep();


	f_serial_puts("step 11\n");
 	wait_uart_empty();
  // disable retention
	disable_retention();

	f_serial_puts("step 12\n");
 	wait_uart_empty();
  // Next, we wake up
	mmc_wakeup();

 
	f_serial_puts("step 13\n");
 	wait_uart_empty();
  // Next, we enable all requests
	enable_mmc_req();
#endif
	
//	disp_pctl();
	
	test_ddr(1);
//	test_ddr(0);
//	test_ddr(1);
	
//	disp_code();	

	restart_arm();
  
}

#endif

#if 0
void enter_power_down()
{
	int i;
	unsigned v1,v2,v;
	unsigned rtc_ctrl;
	unsigned power_key;
	//*******************************************
	//*  power down flow  
	//*******************************************
	f_serial_puts("enter_power_down\n");

	arm_restart();
	return;
	
	// disable all memory accesses.
    disable_mmc_req();
    
 	 f_serial_puts("arc 0\n");
   
    //save registers for clk and ddr
    store_restore_plls(1);

	f_serial_puts("arc 1\n");
	wait_uart_empty();
    
    //mmc enter sleep
    mmc_sleep();
//    delay_ms(20);
    
    // save ddr power
    APB_Wr(MMC_PHY_CTRL, APB_Rd(MMC_PHY_CTRL)|(1<<0)|(1<<8)|(1<<13));
    APB_Wr(UPCTL_PHYCR_ADDR, APB_Rd(UPCTL_PHYCR_ADDR)|(1<<6));
    APB_Wr(UPCTL_DLLCR9_ADDR, APB_Rd(UPCTL_DLLCR9_ADDR)|(1<<31));
// 	  delay_ms(20);

 	// power down DDR
 	writel(readl(P_HHI_DDR_PLL_CNTL)|(1<<15),P_HHI_DDR_PLL_CNTL);

	// enable retention
	enable_retention();

	writel(0,P_AO_RTI_STATUS_REG1);

 	// reset A9
	setbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);
	 
	// enable iso ee for A9
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<4)),P_AO_RTI_PWR_CNTL_REG0);
	f_serial_puts("arc 2\n");
	wait_uart_empty();

#ifdef POWER_OFF_HDMI_VCC
	reg7_off();
#endif
#ifdef POWER_OFF_AVDD33
	reg5_off();
#endif

#ifdef POWER_OFF_EE 
	//iso EE from AO
	//comment isolate EE. otherwise cannot detect power key.
	// writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<0)),P_AO_RTI_PWR_CNTL_REG0); 
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<2)),P_AO_RTI_PWR_CNTL_REG0);
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<3)),P_AO_RTI_PWR_CNTL_REG0);

	//?? Gate off clk81 to EE domain
	writel(readl(P_AO_RTI_GEN_CNTL_REG0)&(~(1<<12)),P_AO_RTI_GEN_CNTL_REG0);
	//-------------------------------
	//turn off EE voltage
	//v = readl(0xC8100024);
	//v &= ~(1<<9);
	//v &= ~(1<<25);
	//writel(v,0xC8100024);
#else
	// ee use 32k
	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
#endif

	
	// change RTC filter for 32k
  rtc_ctrl = readl(0xC810074c);
	writel(0x00800000,0xC810074c);
	// switch to 32k
    writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(1<<8),P_AO_RTI_PWR_CNTL_REG0);
    udelay(100);
#ifdef POWER_OFF_VDDIO 
	vddio_off(); 
#endif		
#ifdef POWER_OFF_AVDD25
	reg6_off();
#endif
	udelay(100);
#if (defined(POWER_DOWN_VCC12) || defined(POWER_DOWN_DDR))
	switch_voltage(1);
#endif
#ifdef POWER_DOWN_DDR
	powerdown_ddr();
#endif
#ifdef POWER_DOWN_VCC12
	powerdown_vcc12();
#endif

	// gate off REMOTE, UART
	writel(readl(P_AO_RTI_GEN_CNTL_REG0)&(~(0xF)),P_AO_RTI_GEN_CNTL_REG0);
	// wait key
    power_key = readl(0Xc8100744);
    #if 1
    while (((power_key&4) != 0)&&((power_key&8) == 0))
   {
     	power_key = readl(0Xc8100744);
   }
   #else
    for(i=0;i<64;i++)
    {
        udelay(1000);
        //udelay(1000);
    }
   #endif
    
	// gate on REMOTE, I2C s/m, UART
	writel(readl(P_AO_RTI_GEN_CNTL_REG0)|0xF, P_AO_RTI_GEN_CNTL_REG0); 
	udelay(10);
#ifdef POWER_DOWN_DDR
	powerup_ddr();
#endif
#ifdef POWER_DOWN_VCC12
	powerup_vcc12();
#endif
#if (defined(POWER_DOWN_VCC12) || defined(POWER_DOWN_DDR))
	switch_voltage(0);
#endif

#ifdef POWER_OFF_AVDD25
	reg6_on();
#endif
#ifdef POWER_OFF_VDDIO 
	vddio_on();
#endif
	udelay(100);
   // switch to clk81 
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(0x1<<8)),P_AO_RTI_PWR_CNTL_REG0);
	udelay(100);
	// restore RTC filter
	writel(rtc_ctrl,0xC810074c);

	f_serial_puts("arc 3\n");
	wait_uart_empty();

	// set AO interrupt mask
	writel(0xFFFF,P_AO_IRQ_STAT_CLR);
	
#ifdef POWER_OFF_EE
	//turn on EE voltage
	//v = readl(0xC8100024);
	//v &= ~(1<<9);
	//v |= (1<<25);
	//writel(v,0xC8100024);
	//delay_ms(200);

	// un-iso AO domain from EE bit0=signals, bit1=reset, bit2=irq, bit3=test_mode
 	writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(0xD<<0),P_AO_RTI_PWR_CNTL_REG0);

	//un isolate the reset in the EE
	writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(0x1<<5),P_AO_RTI_PWR_CNTL_REG0);

	writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(0x1<<5)|(1<<3)|(1<<2)|(1<<1)|(1<<0), \
			   P_AO_RTI_PWR_CNTL_REG0);
#else
    // ee go back to clk81
	writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
#endif
	
#ifdef POWER_OFF_AVDD33
	reg5_on();
#endif
#ifdef POWER_OFF_HDMI_VCC
	reg7_on();
#endif    
	f_serial_puts("arc 4\n");
	wait_uart_empty();

    store_restore_plls(0);
     
    init_ddr_pll();
    
		uart_reset();

    reset_mmc();

    // initialize mmc and put it to sleep
    init_pctl();

    mmc_sleep();
    
    // disable retention
    disable_retention();

    // Next, we wake up
    mmc_wakeup();

    // Next, we enable all requests
    enable_mmc_req();
	f_serial_puts("arc 5\n");
	wait_uart_empty();

//	f_serial_puts("restart arm...\n");
	
	//0. make sure a9 reset
	setbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);
		
	//1. write flag
	if (power_key&8)
		writel(0xabcd1234,P_AO_RTI_STATUS_REG2);
	else
		writel(0x1234abcd,P_AO_RTI_STATUS_REG2);
	
	//2. remap AHB SRAM
//	writel(3,P_AO_REMAP_REG0);
	writel(1,P_AHB_ARBDEC_REG);
 
	//3. turn off romboot clock
	writel(readl(P_HHI_GCLK_MPEG1)&0x7fffffff,P_HHI_GCLK_MPEG1);
 
	//4. Release ISO for A9 domain.
	setbits_le32(P_AO_RTI_PWR_CNTL_REG0,1<<4);

	//reset A9
	writel(0xF,P_RESET4_REGISTER);// -- reset arm.ww
	writel(1<<14,P_RESET2_REGISTER);// -- reset arm.mali
	delay_ms(1);
	clrbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19); // release A9 reset
  
 	f_serial_puts("arc 6\n");
	wait_uart_empty();
 
//	delay_1s();
//	delay_1s();
//	delay_1s();
}
#endif

//#define ART_CORE_TEST
#ifdef ART_CORE_TEST
void test_arc_core()
{
    int i;
    int j,k;
    unsigned int power_key=0;
    
    for(i=0;i<1000;i++)
    {
        asm("mov r0,r0");
        asm("mov r0,r0");
        //udelay(1000);
        //udelay(1000);
        
    }
    
    
	f_serial_puts("\n");
	wait_uart_empty();

    writel(0,P_AO_RTI_STATUS_REG1);    

 	// reset A9 clock
	//setbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);

	// enable iso ee for A9
	//writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<4)),P_AO_RTI_PWR_CNTL_REG0);


	// wait key
    power_key = readl(0Xc8100744);
    
    f_serial_puts("get power_key\n");
    #if 0
    while (((power_key&4) != 0)&&((power_key&8) == 0))
   {
     	power_key = readl(0Xc8100744);
   }
   #else
    for(i=0;i<1000;i++)
    {
        for(j=0;j<1000;j++)
        {
            for(k=0;k<100;k++)
            {
                asm("mov r0,r0");
            }
        }
        //udelay(1000);
        //udelay(1000);
        
    }
   #endif

    f_serial_puts("delay 2s\n");

	//0. make sure a9 reset
//	setbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);

#if 0
	//1. write flag
	if (power_key&8)
		writel(0xabcd1234,P_AO_RTI_STATUS_REG2);
	else
		writel(0x1234abcd,P_AO_RTI_STATUS_REG2);
#endif
	//2. remap AHB SRAM
	writel(3,P_AO_REMAP_REG0);
	writel(2,P_AHB_ARBDEC_REG);
	
	f_serial_puts("remap arm arc\n");

	//3. turn off romboot clock
	writel(readl(P_HHI_GCLK_MPEG1)&0x7fffffff,P_HHI_GCLK_MPEG1);
	
	f_serial_puts("off romboot clock\n");

	//4. Release ISO for A9 domain.
	//setbits_le32(P_AO_RTI_PWR_CNTL_REG0,1<<4);

	//reset A9
	writel(0xF,P_RESET4_REGISTER);// -- reset arm.ww
//	writel(1<<14,P_RESET2_REGISTER);// -- reset arm.mali
	delay_ms(1);
//	clrbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19); // release A9 reset

//	f_serial_puts("arm reboot\n");
//	wait_uart_empty();
    f_serial_puts("arm reboot\n");

}
#endif
#define _UART_DEBUG_COMMUNICATION_
int main(void)
{
	unsigned cmd;
	char c;
	int i = 0,j;
	timer_init();
#ifdef POWER_OFF_VDDIO	
	f_serial_puts("sleep ... off\n");
#else
	f_serial_puts("sleep .......\n");
#endif
		
	while(1){
		
		cmd = readl(P_AO_RTI_STATUS_REG0);
		if(cmd == 0)
		{
			delay_ms(10);
			continue;
		}
		c = (char)cmd;
		if(c == 't')
		{
#if (defined(POWER_OFF_VDDIO) || defined(POWER_OFF_HDMI_VCC) || defined(POWER_OFF_AVDD33) || defined(POWER_OFF_AVDD25))
			init_I2C();
#endif
			copy_reboot_code();
			enter_power_down();
			//test_arc_core();
			break;
		}
		else if(c == 'q')
		{
				serial_puts(" - quit command loop\n");
				writel(0,P_AO_RTI_STATUS_REG0);
			  break;
		}
		else
		{
				serial_puts(" - cmd no support (ARC)\n");
		}
		//command executed
		writel(0,P_AO_RTI_STATUS_REG0);
	}
	
	while(1){
	    udelay(6000);
	    cmd = readl(P_AO_RTI_STATUS_REG1);
	    c = (char)cmd;
	    if(c == 0)
	    {
	        udelay(6000);
	        cmd = readl(P_AO_RTI_STATUS_REG1);
	        c = (char)cmd;
	        if((c == 0)||(c!='r'))
	        {
	            #ifdef _UART_DEBUG_COMMUNICATION_
	            serial_put_hex(cmd,32);
	            f_serial_puts(" arm boot fail\n\n");
	            wait_uart_empty();
	            #endif
	            #if 0 //power down 
	            cmd = readl(P_AO_GPIO_O_EN_N);
	            cmd &= ~(1<<6);
	            cmd &= ~(1<<22);
	            writel(cmd,P_AO_GPIO_O_EN_N);
	            #endif
	        }
	    }
	    else if(c=='r')
	    {
	        writel(0,0xc8100030);
	        #ifdef _UART_DEBUG_COMMUNICATION_
	        //f_serial_puts("arm boot succ\n");
	        //wait_uart_empty();
	        #endif
	    }
	    else
	    {
	        #ifdef _UART_DEBUG_COMMUNICATION_
	        serial_put_hex(cmd,32);
	        f_serial_puts(" arm unkonw state\n");
	        wait_uart_empty();
	        #endif
	    }
	    //cmd='f';
	    //writel(cmd,P_AO_RTI_STATUS_REG1);
	    
		asm(".long 0x003f236f"); //add sync instruction.
		asm("SLEEP");
	}
	return 0;
}
unsigned clk_settings[2]={0,0};
unsigned pll_settings[2][3]={{0,0,0},{0,0,0}};
void store_restore_plls(int flag)
{
    int i;
    if(flag)
    {
#ifdef POWER_OFF_EE 
        pll_settings[0][0]=readl(P_HHI_SYS_PLL_CNTL);
        pll_settings[0][1]=readl(P_HHI_SYS_PLL_CNTL2);
        pll_settings[0][2]=readl(P_HHI_SYS_PLL_CNTL3);
		
        pll_settings[1][0]=readl(P_HHI_OTHER_PLL_CNTL);
        pll_settings[1][1]=readl(P_HHI_OTHER_PLL_CNTL2);
        pll_settings[1][2]=readl(P_HHI_OTHER_PLL_CNTL3);

        clk_settings[0]=readl(P_HHI_SYS_CPU_CLK_CNTL);
        clk_settings[1]=readl(P_HHI_MPEG_CLK_CNTL);
#endif

        save_ddr_settings();
        return;
    }    
    
#ifdef POWER_OFF_EE 
    /* restore from default settings */ 
    writel(pll_settings[0][0]|0x8000, P_HHI_SYS_PLL_CNTL);
    writel(pll_settings[0][1], P_HHI_SYS_PLL_CNTL2);
    writel(pll_settings[0][2], P_HHI_SYS_PLL_CNTL3);
    writel(pll_settings[0][0]&(~0x8000),P_HHI_SYS_PLL_CNTL);
    writel(1<<2, P_RESET5_REGISTER);

    writel(pll_settings[1][0]|0x8000, P_HHI_OTHER_PLL_CNTL);
    writel(pll_settings[1][1], P_HHI_OTHER_PLL_CNTL2);
    writel(pll_settings[1][2], P_HHI_OTHER_PLL_CNTL3);
    writel(pll_settings[1][0]&(~0x8000),P_HHI_OTHER_PLL_CNTL);
    writel(1<<1, P_RESET5_REGISTER);

    writel(clk_settings[0],P_HHI_SYS_CPU_CLK_CNTL);
    writel(clk_settings[1],P_HHI_MPEG_CLK_CNTL);	    
    delay_ms(50);
#endif
}

void __raw_writel(unsigned val,unsigned reg)
{
	(*((volatile unsigned int*)(reg)))=(val);
	asm(".long 0x003f236f"); //add sync instruction.
}

unsigned __raw_readl(unsigned reg)
{
	asm(".long 0x003f236f"); //add sync instruction.
	return (*((volatile unsigned int*)(reg)));
}
