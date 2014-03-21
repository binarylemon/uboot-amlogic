
/*
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *  Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Trustzone API
 *
 * Copyright (C) 2012 Amlogic, Inc.
 *
 * Author: Platform-SH@amlogic.com
 *
 */

#ifndef __MESON6_TRUSTZONE_H
#define __MESON6_TRUSTZONE_H

#include <asm/arch/io.h>
#include <amlogic/efuse.h>
#include <asm/cache.h>

#define CALL_TRUSTZONE_HAL_API		0x5
#define TRUSTZONE_HAL_API_EFUSE	0x100
#define TRUSTZONE_HAL_API_STORAGE   0x200

#define CALL_TRUSTZONE_MON                      0x4
#define TRUSTZONE_MON_CORE_RD_SOC_REV1          0x209
#define TRUSTZONE_MON_CORE_RD_SOC_REV2          0x20A


#ifdef CONFIG_MESON_STORAGE_BURN
struct storage_hal_api_arg{
	unsigned int cmd;
	unsigned int namelen;
	unsigned int name_phy_addr;
	unsigned int datalen;
	unsigned int data_phy_addr;	
	unsigned int retval_phy_addr;
};

#define STORAGE_HAL_API_INIT        0
#define STORAGE_HAL_API_WRITE    1
#define STORAGE_HAL_API_QUERY	 2
#ifdef CONFIG_MESON_STORAGE_DEBUG
#define STORAGE_HAL_API_READ      3
#endif

#endif







#endif
