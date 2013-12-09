#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <nand.h>
#include <asm/arch/nand.h>
#include <common.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <div64.h>
#include <linux/err.h>
#include<partition_table.h>

static inline int isstring(char *p)
{
	char *endptr = p;
	while (*endptr != '\0') {
		if (!(((*endptr >= '0') && (*endptr <= '9')) 
			|| ((*endptr >= 'a') && (*endptr <= 'f'))
			|| ((*endptr >= 'A') && (*endptr <= 'F'))
			|| (*endptr == 'x') || (*endptr == 'X')))
			return 1;
		endptr++;
	}

	return 0;
}

static inline int str2long(char *p, ulong *num)
{
	char *endptr;
	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}
static inline int str2longlong(char *p, unsigned long long *num)
{
	char *endptr;
    
	*num = simple_strtoull(p, &endptr, 16);
	if(*endptr!='\0')
	{
	    switch(*endptr)
	    {
	        case 'g':
	        case 'G':
	            *num<<=10;
	        case 'm':
	        case 'M':
	            *num<<=10;
	        case 'k':
	        case 'K':
	            *num<<=10;
	            endptr++;
	            break;
	    }
	}
	
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

static int get_off_size(int argc, char *argv[],  loff_t *off, loff_t *size)
{
	if (argc >= 1) {
		if (!(str2longlong(argv[0], (unsigned long long*)off))) {
			store_msg("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
		*size = 0;
	}

	if (argc >= 2) {
		if (!(str2longlong(argv[1], (unsigned long long *)size))) {
			store_msg("'%s' is not a number\n", argv[1]);
			return -1;
		}
	}else{
		*size = 0;
	} 

	store_dbg("offset 0x%llx, size 0x%llx", *off, *size);

	return 0;
}

int do_store(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, init_flag=0,dev, ret = 0;
	uint64_t addr;	
	loff_t off=0, size=0;	
	char *cmd, *s, *area;
	char	str[128];
    unsigned char *tmp_buf= NULL;
    
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	
	if (strcmp(cmd, "erase") == 0){

		area = argv[2];
		
		if(strcmp(area, "boot") == 0){
			if(POR_NAND_BOOT()){
				off =  simple_strtoul(argv[3], NULL, 16);
				size =  simple_strtoul(argv[4], NULL, 16);
				store_dbg("NAND BOOT,erase uboot : %s %d  off =%llx ,size=%llx",__func__,__LINE__, off, size);

				ret = run_command("amlnf  deverase boot 0",0);
				if(ret != 0){
					store_msg("nand cmd %s failed ",cmd);
					return -1;
				}
				return ret;
			}else if(POR_SPI_BOOT()){
				off =  simple_strtoul(argv[3], NULL, 16);
				size =  simple_strtoul(argv[4], NULL, 16);

				store_dbg("SPI BOOT,erase uboot :  %s %d  off =%llx ,size=%llx",__func__,__LINE__,off,size);
				
				ret = run_command("sf probe 2",0);
				if(ret != 0){
					store_msg("nand cmd %s failed",cmd);
					return -1;
				}
				ret = run_command("sf erase 0 100000",0);
				if(ret != 0){
					store_msg("nand cmd %s failed",cmd);
					return -1;
				}
				return ret;
			}else if(POR_EMMC_BOOT()){
				off =  simple_strtoul(argv[3], NULL, 16);
				size =  simple_strtoul(argv[4], NULL, 16);

				store_dbg("MMC BOOT,erase uboot :  %s %d  off =%llx ,size=%llx",__func__,__LINE__,off,size);
				
				sprintf(str, "mmc  erase bootloader");
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("mmc cmd %s failed",cmd);
					return -1;
				}
				return ret;
			}else{			
				store_dbg("CARD BOOT,erase uboot :  %s %d  off =%llx ,size=%llx",__func__,__LINE__,off,size);
				return 0;
			}
		}
		else if(strcmp(area, "data") == 0){
			
			if(POR_NAND_BOOT()){
				store_dbg("NAND BOOT,erase data : %s %d  off =%llx ,size=%llx",__func__,__LINE__, off, size);

				ret = run_command("amlnf  deverase data 0",0);
				if(ret != 0){
					store_msg("nand cmd %s failed ",cmd);
					return -1;
				}

				ret = run_command("amlnf  deverase code 0",0);
				if(ret != 0){
					store_msg("nand cmd %s failed ",cmd);
					return -1;
				}
				return ret;
			}
			else if(POR_SPI_BOOT()){
				store_dbg("SPI BOOT,erase data : %s %d  off =%llx ,size=%llx",__func__,__LINE__, off, size);

				if(device_boot_flag == SPI_NAND_FLAG){		
					store_dbg("spi+nand , %s %d ",__func__,__LINE__);
					ret = run_command("amlnf  deverase data 0",0);
					if(ret != 0){
						store_msg("nand cmd %s failed ",cmd);
						return -1;
					}

					ret = run_command("amlnf  deverase code 0",0);
					if(ret != 0){
						store_msg("nand cmd %s failed ",cmd);
						return -1;
					}
				}
				if(device_boot_flag == SPI_EMMC_FLAG){
					store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
					off = size =0;
					ret = run_command("mmc  erase  1",0); // whole
					if(ret != 0){
						store_msg("mmc cmd %s failed ",cmd);
						return -1;
					}
				}

				return ret;
			} 
			else if(POR_EMMC_BOOT()){
				store_dbg("MMC BOOT,erase data : %s %d  off =%llx ,size=%llx",__func__,__LINE__, off, size);
				off = size =0;
				ret = run_command("mmc erase  1",0); //whole
				if(ret != 0){
					store_msg("mmc cmd %s failed ",cmd);
					return -1;
				}
				return ret;
			}else{			
				store_dbg("CARD BOOT,erase data : %s %d  off =%llx ,size=%llx",__func__,__LINE__, off, size);
				return 0;
			}
		}
		else {
			goto usage;
		}
		
	}
	else if(strcmp(cmd, "read") == 0){
		if (argc < 6)
			goto usage;
		
		s = argv[2];	
		addr = (ulong)simple_strtoul(argv[3], NULL, 16);
		if (get_off_size(argc - 4, argv + 4, &off, &size) != 0)
			goto usage;
		
		store_dbg("addr = %llx off= 0x%llx  size=0x%llx",addr,off,size);
		if((POR_NAND_BOOT())){	
			sprintf(str, "amlnf  read_byte %s 0x%llx  0x%llx  0x%llx",s, addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed ",cmd);
				return -1;
			}
			return ret;
		}else if((POR_SPI_BOOT())){
			if(device_boot_flag == SPI_NAND_FLAG){
				sprintf(str, "amlnf  read_byte %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
				store_dbg("command:	%s\n", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("nand cmd %s failed \n",cmd);
					return -1;
				}
			}
			if(device_boot_flag == SPI_EMMC_FLAG){
				store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
				sprintf(str, "mmc  read %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
				store_dbg("command:	%s\n", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("mmc cmd %s failed \n",cmd);
					return -1;
				}
			}
			return ret;
		}
		else if(POR_EMMC_BOOT()) {
			store_dbg("MMC BOOT, %s %d \n",__func__,__LINE__);
			sprintf(str, "mmc  read %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
			store_dbg("command:	%s\n", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed \n",cmd);
				return -1;
			}
			return ret;
		}else{
			store_dbg("CARD BOOT, %s %d ",__func__,__LINE__);
			
			return 0;
		}
	}
	else if(strcmp(cmd, "write") == 0){
		if (argc < 6)
			goto usage;
		s = argv[2];	
		addr = (ulong)simple_strtoul(argv[3], NULL, 16);
		if (get_off_size(argc - 4, argv + 4, &off, &size) != 0)
			goto usage;
		if((POR_NAND_BOOT())){	

			sprintf(str, "amlnf  write_byte %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed ",cmd);
				return -1;
			}
		}else if((POR_SPI_BOOT())){
			if(device_boot_flag == SPI_NAND_FLAG){
				store_dbg("spi+nand , %s %d ",__func__,__LINE__);
				sprintf(str, "amlnf  write_byte %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
				store_dbg("command:	%s", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("nand cmd %s failed \n",cmd);
					return -1;
				}
			}
			if(device_boot_flag == SPI_EMMC_FLAG){
				store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
				sprintf(str, "mmc  write %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
				store_dbg("command: %s\n", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("mmc cmd %s failed \n",cmd);
					return -1;
				}
			}
		}
		else if(POR_EMMC_BOOT())  {
			store_dbg("MMC BOOT, %s %d \n",__func__,__LINE__);
			sprintf(str, "mmc  write %s 0x%llx  0x%llx  0x%llx", s, addr, off, size);
			store_dbg("command: %s\n", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed \n",cmd);
				return -1;
			}
			return ret;
		}else{
			store_dbg("CARD BOOT, %s %d ",__func__,__LINE__);
			return 0;
		}
		return ret;
	}
	else if(strcmp(cmd, "rom_write") == 0){
		if (argc < 5)
			goto usage;
		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		if (get_off_size(argc - 3, argv + 3, &off, &size) != 0)
			goto usage;
		if(POR_NAND_BOOT()){
			sprintf(str, "amlnf  rom_write  0x%llx  0x%llx  0x%llx",  addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}else if (POR_SPI_BOOT()){
			ret = run_command("sf  probe 2",0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			sprintf(str, "sf  erase  0x%llx  0x%llx ", off, size);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			sprintf(str, "sf  write 0x%llx  0x%llx  0x%llx ",addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}
		else if(POR_EMMC_BOOT()){
			store_dbg("MMC BOOT, %s %d \n",__func__,__LINE__);
			tmp_buf= (unsigned char *)addr;
			//modify the 55 AA info for emmc uboot
			tmp_buf[510]=0;
			tmp_buf[511]=0;
			sprintf(str, "mmc  write bootloader 0x%llx  0x%llx  0x%llx", addr, off, size);
			store_dbg("command: %s\n", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed \n",cmd);
				return -1;
			}
			return ret;
		}else{
			store_dbg("CARD BOOT, %s %d",__func__,__LINE__);
			return 0;
		}

	}
	else if(strcmp(cmd, "rom_read") == 0){
		if (argc < 5)
			goto usage;
		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		if (get_off_size(argc - 3, argv + 3, &off, &size) != 0)
			goto usage;
		if(POR_NAND_BOOT()){
			sprintf(str, "amlnf  rom_read  0x%llx  0x%llx  0x%llx",  addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}else if (POR_SPI_BOOT()){
			ret = run_command("sf  probe 2",0);
			if(ret != 0){
				return -1;
			}
			sprintf(str, "sf  read 0x%llx  0x%llx  0x%llx ",addr, off, size);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}else if (POR_EMMC_BOOT()){
			store_dbg("MMC BOOT, %s %d \n",__func__,__LINE__);
			sprintf(str, "mmc  read bootloader 0x%llx  0x%llx  0x%llx", addr, off, size);
			store_dbg("command: %s\n", str);
			tmp_buf= (unsigned char *)addr;
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed \n",cmd);
				return -1;
			}
		    tmp_buf[510]=0x55;
			tmp_buf[511]=0xaa;
			return ret;
		}else{
			store_dbg("CARD BOOT, %s %d ",__func__,__LINE__);
			return 0;
		}

	}
	else if (strcmp(cmd, "rom_protect") == 0){
		if (argc < 3)
			goto usage;
		
		area = argv[2];
		if(POR_NAND_BOOT()){	
			sprintf(str, "amlnf  rom_protect  %s", area);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}
	}
	else if (strcmp(cmd, "scrub") == 0){	
		off = (ulong)simple_strtoul(argv[2], NULL, 16);
		if(off == 0){
			sprintf(str, "amlnf  erase %d", off);
		}else{
			off = 0x0;
			sprintf(str, "amlnf  scrub %d", off);
		}
		if((POR_NAND_BOOT()) ){	
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
		}else if(POR_SPI_BOOT()){
			if(device_boot_flag == SPI_NAND_FLAG){
				store_dbg("spi+nand , %s %d ",__func__,__LINE__);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("nand cmd %s failed",cmd);
					return -1;
				}
			}
			if(device_boot_flag == SPI_EMMC_FLAG){
				store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
				ret = run_command("mmc erase whole",0);
				if(ret != 0){
					store_msg("mmc cmd %s failed \n",cmd);
					return -1;
				}
			}
		}
		return ret;
	}
	else if(strcmp(cmd, "init") == 0){
		
		init_flag = (int)simple_strtoul(argv[2], NULL, 16);
		store_dbg("init_flag %d",init_flag);
		if(POR_NAND_BOOT()){	
			sprintf(str, "amlnf  init  %d ",init_flag);
			printf("command:	%s\n", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed ",cmd);
				return -1;
			}
			return ret;
		}else if(POR_SPI_BOOT()){

			if(device_boot_flag == -1){
				ret = run_command("sf probe 2", 0);
				if(ret){
					store_msg(" cmd %s failed \n",cmd);
					return -1;
				}
				sprintf(str, "amlnf  init  %d ",init_flag);
				store_dbg("command:	%s", str);
				ret = run_command(str, 0);
				if(ret < 0){
					store_msg("nand cmd %s failed \n",cmd);
					device_boot_flag = SPI_EMMC_FLAG;
					store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
					ret = run_command("mmcinfo 1", 0);
					if(ret != 0){
						store_msg("mmc cmd %s failed \n",cmd);
						return -2;
					}
					return 0;
				}
				device_boot_flag = SPI_NAND_FLAG;		
				return 0;
			}
			
			if(device_boot_flag == SPI_NAND_FLAG){
				store_dbg("spi+nand , %s %d ",__func__,__LINE__);
				sprintf(str, "amlnf  init  %d ",init_flag);
				store_dbg("command:	%s", str);
				ret = run_command(str, 0);
			}
			if(device_boot_flag == SPI_EMMC_FLAG){
				store_dbg("spi+mmc , %s %d ",__func__,__LINE__);
				ret = run_command("mmcinfo 1", 0);

                if (init_flag = 2) // need erase whole emmc/tsd
                    ret = run_command("mmc erase whole", 0);
			}
			
			if(ret != 0){
				store_msg("cmd %s failed \n",cmd);
				return -1;
			}
			
			return ret;
		}
		else if(POR_EMMC_BOOT()){
			store_dbg("MMC BOOT, %s %d \n",__func__,__LINE__);
			ret = run_command("mmcinfo 1", 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed \n",cmd);
				return -1;
			}
			return ret;
		}else{
			store_dbg("CARD BOOT, %s %d",__func__,__LINE__);
			return 0;
		}
	}
	else if(strcmp(cmd, "size") == 0){

		if (argc < 4)
			goto usage;
		
		s = argv[2];
		addr = (ulong)simple_strtoul(argv[3], NULL, 16);
		if(POR_NAND_BOOT()){	
			sprintf(str, "amlnf  size  %s %llx",s,addr);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}else if(POR_SPI_BOOT()){
			if(device_boot_flag == SPI_NAND_FLAG){
				sprintf(str, "amlnf  size  %s %llx",s,addr);
				store_dbg("command:	%s", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("nand cmd %s failed",cmd);
					return -1;
				}
			}
			if(device_boot_flag == SPI_EMMC_FLAG){
				store_dbg("MMC , %s %d ",__func__,__LINE__);
				sprintf(str, "mmc  size  %s %llx",s,addr);
				store_dbg("command:	%s", str);
				ret = run_command(str, 0);
				if(ret != 0){
					store_msg("mmc cmd %s failed",cmd);
					return -1;
				}
			}
			return ret;
		}
		else if(POR_EMMC_BOOT()){
			store_dbg("MMC , %s %d ",__func__,__LINE__);
			sprintf(str, "mmc  size  %s %llx",s,addr);
			store_dbg("command:	%s", str);
			ret = run_command(str, 0);
			if(ret != 0){
				store_msg("mmc cmd %s failed",cmd);
				return -1;
			}
			return ret;
		}
		else if(POR_CARD_BOOT()){
			store_dbg("CARD BOOT , %s %d ",__func__,__LINE__);
			return 0;
		}
	}
	else if(strcmp(cmd, "exit") == 0){
		
		if(POR_NAND_BOOT()){	
			ret = run_command("amlnf exit", 0);
			if(ret != 0){
				store_msg("nand cmd %s failed",cmd);
				return -1;
			}
		}
		return 0;
	}
	else{
		goto usage;
	}

	return ret;
	
usage:
	cmd_usage(cmdtp);
	return 1;

}


U_BOOT_CMD(store, CONFIG_SYS_MAXARGS, 1, do_store,
	"STORE sub-system",
	"store init flag\n"
	"store read name addr off|partition size\n"
	"    read 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"store write name addr off|partition size\n"
	"    write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"store rom_write add off size.\n"
	"	write uboot to the boot device\n"
	"store erase boot/data: \n"
	"	erase the area which is uboot or data \n"
	"store scrub off|partition size\n"
	"	scrub the area from offset and size \n"
);

