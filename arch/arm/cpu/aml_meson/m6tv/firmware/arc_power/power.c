/*Power manager for M6TV*/
//#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/reg_addr.h>
void power_off_at_24M()
{	

	serial_put_hex(readl(P_AO_RTI_PIN_MUX_REG),32);
	f_serial_puts(" \n");
	serial_put_hex(readl(P_AO_GPIO_O_EN_N),32);
	f_serial_puts(" \n");
	serial_put_hex(readl(P_AO_GPIO_I),32);
	f_serial_puts(" \n");
	wait_uart_empty();
	
	writel(readl(P_AO_GPIO_O_EN_N) & (~(1<<25)),P_AO_GPIO_O_EN_N);
	udelay(200);
	writel(readl(P_AO_GPIO_O_EN_N) & (~(1<<9)),P_AO_GPIO_O_EN_N);
	udelay(200);
	writel(readl(P_AO_GPIO_O_EN_N) & (~(1<<21)),P_AO_GPIO_O_EN_N);
	udelay(200);
	writel(readl(P_AO_GPIO_O_EN_N) & (~(1<<5)),P_AO_GPIO_O_EN_N);
	serial_put_hex(readl(P_AO_GPIO_O_EN_N),32);
	f_serial_puts(" \n");
	wait_uart_empty();

}
void power_on_at_24M()
{

	writel(readl(P_AO_GPIO_O_EN_N) | (1<<21),P_AO_GPIO_O_EN_N);
	udelay(200);
	writel(readl(P_AO_GPIO_O_EN_N) | (1<<5),P_AO_GPIO_O_EN_N);
	udelay(24000000);
	writel(readl(P_AO_GPIO_O_EN_N) | (1<<25),P_AO_GPIO_O_EN_N);
	udelay(200);
	writel(readl(P_AO_GPIO_O_EN_N) | (1<<9),P_AO_GPIO_O_EN_N);
	
	serial_put_hex(readl(P_AO_GPIO_O_EN_N),32);
	f_serial_puts(" \n");
	wait_uart_empty();
}

void power_off_at_32k()
{
}

void power_on_at_32k()
{
}

