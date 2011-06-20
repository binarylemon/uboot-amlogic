#include <common.h>
#include <aml_lcd.h>
#include <asm/arch/tcon.h>

static struct panel_dev panel_devs;

static struct panel_dev* panel_get_by_name(char* name);

static void opt_cmd_help(void)
{
	printf("Help:\n");
	printf("enable	-enable lcd\n");
	printf("disable	-disable lcd\n");
	printf("bl_on	-lcd backlight on\n");
	printf("bl_off	-lcd backlight off\n");
	printf("set_bl_level <level>	-set backlight level\n");
}

int lcd_opt_cmd(int argc, char *argv[])
{
	struct panel_dev *pdev = panel_get_by_name(getenv ("panel"));
	if(pdev == NULL)
	{
		printf("panel:%s cannot be identified.\n", getenv ("panel"));
		printf("please setenv panel ...\n");
	}
	if(strcmp(argv[0], "enable") == 0)
	{
		//tcon_remove();
		pdev->panel_setup_gama_table(pdev->tcon_cfg);
		panel_opt.enable();
		tcon_probe(*(pdev->tcon_cfg));
	}
	else if(strcmp(argv[0], "disable") == 0)
	{
		panel_opt.disable();
		tcon_remove();
	}
	else if(strcmp(argv[0], "bl_on") == 0)
	{
		panel_opt.bl_on();
	}
	else if(strcmp(argv[0], "bl_off") == 0)
	{
		panel_opt.bl_off();
	}
	else if(strcmp(argv[0], "set_bl_level") == 0)
	{
		panel_opt.set_bl_level(simple_strtoul(argv[1], NULL, 10));
	}
	else
	{
		printf("Current device is panel.\n");
		opt_cmd_help();
		return 1;
	}
	return 0;	
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
static struct list_head* panel_get_list(void)
{
	return &(panel_devs.list);
}

static struct panel_dev* panel_get_by_name(char* name)
{
	struct list_head *pos;
	struct panel_dev *dev;

	if(!name)
		return NULL;

	list_for_each(pos, &(panel_devs.list)) {
		dev = list_entry(pos, struct panel_dev, list);
		if(strcmp(dev->name, name) == 0)
			return dev;
	}

	return NULL;
}

static struct panel_dev* panel_clone(struct panel_dev *dev)
{
	struct panel_dev *_dev;

	if(!dev)
		return NULL;

	_dev = calloc(1, sizeof(struct panel_dev));
	if(!_dev)
		return NULL;
	memcpy(_dev, dev, sizeof(struct panel_dev));

	_dev->tcon_cfg = calloc(1, sizeof(tcon_conf_t));
	if(!_dev)
		return NULL;
	memcpy(_dev->tcon_cfg, dev->tcon_cfg, sizeof(tcon_conf_t));
	
	strncpy(_dev->name, dev->name, 16);

	return _dev;
}

int panel_register (struct panel_dev * dev)
{
	struct panel_dev *_dev;
	/* Initialize the list */
	INIT_LIST_HEAD(&(panel_devs.list));

	_dev = panel_clone(dev);
	if(!_dev)
		return -1;
	list_add_tail(&(_dev->list), &(panel_devs.list));
	return 0;
}

int aml_lcd_init(void)
{
#ifdef CONFIG_PANEL_T13
	if(panel_register(&panel_t13) < 0) goto panel_register_failed;
#endif
	return 0;
panel_register_failed:
	printf("panel register failed!\n");
	return -1;
}
