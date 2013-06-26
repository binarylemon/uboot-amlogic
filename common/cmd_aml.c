/*
 * Command for Amlogic Custom.
 *
 * Copyright (C) 2012 Amlogic.
 * Elvis Yu <elvis.yu@amlogic.com>
 */

#include <common.h>
#include <command.h>

extern inline void key_init(void);
extern inline int get_key(void);
extern inline int is_ac_online(void);
extern void power_off(void);
extern inline int get_charging_percent(void);
extern inline int set_charging_current(int current);


static int do_getkey (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static int key_enable = 0;
	if(!key_enable)
	{
		key_init();
		key_enable = 1;
	}
	return !get_key();
}


U_BOOT_CMD(
	getkey,	1,	0,	do_getkey,
	"get POWER key",
	"/N\n"
	"This command will get POWER key'\n"
);

#ifdef CHECK_ALL_REGULATORS
static inline int do_chk_all_regulators (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return check_all_regulators();
}

U_BOOT_CMD(
	chk_all_regulators,	1,	0,	do_chk_all_regulators,
	"check all regulators",
	"/N\n"
	"This command will check all regulators\n"
);
#endif

static inline int do_ac_online (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return !is_ac_online();
}


U_BOOT_CMD(
	ac_online,	1,	0,	do_ac_online,
	"get ac adapter online",
	"/N\n"
	"This command will get ac adapter online'\n"
);


static int do_poweroff (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	power_off();
	return 0;
}


U_BOOT_CMD(
	poweroff,	1,	0,	do_poweroff,
	"system power off",
	"/N\n"
	"This command will let system power off'\n"
);

static int do_get_batcap (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char precent_str[16];
	int precent = get_charging_percent();
	printf("Battery CAP: %d%%\n", precent);
	sprintf(precent_str, "%d", precent);
	setenv("battery_cap", precent_str);
	return 0;
}


U_BOOT_CMD(
	get_batcap,	1,	0,	do_get_batcap,
	"get battery capability",
	"/N\n"
	"This command will get battery capability\n"
	"capability will set to 'battery_cap'\n"
);

static int do_set_chgcur (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int current = simple_strtol(argv[1], NULL, 10);

	set_charging_current(current);
	
	printf("Charging current: %smA\n", argv[1]);
	setenv("charging_current", argv[1]);
	return 0;
}


U_BOOT_CMD(
	set_chgcur,	2,	0,	do_set_chgcur,
	"set battery charging current",
	"/N\n"
	"set_chgcur <current>\n"
	"unit is mA\n"
);

/* set  usb current limit */
#ifdef CONFIG_AW_AXP20
extern int axp_charger_set_usbcur_limit(int usbcur_limit);
#endif
#ifdef CONFIG_AML_PMU
extern int aml_pmu_set_usb_curr_limit(int curr);
#endif
static int pmu_set_usbcur_limit(int usbcur_limit)
{
#ifdef CONFIG_AW_AXP20
    return axp_charger_set_usbcur_limit(usbcur_limit);
#endif
#ifdef CONFIG_AML_PMU
    return aml_pmu_set_usb_curr_limit(usbcur_limit);
#endif
}
static int do_set_usbcur_limit(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
        return cmd_usage(cmdtp);
    int usbcur_limit = simple_strtol(argv[1], NULL, 10);
    pmu_set_usbcur_limit(usbcur_limit);
    printf("set usbcur_limit: %smA\n", argv[1]);
    if (argc == 2 )
		{
    setenv("usbcur_limit", argv[1]);
		}
    return 0;
}
U_BOOT_CMD(
	set_usbcur_limit,	3,	0,	do_set_usbcur_limit,
	"set pmu usb limit current",
	"/N\n"
	"set_usbcur_limit <current>\n"
	"unit is mA\n"
);

