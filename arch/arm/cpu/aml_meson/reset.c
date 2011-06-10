#include <config.h>
#include <asm/arch/io.h>
void reset_cpu(ulong addr)
{
    setbits_le32(P_WATCHDOG_TC,1<<WATCHDOG_ENABLE_BIT);
    while(1);
}

     