/*
 * Command for suspend.
 *
 * Copyright (C) 2012 Amlogic.
 * Elvis Yu <elvis.yu@amlogic.com>
 */

#include <common.h>
#include <command.h>

extern void meson_pm_suspend(void);

static int do_suspend (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	meson_pm_suspend();
	return 0;
}



U_BOOT_CMD(
	suspend,	1,	0,	do_suspend,
	"suspend",
	"/N\n"
	"This command will into suspend\n"
);

