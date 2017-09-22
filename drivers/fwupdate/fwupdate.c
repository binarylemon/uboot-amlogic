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

/* mask and bit shift in RTC scratch register for each flag */
#define MSK_UPDATE		1
#define SHIFT_UPDATE		0
#define MSK_LOCK		0x02
#define SHIFT_LOCK		1
#define MSK_SIGN		0x04
#define SHIFT_SIGN		2
#define MSK_FAIL		0x100
#define SHIFT_FAIL		8
#define MSK_BOOTCNT		0xff0000
#define SHIFT_BOOTCNT		16


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
static int flag_write(uint32_t msk, uint32_t shift, uint8_t data)
{
    unsigned int rtcWord = 0;

    MsgP("wr data is 0x%.8x, mask=0x%.8x, shift=%d\n", data, msk, shift);

    rtcWord = readl(_AML_RTC_REG_ADDR);
    rtcWord = (rtcWord & ~msk) | ((data << shift) & msk);
    writel(rtcWord, _AML_RTC_REG_ADDR);

    return 0;
}

static int flag_read(uint32_t msk, uint32_t shift, uint8_t* data)
{
    unsigned int rtcWord = 0;

    rtcWord = readl(_AML_RTC_REG_ADDR);
    MsgP("rtcWord is 0x%08x\n", rtcWord);
    *data = (rtcWord & msk) >> shift;
    
    MsgP("rd data is 0x%.8x, mask=0x%.8x, shift=%d\n", *data, msk, shift);

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

#define FWUPDATE_FLAGS(name, msk, shift) \
	int fwupdate_get##name (uint8_t *ptr) {return flag_read(msk, shift, ptr);} \
	int fwupdate_set##name (uint8_t val) {return flag_write(msk, shift, val);}

FWUPDATE_FLAGS(UpdateFlag, MSK_UPDATE, SHIFT_UPDATE);
FWUPDATE_FLAGS(LockFlag, MSK_LOCK, SHIFT_LOCK);
FWUPDATE_FLAGS(SignFlag, MSK_SIGN, SHIFT_SIGN);
FWUPDATE_FLAGS(FailFlag, MSK_FAIL, SHIFT_FAIL);
FWUPDATE_FLAGS(BootCount, MSK_BOOTCNT, SHIFT_BOOTCNT);

#ifdef CONFIG_BOOTCOUNT_LIMIT
void bootcount_store(ulong a)
{
	int status = fwupdate_setBootCount((uint32_t)a);

	if (0 != status)
		printf("ERROR: fwupdate_setBootCount() failed!\n");
	else
		printf("BOOTCOUNT is %ld\n", a);
}

ulong bootcount_load(void)
{
	uint8_t bootcount = 0xFF;

	int status = fwupdate_getBootCount(&bootcount);

	if (0 != status)
		printf("ERROR: getBootCount() failed!\n");

	return bootcount;
}
#endif /* CONFIG_BOOTCOUNT_LIMIT */

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
	uint8_t     lock;
	uint8_t     sign;
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
		} else if (strcmp(fwupflag, "lock") == 0) {
			if (0 != fwupdate_setLockFlag(0)) {
				return 1;
			}
		} else if (strcmp(fwupflag, "sign") == 0) {
			if (0 != fwupdate_setSignFlag(0)) {
				return 1;
			}
		}
		return 0;
	}

	if (strcmp(cmd, "flags") == 0) {
		uint8_t failFlag = 0;
		uint8_t updateFlag = 0;
		uint8_t lockFlag = 0;
		uint8_t signFlag = 0;

		fwupdate_getFailFlag(&failFlag);
		fwupdate_getUpdateFlag(&updateFlag);
		fwupdate_getLockFlag(&lockFlag);
		fwupdate_getSignFlag(&signFlag);

		printf("INFO: Update flags: FAIL: %d, UPDATE: %d, LOCK: %d SIGN: %d\n",
		       failFlag, updateFlag, lockFlag, signFlag);
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
		} else if (strcmp(fwupflag, "lock") == 0) {
			if (0 != fwupdate_setLockFlag(1)) {
				return 1;
			}
		} else if (strcmp(fwupflag, "sign") == 0) {
			if (0 != fwupdate_setSignFlag(1)) {
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
	 *   fwup lock
	 */
	if (strcmp(cmd, "lock") == 0) {

		if (0 > fwupdate_getLockFlag(&lock)) {
			return 1;
		} else {
			return (lock ? 0 : 1);
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1
	 *   fwup sign
	 */
	if (strcmp(cmd, "sign") == 0) {

		if (0 > fwupdate_getSignFlag(&sign)) {
			return 1;
		} else {
			return (sign ? 0 : 1);
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
		"fwup lock       - checks if lock flag is set\n"
		"fwup sign       - checks if sign flag is set\n"
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
