/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Michael Zaidman, Kodak, michael.zaidman@kodak.com
 * post_word_{load|store} cleanup.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _SYS_TEST_H
#define _SYS_TEST_H

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_SYS_TEST

#define SYSTEST_PASSED		1
#define SYSTEST_FAILED		0
#define NAMESIZE 16

struct sys_test {
	char *name;
	char *cmd;
	char *desc;	
	int (*test) (int argc, char* argv[]);
	int (*init) (void);		
};

struct sys_logdev{
		char name[NAMESIZE];
		int (*init)();
		void (*puts)(const char *s, int level);
};

extern struct sys_test systest_list[];
extern unsigned int systest_list_size;

int systest_run(char *name, int argc, char* const argv[]);
int systest_info(char *name);
int systest_log(char * message, int level);

int drv_logdev_init(void);
int systest_log(char *message, int level);

#endif /* CONFIG_SYS_TEST */
#endif /* _SYS_TEST_H */
