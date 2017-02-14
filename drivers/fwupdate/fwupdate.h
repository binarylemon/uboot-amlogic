/*
 * fwupdate.h
 *
 * Copyright (C) 2012, StreamUnlimited Engineering GmbH, http://www.streamunlimited.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __FWUPDATE_H
#define __FWUPDATE_H

#include <common.h>

#define MAX_BOOT_COUNT	(6UL)

int fwupdate_init(void);

int fwupdate_getUpdateFlag(uint8_t *pUpdateFlag);
int fwupdate_setUpdateFlag(uint8_t updateFlag);

int fwupdate_getFailFlag(uint8_t *pFailFlag);
int fwupdate_setFailFlag(uint8_t failFlag);

int fwupdate_getBootCount(uint8_t* pBootCnt);
int fwupdate_setBootCount(uint8_t bootCnt);

typedef enum {
    BS_Off = 0,
    BS_Normal,
    BS_DontUnplug,          /* flashing new FW, ... */
    BS_HardFailure,         /* cannot boot, ... */
    BS_BootingKernel,
} BoardState;

#endif	/* __FWUPDATE_H */
