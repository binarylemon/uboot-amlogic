#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/efuse.h>
#include "efuse_bch_8.h"
#include "efuse_regs.h"


char efuse_buf[EFUSE_BYTES] = {0};

extern int efuseinfo_num;
extern efuseinfo_t efuseinfo[];
extern int efuse_active_version;
extern int efuse_active_customerid;
extern pfn efuse_getinfoex;
extern pfn_byPos efuse_getinfoex_byPos;
extern int printf(const char *fmt, ...);
extern void __efuse_write_byte( unsigned long addr, unsigned long data );
extern void __efuse_read_dword( unsigned long addr, unsigned long *data);
extern void efuse_init(void);

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

 	// Enabel auto-read mode    
    WRITE_EFUSE_REG_BITS( P_EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
             CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
 
    pos = (pos/4)*4;
    for (pdw = contents; dwsize-- > 0 && pos < EFUSE_BYTES; pos += 4, ++pdw)
		__efuse_read_dword(pos, pdw);	    

     // Disable auto-read mode    
    WRITE_EFUSE_REG_BITS( P_EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
             CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
            
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

static int cpu_is_before_m6(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c0, c0, 5	@ get MPIDR" : "=r" (val) : : "cc");
	
	return ((val & 0x40000000) == 0x40000000);
}

static int efuse_readversion(void)
{
//	loff_t ppos;
	char ver_buf[4], buf[4];
	efuseinfo_item_t info;
	
	if(efuse_active_version != -1)
		return efuse_active_version;
		
	efuse_init();
	efuse_set_versioninfo(&info);
	memset(ver_buf, 0, sizeof(ver_buf));		
	memset(buf, 0, sizeof(buf));
	
	efuse_read(buf, info.enc_len, (loff_t *)(&info.offset));
	if(info.bch_en)
		efuse_bch_dec(buf, info.enc_len, ver_buf, info.bch_reverse);
	else
		memcpy(ver_buf, buf, sizeof(buf));

#ifdef CONFIG_M6   //version=2
	if(ver_buf[0] == 2){
		efuse_active_version = ver_buf[0];
		return ver_buf[0];
	}
	else
		return -1;

#elif defined(CONFIG_M3)   //version=1
	if(ver_buf[0] == 1){
		efuse_active_version = ver_buf[0];
		return ver_buf[0];
	}
	else
		return -1;
		
#elif defined(CONFIG_M1)   //version=0
	if(ver_buf[0] == 0){
		efuse_active_version = ver_buf[0];
		reurn ver_buf[0];
	}
	else
		reurn -1;
#else
	return -1;
#endif	
	
}

static int efuse_getinfo_byPOS(unsigned pos, efuseinfo_item_t *info)
{
	int ver;
	int i;
	efuseinfo_t *vx = NULL;
	efuseinfo_item_t *item = NULL;
	int size;
	int ret = -1;		
	
	unsigned versionPOS;
	if(cpu_is_before_m6())
		versionPOS = EFUSE_VERSION_OFFSET; //380;
	else
		versionPOS = V2_EFUSE_VERSION_OFFSET; //3;
	if(pos == versionPOS){
		efuse_set_versioninfo(info);
		return 0;
	}
	
	ver = efuse_readversion();
		if(ver < 0){
			printf("efuse version is not selected.\n");
			return -1;
		}
		
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
		
		// BSP setting priority is higher than version table
		if((efuse_getinfoex_byPos != NULL)){
			ret = efuse_getinfoex_byPos(pos, info);			
			if(ret >=0 )
				return ret;
		}
		
		item = vx->efuseinfo_version;
		size = vx->size;		
		ret = -1;		
		for(i=0; i<size; i++, item++){			
			if(pos == item->offset){
				strcpy(info->title, item->title);				
				info->offset = item->offset;				
				info->data_len = item->data_len;			
				info->enc_len = item->enc_len;
				info->bch_en = item->bch_en;
				info->bch_reverse = item->bch_reverse;			
				info->we=item->we;		
				ret = 0;
				break;
			}
		}
		
		//if((ret < 0) && (efuse_getinfoex != NULL))
		//	ret = efuse_getinfoex(id, info);		
		if(ret < 0)
			printf("POS:%d is not found.\n", pos);
			
		return ret;
}


int efuse_chk_written(loff_t pos, size_t count)
{
	loff_t local_pos = pos;	
	int i;
	//unsigned char* buf = NULL;
	char buf[EFUSE_BYTES];
	efuseinfo_item_t info;
	unsigned enc_len ;		
	
	if(efuse_getinfo_byPOS(pos, &info) < 0){
		printf("not found the position:%d.\n", (int)pos);
		return -1;
	}
	 if(count>info.data_len){
		printf("data length: %d is out of EFUSE layout!\n", count);
		return -1;
	}
	if(count == 0){
		printf("data length: 0 is error!\n");
		return -1;
	}
	
	efuse_init();
	enc_len = info.enc_len;				
	if (efuse_read(buf, enc_len, &local_pos) == enc_len) {
		for (i = 0; i < enc_len; i++) {
			if (buf[i]) {
				printf("pos %d value is %d", (size_t)(pos + i), buf[i]);
				return 1;
			}
		}
	}	
	return 0;
}



int efuse_read_usr(char *buf, size_t count, loff_t *ppos)
{	
	char data[EFUSE_BYTES];
//	int ret;
	unsigned enc_len;			
	char *penc = NULL;
	char *pdata = NULL;		
	int reverse = 0;
	unsigned pos = (unsigned)*ppos;
	efuseinfo_item_t info;	
		
	if(efuse_getinfo_byPOS(pos, &info) < 0){
		printf("not found the position:%d.\n", pos);
		return -1;
	}			
	if(count>info.data_len){
		printf("data length: %d is out of EFUSE layout!\n", count);
		return -1;
	}
	if(count == 0){
		printf("data length: 0 is error!\n");
		return -1;
	}
	
	efuse_init();
	enc_len = info.enc_len;
	reverse=info.bch_reverse;			
	memset(efuse_buf, 0, EFUSE_BYTES);	
	memset(data, 0, EFUSE_BYTES);
		
	penc = efuse_buf;
	pdata = data;			
	if(info.bch_en){						
		efuse_read(efuse_buf, enc_len, ppos);		
		while(enc_len >= 31){
			efuse_bch_dec(penc, 31, pdata, reverse);
			penc += 31;
			pdata += 30;
			enc_len -= 31;
		}
		if((enc_len > 0))
			efuse_bch_dec(penc, enc_len, pdata, reverse);
	}	
	else
		efuse_read(pdata, enc_len, ppos);	
		
	memcpy(buf, data, count);		

	return count;	
}

int efuse_write_usr(char* buf, size_t count, loff_t* ppos)
{
	char data[EFUSE_BYTES];
//	int ret;		
	char *pdata = NULL;
	char *penc = NULL;			
	unsigned enc_len,data_len, reverse;
	unsigned pos = (unsigned)*ppos;	
	efuseinfo_item_t info;
	
	if(efuse_getinfo_byPOS(pos, &info) < 0){
		printf("not found the position:%d.\n", pos);
		return -1;
	}
	if(count>info.data_len){
		printf("data length: %d is out of EFUSE layout!\n", count);
		return -1;
	}
	if(count == 0){
		printf("data length: 0 is error!\n");
		return -1;
	}	
	
	if(efuse_chk_written(pos, info.data_len)){
		printf("error: efuse has written.\n");
		return -1;
	}
		
	memset(data, 0, EFUSE_BYTES);
	memset(efuse_buf, 0, EFUSE_BYTES);
	
	efuse_init();	
	memcpy(data, buf, count)	;	
	pdata = data;
	penc = efuse_buf;			
	enc_len=info.enc_len;
	data_len=info.data_len;
	reverse = info.bch_reverse;	
	
	if(info.bch_en){				
		while(data_len >= 30){
			efuse_bch_enc(pdata, 30, penc, reverse);
			data_len -= 30;
			pdata += 30;
			penc += 31;		
		}
		if(data_len > 0)
			efuse_bch_enc(pdata, data_len, penc, reverse);
	}	
	else
		memcpy(penc, pdata, enc_len);
		
	efuse_write(efuse_buf, enc_len, ppos);
	
	return enc_len;	
}

void efuse_set_versioninfo(efuseinfo_item_t *info)
{
	strcpy(info->title, "version");			
	if(cpu_is_before_m6()){
			info->offset = EFUSE_VERSION_OFFSET; //380;		
			info->data_len = EFUSE_VERSION_DATA_LEN; //3;	
			info->enc_len = EFUSE_VERSION_ENC_LEN; //4;
			info->bch_en = EFUSE_VERSION_BCH_EN; //1;		
			info->we = 1;									//add 
			info->bch_reverse = EFUSE_VERSION_BCH_REVERSE;
	}
		else{
			info->offset = V2_EFUSE_VERSION_OFFSET; //3;		
			info->data_len = V2_EFUSE_VERSION_DATA_LEN; //1;		
			info->enc_len = V2_EFUSE_VERSION_ENC_LEN; //1;
			info->bch_en = V2_EFUSE_VERSION_BCH_EN; //0;
			info->we = 1;									//add 
			info->bch_reverse = V2_EFUSE_VERSION_BCH_REVERSE;
		}
}


int efuse_getinfo(char *title, efuseinfo_item_t *info)
{
	int ver;
	int i;
	efuseinfo_t *vx = NULL;
	efuseinfo_item_t *item = NULL;
	int size;
	int ret = -1;		
	
	if(strcmp(title, "version") == 0){
		efuse_set_versioninfo(info);		
		return 0;	
	}
	
		ver = efuse_readversion();
		if(ver < 0){
			printf("efuse version is not selected.\n");
			return -1;
		}		
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
		
		// BSP setting priority is higher than versiontable
		if(efuse_getinfoex != NULL){
			ret = efuse_getinfoex(title, info);		
			if(ret >= 0)
				return ret;
		}
		
		item = vx->efuseinfo_version;
		size = vx->size;
		ret = -1;		
		for(i=0; i<size; i++, item++){
			if(strcmp(item->title, title) == 0){
				strcpy(info->title, item->title);				
				info->offset = item->offset;
				info->enc_len = item->enc_len;
				info->data_len = item->data_len;
				info->we = item->we;
				info->bch_en = item->bch_en;				
				info->bch_reverse = item->bch_reverse;	
				ret = 0;
				break;
			}
		}
		
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
	efuse_init();	
	
	if(cpu_is_before_m6()){
		ppos = 380;
		efuse_read(efuse_buf, 4, &ppos);
		efuse_bch_dec(efuse_buf, 4, buf, 0);
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

/*void efuse_getinfo_version(efuseinfo_item_t *info)
{
	strcpy(info->title, "version");
	info->we = 1;
	info->bch_reverse = 0;
		
	if(cpu_is_before_m6()){
		info->offset = 380;
		info->enc_len = 4;
		info->data_len = 3;
		info->bch_en = 1;	
	}
	else{
		info->offset = 3;
		info->enc_len = 1;
		info->data_len = 1;
		info->bch_en = 0;
	}		
}*/

char* efuse_dump(void)
{
	int i=0;
    //unsigned pos;
    memset(efuse_buf, 0, sizeof(efuse_buf));
	efuse_init();

 	// Enabel auto-read mode
    WRITE_EFUSE_REG_BITS( P_EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_ON,
             CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );

	for(i=0; i<EFUSE_BYTES; i+=4)
		__efuse_read_dword(i,  (unsigned long*)(&(efuse_buf[i])));	    
		
     // Disable auto-read mode    
    WRITE_EFUSE_REG_BITS( P_EFUSE_CNTL1, CNTL1_AUTO_RD_ENABLE_OFF,
             CNTL1_AUTO_RD_ENABLE_BIT, CNTL1_AUTO_RD_ENABLE_SIZE );
     
     return (char*)efuse_buf;
}

