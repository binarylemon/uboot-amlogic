#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/efuse.h>
#include "efuse_bch_8.h"

extern efuseinfo_t efuse_info[];
extern unsigned efuse_info_cnt;

#define EFUSE_WRITE 0
#define EFUSE_READ 1
#define EFUSE_DUMP 2

int do_efuse(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0 ;
	int i, j;
	char addr[EFUSE_BYTES];
	char *title;
	char *op;	
	char *s;
	char *end;
	int action = -1;	
		
	if(argc < 2){
		cmd_usage(cmdtp);
		return -1;
	}
	if(strncmp(argv[1], "read", 4) == 0)
		action=EFUSE_READ;
	else if(strncmp(argv[1], "write", 5) == 0)
		action=EFUSE_WRITE;
#ifdef CONFIG_EFUSE_DUMP
	else if(strcmp(argv[1], "dump") == 0)
		action=EFUSE_DUMP;
#endif			
	else{
		printf("%s arg error\n", argv[1]);
		return -1;
	}
				
	// efuse read/write	
	if(action == EFUSE_READ || action==EFUSE_WRITE){		
		title = argv[2];
		for(i=0; i<efuse_info_cnt; i++){
			if(strcmp(title, efuse_info[i].title) == 0)
				break;
		}
		if(i>=efuse_info_cnt){
			printf("%s arg error. \n", title);
			return -1;
		}	
					
		// efuse read
		if(action == EFUSE_READ){
			op = (char*)efuse_read_usr(i);
			printf("%s is: ", title);
			for(j=0; j<efuse_info[i].data_len; j++)
				printf(":%02x", op[j]);
			printf("\n");
		}
		// efuse write
		else{			
			if(!(efuse_info[i].we)){
				printf("%s write unsupport now. \n", title);
				return -1;
			}
			if(argc<4){
				printf("arg count error\n");
				return -1;
			}
			memset(addr, 0, sizeof(addr));	
			s=argv[3];
			for(j=0; j<efuse_info[i].data_len; j++){
				addr[j] = s ? simple_strtoul(s, &end, 16) : 0;
				if (s)
					s = (*end) ? end+1 : end;
			}
			if(efuse_write_usr(i, addr)){
				printf("error: efuse has written.\n");
				return -1;
			}
			else
				printf("%s written done.\n", efuse_info[i].title);			
		}
	}
	// efuse dump
#ifdef CONFIG_EFUSE_DUMP
	else if(action == EFUSE_DUMP){
		op = efuse_dump();		
		printf("Raw efuse data: \n");
		for(i=0; i<EFUSE_BYTES; i++){
			if(i%16 == 0)
				printf("%03x:  ", i);
			printf("%02x ", op[i]);
			if(i%16 == 15)
				printf("\n");
		}	
		printf("efuse raw data dump finish \n");
	}
#endif	
	else{
		printf("arg error\n");
		return -1;	
	}
		
	return ret ;		
}

U_BOOT_CMD(
	efuse,	4,	1,	do_efuse,
	"efuse licence/mac/hdcp/usid read/write or dump raw efuse data commands",
	"[read/write] [licence/mac/hdc/usid] [mem_addr]\n"
	"	   [read/wirte] para read ; write ;\n"
	"				read need not mem_addr;write need\n"
	"				read to get efuse context\n"
	"				write to write efuse\n"
	"	   [mem_addr] usr do \n"
	"efuse [dump]\n"
	"	   dump raw efuse data\n"
);

/****************************************************/