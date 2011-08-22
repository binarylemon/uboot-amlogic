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
 *
 * Be sure to mark tests to be run before relocation as such with the
 * CONFIG_SYS_POST_PREREL flag so that logging is done correctly if the
 * logbuffer support is enabled.
 */

#include <common.h>
#include <systest.h>

extern int memory_systest (int argc, char* argv[]);
extern int l1cache_systest (int argc, char* argv[]);
extern int l2cache_systest (int argc, char* argv[]);
extern int bist_systest (int argc, char* argv[]);
extern int pll_systest (int argc, char* argv[]);
extern int rtc_systest (int argc, char* argv[]);
extern int nand_systest (int argc, char* argv[]);
extern int sdcard_systest (int argc, char* argv[]);
extern int i2c_systest (int argc, char* argv[]);
extern int adc_systest (int argc, char* argv[]);

extern int adc_systest_init(void);

struct sys_test systest_list[] =
{
#ifdef CONFIG_SYS_TEST_MEM
#if defined(CONFIG_TEST_MEM)
	CONFIG_TEST_MEM,
#else	
{
	"Memory test",
	"memory",
	"This test checks RAM.",
	&memory_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_L1CACHE
#if defined(CONFIG_TEST_L1CACHE)
	CONFIG_TEST_L1CACHE,
#else
{
	"L1-Cache test",
	"l1cache",
	"This test checks l1-cache.",
	&l1cache_systest,
	NULL
},
#endif		
#endif

#ifdef CONFIG_SYS_TEST_L1CACHE
#if defined(CONFIG_TEST_L2CACHE)
	CONFIG_TEST_L2CACHE,
#else
{
	"L2-Cache test",
	"l2cache",
	"This test checks l2cache",
	&l2cache_systest,
	NULL
},	
#endif
#endif

#ifdef CONFIG_SYS_TEST_BIST
#if defined(CONFIG_TEST_BIST)
	CONFIG_TEST_BIST,
#else
{
	"BIST test",
	"bist",
	"This test checks bist which support l1cache/l2cache bist",
	&bist_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_PLL
#if defined(CONFIG_TEST_PLL)
	CONFIG_TEST_PLL,
#else
{
	"PLL test",
	"pll",
	"display the specified pll range which can be sys/other/demod/video/audio",
	&pll_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_RTC
#if defined(CONFIG_TEST_RTC)
	CONFIG_TEST_RTC,
#else
{
	"RTC test",
	"rtc",
	"This test checks RTC",
	&rtc_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_NAND
#if defined(CONFIG_TEST_NAND)
	CONFIG_TEST_NAND,
#else
{
	"NAND test",
	"nand",
	"This test checks NAND",
	&nand_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_SDCARD
#if defined(CONFIG_TEST_SDCARD)
	CONFIG_TEST_SDCARD,
#else
{
	"SDCARD test",
	"sdcard",
	"This test checks SDCARD",
	&sdcard_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_I2C
#if defined(CONFIG_TEST_I2C)
	CONFIG_TEST_I2C,
#else
{
	"I2C test",
	"i2c",
	"This test checks I2C, format: i2c [addr]",
	&i2c_systest,
	NULL
},
#endif
#endif

#ifdef CONFIG_SYS_TEST_ADC
#if defined(CONFIG_TEST_ADC)
	CONFIG_TEST_ADC,
#else
{
	"ADC test",
	"adc",
	"This test checks ADC ",
	&adc_systest,
	&adc_systest_init
},
#endif
#endif


};




unsigned int systest_list_size = sizeof (systest_list) / sizeof (struct sys_test);
