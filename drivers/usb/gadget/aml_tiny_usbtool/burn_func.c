#include <common.h>
#include "usb_pcd.h"


#if defined(WRITE_TO_NAND_ENABLE) || defined(WRITE_TO_EMMC_ENABLE)
#include <common.h>
#include <linux/ctype.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/nand.h>
#include <linux/types.h>
#include <div64.h>
#include <linux/err.h>
char namebuf[20];
char databuf[4096];
//char listkey[1024];
int secukey_inited=0;
//extern ssize_t uboot_key_init();
extern int uboot_key_initial(char *device);
extern ssize_t uboot_get_keylist(char *keyname);
extern ssize_t uboot_key_read(char *keyname, char *keydata);
extern ssize_t uboot_key_write(char *keyname, char *keydata);
//extern int nandkey_provider_register();
//extern int key_set_version(char *device);
int ensure_secukey_init(void);
int cmd_secukey(int argc, char * const argv[], char *buf);
#endif

#define SECUKEY_BYTES     512
//test efuse read
#define EFUSE_READ_TEST_ENABLE

//hdcp verify enable
#define WRITE_HDCP_VERIFY_ENABLE

//efuse version
#define EFUSE_VERSION_MESON3        "01:02:03"
#define EFUSE_VERSION_MESON6        "02"

//extern 
#ifdef WRITE_TO_EFUSE_ENABLE
#ifdef CONFIG_AML_MESON3
extern int do_efuse_usb(int argc, char * const argv[], char *buf);
#elif defined(CONFIG_AML_MESON6)
extern int cmd_efuse(int argc, char * const argv[], char *buf);
#endif
#endif

/* hdcp key verify code */
#define DWORD unsigned int  //4 bytes
#define BYTE unsigned char   //1 byte
#define SHA1_MAC_LEN 20

typedef struct {
        DWORD state[5];
        DWORD count[2];
        BYTE buffer[64];
} SHA1_CTX;

void SHA1Reset(SHA1_CTX *context);
void SHA1Input(SHA1_CTX *context, BYTE *data, DWORD len);
void SHA1Result(SHA1_CTX *context, BYTE *digest);//20
void SHA1Transform_H(DWORD *state, BYTE *buffer); //5  64

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | \
        (rol(block->l[i], 8) & 0x00FF00FF))
#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
        block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) \
        z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30);
#define R1(v,w,x,y,z,i) \
        z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30);
#define R2(v,w,x,y,z,i) \
        z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) \
        z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
        w = rol(w, 30);
#define R4(v,w,x,y,z,i) \
        z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
        w=rol(w, 30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1Transform_H(DWORD *state, BYTE *buffer)
{
        DWORD a, b, c, d, e;
        typedef union {
                BYTE c[64];
                DWORD l[16];
        } CHAR64LONG16;
        CHAR64LONG16 *block;

        DWORD workspace[16];
        block = (CHAR64LONG16 *)workspace;
        memcpy(block, buffer, 64);

        /* Copy context->state[] to working vars */
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        /* 4 rounds of 20 operations each. Loop unrolled. */
        R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);udelay(100);
        R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);udelay(100);
        R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);udelay(100);
        R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);udelay(100);
        R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);udelay(100);
        R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);udelay(100);
        R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);udelay(100);
        R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);udelay(100);
        R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);udelay(100);
        R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);udelay(100);
        R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);udelay(100);
        R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);udelay(100);
        R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);udelay(100);
        R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);udelay(100);
        R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);udelay(100);
        R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);udelay(100);
        R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);udelay(100);
        R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);udelay(100);
        R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);udelay(100);
        R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);udelay(100);
        /* Add the working vars back into context.state[] */
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        /* Wipe variables */
        a = b = c = d = e = 0;

        memset(block, 0, 64);
}


/* SHA1Reset - Initialize new context */

void SHA1Reset(SHA1_CTX *context)
{
        /* SHA1 initialization constants */
        context->state[0] = 0x67452301;
        context->state[1] = 0xEFCDAB89;
        context->state[2] = 0x98BADCFE;
        context->state[3] = 0x10325476;
        context->state[4] = 0xC3D2E1F0;
        context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Input(SHA1_CTX* context, BYTE *_data, DWORD len)
{
        DWORD i, j;
        BYTE *data = _data;

        j = (context->count[0] >> 3) & 63;
        if ((context->count[0] += len << 3) < (len << 3))
                context->count[1]++;
        context->count[1] += (len >> 29);
        if ((j + len) > 63) {
                memcpy(&context->buffer[j], data, (i = 64-j));
                SHA1Transform_H(context->state, context->buffer);
                for ( ; i + 63 < len; i += 64) {
                        SHA1Transform_H(context->state, &data[i]);
                }
                j = 0;
        }
        else i = 0;
        memcpy(&context->buffer[j], &data[i], len - i);

}


/* Add padding and return the message digest. */

void SHA1Result(SHA1_CTX *context, BYTE *digest)
{
        DWORD i;
        BYTE finalcount[8];

        for (i = 0; i < 8; i++) {
                finalcount[i] = (BYTE)
                        ((context->count[(i >= 4 ? 0 : 1)] >>
                          ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
        }
        SHA1Input(context, (BYTE *) "\200", 1);
        while ((context->count[0] & 504) != 448) {
                SHA1Input(context, (BYTE *) "\0", 1);
        }
        SHA1Input(context, finalcount, 8);  /* Should cause a SHA1Transform_H()
                                              */
        for (i = 0; i < 20; i++) {
                digest[i] = (BYTE)
                        ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
                         255);
        }
        /* Wipe variables */
        i = 0;
        memset(context->buffer, 0, 64);
        memset(context->state, 0, 20);
        memset(context->count, 0, 8);
        memset(finalcount, 0, 8);
}
 /**************************************************************************
 * NOTES:       Test Vectors (from FIPS PUB 180-1) to verify implementation
 *              1- Input : "abc"
 *              Output : A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 *              2- Input : "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *              Output : 84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 *              2- Input : A million repetitions of 'a' - not applied (memory shortage)
 *              Output : 34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 *              More test vectors can be obtained from FIPS web site
 ***************************************************************************/
void SHA1_Perform(BYTE *indata, DWORD inlen, BYTE *outdata) //calculate SHA-1 API
{
    SHA1_CTX sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, indata, inlen);	
    SHA1Result(&sha, outdata);	
} 

typedef struct 
{
    unsigned char ksv[5];
    unsigned char rsv[3];
    unsigned char dpk[280];
    unsigned char sha[20];
}hdcp_llc_file;

#if defined(WRITE_TO_EFUSE_ENABLE)
static int run_efuse_cmd(int argc, char *argv[], char *buff)
{
   int ret = -1;
   
#ifdef CONFIG_AML_MESON3
   ret = do_efuse_usb(argc, argv, buff);
#elif defined(CONFIG_AML_MESON6)
   ret = cmd_efuse(argc, argv, buff);
#endif

   return ret;
}
#endif    /* WRITE_TO_EFUSE_ENABLE */

#if defined(EFUSE_READ_TEST_ENABLE)
static int test_efuse_read(int argc, char *argv[], char *cmpBuff)
{
   int i = 0, j = 0, ret = -1;
   int hdcp_flag = 0, hdcp_key_len = 288;
   char *hdcp = NULL;
   char efuse_data[SECUKEY_BYTES], reBuff[SECUKEY_BYTES], tmpBuf[SECUKEY_BYTES];

   printf("-----Test efuse read commond:\n");
   for(i=0; i<argc; i++) 
      printf("argv[%d]=%s\n", i, argv[i]);

   if(!strncmp(argv[0], "efuse", 5) && !strncmp(argv[1], "read", 4) && 
      (!strncmp(argv[2], "version", 7) ||!strncmp(argv[2], "mac_wifi", 8) ||
      !strncmp(argv[2], "mac_bt", 6) ||!strncmp(argv[2], "mac", 3) ||
      !strncmp(argv[2], "usid", 4) ||!strncmp(argv[2], "hdcp", 4))) {
      goto run;
   }
   else {
      printf("test efuse read commond not mach\n");
      return -3;
   }

run:
   memset(efuse_data, 0, sizeof(efuse_data));
   memset(reBuff, 0, sizeof(reBuff));
   memset(tmpBuf, 0, sizeof(tmpBuf));

   ret = run_efuse_cmd(argc, argv, reBuff);
   if(!ret) {
      // test efuse read version
      if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "version", strlen("version"))) {
#ifdef CONFIG_AML_MESON3
         sprintf(efuse_data, "%02x:%02x:%02x", reBuff[0], reBuff[1], reBuff[2]);
#elif defined(CONFIG_AML_MESON6)
         sprintf(efuse_data, "%02x", reBuff[0]);
#endif
         if(!strncmp(efuse_data, cmpBuff, strlen(cmpBuff))) {
            printf("test efuse read version success,read version=%s\n", efuse_data);
            return 0;
         }
         else {
            printf("test efuse read version success,read version=%s, but not mach %s\n", efuse_data, cmpBuff);
            return -1;
         }
      }
      // test efuse read mac/mac_bt/mac_wifi
      else if(!strncmp(argv[1], "read", strlen("read")) && (!strncmp(argv[2], "mac", strlen("mac")) ||
         !strncmp(argv[2], "mac_bt", strlen("mac_bt")) ||!strncmp(argv[2], "mac_wifi", strlen("mac_wifi")))) {
         sprintf(efuse_data, "%02x:%02x:%02x:%02x:%02x:%02x", reBuff[0], reBuff[1], reBuff[2], reBuff[3], reBuff[4], reBuff[5]);
         if(!strncmp(efuse_data, cmpBuff, strlen(cmpBuff))) {
            printf("test efuse read %s success,read %s=%s\n", argv[2], argv[2], efuse_data);
            return 0;
         }
         else {
            printf("test efuse read %s success,read %s=%s, but not mach %s\n", argv[2], argv[2], efuse_data, cmpBuff);
            return -1;
         }
      }
      // test efuse read usid
      else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "usid", strlen("usid"))) {
         for(i=0; i<strlen(cmpBuff); i++) {
            sprintf(tmpBuf, "%c", reBuff[i]);
            sprintf(&efuse_data[j], "%s", tmpBuf);
            j += strlen(tmpBuf);
            memset(tmpBuf, 0, sizeof(tmpBuf));
         }
         if(!strncmp(efuse_data, cmpBuff, strlen(cmpBuff))) {
            printf("test efuse read usid success,read usid=%s\n", efuse_data);
            return 0;
         }
         else {
            printf("test efuse read usid success,read usid=%s, but not mach %s\n", efuse_data, cmpBuff);
            return -1;
         }
      }
      // test efuse read hdcp
      else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
         hdcp = cmpBuff;
         for(i=0; i<hdcp_key_len; i++) {
            if(reBuff[i] != *hdcp ++) {
               hdcp_flag = 1;
               break;
            }
         }
         if(!hdcp_flag) {
            printf("test efuse read hdcp success\n");
            printf("read hdcp=");
            for(i=0; i<hdcp_key_len; i++)
               printf("%02x:", reBuff[i]);
            printf("\n");
            return 0;
         }
         else {
            printf("test efuse read hdcp success,but not mach\n");
            return -1;
         }
      }
   }
   else {
      printf("test efuse read %s fail\n", argv[2]);
      return -2;
   }

   return ret;
}
#endif    /* EFUSE_READ_TEST_ENABLE */

int burn_board(const char *dev, void *mem_addr, u64 offset, u64 size)
{
	char	str[128];
	printf("burn_board!!!\n");
	printf("CMD: dev=%s, mem_addr=0x%x, offset=0xllx, size=0x%llx\n", dev, mem_addr, offset, size);
	if(!strncmp("nand", *dev, 4))
	{
		sprintf(str, "nand erase 0x%llx 0x%llx}", offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
		sprintf(str, "nand write 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else if(!strncmp("spi", *dev, 3))
	{
		run_command("sf probe 2", 0);
		sprintf(str, "sf erase 0x%llx 0x%llx}", offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
		sprintf(str, "sf write 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else if(!strncmp("emmc", *dev, 4))
	{
		sprintf(str, "mmc write 1 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else
	{
		printf("Invalid Argument!\n");
		return -1;
	}
	return 0;
}

static int usb_bootm(const void *addr)
{
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "bootm %x", addr);
	return run_command(cmd, 0);
}

u32 checkcum_32(const unsigned char *buf, u32 len)
{
	u32 fake_len, chksum = 0;
	u32 *ptr = buf;
	int i;
	printf("buf=0x%08x, len=0x%x\n", buf, len);
	if(len%4)
	{
		fake_len = len - len%4 + 4;
		memset((buf+len), 0, (fake_len-len));
	}
	else
	{
		fake_len = len;
	}
	printf("fake_len=0x%x\n", fake_len);
	for(i=0; i<fake_len; i+=4, ptr++)
	{
		chksum += *ptr;
	}
	return chksum;
}


int usb_run_command (const char *cmd, char* buff)
{
	int ret = -1, flag = 0;
	u32 addr = 0, length = 0;
	u32 crc_value, crc_verify = 0;
	int argc;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	unsigned long upgrade_step;
	printf("cmd: %s\n", cmd);
	
	memset(buff, 0, CMD_BUFF_SIZE);
	if(strncmp(cmd,"get update result",(sizeof("get update result")-1)) == 0){
		ret = usb_get_update_result();
		if(!ret)
		{
			strcpy(buff, "success");
		}
		else
		{
			strcpy(buff, "fail");
		}
		return ret;
	}
	else if(strncmp(cmd,"usb_bootm",(sizeof("usb_bootm")-1)) == 0){
		addr = *((u32*)(&cmd[60]));
		strcpy(buff, "okay");
		usb_bootm(addr);
		strcpy(buff, "fail");
		return -1;
	}
	else if(strncmp(cmd,"crc",(sizeof("crc")-1)) == 0){
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}
		addr = simple_strtoul (argv[1], NULL, 16);
		length = simple_strtoul (argv[2], NULL, 10);
		crc_verify = simple_strtoul (argv[3], NULL, 16);
		//crc_value = crc32 (0, (const uchar *) addr, length);
		crc_value = checkcum_32(addr, length);
		printf("crc_value=0x%x\n", crc_value);
		if(crc_verify == crc_value)
		{
			strcpy(buff, "success");
		}
		else
		{
			strcpy(buff, "failed");
		}
	}
	else if(strncmp(cmd,"cmd_in_mem",(sizeof("cmd_in_mem")-1)) == 0){
		char *cmd_in_mem = NULL;
		/* Extract arguments */
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}
		cmd_in_mem = (char *)simple_strtoul(argv[1], NULL, 0);
		printf("cmd_in_mem: %s\n", cmd_in_mem);
		if(run_command(cmd_in_mem, flag))
		{
			strcpy(buff, "fail");
			return -1;
		}
		else
		{
			strcpy(buff, "okay");
		}
	}
#if 0
	/*
		//efuse read/write command
		"efuse read version"	 	//cmd: efuse read version
		"efuse write version"	 	//cmd: efuse write version
		"efuse read mac"		 	//cmd: efuse read mac
		"efuse write mac "		 	//cmd: efuse write mac xx:xx:xx:xx:xx:xx
		"efuse read bt_mac"	 	//cmd: efuse read bt_mac
		"efuse write bt_mac "	 	//cmd: efuse write bt_mac xx:xx:xx:xx:xx:xx
		"efuse read wifi_mac"	 	//cmd: efuse read wifi_mac
		"efuse write wifi_mac "	 	//cmd: efuse write wifi_mac xx:xx:xx:xx:xx:xx
		"efuse read usid"	 		//cmd: efuse read usid
		"efuse write usid "	 		//cmd: efuse write usid xxxxx...
		"read hdcp"				//cmd: read hdcp
		"write hdcp:"			 	//cmd: write hdcp:xxxxx...		
	*/
	else if( !strncmp(cmd,"efuse",(sizeof("efuse")-1))  || 
		      !strncmp(cmd,"read hdcp",(sizeof("read hdcp")-1))  || 
		      !strncmp(cmd,"write hdcp:",(sizeof("write hdcp:")-1))){
		int argc;
		int i, ret, usid_flag = 0, hdcp_flag = 0, flag = 0;
		int usid_len, hdcp_len, len;
		int writeHdcp_flag = 1;
		char efuse_data[512], test_buff[512], hdcp_verify_data[512];
#ifdef  WRITE_TO_EFUSE_ENABLE	
		char *Argv1[3]={"efuse","read","hdcp"};
		char *Argv2[4]={"efuse","write","hdcp",""};
#elif defined(WRITE_TO_NAND_ENABLE)		
		char *Argv1[3]={"secukey","read","hdcp"};
		char *Argv2[4]={"secukey","write","hdcp",""};	
#endif		
		char *hdcp;
		char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/

		/* Extract arguments */
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}

#ifdef  WRITE_TO_EFUSE_ENABLE
		printf("usb_burning to efuse\n");
#elif defined(WRITE_TO_NAND_ENABLE)	
		printf("usb_burning to nand\n");
#endif
		memset(efuse_data,0,sizeof(efuse_data));
		memset(hdcp_verify_data,0,sizeof(hdcp_verify_data));
		
		//m3/m6 version
		if((!strncmp(argv[1],"write",sizeof("write"))) &&  (!strncmp(argv[2],"version",sizeof("version"))))
		{
			argc ++;	
			#ifdef CONFIG_AML_MESON3
				argv[3] = EFUSE_VERSION_MESON3;				//m3 version
				printf("CONFIG_AML_MESON3(version:%s)\n",argv[3]);
			#elif defined(CONFIG_AML_MESON6) 
				argv[3] = EFUSE_VERSION_MESON6;				//m6 version
				printf("CONFIG_AML_MESON6(version:%s)\n",argv[3]);
			#endif
		}


		//bt_mac/wifi_mac -->mac_bt/mac_wifi
		if(((!strncmp(argv[1],"read",sizeof("read")))||(!strncmp(argv[1],"write",sizeof("write")))) &&  
		      (!strncmp(argv[2],"bt_mac",sizeof("bt_mac"))))
			strcpy(argv[2], "mac_bt");
		else if(((!strncmp(argv[1],"read",sizeof("read")))||(!strncmp(argv[1],"write",sizeof("write")))) &&
	      		      (!strncmp(argv[2],"wifi_mac",sizeof("wifi_mac"))))
	      		 strcpy(argv[2], "mac_wifi"); 


		//hdcp
		if((!strncmp(argv[0],"read",sizeof("read"))) &&  (!strncmp(argv[1],"hdcp",sizeof("hdcp"))))
		{
			argv[0] = Argv1[0];
			argv[1] = Argv1[1];
			argv[2] = Argv1[2];
			argc = 3;
		}
		else if((!strncmp(argv[0],"write",sizeof("write"))) &&  (!strncmp(argv[1],"hdcp:",sizeof("hdcp:"))))
		{
			#define HDCP_DATA_ADDR	(volatile unsigned long *)(0x82000000)	//get hdcp data from address:0x82000000 
			hdcp = HDCP_DATA_ADDR;
					
			printf("receive hdcp_data=");
			for(i=0;i<288;i++)												//read 288 hdcp datas		
			{
				efuse_data[i] = *hdcp++;
				printf("%.2x:", efuse_data[i]);
			}	
			printf("\n");
			
			printf("receive hdcp_verify_data=");
			for(i=0;i<20;i++)													//read 20 hdcp verify datas		
			{
				hdcp_verify_data[i] = *hdcp++;
				printf("%.2x:", hdcp_verify_data[i]);
			}	
			printf("\n");

#ifdef  WRITE_HDCP_VERIFY_ENABLE	
		    	SHA1_CTX sha;
		    	BYTE Message_Digest[20];
		    	hdcp_llc_file *llc_key;

			memset(Message_Digest, 0, sizeof(Message_Digest));
			llc_key = (hdcp_llc_file *)efuse_data;

			printf("llc_key->ksv=");
			for(i=0;i<5;i++)
				printf("%.2x:", llc_key->ksv[i]);	
				
			printf("\nllc_key->rsv=");
			for(i=0;i<3;i++)
				printf("%.2x:", llc_key->rsv[i]);
				
			printf("\nllc_key->dpk=");
			for(i=0;i<280;i++)
				printf("%.2x:", llc_key->dpk[i]);
				
			printf("\nllc_key->sha=");
			for(i=0;i<20;i++)
				printf("%.2x:", llc_key->sha[i]);
			printf("\n");


			printf("start to verify hdcp data...\n");
		    	SHA1Reset(&sha);
		    	SHA1Input(&sha, (unsigned char*)llc_key, 288);
			SHA1Result(&sha, Message_Digest);

			printf("calculate hdcp_verify_data=");
			for(i=0; i<20; i++)
				printf("%.2x:", Message_Digest[i]);
			printf("\n");

			for(i=0; i<20; i++)
			{
				if(hdcp_verify_data[i] != Message_Digest[i])
				{
					writeHdcp_flag = 0;
					break;
				}
			}
#endif
			if(writeHdcp_flag)			//verify success,hdcp can write
			{
				memcpy(Argv2[3], efuse_data, SECUKEY_BYTES);					//copy hdcp datas
				argv[0] = Argv2[0];
				argv[1] = Argv2[1];
				argv[2] = Argv2[2];
				argv[3] = Argv2[3];
				argc = 4; 
			}
			else
			{
				sprintf(buff, "%s", "failed:(hdcp data verify not mach)");
				printf("%s\n",buff);
				return -1;
			}	
			
		}


#ifdef  WRITE_TO_NAND_ENABLE
		argv[0] = Argv2[0];			//if defined WRITE_TO_NAND_ENABLE,"efuse" convert to "secukey"
#endif

		//printf argv[0]--argv[argc-1]
		if((!strncmp(argv[1],"write",sizeof("write"))) &&  (!strncmp(argv[2],"hdcp",sizeof("hdcp"))))
		{
			for(i=0;i<argc-1;i++)  printf("argv[%d]=%s\n", i,argv[i]); 
			hdcp = argv[3];
			printf("argv[3]="); 
			for(i=0;i<SECUKEY_BYTES;i++)  printf("%02x:", *hdcp ++);  
			printf("\n"); 
		}
		else
			for(i=0;i<argc;i++)  printf("argv[%d]=%s\n", i,argv[i]);  
		

		
#ifdef  WRITE_TO_EFUSE_ENABLE	
#ifdef CONFIG_AML_MESON3
		ret = do_efuse_usb(argc, argv, buff);			//efuse read/write in m3 platform
#elif defined(CONFIG_AML_MESON6)
		ret = cmd_efuse(argc, argv, buff);				//efuse read/write in m6 platform
#endif
#elif defined(WRITE_TO_NAND_ENABLE)
		//write to nand
		if ((!strncmp(argv[1],"read",sizeof("read"))) && ( !strncmp(argv[2],"version",sizeof("version"))))
		{
			sprintf(buff, "%s", "failed:(version is not writen)");
			printf("%s\n",buff);
			return 0;
		}
		else if ((!strncmp(argv[1],"write",sizeof("write"))) && ( !strncmp(argv[2],"version",sizeof("version"))))
		{
			ret = ensure_secukey_init();
			if (ret==0 || ret==1)						//init nand success or already inited.
			{
				#ifdef CONFIG_AML_MESON3
					sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON3);
				#elif defined(CONFIG_AML_MESON6) 
					sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON6);
				#endif	
				printf("%s\n",buff);
				return 0;				
			}		
			else	 									//init nand failed!!
			{	
				printf("init nand failed!!\n");
				sprintf(buff, "failed:(write version failed)");	
				printf("%s\n",buff);	
				return -1;
			}
		}		

		ret = cmd_secukey(argc, argv, buff);									
#endif

		if(!ret)														
		{
#ifdef  WRITE_TO_EFUSE_ENABLE	
			//read version/mac/bt_mac/wifi_mac/usid/hdcp sucess 
			if(!strncmp(argv[1],"read",sizeof("read")))
			{														
				if(!strncmp(argv[2],"version",sizeof("version"))) 
				{
					#ifdef CONFIG_AML_MESON3
						sprintf(efuse_data, "%02x:%02x:%02x", buff[0],buff[1],buff[2]);		
					#elif defined(CONFIG_AML_MESON6) 
						sprintf(efuse_data, "%02x", buff[0]);
					#endif

					if(simple_strtol(&efuse_data[0], NULL, 16) != 0x00)
				    		sprintf(buff, "success:(%s)", efuse_data);
					else
						sprintf(buff, "%s", "failed:(version is not writen)");	
				
				}
				else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
				{
					for(i=0;i<6;i++)
					{
						if(buff[i] != 0x00)
						{
							flag = 1;
							break;
						}		
					}
					if(flag)
					{													
						sprintf(efuse_data, "%02x:%02x:%02x:%02x:%02x:%02x", buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);	
						sprintf(buff, "success:(%s)", efuse_data);				//have writen mac/mac_bt/mac_wifi
					}						
					else
						sprintf(buff, "failed:(%s has been not writen)", argv[2]);	//haven't write mac/mac_bt/mac_wifi	
					
				}	
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
				{
					printf("usid_len=%d, usid=%s\n", strlen(buff), buff);
					for(i=0; i<strlen(buff); i++)
					{
						if(buff[i] != 0x00)
						{
							usid_flag = 1;
							break;
						}
					}
					if(usid_flag)
					{
						//sprintf(buff, "%s", "success:(usid has been writen)");	//have writen usid
						memcpy(efuse_data, buff, strlen(buff));
						sprintf(buff, "success:(%s)", efuse_data);	
					}
					else
						sprintf(buff, "%s", "failed:(usid has been not writen)");	//haven't write usid	
						
				}
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
				{
					printf("hdcp_data=");
					for(i=0; i<300; i++)
						printf("%.2x:",buff[i]);
					printf("\n");
					
					for(i=0; i<288; i++)
					{
						if(buff[i] != 0x00)
						{
							hdcp_flag = 1;
							break;
						}
					}
					if(hdcp_flag)
					{
						sprintf(buff, "%s", "success:(hdcp has been writen)");	//have writen hdcp
					}
					else
						sprintf(buff, "%s", "failed:(hdcp has been not writen)");	//haven't write hdcp		
				}	
			}			
#elif defined(WRITE_TO_NAND_ENABLE)		//nand
			if(!strncmp(argv[1],"read",sizeof("read")))
			{
				 if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
				{
					strcpy(namebuf,argv[2]);
					memset(databuf, 0, sizeof(databuf));

					len = uboot_key_read(namebuf, databuf);
					printf("%s_len = uboot_key_read(%s, databuf)\n",argv[2], namebuf);
					if(len>0)
					{
						printf("%s_len=%d\n",argv[2], len/2);
						printf("%s_data=%s\n", argv[2], buff);
						sprintf(efuse_data, "%s", buff);
						sprintf(buff, "success:(%s)", efuse_data);				 //have writen mac/mac_bt/mac_wifi
					}
					else if(len<0)
					{
						printf("%s_len=%d\n",argv[2], len);
						sprintf(buff, "failed:(%s has been not writen)", argv[2]);	//haven't write mac/mac_bt/mac_wifi													
					}	
					
				}
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
				{
					strcpy(namebuf,"usid");
					memset(databuf, 0, sizeof(databuf));
					
					usid_len = uboot_key_read(namebuf, databuf);
					printf("usid_len = uboot_key_read(%s, databuf)\n",namebuf);
					if(usid_len>0)
					{
						printf("usid_len=%d\n",usid_len/2);
						printf("usid_data=%s\n",buff);	
						//sprintf(buff, "%s", "success:(usid has been writen)");	//have writen usid
						memcpy(efuse_data, buff, strlen(buff));
						sprintf(buff, "success:(%s)", efuse_data);	
					}	
					else if(usid_len<0)
					{
						printf("usid_len=%d\n",usid_len);
						sprintf(buff, "%s", "failed:(usid has been not writen)");	//haven't write usid	
					}	
						
				}
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
				{	
					strcpy(namebuf,"hdcp");
					memset(databuf, 0, sizeof(databuf));

					hdcp_len = uboot_key_read(namebuf, databuf);
					printf("hdcp_len = uboot_key_read(%s, databuf)\n",namebuf);
					if(hdcp_len>0)
					{
						printf("hdcp_len=%d\n",hdcp_len/2);
						printf("hdcp_data=");
						for(i=0; i<512; i++)
							printf("%.2x:",buff[i]);
						printf("\n");		
						sprintf(buff, "%s", "success:(hdcp has been writen)");	//have writen hdcp						
					}	
					else if(hdcp_len<0)
					{
						printf("hdcp_len=%d\n",hdcp_len);
						sprintf(buff, "%s", "failed:(hdcp has been not writen)");	//haven't write hdcp							
					}	
						
				}	
			}
#endif			
			//write version/mac/bt_mac/wifi_mac/usid/hdcp sucess 
			else if( !strncmp(argv[1],"write",sizeof("write")))
			{
				if(!strncmp(argv[2],"version",sizeof("version")))			
				{
					#ifdef CONFIG_AML_MESON3
						sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON3);
					#elif defined(CONFIG_AML_MESON6) 
						sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON6);
					#endif					
				}
				else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) || 
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )
					sprintf(buff, "success:(%s)", argv[3]);
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
					//sprintf(buff, "%s", "success:(write usid success)");
					sprintf(buff, "success:(%s)", argv[3]);
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
					sprintf(buff, "%s", "success:(write hdcp success)");

//efuse write success, test efuse read
#if defined( WRITE_TO_EFUSE_ENABLE) && defined(EFUSE_READ_TEST_ENABLE)
				printf("#####efuse write %s success,now test efuse read %s:\n",argv[2], argv[2]);
				memset(test_buff,0,sizeof(test_buff));
				argv[0] 	= "efuse";	
				argv[1]  	= "read";	
				argv[2]  	= argv[2];
				argc = 3;
#ifdef CONFIG_AML_MESON3
				ret = do_efuse_usb(argc, argv, test_buff);			//efuse read/write in m3 platform
#elif defined(CONFIG_AML_MESON6)
				ret = cmd_efuse(argc, argv, test_buff);				//efuse read/write in m6 platform
#endif
				if(!ret)
				{
					if( !strncmp(argv[2],"version",sizeof("version")) )
					{
						#ifdef CONFIG_AML_MESON3
							sprintf(efuse_data, "%02x:%02x:%02x", test_buff[0],test_buff[1],test_buff[2]);		
						#elif defined(CONFIG_AML_MESON6) 
							sprintf(efuse_data, "%02x", test_buff[0]);
						#endif	
						printf("test success,version=%s\n", efuse_data);
					}	
					else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
					{
						sprintf(efuse_data, "%02x:%02x:%02x:%02x:%02x:%02x", test_buff[0],test_buff[1],test_buff[2],test_buff[3],test_buff[4],test_buff[5]);
						printf("test success,%s=%s\n", argv[2], efuse_data);
					}	
					else if( !strncmp(argv[2],"usid",sizeof("usid")) ) 
					{
						printf("test success,%s=%s\n", argv[2], test_buff);
					}
					else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) ) 
					{
						printf("test success,%s=", argv[2]);
						for(i=0;i<512;i++)
							printf("%02x:", test_buff[i]);
						printf("\n");
					}						
				}
				else
					printf("test efuse read %s fail\n", argv[2]);	
#endif
			}

		}
		else														
		{
			//read version/mac/bt_mac/wifi_mac/usid/hdcp fail
			if(!strncmp(argv[1],"read",sizeof("read")))
				sprintf(buff, "failed:(read %s failed)", argv[2]);
									
			//write version/mac/bt_mac/wifi_mac/usid/hdcp fail
			else if(!strncmp(argv[1],"write",sizeof("write")))
				sprintf(buff, "failed:(write %s failed)", argv[2]);				
			
			printf("%s\n",buff);
			return -1;
		}	
	}
#if defined( WRITE_TO_NAND_ENABLE)
	else if(!strncmp(cmd, "secukey_nand", strlen("secukey_nand"))) {
         int i = 0, ret = -1;
         char key_data[SECUKEY_BYTES];
         char *Argv[4] = {"nand", "write", "boardid", ""};
         char *boardid = NULL;
         int boardid_len = 0, boardid_key_len = 0;

         /* Extract arguments */
         if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
         }

         memset(key_data, 0, sizeof(key_data));

         argv[0] = Argv[0];

         if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "boardid", strlen("boardid"))) {
#define BOARDID_DATA_ADDR	(volatile unsigned long *)(0x82000000)    //get boardid data from address:0x82000000
               char length[4] = {0};
               boardid = BOARDID_DATA_ADDR;
               for(i=0; i<4; i++) {
                  length[i] = *boardid++;
                  printf("length[%d]=0x%02x\n", i, length[i]);
               }
               boardid_key_len = (int)((length[3]<<24)|(length[2]<<16)|(length[1]<<8)|(length[0]));
               printf("boardid_key_len=%d\n", boardid_key_len);
               for(i=0; i<boardid_key_len; i++) {
                  key_data[i] = *boardid++;
               }
               printf("receive boardid_data=%s\n", key_data);
               memcpy(Argv[3], key_data, SECUKEY_BYTES);					//copy boardid datas
               argv[0] = Argv[0];
               argv[1] = Argv[1];
               argv[2] = Argv[2];
               argv[3] = Argv[3];
               argc = 4;
         }

         for(i=0; i<argc; i++) {
            printf("argv[%d]=%s\n", i, argv[i]);
         }


         /* version */
         if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "version", strlen("version"))) {
              sprintf(buff, "%s", "failed:(nand not be initialized)");
              printf("%s\n",buff);
              return 0;
         }
         else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "version", strlen("version"))) {
            ret = ensure_secukey_init();
            if (ret==0 || ret==1) {                              //init nand success or already inited.
               sprintf(buff, "%s", "success:(init nand success)");
               printf("%s\n", buff);
               return 0;
            }
            else {                                                       //init nand failed!!
               sprintf(buff, "%s", "failed:(init nand failed)");
               printf("%s\n", buff);
               return -1;
            }
         }

         /* boardid */
         if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "boardid", strlen("boardid"))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  strcpy(namebuf, "boardid");
                  memset(databuf, 0, sizeof(databuf));
                  boardid_len = uboot_key_read(namebuf, databuf);
                  printf("boardid_len = uboot_key_read(%s, databuf)\n", namebuf);
                  if(boardid_len>0) {
                     //printf("boardid_len=%d\n", boardid_len/2);
                     printf("boardid_data=%s\n", buff);
                     memcpy(key_data, buff, strlen(buff));
                     sprintf(buff, "%s", "success:(boardid has been writen)");
                  }
                  else if(boardid_len<0) {
                     //printf("boardid_len=%d\n", boardid_len);
                     sprintf(buff, "%s", "failed:(boardid has been not writen)");
                  }
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
         }
         else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "boardid", strlen("boardid"))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  sprintf(buff, "%s", "success:(nand write boardid success)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
         }
	}
#endif      /* WRITE_TO_NAND_ENABLE */

#else

/*
  *	burn keys to efuse/nand/emmc common command:
  *	"efuse read version"
  *	"efuse write version"
  *	"efuse read mac"
  *	"efuse write mac xx:xx:xx:xx:xx:xx"
  *	"efuse read bt_mac"
  *	"efuse write bt_mac xx:xx:xx:xx:xx:xx"
  *	"efuse read wifi_mac"
  *	"efuse write wifi_mac xx:xx:xx:xx:xx:xx"
  *	"efuse read usid"
  *	"efuse write usid xxxxx..."
  *	"read hdcp"
  *	"write hdcp:"   (hdcp key datas form 0x82000000 address)
  *
  *	or: burn keys to efuse/nand private command:
  *	"secukey_efuse/secukey_nand read version"
  *	"secukey_efuse/secukey_nand write version"
  *	"secukey_efuse/secukey_nand read mac"
  *	"secukey_efuse/secukey_nand write mac xx:xx:xx:xx:xx:xx"
  *	"secukey_efuse/secukey_nand read bt_mac"
  *	"secukey_efuse/secukey_nand write bt_mac xx:xx:xx:xx:xx:xx"
  *	"secukey_efuse/secukey_nand read wifi_mac"
  *	"secukey_efuse/secukey_nand write wifi_mac xx:xx:xx:xx:xx:xx"
  *	"secukey_efuse/secukey_nand read usid"
  *	"secukey_efuse/secukey_nand write usid xxxxx..."
  *	"secukey_efuse/secukey_nand read hdcp"
  *	"secukey_efuse/secukey_nand write hdcp:" (hdcp key datas form 0x82000000 address)
  *	"secukey_nand read boardid"
  *	"secukey_nand write boardid:"    (boardid key datas form 0x82000000 address)
  **/
      else if(!strncmp(cmd, "efuse", strlen("efuse")) ||
            !strncmp(cmd, "read hdcp", strlen("read hdcp")) ||!strncmp(cmd, "write hdcp:", strlen("write hdcp:")) ||
            !strncmp(cmd, "secukey_efuse", strlen("secukey_efuse")) ||!strncmp(cmd, "secukey_nand", strlen("secukey_nand"))) {
            int i = 0, ret = -1, error = -1;
            int flag = 0, usid_flag = 0, hdcp_flag = 0, writeHdcp_flag = 1;
            int hdcp_key_len = 288, boardid_key_len = 0;
            char key_data[SECUKEY_BYTES], hdcp_verify_data_receive[20], hdcp_verify_data_calculate[20];
            char *hdcp = NULL, *boardid = NULL;
            char *Argv1[3] = {"efuse", "read", "hdcp"}, *Argv2[4] = {"efuse", "write", "hdcp", ""};
#if defined(WRITE_TO_NAND_ENABLE)
            char *Argv3[3] = {"nand", "read", "hdcp"}, *Argv4[4] = {"nand", "write", "hdcp", ""};
            char *Argv5[4] = {"nand", "write", "boardid", ""};
#elif defined(WRITE_TO_EMMC_ENABLE)
            char *Argv3[3] = {"emmc", "read", "hdcp"}, *Argv4[4] = {"emmc", "write", "hdcp", ""};
            char *Argv5[4] = {"emmc", "write", "boardid", ""};
#endif

            /* Extract arguments */
            if ((argc = parse_line (cmd, argv)) == 0) {
               return -1;	/* no command at all */
            }

            memset(key_data, 0, sizeof(key_data));
            memset(hdcp_verify_data_receive, 0, sizeof(hdcp_verify_data_receive));
            memset(hdcp_verify_data_calculate, 0, sizeof(hdcp_verify_data_calculate));

/* Burn key to efuse */
/* ---Command process */
#ifdef  WRITE_TO_EFUSE_ENABLE
            if(strncmp(argv[0], "efuse", strlen("efuse")) && 
               strncmp(argv[0], "read", strlen("read")) && strncmp(argv[0], "write", strlen("write")) &&  //cmd:read hdcp/write hdcp:
               strncmp(argv[0], "secukey_efuse", strlen("secukey_efuse"))) {
               sprintf(buff, "%s", "failed:(code compiled to efuse,but cmd not mach with pc send)");
               printf("%s\n", buff);
               return -1;
            }
            printf("burn key to efuse. convert command...\n");
            if(!strncmp(argv[0], "secukey_efuse", strlen("secukey_efuse")))
               argv[0] = "efuse";

            if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "version", strlen("version"))) {
               argc ++;
#ifdef CONFIG_AML_MESON3
               argv[3] = EFUSE_VERSION_MESON3;
               printf("CONFIG_AML_MESON3 VERSION(version:%s)\n",argv[3]);
#elif defined(CONFIG_AML_MESON6)
               argv[3] = EFUSE_VERSION_MESON6;
               printf("CONFIG_AML_MESON6 VERSION(version:%s)\n",argv[3]);
#endif
            }

            if(!strncmp(argv[2], "bt_mac", strlen("bt_mac")))
               strncpy(argv[2], "mac_bt", strlen("mac_bt"));

            if(!strncmp(argv[2], "wifi_mac", strlen("wifi_mac")))
               strncpy(argv[2], "mac_wifi", strlen("mac_wifi"));

            if(!strncmp(argv[0], "read", strlen("read")) && !strncmp(argv[1], "hdcp", strlen("hdcp"))) {
               argv[0] = Argv1[0];
               argv[1] = Argv1[1];
               argv[2] = Argv1[2];
               argc = 3;
            }

            if(!strncmp(argv[0], "write", strlen("write")) && !strncmp(argv[1], "hdcp:", strlen("hdcp:")) ||
                !strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "hdcp:", strlen("hdcp:"))) {
#define HDCP_DATA_ADDR	(volatile unsigned long *)(0x82000000)	//get hdcp data from address:0x82000000 
               hdcp = HDCP_DATA_ADDR;
               printf("receive %d hdcp key datas from address:0x82000000:\n", hdcp_key_len);
               for(i=0; i<hdcp_key_len; i++) {                                            //read 288 hdcp datas
                  key_data[i] = *hdcp++;
                  printf("%.2x:", key_data[i]);
               }
               printf("\nreceive 20 hdcp key verify datas:\n");
               for(i=0; i<20; i++) {											     //read 20 hdcp verify datas
                  hdcp_verify_data_receive[i] = *hdcp++;
                  printf("%.2x:", hdcp_verify_data_receive[i]);
               }
               printf("\n");

#ifdef  WRITE_HDCP_VERIFY_ENABLE
               printf("start to verify %d hdcp key datas...\n", hdcp_key_len);
               SHA1_Perform(key_data, hdcp_key_len, hdcp_verify_data_calculate);
               printf("verify & get 20 hdcp verify datas:\n");
               for(i=0; i<20; i++)
			   printf("%.2x:", hdcp_verify_data_calculate[i]);
               printf("\n");

               for(i=0; i<20; i++) {
			   if(hdcp_verify_data_receive[i] != hdcp_verify_data_calculate[i]) {
			      writeHdcp_flag = 0;
			      break;
			   }
               }
#endif
               if(writeHdcp_flag)	{		                                                   //hdcp can write
                  memcpy(Argv2[3], key_data, hdcp_key_len);		         //copy hdcp datas
                  argv[0] = Argv2[0];
                  argv[1] = Argv2[1];
                  argv[2] = Argv2[2];
                  argv[3] = Argv2[3];
                  argc = 4;
               }
               else {
                  sprintf(buff, "%s", "failed:(hdcp data verify not mach)");
                  printf("%s\n",buff);
                  return -1;
               }
            }
#endif   /* WRITE_TO_EFUSE_ENABLE */


/* Burn key to nand */
/* ---Command process */
#if defined(WRITE_TO_NAND_ENABLE) || defined(WRITE_TO_EMMC_ENABLE)
            if(strncmp(argv[0], "efuse", strlen("efuse")) && 
               strncmp(argv[0], "read", strlen("read")) && strncmp(argv[0], "write", strlen("write")) && 
               strncmp(argv[0], "secukey_nand", strlen("secukey_nand"))) {
               sprintf(buff, "failed:(code compiled to %s,but cmd not mach with pc send)", Argv3[0]);
               printf("%s\n", buff);
               return -1;
            }

            printf("burn key to %s. convert command...\n", Argv3[0]);
            if(!strncmp(argv[0], "efuse", strlen("efuse"))||!strncmp(argv[0], "secukey_nand", strlen("secukey_nand")))
               argv[0] = Argv3[0];

            if(!strncmp(argv[2], "bt_mac", strlen("bt_mac")))
               strncpy(argv[2], "mac_bt", strlen("mac_bt"));
            
            if(!strncmp(argv[2], "wifi_mac", strlen("wifi_mac")))
               strncpy(argv[2], "mac_wifi", strlen("mac_wifi"));

            if(!strncmp(argv[0], "read", strlen("read")) && !strncmp(argv[1], "hdcp", strlen("hdcp"))) {
               argv[0] = Argv3[0];
               argv[1] = Argv3[1];
               argv[2] = Argv3[2];
               argc = 3;
            }

            if(!strncmp(argv[0], "write", strlen("write")) && !strncmp(argv[1], "hdcp:", strlen("hdcp:")) ||
                !strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "hdcp:", strlen("hdcp:"))) {
#define HDCP_DATA_ADDR	(volatile unsigned long *)(0x82000000)
               hdcp = HDCP_DATA_ADDR;
               printf("receive %d hdcp key datas from address:0x82000000:\n", hdcp_key_len);
               for(i=0; i<hdcp_key_len; i++) {
                  key_data[i] = *hdcp++;
                  printf("%.2x:", key_data[i]);
               }
               printf("\nreceive 20 hdcp key verify datas:\n");
               for(i=0; i<20; i++) {
                  hdcp_verify_data_receive[i] = *hdcp++;
                  printf("%.2x:", hdcp_verify_data_receive[i]);
               }
               printf("\n");

#ifdef  WRITE_HDCP_VERIFY_ENABLE
               printf("start to verify %d hdcp key datas...\n", hdcp_key_len);
               SHA1_Perform(key_data, hdcp_key_len, hdcp_verify_data_calculate);
               printf("verify & get 20 hdcp verify datas:\n");
               for(i=0; i<20; i++)
			   printf("%.2x:", hdcp_verify_data_calculate[i]);
               printf("\n");

               for(i=0; i<20; i++) {
			   if(hdcp_verify_data_receive[i] != hdcp_verify_data_calculate[i]) {
			      writeHdcp_flag = 0;
			      break;
			   }
               }
#endif
               if(writeHdcp_flag)	{
                  memcpy(Argv4[3], key_data, hdcp_key_len);
                  argv[0] = Argv4[0];
                  argv[1] = Argv4[1];
                  argv[2] = Argv4[2];
                  argv[3] = Argv4[3];
                  argc = 4;
               }
               else {
                  sprintf(buff, "%s", "failed:(hdcp data verify not mach)");
                  printf("%s\n",buff);
                  return -1;
               }
            }

            if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "boardid:", strlen("boardid:"))) {
#define BOARDID_DATA_ADDR	(volatile unsigned long *)(0x82000000)//get boardid data from address:0x82000000
               char length[4] = {0};
               boardid = BOARDID_DATA_ADDR;
               for(i=0; i<4; i++) {
                  length[i] = *boardid++;
                  //printf("length[%d]=0x%02x\n", i, length[i]);
               }
               boardid_key_len = (int)((length[3]<<24)|(length[2]<<16)|(length[1]<<8)|(length[0]));
               printf("boardid_key_len=%d(maximum length limit is %d)\n", boardid_key_len, SECUKEY_BYTES);
                  for(i=0; i<boardid_key_len; i++) {
                     key_data[i] = *boardid++;
                  }
                  printf("receive %d boardid key datas from address:0x82000000:\n%s\n", boardid_key_len, key_data);
                  memcpy(Argv5[3], key_data, SECUKEY_BYTES);                  //copy boardid datas
                  argv[0] = Argv5[0];
                  argv[1] = Argv5[1];
                  argv[2] = Argv5[2];
                  argv[3] = Argv5[3];
                  argc = 4;
            }
#endif   /* WRITE_TO_NAND_ENABLE */


            //printf argv[0]--argv[argc-1]
            if(!strncmp(argv[1], "write", strlen("write")) &&  !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
               for(i=0; i<argc-1; i++) printf("argv[%d]=%s\n", i, argv[i]);
               hdcp = argv[3];
               printf("argv[3]=");
               for(i=0; i<hdcp_key_len; i++) printf("%02x:", *hdcp ++);
               printf("\n");
            }
            else
               for(i=0; i<argc; i++)  printf("argv[%d]=%s\n", i,argv[i]);

/* Burn key to efuse */
/* ---The actual function to read & write operation */
#ifdef  WRITE_TO_EFUSE_ENABLE
            /* read/write version */
            if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "version", strlen("version"))) {
               ret = run_efuse_cmd(argc, argv, buff);
#ifdef CONFIG_AML_MESON3
               if(!ret) {
                  for(i=0; i<3; i++) {
                     if(buff[i] != 0x00) {
                        flag = 1;
                        break;
                     }
                  }
                  if(flag) {
                     sprintf(key_data, "%02x:%02x:%02x", buff[0], buff[1], buff[2]);
                     printf("version=%s\n", key_data);
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "%s", "failed:(version has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
#elif defined(CONFIG_AML_MESON6)
               if(!ret) {
                  if(buff[0] != 0x00) {
                     sprintf(key_data, "%02x", buff[0]);
                     printf("version=%s\n", key_data);
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "%s", "failed:(version has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
#endif
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "version", strlen("version"))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
#ifndef EFUSE_READ_TEST_ENABLE
                  sprintf(buff, "success:(%s)", argv[3]);
#else
                  argv[1] = "read";
                  argc = 3;
                  ret = test_efuse_read(argc, argv, argv[3]);
                  if(!ret)
                     sprintf(buff, "success:(%s)", argv[3]);
                  else if(ret == -1) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read data not match write)");
                     printf("%s\n", buff);
                     return -1;
                  }
                  else if(ret == -2) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read fail)");
                     printf("%s\n", buff);
                     return -1;
                  }
#endif
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write mac/mac_bt/mac_wifi */
            else if(!strncmp(argv[1], "read", strlen("read")) && (!strncmp(argv[2], "mac", strlen("mac")) ||
               !strncmp(argv[2], "mac_bt", strlen("mac_bt")) ||!strncmp(argv[2], "mac_wifi", strlen("mac_wifi")))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
                  for(i=0; i<6; i++) {
                     if(buff[i] != 0x00) {
                        flag = 1;
                        break;
                     }
                  }
                  if(flag) {
                     sprintf(key_data, "%02x:%02x:%02x:%02x:%02x:%02x", buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);
                     printf("%s_key_data=%s\n", argv[2], key_data);
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "failed:(%s has been not writen)", argv[2]);
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && (!strncmp(argv[2], "mac", strlen("mac")) || 
               !strncmp(argv[2], "mac_bt", strlen("mac_bt")) ||!strncmp(argv[2], "mac_wifi", strlen("mac_wifi")))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
#ifndef EFUSE_READ_TEST_ENABLE
                  sprintf(buff, "success:(%s)", argv[3]);
#else
                  argv[1] = "read";
                  argc = 3;
                  ret = test_efuse_read(argc, argv, argv[3]);
                  if(!ret)
                     sprintf(buff, "success:(%s)", argv[3]);
                  else if(ret == -1) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read data not match write)");
                     printf("%s\n", buff);
                     return -1;
                  }
                  else if(ret == -2) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read fail)");
                     printf("%s\n", buff);
                     return -1;
                  }
#endif
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write usid */
            else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "usid", strlen("usid"))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
                  for(i=0; i<strlen(buff); i++) {
                     if(buff[i] != 0x00) {
                        usid_flag = 1;
                        break;
                     }
                  }
                  if(usid_flag) {
                     printf("usid_key_data=%s\n", buff);
                     memcpy(key_data, buff, strlen(buff));
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "%s", "failed:(usid has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "usid", strlen("usid"))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
#ifndef EFUSE_READ_TEST_ENABLE
                  sprintf(buff, "success:(%s)", argv[3]);
#else
                  argv[1] = "read";
                  argc = 3;
                  ret = test_efuse_read(argc, argv, argv[3]);
                  if(!ret)
                     sprintf(buff, "success:(%s)", argv[3]);
                  else if(ret == -1) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read data not match write)");
                     printf("%s\n", buff);
                     return -1;
                  }
                  else if(ret == -2) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read fail)");
                     printf("%s\n", buff);
                     return -1;
                  }
#endif
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write hdcp */
            else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
                  for(i=0; i<hdcp_key_len; i++) {
                     if(buff[i] != 0x00) {
                        hdcp_flag = 1;
                        break;
                     }
                  }
                  if(hdcp_flag) {
                     printf("hdcp_key_data=");
                     for(i=0; i<hdcp_key_len; i++)
                        printf("%.2x:", buff[i]);
                     printf("\n");
                     sprintf(buff, "%s", "success:(hdcp has been writen)");
                  }
                  else
                     sprintf(buff, "%s", "failed:(hdcp has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
               ret = run_efuse_cmd(argc, argv, buff);
               if(!ret) {
#ifndef EFUSE_READ_TEST_ENABLE
                  sprintf(buff, "%s", "success:(efuse write hdcp success)");
#else
                  argv[1] = "read";
                  argc = 3;
                  ret = test_efuse_read(argc, argv, argv[3]);
                  if(!ret)
                     sprintf(buff, "%s", "success:(efuse write hdcp success)");
                  else if(ret == -1) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read data not match write)");
                     printf("%s\n", buff);
                     return -1;
                  }
                  else if(ret == -2) {
                     sprintf(buff, "%s", "failed:(efuse write success,but test read fail)");
                     printf("%s\n", buff);
                     return -1;
                  }
#endif
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* no command mach */
            else {
               sprintf(buff, "%s", "failed:(No command mached)");
               printf("%s\n", buff);
               return -1;
            }
#endif   /* WRITE_TO_EFUSE_ENABLE */


/* Burn key to nand */
/* ---The actual function to read & write operation */
#if defined(WRITE_TO_NAND_ENABLE) || defined(WRITE_TO_EMMC_ENABLE)
            /* read/write version */
            if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "version", strlen("version"))) {
               sprintf(buff, "failed:(%s not be initialized)", argv[0]);
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "version", strlen("version"))) {
               ret = ensure_secukey_init();
               if (ret==0 || ret==1) {                              //init nand/emmc success or already inited.
                  sprintf(buff, "success:(init %s success)", argv[0]);
               }
               else {                                                       //init nand/emmc failed!!
                  sprintf(buff, "failed:(init %s failed)", argv[0]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write mac/mac_bt/mac_wifi */
            else if(!strncmp(argv[1], "read", strlen("read")) && (!strncmp(argv[2], "mac", strlen("mac")) ||
               !strncmp(argv[2], "mac_bt", strlen("mac_bt")) ||!strncmp(argv[2], "mac_wifi", strlen("mac_wifi")))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  strncpy(namebuf, argv[2], strlen(argv[2]));
                  memset(databuf, 0, sizeof(databuf));
                  error = uboot_key_read(namebuf, databuf);
                  if(error >= 0) {
                     printf("%s_key_data=%s\n", argv[2], buff);
                     sprintf(key_data, "%s", buff);
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "failed:(%s has been not writen)", argv[2]);
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && (!strncmp(argv[2], "mac", strlen("mac")) ||
               !strncmp(argv[2], "mac_bt", strlen("mac_bt")) ||!strncmp(argv[2], "mac_wifi", strlen("mac_wifi")))) {
               for(i=0; i<4; i++) buff[i] = (char)((strlen(argv[3]) >> (i*8)) & 0xff);
               ret = cmd_secukey(argc, argv, buff);
               if(!ret)
                  sprintf(buff, "success:(%s)", argv[3]);
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write usid */
            else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "usid", strlen("usid"))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  strncpy(namebuf, "usid", strlen("usid"));
                  memset(databuf, 0, sizeof(databuf));
                  error = uboot_key_read(namebuf, databuf);
                  if(error >= 0) {
                     printf("usid_key_data=%s\n", buff);
                     memcpy(key_data, buff, strlen(buff));
                     sprintf(buff, "success:(%s)", key_data);
                  }
                  else
                     sprintf(buff, "%s", "failed:(usid has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "usid", strlen("usid"))) {
               for(i=0; i<4; i++) buff[i] = (char)((strlen(argv[3]) >> (i*8)) & 0xff);
               ret = cmd_secukey(argc, argv, buff);
               if(!ret)
                  sprintf(buff, "success:(%s)", argv[3]);
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write hdcp */
            else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  strncpy(namebuf, "hdcp", strlen("hdcp"));
                  memset(databuf, 0, sizeof(databuf));
                  error = uboot_key_read(namebuf, databuf);
                  if(error >= 0) {
                     printf("hdcp_key_data=");
                     for(i=0; i<hdcp_key_len; i++)
                        printf("%.2x:", buff[i]);
                     printf("\n");
                     sprintf(buff, "%s", "success:(hdcp has been writen)");
                  }
                  else
                     sprintf(buff, "%s", "failed:(hdcp has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "hdcp", strlen("hdcp"))) {
               for(i=0; i<4; i++) buff[i] = (char)((hdcp_key_len >> (i*8)) & 0xff);
               ret = cmd_secukey(argc, argv, buff);
               if(!ret)
                  sprintf(buff, "success:(%s write hdcp success)", argv[0]);
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* read/write boardid */
            else if(!strncmp(argv[1], "read", strlen("read")) && !strncmp(argv[2], "boardid", strlen("boardid"))) {
               ret = cmd_secukey(argc, argv, buff);
               if(!ret) {
                  strncpy(namebuf, "boardid", strlen(boardid));
                  memset(databuf, 0, sizeof(databuf));
                  error = uboot_key_read(namebuf, databuf);
                  if(error >= 0) {
                     printf("boardid_key_data=%s\n", buff);
                     memcpy(key_data, buff, strlen(buff));
                     sprintf(buff, "%s", "success:(boardid has been writen)");
                  }
                  else
                     sprintf(buff, "%s", "failed:(boardid has been not writen)");
               }
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }
            else if(!strncmp(argv[1], "write", strlen("write")) && !strncmp(argv[2], "boardid", strlen("boardid"))) {
               for(i=0; i<4; i++) buff[i] = (char)((strlen(argv[3]) >> (i*8)) & 0xff);
               ret = cmd_secukey(argc, argv, buff);
               if(!ret)
                  sprintf(buff, "success:(%s write boardid success)", argv[0]);
               else {
                  sprintf(buff, "failed:(%s %s %s failed)", argv[0], argv[1], argv[2]);
                  printf("%s\n", buff);
                  return -1;
               }
            }

            /* no command mach */
            else {
               sprintf(buff, "%s", "failed:(No command mached)");
               printf("%s\n", buff);
               return -1;
            }
#endif   /* WRITE_TO_NAND_ENABLE || WRITE_TO_EMMC_ENABLE */
      }
#endif
	else
	{
		if(run_command(cmd, flag))
		{
			strcpy(buff, "fail");
			return -1;
		}
		else
		{
			strcpy(buff, "okay");
		}
	}

	printf("%s\n",buff);
	return 0;
}



#if defined(WRITE_TO_NAND_ENABLE) || defined(WRITE_TO_EMMC_ENABLE)
int ensure_secukey_init(void)
{
	int error;
	char *cmd;

	if (secukey_inited){
		printk("flash device already inited!!\n");
		return 1;
	}
	
	printk("should be inited first!\n");

#if defined(CONFIG_AML_NAND_KEY)
	error = uboot_key_init();
	if(error >= 0){
		error = nandkey_provider_register();
		if(error >= 0){
			 error = key_set_version("nand");
			 if(error >= 0){
				printk("init key ok!!\n");
				secukey_inited = 1;
				return 0;
			}
		}
	}
	else
	{
		printk("init error\n");
		return -1;
	}
#elif defined (CONFIG_AML_EMMC_KEY)
	error = uboot_key_init();
	if(error >= 0){
		error = emmckey_provider_register();
		if(error >= 0){
			 error = key_set_version("emmc");
			 if(error >= 0){
				printk("init key ok!!\n");
				secukey_inited = 1;
				return 0;
			}
		}
	}
	else
	{
		printk("init error\n");
		return -1;
	}
#endif

	return -1;
}

char i_to_asc(char para)
{
	if(para>=0 && para<=9)
		para = para+'0';
	else if(para>=0xa && para<=0xf)
		para = para+'a'-0xa;
		
		return para;
}

char asc_to_i(char para)
{
	if(para>='0' && para<='9')
		para = para-'0';
	else if(para>='a' && para<='f')
		para = para-'a'+0xa;
	else if(para>='A' && para<='F')
		para = para-'A'+0xa;
		
		return para;
}

int cmd_secukey(int argc, char * const argv[], char *buf)
{
	int i,j, ret = 0,error;
	char *cmd;
	char *name;
	char *data;
	/* at least two arguments please */
	if (argc < 2)
		goto usage;
	cmd = argv[1];
	
	memset(databuf, 0, sizeof(databuf));
	if (secukey_inited){
		if (argc > 2&&argc<5){
			if(!strcmp(cmd,"read")){
				if (argc>3)
					goto usage;
				name=argv[2];
				memset(buf, 0, CMD_BUFF_SIZE);
				strcpy(namebuf,name);
				error=uboot_key_read(namebuf, databuf);
                for(i=0; i<CMD_BUFF_SIZE*2; i++)
                    printf(":%c", databuf[i]);
                printf("\n");
				if(error>=0){
					for (i=0,j=0; i<CMD_BUFF_SIZE*2; i++,j++){
							buf[j]= (((asc_to_i(databuf[i]))<<4) | (asc_to_i(databuf[++i])));
					}
					printf("%s is: ", namebuf);
					for(i=0; i<CMD_BUFF_SIZE; i++)
						printf(":%02x", buf[i]);
					printf("\n");
					return 0;
				}
				else{
					printk("read error!!\n");
					return -1;
				}
			}
			if(!strcmp(cmd,"write")){
				if (argc!=4)
					goto usage;
				name=argv[2];
				data=argv[3];
				strcpy(namebuf,name);
				int secukey_len = (int)((buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]));
				printf("secukey_len=%d\n", secukey_len);
				for (i=0,j=0; i<secukey_len; i++,j++){
						databuf[j]= i_to_asc((data[i]>>4) & 0x0f);
						databuf[++j]= i_to_asc((data[i]) & 0x0f);
						printk("%02x:%02x:", databuf[j-1], databuf[j]);
				}
				printk("right here!!!\n");
				//memcpy(buf,databuf,SECUKEY_BYTES*2);
				error=uboot_key_write(namebuf, databuf);
				if(error>=0){
					printk("write key ok!!\n");
					return 0;
				}
				else{
					printk("write error!!\n");
					return -1;
				}	
			}
		}
	}
	else
		goto usage ;
		
usage:
	return 1;
}
#endif
