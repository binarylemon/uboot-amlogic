#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/efuse.h>
#include <efuse_bch_8.h>
#include "efuse_regs.h"


/* efuse layout
http://wiki-sh.amlogic.com/index.php/How_To_burn_the_info_into_E-Fuse
0~3			licence				1 check byte			4 bytes(in total)
4~10			mac				1 check byte			7 bytes(in total)
12~322			hdcp				10 check byte			310 bytes(in total)
322~384  		usid				2 check byte		 	62 bytes(in total)
*/

#define EFUSE_LICENCE_OFFSET 				0
#define EFUSE_LICENCE_DATA_BYTES 		3
#define EFUSE_LICENCE_ENC_BYTES		4

#define EFUSE_MACADDR_OFFSET			4
#define EFUSE_MACADDR_DATA_BYTES	6
#define EFUSE_MACADDR_ENC_BYTES 	7

#define EFUSE_HDMHDCP_OFFSET		12
#define EFUSE_HDMHDCP_DATA_BYTES 300
#define EFUSE_HDMHDCP_ENC_BYTES 310

#define EFUSE_USERIDF_OFFSET 322
#define EFUSE_USERIDF_DATA_BYTES 60
#define EFUSE_USERIDF_ENC_BYTES 62

#ifdef CONFIG_REFB09_NEW
//for m1 refb09 new
#define EFUSE_USERIDF_REFB09_NEW_OFFSET 324
#define EFUSE_USERIDF_REFB09_NEW_DATA_BYTES 20
#define EFUSE_USERIDF_REFB09_NEW_ENC_BYTES 21
#endif

char efuse_buf[EFUSE_BYTES] = {0};
efuseinfo_t efuse_info[] = 
{
	{
		.title = "licence",
		.nID = USR_LICENCE,
		.offset = EFUSE_LICENCE_OFFSET,
		.enc_len = EFUSE_LICENCE_ENC_BYTES,
		.data_len = EFUSE_LICENCE_DATA_BYTES,
		.we = 0,
	},
	{
		.title = "mac",
		.nID = USR_MACADDR,
		.offset = EFUSE_MACADDR_OFFSET,
		.enc_len = EFUSE_MACADDR_ENC_BYTES,
		.data_len = EFUSE_MACADDR_DATA_BYTES,
		.we = 1,
	},
	{
		.title = "hdcp",
		.nID = USR_HDMIHDCP,
		.offset = EFUSE_HDMHDCP_OFFSET,
		.enc_len = EFUSE_HDMHDCP_ENC_BYTES,
		.data_len = EFUSE_HDMHDCP_DATA_BYTES,
		.we = 0,
	},
	{
		.title = "usid",
		.nID = USR_USERIDF,
		.offset = EFUSE_USERIDF_OFFSET,
		.enc_len = EFUSE_USERIDF_ENC_BYTES,
		.data_len = EFUSE_USERIDF_DATA_BYTES,
		.we = 0,
	},	
#ifdef CONFIG_REFB09_NEW	
	// for refb09 new
	{
		.title = "USID",
		.nID = USR_USERIDF_REFB09_NEW,
		.offset = EFUSE_USERIDF_REFB09_NEW_OFFSET,
		.enc_len = EFUSE_USERIDF_REFB09_NEW_ENC_BYTES,
		.data_len = EFUSE_USERIDF_REFB09_NEW_DATA_BYTES,
		.we = 0,		
	},
#endif	
};
unsigned efuse_info_cnt = sizeof(efuse_info)/sizeof(efuseinfo_t);

static void __efuse_write_byte( unsigned long addr, unsigned long data );
static void __efuse_read_dword( unsigned long addr, unsigned long *data);

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
    unsigned residunt = pos - (pos/4)*4;
    unsigned int dwsize = (count+residunt+3)/4;
    
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

int efuse_chk_written(int usr_type)
{
	int ret = 0;
	char buf[EFUSE_BYTES];

	loff_t ppos = efuse_info[usr_type].offset;
	size_t count = efuse_info[usr_type].enc_len;	
	
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

char *efuse_read_usr(int usr_type)
{
	char enc_buf[EFUSE_BYTES];
	loff_t ppos = efuse_info[usr_type].offset;
	unsigned enc_len = efuse_info[usr_type].enc_len;
	//unsigned data_len = efuse_info[usr_type].data_len;
	char *pdata = efuse_buf;
	char *penc = enc_buf;
	
	efuse_init();
	memset(efuse_buf, 0, sizeof(efuse_buf));
	memset(enc_buf, 0, sizeof(enc_buf));
	efuse_read(enc_buf, enc_len, &ppos);
	
	while(enc_len >= 31){
		efuse_bch_dec(penc, 31, pdata);
		penc += 31;
		pdata += 30;
		enc_len -= 31;
	}
	if(enc_len > 0)
		efuse_bch_dec(penc, enc_len, pdata);
		
	return (char*)efuse_buf;	
}

int efuse_write_usr(int usr_type, char *data)
{
	//int ret = 0;		
	if(efuse_chk_written(usr_type))
		return -1;
	
	loff_t ppos = efuse_info[usr_type].offset;
	unsigned enc_len = efuse_info[usr_type].enc_len;
	unsigned data_len = efuse_info[usr_type].data_len;
	char *pdata = data;
	char *penc = efuse_buf;	
	memset(efuse_buf, 0, sizeof(efuse_buf));
		
	while(data_len >= 30){
		efuse_bch_enc(pdata, 30, penc);
		data_len -= 30;
		pdata += 30;
		penc += 31;		
	}
	if(data_len > 0)
		efuse_bch_enc(pdata, data_len, penc);

	efuse_write(efuse_buf, enc_len, &ppos);
	
	return 0 ;
}


#ifdef CONFIG_EFUSE_DUMP
char* efuse_dump()
{
	  int i=0;
    unsigned pos;
    memset(efuse_buf, 0, sizeof(efuse_buf));

#ifdef CONFIG_AML_MESION_3		
	// Enabel auto-read mode
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif            

	for(i=0; i<EFUSE_BYTES; i+=4)
		__efuse_read_dword(i,  &(efuse_buf[i]));	    
		
#ifdef CONFIG_AML_MESION_3		
    // Disable auto-read mode    
    WRITE_CBUS_REG_BITS( EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
            CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
#endif
     
     return (char*)efuse_buf;
}
#endif