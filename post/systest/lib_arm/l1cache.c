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

#define no_cache_mem_start 0xa1000000
#define cache_mem_start 0x81000000
#define cache_size  0x4000

int sum = 0;
//===================================================================================

const unsigned L1_cache_pattern[]={
    0xaaaaaaaa,
    0xdd22ee11, 0x7788bb44, 0x337755aa, 0xff00aa55,
    0xff000000, 0x000000ff, 0x00ffffff, 0xffffff00,
    0x01000200, 0x04000800, 0x10002000, 0x40008000,
    0xfefffbff, 0xf7fffeff, 0xefffbfff, 0x7fffdfff,
    0x00100020, 0x00400080, 0x00100020, 0x00400800,
    0xfffefffb, 0xfff7fffe, 0xffefffbf, 0xff7fffdf,
    0x55aa00ff, 0xaa0055ff, 0x55ffaa00, 0xff00aa55,
    0xdd22ee11, 0x7788bb44, 0xdd22ee11, 0x7788bb44,

    0x01fe00ff, 0x01fe00ff, 0x01fe00ff, 0x01fe00ff,
    0x02fc00ff, 0x02fc00ff, 0x02fc00ff, 0x02fc00ff,
    0x04fb00ff, 0x04fb00ff, 0x04fb00ff, 0x04fb00ff,
    0x08f700ff, 0x08f700ff, 0x08f700ff, 0x08f700ff,
    0x10ef00ff, 0x10ef00ff, 0x10ef00ff, 0x10ef00ff,
    0x20df00ff, 0x20df00ff, 0x20df00ff, 0x20df00ff,
    0x40bf00ff, 0x40bf00ff, 0x40bf00ff, 0x40bf00ff,
    0x807f00ff, 0x807f00ff, 0x807f00ff, 0x807f00ff,

    0xfe01ff00, 0xfe01ff00, 0xfe01ff00, 0xfe01ff00,
    0xfd02ff00, 0xfd02ff00, 0xfd02ff00, 0xfd02ff00,
    0xfb04ff00, 0xfb04ff00, 0xfb04ff00, 0xfb04ff00,
    0xf708ff00, 0xf708ff00, 0xf708ff00, 0xf708ff00,
    0xef10ff00, 0xef10ff00, 0xef10ff00, 0xef10ff00,
    0xdf20ff00, 0xdf20ff00, 0xdf20ff00, 0xdf20ff00,
    0xbf40ff00, 0xbf40ff00, 0xbf40ff00, 0xbf40ff00,
    0x7f80ff00, 0x7f80ff00, 0x7f80ff00, 0x7f80ff00
    };
//==============================================================
unsigned test_w_l1cache(unsigned fill_value, unsigned modify_value)
{
	unsigned *addr;
	unsigned size, err_addr, val;
	int i;

	// current dcache is disable
	// clear no-cache memory block
	addr = (unsigned*)no_cache_mem_start;
	size = (cache_size)/sizeof(unsigned);
	for(i=0; i<size; i++, addr++)
		*addr = fill_value;
		
	// map cache-memory data to cache
	addr = (unsigned*)cache_mem_start;
	size = cache_size/CONFIG_SYS_CACHE_LINE_SIZE;	
	dcache_enable();		
	for(i=0; i<size; i++, addr+=CONFIG_SYS_CACHE_LINE_SIZE)
		val = *addr;
		
	// write to cache
	addr = (unsigned*)cache_mem_start;	
	size = cache_size/sizeof(unsigned);			
	for(i=0; i<size; i++, addr++){
		*addr = modify_value;			
	}		
		
	dcache_flush();
	dcache_clean();
	dcache_disable();
	dcache_invalid();
	
	asm("mov r0, r0");
	asm("mov r0, r0");
	asm("mov r0, r0");
	
	err_addr = 0;
	addr = (unsigned*)no_cache_mem_start;	
	for(i=0; i<size; i++, addr++){
		if(*addr != modify_value){							
			err_addr = addr;			
			break;
		}
	}	
	return err_addr;	
	
}

//==============================================================
int test_sub_l1cache(void)
{
	int i;
	unsigned result, pattern=0;
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
	
	for(i=0; i<ARRAY_SIZE(L1_cache_pattern); i++){			
		result = test_w_l1cache(0x55555555, L1_cache_pattern[i]);			
		if(result != 0){						
			pattern = L1_cache_pattern[i];
			break;
		}		
		result = test_w_l1cache(0x55555555, ~L1_cache_pattern[i]);				
		if(result != 0){		
			pattern = ~L1_cache_pattern[i];
			break;
		}			
	}
	
	if(status == ON)
		dcache_enable();
		
	if(i<ARRAY_SIZE(L1_cache_pattern)){
		sprintf(systest_info_line, "%s:%d: l1cache: test fail: Error address=0x%x, pattern=0x%x\n",
													__FILE__, __LINE__, result, pattern);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return -1;
	}
	else{
		sprintf(systest_info_line, "l1cache test pattern count=%d\n", ARRAY_SIZE(L1_cache_pattern));
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return 1;	
	}	
	
}

//==============================================================
int l1cache_systest(int argc, char *argv[])
{

	if(argc < 2){								
		if(test_sub_l1cache() < 0)
			systest_log("l1cache: test fail.\n", SYSTEST_INFO_L1);	
		else
			systest_log("l1cache: test pass.\n", SYSTEST_INFO_L1);			
	}
	else
		systest_log("l1cache: test fail: parameter number error.\n", SYSTEST_INFO_L1);
	
	return 0;
	
}