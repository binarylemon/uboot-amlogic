#ifndef __AMLOGIC_NAND_CNTL_H_
#define __AMLOGIC_NAND_CNTL_H_

#include <linux/types.h>
#define MAX_CMD_SIZE 32
//typedef unsigned uint32_t;
typedef struct __cntl_cmd_s {
    uint32_t size;
    uint32_t cmd[MAX_CMD_SIZE];
}cntl_cmd_t;
typedef uint64_t cntl_feature_t;
typedef struct __cntl_info_s cntl_t;
typedef struct __ecc_info_s ecc_t;
struct __ecc_info_s {
    char *      name;
    //this parameter is basic ecc desription section
    uint8_t     mode:4;//0~16
    uint8_t     info:4;//0,1,2
    uint8_t     bits;
    uint8_t     data;//512/8,1024/8,0
    uint8_t     parity;
    uint32_t    max;
};

typedef uint32_t jobkey_t;
typedef int32_t dma_t;
typedef struct __dma_desc_s{
    dma_t       dma;
    uint16_t    page_size;
    uint16_t    io_size;
    uint8_t     info;
    uint8_t     parity;
    uint8_t     pages;
}dma_desc_t;
struct __cntl_info_s{
    const char * name;
    cntl_feature_t feature;
    uint32_t    nand_cycle; //cycle time int 0.1ns 
    ecc_t       *ecc;
    int32_t  (* ecc2dma)(ecc_t * orig,dma_desc_t* dma,uint32_t size,uint32_t short_size,uint32_t seed);
    int32_t  (* info2data)(void * data,void * info,dma_t dma);//-1,found ecc fail,>=0,ecc counter .
    int32_t  (* data2info)(void * info,void * data,dma_t dma);//-1,error found
    
    /** configure and control function **/
    int32_t (* config)(cntl_t *, uint32_t config,...);
    /** fifo relative functions **/
    uint32_t (* size)(cntl_t *);
    uint32_t (* avail)(cntl_t *);
    uint32_t (* head)(cntl_t *);
    uint32_t (* tail)(cntl_t *);

    /** nand command routines*/
    int32_t   (* ctrl)(cntl_t *, uint16_t ce,uint16_t ctrl);
    int32_t   (* wait)(cntl_t *, uint8_t mode,uint16_t ce,uint8_t cycle_log2);
    int32_t    (* nop)(cntl_t *, uint16_t ce,uint16_t cycles);
    int32_t    (* sts)(cntl_t *,jobkey_t* job, uint16_t mode);
    int32_t   (* readbytes)(cntl_t *,void * addr,dma_t dma_mode);
    int32_t  (* writebytes)(cntl_t *,void * addr,dma_t dma_mode);
    int32_t     (* readecc)(cntl_t *,void * addr ,void * info,dma_t dma_mode);
    int32_t    (* writeecc)(cntl_t *,void * addr ,void * info,dma_t dma_mode);
    int32_t    (* seed)(cntl_t *, uint16_t seed);//0 disable

    /** util functions for async mode **/
    jobkey_t*  (* job_get)(cntl_t * cntl_t,uint32_t mykey);
    int32_t  (* job_free)(cntl_t * cntl_t,jobkey_t* job);
    int32_t  (* job_status)(cntl_t * cntl_t,jobkey_t* job);
    void *   priv;
};

#define FEATURE_SUPPORT_SHORT_ECC       (1<<0)
#define FEATURE_SUPPORT_NO_RB           (1<<1)
#define FEATURE_SUPPORT_STS_INTERRUPT   (1<<2)
#define FEATURE_SUPPORT_TOGGLE          (1<<3)
#define FEATURE_SUPPORT_SYNC            (1<<3)
#define FEATURE_SUPPORT_CMDFIFO         (1<<4)
#define FEATURE_SUPPORT_SCRAMBLE        (1<<5)
#define FEATURE_SUPPORT_MAX_CES         (0xf<<6)

/*
    config command
*/
#define NAND_CNTL_MODE_SET      0   // uint32_t io_base,uint32_t nand_delay,uint8_t edo,uint8_t rbmod,void (* claim_bus)(uint32_t get)
#define NAND_CNTL_TIME_SET      1   // t_rea,t_rhoh
#define NAND_CNTL_FIFO_MODE     2
#define NAND_CNTL_POWER_SET     3   //@todo

#endif
