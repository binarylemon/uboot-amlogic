#include <stdarg.h>
#include <amlogic/nand/cntl.h>
#include <amlogic/nand/platform.h>
#include <asm/arch/reg_addr.h>
static ecc_t ecc_table[] =
		{
		{ .name = "BCH OFF  ", .mode = 0, .bits = 0, .data = 0, .parity = 0,
				.info = 0 },
		{ .name = "BCH 8/512", .mode = 1, .bits = 8, .data = 64, .parity = 14,
				.info = 2 },
		{ .name = "BCH  8/1k", .mode = 2, .bits = 8, .data = 128, .parity = 14,
				.info = 2 },
		{ .name = "BCH 16/1k", .mode = 3, .bits = 16, .data = 128, .parity = 28,
				.info = 2 },
		{ .name = "BCH 24/1k", .mode = 4, .bits = 24, .data = 128, .parity = 42,
				.info = 2 },
		{ .name = "BCH 30/1k", .mode = 5, .bits = 30, .data = 128, .parity = 54,
				.info = 2 },
		{ .name = "BCH 40/1k", .mode = 6, .bits = 40, .data = 128, .parity = 70,
				.info = 2 },
		{ .name = "BCH 60/1k", .mode = 7, .bits = 60, .data = 128,
				.parity = 106, .info = 2 },
				{ .name = NULL, .mode = 0, .bits = 0, .data = 0, .parity = 0,
						.info = 0 }, };
static int32_t
v3_config(cntl_t *, uint32_t config, ...); //done the basic
static int32_t
v3_fifo(cntl_t *, uint32_t config, ...);
static uint32_t
v3_size(cntl_t *);
static uint32_t
v3_avail(cntl_t *);
static uint32_t
v3_head(cntl_t *);
static uint32_t
v3_tail(cntl_t *);
static int32_t
v3_ctrl(cntl_t *, uint16_t ce, uint16_t ctrl);
static int32_t
v3_wait(cntl_t *, uint8_t mode, uint16_t ce, uint8_t cycle_log2);
static int32_t
v3_nop(cntl_t *, uint16_t ce, uint16_t cycles);
static int32_t
v3_sts(cntl_t *, jobkey_t job, uint16_t mode);
static int32_t
v3_readbytes(cntl_t *, jobkey_t job, void * addr, uint16_t size);
static int32_t
v3_writebytes(cntl_t *, jobkey_t job, void * addr, uint16_t size);
static int32_t
v3_readecc(cntl_t *, jobkey_t job, void * addr, uint16_t size, void * info,
		ecc_t * ecc);
static int32_t
v3_writeecc(cntl_t *, jobkey_t job, void * addr, uint16_t size, void * info,
		ecc_t * ecc);
static jobkey_t
v3_job_get(cntl_t * cntl_t);
static int32_t
v3_job_select(cntl_t * cntl_t, jobkey_t job);
static int32_t
v3_job_finish(cntl_t * cntl_t, jobkey_t job);
static int32_t
v3_job_status(cntl_t * cntl_t, jobkey_t job);
struct v3_priv
{
	uint32_t delay;
	uint32_t edo;
	uint32_t reg_base;
	uint32_t fifo_mode;
	struct aml_nand_platform * plat;
};
static struct v3_priv v3_priv =
{ .delay = 90, .edo = 2, .reg_base = P_NAND_CMD,

};
#define DEFINE_CNTL_PRIV(priv,cntl) struct v3_priv * priv=get_priv(cntl)
static inline struct v3_priv *
get_priv(cntl_t * cntl)
{
	return cntl->priv;
}

static cntl_t v3_driver =
{ .name = "aml_v3_nand_driver",
		.feature = 0x3f,
		.nand_cycle = 0, //cycle time int 0.1ns
		.ecc = &ecc_table,
		/** configure and control function **/
		.config = v3_config,
		/** fifo relative functions **/
		.fifo = v3_fifo, .size = v3_size, .avail = v3_avail, .head = v3_head,
		.tail = v3_tail,

		/** nand command routines*/
		.ctrl = v3_ctrl, .wait = v3_wait, .nop = v3_nop, .sts = v3_sts,
		.readbytes = v3_readbytes, .writebytes = v3_writebytes, .readecc =
				v3_readecc, .writeecc = v3_writeecc,

		/** util functions for async mode **/
		.job_get = v3_job_get, .job_select = v3_job_select, .job_finish =
				v3_job_finish, .job_status = v3_job_status, .priv = &v3_priv };

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
/*
 Common Nand Read Flow
 */
#define CE0         (0xe<<10)
#define CE1         (0xd<<10)
#define CE2         (0xb<<10)
#define CE3         (0x7<<10)
#define CE_NOT_SEL  (0xf<<10)

#define IO4 ((0xe<<10)|(1<<18))
#define IO5 ((0xd<<10)|(1<<18))
#define IO6 ((0xb<<10)|(1<<18))
#define CLE         (0x5<<14)
#define ALE         (0x6<<14)
#define DWR         (0x4<<14)
#define DRD         (0x8<<14)
#define IDLE        (0xc<<14)
#define RB  		(1<<20)

#define STANDBY     (0xf<<10)

#define PER_INFO_BYTE 8
#define SIZE_INT	  (sizeof(unsigned int))

#define M2N  ((0<<17) | (2<<20) | (1<<19))
#define N2M  ((1<<17) | (2<<20) | (1<<19))

#define M2N_NORAN  0x00200000
#define N2M_NORAN  0x00220000

#define STS  ((3<<17) | (2<<20))
#define ADL  ((0<<16) | (3<<20))
#define ADH  ((1<<16) | (3<<20))
#define AIL  ((2<<16) | (3<<20))
#define AIH  ((3<<16) | (3<<20))
#define ASL  ((4<<16) | (3<<20))
#define ASH  ((5<<16) | (3<<20))
#define SEED ((8<<16) | (3<<20))
/**
 CMD relative Macros
 Shortage word . NFC
 */
#define NFC_CMD_IDLE(ce,time)          ((ce)|IDLE|(time&0x3ff))
#define NFC_CMD_CLE(ce,cmd  )          ((ce)|CLE |(cmd &0x0ff))
#define NFC_CMD_ALE(ce,addr )          ((ce)|ALE |(addr&0x0ff))
#define NFC_CMD_RB(ce,time  )          ((ce)|RB  |(time&0x3ff))
#define NFC_CMD_STANDBY(time)          (STANDBY  |(time&0x3ff))
#define NFC_CMD_ADL(addr)              (ADL     |(addr&0xffff))
#define NFC_CMD_ADH(addr)              (ADH|((addr>>16)&0xffff))
#define NFC_CMD_AIL(addr)              (AIL     |(addr&0xffff))
#define NFC_CMD_AIH(addr)              (AIH|((addr>>16)&0xffff))
#define NFC_CMD_M2N(size,ecc)          (M2N |ecc|(size&0x3fff))
#define NFC_CMD_N2M(size,ecc)          (N2M |ecc|(size&0x3fff))
#define NFC_CMD_DWR(data)              (DWR     |(data&0xff  ))
#define NFC_CMD_DRD(    )              (DRD                   )

#define NFC_INFO_GET()                      (readl(NFC_REG_CMD))
#define NFC_CMDFIFO_SIZE()                  ((NFC_INFO_GET()>>22)&0x1f)

#define NFC_CMD_WAIT_EMPTY()       {while(readl(NFC_REG_CFG)&(1<<12));NFC_CMD_STANDBY(0);NFC_CMD_STANDBY(0);while(NFC_CMDFIFO_SIZE()>0);}

static int32_t v3_config(cntl_t * cntl, uint32_t config, ...)
{
	DEFINE_CNTL_PRIV(priv,cntl);
	va_list ap;
	struct aml_nand_platform * plat;
	int32_t ret;
	uint32_t int_temp;
	uint8_t char_temp;
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
			uint16_t mode=va_arg(ap,uint16_t)&3;
			uint16_t bus_cycle,t_rea=va_arg(ap,uint16_t);
			uint16_t bus_time,t_rhoh=va_arg(ap,uint16_t);
			uint16_t sync_adjust=0;
			assert(mode<3);
			if(mode)
			sync_adjust=va_arg(ap,uint16_t)&1;
			assert(t_rea>=priv->plat->t_rea);
			assert(t_rhoh>=priv->plat->t_rhoh);
			assert(v3_time_caculate(&t_rea,&t_rhoh,priv->edo,priv->delay,clk_get_rate(priv->plat->clk_src)));
			NFC_CMD_WAIT_EMPTY();
			bus_cycle=t_rhoh;
			bus_time=t_rea;
			clrsetbits_le32(NFC_REG_CFG,0xfff|(1<<16),(bus_cycle&0x1f)|((bus_time&0x1f)<<5)| (mode<<10)|(sync_adjust<<16));
		}
		break;

		case NAND_CNTL_FIFO_MODE: //uint16_t start(bit0 cmd fifo: 1=enable 0=disable , bit1 interrupt : 0=disable,1=enable),
		{
			uint16_t mode=va_arg(ap,uint16_t)&3;
			uint16_t xor=mode^priv->fifo_mode;
			if(xor==0)
			break;
			NFC_CMD_WAIT_EMPTY();
			priv->fifo_mode=mode;
			if(xor&2)
			{
				clrsetbits_le32(NFC_REG_CFG,3<<20,(mode&1?3:0)<<20);
			}
		}
		break;
		case NAND_CNTL_POWER_SET:
		///
		break;
	}
	va_end(ap);
	return 0;
}
static int32_t v3_fifo_write(cntl_t *cntl_t, uint32_t cmd)
{

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
	DEFINE_CNTL_PRIV(priv, cntl);
	if (ctrl & 0x100)
		return v3_fifo_write(cntl, NFC_CMD_CLE(NFC_CE(ce),ctrl));
	return v3_fifo_write(cntl, NFC_CMD_ALE(NFC_CE(ce),ctrl));
}
static int32_t v3_wait(cntl_t * cntl, uint8_t mode, uint16_t ce,
		uint8_t cycle_log2)
{
	uint32_t cmd;
	if (mode & 1)
		return v3_fifo_write(cntl, NFC_CMD_RB(NFC_CE(ce),cycle_log2));
	else
	{
		return v3_fifo_write(cntl, NFC_CMD_RBIO(NFC_RBIO(ce), cycle_log2));
	}
	return 0;
}
static int32_t v3_nop(cntl_t * cntl, uint16_t ce, uint16_t cycles)
{
	return v3_fifo_write(cntl, NFC_CMD_IDLE(NFC_CE(ce),cycles));
}
static int32_t v3_sts(cntl_t * cntl, jobkey_t job, uint16_t mode)
{

}
