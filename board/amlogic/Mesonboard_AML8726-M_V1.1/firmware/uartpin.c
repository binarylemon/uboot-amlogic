#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
SPL_STATIC_FUNC int serial_set_pin_port(unsigned port_base)
{
#warning todo warning 
    if(port_base==UART_PORT_0)
    {
        return 0;
    }
    if(port_base==UART_PORT_1)
    {
        return 0;
    }
    return -1;
}