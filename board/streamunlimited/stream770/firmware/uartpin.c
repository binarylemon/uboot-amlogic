#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
SPL_STATIC_FUNC int serial_set_pin_port(unsigned port_base)
{
	// UART C
	// GPIODV_24==tx,GPIODV_25==rx
	setbits_le32(P_PERIPHS_PIN_MUX_6, 3 << 22);
	return 0;
}
