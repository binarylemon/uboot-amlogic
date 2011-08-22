/*
 * (C) Copyright 2002-2007
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * Code used from linux/kernel/printk.c
 * Copyright (C) 1991, 1992  Linus Torvalds
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
 *
 * Comments:
 *
 * After relocating the code, the environment variable "loglevel" is
 * copied to console_loglevel.  The functionality is similar to the
 * handling in the Linux kernel, i.e. messages logged with a priority
 * less than console_loglevel are also output to stdout.
 *
 * If you want messages with the default level (e.g. POST messages) to
 * appear on stdout also, make sure the environment variable
 * "loglevel" is set at boot time to a number higher than
 * default_message_loglevel below.
 */

/*
 * Logbuffer handling routines
 */

#include <common.h>
#include <command.h>
#include <systest.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_SYSTEST_DEV_NUM 20
static struct sys_logdev logdev[MAX_SYSTEST_DEV_NUM];
int dev_pos=0;

#define MAX_TOKEN_LEN 128
char systest_info_line[MAX_TOKEN_LEN];

//================================================================
static void logdev_register(struct sys_logdev *p)
{
	int i;	
	if(p){		
		for(i=0; i<MAX_SYSTEST_DEV_NUM; i++){
			if(strcmp(logdev[i].name, p->name) == 0)
				break;		
		}
		if(i == MAX_SYSTEST_DEV_NUM){
			strcpy(logdev[dev_pos].name, p->name);
			logdev[dev_pos].init = p->init;
			logdev[dev_pos++].puts = p->puts;			
		}
	}	
}
//================================================================
static void logdev_default_puts(char* message, int level)
{
	serial_puts(message);	
}
//================================================================
static void logdev_default_init(void)
{
	struct sys_logdev dev;
	strcpy(dev.name, "serial");
	dev.init = NULL;
	dev.puts = logdev_default_puts;	

	logdev_register(&dev);
}
//================================================================
int drv_logdev_init (void)
{
	// has initialed
	if(dev_pos != 0)
		return 0;
		
	logdev_default_init();
	
#ifdef CONFIG_SYS_TEST_LOGDEV_SDCARD
	logdev_sdcard_inti();
#endif	
	
	int i=0;
	for(i=0; i<dev_pos; i++){
		if(logdev[i].init)
			logdev[i].init();
	}
	return 0;		
}
//================================================================
int systest_log (char *message, int level)
{
	int i=0;	
	for(i=0; i<dev_pos; i++){
			if(logdev[i].puts)
				logdev[i].puts(message, level);	
	}
	return 0;		
}
