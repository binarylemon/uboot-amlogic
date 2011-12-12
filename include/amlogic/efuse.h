#ifndef __EFUSE_H
#define __EFUSE_H

#include <config.h>

typedef struct efuseinfo{
	char title[40];
	int nID;
	unsigned offset;    // write offset
	unsigned enc_len;
	unsigned data_len;
	int we;    // write enable 	 
} efuseinfo_t;

char *efuse_read_usr(int usr_type);
int efuse_write_usr(int usr_type, char *data);
int efuse_chk_written(int usr_type);

#define EFUSE_BYTES            384  //(EFUSE_BITS/8)
#define EFUSE_DWORDS            96  //(EFUSE_BITS/32)

#define USR_LICENCE		0
#define USR_MACADDR		1
#define USR_HDMIHDCP		2
#define USR_USERIDF		3
#ifdef CONFIG_REFB09_NEW
#define USR_USERIDF_REFB09_NEW USR_USERIDF+1
#else	
#define USR_USERIDF_REFB09_NEW USR_USERIDF
#endif
// next item definition need use the following format because the ID is used as index
//#ifdef xxx
//#define USR_XXX USR_USERIDF_REFB09+1
//#else
//#define USR_XXX USR_USERIDF_REFB09
//#endif

#endif

