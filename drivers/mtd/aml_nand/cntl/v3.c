#include <stdarg.h>
#include <common.h>
#define assert(x) ((void)0)
#include <amlogic/nand/cntl.h>
#include <amlogic/nand/platform.h>
#include <asm/io.h>
#include <asm/arch/nand.h>
static ecc_t ecc_table[] ={
    { .name = "BCH OFF  ", .mode = 0, .bits = 0,  .data = 0,    .parity = 0,    .info = 0,.max=(1<<14)-1 },
    { .name = "BCH 8/512", .mode = 1, .bits = 8,  .data = 64,   .parity = 14,   .info = 2,.max=0x3f*512  },
    { .name = "BCH  8/1k", .mode = 2, .bits = 8,  .data = 128,  .parity = 14,   .info = 2,.max=0x3f*1024 },
    { .name = "BCH 16/1k", .mode = 3, .bits = 16, .data = 128,  .parity = 28,   .info = 2,.max=0x3f*1024 },
    { .name = "BCH 24/1k", .mode = 4, .bits = 24, .data = 128,  .parity = 42,   .info = 2,.max=0x3f*1024 },
    { .name = "BCH 30/1k", .mode = 5, .bits = 30, .data = 128,  .parity = 54,   .info = 2,.max=0x3f*1024 },
    { .name = "BCH 40/1k", .mode = 6, .bits = 40, .data = 128,  .parity = 70,   .info = 2,.max=0x3f*1024 },
    { .name = "BCH 60/1k", .mode = 7, .bits = 60, .data = 128,  .parity = 106,  .info = 2,.max=0x3f*1024 },
    { .name = NULL, .mode = 0, .bits = 0, .data = 0, .parity = 0,.info = 0 },
};
static int32_t v3_config(cntl_t *, uint32_t config, ...); //done the basic
static int32_t v3_fifo(cntl_t *, uint32_t config, ...);
static uint32_t v3_size(cntl_t *);
static uint32_t v3_avail(cntl_t *);
static uint32_t v3_head(cntl_t *);
static uint32_t v3_tail(cntl_t *);
static int32_t v3_ctrl(cntl_t *, uint16_t ce, uint16_t ctrl);
static int32_t v3_wait(cntl_t *, uint8_t mode,uint16_t ce,uint8_t cycle_log2);
static int32_t v3_nop(cntl_t *, uint16_t ce, uint16_t cycles);
static int32_t v3_sts(cntl_t *, jobkey_t *job, uint16_t mode);
static int32_t v3_readbytes(cntl_t  *,void * addr, dma_t dma_mode);
static int32_t v3_writebytes(cntl_t *,void * addr, dma_t dma_mode);
static int32_t v3_readecc(cntl_t * , void * addr, void * info,dma_t dma_mode);
static int32_t v3_writeecc(cntl_t *, void * addr, void * info,dma_t dma_mode);
static jobkey_t * v3_job_get(cntl_t * cntl_t,uint32_t mykey);
static int32_t v3_job_select(cntl_t * cntl_t, jobkey_t * job);
static int32_t v3_job_finish(cntl_t * cntl_t, jobkey_t * job);
static int32_t v3_job_status(cntl_t * cntl_t, jobkey_t * job);
static int32_t v3_ecc2dma(ecc_t * orig,dma_desc_t* dma,uint32_t size,uint32_t short_size,uint32_t seed);
static int32_t v3_info2data(void * data,void * info,dma_t dma);//-1,found ecc fail,>=0,ecc counter .
static int32_t v3_data2info(void * info,void * data,dma_t dma);//-1,error found

typedef struct {
    uint8_t st[2];
    uint8_t seq;
    uint8_t done;
    uint32_t key;
}sts_t;
struct v3_priv
{
	uint32_t    delay;
	uint32_t    edo;
	uint32_t    reg_base;
	uint32_t    fifo_mode;
	uint32_t    sts_size;
	uint32_t    temp;
	sts_t*      sts_buf;
	uint32_t    fifo_size;
	uint32_t *  cmd_fifo;
	struct aml_nand_platform * plat;
};
#define v3_lock_sts_buf(priv)
#define v3_unlock_sts_buf(priv)
static struct v3_priv v3_priv =
{ .delay = 90, .edo = 2, .reg_base = P_NAND_CMD,

};
#define DEFINE_CNTL_PRIV(priv,cntl) struct v3_priv * priv=get_priv(cntl)
static inline struct v3_priv *get_priv(cntl_t * cntl)
{
	return cntl->priv;
}

static cntl_t v3_driver =
{
    .name = "aml_v3_nand_driver",
	.feature = 0x3f,
	.nand_cycle = 0, //cycle time int 0.1ns
	.ecc = ecc_table,
	/** configure and control function **/
	.config = v3_config,
	/** fifo relative functions **/
	.fifo = v3_fifo,
	.size = v3_size,
	.avail = v3_avail,
	.head = v3_head,
	.tail = v3_tail,

	/** nand command routines*/
	.ctrl = v3_ctrl,
	.wait = v3_wait,
	.nop = v3_nop,
	.sts = v3_sts,
	.readbytes = v3_readbytes,
	.writebytes = v3_writebytes,
	.readecc =	v3_readecc,
	.writeecc = v3_writeecc,

	/** util functions for async mode **/
	.job_get = v3_job_get,
	.job_select = v3_job_select,
	.job_finish =v3_job_finish,
	.job_status = v3_job_status,
	/** ecc dma relative functions **/
	.ecc2dma    =  v3_ecc2dma   ,
    .info2data  =  v3_info2data ,
    .data2info  =  v3_data2info ,
    .priv = &v3_priv
};

#define NFC_REG_CMD           (priv->reg_base+0x00)
#define NFC_REG_CFG           (priv->reg_base+0x04)
#define NFC_REG_DADR          (priv->reg_base+0x08)
#define NFC_REG_IADR          (priv->reg_base+0x0C)
#define NFC_REG_BUF           (priv->reg_base+0x10)
#define NFC_REG_INFO          (priv->reg_base+0x14)
#define NFC_REG_DC            (priv->reg_base+0x18)
#define NFC_REG_ADR           (priv->reg_base+0x1C)
#define NFC_REG_DL            (priv->reg_base+0x20)
#define NFC_REG_DH            (priv->reg_base+0x24)

#define NFC_CMD_WAIT_EMPTY()       wait_fifo_empty(priv)
static inline void wait_fifo_empty(struct v3_priv * priv)
{
    uint32_t st;
    do{
        st=readl(NFC_REG_CFG);
    }while((st&(1<<12))!=0);
    while(NFC_CMDFIFO_SIZE()>0){};
    writel(P_NAND_CMD,NFC_CMD_STANDBY(0));
    writel(P_NAND_CMD,NFC_CMD_STANDBY(0));
    while(NFC_CMDFIFO_SIZE()>0){};
}
static const uint32_t v3_ce[]={CE0,CE1,CE2,CE3,CE_NOT_SEL};
static const uint32_t v3_rbio[]={IO4,IO5,IO6};
#define NFC_CE(ce)      (v3_ce[ce])
#define NFC_RBIO(io)    (v3_rbio[io])
static int32_t v3_config(cntl_t * cntl, uint32_t config, ...)
{
	DEFINE_CNTL_PRIV(priv,cntl);
	va_list ap;
	struct aml_nand_platform * plat;
	int32_t ret;
	uint32_t int_temp;
	uint8_t char_temp;
	uint16_t mode;
	void* p_temp;
	va_start(ap, config);
	switch(config)
	{
		case NAND_CNTL_MODE_SET: // struct aml_nand_platform *
		{
			plat=va_arg(ap,struct aml_nand_platform *);
			if(plat)
			    priv->plat=plat;
			priv->delay=plat->delay?plat->delay:priv->delay;
			priv->reg_base=plat->reg_base?plat->reg_base:priv->reg_base;
			assert(priv->delay<120);
			assert(priv->edo<10);
			assert(priv->reg_base>0);
		}
		break;
		case NAND_CNTL_TIME_SET: //uint16_t mode(0:async,1:sync mode,2 toggle),uint16_t t_rea,uint16_t t_rhoh,uint16_t sync_adjust
		{
			mode=va_arg(ap,uint32_t)&3;

			uint16_t bus_cycle,t_rea=(uint16_t)va_arg(ap,uint32_t);
			uint16_t bus_time,t_rhoh=(uint16_t)va_arg(ap,uint32_t);
			uint16_t sync_adjust=0;
			assert(mode<3);
			if(mode)
			    sync_adjust=(uint16_t)va_arg(ap,uint32_t)&1;
			assert(t_rea>=priv->plat->t_rea);
			assert(t_rhoh>=priv->plat->t_rhoh);
			assert(v3_time_caculate(&t_rea,&t_rhoh,priv->edo,priv->delay,clk_get_rate(priv->plat->clk_src)));
			NFC_CMD_WAIT_EMPTY();
			bus_cycle=t_rhoh;
			bus_time=t_rea;
			clrsetbits_le32(NFC_REG_CFG,0xfff|(1<<16),(bus_cycle&0x1f)|((bus_time&0x1f)<<5)| (mode<<10)|(sync_adjust<<16));
		}
		break;
        case NAND_CNTL_POWER_SET:
		///
		printf("Not Implement\n");
		break;
		case NAND_CNTL_FIFO_MODE: //uint16_t start(bit0 cmd fifo: 1=enable 0=disable , bit1 interrupt : 0=disable,1=enable),
		{
			mode=((uint16_t)va_arg(ap,uint32_t))&3;
			uint16_t xor=mode^priv->fifo_mode;
			if(xor!=0)
			{
			    NFC_CMD_WAIT_EMPTY();
    			priv->fifo_mode=mode;
    			if(xor&2)
    			{
    				clrsetbits_le32(NFC_REG_CFG,3<<20,(mode&1?3:0)<<20);
    			}
		    }
		}
		break;
		default:
		    break;

	}
	va_end(ap);
	return 0;
}

#define V3_FIFO_WRITE(ret,cmd) {if((ret=v3_fifo_write(cntl,cmd))<0)return ret;}
static int32_t v3_fifo_write(cntl_t *cntl, uint32_t cmd)
{
    DEFINE_CNTL_PRIV(priv, cntl);
//    
//    if(NFC_CMDFIFO_AVAIL
	///@todo implement it
	return -1;
}

static int32_t v3_fifo_anchor(cntl_t *cntl_t, jobkey_t job)
{
    

	///@todo implement it
	return -1;
}

static int32_t v3_ctrl(cntl_t *cntl, uint16_t ce, uint16_t ctrl)
{
    int32_t ret;
//	DEFINE_CNTL_PRIV(priv, cntl);
	if (ctrl & 0x100)
		V3_FIFO_WRITE(ret,NFC_CMD_CLE(NFC_CE(ce),ctrl));
	V3_FIFO_WRITE(ret, NFC_CMD_ALE(NFC_CE(ce),ctrl));
	return 0;
}
static int32_t v3_wait(cntl_t * cntl, uint8_t mode, uint16_t ce,uint8_t cycle_log2)
{
	uint32_t cmd;
	int32_t ret;
	if (mode & 1)
        cmd=NFC_CMD_RB(NFC_CE(ce),cycle_log2);
    else
        cmd=NFC_CMD_RBIO(NFC_RBIO(ce), cycle_log2);
	V3_FIFO_WRITE(ret,cmd);
	return 0;
}
static int32_t v3_nop(cntl_t * cntl, uint16_t ce, uint16_t cycles)
{
    int32_t ret;
	V3_FIFO_WRITE(ret, NFC_CMD_IDLE(NFC_CE(ce),cycles));
	return ret;
}

static uint32_t v3_alloc_sts(cntl_t * cntl,uint32_t  key)
{
    int i;
    DEFINE_CNTL_PRIV(priv, cntl);
    if(key==0)
    {
        return (uint32_t)&(priv->sts_buf[priv->sts_size-1]);
    }
    v3_lock_sts_buf(priv);
    for(i=0;i<priv->sts_size-2;i++)
    {
        if(priv->sts_buf[i].key==0)
        {
            priv->sts_buf[i].key=key;
            v3_unlock_sts_buf(priv);
            return (uint32_t)&(priv->sts_buf[i]);
        }
    }
    v3_unlock_sts_buf(priv);
    return -1;
}
static int32_t v3_free_sts(cntl_t * cntl,uint32_t sts)
{
    DEFINE_CNTL_PRIV(priv, cntl);
    uint32_t id;
    id=(sts-(uint32_t)priv->sts_buf);
    if(id%=sizeof(priv->sts_buf[0]))
        return -1;
    id/=sizeof(priv->sts_buf[0]);
    if(id==priv->sts_size - 1)
        return 0;
    v3_lock_sts_buf(priv);
    priv->sts_buf[id].done=0;
    priv->sts_buf[id].key=0;
    v3_unlock_sts_buf(priv);
    return 0;
}

static int32_t v3_sts(cntl_t * cntl, jobkey_t *job, uint16_t mode)
{
    int32_t ret;
    uint32_t sts_addr=(uint32_t)job;
    if(sts_addr==0)
        return -1;
    V3_FIFO_WRITE(ret,NFC_CMD_ASL(sts_addr));
    V3_FIFO_WRITE(ret,NFC_CMD_ASH(sts_addr));
    V3_FIFO_WRITE(ret,NFC_CMD_STS(mode));
    return ret;
}
static int32_t v3_ecc2dma(ecc_t * orig,dma_desc_t* dma_desc,uint32_t size,uint32_t short_size,uint32_t seed_en)
{
    uint32_t remainder,max,pages,short_dma;
    if(orig==NULL)
        orig=&ecc_table[0];//BCH off
    if(orig->bits==0 )//BCH off
    {
        //ignore short_size;
        if(size>orig->max)
            return orig->max - size;
        dma_desc->dma=(seed_en?(1<<19):0)|size;
        dma_desc->pages=1;
        dma_desc->page_size=size;
        dma_desc->info=0;
        dma_desc->io_size=(size);
        dma_desc->parity=0;
        return 0;
    }
    if((short_size&7)||(size&7))
        return -1;
    if(short_size>(63<<3))
        return -2;
    if(short_size==0&&size==0)
        return -3;
    if(short_size!=0&&size==0)
        return -4;
    
    max=short_size?0x3f*short_size:orig->max;
    if(size>max)
        return max - size ;
    if(short_size)
    {
        short_dma=(1<<13)|(short_size<<2);
    }else{
        short_size=orig->data<<3;
        short_dma=0;
    }
    remainder=size%short_size;
    pages=size/short_size;
    if(remainder)
        return -5;
    dma_desc->dma=(seed_en?(1<<19):0)|short_dma|((orig->mode&7)<<14)|pages;
    dma_desc->pages=pages;
    dma_desc->page_size=short_size;
    dma_desc->info=8;
    dma_desc->io_size=(short_size+orig->info+orig->parity);
    dma_desc->parity=orig->parity;
    return 0;
}
typedef struct __info_s{
    uint8_t info[2];
    uint8_t zero;
    uint8_t err:6;
    uint8_t ecc:1;
    uint8_t done:1;
    uint32_t data_addr;
}info_t;
static int32_t v3_info2data(void * data,void * inf,dma_t dma)//-1,found ecc fail,>=0,ecc counter .
{
    uint32_t pages=dma&0x3f;
    uint32_t bits;
    info_t *info;
    uint8_t *dat;
    int ret,i;
    if((dma&(7<<14))==0)
        return -1;//BCH off , no need
    bits=ecc_table[(dma&(7<<14))].bits;
    for(ret=0,i=0,dat=data,info=inf;i<pages;i++,dat+=2)
    {
        if(info[i].ecc)
        {
            if(info[i].err==0x3f)
                return -2;//uncorrectable 
            ret=max(ret,info[i].err);
            dat[0]=info[i].info[0];
            dat[1]=info[i].info[1];
        }else{
            BUG();
        }
        
    }
    return ret;
}
static int32_t v3_data2info(void * inf,void * data,dma_t dma)
{
    uint32_t pages=dma&0x3f;
    uint32_t bits;
    info_t *info;
    uint8_t *dat;
    int ret,i;
    if((dma&(7<<14))==0)
        return -1;//BCH off , no need
    for(ret=0,i=0,dat=data,info=inf;i<pages;i++,dat+=2)
    {
        info[i].info[0]=dat[0];
        info[i].info[1]=dat[1];
        info[i].data_addr=0;
        info[i].done=0;
    }
    return ret;

}

static inline int32_t dma_set_addr(cntl_t  * cntl,uint32_t data,uint32_t info)
{
    int32_t ret;
    V3_FIFO_WRITE(ret,NFC_CMD_ADL(data));
    V3_FIFO_WRITE(ret,NFC_CMD_ADH(data));
    V3_FIFO_WRITE(ret,NFC_CMD_AIL(info));
    V3_FIFO_WRITE(ret,NFC_CMD_AIH(info));
    return 0;
}
static int32_t v3_readbytes(cntl_t  * cntl,void * addr, dma_t dma_mode)
{
    dma_t dma;
    int32_t ret;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))==0);
    assert((dma_mode&((1<<14)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<14)-1));
    if((ret=dma_set_addr(cntl,(uint32_t)addr,priv->temp))<0)return ret;
    V3_FIFO_WRITE(ret,NFC_CMD_READ(dma));
    return ret;
}
static int32_t v3_writebytes(cntl_t * cntl,void * addr, dma_t dma_mode)
{
    dma_t dma;
    int32_t ret;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))==0);
    assert((dma_mode&((1<<14)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<14)-1));
    if((ret=dma_set_addr(cntl,(uint32_t)addr,priv->temp))<0)
        return ret;
    V3_FIFO_WRITE(ret,NFC_CMD_WRITE(dma));
    return ret;
}
static int32_t v3_readecc(cntl_t * cntl, void * addr, void * info,dma_t dma_mode)
{
    dma_t dma;
    int32_t ret;
//    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))!=0);
    assert((dma_mode&((1<<17)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<17)-1));
    if((ret=dma_set_addr(cntl,(uint32_t)addr,(uint32_t)info))<0)
        return ret;
    V3_FIFO_WRITE(ret,NFC_CMD_READ(dma));
    return ret;
}
static int32_t v3_writeecc(cntl_t * cntl, void * addr, void * info,dma_t dma_mode)
{
    dma_t dma;
    int32_t ret;
//    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))!=0);
    assert((dma_mode&((1<<17)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<17)-1));
    if((ret=dma_set_addr(cntl,(uint32_t)addr,(uint32_t)info))<0)
        return ret;
    V3_FIFO_WRITE(ret,NFC_CMD_READ(dma));
    return ret;
}


void board_nand_init()
{
    v3_driver.name="hh";
}