#include <common.h>
#include <command.h>
#include <config.h>
#include <version.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <systest.h>


extern char systest_info_line[];

#define MAX_ITEM_NAME_LEN 20
#define ON 1
#define OFF 0

const static unsigned datapattern[] = {
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 
		0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
		0x10000, 0x20000, 0x40000, 0x80000,
		0x100000, 0x200000, 0x400000, 0x800000,
		0x1000000, 0x2000000, 0x4000000, 0x8000000,
		0x10000000, 0x20000000, 0x40000000, 0x80000000,
		0xaaaaaaaa, 0xcccccccc, 0xf0f0f0f0, 0xff00ff00,0xffff0000,
		0xffffffff, 0x00000000, 0x0000ffff, 0x00ff00ff,0x0f0f0f0f,
		0x33333333, 0x55555555	
};
const unsigned otherpattern = 0x01234567;

//==============================================================
static int mem_test_dataline(volatile unsigned *addr)
{
	int i;
    for(i=0; i<ARRAY_SIZE(datapattern);i++)
    {
        *addr = datapattern[i];
        if (*addr != datapattern[i]) 
        {
        	sprintf(systest_info_line, "Memory (dataline) error at 0x%x, wrote 0x%x, read 0x%x\n",
        												addr, datapattern[i], *addr);
        	systest_log(systest_info_line, SYSTEST_INFO_L2);
            return -1;            
        }        
    }
    return 1;
}
//==============================================================
static int mem_test_addrline(unsigned  *baseAddress, unsigned long nBytes)
{
    unsigned addressMask = (nBytes/sizeof(unsigned) - 1);
    unsigned offset;
    unsigned testOffset;

    unsigned pattern     =  0xAAAAAAAA;
    unsigned antipattern = 0x55555555;

    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        baseAddress[offset] = pattern;
    }

    testOffset = 0;
    baseAddress[testOffset] = antipattern;

    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        if (baseAddress[offset] != pattern)
        {
            sprintf(systest_info_line, "Memory (address line) error at address 0x%x, wrote 0x%x, read 0x%x\n", 
            												&(baseAddress[offset]), pattern, baseAddress[offset]);
            systest_log(systest_info_line, SYSTEST_INFO_L2);
            return -1;
        }
    }

    baseAddress[testOffset] = pattern;
   for (testOffset = 1; (testOffset & addressMask) != 0; testOffset <<= 1)
    {
        baseAddress[testOffset] = antipattern;

		if (baseAddress[0] != pattern)
		{
			sprintf(systest_info_line, "Memory (address line) error at address 0x%x, wrote 0x%x, read 0x%x\n", 
            												baseAddress, pattern, baseAddress[0]);
            systest_log(systest_info_line, SYSTEST_INFO_L2);
            return -1;			
		}

        for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
        {
            if ((baseAddress[offset] != pattern) && (offset != testOffset))
            {
               // return ((unsigned long *) &baseAddress[testOffset]);
                sprintf(systest_info_line, "Memory (address line) error at address 0x%x, wrote 0x%x, read 0x%x\n", 
            												&(baseAddress[offset]), pattern, baseAddress[offset]);
            	systest_log(systest_info_line, SYSTEST_INFO_L2);
            	return -1;
            }
        }

        baseAddress[testOffset] = pattern;
    }

    return 1;
}
//==============================================================
#ifdef CONFIG_ENABLE_MEM_DEVICE_TEST		
static int mem_test_device(volatile unsigned long * baseAddress, unsigned long nBytes)	
{
    unsigned long offset;
    unsigned long nWords = nBytes / sizeof(unsigned long);
    int i;
    unsigned long pattern;
    unsigned long antipattern;    

    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
        baseAddress[offset] = pattern;

    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        if (baseAddress[offset] != pattern)
        {
        		sprintf(systest_info_line, "Memory test error at address 0x%x, wrote 0x%lx, read 0x%lx\n", 
            												&baseAddress[offset], pattern, baseAddress[offset]);
            	systest_log(systest_info_line, SYSTEST_INFO_L2);
            	return -1;
          //  return ((unsigned long *) &baseAddress[offset]);
        }

        antipattern = ~pattern;
        baseAddress[offset] = antipattern;

    }
        
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        antipattern = ~pattern;
        if (baseAddress[offset] != antipattern)
        {
            //return ((datum *) &baseAddress[offset]);
            sprintf(systest_info_line, "Memory test error at address 0x%x, wrote 0x%lx, read 0x%lx\n", 
            												&baseAddress[offset], pattern, baseAddress[offset]);
            systest_log(systest_info_line, SYSTEST_INFO_L2);
            return -1;
        }
    }
    return 1;
}
#endif
//==============================================================
static int test_sub_nocache(void)
{
	int status;
	
	status = dcache_status();
	if(dcache_status() == OFF)
		dcache_enable();	
	dcache_flush();	
	icache_invalid();		
	dcache_clean();
	dcache_disable();
	 // must invalid dcache after dcache_disable	
	 // if no valid dcache, dcache_enable() will jump here
	dcache_invalid();  
	asm("mov r0, r0");
	asm("mov r0, r0");
	asm("mov r0, r0");
	
		if(mem_test_dataline((volatile unsigned *) PHYS_MEMORY_START) < 0){
			systest_log("memory data line test fail.\n", SYSTEST_INFO_L2);
			//return -1;
			goto fail;
		}
		
		systest_log("memory data line test pass.\n", SYSTEST_INFO_L2);		
		if(mem_test_addrline((volatile unsigned *)PHYS_MEMORY_START,PHYS_MEMORY_SIZE) < 0){
			systest_log("memory address line test fail. \n", SYSTEST_INFO_L2);
			//return -1;
			goto fail;
		}
		
		systest_log("memory address line test pass.\n", SYSTEST_INFO_L2);
#ifdef CONFIG_ENABLE_MEM_DEVICE_TEST		
		if(mem_test_device((volatile unsigned *)PHYS_MEMORY_START,PHYS_MEMORY_SIZE) < 0){
			systest_log("memory device test fail.\n", SYSTEST_INFO_L2);
			//return -1;
			goto fail;
		}
		systest_log("memory device test pass.\n", SYSTEST_INFO_L2);
#endif		
	if(status == ON)
		dcache_enable();
		return 1;		
		
fail:
	if(status == ON)
		dcache_enable();
	return -1;		
}
//==============================================================
// systest run memory
// systest run
//==============================================================
int memory_systest(int argc, char *argv[])
{
	if(argc < 2){
		if(test_sub_nocache() < 0)
			systest_log("memory: test fail.\n", SYSTEST_INFO_L1);	
		else
			systest_log("memory: test pass.\n", SYSTEST_INFO_L1);	
	}
	else{
		systest_log("memory: test fail: parameter number error.\n", SYSTEST_INFO_L1);
	}
	return 0;	
}