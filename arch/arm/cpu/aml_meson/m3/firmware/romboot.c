#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/romboot.h>
#ifndef FIRMWARE_IN_ONE_FILE
#define STATIC_PREFIX
#else
#define STATIC_PREFIX static inline
#endif
#ifndef CONFIG_AML_UBOOT_MAGIC
#define CONFIG_AML_UBOOT_MAGIC 0x12345678
#endif
#ifndef memcpy
#define memcpy ipl_memcpy:
#endif
#ifndef CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK
short check_sum(unsigned * addr,unsigned short check_sum,unsigned size);
#else
STATIC_PREFIX short check_sum(unsigned * addr,unsigned short check_sum,unsigned size)
{
    int i;
    unsigned short * p=(unsigned short *)addr;
    serial_put_dword(addr[15]);
    if(addr[15]!=CONFIG_AML_UBOOT_MAGIC)
        return -1;
#if 0
    for(i=0;i<size>>1;i++)
        check_sum^=p[i];
#endif
    return 0;
}
#endif

SPL_STATIC_FUNC void fw_print_info(unsigned por_cfg,unsigned stage)
{
    serial_puts("Boot from");
    if(stage==0){
        serial_puts(" internal device ");
        if(POR_GET_1ST_CFG(por_cfg) != 0)	{
	        switch(POR_GET_1ST_CFG(por_cfg))
	        {
	        		case POR_1ST_NAND:
	        			serial_puts("1st NAND\n");
	        		break;
	         		case POR_1ST_NAND_RB:
	        			serial_puts("1st NAND RB\n");
	        		break;
	        		case POR_1ST_SPI:
	        			serial_puts("1st SPI\n");
	        		break;
	        		case POR_1ST_SPI_RESERVED:
	        			serial_puts("1st SPI RESERVED\n");
	        		break;
	        		case POR_1ST_SDIO_C:
	        			serial_puts("1st SDIO C\n");
	        		break;
	        		case POR_1ST_SDIO_B:
	        			serial_puts("1st SDIO B\n");
	        		break;
	        		case POR_1ST_SDIO_A:
	        			serial_puts("1ST SDIO A\n");
	        		break;       				
	        }
      }
      else{
      	switch(POR_GET_2ND_CFG(por_cfg))
      	{
      		case POR_2ND_SDIO_C:
      			serial_puts("2nd SDIO C\n");
      			break;
      		case POR_2ND_SDIO_B:
      			serial_puts("2nd SDIO B\n");
      			break;
      		case POR_2ND_SDIO_A:
      			serial_puts("2nd SDIO A\n");
      			break;
      		default:
      			serial_puts("Never checked\n");
       			break;
      	}
      }
      return;
    }
    serial_puts(" external device ");
    return ;
}

#ifdef CONFIG_BOARD_8726M_ARM
#define P_PREG_JTAG_GPIO_ADDR  (volatile unsigned long *)0xc110802c
#endif

STATIC_PREFIX int fw_load_intl(unsigned por_cfg,unsigned target,unsigned size)
{
    int rc=0;
    unsigned * mem;
    unsigned len;
    switch(POR_GET_1ST_CFG(por_cfg))
    {
        case POR_1ST_NAND:
        case POR_1ST_NAND_RB:
            rc=nf_read(target,size);
            break;
        case POR_1ST_SPI :
        case POR_1ST_SPI_RESERVED :
            if(por_cfg&POR_ROM_BOOT_ENABLE)
                mem=(unsigned *)(NOR_START_ADDR+READ_SIZE);
            else
                mem=(unsigned *)(NOR_START_ADDR+READ_SIZE+ROM_SIZE);
            spi_init();
#if CONFIG_UCL

            if(!rc){
	            serial_puts("ucl decompress\n");
	            rc=uclDecompress(target,&len,mem);
                serial_puts("decompress finished\n");
            }
#endif
            if((rc=check_sum(target,0,0))==0)
            {
            	serial_puts("Boot From SPI");
#if CONFIG_UCL==0
                ipl_memcpy((unsigned*)target,mem,size);
#endif
            }
            break;
        case POR_1ST_SDIO_C:
        case POR_1ST_SDIO_B:
        case POR_1ST_SDIO_A:
           rc=sdio_read(target,size,0);
           break;
        default:
           return 1;
    }
    if(rc==0)
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
    return rc;
}
STATIC_PREFIX int fw_init_extl(unsigned por_cfg)
{
	 int rc=sdio_init(por_cfg);
   return rc;
}
STATIC_PREFIX int fw_load_extl(unsigned por_cfg,unsigned target,unsigned size)
{
    unsigned temp_addr;
#if CONFIG_UCL
    temp_addr=target-0x800000;
#else
    temp_addr=target;
#endif
    int rc=sdio_read(temp_addr,size,por_cfg);
#if CONFIG_UCL
    if(!rc){
	    serial_puts("ucl decompress\n");
	    rc=uclDecompress(target,&len,temp_addr);
        serial_puts("decompress finished\n");
    }
#endif

    if(!rc)
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
    return rc;
}
struct load_tbl_s{
    unsigned dest;
    unsigned src;
    unsigned size;
};
extern struct load_tbl_s __load_table[2];
STATIC_PREFIX void load_ext(unsigned por_cfg,unsigned bootid,unsigned target)
{
    int i,rc;
    unsigned temp_addr;
    unsigned len;
#if CONFIG_UCL
    temp_addr=target-0x800000;
#else
    temp_addr=target;
#endif  
     
    if(bootid==0&&(POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI||POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI_RESERVED))
    {
        // spi boot
        if(por_cfg&POR_ROM_BOOT_ENABLE)
            temp_addr=(unsigned *)(NOR_START_ADDR+READ_SIZE);
        else
            temp_addr=(unsigned *)(NOR_START_ADDR+READ_SIZE+ROM_SIZE);
        
    }
        
    for(i=0;i<sizeof(__load_table)/sizeof(__load_table[0]);i++)
    {
        if(__load_table[i].size==0)
            continue;
#if CONFIG_UCL
        if( __load_table[i].size&(~0x3fffff))
        {
            rc=uclDecompress(__load_table[i].dest,&len,temp_addr+__load_table[i].src);
            if(rc)
            {
                serial_put_dword(i);
                serial_puts("decompress Fail\n");
            }
        }else
        
#endif  
        memcpy(__load_table[i].dest,__load_table[i].src+temp_addr,__load_table[i].size&0x3fffff);      
    }
}


