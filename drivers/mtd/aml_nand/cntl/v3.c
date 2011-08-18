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
	uint32_t    nfc_ce;

	uint32_t 	int_seq;
	uint32_t	tail;
	uint32_t    fifo_mask;
	uint32_t    fifo_size;
	uint32_t    fifo_tail;
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


#define NFC_CMD_WAIT_EMPTY()       wait_fifo_empty(priv)
static inline void wait_fifo_empty(struct v3_priv * priv)
{
    uint32_t st;
    do{
        st=readl(P_NAND_CFG);
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
			clrsetbits_le32(P_NAND_CFG,0xfff|(1<<16),(bus_cycle&0x1f)|((bus_time&0x1f)<<5)| (mode<<10)|(sync_adjust<<16));
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
    				clrsetbits_le32(P_NAND_CFG,3<<20,(mode&1?3:0)<<20);
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

static inline void v3_write_hardware_fifo(struct v3_priv * priv,uint32_t cmd)
{
	if(cmd==NAND_CMD_STS(2))
		priv->int_seq=priv->tail;
	priv->tail++;
	writel(cmd,P_NAND_CMD);
}

static inline uint32_t v3_cmd_fifo_head(struct v3_priv * priv)
{
    uint32_t head=readl(P_NAND_CADR);
    uint32_t mask=(priv->fifo_mask<<3)|7;
	uint32_t fifo=(uint32_t)priv->cmd_fifo;
	if(head<fifo||head>fifo+mask)
	    return 0;
	head-=fifo;
	head>>=2;
	return head;
}
static inline uint32_t  v3_cmd_fifo_tail(struct v3_priv * priv)
{
	uint32_t fifo_mask=(priv->fifo_mask<<1)|1;
    return (priv->fifo_tail )&(fifo_mask);
}
static inline uint32_t v3_cmd_fifo_size(struct v3_priv * priv)
{
	uint32_t tail=v3_cmd_fifo_tail(priv);
	return (priv->fifo_tail - v3_cmd_fifo_head(priv))&(priv->fifo_mask);
}
static inline uint32_t v3_cmd_fifo_avail(struct v3_priv * priv)
{
	return priv->fifo_mask-v3_cmd_fifo_size(priv);
}

static inline int32_t v3_cmd_fifo_compare(struct v3_priv * priv)
{
	uint32_t head=readl(P_NAND_CADR);
	uint32_t tail=(uint32_t)&(priv->cmd_fifo[priv->fifo_tail]);
	if(head>tail)
	{
		return 1;
	}
	return 0;
}
#define nfc_mb(a,b)
static inline void v3_cmd_fifo_reset(struct v3_priv * priv)
{
    uint32_t head=readl(P_NAND_CADR);
	uint32_t mask=(priv->fifo_mask<<2)|3;
	uint32_t fifo=(uint32_t)priv->cmd_fifo;
	if(!(head<fifo||head>fifo+mask))
		return;
	uint32_t * phead=(uint32_t *)head;
	if(head>fifo+mask&&*phead==0)
		writel((uint32_t)(priv->cmd_fifo),P_NAND_CADR);
}
#define cmd(a)  cmd_##a
#define ret(a)  ret_##a
#define EXTEND(name,a)   name(a)
#define TMP_VAR(name) EXTEND(name,__LINE__)
#define V3_FIFO_WRITE(x...) {   uint32_t TMP_VAR(cmd)[]={x};                            \
                                int32_t TMP_VAR(ret);                                   \
                                if(( TMP_VAR(ret)=v3_fifo_write(priv,TMP_VAR(cmd),             \
                                    sizeof(TMP_VAR(cmd))/sizeof(TMP_VAR(cmd)[0])))<0)   \
                                    return TMP_VAR(ret);}
static inline int32_t v3_check_fifo_interrupt(uint32_t * cmd_q,uint32_t size,uint32_t modify)
{
	uint32_t cmd;
	int i;
	for(i=0;i<size;i++)
	{
		cmd=cmd_q[i];
		if(cmd==0)
			return 0;
		switch(cmd&(0xf<<18))
		{
		case (4<<18):
		case (5<<18):
		case (6<<18):
		case (7<<18):
			if(cmd&(0xf<<14))
				return 1;
			if(modify)
			{
				cmd_q[i]=cmd|((cmd&(0xf<<10))<<4);
				return 1;
			}
			break;
		case (9<<18):
			if(cmd==NFC_CMD_STS(2))
				return 1;
			if(modify)
			{
				cmd_q[i]=NFC_CMD_STS(2);
				return 1;
			}

			break;
		default:
			break;
		}
	}
	return 0;
}
#define nfc_dmb()
static int32_t v3_fifo_write(struct v3_priv *priv, uint32_t cmd_q[],uint32_t size)
{
    uint32_t mb_tag;
    uint32_t head,tail,sizefifo,avail,hw_avail;
    uint32_t begin,end,mask,int_tag;
    uint32_t * cmd_fifo;
    uint32_t tmp;
    int i,j;
    hw_avail=NFC_CMDFIFO_AVAIL();
    if((priv->fifo_mode&1)==0&&hw_avail<size)//mode does not match
        return -2;
    sizefifo=v3_cmd_fifo_size(priv);
    avail=v3_cmd_fifo_avail(priv);
    if(sizefifo==0&&(avail+hw_avail)<size)//no enough space
   		return -1;
    head=v3_cmd_fifo_head(priv);
    tail=v3_cmd_fifo_tail(priv);
    if(tail<head&&avail<size)//no enough space
    	return -1;
    begin=tail;
    end=tail+size;
    cmd_fifo=priv->cmd_fifo;
    if((priv->fifo_mode&2)==0//no interrupt mode
    	||(end<mask-31)	)
	{
    	goto write_fifo_raw;
	}


    ///@todo I am writting this section
    ///end > mask-31 , we must do interrupt check;
    int_tag=0;
    if(begin>mask-31)
    {
    	int_tag=v3_check_fifo_interrupt(&cmd_fifo[mask-31],begin-mask+31,0);
    }
    if(int_tag==0)
    {
    	uint32_t c_s,c_e;
    	c_s=begin>mask-31?0:mask-31-begin;
    	c_e=end>mask?mask+1:size;
    	int_tag=v3_check_fifo_interrupt(&cmd_fifo[c_s],c_e-c_s,1);
    }
    if(int_tag)
    {
    	goto write_fifo_raw;
    }
    if(avail<size+5&&begin<mask-4)//no enough space
    	return -1;
    cmd_fifo[begin+1]=NFC_CMD_ASL(priv->temp);
    cmd_fifo[begin+2]=NFC_CMD_ASH(priv->temp),
    cmd_fifo[begin+3]=NFC_CMD_STS(2);
    cmd_fifo[begin+4]=NFC_CMD_IDLE(priv->nfc_ce,0);
    cmd_fifo[begin+5]=0;
    cmd_fifo[begin]=NFC_CMD_IDLE(CE_NOT_SEL,0);
    nfc_dmb();
    if(begin>=mask-4)
    	priv->fifo_tail+=begin+5-mask;
    else
    	priv->fifo_tail+=5;
	priv->tail+=5;
	tail=v3_cmd_fifo_tail(priv);
	begin=tail;
	end=tail+size;
write_fifo_raw:
	if(end<mask+1)
	{
		memcpy(&cmd_fifo[begin+1],&cmd_q[1],(size)*sizeof(uint32_t));
	}else{
		cmd_fifo[mask+1]=0;
		memcpy(&cmd_fifo[begin+1],&cmd_q[1],(mask-tail)*sizeof(uint32_t));
		memcpy(&cmd_fifo[0],cmd_q[(mask+1-tail)],(size+1-(mask+1-tail))*sizeof(uint32_t));
	}
	cmd_fifo[begin]=cmd_q[0];
	nfc_dmb();
	priv->tail+=size;
	priv->fifo_tail+=size;
	if(priv->fifo_mode&2)
		setbits_le32(P_NAND_CFG,1<<12);//start fifo immediatly
	return 0;
}

static int32_t v3_fifo_anchor(cntl_t *cntl_t, jobkey_t job)
{
	///@todo implement it
	return -1;
}
static inline int32_t v3_check_and_insert_interrupt(struct v3_priv * priv,uint32_t nfc_ce)
{
    uint32_t tail,leave,avail;
    int32_t ret;
    if(priv->nfc_ce==nfc_ce)
        return 0;
    
    if((priv->fifo_mode&2)==0 ||//interrupt disable
        NFC_CMDFIFO_AVAIL())    //write to hardware directorly 
    {
        priv->nfc_ce=nfc_ce;
        return 0;
    }
    
    
    tail=v3_cmd_fifo_tail(priv);
    if(tail<priv->fifo_mask-31)//no need insert interrupt 
    {
        priv->nfc_ce=nfc_ce;
        return 0;
    }
    avail=v3_cmd_fifo_avail(priv);
    leave=priv->fifo_mask+1-tail;
    if(avail>3&&leave>3)
    {
        V3_FIFO_WRITE(
            NFC_CMD_IDLE(CE_NOT_SEL,0),
            NFC_CMD_ASL(priv->temp),
            NFC_CMD_ASH(priv->temp),
            NFC_CMD_STS(2));
        priv->nfc_ce=nfc_ce;
        return 0;
    }
    if(leave<4&&avail>4-leave)
    {
    	priv->cmd_fifo[tail]  =NFC_CMD_IDLE(CE_NOT_SEL,0);
    	priv->cmd_fifo[tail+1]=NFC_CMD_ASL(priv->temp),
        priv->cmd_fifo[tail+2]=NFC_CMD_ASH(priv->temp),
        priv->cmd_fifo[tail+3]=NFC_CMD_STS(2);
    	priv->cmd_fifo[tail+4]=0;
    	nfc_mb(&priv->cmd_fifo[fifo_tail],5*sizeof(priv->cmd_fifo[0]));
    	priv->fifo_tail+=leave;
    	priv->cmd_fifo[0]=0;
    	nfc_mb(&priv->cmd_fifo[0],sizeof(priv->cmd_fifo[0]));
    	priv->nfc_ce=nfc_ce;
    	return 0;
    }
    return -1;
} 
static int32_t v3_ctrl(cntl_t *cntl, uint16_t ce, uint16_t ctrl)
{
    DEFINE_CNTL_PRIV(priv,cntl);
    v3_check_and_insert_interrupt(priv,NFC_CE(ce));
	if (ctrl & 0x100)
		V3_FIFO_WRITE(NFC_CMD_CLE(NFC_CE(ce),ctrl));
	V3_FIFO_WRITE(NFC_CMD_ALE(NFC_CE(ce),ctrl));
	return 0;
}
static int32_t v3_wait(cntl_t * cntl, uint8_t mode, uint16_t ce,uint8_t cycle_log2)
{
	uint32_t cmd;
    DEFINE_CNTL_PRIV(priv,cntl);
	if (mode & 1)
	{
		v3_check_and_insert_interrupt(priv,NFC_CE(ce));
        cmd=NFC_CMD_RB(NFC_CE(ce),cycle_log2);
	}
    else
        cmd=NFC_CMD_RBIO(NFC_RBIO(ce), cycle_log2);
	V3_FIFO_WRITE(cmd);
	return 0;
}
static int32_t v3_nop(cntl_t * cntl, uint16_t ce, uint16_t cycles)
{

    DEFINE_CNTL_PRIV(priv,cntl);
    v3_check_and_insert_interrupt(priv,NFC_CE(ce));
	V3_FIFO_WRITE( NFC_CMD_IDLE(NFC_CE(ce),cycles));
	return 0;
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

    DEFINE_CNTL_PRIV(priv, cntl);
    uint32_t sts_addr=(uint32_t)job;
    if(sts_addr==0)
        return -1;
    V3_FIFO_WRITE(
        NFC_CMD_ASL(sts_addr),
        NFC_CMD_ASH(sts_addr),
        NFC_CMD_STS(mode));
    return 0;
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
/**
 * @param inf
 * @param data
 * @param dma
 * @return
 */
static int32_t v3_data2info(void * inf,void * data,dma_t dma)
{
    uint32_t pages=dma&0x3f;
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
#define dma_set_addr(data,info) NFC_CMD_ADL(data),NFC_CMD_ADH(data),NFC_CMD_AIL(info),NFC_CMD_AIH(info)
static int32_t v3_readbytes(cntl_t  * cntl,void * addr, dma_t dma_mode)
{
    dma_t dma;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))==0);
    assert((dma_mode&((1<<14)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<14)-1));

    
    V3_FIFO_WRITE(dma_set_addr((uint32_t)addr,priv->temp),NFC_CMD_READ(dma));
    return 0;
}
static int32_t v3_writebytes(cntl_t * cntl,void * addr, dma_t dma_mode)
{
    dma_t dma;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))==0);
    assert((dma_mode&((1<<14)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<14)-1));
    V3_FIFO_WRITE(dma_set_addr((uint32_t)addr,priv->temp),NFC_CMD_WRITE(dma));
    return 0;
}
static int32_t v3_readecc(cntl_t * cntl, void * addr, void * info,dma_t dma_mode)
{
    dma_t dma;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))!=0);
    assert((dma_mode&((1<<17)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<17)-1));
    V3_FIFO_WRITE(dma_set_addr((uint32_t)addr,(uint32_t)info),NFC_CMD_READ(dma));
    return 0;
}
static int32_t v3_writeecc(cntl_t * cntl, void * addr, void * info,dma_t dma_mode)
{
    dma_t dma;
    DEFINE_CNTL_PRIV(priv, cntl);
    assert(dma_mode>0);
    assert((dma_mode&(7<<14))!=0);
    assert((dma_mode&((1<<17)-1))!=0);
    dma=dma_mode&((1<<19)|((1<<17)-1));
    V3_FIFO_WRITE(dma_set_addr((uint32_t)addr,(uint32_t)info),NFC_CMD_READ(dma));
    return 0;
}
/**
 *
 * @param
 * @param config
 * @return
 */

static int32_t v3_fifo(cntl_t *cntl, uint32_t config, ...)
{
	return -1;
}
static uint32_t v3_size(cntl_t * cntl)
{
    dma_t dma;
    int32_t ret;
    DEFINE_CNTL_PRIV(priv, cntl);

}
static uint32_t v3_avail(cntl_t *);
static uint32_t v3_head(cntl_t *);
static uint32_t v3_tail(cntl_t *);


void board_nand_init()
{
    v3_driver.name="hh";
}
