#include <stdarg.h>
#include <common.h>
#define assert(x) ((void)0)
#include <amlogic/nand/cntl.h>
#include <amlogic/nand/platform.h>
#include <asm/io.h>
#include <asm/arch/nand.h>
#include <asm/dma-mapping.h>

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
static int32_t v3_config(cntl_t * cntl, uint32_t config, va_list args); //done the basic
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
static int32_t v3_job_free(cntl_t * cntl_t, jobkey_t * job);
static uint32_t v3_job_key(cntl_t * cntl_t, jobkey_t * job);
static uint32_t v3_job_status(cntl_t * cntl, jobkey_t * job);
static int32_t v3_ecc2dma(ecc_t * orig,dma_desc_t* dma,uint32_t size,uint32_t short_size,uint32_t seed);
static int32_t v3_info2data(void * data,void * info,dma_t dma);//-1,found ecc fail,>=0,ecc counter .
static int32_t v3_data2info(void * info,void * data,dma_t dma);//-1,error found
static int32_t v3_convert_cmd(cmdq_t * in,cmdq_t* out);
static int32_t v3_write_cmd(cntl_t * ,cmdq_t * cmd);
static int32_t v3_seed(cntl_t *, uint16_t seed);//0 disable
/**
 *
 * @param cntl_t controller
 * @param jobs	in/out parameter ,the finish status job list
 * @param size	input jobs size
 * @return <0 , error ; >=0 , return size of jobs
 */
static int32_t v3_job_lookup(cntl_t * cntl_t, jobkey_t ** jobs,uint32_t size);

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
	.size = v3_size,
	.avail = v3_avail,
	.head = v3_head,
	.tail = v3_tail,

	/** nand command routines*/
	.convert_cmd=v3_convert_cmd,
	.write_cmd=v3_write_cmd,
	.ctrl = v3_ctrl,
	.wait = v3_wait,
	.nop = v3_nop,
	.sts = v3_sts,
	.readbytes = v3_readbytes,
	.writebytes = v3_writebytes,
	.readecc =	v3_readecc,
	.writeecc = v3_writeecc,
	.seed=v3_seed,

	/** util functions for async mode **/
	.job_get = v3_job_get,
	.job_free =v3_job_free,
	.job_lookup = v3_job_lookup,
	.job_key=v3_job_key,
	.job_status=v3_job_status,
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
static inline uint32_t v3_cmd_fifo_size(struct v3_priv * priv);
#define CNTL_BUF_SIZE	(4*1026)
#define CMD_FIFO_PLUS	64
static int32_t v3_config(cntl_t * cntl, uint32_t config, va_list args)
{
	DEFINE_CNTL_PRIV(priv,cntl);
//	va_list args;
	struct aml_nand_platform * plat;
	int32_t ret;
	uint32_t int_temp;
	uint8_t char_temp;
	uint16_t mode;
	void* p_temp;
//	va_start(args, config);
	switch(config)
	{
		case NAND_CNTL_INIT: // struct aml_nand_platform *
		{
			plat=va_arg(args,struct aml_nand_platform *);
			if(plat)
			    priv->plat=plat;
			priv->delay=plat->delay?plat->delay:priv->delay;
			priv->reg_base=plat->reg_base?plat->reg_base:priv->reg_base;
			assert(priv->delay<120);
			assert(priv->edo<10);
			assert(priv->reg_base>0);
			priv->cmd_fifo=(uint32_t*)dma_alloc_coherent(CNTL_BUF_SIZE*sizeof(cmd_t),&int_temp);
			priv->fifo_mask=(CNTL_BUF_SIZE>>2)-1;
			priv->sts_buf=(sts_t*)&(priv->cmd_fifo[(CNTL_BUF_SIZE>>2)+CMD_FIFO_PLUS]);
			priv->temp=(uint32_t)&(priv->cmd_fifo[(CNTL_BUF_SIZE)-2]);
			priv->sts_size=(priv->temp - (uint32_t)priv->sts_buf)/sizeof(sts_t);
		}
		break;
		case NAND_CNTL_TIME_SET: //uint16_t mode(0:async,1:sync mode,2 toggle),uint16_t t_rea,uint16_t t_rhoh,uint16_t sync_adjust
		{
			mode=va_arg(args,uint32_t)&3;

			uint16_t bus_cycle,t_rea=(uint16_t)va_arg(args,uint32_t);
			uint16_t bus_time,t_rhoh=(uint16_t)va_arg(args,uint32_t);
			uint16_t sync_adjust=0;
			assert(mode<3);
			if(mode)
			    sync_adjust=(uint16_t)va_arg(args,uint32_t)&1;
			/*
			assert(t_rea>=priv->plat->t_rea);
			assert(t_rhoh>=priv->plat->t_rhoh);
			*/
			t_rea=max(t_rea,priv->plat->t_rea);
			t_rhoh=max(t_rhoh,priv->plat->t_rhoh);
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
			mode=((uint16_t)va_arg(args,uint32_t))&3;
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
		case NAND_CNTL_FIFO_RESET://uint16_t mode : 1, force reset ; 0, reset possible
			mode=((uint16_t)va_arg(args,uint32_t))&3;
			if(mode)
				NFC_CMD_WAIT_EMPTY();
			if(v3_cmd_fifo_size(priv))
				break;
			writel((uint32_t)(priv->cmd_fifo),P_NAND_CADR);
			break;
		default:
		    break;

	}
//	va_end(args);
	return 0;
}
#define nfc_dmb()
#define STS_2_CMD_SIZE 	5

#define ANCHOR_POS_E		(priv->fifo_mask+1-(NFC_CMDFIFO_MAX-1))
#define ANCHOR_POS_S 		(ANCHOR_POS_E - STS_2_CMD_SIZE )


static inline uint32_t v3_cmd_fifo_head(struct v3_priv * priv)
{
    uint32_t head=readl(P_NAND_CADR);
    uint32_t fifo_size=(priv->fifo_mask+1);
	uint32_t fifo=(uint32_t)priv->cmd_fifo;
	head-=fifo;
	head>>=2;
	if(head>fifo_size+CMD_FIFO_PLUS)
	    return 0;
	return head;
}
static inline uint32_t  v3_cmd_fifo_tail(struct v3_priv * priv)
{
	return (priv->fifo_tail )&(priv->fifo_mask);
}
static inline uint32_t v3_cmd_fifo_size(struct v3_priv * priv)
{
	uint32_t tail=v3_cmd_fifo_tail(priv);
	uint32_t head=v3_cmd_fifo_head(priv);
	uint32_t tmp,tmp1;
	uint32_t i;
	uint32_t *cmd_fifo=priv->cmd_fifo;
	uint32_t fifo_size=(priv->fifo_mask+1);

	if(head>=fifo_size&&cmd_fifo[head]==0)
		head=0;

	if(head>tail)
	{
		tmp=head>=fifo_size?head:fifo_size;
		for(i=0;cmd_fifo[tmp+i]!=0;i++);
		tmp1=head>=fifo_size?0:head;
		return ((tail-tmp1)&priv->fifo_mask)+i;
	}
	return tail-head;
}
static inline uint32_t v3_cmd_fifo_avail_plus(struct v3_priv * priv,uint32_t plus)
{
	uint32_t tail=v3_cmd_fifo_tail(priv);
	uint32_t head=v3_cmd_fifo_head(priv);
	uint32_t i;
	uint32_t fifo_size=(priv->fifo_mask+1);
	if(head>tail)
	{
		i=(head-tail-1);
		return i;
	}
	if(head==tail)
	{
		return fifo_size+plus;
	}

	return fifo_size+plus-tail+head-1;
}
static inline uint32_t v3_cmd_fifo_avail(struct v3_priv * priv)
{
	return v3_cmd_fifo_avail_plus(priv,0);
}

#define nfc_mb(a,b)
#if 0
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
#endif
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
	//todo reconsider it
	uint32_t cmd;
	int i;
	for(i=0;i<size;i++)
	{
		cmd=cmd_q[i];
		if(cmd==0)
			return -1;
		switch(cmd&(0xf<<18))
		{
		case (4<<18):
		case (5<<18):
		case (6<<18):
		case (7<<18):
			if(cmd&(0xf<<14))
				return i;
			if(modify)
			{
				cmd_q[i]=cmd|((cmd&(0xf<<10))<<4);
				return i;
			}
			break;
		case (9<<18):
			if(cmd==NFC_CMD_STS(2))
				return i;
			if(modify)
			{
				cmd_q[i]=NFC_CMD_STS(2);
				return i;
			}

			break;
		default:
			break;
		}
	}
	return -1;
}
static uint32_t v3_find_ce(uint32_t * cmd , uint32_t tag,uint32_t old)
{
	uint32_t ce=old;
	while(*cmd)
	{
		if(((*cmd)&(0xf<<18))==0)
		{

			ce=(*cmd)&(0xf<<10);
			if(tag==0)
				return ce;
		}

		cmd++;
	}
	return ce;
}
static int32_t v3_fifo_write(struct v3_priv *priv, uint32_t cmd_q[],uint32_t size)
{

    uint32_t head,tail=0,sizefifo,avail,hw_avail;
    uint32_t begin,fifo_size=0;
    uint32_t anchor_s,anchor_e;
    uint32_t *cmd_fifo=priv->cmd_fifo;
    uint32_t cmd_size;
    uint32_t * cmd;
    uint32_t * write_cmd[4]={NULL,NULL,NULL,NULL};
    uint32_t write_size[4]={0,0,0,0};
    int32_t int_tag;
    uint32_t int_pos;
    int i;
    hw_avail=NFC_CMDFIFO_AVAIL();
    if((priv->fifo_mode&1)==0)
    {
    	if(hw_avail<size)//mode does not match
    		return -2;//mode does not match
    	write_cmd[0]=cmd_q;
    	write_size[0]=size;
    	goto write_fifo_raw;
    }
    sizefifo=v3_cmd_fifo_size(priv);//if size == 0 , this function should reset fifo
    avail=v3_cmd_fifo_avail_plus(priv,CMD_FIFO_PLUS);
    if(sizefifo==0)
    {
    	if((avail+hw_avail)<size)//no enough space
    		return -1;//no enough space now
    	if(hw_avail>=size)
    	{
    		write_cmd[0]=cmd_q;
			write_size[0]=size;
			goto write_fifo_raw;
    	}else{
    		write_cmd[0]=cmd_q;
			write_size[0]=hw_avail;
			cmd=&cmd_q[hw_avail];
			cmd_size=size-hw_avail;
    	}
    }
    else if(avail<size)
    {
    	return -1;//no enough space now
    }else{
    	cmd=cmd_q;
    	cmd_size=size;
    }
    fifo_size=priv->fifo_mask +1;
    head=v3_cmd_fifo_head(priv);
    tail=v3_cmd_fifo_tail(priv);
    anchor_s=ANCHOR_POS_E;
    anchor_e=fifo_size+CMD_FIFO_PLUS;

    if(tail>anchor_s)
    {
    	int_tag=v3_check_fifo_interrupt(&cmd_fifo[anchor_s],tail-anchor_s,0);
    	begin=0;
    }else{
    	if(tail+cmd_size<anchor_s)
    		int_tag=0;
    	else{
    		begin=anchor_s-tail;
    		int_tag=-1;
    	}
    }
    if(int_tag<0)
    {
    	int_tag=v3_check_fifo_interrupt(&cmd[begin],cmd_size-begin,1);
    }
    if(int_tag>=0)
    {
    	write_cmd[1]=cmd;
    	write_size[1]=cmd_size;
    	goto write_fifo_raw;
    }
    if(avail<STS_2_CMD_SIZE||avail<cmd_size+STS_2_CMD_SIZE)
    	return -1;//no enough space .
#define INSERT_INTERRUPT(ce)    {NFC_CMD_IDLE(CE_NOT_SEL,0),\
		NFC_CMD_ASL(priv->temp),	\
    	NFC_CMD_ASH(priv->temp),	\
    	NFC_CMD_STS(2),				\
    	NFC_CMD_IDLE(ce,0)			\
		}
    uint32_t old_ce=v3_find_ce(cmd,0,priv->nfc_ce);
    uint32_t new_ce=v3_find_ce(cmd,1,priv->nfc_ce);

    if(cmd_size>NFC_CMDFIFO_MAX-2 || write_size[0])
    {
    	write_cmd[1]=cmd;
    	write_size[1]=cmd_size;

    	uint32_t test[]=INSERT_INTERRUPT(new_ce);
    	write_cmd[2]=&test[0];
    	write_size[2]=5;

    	int_pos=2;
    }else{

    	uint32_t test[]=INSERT_INTERRUPT(old_ce);
    	write_cmd[1]=&test[0];
    	write_size[1]=5;
    	write_cmd[2]=cmd;
    	write_size[2]=cmd_size;
    }



write_fifo_raw:
	if(write_cmd[0]&&write_size[0])
	{
		for(i=0;i<write_size[0];i++,priv->tail++)
		{
			writel(write_cmd[0][i],P_NAND_CMD);
		}
	}
	uint32_t keep=0,keep_pos=tail;
	for(i=1;i<sizeof(write_size)/sizeof(write_size[0]);i++)
	{
		if(!(write_cmd[i]&&write_size[i]))
			continue;
		if(keep==0)
		{
			keep=write_cmd[i][0];
			write_cmd[i][0]=0;
		}
		memcpy(&cmd_fifo[tail],write_cmd[i],write_size[i]*sizeof(write_cmd[i][0]));
		priv->tail+=write_size[i];
		tail+=write_size[i];
	}
	if(keep)cmd_fifo[keep_pos]=keep;
	nfc_dmb();
	if(tail>fifo_size)
		priv->tail=0;
	else
		priv->tail=tail;
	sizefifo=v3_cmd_fifo_size(priv);//if size == 0 , this function should reset fifo
	priv->nfc_ce=new_ce;
	if((priv->fifo_mode&2)&&sizefifo)
		setbits_le32(P_NAND_CFG,1<<12);//start fifo immediatly
	return 0;
}
/***
 *
 * @param cntl
 * @param config
 * @return
 */
static int32_t v3_fifo(cntl_t *cntl, uint32_t config, ...)
{
	return -1;
}
static uint32_t v3_size(cntl_t * cntl)
{
	DEFINE_CNTL_PRIV(priv, cntl);
	uint32_t size,hw_size;
	hw_size=NFC_CMDFIFO_SIZE();
	if((priv->fifo_mode&1)==0)//NO Fifo Buffer return
	{
		return hw_size;
	}
	size=v3_cmd_fifo_size(priv);//if size == 0 , this function should reset fifo
	return size+hw_size;
}
static uint32_t v3_avail(cntl_t * cntl)
{
	DEFINE_CNTL_PRIV(priv, cntl);
	uint32_t tail,size,avail,hw_avail;
	hw_avail=NFC_CMDFIFO_AVAIL();
	if((priv->fifo_mode&1)==0)//NO Fifo Buffer return
	{
		return hw_avail;
	}
	avail=v3_cmd_fifo_avail(priv);//if size == 0 , this function should reset fifo
	size=v3_cmd_fifo_size(priv);//if size == 0 , this function should reset fifo
	if(size==0)
		return hw_avail+priv->fifo_mask+1;
	tail=v3_cmd_fifo_tail(priv);
	if(tail>ANCHOR_POS_S)
		return avail>STS_2_CMD_SIZE?avail-STS_2_CMD_SIZE:0;
	return avail;
}
static uint32_t v3_tail(cntl_t * cntl)
{
	DEFINE_CNTL_PRIV(priv, cntl);
	return priv->tail;
}
static uint32_t v3_head(cntl_t *cntl)
{
	return cntl->tail(cntl)-cntl->size(cntl);
}

static inline int32_t v3_check_and_insert_interrupt(struct v3_priv * priv,uint32_t nfc_ce)
{
    uint32_t tail,leave,avail;
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
static int32_t v3_seed(cntl_t * cntl, uint16_t seed)
{
		DEFINE_CNTL_PRIV(priv,cntl);
		V3_FIFO_WRITE( NFC_CMD_SEED(seed));
		return 0;
}
static int32_t v3_convert_cmd(cmdq_t * inq,cmdq_t* outq)
{
	uint32_t cur=0;
	cmd_t * in;
	cmd_t * out;
	in=inq->cmd;
	out=outq->cmd;
	while(*in!=0&&cur<outq->size)
	{

		switch(*in&(0xf<<28))
		{
		case 1: //cle
			*out = NFC_CMD_CLE(NFC_CE((*in>>24)&0x3),*in&0xff);
			break;
		case 2: //ale
			*out = NFC_CMD_ALE(NFC_CE((*in>>24)&0x3),*in&0xff);
			break;
		case 8: //nop
			*out = NFC_CMD_CLE(NFC_CE((*in>>24)&0x3),*in&0xffff);
			break;
		case 3: //wait
			if((*out>>20)&0xf)
				*out = NFC_CMD_RB(NFC_CE((*in>>24)&0x3),*in&0xff);
			else
				*out = NFC_CMD_RBIO(NFC_RBIO(((*in>>24)&0x3)),(*in&0xff));
			break;
		case 4:
			*out=NFC_CMD_SEED(*in);
			break;
		case 5:
			*out++=NFC_CMD_ASL(*(in+1));
			*out++=NFC_CMD_ASH(*(in+1));

			*out=NFC_CMD_STS((*in>>24)&3);
			in++;
			cur+=2;
			break;
		case 6:
			*out++ = NFC_CMD_ADL(*(in+1));
			*out++ = NFC_CMD_ADH(*(in+1));

			*out++ = NFC_CMD_AIL(*(in+2));
			*out++ = NFC_CMD_AIH(*(in+2));
			*out = NFC_CMD_READ(*in);
			in+=2;
			cur+=4;
			break;
		case 7:
			*out++ = NFC_CMD_ADL(*(in+1));
			*out++ = NFC_CMD_ADH(*(in+1));

			*out++ = NFC_CMD_AIL(*(in+2));
			*out++ = NFC_CMD_AIH(*(in+2));
			*out = NFC_CMD_WRITE(*in);
			in+=2;
			cur+=4;
			break;


		}
		in++;
		cur++;
		out++;
	}
	return cur<outq->size?0:-1;
}
static int32_t v3_write_cmd(cntl_t * cntl ,cmdq_t * cmdq)
{
	DEFINE_CNTL_PRIV(priv,cntl);
	uint32_t size=0;
	cmd_t * p=cmdq->cmd;
	return v3_fifo_write(priv,p,cmdq->size);
}

static int32_t v3_ctrl(cntl_t *cntl, uint16_t ce, uint16_t ctrl)
{
    DEFINE_CNTL_PRIV(priv,cntl);
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
	V3_FIFO_WRITE( NFC_CMD_IDLE(NFC_CE(ce),cycles));
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
 * Jobkey relative functions
 *
 *
 */
static jobkey_t * v3_job_get(cntl_t * cntl,uint32_t mykey)
{
	DEFINE_CNTL_PRIV(priv, cntl);
	int i;
	assert(mykey!=0);
	v3_lock_sts_buf();
	for(i=0;i<priv->sts_size;i++)
	{
		if(priv->sts_buf[i].key==0)
		{
			priv->sts_buf[i].key=mykey;
			priv->sts_buf[i].done=0;
			v3_unlock_sts_buf();
			return (jobkey_t*)(priv->sts_buf+i);
		}
	}
	v3_unlock_sts_buf();
	return NULL;
}
static int32_t v3_job_free(cntl_t * cntl, jobkey_t * job)
{
	sts_t * p=(sts_t *)job;
	v3_lock_sts_buf();
	p->key=0;
	v3_unlock_sts_buf();
	return 0;
}

/**
 *
 * @param cntl_t controller
 * @param jobs	in/out parameter ,the finish status job list
 * @param size	input jobs size
 * @return <0 , error ; >=0 , return size of jobs
 */
static int32_t v3_job_lookup(cntl_t * cntl, jobkey_t ** jobs,uint32_t size)//
{
	DEFINE_CNTL_PRIV(priv, cntl);
	int32_t i;
	int32_t ret;
	assert(jobs!=0);

	for(i=0,ret=0;i<priv->sts_size&&ret<size;i++)
	{
		if(priv->sts_buf[i].key==0)
			continue;
		if(priv->sts_buf[i].done==0)
			continue;
		jobs[ret++]=(jobkey_t*)&(priv->sts_buf[i]);
	}
	return ret;
}
static uint32_t v3_job_key(cntl_t * cntl, jobkey_t * job)
{
	sts_t * p=(sts_t*)job;
	return p->key;
}
static uint32_t v3_job_status(cntl_t * cntl, jobkey_t * job)
{
	sts_t * p=(sts_t*)job;
	return p->st[0];
}

cntl_t * get_v3(void)
{
	return &v3_driver;
}

