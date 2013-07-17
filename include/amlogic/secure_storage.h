#ifndef __SECURE_STORAGE__H__
#define __SECURE_STORAGE__H__


#ifdef CONFIG_SECURE_NAND
extern int secure_storage_nand_read(char *buf,unsigned int len);
extern int secure_storage_nand_write(char *buf,unsigned int len);
#else
inline int secure_storage_nand_read(char *buf,unsigned int len)
{
	return -1;
}
inline int secure_storage_nand_write(char *buf,unsigned int len)
{
	return -1;
}
#endif

#ifdef CONFIG_SPI_NOR_SECURE_STORAGE
extern int secure_storage_spi_write(u8 *buf,u32 len);
extern int secure_storage_spi_read(u8 *buf,u32 len);
#else
inline int secure_storage_spi_write(u8 *buf,u32 len)
{
	return -1;
}
inline int secure_storage_spi_read(u8 *buf,u32 len)
{
	return -1;
}
#endif

#ifdef CONFIG_SECURESTORAGEKEY
#include "../../drivers/securestorage/sw_board_asm.h"

//#define SECUREOS_KEY_DEFAULT_ADDR	0xa00e0000
//#define SECUREOS_KEY_DEFAULT_SIZE	(128*1024)
#define SECUREOS_KEY_DEFAULT_ADDR	COMM_NS_CONTENT_ADDR
#define SECUREOS_KEY_DEFAULT_SIZE	COMM_NS_CONTENT_SIZE

extern int secure_storage_init();
extern int secure_storage_read_a_key(unsigned char* name, unsigned int namelen, unsigned char* out, unsigned int outlen, unsigned int*datalen);
extern int secure_storage_write_a_key(unsigned char* name, unsigned int namelen, unsigned char* data, unsigned int datalen);
#else

#define SECUREOS_KEY_DEFAULT_ADDR	0xa00e0000
#define SECUREOS_KEY_DEFAULT_SIZE	(128*1024)

inline int secure_storage_init(void)
{
	return -1;
}
inline int secure_storage_read_a_key(unsigned char* name, unsigned int namelen, unsigned char* out, unsigned int outlen, unsigned int*datalen)
{
	return -1;
}
inline int secure_storage_write_a_key(unsigned char* name, unsigned int namelen, unsigned char* data, unsigned int datalen)
{
	return -1;
}
#endif

#if defined(CONFIG_CMD_SECURESTORE) && defined(CONFIG_CMD_RANDOM) && defined(CONFIG_EFUSE)
extern int securestore_key_init( char *seed,int len);
extern int securestore_key_read(char *keyname,char *keybuf,unsigned int keylen);
extern int securestore_key_write(char *keyname, char *keybuf,unsigned int keylen);
extern int securestore_key_uninit();
#else
int securestore_key_init( char *seed,int len)
{
	return -1;
}
int securestore_key_read(char *keyname,char *keybuf,unsigned int keylen)
{
	return -1;
}
int securestore_key_write(char *keyname, char *keybuf,unsigned int keylen)
{
	return -1;
}
int securestore_key_uninit()
{
	return -1;
}
#endif


#endif
