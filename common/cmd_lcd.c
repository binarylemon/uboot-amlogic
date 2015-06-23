#include <common.h>
#include <command.h>
#include <amlogic/aml_lcd.h>

static int do_lcd_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	panel_oper.enable();
	return 0;
}

static int do_lcd_disable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	panel_oper.disable();
	return 0;
}

static int do_lcd_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	panel_oper.test(simple_strtoul(argv[1], NULL, 10));
	return 0;
}

static int do_lcd_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	panel_oper.info();
	return 0;
}

static int do_lcd_bl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int level;

	if (argc == 1) {
		return -1;
	}
	if (strcmp(argv[1], "on") == 0) {
		panel_oper.bl_on();
	} else if (strcmp(argv[1], "off") == 0) {
		panel_oper.bl_off();
	} else if (strcmp(argv[1], "level") == 0) {
		if (argc == 3) {
			level = (unsigned int)simple_strtoul(argv[2], NULL, 10);
			panel_oper.set_bl_level(level);
		} else {
			return 1;
		}
	} else {
		return 1;
	}
	return 0;
}

static cmd_tbl_t cmd_lcd_sub[] = {
	U_BOOT_CMD_MKENT(enable, 2, 0,  do_lcd_enable, "", ""),
	U_BOOT_CMD_MKENT(disable, 2, 0, do_lcd_disable, "", ""),
	U_BOOT_CMD_MKENT(test, 2, 0,    do_lcd_test, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 0,    do_lcd_info, "", ""),
	U_BOOT_CMD_MKENT(bl, 3, 0,      do_lcd_bl, "", ""),
};

static int do_lcd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	/* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_lcd_sub[0], ARRAY_SIZE(cmd_lcd_sub));

	if (c) {
		return c->cmd(cmdtp, flag, argc, argv);
	} else {
		cmd_usage(cmdtp);
		return 1;
	}
}

U_BOOT_CMD(
	lcd,	8,	0,	do_lcd,
	"lcd sub-system",
	"lcd enable   - enable lcd\n"
	"lcd disable  -disable lcd\n"
	"lcd test     -test lcd display\n"
	"lcd info     -print lcd driver info\n"
	"lcd bl on    -lcd backlight on\n"
	"lcd bl off   -lcd backlight off\n"
	"lcd bl level <level>  -set backlight level\n"
);
