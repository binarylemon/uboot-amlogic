#include <config.h>
#include <asm/arch/io.h>
void reset_cpu(ulong addr)
{
    setbits_le32(P_WATCHDOG_TC,1<<WATCHDOG_ENABLE_BIT);
#if defined(CONFIG_M6) || defined(CONFIG_M8)
		writel((1<<22) | (3<<24), P_WATCHDOG_TC);
#endif
    while(1);
}

