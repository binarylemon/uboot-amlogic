#define  CONFIG_AMLROM_SPL 1
#include <pll.c>
#include <timer.c>
#include <uartpin.c>
#include <pinmux.c>
#include <sdpinmux.c>
#include <serial.c>
#include <timming.c>
#include <memtest.c>
#include <ddr.c>

#include <sdio.c>
#include <debug_rom.c>

#include <loaduboot.c>

unsigned main(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
    int i;
	//Adjust 1us timer base
	timer_init();

	serial_init(UART_CONTROL_SET(CONFIG_BAUDRATE,CONFIG_CRYSTAL_MHZ*1000000));
	serial_put_dword(get_timer(0));
    writel(0,P_WATCHDOG_TC);//disable Watchdog
    debug_rom(__FILE__,__LINE__);
    // initial pll
    pll_init(&__plls);
	// initial ddr
    ddr_init_test();
    // load uboot
	load_uboot(__TEXT_BASE,__TEXT_SIZE);
	serial_puts("Systemp Started\n");
}
