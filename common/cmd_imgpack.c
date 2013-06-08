/*
 * Command for img pack.
 *
 * Copyright (C) 2012 Amlogic.
 * Elvis Yu <elvis.yu@amlogic.com>
 */


#include <common.h>
#include <command.h>
#include <asm/types.h>


#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/


struct pack_header{
	__u32	magic;	/* Image Header Magic Number	*/
	__u32	hcrc;	/* Image Header CRC Checksum	*/
	__u32	size;	/* Image Data Size		*/
	__u32	start;	/* Data	 Load  Address		*/
	__u32	end;		/* Entry Point Address		*/
	__u32	next;	/* Image Creation Timestamp	*/
	__u32	dcrc;	/* Image Data CRC Checksum	*/
	__u8	index;		/* Operating System		*/
	__u8	nums;	/* CPU architecture		*/
	__u8	type;	/* Image Type			*/
	__u8	comp;	/* Compression Type		*/
	__u8	name[IH_NMLEN];	/* Image Name		*/
} __attribute__ ((packed));


static int do_unpackimg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	void *addr;
	__u32 pos;
	struct pack_header *pack_header_p;
	char env_name[IH_NMLEN*2];
	char env_data[IH_NMLEN*2];
	
	if (argc < 2)
	{
		cmd_usage(cmdtp);
		return -1;
	}

	addr = (void *)simple_strtoul (argv[1], NULL, 16);
	pos = 0;
	while(1)
	{
		pack_header_p = (struct pack_header *)(addr + pos);
		if(pack_header_p->magic != IH_MAGIC)
		{
			printf("wrong pack img!\n");
			return -1;
		}

		sprintf(env_name, "%s_offset", pack_header_p->name);
		sprintf(env_data, "0x%x", (unsigned int)(addr+pack_header_p->start));
		setenv(env_name, env_data);
		
		sprintf(env_name, "%s_size", pack_header_p->name);
		sprintf(env_data, "0x%x", pack_header_p->size);
		setenv(env_name, env_data);
		if(pack_header_p->next == 0)
		{
			break;
		}
		else
		{
			pos = pack_header_p->next;
		}
	}
	return 0;
}

U_BOOT_CMD(
	unpackimg,	2,	0,	do_unpackimg,
	"unpack imgpack to single",
	"unpackimg <addr>		- open a imgpack in addr\n"
);

static int do_get_img_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	void *addr;
	__u32 pos;
	struct pack_header *pack_header_p;
	char env_name[IH_NMLEN*2];
	char env_data[IH_NMLEN*2];
	
	if (argc < 3)
	{
		cmd_usage(cmdtp);
		return -1;
	}
	//printf("############ do_get_img_size #############\n");
	addr = simple_strtoul (argv[1], NULL, 16);
	pos = 0;
	while(1)
	{
		pack_header_p = (struct pack_header *)(addr + pos);
		if(pack_header_p->magic != IH_MAGIC)
		{
			printf("wrong pack img!\n");
			return -1;
		}

		
		if(pack_header_p->next == 0)
		{
			sprintf(env_name, "%s", argv[2]);
			sprintf(env_data, "0x%x", pack_header_p->start + pack_header_p->size);
			setenv(env_name, env_data);
			saveenv();
			break;
		}
		else
		{
			pos = pack_header_p->next;
		}
	}
	return 0;
}

U_BOOT_CMD(
	get_img_size,	3,	0,	do_get_img_size,
	"get img size and save the result as a environment variable",
	"get_img_size <addr> <env>  - check the img in addr and save the total size to env\n"
);


