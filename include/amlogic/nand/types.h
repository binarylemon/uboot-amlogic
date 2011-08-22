#ifndef __AMLOGIC_NAND_TYPES_H_
#define __AMLOGIC_NAND_TYPES_H_
#include <linux/types.h>
typedef struct __nand_flash_dev_s nand_dev_t;
#define MAX_ID_LEN  8
struct __nand_flash_dev_s {
	char *name;
	union{
		uint8_t id[MAX_ID_LEN];
		uint64_t id64;
	}id;
	uint64_t id64_mask;
	uint64_t chipsize;
	uint64_t feature;
	uint32_t pagesize;
	uint32_t erasesize;
	uint32_t oobsize;
	uint16_t dienum;
	uint16_t planenum;



};








#endif
