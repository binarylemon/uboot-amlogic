#include <common.h>
#include <nand.h>
#include <mmc.h>
#include <asm/arch/nand.h>

extern char systest_info_line[];
//============================================================================
static int nand_sub_test(nand_info_t *nand, int idx)
{
	if (!nand) {
		nand_init();
		if (!nand) {
			sprintf(systest_info_line, "%s:%d: NAND[device:%d]: no NAND device available.\n", __FILE__, __LINE__, idx);
			systest_log(systest_info_line, SYSTEST_INFO_L2);
			return -1;
		}
	}
	if (nand->name) {
		struct nand_chip *chip = nand->priv;					
/*#ifdef CONFIG_MTD_DEVICE
	sprintf(systest_info_line, "NAND: Device %d: %s, %s sector size %u KiB \n", idx, 
			nand->name, nand->info, nand->erasesize >> 10);
	systest_log(systest_info_line, SYSTEST_INFO_L2);
#else 
*/
	sprintf(systest_info_line, "NAND: Device %d: %s, sector size %u KiB \n", idx, 
			nand->name, nand->erasesize >> 10);
	systest_log(systest_info_line, SYSTEST_INFO_L2);			
//#endif
	}
	
	return 1;

}

//==================================================================================
int nand_systest(int argc, char *argv[])
{	
	int i, ret;
	nand_info_t *nand;
	if(argc < 2){
		ret = 0;
		for(i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE+1; i++){
			nand = &(nand_info[i]);
			if(nand_sub_test(nand, i) < 0)
				ret = -1;				
		}
		if(ret < 0)
			systest_log("NAND: test fail.\n", SYSTEST_INFO_L1);		
		else
			systest_log("NAND: test pass.\n", SYSTEST_INFO_L1);
		return 0;
	}
	else{
		systest_log("NAND: test fail: parameter number error.\n", SYSTEST_INFO_L1);
		return 0;
	}

}
