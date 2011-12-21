#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/efuse.h>
#include "efuse_bch_8.h"
#include "efuse_regs.h"


char efuse_buf[EFUSE_BYTES] = {0};
static void __efuse_write_byte( unsigned long addr, unsigned long data );
static void __efuse_read_dword( unsigned long addr, unsigned long *data);

extern int efuseinfo_num;
extern efuseinfo_t efuseinfo[];
extern int efuse_active_version;
extern int efuse_active_customerid;
extern pfn efuse_getinfoex;
extern int printf(const char *fmt, ...);

static void __efuse_write_byte( unsigned long addr, unsigned long data )
{
	//printf("addr=%d, data=%x\n", addr, data);
    unsigned long auto_wr_is_enabled = 0;

    if ( READ_CBUS_REG( EFUSE_CNTL1) & ( 1 << CNTL1_AUTO_WR_ENABLE_BIT ) )
    {
        auto_wr_is_enabled = 1;
    }
    else
    {
        /* temporarily enable Write mode */
        WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_ENABLE_ON,
            CNTL1_AUTO_WR_ENABLE_BIT, CNTL1_AUTO_WR_ENABLE_SIZE );
    }

    /* write the address */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, addr,
        CNTL1_BYTE_ADDR_BIT, CNTL1_BYTE_ADDR_SIZE );
    /* set starting byte address */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_BYTE_ADDR_SET_ON,
        CNTL1_BYTE_ADDR_SET_BIT, CNTL1_BYTE_ADDR_SET_SIZE );
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_BYTE_ADDR_SET_OFF,
        CNTL1_BYTE_ADDR_SET_BIT, CNTL1_BYTE_ADDR_SET_SIZE );

    /* write the byte */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, data,
        CNTL1_BYTE_WR_DATA_BIT, CNTL1_BYTE_WR_DATA_SIZE );
    /* start the write process */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_START_ON,
        CNTL1_AUTO_WR_START_BIT, CNTL1_AUTO_WR_START_SIZE );
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_START_OFF,
        CNTL1_AUTO_WR_START_BIT, CNTL1_AUTO_WR_START_SIZE );
    /* dummy read */
    READ_CBUS_REG( EFUSE_CNTL1 );

    while ( READ_CBUS_REG(EFUSE_CNTL1) & ( 1 << CNTL1_AUTO_WR_BUSY_BIT ) )
    {
        udelay(1);
    }

    /* if auto write wasn't enabled and we enabled it, then disable it upon exit */
    if (auto_wr_is_enabled == 0 )
    {
        WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_ENABLE_OFF,
            CNTL1_AUTO_WR_ENABLE_BIT, CNTL1_AUTO_WR_ENABLE_SIZE );
    }
}

static void __efuse_read_dword( unsigned long addr, unsigned long *data )
{
    unsigned long auto_rd_is_enabled = 0;
    
    if( READ_CBUS_REG(EFUSE_CNTL1) & ( 1 << CNTL1_AUTO_RD_ENABLE_BIT ) )
    {
        auto_rd_is_enabled = 1;
    }
    else
    {
        /* temporarily enable Read mode */
        WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
    }

    /* write the address */
    
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, addr,
        CNTL1_BYTE_ADDR_BIT,  CNTL1_BYTE_ADDR_SIZE );
    	
    /* set starting byte address */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_BYTE_ADDR_SET_ON,
        CNTL1_BYTE_ADDR_SET_BIT, CNTL1_BYTE_ADDR_SET_SIZE );	
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_BYTE_ADDR_SET_OFF,
        CNTL1_BYTE_ADDR_SET_BIT, CNTL1_BYTE_ADDR_SET_SIZE );
   /* start the read process */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_START_ON,
        CNTL1_AUTO_RD_START_BIT, CNTL1_AUTO_RD_START_SIZE );      
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_WR_START_OFF,
        CNTL1_AUTO_RD_START_BIT, CNTL1_AUTO_RD_START_SIZE );
      
    /* dummy read */
    READ_CBUS_REG( EFUSE_CNTL1 );
    while ( READ_CBUS_REG(EFUSE_CNTL1) & ( 1 << CNTL1_AUTO_RD_BUSY_BIT ) )
    {
        udelay(1);
    }
    /* read the 32-bits value */
    ( *data ) = READ_CBUS_REG( EFUSE_CNTL2 );    
    /* if auto read wasn't enabled and we enabled it, then disable it upon exit */
    if ( auto_rd_is_enabled == 0 )
    {
        WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
    }

    //printf("__efuse_read_dword: addr=%ld, data=0x%lx\n", addr, *data);
}

int efuse_init(void)
{	
    /* disable efuse encryption */
    WRITE_CBUS_REG_BITS( EFUSE_CNTL4, CNTL1_AUTO_WR_ENABLE_OFF,
        CNTL4_ENCRYPT_ENABLE_BIT, CNTL4_ENCRYPT_ENABLE_SIZE );

    return 0;
}

ssize_t efuse_read(char *buf, size_t count, loff_t *ppos )
{
    unsigned long contents[EFUSE_DWORDS];
	unsigned pos = *ppos;
    unsigned long *pdw;
    unsigned residunt = pos%4;
    unsigned int dwsize = (count+residunt+3)>>2;
    
	if (pos >= EFUSE_BYTES)
		return 0;
	if (count > EFUSE_BYTES - pos)
		count = EFUSE_BYTES - pos;
	if (count > EFUSE_BYTES)
		return -1;

    memset(contents, 0, sizeof(contents));

#ifdef CONFIG_M3
	// Enabel auto-read mode
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif

    pos = (pos/4)*4;
    for (pdw = contents; dwsize-- > 0 && pos < EFUSE_BYTES; pos += 4, ++pdw)
		__efuse_read_dword(pos, pdw);	    

#ifdef CONFIG_M3
    // Disable auto-read mode    
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif
            
	memcpy(buf, (char*)contents+residunt, count);	
	
    *ppos += count;
    return count;
}

ssize_t efuse_write(const char *buf, size_t count, loff_t *ppos )
{ 	
	unsigned pos = *ppos;
	const char *pc;

	if (pos >= EFUSE_BYTES)
		return 0;	/* Past EOF */
	if (count > EFUSE_BYTES - pos)
		count = EFUSE_BYTES - pos;
	if (count > EFUSE_BYTES)
		return -1;

	//Wr( EFUSE_CNTL1, Rd(EFUSE_CNTL1) |  (1 << 12) );
    
    for (pc = buf; count--; ++pos, ++pc)
		__efuse_write_byte(pos, *pc);
		
	*ppos = pos;
	
	   // Disable the Write mode
    //Wr( EFUSE_CNTL1, Rd(EFUSE_CNTL1) & ~(1 << 12) );

	return count;
}

static int efuse_chk_written(efuseinfo_item_t* info)
{
	int ret = 0;
	char buf[EFUSE_BYTES];

	loff_t ppos = info->offset;
	size_t count = info->enc_len;	
	
	efuse_init();
	memset(buf,0,sizeof(buf));	
	if(efuse_read(buf, count, &ppos)<=0)
		return -1;
		
	char ckbit=0x0;
	int i;
	for(i=0;i<count;i++){
		if(buf[i]|ckbit){
			ret = -1;
			break ;
		}
	}
	return ret;	
}

char *efuse_read_usr(efuseinfo_item_t* info)
{
	loff_t ppos = info->offset;
	unsigned enc_len = info->enc_len;	
	
	char enc_buf[EFUSE_BYTES];
	char *pdata = efuse_buf;
	char *penc = enc_buf;
	
	efuse_init();
	memset(efuse_buf, 0, sizeof(efuse_buf));
	
	if(info->bch_en != 0){
		memset(enc_buf, 0, sizeof(enc_buf));
		efuse_read(enc_buf, enc_len, &ppos);
		while(enc_len >= 31){
			efuse_bch_dec(penc, 31, pdata);
			penc += 31;
			pdata += 30;
			enc_len -= 31;
		}
		if((enc_len > 0))
			efuse_bch_dec(penc, enc_len, pdata);
	}	
	else
		efuse_read(efuse_buf, enc_len, &ppos);	
		
	return (char*)efuse_buf;	
}

int efuse_write_usr(efuseinfo_item_t* info, char *data)
{	
	if(efuse_chk_written(info))
		return -1;
	
	loff_t ppos = info->offset;
	unsigned enc_len = info->enc_len;
	unsigned data_len = info->data_len;
	char *pdata = data;
	char *penc = efuse_buf;	
	memset(efuse_buf, 0, sizeof(efuse_buf));
		
	if(info->bch_en != 0){
		while(data_len >= 30){
			efuse_bch_enc(pdata, 30, penc);
			data_len -= 30;
			pdata += 30;
			penc += 31;		
		}
		if(data_len > 0)
			efuse_bch_enc(pdata, data_len, penc);
	}	
	else
		memcpy(penc, pdata, enc_len);
	
	efuse_write(efuse_buf, enc_len, &ppos);
	
	return 0 ;
}

static int cpu_is_before_m6(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c0, c0, 5	@ get MPIDR" : "=r" (val) : : "cc");
	
	return ((val & 0x40000000) == 0x40000000);
}

static int efuse_is_all_free(void)
{
	char *op = efuse_dump();
	int i;
	for(i=0; i<EFUSE_BYTES; i++)
		if(op[i] != 0)
			return 0;
	
	return 1;	
}

static int efuse_readversion(void)
{
	if(efuse_active_version != -1)
		return efuse_active_version;
	
	loff_t ppos;
	char ver_buf[4];
	memset(ver_buf, 0, sizeof(ver_buf));
	
	if(cpu_is_before_m6()){    // M1, M2, M3, A3
		ppos = 380;
		efuse_read(efuse_buf, 4, &ppos);
		efuse_bch_dec(efuse_buf, 4, ver_buf);		
		if(ver_buf[0] != 0){
			efuse_active_version = ver_buf[0];
			return ver_buf[0];
		}
		else{   // distinguish free efuse layout and M1/M2 old version
			if(efuse_is_all_free())
				return -1;
			else
				return 0;
		}
	}
	else{
		ppos = 3;
		efuse_read(ver_buf, 1, &ppos);
		if(ver_buf[0] != 0){
			efuse_active_version = ver_buf[0];
			return ver_buf[0];
		}
		else
			return -1;	
	}		
}

int efuse_getinfo(char *title, efuseinfo_item_t *info)
{
		int ver = efuse_readversion();
		if(ver < 0){
			printf("efuse version is not selected.\n");
			return -1;
		}
		int i;
		efuseinfo_t *vx = NULL;
		for(i=0; i<efuseinfo_num; i++){
			if(efuseinfo[i].version == ver){
				vx = &(efuseinfo[i]);
				break;
			}				
		}
		if(!vx){
			printf("efuse version %d is not supported.\n", ver);
			return -1;
		}	
		
		efuseinfo_item_t *item = vx->efuseinfo_version;
		int size = vx->size;
		int ret = -1;		
		for(i=0; i<size; i++, item++){
			if(strcmp(item->title, title) == 0){
				strcpy(info->title, item->title);				
				info->offset = item->offset;
				info->enc_len = item->enc_len;
				info->data_len = item->data_len;
				info->we = item->we;
				info->bch_en = item->bch_en;				
				ret = 0;
				break;
			}
		}
		
		if((ret < 0) && (efuse_getinfoex != NULL))
			ret = efuse_getinfoex(title, info);		
		if(ret < 0)
			printf("%s is not found.\n", title);
			
		return ret;
}

unsigned efuse_readcustomerid(void)
{
	if(efuse_active_customerid != 0)
		return efuse_active_customerid;
	
	loff_t ppos;
	char buf[4];
	memset(buf, 0, sizeof(buf));
	
	if(cpu_is_before_m6()){
		ppos = 380;
		efuse_read(efuse_buf, 4, &ppos);
		efuse_bch_dec(efuse_buf, 4, buf);
		if((buf[1] != 0) || (buf[2] != 0))
			efuse_active_customerid = (buf[2]<<8) + buf[1];		
	}
	else{
		ppos = 4;
		efuse_read(buf, 4, &ppos);		
		int i;
		unsigned val = 0;
		for(i=3; i>=0;i--)
			val = ((val<<8) + buf[i]);
		if(val != 0)
			efuse_active_customerid = val;			
	}
	
	return efuse_active_customerid;
}

void efuse_getinfo_version(efuseinfo_item_t *info)
{
	strcpy(info->title, "version");
	info->we = 1;
		
	if(cpu_is_before_m6()){
		info->offset = 380;
		info->enc_len = 4;
		info->data_len = 3;
		info->bch_en = 1;	
	}
	else{
		info->offset = 3;
		info->enc_len = 5;
		info->data_len = 5;
		info->bch_en = 0;
	}		
}

char* efuse_dump(void)
{
	int i=0;
    //unsigned pos;
    memset(efuse_buf, 0, sizeof(efuse_buf));

#ifdef CONFIG_AML_MESION_3		
	// Enabel auto-read mode
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif            

	for(i=0; i<EFUSE_BYTES; i+=4)
		__efuse_read_dword(i,  (unsigned long*)(&(efuse_buf[i])));	    
		
#ifdef CONFIG_AML_MESION_3		
    // Disable auto-read mode    
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif
     
     return (char*)efuse_buf;
}

