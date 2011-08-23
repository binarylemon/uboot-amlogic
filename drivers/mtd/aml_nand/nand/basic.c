/*
 * basic.c
 *
 *  Created on: Aug 23, 2011
 *      Author: jerry.yu
 */
#include <linux/types.h>
#include <amlogic/nand/cntl.h>
#include <amlogic/nand/platform.h>
#include <amlogic/nand/types.h>
#include <asm/dma-mapping.h>

typedef struct __nand_command_s nand_cmd_t;
struct __nand_command_s{
	int32_t stat;
	void* 	para;
};
typedef struct nand_cfg_s nand_cfg_t;
typedef struct nand_ce_s nand_ce_t;
struct nand_ce_s{
	unsigned dummy;
};
struct nand_cfg_s{
	nand_dev_t type;
	uint32_t   ce_mask;
	nand_ce_t  chip[4];
};
struct id_read_s{
	uint64_t id;
	uint32_t onfi;
};
static nand_cfg_t nand_cfg;

#define min(a,b)	(a)<(b)?(a):(b)
#define NAND_CLE(a)		((a)|0x100)
#define NAND_CMD_RESET 	NAND_CLE(0xff)
#define NAND_CMD_STATUS NAND_CLE(0x70)
int32_t nand_reset_identy(nand_cfg_t * cfg,struct aml_nand_platform * plat,cntl_t *cntl)
{
	int32_t num,i,max_ce;
	void * addr;
	max_ce=min(cntl->feature&FEATURE_SUPPORT_MAX_CES,plat->ce_num?plat->ce_num:FEATURE_SUPPORT_MAX_CES);
	struct id_read_s *id;
	addr=dma_alloc_coherent(max_ce*sizeof(struct id_read_s),(dma_addr_t *)&id);
	for(i=0;i<max_ce;i++)
	{
		cntl_ctrl(i,NAND_CMD_RESET);
	}
	for(i=0;i<max_ce;i++)
	{

#if 0
		cntl_ctrl(i,NAND_CMD_STATUS);
		cntl_wait(RBIO)
		cntl_ctrl(i,NAND_CMD_ID);
		cntl_dma_id();
		cntl_ctrl(i,)
#endif
	}


	return num;
}
int32_t nand_probe(struct aml_nand_platform * plat)
{

	if(cntl_init(plat)<0)
		return -1;
	if(nand_reset_identy(&nand_cfg,plat,cntl_get())<0)
		return -1;
	return 0;
}
