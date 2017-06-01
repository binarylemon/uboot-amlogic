/*
 * common/cmd_rsvmem.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_CMD_RSVMEM

#define RSVMEM_DEBUG_ENABLE 0
#if RSVMEM_DEBUG_ENABLE
#define rsvmem_dbg(fmt...)	printf("[rsvmem] "fmt)
#else
#define rsvmem_dbg(fmt...)
#endif
#define rsvmem_info(fmt...)	printf("[rsvmem] "fmt)
#define rsvmem_err(fmt...)	printf("[rsvmem] "fmt)


#define MESON_SECURE_FLAG_REG 0xC11081F0
static bool meson_secure_enabled(void)
{
	if (readl(MESON_SECURE_FLAG_REG))
		return true;
	else
		return false;
}

static int do_rsvmem_check(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	char cmdbuf[128];
	char *fdtaddr = NULL;
	int ret = 0;

	rsvmem_info("reserved memory check\n");
	if (!meson_secure_enabled())
		return 0;

	rsvmem_info("secure enabled!\n");
	fdtaddr = getenv("fdtaddr");
	if (fdtaddr == NULL) {
		rsvmem_err("get fdtaddr NULL!\n");
		return -1;
	}

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "fdt addr %s;", fdtaddr);
	rsvmem_dbg("CMD: %s\n", cmdbuf);
	ret = run_command(cmdbuf, 0);
	if (ret != 0 ) {
		rsvmem_err("fdt addr error.\n");
		return -2;
	}

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "fdt set /reserved-memory/linux,secos status okay;");
	rsvmem_dbg("CMD: %s\n", cmdbuf);
	ret = run_command(cmdbuf, 0);
	if (ret != 0 ) {
		rsvmem_err("bl32 reserved memory set status error.\n");
		return -3;
	}

	return ret;
}

static cmd_tbl_t cmd_rsvmem_sub[] = {
	U_BOOT_CMD_MKENT(check, 2, 0, do_rsvmem_check, "", ""),
};

static int do_rsvmem(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	/* Strip off leading 'rsvmem' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_rsvmem_sub[0], ARRAY_SIZE(cmd_rsvmem_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return -1;
}

U_BOOT_CMD(
		rsvmem, 2, 0,	do_rsvmem,
		"reserve memory",
		"check                   - check reserved memory\n"
	  );
#endif
