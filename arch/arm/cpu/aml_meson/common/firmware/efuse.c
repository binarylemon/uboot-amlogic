#include <asm/arch/reg_addr.h>
#include <asm/arch/io.h>

#define EFUSE_DEBUG

// suppose src is 4B align, size is 4B align
void efuse_read(unsigned *dest, unsigned src, unsigned size)
{
	int i;
	volatile unsigned dummy;
	
	// efuse hardware init
	writel((readl(P_EFUSE_CNTL4)& ~(1<<10)), P_EFUSE_CNTL4);  // Disable encryption
	writel((readl(P_EFUSE_CNTL1)& ~(1<<27)), P_EFUSE_CNTL1);  // Clear PowerDown flag
	writel((readl(P_EFUSE_CNTL1)|(1<<24)), P_EFUSE_CNTL1);    //Enable autoread mode
	
	// set src addr
	writel((readl(P_EFUSE_CNTL1)& ~(0x3FF<<0))|(src&0x3FF), P_EFUSE_CNTL1);
	writel((readl(P_EFUSE_CNTL1)|(1<<11)), P_EFUSE_CNTL1);
	writel((readl(P_EFUSE_CNTL1)& ~(1<<11)), P_EFUSE_CNTL1);
	
	 for(i=0; i< (size>>2); i++){
	 	writel((readl(P_EFUSE_CNTL1)| (1<<25)), P_EFUSE_CNTL1);
	 	writel((readl(P_EFUSE_CNTL1)& ~(1<<25)), P_EFUSE_CNTL1);
	 	
	 	// make eclipse happy
	 	do{
	 		dummy = readl(P_EFUSE_CNTL1);
	 		dummy = readl(P_EFUSE_CNTL1);
	 	}while(dummy & (1<<26));
	 	
	 	dest[i] = readl(P_EFUSE_CNTL2);
	}
	
}