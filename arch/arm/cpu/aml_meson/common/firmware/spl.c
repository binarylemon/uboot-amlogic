#define  CONFIG_AMLROM_SPL 1
#include <timer.c>
#include <timming.c>
#include <uartpin.c>
#include <serial.c>
#include <pinmux.c>
#include <sdpinmux.c>
#include <memtest.c>
#include <pll.c>
#ifdef CONFIG_POWER_SPL
#include <hardi2c_lite.c>
#include <power.c>
#endif
#include <ddr.c>
//#include <mtddevices.c>
#include <sdio.c>
//#include <debug_rom.c>
#include <arch_init.c>

extern void ipl_memcpy(void*, const void *, __kernel_size_t);
#define memcpy ipl_memcpy

#include <loaduboot.c>
#ifdef CONFIG_ACS
#include <storage.c>
#include <acs.c>
#endif

#include <asm/arch/reboot.h>

#ifdef CONFIG_POWER_SPL
#include <amlogic/aml_pmu_common.h>
#endif

#ifdef CONFIG_MESON_TRUSTZONE
#include <secureboot.c>
unsigned int ovFlag;
#endif

void boot_info(void);
void cpu_info(void);

unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
	//arch related init
	arch_init();

	//serial init
	serial_init(readl(P_UART_CONTROL(UART_PORT_CONS))|UART_CNTL_MASK_TX_EN|UART_CNTL_MASK_RX_EN);

	//print boot time, build time
	boot_info();

#ifdef CONFIG_POWER_SPL
	power_init(POWER_INIT_MODE_NORMAL);
#endif

#if !defined(CONFIG_VLSI_EMULATOR)
	// initial pll
	pll_init(&__plls);

#if !defined(CONFIG_M3)
	serial_init(__plls.uart);
#endif

#else
	serial_init(readl(P_UART_CONTROL(UART_PORT_CONS))|UART_CNTL_MASK_TX_EN|UART_CNTL_MASK_RX_EN);
	serial_puts("\n\nAmlogic log: UART OK for emulator!\n");
#endif //#if !defined(CONFIG_VLSI_EMULATOR)

	//print cpu clk
	cpu_info();

	//init ddr and print init time
	unsigned int nTEBegin = TIMERE_GET();
	ddr_init_test();
	serial_puts("\nDDR init use : ");
	serial_put_dec(get_utimer(nTEBegin));
	serial_puts(" us\n");

#if defined(CONFIG_AML_A5) && defined(CONFIG_AML_SECU_BOOT_V2) && defined(CONFIG_AML_SPL_L1_CACHE_ON)
    asm volatile ("ldr	sp, =(0x12000000)");
    //serial_puts("aml log : set SP to 0x12000000\n");
#endif

	//asm volatile ("wfi");
	    
    // load uboot
#ifdef CONFIG_ENABLE_WATCHDOG
	if(load_uboot(__TEXT_BASE,__TEXT_SIZE)){
   		serial_puts("\nload uboot error,now reset the chip");
		AML_WATCH_DOG_START();
	}
#else
    load_uboot(__TEXT_BASE,__TEXT_SIZE);
#endif


    serial_puts("\nTE : ");
	serial_put_dec(TIMERE_GET());
	serial_puts("\n");

	//asm volatile ("wfi");
	// load secureOS
#ifdef CONFIG_MESON_TRUSTZONE
	if(load_secureos()){
#ifdef CONFIG_BL31
		serial_puts("\nload BL31 fail,now reset the chip");
#else
		serial_puts("\nload secureOS fail,now reset the chip");
#endif
		ovFlag = 1;		
		AML_WATCH_DOG_START();
	}
	else{		
		ovFlag = 0;
#ifdef CONFIG_BL31
		serial_puts("\nBL31 System Started\n");
#else
		serial_puts("\nOV System Started\n");
#endif
		serial_wait_tx_empty();    
	}
#endif	

    serial_puts("\nSystem Started\n");

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	unsigned int spl_boot_end = TIMERE_GET();
	serial_puts("\ntime: spl boot time(us):");
	serial_put_dec(spl_boot_end);
	//serial_put_dword((spl_boot_end));
#endif

#if (MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON8)
	//if bootup failed, switch to next boot device
	AML_WATCH_DOG_DISABLE(); //disable watchdog
	//temp added
	writel(0,P_AO_RTI_STATUS_REG0);
#endif

#if defined(CONFIG_AML_A5) && defined(CONFIG_AML_SECU_BOOT_V2) && defined(CONFIG_AML_SPL_L1_CACHE_ON)

    unsigned int fpAddr = CONFIG_SYS_TEXT_BASE;

#ifdef CONFIG_IMPROVE_UCL_DEC
    fpAddr = CONFIG_SYS_TEXT_BASE+0x800000;
#endif

#ifdef CONFIG_MESON_TRUSTZONE
    fpAddr = SECURE_OS_DECOMPRESS_ADDR;
#endif

    typedef  void (*t_func_v1)(void);
    t_func_v1 fp_program = (t_func_v1)fpAddr;
    //here need check ?
    fp_program();
#endif

#ifdef CONFIG_MESON_TRUSTZONE		
    return ovFlag;
#else
	return 0;
#endif    
}

void boot_info(void) {
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
}

void cpu_info(void) {
    //print cpu info
    unsigned int nPLL = readl(P_HHI_A9_CLK_CNTL);
    unsigned int nA9CLK = CONFIG_CRYSTAL_MHZ;
	if ((nPLL & (1<<7)) && (nPLL & (1<<0)))
    {
        nPLL = readl(P_HHI_SYS_PLL_CNTL);
        nA9CLK = ((24 / ((nPLL>>9)& 0x1F) ) * (nPLL & 0x1FF))/ (1<<((nPLL>>16) & 0x3));
    }
    serial_puts("\nCPU clock is ");
    serial_put_dec(nA9CLK);
    serial_puts("MHz\n\n");
}
