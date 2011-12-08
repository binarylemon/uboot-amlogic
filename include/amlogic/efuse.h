#ifndef __EFUSE_H
#define __EFUSE_H

typedef struct efuseinfo{
	char title[40];
	int nID;
	unsigned offset;    // write offset
	unsigned enc_len;
	unsigned data_len;
	int we;    // write enable 	 
} efuseinfo_t;

unsigned char *efuse_read_usr(int usr_type);
int efuse_write_usr(int usr_type, unsigned char *data);

#define EFUSE_BYTES            384  //(EFUSE_BITS/8)
#define EFUSE_DWORDS            96  //(EFUSE_BITS/32)

#endif

