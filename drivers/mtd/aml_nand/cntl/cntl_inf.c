/*
 * cntl_inf.c
 *
 *  Created on: Aug 22, 2011
 *      Author: jerry.yu
 */

#include <amlogic/nand/cntl.h>
#include <amlogic/nand/platform.h>
#include <malloc.h>
#include <errno.h>
#ifndef assert
#define assert(x) ((void)0)
#endif
#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE 1
#endif

#ifndef in_interrupt
#define in_interrupt() FALSE
#endif
static cntl_t * cntl=NULL;
uint32_t cntl_try_lock(void)
{
	/**
	 * @todo implement this function
	 */
	return -1;
}
void cntl_unlock(void)
{
	/**
	 * @todo implement this function
	 */
	return;
}
void cntl_init(struct aml_nand_platform * plat)
{

    cntl=get_v3();
    // struct aml_nand_platform *

    cntl_config(NAND_CNTL_INIT,plat);
    //uint16_t mode(0:async,1:sync mode,2 toggle),uint16_t t_rea,uint16_t t_rhoh,uint16_t sync_adjust(optional)
    cntl_config(NAND_CNTL_TIME_SET,0,0,0);
}

int32_t cntl_config(uint32_t config, ...)
{
	int32_t ret;
	va_list args;
	va_start(args,config);
	ret=cntl->config(cntl,config,args);
	va_end(args);
	return ret;
}
uint32_t cntl_size(void)
{
	assert(cntl!=NULL);
	return cntl->size(cntl);
}
uint32_t cntl_avail(void)
{
	assert(cntl!=NULL);
	return cntl->avail(cntl);
}
uint32_t cntl_head(void)
{
	assert(cntl!=NULL);
	return cntl->head(cntl);
}
uint32_t cntl_tail(void)
{
	assert(cntl!=NULL);
	return cntl->tail(cntl);
}

int32_t cntl_ctrl(uint16_t ce, uint16_t ctrl)
{
	assert(cntl!=NULL);
	return cntl->ctrl(cntl,ce,ctrl);
}
int32_t cntl_wait(uint8_t mode,uint16_t ce,uint8_t cycle_log2)
{
	assert(cntl!=NULL);
	return cntl->wait(cntl,mode,ce,cycle_log2);
}
int32_t cntl_nop(  uint16_t ce, uint16_t cycles)
{
	assert(cntl!=NULL);
	return cntl->nop(cntl,ce,cycles);

}
int32_t cntl_sts(  jobkey_t *job, uint16_t mode)
{
	assert(cntl!=NULL);
	return cntl->sts(cntl,job,mode);
}
int32_t cntl_readbytes(void * addr, dma_t dma_mode)
{
	assert(cntl!=NULL);
	return cntl->readbytes(cntl,addr,dma_mode);

}
int32_t cntl_writebytes( void * addr, dma_t dma_mode)
{
	assert(cntl!=NULL);
	return cntl->writebytes(cntl,addr,dma_mode);

}
int32_t cntl_readecc(void * addr, void * info,dma_t dma_mode)
{
	assert(cntl!=NULL);
	return cntl->readecc(cntl,addr,info,dma_mode);
}

int32_t cntl_writeecc(void * addr, void * info,dma_t dma_mode)
{
	assert(cntl!=NULL);
	return cntl->writeecc(cntl,addr,info,dma_mode);
}
jobkey_t *  cntl_job_get(uint32_t mykey)
{
	assert(cntl!=NULL);
	return cntl->job_get(cntl,mykey);
}
int32_t cntl_job_free( jobkey_t * job)
{
		assert(cntl!=NULL);
		return cntl->job_free(cntl,job);
}
int32_t cntl_job_lookup( jobkey_t ** jobs,uint32_t size)
{
	assert(cntl!=NULL);
	return cntl->job_lookup(cntl,jobs,size);
}


int32_t cntl_seed(  uint16_t seed)//0 disable
{
	assert(cntl!=NULL);
	return cntl->seed(cntl,seed);
}

int32_t cntl_ecc2dma(ecc_t * orig,dma_desc_t* dma,uint32_t size,uint32_t short_size,uint32_t seed)
{
	assert(cntl!=NULL);
	return cntl->ecc2dma(orig,dma,size,short_size,seed);
}

int32_t cntl_info2data(void * data,void * info,dma_t dma)//-1,found ecc fail,>=0,ecc counter .
{
	assert(cntl!=NULL);
	return cntl->info2data(data,info,dma);
}

int32_t cntl_data2info(void * info,void * data,dma_t dma)//-1,error found
{
	assert(cntl!=NULL);
	return cntl->data2info(info,data,dma);
}

int32_t cmdq_alloc(cmdq_t * cmdq)
{
	cmdq->cmd=(cmd_t*)malloc(cmdq->size*sizeof(cmd_t));
	if(cmdq->cmd==NULL)
		return -1;
	return 0;
}
void cmdq_free(cmdq_t * cmdq)
{
	if(cmdq->cmd)
		free(cmdq->cmd);
	cmdq->cmd=NULL;
}

int32_t cntl_write_cmd(cmdq_t * in,cmdq_t * out)
{
	assert(cntl!=NULL);
	int32_t ret=0;
	uint32_t step=0x10;
	if (in != NULL)
	{
		do
		{

			if (ret < 0 || out->cmd == NULL || in_interrupt()==FALSE)
			{
				cmdq_free(out);
				out->size += step;
				if (cmdq_alloc(out) < 0)
					return -1; //space not enough
			}

			ret = cntl->convert_cmd(in, out);
		} while (ret < 0 && in_interrupt()==FALSE);
	}
	if(ret==0)
	{
		if(cntl_try_lock())
			return -EAGAIN;
		ret=cntl->write_cmd(cntl,out);
		cntl_unlock();
	}
	return ret;
}
int32_t cntl_finish_jobs(void (* cb_finish)(uint32_t key,uint32_t st))
{
	assert(cb_finish!=NULL);
	jobkey_t * job;
	uint32_t key;
	uint32_t st;
	while(cntl_job_lookup(&job,1)>0)
	{
		if(cntl_try_lock())
			continue;
		key=cntl->job_key(cntl,job);
		st=cntl->job_status(cntl,job);
		cntl->job_free(cntl,job);
		cntl_unlock();
		cb_finish(key,st);
	}

	return 0;
}


