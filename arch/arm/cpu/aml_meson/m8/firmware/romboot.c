#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/romboot.h>

#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
extern int uclDecompress(char* op, unsigned* o_len, char* ip);
#endif
#endif

#ifndef FIRMWARE_IN_ONE_FILE
#define STATIC_PREFIX
#else
#define STATIC_PREFIX static inline
#endif
#ifndef CONFIG_AML_UBOOT_MAGIC
#define CONFIG_AML_UBOOT_MAGIC 0x12345678
#endif

#ifndef CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK
short check_sum(unsigned * addr,unsigned short check_sum,unsigned size);
#else
STATIC_PREFIX short check_sum(unsigned * addr,unsigned short check_sum,unsigned size)
{    
    serial_put_dword(addr[15]);
    if(addr[15]!=CONFIG_AML_UBOOT_MAGIC)
        return -1;
#if 0
	int i;   
	unsigned short * p=(unsigned short *)addr;
    for(i=0;i<size>>1;i++)
        check_sum^=p[i];
#endif
    return 0;
}
#endif

#if defined(CONFIG_M8_SECU_BOOT)
static int aml_m8_sec_boot_check(unsigned char *pSRC)
{

#if !defined(CONFIG_AMLROM_SPL)
	#define ipl_memcpy memcpy
#endif //

#define AMLOGIC_CHKBLK_ID  (0x434C4D41) //414D4C43 AMLC
#define AMLOGIC_CHKBLK_VER (1)

	typedef struct {
		unsigned int	nSizeH; 		  ///4
		struct st_secure{
			unsigned int	nORGFileLen;  ///4
			unsigned int	nSkippedLen;  ///4
			unsigned int	nHASHLength;  ///4
			unsigned int	nAESLength;   ///4
			unsigned char	szHashKey[116];//116
		}secure; //136
		unsigned char	szAES_Key_IMG[108];//108
		unsigned int	nSizeT; 		  ///4
		unsigned int	nVer;			  ///4
		unsigned int	unAMLID;		  ///4 AMLC
	}st_aml_chk_blk; //256

	typedef struct{
		int ver;
		int len;
		unsigned char szBuffer[200];
	} rsa_context;

	typedef struct{
		int nr;
		unsigned int buff[80];
	} aes_context;

	int i;
	int nRet = -1;
	rsa_context rsa_ctx;

	unsigned int action[2][10]={
		{0xd90402F4,0xd904038c,0xd904706c,0xd904441c,0xd9047458,
		 0xd9044518,0xd904455c,0xd904459c,0xd9046e44,0xd9046ef4},
		{0xd90402F4,0xd904038c,0xd904706c,0xd904441c,0xd9047458,
		 0xd9044518,0xd904455c,0xd904459c,0xd9046e44,0xd9046ef4}};

	int nStep = 0;
	switch(* (unsigned int *)0xd9040004)
	{
	case 0x25e2:break;
	default: goto exit;break;
	}

	typedef  void (*func_1)( unsigned int * p, unsigned int a ,unsigned int b);
	typedef  void (*func_2)( rsa_context *ctx, int a, int b);
	typedef  int  (*func_3)( rsa_context *ctx, int a );
	typedef  int  (*func_4)( rsa_context *ctx, const unsigned char *pa,unsigned char *pb);
	typedef  void (*func_5)( aes_context *ctx, const char *p);
	typedef  void (*func_6)( unsigned char *iv );
	typedef  void (*func_7)( aes_context *ctx,unsigned char *pa, unsigned int *pb, unsigned int *pc );
	typedef  void (*func_8)( const unsigned char *pa, unsigned int a,unsigned char *pb, int b );

	func_1 fp_1 = (func_1)action[nStep][1];
	func_2 fp_2 = (func_2)action[nStep][2];
	func_3 fp_3 = (func_3)action[nStep][3];
	func_4 fp_4 = (func_4)action[nStep][4];
	func_5 fp_5 = (func_5)action[nStep][5];
	func_6 fp_6 = (func_6)action[nStep][6];
	func_7 fp_7 = (func_7)action[nStep][7];
	func_8 fp_8 = (func_8)action[nStep][8];

	unsigned int nState  = 0;
	fp_1(&nState,0,4);
	fp_2(&rsa_ctx,0,0);
	fp_3(&rsa_ctx,(nState & (1<<23)) ? 1 : 0);
	rsa_ctx.len = (nState & (1<<23)) ? 256 : 128;

	st_aml_chk_blk chk_blk;
	ipl_memcpy((unsigned char*)&chk_blk,(unsigned char*)pSRC,sizeof(chk_blk));
	unsigned char *pRSABuf = (unsigned char *)&chk_blk;
	for(i = 0;i< sizeof(chk_blk);i+=rsa_ctx.len)
		if(fp_4(&rsa_ctx, pRSABuf+i, pRSABuf+i ))
			goto exit;

	//check ID
	if(AMLOGIC_CHKBLK_ID != chk_blk.unAMLID ||
			AMLOGIC_CHKBLK_VER < chk_blk.nVer)
			goto exit;


	//size is match or not
	if(sizeof(st_aml_chk_blk) != chk_blk.nSizeH ||
		sizeof(st_aml_chk_blk) != chk_blk.nSizeT ||
		chk_blk.nSizeT != chk_blk.nSizeH)
		goto exit;

	//check skipped len
	if((sizeof(st_aml_chk_blk) != chk_blk.secure.nSkippedLen))
		goto exit;

	if(chk_blk.secure.nSkippedLen)
		ipl_memcpy((unsigned char*)pSRC,(unsigned char*)(pSRC+chk_blk.secure.nORGFileLen),
			chk_blk.secure.nSkippedLen);

	aes_context aes_ctx;
	uint8_t aes_key[32+16], *aes_IV;

	ipl_memcpy((unsigned char*)aes_key,(unsigned char*)chk_blk.szAES_Key_IMG,sizeof(aes_key));
	aes_IV = &aes_key[32];

	unsigned int *ct32 = (unsigned int *)(pSRC);

	fp_5( &aes_ctx, aes_key );
	fp_6(aes_IV);

	for (i=0; i<(chk_blk.secure.nAESLength)/16; i++)
		fp_7 ( &aes_ctx, aes_IV, &ct32[i*4], &ct32[i*4] );

	unsigned char hashIMGCHK[32];
	fp_8( pSRC,chk_blk.secure.nHASHLength, hashIMGCHK, 0 );

	for(i = 0;i<sizeof(hashIMGCHK);++i)
		if(hashIMGCHK[i] != chk_blk.secure.szHashKey[i])
			goto exit;

	nRet = 0;

exit:

	if(nRet)
	{
#if defined(CONFIG_AMLROM_SPL)
		serial_puts("Aml log : ERROR! TPL secure check fail!\n");
#else
		printf("Aml log : ERROR! Image secure check fail!\n");
#endif
	}
	else
	{

#if defined(CONFIG_AMLROM_SPL)
		serial_puts("Aml log : TPL secure check pass!\n");
#else
		printf(" Aml log : Image secure check pass!\n");

#endif

	}

#if !defined(CONFIG_AMLROM_SPL)
	#undef ipl_memcpy
#endif //
	return nRet;
}
	
#endif //CONFIG_M8_SECU_BOOT


SPL_STATIC_FUNC void fw_print_info(unsigned por_cfg,unsigned stage)
{
    serial_puts("Boot from");
    if(stage==0){
        serial_puts(" internal device ");
        if(POR_GET_1ST_CFG(por_cfg) != 0)	{
	    switch(POR_GET_1ST_CFG(por_cfg))
            //switch(POR_1ST_SPI)
	        {
	        		case POR_1ST_NAND:
	        			serial_puts("1st NAND\n");
	        		break;
	         		//case POR_1ST_NAND_RB:
	        		//	serial_puts("1st NAND RB\n");
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
    unsigned temp_addr;
#if CONFIG_UCL
#if defined (CONFIG_VLSI_EMULATOR)
    temp_addr=target;
#else
    temp_addr=target+0x800000;
#endif
#else
    temp_addr=target;
#endif

    unsigned * mem;    
    
    serial_puts("amlogic log : TPL size is ");
    serial_put_dec(size);
    serial_puts("Bytes\n");

    unsigned int nTEBegin = TIMERE_GET();
    
    //switch(POR_1ST_SPI)
    switch(POR_GET_1ST_CFG(por_cfg))
    {
        case POR_1ST_NAND:
        //case POR_1ST_NAND_RB:        	
            rc=nf_read(temp_addr,size);            
            break;
        case POR_1ST_SPI :
        case POR_1ST_SPI_RESERVED :
            mem=(unsigned *)(NOR_START_ADDR+READ_SIZE);
            spi_init();
#if CONFIG_UCL==0
            if((rc=check_sum(target,0,0))!=0)
            {
                return rc;
            }
#endif
            serial_puts("Boot From SPI\n");
            //memcpy((unsigned*)temp_addr,mem,size);
			ipl_memcpy((unsigned char*)temp_addr,(unsigned char*)mem,size);
            break;
        case POR_1ST_SDIO_C:
        	serial_puts("Boot From SDIO C\n");
        	rc=sdio_read(temp_addr,size,POR_2ND_SDIO_C<<2);
        	break;
        case POR_1ST_SDIO_B:
        	rc=sdio_read(temp_addr,size,POR_2ND_SDIO_B<<2);break;
        case POR_1ST_SDIO_A:
           rc=sdio_read(temp_addr,size,POR_2ND_SDIO_A<<2);break;
           break;
        default:
           return 1;
    }

    serial_puts("\nLoad image done and time use : ");
    serial_put_dec(get_utimer(nTEBegin));
    serial_puts(" us\n");

#if defined(CONFIG_M8_SECU_BOOT)

	if(aml_m8_sec_boot_check((unsigned char *)temp_addr))
	{		
		writel((1<<22)|(3<<24)|500, P_WATCHDOG_TC);
		while(1);
	}	
#endif //CONFIG_M8_SECU_BOOT

    nTEBegin = TIMERE_GET();

#if defined (CONFIG_VLSI_EMULATOR)
    serial_puts("Load uncompressed image for PxP!\n");
#else	

#if CONFIG_UCL    
#ifndef CONFIG_IMPROVE_UCL_DEC
	unsigned len;    
    if(rc==0){
        serial_puts("ucl decompress\n");
        rc=uclDecompress((char*)target,&len,(char*)temp_addr);
        serial_puts(rc?"decompress false\n":"decompress true\n");
    }
#endif    
#endif    
    serial_puts("\nImage ucl-decompress done and time use : ");
    serial_put_dec(get_utimer(nTEBegin));
    serial_puts(" us\n");
#endif //#if defined (CONFIG_VLSI_EMULATOR)

#ifndef CONFIG_IMPROVE_UCL_DEC
    if(rc==0)    	
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
#else
    if(rc==0)    	
        rc=check_sum((unsigned*)temp_addr,magic_info->crc[1],size);
#endif              	
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
    temp_addr=target+0x800000;
#else
    temp_addr=target;
#endif
    int rc=sdio_read(temp_addr,size,por_cfg);   

#if defined(CONFIG_M8_SECU_BOOT)
	if(aml_m8_sec_boot_check((unsigned char *)temp_addr))
	{		
		writel((1<<22)|(3<<24)|500, P_WATCHDOG_TC);
		while(1);
	}	
#endif //CONFIG_M8_SECU_BOOT


#if defined (CONFIG_VLSI_EMULATOR)
    serial_puts("Load uncompressed image from SD 1 for PxP!\n");
#else


#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
	unsigned len;
    if(!rc){
	    serial_puts("ucl decompress\n");
	    rc=uclDecompress((char*)target,&len,(char*)temp_addr);
        serial_puts(rc?"decompress false\n":"decompress true\n");
    }
#endif    
#endif

#endif //#if defined (CONFIG_VLSI_EMULATOR)

#ifndef CONFIG_IMPROVE_UCL_DEC
    if(!rc)
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
#else
    if(!rc)
        rc=check_sum((unsigned*)temp_addr,magic_info->crc[1],size);
#endif        
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
    int i;
    unsigned temp_addr;
#if CONFIG_UCL
    temp_addr=target+0x800000;
#else
    temp_addr=target;
#endif  
     
    //~ if(bootid==0&&(POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI||POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI_RESERVED))
    //~ {
        // spi boot
        temp_addr=(unsigned)(NOR_START_ADDR+READ_SIZE);
    //~ }
        
    for(i=0;i<sizeof(__load_table)/sizeof(__load_table[0]);i++)
    {
        if(__load_table[i].size==0)
            continue;
#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
    	unsigned len;
    	int rc;
        if( __load_table[i].size&(~0x3fffff))
        {
            rc=uclDecompress((char*)(__load_table[i].dest),&len,(char*)(temp_addr+__load_table[i].src));
            if(rc)
            {
                serial_put_dword(i);
                serial_puts("decompress Fail\n");
            }
        }else
#endif        
#endif  
        memcpy((void*)(__load_table[i].dest),(const void*)(__load_table[i].src+temp_addr),__load_table[i].size&0x3fffff);      
    }
}


