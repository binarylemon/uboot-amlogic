
#ifndef _ASM_NAND_H_INCLUDED
#define _ASM_NAND_H_INCLUDED
//This File only include the register define relative code
//It should describe some
#include <asm/arch/io.h>
#include "reg_addr.h"
#include <config.h>
#if CONFIG_JERRY_NAND_TEST
/** Register defination **/

#define NAND_CYCLE_DELAY	  90
#ifndef NFC_BASE
#define NFC_BASE			  CBUS_REG_ADDR(NAND_CMD)
#endif
#define NFC_OFF_CMD           (0 )
#define NFC_OFF_CFG           (4 )
#define NFC_OFF_DADR          (8 )
#define NFC_OFF_IADR          (12)
#define NFC_OFF_BUF           (16)
#define NFC_OFF_INFO          (20)
#define NFC_OFF_DC            (24)
#define NFC_OFF_ADR           (28)
#define NFC_OFF_DL            (32)
#define NFC_OFF_DH            (36)

#define P_NAND_CMD             (NFC_BASE+0 )
#define P_NAND_CFG             (NFC_BASE+4 )
#define P_NAND_DADR            (NFC_BASE+8 )
#define P_NAND_IADR            (NFC_BASE+12)
#define P_NAND_BUF             (NFC_BASE+16)
#define P_NAND_INFO            (NFC_BASE+20)
#define P_NAND_DC              (NFC_BASE+24)
#define P_NAND_ADR             (NFC_BASE+28)
#define P_NAND_DL              (NFC_BASE+32)
#define P_NAND_DH              (NFC_BASE+36)
#define P_NAND_CADR            (NFC_BASE+40)
#define P_NAND_SADR            (NFC_BASE+44)

/*
   Common Nand Read Flow
*/
#define CE0         (0xe<<10)
#define CE1         (0xd<<10)
#define CE2         (0xb<<10)
#define CE3         (0x7<<10)
#define CE_NOT_SEL  (0xf<<10)
#define IO4         (0xe<<10)
#define IO5         (0xd<<10)
#define IO6         (0xb<<10)
#define CLE         (0x5<<14)
#define ALE         (0x6<<14)
#define DWR         (0x4<<14)
#define DRD         (0x8<<14)
#define IDLE        (0xc<<14)
#define RB  		(1<<20)

#define STANDBY     (0xf<<10)

#define PER_INFO_BYTE 8       //64 bit  P_NAND_INFO now
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
    Nand Flash Controller (M3)
    Global Macros
*/
/**
   Config Group
*/
#define NFC_SET_CMD_START()						   		setbits_le32(P_NAND_CFG,1<<12)
#define NFC_SET_CMD_AUTO()						   		setbits_le32(P_NAND_CFG,1<<13)
#define NFC_GET_CMD_FIFO_STATUS()                       ((readl(P_NAND_CFG)>>12)&3)

#define NFC_SET_STS_IRQ(en)					       		clrsetbits_le32(P_NAND_CFG,1<<20,en<<20)
#define NFC_SET_CMD_IRQ(en)					       		clrsetbits_le32(P_NAND_CFG,1<<21,en<<21)
#define NFC_SET_TIMING_ASYC(bus_tim,bus_cyc)       		WRITE_CBUS_REG_BITS(NAND_CFG,((bus_cyc&31)|((bus_tim&31)<<5)|(0<<10)),0,12)
#define NFC_SET_TIMING_SYNC(bus_tim,bus_cyc,sync_mode)  WRITE_CBUS_REG_BITS(NAND_CFG,(bus_cyc&31)|((bus_tim&31)<<5)|((sync_mode&2)<<10),0,12)
#define NFC_SET_TIMING_SYNC_ADJUST()
#define NFC_SET_DMA_MODE(is_apb,spare_only)        WRITE_CBUS_REG_BITS(NAND_CFG,((spare_only<<1)|(is_apb)),14,2)

/**
    CMD relative Macros
    Shortage word . NFCC
*/
#define NFC_CMD_IDLE(ce,time)          ((ce)|IDLE|(time&0x3ff))
#define NFC_CMD_CLE(ce,cmd  )          ((ce)|CLE |(cmd &0x0ff))
#define NFC_CMD_ALE(ce,addr )          ((ce)|ALE |(addr&0x0ff))
#define NFC_CMD_STANDBY(time)          (STANDBY  |(time&0x3ff))
#define NFC_CMD_ADL(addr)              (ADL     |(addr&0xffff))
#define NFC_CMD_ADH(addr)              (ADH|((addr>>16)&0xffff))
#define NFC_CMD_AIL(addr)              (AIL     |(addr&0xffff))
#define NFC_CMD_AIH(addr)              (AIH|((addr>>16)&0xffff))
#define NFC_CMD_ASL(addr)              (ASL     |(addr&0xffff))
#define NFC_CMD_ASH(addr)              (ASH|((addr>>16)&0xffff))

#define NFC_CMD_DWR(data)              (DWR     |(data&0xff  ))
#define NFC_CMD_DRD(ce,size)           (ce|DRD|size)
#define NFC_CMD_RB(ce,time  )          ((ce)|RB  |(time&0x1f))
#define NFC_CMD_RB_ID(ce,id,time  )          ((ce)|RB  |(time&0x1f)|((id&0x1f)<<5))

#define NFC_CMD_RB_INT(ce,time)        ((ce)|RB|(((ce>>10)^0xf)<<14)|(time&0x1f))
#define NFC_CMD_RBIO(time,io)		   (RB|io|(time&0x1f)|(1<<18))
#define NFC_CMD_RBIO_ID(io,id,time)		   (RB|io|(time&0x1f)|(1<<18)|((id&0x1f)<<5))
#define NFC_CMD_RBIO_INT(io,time)      (RB|(((io>>10)^0x7)<<14)|(time&0x1f)|(1<<18))
#define NFC_CMD_SEED(seed)			   (SEED|(seed&0x7fff))
#define NFC_CMD_STS(tim) 			   (STS|(tim&3))
#define NFC_CMD_M2N(ran,ecc,sho,pgsz,pag)      ((ran?M2N:M2N_NORAN)|(ecc<<14)|(sho<<13)|((pgsz&0x7f)<<6)|(pag&0x3f))
#define NFC_CMD_N2M(ran,ecc,sho,pgsz,pag)      ((ran?N2M:N2M_NORAN)|(ecc<<14)|(sho<<13)|((pgsz&0x7f)<<6)|(pag&0x3f))
#define NFC_DMA_MASK                            ((1<<19)|(0x1ffff))
#define NFC_CMD_WRITE(dma)      (M2N_NORAN|(dma&(NFC_DMA_MASK)))
#define NFC_CMD_READ(dma)       (N2M_NORAN|(dma&(NFC_DMA_MASK)))




/**
    Alias for CMD
*/
#define NFC_CMD_D_ADR(addr)         NFC_CMD_ADL(addr),NFC_CMD_ADH(addr)
#define NFC_CMD_I_ADR(addr)         NFC_CMD_ADI(addr),NFC_CMD_ADI(addr)


/**
    Register Operation and Controller Status
*/
#define NFC_SEND_CMD(cmd)           (WRITE_CBUS_REG(NAND_CMD,cmd))
#define NFC_READ_INFO()             (READ_CBUS_REG(NAND_CMD))
/** ECC defination(M3) */

#define NAND_ECC_NONE             (0x0)
#define NAND_ECC_BCH8_512         (0x1)
#define NAND_ECC_BCH8_1K          (0x2)
#define NAND_ECC_BCH16            (0x3)
#define NAND_ECC_BCH24            (0x4)
#define NAND_ECC_BCH30			  (0x5)
#define NAND_ECC_BCH40			  (0x6)
#define NAND_ECC_BCH60			  (0x7)

/**
    Cmd FIFO control
*/
#define NFC_CMD_FIFO_GO()               (WRITE_CBUS_REG(NAND_CMD,(1<<30)))
#define NFC_CMD_FIFO_RESET()            (WRITE_CBUS_REG(NAND_CMD,(1<<31)))
/**
    ADDR operations
*/
#define NFC_SET_DADDR(a)         (WRITE_CBUS_REG(NAND_DADR,(unsigned)a))
#define NFC_SET_IADDR(a)         (WRITE_CBUS_REG(NAND_IADR,(unsigned)a))
#define NFC_SET_SADDR(a)		 (WRITE_CBUS_REG(NAND_SADR,(unsigned)a))

/**
    Send command directly
*/
#define NFC_SEND_CMD_IDLE(ce,time)          NFC_SEND_CMD(NFC_CMD_IDLE(ce,time))
#define NFC_SEND_CMD_CLE(ce,cmd  )          NFC_SEND_CMD(NFC_CMD_CLE(ce,cmd))
#define NFC_SEND_CMD_ALE(ce,addr )          NFC_SEND_CMD(NFC_CMD_ALE(ce,addr))
#define NFC_SEND_CMD_STANDBY(time)          NFC_SEND_CMD(NFC_CMD_STANDBY(time))
#define NFC_SEND_CMD_ADL(addr)              NFC_SEND_CMD(NFC_CMD_ADL(addr))
#define NFC_SEND_CMD_ADH(addr)              NFC_SEND_CMD(NFC_CMD_ADH(addr))
#define NFC_SEND_CMD_AIL(addr)              NFC_SEND_CMD(NFC_CMD_AIL(addr))
#define NFC_SEND_CMD_AIH(addr)              NFC_SEND_CMD(NFC_CMD_AIH(addr))
#define NFC_SEND_CMD_DWR(data)              NFC_SEND_CMD(NFC_CMD_DWR(data))
#define NFC_SEND_CMD_DRD(ce,size)           NFC_SEND_CMD(NFC_CMD_DRD(ce,size))
#define NFC_SEND_CMD_RB(ce,time)          	NFC_SEND_CMD(NFC_CMD_RB(ce,time))
#define NFC_SEND_CMD_SEED(seed)				NFC_SEND_CMD(NFC_CMD_SEED(seed))
#define NFC_SEND_CMD_M2N(ran,ecc,sho,pgsz,pag)   NFC_SEND_CMD(NFC_CMD_M2N(ran,ecc,sho,pgsz,pag))
#define NFC_SEND_CMD_N2M(ran,ecc,sho,pgsz,pag)   NFC_SEND_CMD(NFC_CMD_N2M(ran,ecc,sho,pgsz,pag))

#define NFC_SEND_CMD_M2N_RAW(ran,len)	NFC_SEND_CMD((ran?M2N:M2N_NORAN)|(len&0x3fff))
#define NFC_SEND_CMD_N2M_RAW(ran,len)   NFC_SEND_CMD((ran?N2M:N2M_NORAN)|(len&0x3fff))

/**
    Cmd Info Macros
*/
#define NFC_CMDFIFO_MAX						(0x1f)
#define NFC_INFO_GET()                      (readl(P_NAND_CMD))
#define NFC_CMDFIFO_SIZE()                  ((NFC_INFO_GET()>>22)&0x1f)
#define NFC_CMDFIFO_AVAIL()                 (0x1f-((NFC_INFO_GET()>>22)&0x1f))
#define NFC_CHECEK_RB_TIMEOUT()             ((NFC_INFO_GET()>>27)&0x1)
#define NFC_GET_RB_STATUS(ce)               (((NFC_INFO_GET()>>28)&(~(ce>>10)))&0xf)
#define NFC_GET_BUF() 					    READ_CBUS_REG(NAND_BUF)
#define NFC_SET_CFG(val) 			      	(WRITE_CBUS_REG(NAND_CFG,(unsigned)val))
#define NFC_FIFO_CUR_CMD()				    ((NFC_INFO_GET()>>22)&0x3FFFFF)


#define NAND_INFO_DONE(a)         (((a)>>31)&1)
#define NAND_ECC_ENABLE(a)        (((a)>>30)&1)
#define NAND_ECC_CNT(a)           (((a)>>24)&0x3f)
#define NAND_ZERO_CNT(a)	      (((a)>>16)&0x3f)
#define NAND_INFO_DATA_2INFO(a)   ((a)&0xffff)
#define NAND_INFO_DATA_1INFO(a)   ((a)&0xff)


#define NFC_SET_SPARE_ONLY()			(SET_CBUS_REG_MASK(NAND_CFG,1<<15))
#define NFC_CLEAR_SPARE_ONLY()			(CLEAR_CBUS_REG_MASK(NAND_CFG,1<<15))
#define NFC_GET_BUF() 					READ_CBUS_REG(NAND_BUF)
#define NFC_GET_CFG() 					READ_CBUS_REG(NAND_CFG)


static inline void  NAND_IO_ENABLE(uint32_t mode)
{
		if(mode==0)
		{
			clrbits_le32(P_PERIPHS_PIN_MUX_2,(0x3ff<<18));//disable nand
			return ;
		}
		setbits_le32(P_PAD_PULL_UP_REG3,(0xff<<0) | (1<<16));
        clrbits_le32(P_PERIPHS_PIN_MUX_4,(0x1f<<26));//sdxc
        clrbits_le32(P_PERIPHS_PIN_MUX_6,(0x3f<<24));//sdio-C
        clrbits_le32(P_PERIPHS_PIN_MUX_3,(0x1<<31));//I2C_SDA--production
        clrsetbits_le32(P_PERIPHS_PIN_MUX_5,(0x7<<7)|(0x7<<1),(0x7<<7));//disable spi
        setbits_le32(P_PERIPHS_PIN_MUX_2,(0x3ff<<18));//set nand
}
#define NAND_IO_DISABLE(mode) NAND_IO_ENABLE(0)
#else
#include "aml_nand.h"

#define NAND_IO_ENABLE(mode)        {               \
        SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0x1f5); \
	    SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_1, ((1<<30) | (1<<28) | (1<<26) | (1<<24))); \
	    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_1, ((1<<29) | (1<<27) | (1<<25) | (1<<23))); \
	    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, ((1<<29) | (1<<28) | (1<<27) | (1<<26) | (1<<25) | (1<<24))); \
    }
#define NAND_IO_DISABLE(mode) {                         \
        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0x7fff); \
	    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_1, ((1<<30) | (1<<28) | (1<<26) | (1<<24)));    \
    }
#endif



#endif // NAND_H_INCLUDED

