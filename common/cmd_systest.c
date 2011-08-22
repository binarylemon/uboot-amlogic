/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <systest.h>

extern char systest_info_line[];
int do_systest (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int i;
	if(argc == 1){
		cmd_usage(cmdtp);
		return 1;
	}
	
	if(drv_logdev_init() != 0)	
		return 1;
		
	if ( strcmp (argv[1], "info") == 0) {
		/* List test info */
		if (argc == 2) {
			systest_log ("Available tests:\n", SYSTEST_INFO_L3);
			systest_info (NULL);
			systest_log("Use 'systest [<test1> [<test2> ...]]'"
					" to get more info.\n", SYSTEST_INFO_L3);
			systest_log("Use 'systest run <test> '"
					" to run test.\n", SYSTEST_INFO_L3);
		} else {
			for (i = 2; i < argc; i++) {
			    if (systest_info (argv[i]) != 0){
					sprintf (systest_info_line, "%s - no such test\n", argv[i]);
					systest_log(systest_info_line, SYSTEST_INFO_L3);
				}
			}
		}
	} else if(strcmp(argv[1], "run") == 0){
		/* Run tests */
		if (argc == 2) {
			systest_run (NULL,0, NULL);
		} else {			
			    if (systest_run (argv[2], argc-2, argv+2) < 0){
					sprintf (systest_info_line,"%s - unable to execute the test\n",	argv[i]);
					systest_log(systest_info_line, SYSTEST_INFO_L3);					
				}				
		}
	}

	return 0;
}
/***************************************************/

U_BOOT_CMD(
	systest,	CONFIG_SYS_MAXARGS,	1,	do_systest,
	"perform factory or system test",
	"systest info\n"
	     "    - print list of available tests\n"
	"systest info [test1 [test2]]\n"
	"         - print information about specified tests\n"
	"systest run - run all available tests\n"
	"systest run [test1 [test2]]\n"
	"         - run specified tests"
);
