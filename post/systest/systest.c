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

#include <common.h>
#include <systest.h>

DECLARE_GLOBAL_DATA_PTR;
#define SYSTEST_MAX_NUMBER		32

extern char systest_info_line[];

//====================================================================================
static int systest_info_single (struct sys_test *test, int full)
{	
	if (full)
		sprintf (systest_info_line, "%s - %s\n""  %s\n", test->cmd, test->name, test->desc);		
	else
		sprintf (systest_info_line,"  %-15s - %s\n", test->cmd, test->name);		
	
	systest_log(systest_info_line, SYSTEST_INFO_L3);

	return 0;
	
}
//====================================================================================
int systest_info (char *name)
{
	unsigned int i;

	if (name == NULL) {
		for (i = 0; i < systest_list_size; i++) {
			systest_info_single (systest_list + i, 0);
		}
		return 0;
	}
	else {
		for (i = 0; i < systest_list_size; i++) {
			if (strcmp (systest_list[i].cmd, name) == 0)
				break;
		}

		if (i < systest_list_size) {
			return systest_info_single (systest_list + i, 1);
		} else {
			return -1;
		}
	}
}
//====================================================================================
static int systest_run_single (struct sys_test *test,	int argc, char* argv[])
{
	if(!test || !(test->test))
		return -1;
	
	if(test->init)
		test->init();		
	return test->test(argc, argv);
	
}
//====================================================================================
int systest_run (char *name, int argc, char* const argv[])
{
	int i=0;
	if(name == NULL){
		for(i=0; i<systest_list_size; i++)
			systest_run_single(systest_list+i, 0, NULL);		
	}
	else{		
		for (i = 0; i < systest_list_size; i++) {			
			if (strcmp (systest_list[i].cmd, name) == 0)
				break;
		}
		if (i < systest_list_size) 
			return(systest_run_single (systest_list + i, argc,	argv));
		else{			
			return -1;		
		}
	}	
	return 0;
}

//====================================================================================
int test_isnum(const char *s)
{
	while((*s!=0) && (*s != ' ')){
		if((*s >= '0') && (*s <= '9'))
			continue;
		else if((*s >= 'a') && (*s <= 'f'))
			continue;
		else if((*s >= 'A') && (*s <= 'F'))
			continue;
		else if(*s == 'x')
			continue;
		else
			return -1;
	}
	return 0;
}