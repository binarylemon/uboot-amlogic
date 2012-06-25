#include <common.h>
#include "usb_pcd.h"

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

int usb_run_command (const char *cmd, char* buff)
{
	int ret = -1, flag = 0;
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
		u32 addr = 0;
		addr = *((u32*)(&cmd[60]));
		strcpy(buff, "okay");
		usb_bootm(addr);
		strcpy(buff, "fail");
		return -1;
	}
	else if(strncmp(cmd,"cmd_in_mem",(sizeof("cmd_in_mem")-1)) == 0){
		int argc;
		char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
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
	else if(strncmp(cmd,"efuse",(sizeof("efuse")-1)) == 0){
		int argc;
		char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
		/* Extract arguments */
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}
		if(cmd_efuse(argc, argv, buff))
		{
			strcpy(buff, "fail");
			return -1;
		}
	}
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
	return 0;
}




