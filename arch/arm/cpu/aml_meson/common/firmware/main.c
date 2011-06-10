#include <config.h>
#include <version.h>
#include <asm/arch/io.h>
#include <asm/arch/romboot.h>

#if CONFIG_SPL_DEBUGROM
unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
    	//Adjust 1us timer base
	timer_init();

    serial_init(UART_CONTROL_SET(CONFIG_BAUDRATE,CONFIG_CRYSTAL_MHZ*1000000));
	serial_put_dword(get_timer(0));
	writel(0,P_WATCHDOG_TC);//disable Watchdog	  
    debug_rom(__FILE__,__LINE__);

}

#else
unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
    int i;
	//Adjust 1us timer base
	timer_init();
	
	serial_init(UART_CONTROL_SET(CONFIG_BAUDRATE,CONFIG_CRYSTAL_MHZ*1000000));
	serial_put_dword(get_timer(0));

    if(serial_tstc())
    {
		writel(0,P_WATCHDOG_TC);//disable Watchdog	  
	    debug_rom(__FILE__,__LINE__);
    }


    // initial pll
    pll_init();
	// initial ddr
    ddr_init_test();
    // load uboot	
	load_uboot(__TEXT_BASE,__TEXT_SIZE);
	serial_puts("Systemp Started\n");
}
#endif	