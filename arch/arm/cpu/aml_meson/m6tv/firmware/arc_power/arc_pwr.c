#include <config.h>
//#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/dmc.h>
#include <asm/arch/ddr.h>
#include <asm/arch/memtest.h>
#include <asm/arch/pctl.h>
//#include <asm/arch/c_always_on_pointer.h>
#include "boot_code.dat"

//#define CONFIG_ARC_SARDAC_ENABLE
#ifdef CONFIG_ARC_SARDAC_ENABLE
#include "sardac_arc.c"
#endif

//----------------------------------------------------
unsigned UART_CONFIG_24M= (200000000/(115200*4)  );
unsigned UART_CONFIG= (32*1000/(300*4));
//#define EN_DEBUG
//----------------------------------------------------
//functions declare

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

//#define f_serial_puts(a)
//#define serial_put_hex(a,b)
//#define wait_uart_empty()
//#define udelay(a)

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

void udelay(int i)
{
    int delays = 0;
    for(delays=0;delays<i;delays++)
    {
        asm("mov r0,r0");
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

#ifdef CONFIG_AW_AXP20
#define CHECK_ALL_REGULATORS
#endif


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

	//use sys pll for speed up
	//clrbits_le32(P_HHI_SYS_PLL_CNTL, (1 << 30));
	//setbits_le32(P_HHI_SYS_CPU_CLK_CNTL , (1 << 7));

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
void enter_power_down()
{
	int i;
	unsigned int uboot_cmd_flag=readl(P_AO_RTI_STATUS_REG2);//u-boot suspend cmd flag
	unsigned char vcin_state;
	unsigned char charging_state;

	//	disp_pctl();
	//	test_ddr(0);
	// First, we disable all memory accesses.

	f_serial_puts("step 1\n");

#if 0
#ifdef pwr_ddr_off
	asm(".long 0x003f236f"); //add sync instruction.

	disable_mmc_req();

	serial_put_hex(APB_Rd(MMC_LP_CTRL1),32);
	f_serial_puts("  LP_CTRL1\n");
	wait_uart_empty();

	serial_put_hex(APB_Rd(UPCTL_MCFG_ADDR),32);
	f_serial_puts("  MCFG\n");
	wait_uart_empty();

	store_restore_plls(1);

	APB_Wr(UPCTL_SCTL_ADDR, 1);
	APB_Wr(UPCTL_MCFG_ADDR, 0x60021 );
	APB_Wr(UPCTL_SCTL_ADDR, 2);

	serial_put_hex(APB_Rd(UPCTL_MCFG_ADDR),32);
	f_serial_puts("  MCFG\n");
	wait_uart_empty();

#endif

#ifdef CHECK_ALL_REGULATORS
	// Check regulator
	f_serial_puts("Chk regulators\n");
 	wait_uart_empty();
	check_all_regulators();
#endif

#ifdef pwr_ddr_off
	// MMC sleep 
 	f_serial_puts("Start DDR off\n");
	wait_uart_empty();
	// Next, we sleep
	mmc_sleep();

#if 1
	//Clear PGCR CK
	APB_Wr(PUB_PGCR_ADDR,APB_Rd(PUB_PGCR_ADDR)&(~(3<<12)));
	APB_Wr(PUB_PGCR_ADDR,APB_Rd(PUB_PGCR_ADDR)&(~(7<<9)));
	//APB_Wr(PUB_PGCR_ADDR,APB_Rd(PUB_PGCR_ADDR)&(~(3<<9)));
#endif
	mmc_sleep();
	// enable retention
	//only necessory if you want to shut down the EE 1.1V and/or DDR I/O 1.5V power supply.
	//but we need to check if we enable this feature, we can save more power on DDR I/O 1.5V domain or not.
	enable_retention();

    // save ddr power
    // before shut down DDR PLL, keep the DDR PHY DLL in reset mode.
    // that will save the DLL analog power.
#ifndef POWER_DOWN_DDRPHY
 	f_serial_puts("mmc soft rst\n");
	wait_uart_empty();
	APB_Wr(MMC_SOFT_RST, 0x0);	 // keep all MMC submodules in reset mode
#else
 	f_serial_puts("pwr dn ddr\n");
	wait_uart_empty();
	power_down_ddr_phy();
#endif

  // shut down DDR PLL. 
	writel(readl(P_HHI_DDR_PLL_CNTL)|(1<<30),P_HHI_DDR_PLL_CNTL);

	f_serial_puts("Done DDR off\n");
	wait_uart_empty();

#endif
#endif
 	f_serial_puts("CPU off\n");
 	wait_uart_empty();
	cpu_off();
  
   	f_serial_puts("Set up pwr key\n");
 	wait_uart_empty();
	//enable power_key int	
	writel(0x100,0xc1109860);//clear int
 	writel(readl(0xc1109868)|1<<8,0xc1109868);
	writel(readl(0xc8100080)|0x1,0xc8100080);

	f_serial_puts("Pwr off domains\n");
 	wait_uart_empty();

	//power_off_at_24M();

//	writel(readl(P_AO_RTI_PWR_CNTL_REG0)&(~(1<<4)),P_AO_RTI_PWR_CNTL_REG0);

// ee use 32k, So interrup status can be accessed.
//	writel(readl(P_HHI_MPEG_CLK_CNTL)|(1<<9),P_HHI_MPEG_CLK_CNTL);
//	switch_to_rtc();
	udelay(1000);

	//power_off_at_32K_1();

//	power_off_at_32K_2();

	// gate off REMOTE, UART
	writel(readl(P_AO_RTI_GEN_CTNL_REG0)&(~(0x8)),P_AO_RTI_GEN_CTNL_REG0);

#if 0
//	udelay(200000);//Drain power

	if(uboot_cmd_flag == 0x87654321)//u-boot suspend cmd flag
	{
//		power_off_ddr15();

#ifdef CONFIG_ARC_SARDAC_ENABLE
		do{
			udelay(2000);
			vcin_state=get_charging_state();
			if(!vcin_state)
				break;
		}while((!(readl(0xc1109860)&0x100)) && !(get_adc_sample_in_arc(4)<0x1000));
#else
		do{
			udelay(2000);
			vcin_state=get_charging_state();
			if(!vcin_state)
				break;
		}while(!(readl(0xc1109860)&0x100));
#endif//CONFIG_ARC_SARDAC_ENABLE
//		power_on_ddr15();
	}
	else
	{
		charging_state=get_charging_state();//get state before enter polling
		do{
			udelay(200);
			if(get_charging_state() ^ charging_state)//when the state is changed, wakeup
				break;
		}while(!(readl(0xc1109860)&0x100));
	}

//	while(!(readl(0xc1109860)&0x100)){break;}
#else
	for(i=0;i<200;i++)
   {
        udelay(1000);
        //udelay(1000);
   }
#endif

	//disable power_key int
/*	writel(readl(0xc1109868)&(~(1<<8)),0xc1109868);
	writel(readl(0xc8100080)&(~0x1),0xc8100080);
	writel(0x100,0xc1109860);//clear int
*/
// gate on REMOTE, UART
	writel(readl(P_AO_RTI_GEN_CTNL_REG0)|0x8,P_AO_RTI_GEN_CTNL_REG0);

//	power_on_at_32k_2();

	{
//		power_on_at_32k_1();

	//  In 32k mode, we had better not print any log.
//		store_restore_plls(0);//Before switch back to clk81, we need set PLL

	//	dump_pmu_reg();
	
//		switch_to_81();
	  // ee go back to clk81
//		writel(readl(P_HHI_MPEG_CLK_CNTL)&(~(0x1<<9)),P_HHI_MPEG_CLK_CNTL);
		udelay(10000);
	}

	// power on even more domains
	f_serial_puts("Pwr up avdd33/3gvcc\n");
	wait_uart_empty();
	
//	power_on_at_24M();
// 	uart_reset();

//	f_serial_puts("step 7\n");
//	wait_uart_empty();
//	store_restore_plls(0);
	
#ifdef pwr_ddr_off    
	f_serial_puts("step 8\n");
	wait_uart_empty();  
#if 0
	init_ddr_pll();

	store_vid_pll();
   // Next, we reset all channels 
	reset_mmc();
	f_serial_puts("step 9\n");
	wait_uart_empty();

	// disable retention
	// disable retention before init_pctl is because init_pctl you need to data training stuff.
	disable_retention();

	// initialize mmc and put it to sleep
	init_pctl();
	f_serial_puts("step 10\n");
	wait_uart_empty();

	//print some useful information to help debug.
	serial_put_hex(APB_Rd(MMC_LP_CTRL1),32);
	f_serial_puts("  MMC_LP_CTRL1\n");
	wait_uart_empty();

	serial_put_hex(APB_Rd(UPCTL_MCFG_ADDR),32);
	f_serial_puts("  MCFG\n");
	wait_uart_empty();

	if(uboot_cmd_flag == 0x87654321)//u-boot suspend cmd flag
	{
//		writel(readl(P_HHI_SYS_PLL_CNTL)&(~1<<30),P_HHI_SYS_PLL_CNTL);//power on sys pll
		if(!vcin_state)//plug out ACIN
		{
			shut_down();
			do{
				udelay(20000);
				f_serial_puts("wait shutdown...\n");
				wait_uart_empty();
			}while(1);
		}
		else
		{
			writel(0,P_AO_RTI_STATUS_REG2);
			writel(readl(P_AO_RTI_PWR_CNTL_REG0)|(1<<4),P_AO_RTI_PWR_CNTL_REG0);
			clrbits_le32(P_HHI_SYS_CPU_CLK_CNTL,1<<19);
			writel(10,0xc1109904);
			writel(1<<22|3<<24,0xc1109900);

		    do{udelay(20000);f_serial_puts("wait reset...\n");wait_uart_empty();}while(1);
		}
	}
#endif
#endif   //pwr_ddr_off
  // Moved the enable mmc req and SEC to ARM code.
  //enable_mmc_req();
	
//	disp_pctl();
	
//	test_ddr(1);
//	test_ddr(0);
//	test_ddr(1);
	
//	disp_code();	

	f_serial_puts("restart arm\n");
	wait_uart_empty();

	serial_put_hex(readl(P_VGHL_PWM_REG0),32);
	f_serial_puts("  VGHL_PWM before\n");
	wait_uart_empty();
	writel(0x631000, P_VGHL_PWM_REG0);    //enable VGHL_PWM
	udelay(1000);

	restart_arm();
  
}


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
#if (defined(CONFIG_AW_AXP20) || defined(CONFIG_ACT8942QJ233_PMU) || defined(CONFIG_AML_PMU))
			init_I2C();
#endif
			copy_reboot_code();
			enter_power_down();
			//test_arc_core();
			break;
		}
		else if(c == 'q')
		{
				f_serial_puts(" - quit command loop\n");
				writel(0,P_AO_RTI_STATUS_REG0);
			  break;
		}
		else
		{
				f_serial_puts(" - cmd no support (ARC)\n");
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
		} else if ( cmd == 1 )
		{
			serial_put_hex(cmd,32);
			f_serial_puts(" ARM has started running\n");
			wait_uart_empty();
		} else if ( cmd == 2 )
		{
			serial_put_hex(cmd,32);
			f_serial_puts(" Reenable SEC\n");
			wait_uart_empty();
	}
	    else if(c=='r')
	    {
	        writel(0,0xc8100030);
	        f_serial_puts("arm boot succ\n");
	        wait_uart_empty();
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

unsigned pll_settings[4];
unsigned mpll_settings[10];
unsigned viidpll_settings[4];
unsigned vidpll_settings[4];

#define CONFIG_SYS_PLL_SAVE
void store_restore_plls(int flag)
{
    int i;
    if(flag)
    {
#ifdef CONFIG_SYS_PLL_SAVE 
		pll_settings[0]=readl(P_HHI_SYS_PLL_CNTL);
		pll_settings[1]=readl(P_HHI_SYS_PLL_CNTL2);
		pll_settings[2]=readl(P_HHI_SYS_PLL_CNTL3);
		pll_settings[3]=readl(P_HHI_SYS_PLL_CNTL4);

		for(i=0;i<10;i++)//store mpll
		{
			mpll_settings[i]=readl(P_HHI_MPLL_CNTL + 4*i);
		}
/*
		viidpll_settings[0]=readl(P_HHI_VIID_PLL_CNTL);
		viidpll_settings[1]=readl(P_HHI_VIID_PLL_CNTL2);
		viidpll_settings[2]=readl(P_HHI_VIID_PLL_CNTL3);
		viidpll_settings[3]=readl(P_HHI_VIID_PLL_CNTL4);
*/
		/*Audio PLL instead*/


		vidpll_settings[0]=readl(P_HHI_VID_PLL_CNTL);
		vidpll_settings[1]=readl(P_HHI_VID_PLL_CNTL2);
		vidpll_settings[2]=readl(P_HHI_VID_PLL_CNTL3);
		vidpll_settings[3]=readl(P_HHI_VID_PLL_CNTL4);

#endif //CONFIG_SYS_PLL_SAVE

		//save_ddr_settings();
		return;
    }    
    
#ifdef CONFIG_SYS_PLL_SAVE 

	//temp define
#define P_HHI_MPLL_CNTL5         CBUS_REG_ADDR(HHI_MPLL_CNTL5)

	do{
		//BANDGAP reset for SYS_PLL,VIID_PLL,MPLL lock fail
		//Note: once SYS PLL is up, there is no need to 
		//          use AM_ANALOG_TOP_REG1 for VIID, MPLL
		//          lock fail
		writel(readl(P_HHI_MPLL_CNTL5)&(~1),P_HHI_MPLL_CNTL5); 
		udelay(3);
		writel(readl(P_HHI_MPLL_CNTL5)|1,P_HHI_MPLL_CNTL5); 
		udelay(30); //1ms in 32k for bandgap bootup
		
		writel(1<<29,P_HHI_SYS_PLL_CNTL);		
		writel(pll_settings[1],P_HHI_SYS_PLL_CNTL2);
		writel(pll_settings[2],P_HHI_SYS_PLL_CNTL3);
		writel(pll_settings[3],P_HHI_SYS_PLL_CNTL4);
		writel((pll_settings[0] & ~(1<<30))|1<<29,P_HHI_SYS_PLL_CNTL);
		writel(pll_settings[0] & ~(3<<29),P_HHI_SYS_PLL_CNTL);
		
		//M6_PLL_WAIT_FOR_LOCK(HHI_SYS_PLL_CNTL);

		udelay(10); //wait 100us for PLL lock
	}while((readl(P_HHI_SYS_PLL_CNTL)&0x80000000)==0);

	do{
		//no need to do bandgap reset
		writel(1<<29,P_HHI_MPLL_CNTL);
		for(i=1;i<10;i++)
			writel(mpll_settings[i],P_HHI_MPLL_CNTL+4*i);

		writel((mpll_settings[0] & ~(1<<30))|1<<29,P_HHI_MPLL_CNTL);
		writel(mpll_settings[0] & ~(3<<29),P_HHI_MPLL_CNTL);
		udelay(10); //wait 200us for PLL lock		
	}while((readl(P_HHI_MPLL_CNTL)&0x80000000)==0);
/*
	do{
		//no need to do bandgap reset
		if(!(viidpll_settings[0] & 0x3fff))//M,N domain == 0, not restore vid pll
			break;
		writel(1<<29,P_HHI_VIID_PLL_CNTL);		
		writel(viidpll_settings[1],P_HHI_VIID_PLL_CNTL2);
		writel(viidpll_settings[2],P_HHI_VIID_PLL_CNTL3);
		writel(viidpll_settings[3],P_HHI_VIID_PLL_CNTL4);

		writel((viidpll_settings[0] & ~(1<<30))|1<<29,P_HHI_VIID_PLL_CNTL);
		writel(viidpll_settings[0] & ~(3<<29),P_HHI_VIID_PLL_CNTL);
		udelay(10); //wait 200us for PLL lock		
	}while((readl(P_HHI_VIID_PLL_CNTL)&0x80000000)==0);
*/
	/*Audio PLL instead*/

	udelay(3);
#endif //CONFIG_SYS_PLL_SAVE
}

void store_vid_pll()
{
	if(!(vidpll_settings[0] & 0x7fff))//M,N domain == 0, not restore vid pll
		return;
	do{
		//no need to do bandgap reset
		writel(1<<29,P_HHI_VID_PLL_CNTL);
		//writel((vidpll_settings[0] & ~(1<<30))|1<<29,P_HHI_VID_PLL_CNTL);
		writel(vidpll_settings[1],P_HHI_VID_PLL_CNTL2);
		writel(vidpll_settings[2],P_HHI_VID_PLL_CNTL3);
		writel(vidpll_settings[3],P_HHI_VID_PLL_CNTL4);

		writel((vidpll_settings[0] & ~(1<<30))|1<<29,P_HHI_VID_PLL_CNTL);
		writel(vidpll_settings[0] & ~(3<<29),P_HHI_VID_PLL_CNTL);
		udelay(24000); //wait 200us for PLL lock
	}while((readl(P_HHI_VID_PLL_CNTL)&0x80000000)==0);
}

