/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __APX152_H__
#define __APX152_H__

enum axp152_ldo0_mvolt {
	AXP152_LDO0_MVOLT_5000 = (0 << 4),
	AXP152_LDO0_MVOLT_3300 = (1 << 4),
	AXP152_LDO0_MVOLT_2800 = (2 << 4),
	AXP152_LDO0_MVOLT_2500 = (3 << 4),
};

enum axp152_ldo0_clim {
	AXP152_LDO0_CLIM_NO = 0,
	AXP152_LDO0_CLIM_1500 = 1,
	AXP152_LDO0_CLIM_900 = 2,
	AXP152_LDO0_CLIM_500 = 3,
};


int axp152_set_dcdc2(int mvolt);
int axp152_set_dcdc3(int mvolt);
int axp152_set_dcdc4(int mvolt);
int axp152_set_ldo0(enum axp152_ldo0_mvolt mvolt);
int axp152_set_ldo2(int mvolt);
int axp152_init(void);

#endif
