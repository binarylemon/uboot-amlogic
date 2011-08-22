#include <common.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/arch/io.h>
#include <systest.h>

#define MAX_ITEM_NAME_LEN 20
#define ON 1
#define OFF 0

extern char systest_info_line[];

struct bist_func{
	char name[MAX_ITEM_NAME_LEN];
	int (*func)(int argc, char* argv[]);	
};

const char l1icache_cmd[] = {
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
	0x1, 0x6, 0x7, 0x6, 0x7, 0x6, 0x7, 0x6,
	0x7, 0x2, 0x3, 0x2, 0x3, 0x6, 0x7, 0x6,
	0x7, 0x6, 0x7, 0x4, 0x5, 0x4, 0x5, 0x0, 
	0x1, 0x0, 0x1, 0x4, 0x5, 0x2, 0x3, 0x0, 0x1,
};

const char l1dcache_cmd[] = {
		0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
		0x1, 0x6, 0x7, 0x7, 0x6, 0x7, 0x6, 0x7,
		0x6, 0x7, 0x2, 0x3, 0x2, 0x3, 0x6, 0x7,
		0x2, 0x3, 0x2, 0x3, 0x3, 0x0, 0x1, 0x1,
		0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x4, 0x5,
		0x2, 0x3, 0x3, 0x0, 0x1
};
//=========================================================================
// l1icache
static int bist_l1icache(int argc, char* argv[])
{
	int status, i, ret;
	unsigned val;
	
	status = icache_status();
	//if(status == OFF){
	//	 icache_invalid();
	//	 icache_enable();
	//}
	for(i=0;  i<ARRAY_SIZE(l1icache_cmd);  i++)
		WRITE_CBUS_REG(ISA_BIST_REG0, l1icache_cmd[i]);
	
	udelay(1000);
	WRITE_CBUS_REG(ISA_BIST_REG0, 3);
	udelay(1000);
	val = READ_CBUS_REG(ISA_BIST_REG1);
	if(val & 0x20000){
		sprintf(systest_info_line, "%s:%d: bist:l1icache: test fail.\n", __FILE__, __LINE__);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		ret=-1;
	}
	else{
		sprintf(systest_info_line, "%s:%d: bist:l1dcache: test fail.\n", __FILE__, __LINE__);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		ret=-1;
	}	
	
	if(status == OFF)
		icache_disable();
	return ret;
}

//=========================================================================
static int bist_l1dcache(int argc, char* argv[])
{
	int status, i, ret;
	unsigned val;
	
	status = dcache_status();
	//if(status == OFF){
	//	dcache_flush();
	//	dcache_enable();		
	//}
	for(i=0;  i < ARRAY_SIZE(l1dcache_cmd); i++)
		WRITE_CBUS_REG(ISA_BIST_REG0, l1dcache_cmd[i])	;
	
	udelay(1000);
	WRITE_CBUS_REG(ISA_BIST_REG0, 3);
	udelay(1000);
	val = READ_CBUS_REG(ISA_BIST_REG1);
	if(val & 0x4000){
		sprintf(systest_info_line, "%s:%d: bist:l1dcache: test fail.\n", __FILE__, __LINE__);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		ret=-1;
	}
	else{
		sprintf(systest_info_line, "%s:%d: bist:l1dcache: test pass.\n", __FILE__, __LINE__);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		ret = 0;
		//systest_out(__FILE__, __LINE__, "bist[l1dcache]", "pass");
	}
		
	if(status == OFF)
		dcache_disable();	
	return ret;
}

//=========================================================================
struct bist_func bist_t [] = {
	// L1 icache bist
	{
		.name = "l1icache",
		.func = bist_l1icache,
	},
	// L1 dcache bist
	{
		.name = "l1dcache",
		.func = bist_l1dcache,
	},	
};

//=========================================================================
// bist l1icache/l1dcache
int bist_systest(int argc, char* argv[])
{
	int i, j;
	struct bist_func *pfunc = NULL;
	int ret;
	if(argc < 2){   
		ret = 0;
		for(i=0; i<ARRAY_SIZE(bist_t); i++){
			if(bist_t[i].func(0, NULL) < 0){
				sprintf(systest_info_line, "bist %s: test fail.\n", bist_t[i].name);
				ret = -1;
			}
			else
				sprintf(systest_info_line, "bist %s: test pass/\n", bist_t[i].name)	;
			
			systest_log(systest_info_line, SYSTEST_INFO_L2);			
		}
		if(ret < 0)
			systest_log("bist: test fail.\n", SYSTEST_INFO_L1);
		else
			systest_log("bist: test pass.\n", SYSTEST_INFO_L1);
		
		return 0;
	}
	if(argc > 1){
		ret = 0;
		for(i=1; i<argc; i++){
			pfunc = NULL;			
			for(j=0; j<ARRAY_SIZE(bist_t); j++){
				if(strcmp(argv[1], bist_t[i].name) == 0){
					pfunc = &(bist_t[i]);
					break;
				}			
			}
			if(pfunc){
				if(pfunc->func(argc-i, argv+i) < 0){
					sprintf(systest_info_line, "bist: %s: test fail.\n", argv[i]);
					systest_log(systest_info_line, SYSTEST_INFO_L2);
					ret = -1;
				}
				else{
						sprintf(systest_info_line, "bist: %s: test pass.\n", argv[i]);
						systest_log(systest_info_line, SYSTEST_INFO_L2);
				}
			}
			else{
					sprintf(systest_info_line, "bist: %s not found the bist item.\n", argv[i]);
					systest_log(systest_info_line, SYSTEST_INFO_L2);					
					ret = -1;
			}				
		}
		if(ret < 0)
			systest_log("bist: test fail.\n", SYSTEST_INFO_L1);
		else
			systest_log("bist: test pass.\n", SYSTEST_INFO_L1);
	}	

	return 0;
}

