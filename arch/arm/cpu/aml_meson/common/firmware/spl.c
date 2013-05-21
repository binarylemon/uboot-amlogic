#define  CONFIG_AMLROM_SPL 1
#include <timer.c>
#include <timming.c>
#include <uartpin.c>
#include <serial.c>
#include <pinmux.c>
#include <sdpinmux.c>
#include <memtest.c>
#include <pll.c>
#include <ddr.c>
#include <mtddevices.c>
#include <sdio.c>
#include <debug_rom.c>

#include <loaduboot.c>

#ifdef CONFIG_POWER_SPL
#include <power.c>
#endif

unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{

	//setbits_le32(0xda004000,(1<<0));	//TEST_N enable: This bit should be set to 1 as soon as possible during the Boot process to prevent board changes from placing the chip into a production test mode

//write ENCI_MACV_N0 (CBUS 0x1b30) to 0, disable Macrovision
#if defined(CONFIG_M6) || defined(CONFIG_M6TV)
	writel(0, CBUS_REG_ADDR(ENCI_MACV_N0));
#endif

//Default to open ARM JTAG for M6 only
#if  defined(CONFIG_M6) || defined(CONFIG_M6TV)
	#define AML_M6_JTAG_ENABLE
	#define AML_M6_JTAG_SET_ARM

	//for M6 only. And it will cause M3 fail to boot up.
	//TEST_N enable: This bit should be set to 1 as soon as possible during the 
	//Boot process to prevent board changes from placing the chip into a production test mode
	setbits_le32(0xda004000,(1<<0));

	// set bit [12..14] to 1 in AO_RTI_STATUS_REG0
	// This disables boot device fall back feature in MX Rev-D
	// This still enables bootloader to detect which boot device
	// is selected during boot time. 
	switch(readl(0xc8100000))
	{
	case 0x6b730001:
	case 0x6b730002: writel(readl(0xc8100000) |(0x70<<8),0xc8100000);break;
	}

#endif


#ifdef AML_M6_JTAG_ENABLE
	#ifdef AML_M6_JTAG_SET_ARM
		//A9 JTAG enable
		writel(0x80000510,0xda004004);
		//TDO enable
		writel(0x4000,0xc8100014);
	#elif AML_M6_JTAG_SET_ARC
		//ARC JTAG enable
		writel(0x80051001,0xda004004);
		//ARC bug fix disable
		writel((readl(0xc8100040)|1<<24),0xc8100040);
	#endif	//AML_M6_JTAG_SET_ARM

	//Watchdog disable
	writel(0,0xc1109900);
	//asm volatile ("wfi");

#endif //AML_M6_JTAG_ENABLE

#if defined(WA_AML8726_M3_REF_V10) || defined(SHUTTLE_M3_MID_V1)
	//PWREN GPIOAO_2, PWRHLD GPIOAO_6 pull up
	//@WA-AML8726-M3_REF_V1.0.pdf -- WA_AML8726_M3_REF_V10
	//@Q07CL_DSN_RB_0922A.pdf -- SHUTTLE_M3_MID_V1
	//@AppNote-M3-CorePinMux.xlsx
	clrbits_le32(P_AO_GPIO_O_EN_N, ((1<<2)|(1<<6)));
	setbits_le32(P_AO_GPIO_O_EN_N,((1<<18)|(1<<22)));
#endif
		
//	int i;

    //Adjust 1us timer base
    timer_init();
    //default uart clock.
    //serial_init(__plls.uart);
    serial_put_dword(get_utimer(0));
    writel(0,P_WATCHDOG_TC);//disable Watchdog

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	unsigned spl_boot_start,spl_boot_end;
	spl_boot_start = TIMERE_GET();
#endif

#ifdef CONFIG_M6
	#ifdef CONFIG_PWM_CORE_VOLTAGE
	writel(0x632000, P_VGHL_PWM_REG0);
	writel(0x632000, P_LED_PWM_REG0);
	#else
    writel(0x631000, P_VGHL_PWM_REG0);    //enable VGHL_PWM
    #endif
    __udelay(1000);
#endif

    // initial pll
    pll_init(&__plls);
	serial_init(__plls.uart);

#ifdef CONFIG_POWER_SPL
	serial_puts("\n");
	serial_puts("\ninit power for cpu\n");
    power_init();
#endif
#ifdef ENTRY_DEBUG_ROM
    __udelay(100000);//wait for a uart input
#else
    __udelay(100);//wait for a uart input
#endif
	
	 if(serial_tstc()){
	    debug_rom(__FILE__,__LINE__);
	 }	 

    // initial ddr
    ddr_init_test();

#if 0
	serial_puts("\nMSR clk list:\n");
	int i;
	for(i=0;i<46;i++)
	{
		serial_put_hex(i,8);
		serial_puts("=");
		serial_put_dword(clk_util_clk_msr(i));
   }
#endif
	//asm volatile ("wfi");
	
    // load uboot
#ifdef CONFIG_ENABLE_WATCHDOG
	if(load_uboot(__TEXT_BASE,__TEXT_SIZE)){
   		serial_puts("\nload uboot error,now reset the chip");
		writel((1<<22) | (3<<24)|1000, P_WATCHDOG_TC);
	}
#else
    	load_uboot(__TEXT_BASE,__TEXT_SIZE);
#endif
    serial_puts("\nSystem Started\n");

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	spl_boot_end = TIMERE_GET();
	*((volatile unsigned int*)0x83a00000) = spl_boot_end;
	serial_puts("\nspl boot time(us):");
	serial_put_dword((spl_boot_end-spl_boot_start));
#else
	*((volatile unsigned int*)0x8fa00000) = 0;
#endif

#if 0
    //wait serial_puts end.
    for(i = 0; i < 10; i++)
		  __udelay(1000);
#else
	serial_wait_tx_empty();
#endif

#ifdef CONFIG_M6_TEST_CPU_SWITCH


	extern int get_cup_id(void);
	__udelay(10000);
	serial_puts("\n*************************************\n");
	__udelay(10000);
	serial_puts("CPU switch : CPU #");
	__udelay(10000);
	serial_put_hex(get_cpu_id(),4);
	__udelay(10000);
	serial_puts(" is sleeping\n");
	__udelay(10000);
	serial_puts("*************************************\n\n");
	__udelay(10000);


	writel(__TEXT_BASE,0xd901ff84);
	writel(1|1<<1,0xd901ff80);
	asm volatile ("": : :"memory");
	asm  volatile("dsb");
	asm volatile ("sev");
	while(1);

#endif//CONFIG_M6_TEST_CPU_SWITCH

    return 0;
}
