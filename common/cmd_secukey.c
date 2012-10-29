/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

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
char listkey[1024];
int inited=0;
extern ssize_t uboot_key_init();
extern ssize_t uboot_get_keylist(char *keyname);
extern ssize_t uboot_key_read(char *keyname, char *keydata);
extern ssize_t uboot_key_write(char *keyname, char *keydata);
extern int nandkey_provider_register();
extern int key_set_version(char *device);
/* ------------------------------------------------------------------------- */
int do_secukey(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, ret = 0,error;
	char *cmd;
	char *name;
	char *data;
	/* at least two arguments please */
	if (argc < 2)
		goto usage;
	cmd = argv[1];
	//first read nand key
	if(argc==2)
	{
		if(inited){
			if(!strcmp(cmd,"list"))
			{
				error=uboot_get_keylist(listkey);
				if (error>=0){
					printk("the key name list are:\n%s",listkey);
					return 0;
				}
				else{
					printk("key list error!!check the key  name first!!\n");
					return -1;
				}
					
			}
		} else if (inited==0)
		{
			if (!strcmp(cmd,"nand") ) {
				
				error=uboot_key_init();
				if(error>=0){
					error=nandkey_provider_register();
					if(error>=0){
						 error=key_set_version(cmd);
						 if(error>=0){
							printk("init key ok!!\n");
							inited=1;
							return 0;
						}
					}
				}	
				else
					printk("init error\n");
			}
			else {
				printk("invalid device!!\n");
				return -1;
			}
		}
		else{
			printk("should be inited first!\n");
			return -1;
		}
	}
	if (inited){
		printk("%s,%d\n",__func__,__LINE__);
		if (argc > 2&&argc<5){
			if(!strcmp(cmd,"read")){
				if (argc>3)
					goto usage;
				name=argv[2];
				strcpy(namebuf,name);
				error=uboot_key_read(namebuf, databuf);
				if(error>=0){
					printk("the key name is :%s\n",namebuf);
					printk("the key data is :%s\n",databuf);
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
				strcpy(databuf,data);
				printk("right here!!!\n");
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
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(secukey, CONFIG_SYS_MAXARGS, 1, do_secukey,
	"NAND KEY sub-system",
	"list - show available NAND key name\n"
	"secukey  device - init key in device\n"
	"secukey write keyname data - wirte key data to nand\n"
	"secukey read keyname - read the key data\n"	
);

