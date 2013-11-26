/***********************************************
*****Storage config of board, for ACS use.*****
***********************************************/

#ifndef __STORAGE_H
#define __STORAGE_H

#include <linux/types.h>

#ifndef __ASSEMBLY__

//Partition table defines
#define 	NAND_PART_SIZE_FULL  		-1
#define 	MAX_PART_NUM			16
#define	 MAX_PART_NAME_LEN		 16
#define 	SZ_1M 					0x100000

#define 	STORE_CODE 				1
#define 	STORE_DATA				(1<<1)

#define SPI_BOOT_FLAG 			0
#define NAND_BOOT_FLAG 		1
#define EMMC_BOOT_FLAG 		2
#define CARD_BOOT_FLAG 		3
#define SPI_NAND_FLAG			4
#define SPI_EMMC_FLAG			5

struct partitions {
	char name[MAX_PART_NAME_LEN];			/* identifier string */
	uint64_t size;			/* partition size */
	uint64_t offset;		/* offset within the master space */
	unsigned mask_flags;		/* master flags to mask out for this partition */
};

struct config_nand {
	unsigned enable_slc;
	unsigned order_ce;
	unsigned reserved[2];
};

struct config_mmc {
	unsigned type;
	unsigned port;
	unsigned reserved[2];
};

struct store_config {
	unsigned  store_device_flag;			// indicate storage devices on each board
	struct config_nand  nand_configs;			// specital config for nand
	struct config_mmc  mmc_configs;			// specital config for mmc
};

#endif
#endif
