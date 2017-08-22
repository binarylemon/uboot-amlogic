/*
 * Copyright 2011 Attero Tech, LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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
 * Firmware Update Functions and Commands
 */

#include "fwupdate.h"
#include <common.h>
/*#include <cpsw.h>*/
#include <hush.h>
#include <asm/arch/cpu.h>
/*#include <asm/arch/hardware.h>*/
#include <asm/io.h>
/*#include "board.h"*/

/* offset indexes in RTC scratch0 register for each flag */
#define UPDATE_FLAG_INDEX	0
#define FAIL_FLAG_INDEX		1
#define BOOT_COUNT_FLAG_INDEX	2

#define debugP(fmt...) //printf("L%d:", __LINE__),printf(fmt)
#define errorP(fmt...) printf("[ERR]L%d:", __LINE__),printf(fmt)
#define wrnP(fmt...)   printf("[WRN]"fmt)
#define MsgP(fmt...)   printf("[MSG]"fmt)

void set_board_state(BoardState state)
{
    return;
}

#define  CONFIG_AML_PLATFORM  1
#if CONFIG_AML_PLATFORM
#include <asm/arch/reg_addr.h>
#include <asm/arch/io.h>
#define _AML_RTC_REG_ADDR   P_PREG_STICKY_REG1
static int flag_write (uint8_t index, uint8_t data )
{
    unsigned int rtcWord = 0;

    MsgP("data is 0x%08x, index %d\n", data, index);
    if( index > 3 ) {
        errorP("index[%d] out of [0,2]\n", index);
        return __LINE__;
    }

    rtcWord = readl(_AML_RTC_REG_ADDR);
    rtcWord &= ~(0xffU<<(index * 8));
    rtcWord |= data << (index * 8);
    writel(rtcWord, _AML_RTC_REG_ADDR);

    return 0;
}

static int flag_read(uint8_t index, uint8_t* data)
{
    unsigned int rtcWord = 0;

    if( index > 3 ) {
        errorP("index[%d] out of [0,2]\n", index);
        return __LINE__;
    }

    rtcWord = readl(_AML_RTC_REG_ADDR);
    MsgP("rtcWord is 0x%08x\n", rtcWord);
    *data = (rtcWord >> (index * 8)) & 0xffU;

    return 0;
}
#else
static struct rtc_regs *rtcregs = (struct rtc_regs *)RTC_BASE;
/*
 * Modify selected flag by its index.
 */
static int flag_write( uint8_t index, uint8_t data)
{
	uint32_t valb = 0;
	uint32_t addr = (uint32_t)&rtcregs->scratch0;

	assert(index <= 3);

	if (index > 3)
		return 1;

	valb = readl(addr);

	/*
	 * Modify selected byte in RTC register.
	 * 0: (uint8*)addr + 0;    1: (uint8*)addr + 1; ..
	 */
	((uint8_t *)&valb)[index] = data;

	writel(valb, addr);

	return 0;
}

/*
 * Read selected flag by its index.
 */
static int flag_read(uint8_t index, uint8_t *data)
{
	uint32_t valb = 0;
	uint32_t addr = (uint32_t)&rtcregs->scratch0;

	assert(index <= 3);

	if (index > 3)
		return 1;

	valb = readl(addr);

	/*
	 * Read selected byte in RTC register.
	 * 0: (uint8*)addr + 0;    1: (uint8*)addr + 1; ..
	 */
	*data = ((uint8_t *)&valb)[index];

	return 0;
}

#endif// #if CONFIG_AML_PLATFORM


int fwupdate_init(void)
{
	int status = 0;
	char* bootlimit;

	bootlimit = getenv("bootlimit");
	if (NULL == bootlimit) {
		char buf[16];
		sprintf(buf, "%lu", MAX_BOOT_COUNT);
		setenv("bootlimit", buf);
	}

	return status;
}

int fwupdate_getUpdateFlag(uint8_t *pUpdateFlag)
{
	return flag_read(UPDATE_FLAG_INDEX, pUpdateFlag);
}

int fwupdate_setUpdateFlag(uint8_t updateFlag)
{
	return flag_write(UPDATE_FLAG_INDEX, updateFlag);
}

int fwupdate_getFailFlag(uint8_t* pFailFlag)
{
	return flag_read(FAIL_FLAG_INDEX, pFailFlag);
}

int fwupdate_setFailFlag(uint8_t failFlag)
{
	return flag_write(FAIL_FLAG_INDEX, failFlag);
}

int fwupdate_getBootCount(uint8_t* pBootCnt)
{
	return flag_read(BOOT_COUNT_FLAG_INDEX, pBootCnt);
}

int fwupdate_setBootCount(uint8_t bootCnt)
{
	return flag_write(BOOT_COUNT_FLAG_INDEX, bootCnt);
}

//detect GPIOX_16 pressed not shorter than 100ms
static int fwupdate_getUsbUpdateReq(void)
{
#if CONFIG_AML_PLATFORM
    return !run_command("get_gpio_key 100 GPIOX_16", 0);
#else
	if (board_interface != NULL && board_interface->do_usb_update != NULL)
		return board_interface->do_usb_update();
#endif//
	//panic("Not supported board!");
	return 0;
}

static int do_fwup(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char*       cmd = NULL;
	char*       fwupflag = NULL;
	uint8_t     update;
	uint8_t     fail;
	uint8_t     bootcnt;
	char*       bootlimit;
	uint32_t    bootmax;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	/*
	 * Syntax is:
	 *   0    1     2
	 *   fwup clear flag
	 */
	if (strcmp(cmd, "clear") == 0) {
		if (argc != 3)
			goto usage;
		else
			fwupflag = argv[2];

		if (strcmp(fwupflag, "update") == 0) {
			if (0 != fwupdate_setUpdateFlag(0)) {
				return 1;
			}
		} else if (strcmp(fwupflag, "fail") == 0) {
			if (0 != fwupdate_setFailFlag(0)) {
				return 1;
			}
		}
		return 0;
	}

	if (strcmp(cmd, "flags") == 0) {
		uint8_t failFlag = 0;
		uint8_t updateFlag = 0;

		fwupdate_getFailFlag(&failFlag);
		fwupdate_getUpdateFlag(&updateFlag);

		printf("INFO: Update flags: FAIL: %d, UPDATE: %d\n",
		       failFlag, updateFlag);
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1   2
	 *   fwup set flag
	 */
	if (strcmp(cmd, "set") == 0) {
		if (argc != 3)
			goto usage;
		else
			fwupflag = argv[2];

		if (strcmp(fwupflag, "update") == 0) {
			if (0 != fwupdate_setUpdateFlag(1)) {
				return 1;
			}
		} else if (strcmp(fwupflag, "fail") == 0) {
			if (0 != fwupdate_setFailFlag(1)) {
				return 1;
			}
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup update
	 */
	if (strcmp(cmd, "update") == 0) {

		if (0 > fwupdate_getUpdateFlag(&update)) {
			return 1;
		} else {
			return (update ? 0 : 1);
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup usb_update_req
	 */
	if (strcmp(cmd, "usb_update_req") == 0)
            return fwupdate_getUsbUpdateReq() ? 0 : 1;

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup fail
	 */
	if (strcmp(cmd, "fail") == 0) {

		if (0 > fwupdate_getFailFlag(&fail)) {
			return 1;
		} else {
			return (fail ? 0 : 1);
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup incbootcnt
	 */
	if (strcmp(cmd, "incbootcnt") == 0) {

		if (0 > fwupdate_getBootCount(&bootcnt)) {
			return 1;
		} else if (0 > fwupdate_setBootCount(bootcnt+1)) {
			return 1;
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup bootcnt
	 */
	if (strcmp(cmd, "bootcnt") == 0) {

		if (0 > fwupdate_getBootCount(&bootcnt)) {
			return 1;
		} else {
			bootlimit = getenv("bootlimit");
			if (NULL == bootlimit) {
				bootmax = MAX_BOOT_COUNT;
			} else {
				bootmax = simple_strtoul(bootlimit, NULL, 10);
			}
		}
		return ((bootcnt < bootmax) ? 0 : 1);
	}

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
		fwup, CONFIG_SYS_MAXARGS, 1, do_fwup,
		"Streamunlimited firmware update",
		"clear flag - clears the user requested flag\n"
		"fwup flags      - print current flags\n"
		"fwup set flag   - sets the user requested flag\n"
		"fwup update     - checks if update flag is set\n"
		"fwup usb_update_req - checks if USB update request active\n"
		"fwup fail       - checks if fail flag is set\n"
		"fwup incbootcnt - increments boot count\n"
		"fwup bootcnt    - checks if boot count is less than maximum allowed"
		);

#ifdef CONFIG_HUSH_INIT_VAR
int hush_init_var (void)
{
	int status = 0;

	status |= set_hush_var_with_str_value("UBOOT_RESET", "no");
	status |= set_hush_var_with_str_value("UPDATE_XLOADER", "no");
	status |= set_hush_var_with_str_value("KERNEL_ERASED", "no");
	status |= set_hush_var_with_str_value("UPDATE_ROOTFS", "no");
	status |= set_hush_var_with_str_value("UPDATE_KERNEL", "no");
	status |= set_hush_var_with_str_value("ERROR_STATE", "no");

	return status;
}
#endif
