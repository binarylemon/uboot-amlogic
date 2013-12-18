#define  CONFIG_AMLROM_SPL 1
#include <timer.c>
#include <timming.c>
#include <uartpin.c>
#include <serial.c>
#include <pinmux.c>
#include <sdpinmux.c>
#include <memtest.c>
#include <pll.c>
#include <hardi2c_lite.c>
#include <power.c>
#include <ddr.c>
#include <mtddevices.c>
#include <sdio.c>
#include <debug_rom.c>

#include <loaduboot.c>
#ifdef CONFIG_ACS
#include <storage.c>
#include <acs.c>
#endif

#include <asm/arch/reboot.h>

#ifdef CONFIG_POWER_SPL
extern void power_init(); 
#endif

#ifdef CONFIG_MESON_TRUSTZONE
#include <secureboot.c>
unsigned int ovFlag;
#endif

unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
#ifdef CONFIG_M8
	//enable watchdog for 5s
	//if bootup failed, switch to next boot device
	writel(((1<<22) | 500000), P_WATCHDOG_TC); //5s
#endif
	//setbits_le32(0xda004000,(1<<0));	//TEST_N enable: This bit should be set to 1 as soon as possible during the Boot process to prevent board changes from placing the chip into a production test mode

	writel((readl(0xDA000004)|0x08000000), 0xDA000004);	//set efuse PD=1

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


#if defined(CONFIG_M8)
	//A9 JTAG enable
	writel(0x102,0xda004004);
	//TDO enable
	writel(readl(0xc8100014)|0x4000,0xc8100014);
	
	//detect sdio debug board
	unsigned pinmux_2 = readl(P_PERIPHS_PIN_MUX_2);
	
	// clear sdio pinmux
	setbits_le32(P_PREG_PAD_GPIO0_O,0x3f<<22);
	setbits_le32(P_PREG_PAD_GPIO0_EN_N,0x3f<<22);
	clrbits_le32(P_PERIPHS_PIN_MUX_2,7<<12);  //clear sd d1~d3 pinmux
	
	if(!(readl(P_PREG_PAD_GPIO0_I)&(1<<26))){  //sd_d3 low, debug board in
		serial_puts("\nsdio debug board detected ");
		clrbits_le32(P_AO_RTI_PIN_MUX_REG,3<<11);   //clear AO uart pinmux
		setbits_le32(P_PERIPHS_PIN_MUX_8,3<<9);
		
		if((readl(P_PREG_PAD_GPIO0_I)&(1<<22)))
			writel(0x220,P_AO_SECURE_REG1);  //enable sdio jtag
	}
	else{
		serial_puts("\nno sdio debug board detected ");
		writel(pinmux_2,P_PERIPHS_PIN_MUX_2);
	}
#endif 


#ifdef AML_M6_JTAG_ENABLE
	#ifdef AML_M6_JTAG_SET_ARM
		//A9 JTAG enable
		writel(0x80000510,0xda004004);
		//TDO enable
		writel(readl(0xc8100014)|0x4000,0xc8100014);
	#elif AML_M6_JTAG_SET_ARC
		//ARC JTAG enable
		writel(0x80051001,0xda004004);
		//ARC bug fix disable
		writel((readl(0xc8100040)|1<<24),0xc8100040);
	#endif	//AML_M6_JTAG_SET_ARM

	//Watchdog disable
	//writel(0,0xc1109900);
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

	//Note: Following msg is used to calculate romcode boot time
	//         Please DO NOT remove it!
    serial_puts("\nTE : ");
    unsigned int nTEBegin = TIMERE_GET();
    serial_put_dec(nTEBegin);
    serial_puts("\nBT : ");
	//Note: Following code is used to show current uboot build time
	//         For some fail cases which in SPL stage we can not target
	//         the uboot version quickly. It will cost about 5ms.
	//         Please DO NOT remove it! 
	serial_puts(__TIME__);
	serial_puts(" ");
	serial_puts(__DATE__);
	serial_puts("\n");	

#ifdef CONFIG_POWER_SPL
    power_init();
#endif

#if !defined(CONFIG_VLSI_EMULATOR)
    // initial pll
    pll_init(&__plls);
	serial_init(__plls.uart);
#else
	serial_init(readl(P_UART_CONTROL(UART_PORT_CONS))|UART_CNTL_MASK_TX_EN|UART_CNTL_MASK_RX_EN);
	serial_puts("\n\nAmlogic log: UART OK for emulator!\n");
#endif //#if !defined(CONFIG_VLSI_EMULATOR)


	//TEMP add 
	unsigned int nPLL = readl(P_HHI_SYS_PLL_CNTL);
	unsigned int nDDRCLK = ((24 / ((nPLL>>9)& 0x1F) ) * (nPLL & 0x1FF))/ (1<<((nPLL>>16) & 0x3));
	serial_puts("\nCPU clock is ");
	serial_put_dec(nDDRCLK);
	serial_puts("MHz\n");


    nTEBegin = TIMERE_GET();

    // initial ddr
    ddr_init_test();

    serial_puts("\nDDR init use : ");
    serial_put_dec(get_utimer(nTEBegin));
    serial_puts(" us\n");

	//asm volatile ("wfi");
	
    nTEBegin = TIMERE_GET();
    // load uboot
#ifdef CONFIG_ENABLE_WATCHDOG
	if(load_uboot(__TEXT_BASE,__TEXT_SIZE)){
   		serial_puts("\nload uboot error,now reset the chip");
		writel((1<<22) | (3<<24)|1000, P_WATCHDOG_TC);
	}
#else
    	load_uboot(__TEXT_BASE,__TEXT_SIZE);
#endif

#if defined(CONFIG_AML_V2_USBTOOL)
    //tell uboot it loaded from internal device or external device
    if( 1 == romboot_info->boot_id )//see loaduboot.c, only boot from sdcard when "boot_id == 1"
    {
        writel(MESON_SDC_BURNER_REBOOT, CONFIG_TPL_BOOT_ID_ADDR);
    }
#endif//#if defined(CONFIG_AML_V2_USBTOOL)

    serial_puts("\nLoad UBOOT total use : ");
    serial_put_dec(get_utimer(nTEBegin));
    serial_puts(" us\n");

	//asm volatile ("wfi");
	// load secureOS
#ifdef CONFIG_MESON_TRUSTZONE
	if(load_secureos()){
		serial_puts("\nload secureOS fail,now reset the chip");
		ovFlag = 1;		
		writel((1<<22) | (3<<24)|1000, P_WATCHDOG_TC);
	}
	else{		
		ovFlag = 0;
		serial_puts("\nOV System Started\n");
		serial_wait_tx_empty();    
	}
#endif	

    serial_puts("\nSystem Started\n");

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	unsigned int spl_boot_end = TIMERE_GET();
	*((volatile unsigned int*)0x13a00000) = spl_boot_end;
	serial_puts("\nspl boot time(us):");
	//serial_put_dword((spl_boot_end-spl_boot_start));
#else
	*((volatile unsigned int*)0x1fa00000) = 0;
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

#ifdef CONFIG_M8
	//if bootup failed, switch to next boot device
	writel(0, P_WATCHDOG_TC); //disable watchdog
	//temp added
	writel(0,0xc8100000);
#endif

#ifdef CONFIG_MESON_TRUSTZONE		
    return ovFlag;
#else
	return 0;
#endif    
}

