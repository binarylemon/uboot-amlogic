/*
 * Command for Amlogic Custom.
 *
 * Copyright (C) 2012 Amlogic.
 * Elvis Yu <elvis.yu@amlogic.com>
 */

#include <common.h>
#include <command.h>


static int do_getkey (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static int key_enable = 0;
	if(!key_enable)
	{
		key_init();
		key_enable = 1;
	}
	return !get_key();
}


U_BOOT_CMD(
	getkey,	1,	0,	do_getkey,
	"get POWER key",
	"/N\n"
	"This command will get POWER key'\n"
);


static int do_ac_online (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	static int enable = 0;
	if(!enable)
	{
		ac_online_init();
		enable = 1;
	}

	return !is_ac_online();
}


U_BOOT_CMD(
	ac_online,	1,	0,	do_ac_online,
	"get ac adapter online",
	"/N\n"
	"This command will get ac adapter online'\n"
);


static int do_poweroff (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	power_off();
	return 0;
}


U_BOOT_CMD(
	poweroff,	1,	0,	do_poweroff,
	"system power off",
	"/N\n"
	"This command will let system power off'\n"
);

static int do_get_batcap (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	setenv("battery_cap", "50");
	return 0;
}


U_BOOT_CMD(
	get_batcap,	1,	0,	do_get_batcap,
	"get battery capability",
	"/N\n"
	"This command will get battery capability\n"
	"capability will set to 'battery_cap'\n"
);

