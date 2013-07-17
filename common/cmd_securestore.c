
#include <common.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/err.h>
#include <amlogic/secure_storage.h>
//#include <nand.h>
//#include <asm/arch/nand.h>



#define SECURE_STORAGE_NAND_TYPE	1
#define SECURE_STORAGE_SPI_TYPE		2
#define SECURE_STORAGE_EMMC_TYPE	3

#define SECURE_STORAGE_WRITE_PERMIT		1
#define SECURE_STORAGE_WRITE_PROHIBIT	0


static int storage_type = 0;
static int storage_status=SECURE_STORAGE_WRITE_PROHIBIT;
static unsigned int securestorage_addr=0,securestorage_len=0;
static int securestorage_start=0;
static int sstorekey_start = 0;
int do_securestore(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int len,addr;
	const char *cmd;
	int err;
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	if((!strcmp(cmd,"nand"))||(!strcmp(cmd,"spi"))){
		if(!strcmp(cmd,"nand")){
			storage_type = SECURE_STORAGE_NAND_TYPE;
		}
		if(!strcmp(cmd,"spi")){
			storage_type = SECURE_STORAGE_SPI_TYPE;
		}
		if(argc == 3){
			if(!strcmp(argv[2],"permit")){
				storage_status = SECURE_STORAGE_WRITE_PERMIT;
				printf("secure storage write permit\n");
			}
			if(!strcmp(argv[2],"prohibit")){
				storage_status = SECURE_STORAGE_WRITE_PROHIBIT;
				printf("secure storage write prohibited\n");
			}
		}
		return 0;
	}
	if(!strcmp(cmd,"emmc")){
		//storage_type = SECURE_STORAGE_EMMC_TYPE;
		return 0;
	}
	if(!storage_type ){
		printf("please set device\n");
		return 1;
	}
	if(!strcmp(cmd,"write")){
		if((argc > 2)&&(argc < 4)){
			goto usage;
		}
		if(storage_status != SECURE_STORAGE_WRITE_PERMIT){
			printf("secure storage write is prohibited\n");
			goto usage;
		}
		if(argc >=4){
			addr = simple_strtoul(argv[2], NULL, 16);
			len = simple_strtoul(argv[3], NULL, 16);
		}
		else{
			addr =  SECUREOS_KEY_DEFAULT_ADDR;
			len = SECUREOS_KEY_DEFAULT_SIZE;
		}
		if(storage_type == SECURE_STORAGE_NAND_TYPE){
			err = secure_storage_nand_write((char*)addr,len);
			if(err){
				printf("%s:%d,write key fail\n",__func__,__LINE__);
				return 1;
			}
			printf("write to nand ok\n");
		}
		else if(storage_type == SECURE_STORAGE_SPI_TYPE){
			err = secure_storage_spi_write((char*)addr,len);
			if(err){
				printf("%s:%d,write key fail\n",__func__,__LINE__);
				return 1;
			}
			printf("write to spi ok\n");
		}
		else{
			printf("not support\n");
		}
		return 0;
	}
	if(!strcmp(cmd,"read")){
		if((argc > 2)&&(argc < 4)){
			goto usage;
		}
		if(argc >=4){
			addr = simple_strtoul(argv[2], NULL, 16);
			len = simple_strtoul(argv[3], NULL, 16);
		}
		else{
			addr =  SECUREOS_KEY_DEFAULT_ADDR;
			len = SECUREOS_KEY_DEFAULT_SIZE;
		}
		if(storage_type == SECURE_STORAGE_NAND_TYPE){
			err = secure_storage_nand_read((char*)addr,len);
			if(err){
				printf("%s:%d,read key fail from nand\n",__func__,__LINE__);
				return 1;
			}
			printf("from nand read key ok\n");
		}
		else if(storage_type == SECURE_STORAGE_SPI_TYPE){
			err = secure_storage_spi_read((char*)addr,len);
			if(err){
				printf("%s:%d,read key fail from spi\n",__func__,__LINE__);
				return 1;
			}
			printf("from spi read key ok\n");
		}
		else{
			printf("not support\n");
		}
		return 0;
	}

usage:
	cmd_usage(cmdtp);
	return 1;
}

int do_sstorekey(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int len,addr,reallen;
	const char *cmd,*keyname;
	int err;
	if (argc < 3)
		goto usage;

	cmd = argv[1];
	if((!strcmp(cmd,"nand"))||(!strcmp(cmd,"spi"))){
		if(!strcmp(argv[2],"start")){
			addr = SECUREOS_KEY_DEFAULT_ADDR;
			len = SECUREOS_KEY_DEFAULT_SIZE;
			if(!strcmp(cmd,"nand")){
				err = secure_storage_nand_read((char*)addr,len);
				if(err){
					printf("%s:%d,read key fail from %s\n",__func__,__LINE__,cmd);
					return 1;
				}
			}
			if(!strcmp(cmd,"spi")){
				err = secure_storage_spi_read((char*)addr,len);
				if(err){
					printf("%s:%d,read key fail from %s\n",__func__,__LINE__,cmd);
					return 1;
				}
			}
			err = secure_storage_init();
			if(err){
				printf("%s:%d,secure storage init fail\n",__func__,__LINE__);
				return 1;
			}
			sstorekey_start=1;
			printf("start key read/write\n");
			return 0;
		}
		if(!strcmp(argv[2],"stop")){
			sstorekey_start =0;
			printf("stop key read/write\n");
			return 0;
		}
	}
	if(sstorekey_start == 0){
		printf("please start sstorekey read/write\n");
		return 1;
	}
	if(!strcmp(cmd,"write")){
		if(argc < 5){
			goto usage;
		}
		keyname = argv[2];
		
		addr = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);
		err = secure_storage_write_a_key(keyname, strlen(keyname), (unsigned char* )addr, len);
		if(err){
			printf("write a key fail\n");
			return 1;
		}
		printf("write a key ok\n");
		return 0;
	}
	if(!strcmp(cmd,"read")){
		if(argc < 5){
			goto usage;
		}
		keyname = argv[2];
		addr = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);
		err = secure_storage_read_a_key(keyname,strlen(keyname),(unsigned char*)addr,len,&reallen);
		if(err){
			printf("%s:%d,read a key fail\n",__func__,__LINE__);
			return 1;
		}
		printf("read a key ok\n");
		return 0;
	}
usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(securestore, CONFIG_SYS_MAXARGS, 1, do_securestore,
	"securestore sub-system",
	"device  [permit/prohibit]  --- device: nand/emmc/spi \n"
	"securestore write [addr] [len]  -- write secure key to device\n"
	"securestore read  [addr] [len]   -- read secure key for device\n"
);

U_BOOT_CMD(sstorekey, CONFIG_SYS_MAXARGS, 1, do_sstorekey,
	"sstorekey sub-system",
	"device start/stop    --- device: nand/emmc/spi \n"
	"sstorekey write key-name data-addr data-len  --write a key \n"
	"sstorekey read key-name data-addr data-len    -- read a key\n"
);



#if defined(CONFIG_CMD_SECURESTORE) && defined(CONFIG_CMD_RANDOM) && defined(CONFIG_EFUSE)
static char digit_string[10];
static void inttosacii(unsigned int dat)
{
	int i;
	unsigned char cha;
	memset(digit_string,0,sizeof(cha));
	for(i=8;i>0;i--){
		cha = (dat&(0xf<<((i-1)*4)))>>((i-1)*4);
		if((cha>=0)&&(cha<=9)){
			cha += '0';
		}
		else if((cha>=0xa)&&(cha<=0xf)){
			cha += ('a' - 0xa);
		}
		digit_string[8-i] = cha;
	}
	//printf("dat:%s\n",digit_string);
}

ssize_t efuse_write(const char *buf, size_t count, loff_t *ppos );
ssize_t efuse_read(char *buf, size_t count, loff_t *ppos );
int securestore_key_init( char *seed,int len)
{
	int i;
	char cmd_buf[512];
	char aeskey_data[32];
	int aeskey_flag;
	loff_t aeskey_pos;
	unsigned int keyseed = (seed[0]<<24)|(seed[1]<<16)|(seed[2]<<8)|seed[3];
	
	aeskey_flag = 0;
	aeskey_pos = 404;
	memset(aeskey_data,0,0x20);
	efuse_read(aeskey_data,0x20,&aeskey_pos);
	for(i=0;i<0x20;i++){
		if(aeskey_data[i]!=0){
			aeskey_flag = 1;
			printf("aeskey exist\n");
			break;
		}
	}

	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"sstorekey ");
	strcat(cmd_buf,SECURESTORAGE_DEVICE_NAND);
	strcat(cmd_buf," start");
	//run_command("sstorekey nand start",0);
	run_command(cmd_buf,0);
	
	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"securestore ");
	strcat(cmd_buf,SECURESTORAGE_DEVICE_NAND);
	strcat(cmd_buf," permit");
	//run_command("securestore nand permit",0);
	run_command(cmd_buf,0);
	
	if(!aeskey_flag){
		printf("write aeskey to efuse\n");
		inttosacii(keyseed);
		memset(cmd_buf,0,sizeof(cmd_buf));
		strcpy(cmd_buf,"randomb ");
		strcat(cmd_buf,"gen ");
		strcat(cmd_buf,digit_string);
		strcat(cmd_buf," 20 ");
		strcat(cmd_buf,"0x82000000 ");
		run_command(cmd_buf,0);

		memset(aeskey_data,0,0x20);
		memcpy(aeskey_data,(char*)0x82000000,0x20);
		aeskey_pos = 404;
		efuse_write(aeskey_data,0x20,&aeskey_pos);
	}
	return 0;
}
int securestore_key_read(char *keyname,char *keybuf,unsigned int keylen)
{
	char cmd_buf[512];
	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"sstorekey ");
	strcat(cmd_buf,"read ");
	strcat(cmd_buf,keyname);
	strcat(cmd_buf,"  ");
	inttosacii((unsigned int)keybuf);
	strcat(cmd_buf,digit_string);
	strcat(cmd_buf,"  ");
	inttosacii(keylen);
	strcat(cmd_buf,digit_string);
	strcat(cmd_buf,"  ");

	run_command(cmd_buf,0);
	return 0;
}
int securestore_key_write(char *keyname, char *keybuf,unsigned int keylen)
{
	char cmd_buf[512];
	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"sstorekey ");
	strcat(cmd_buf,"write ");
	strcat(cmd_buf,keyname);
	strcat(cmd_buf,"  ");
	inttosacii((unsigned int)keybuf);
	strcat(cmd_buf,digit_string);
	strcat(cmd_buf,"  ");
	inttosacii(keylen);
	strcat(cmd_buf,digit_string);
	strcat(cmd_buf,"  ");
	
	run_command(cmd_buf,0);
	//run_command("sstorekey write keyname keybuf keylen",0);
	return 0;
}

int securestore_key_uninit()
{
	char cmd_buf[512];
	run_command("securestore write",0);

	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"sstorekey ");
	strcat(cmd_buf,SECURESTORAGE_DEVICE_NAND);
	strcat(cmd_buf," stop");
	//run_command("sstorekey nand stop",0);
	run_command(cmd_buf,0);
	memset(cmd_buf,0,sizeof(cmd_buf));
	strcpy(cmd_buf,"securestore ");
	strcat(cmd_buf,SECURESTORAGE_DEVICE_NAND);
	strcat(cmd_buf," prohibit");
	//run_command("securestore nand prohibit",0);
	run_command(cmd_buf,0);
	return 0;
}


int do_sstorekey_test(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;
	unsigned int len,addr;
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	if(!strcmp(cmd,"init")){
		if(argc>3){
			addr = simple_strtoul(argv[2], NULL, 16);
			len = simple_strtoul(argv[3], NULL, 16);
		}
		else{
			char seedkey[]={1,2,3,4};
			addr = (unsigned int)&seedkey[0];
			len =4;
		}
		securestore_key_init(addr,len);
		return 0;
	}
	if(!strcmp(cmd,"uninit")){
		securestore_key_uninit();
		return 0;
	}
	if(!strcmp(cmd,"write")){
		if(argc<5){
			printf("para too few\n");
			goto usage;
		}
		addr = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);
		securestore_key_write(argv[2], addr,len);
		return 0;
	}
	if(!strcmp(cmd,"read")){
		if(argc<5){
			printf("para too few\n");
			goto usage;
		}
		addr = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);
		securestore_key_read(argv[2],(char*)addr,len);
		return 0;
	}

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(sstorekey_test, CONFIG_SYS_MAXARGS, 1, do_sstorekey_test,
	"sstorekey_test sub-system",
	"init/uninit    --- device: nand/emmc \n"
	"sstorekey_test write key-name data-addr data-len  --write a key \n"
	"sstorekey_test read key-name data-addr data-len    -- read a key\n"
);
#endif

