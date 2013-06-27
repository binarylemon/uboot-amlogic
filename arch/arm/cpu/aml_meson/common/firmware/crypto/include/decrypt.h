#ifndef _DECRYPT_H_
#define _DECRYPT_H_

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

// OV Decrypted Header 124*3
typedef struct 
{
	uint8_t pad[372-256/8-14*16-32*2]; //20 bytes
	//uint32_t uAmlogicID;  // 4B
	uint16_t aesmask[32];	//64 bytes
	uint8_t aeskey[14*16];	//224bytes
	uint8_t shasum[256/8];	//32 bytes
}BL2StorageKey; // 124*3=372B
#define AMLOGIC_CRYPTO_ID   0x434C4d41   //AMLC

// OV Encryped Header 128*3
typedef struct
{
	uint8_t m[128*3];
}EncryptStorage;

#define ERROR_MOD(mod,num) ((short)(((mod<<6)|num)))
/* Data Check Return */                     // 
#define ERROR_MAGIC_CHECK_SUM       ERROR_MOD(1,0)   //
#define ERROR_MAGIC_WORD_ERROR      ERROR_MOD(1,1)      //
#define ERROR_READ_KEY				ERROR_MOD(1,2)
#define ERROR_ENCRYPT_HASH			ERROR_MOD(1,3)
#define ERROR_RAW_HASH				ERROR_MOD(1,4)
#define ERROR_AES_FORMAT			ERROR_MOD(1,5)
#define ERROR_FORMAT				ERROR_MOD(1,6)
#define ERROR_ROMBOOT_ENTRY	ERROR_MOD(1, 7)



#endif