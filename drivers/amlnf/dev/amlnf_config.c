
#include "../include/amlnf_dev.h"
#include "../include/phynand.h"


extern struct partitions *part_table;
struct amlnf_partition * amlnand_config=NULL;

int amlnand_get_partition_table(struct amlnand_chip *aml_chip)
{	
	int ret=0, i;

	if(part_table == NULL){
		aml_nand_msg("part_table from ACS is NULL, do not init nand");
		return -NAND_FAILED;
	}
	
	amlnand_config = aml_nand_malloc(MAX_NAND_PART_NUM * sizeof(struct amlnf_partition));
	if(!amlnand_config){
		aml_nand_dbg("amlnand_config: malloc failed!");
		ret = -NAND_MALLOC_FAILURE;
	}
	
	//show_partition_table();

	memcpy(amlnand_config, part_table, (MAX_NAND_PART_NUM * sizeof(struct amlnf_partition)));
	
	aml_chip->h_cache_dev = 0;
	for (i = 0; i < MAX_NAND_PART_NUM; i++) {
		if (amlnand_config[i].mask_flags == STORE_CACHE) {
			aml_chip->h_cache_dev = 1;/*have cache dev*/
			aml_nand_msg("cache !!!");
		}
		if (amlnand_config[i].size == NAND_PART_SIZE_FULL)
			break;
	}

	return ret;
}




