
#ifndef __AMLOGIC_LCD_H_
#define __AMLOGIC_LCD_H_
#include <common.h>
#include <linux/list.h>
#include <asm/arch/tcon.h>

extern int aml_lcd_init(void);

extern void lcd_power_on(void);
extern void lcd_power_off(void);

/* Panel device information */
struct panel_dev {
	char	name[16];		/* Device name				*/
	tcon_conf_t *tcon_cfg;
	void (*panel_setup_gama_table)(tcon_conf_t *pConf);

	struct list_head list;
};


typedef struct panel_operations {
	void  (*enable)(void);
	void  (*disable)(void);
	void  (*bl_on)(void);
	void  (*bl_off)(void);
	void  (*set_bl_level)(unsigned level);
} panel_operations_t;

extern panel_operations_t panel_opt;
extern int lcd_opt_cmd(int argc, char *argv[]);

#ifdef CONFIG_PANEL_T13
extern struct panel_dev panel_t13;
#endif

#endif


