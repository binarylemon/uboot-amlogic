/***********************************************
*****Storage config of board, for ACS use.*****
Header file: arch/arm/include/asm/arch-xx/storage.h
***********************************************/

#include <asm/arch/storage.h>

#ifdef CONFIG_ACS
//partition tables
struct partitions partition_table[]={
		{
			.name = "boot",
			.size = 9*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "system",
			.size = 152*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "boot1",
			.size = 9*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "userdata",
			.size = NAND_PART_SIZE_FULL,
			.mask_flags = STORE_DATA,
		},
};

struct store_config  store_configs ={
		.store_device_flag = NAND_BOOT_FLAG,
		.nand_configs = {
			.enable_slc = 0,
			.order_ce = 0,
			.reserved[0] = 0,
			.reserved[1] = 0,
		},
		.mmc_configs = {
			.type = (PORT_A_CARD_TYPE | (PORT_B_CARD_TYPE << 4) | (PORT_C_CARD_TYPE << 8)),
			.port = 0,
			.reserved[0] = 0,
			.reserved[1] = 0,
		},
};


#endif
