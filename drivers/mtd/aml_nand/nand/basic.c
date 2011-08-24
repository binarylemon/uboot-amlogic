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
#include "nand_cmd.h"
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
	uint64_t onfi;
};
static nand_cfg_t nand_cfg;

#define min(a,b)	(a)<(b)?(a):(b)
int32_t nand_reset_identy(nand_cfg_t * cfg,struct aml_nand_platform * plat,cntl_t *cntl)
{
	int32_t num,i,max_ce;
	void * addr;
	max_ce=min(cntl->feature&FEATURE_SUPPORT_MAX_CES,plat->ce_num?plat->ce_num:FEATURE_SUPPORT_MAX_CES);
	struct id_read_s *id;
	addr=dma_alloc_coherent(max_ce*sizeof(struct id_read_s),(dma_addr_t *)&id);
	jobkey_t * job=cntl_job_get(-1);
	for(i=0;i<max_ce;i++)
	{
		cntl_ctrl(i,NAND_CLE(NAND_CMD_RESET));
		cntl_ctrl(i,NAND_CLE(NAND_CMD_STATUS));
	}
	for(i=0;i<max_ce;i++)
	{
		cntl_wait(NAND_RB_IO,IO6,16);//wait for 1M/16 nand cycle , about 1sec
		/// read uni id
		cntl_ctrl(i,NAND_CLE(NAND_CMD_READID));
		cntl_ctrl(i,NAND_ALE(0));
		cntl_readbytes(&id[i].id,sizeof(id[i].id));
		/// read onfi id
		cntl_ctrl(i,NAND_CLE(NAND_CMD_READID));
		cntl_ctrl(i,NAND_ALE(0x20));
		cntl_readbytes(&id[i].onfi,sizeof(id[i].onfi));
	}
	cntl_sts(job,STS_NO_INTERRUPT);
	while(cntl_job_status(job,-1)<0);
	cntl_job_free(job);
/**
 * @todo implement this function
	if(nand_cfg_set(&nand_cfg,0,id)<0)
		return -1;
*/
	for(i=0;i<max_ce;i++)
	{
		nanddebug("CE%d:id=%llx,onfi=%llx",id[i].id,id[i].onfi);
	}
	nand_cfg.ce_mask=1;
	num=1;
	for(i=1;i<max_ce;i++)
	{
		if(id[i].id!=id[0].id||id[i].onfi!=id[0].onfi)
		{
			nand_cfg.ce_mask&=~(1<<i);
			continue;
		}
		nand_cfg.ce_mask|=(1<<i);
		num++;
	}

//	dma_free_coherent(addr);



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
