#include <common.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/gpio.h>
#ifdef CONFIG_AML_LCD_EXTERN
#include <amlogic/aml_lcd_extern.h>
#endif
#include "lcd_common.h"

#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>

static const char* lcd_type_table[]={
	"lvds",
	"vbyone",
	"ttl",
	"invalid",
};
#endif

enum {
	LCD_OUTPUT_MODE_1080P = 0,
	LCD_OUTPUT_MODE_1080P50HZ,
	LCD_OUTPUT_MODE_768P60HZ,
	LCD_OUTPUT_MODE_768P50HZ,
	LCD_OUTPUT_MODE_4K2K60HZ420,
	LCD_OUTPUT_MODE_4K2K50HZ420,
	LCD_OUTPUT_MODE_4K2K60HZ,
	LCD_OUTPUT_MODE_4K2K50HZ,
	LCD_OUTPUT_MODE_MAX,
};

struct lcd_info_s {
	char *name;
	int width;
	int height;
	int sync_duration_num;
	int sync_duration_den;
};

static struct lcd_info_s lcd_info[] = {
	{
		.name              = "1080p",
		.width             = 1920,
		.height            = 1080,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
	},
	{
		.name              = "1080p50hz",
		.width             = 1920,
		.height            = 1080,
		.sync_duration_num = 50,
		.sync_duration_den = 1,
	},
	{
		.name              = "768p60hz",
		.width             = 1366,
		.height            = 768,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
	},
	{
		.name              = "768p50hz",
		.width             = 1366,
		.height            = 768,
		.sync_duration_num = 50,
		.sync_duration_den = 1,
	},
	{
		.name              = "4k2k60hz420",
		.width             = 3840,
		.height            = 2160,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
	},
	{
		.name              = "4k2k50hz420",
		.width             = 3840,
		.height            = 2160,
		.sync_duration_num = 50,
		.sync_duration_den = 1,
	},
	{
		.name              = "4k2k60hz",
		.width             = 3840,
		.height            = 2160,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
	},
	{
		.name              = "4k2k50hz",
		.width             = 3840,
		.height            = 2160,
		.sync_duration_num = 50,
		.sync_duration_den = 1,
	},
	{
		.name              = "invalid",
		.width             = 1920,
		.height            = 1080,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
	},
};

static void lcd_vmode_is_mached(Lcd_Config_t *pConf, int index)
{
	if ((pConf->lcd_basic.h_active != lcd_info[index].width) ||
		(pConf->lcd_basic.v_active != lcd_info[index].height)) {
		printf("lcd error: outputmode and panel_type is not matched\n");
	}
}

static int _load_lcd_output_mode(Lcd_Config_t *pConf)
{
	char *mode;
	int lcd_output_mode, i;

	mode = getenv("outputmode");
	lcd_printf("lcd: output mode: %s\n", mode);
	for (i = 0; i < ARRAY_SIZE(lcd_info); i++) {
		if (strcmp(mode, lcd_info[i].name) == 0)
			break;
	}
	lcd_output_mode = i;
	if (lcd_output_mode >= LCD_OUTPUT_MODE_MAX)
		printf("lcd: output mode is not support\n");

	pConf->lcd_timing.sync_duration_num = lcd_info[lcd_output_mode].sync_duration_num;
	pConf->lcd_timing.sync_duration_den = lcd_info[lcd_output_mode].sync_duration_den;

	lcd_vmode_is_mached(pConf, lcd_output_mode);

	return lcd_output_mode;
}

static int _load_lcd_config_from_dtd(char *dt_addr, Lcd_Config_t *pConf)
{
#ifdef CONFIG_OF_LIBFDT
	int parent_offset;
	int child_offset;
	char propname[30];
	char* propdata;
	int i;
	int temp;

	parent_offset = fdt_path_offset(dt_addr, "/lcd");
	if (parent_offset < 0) {
		printf("lcd error: not find /lcd node %s.\n",fdt_strerror(parent_offset));
		return 0;
	}

	/* driver version detect */
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "version", NULL);
	if (propdata == NULL) {
		pConf->version = 0;
	} else {
		pConf->version = (unsigned char)(be32_to_cpup((u32*)propdata));
	}
	lcd_printf("lcd: version: %d\n", pConf->version);

	char *panel_type = getenv("panel_type");
	if (panel_type == NULL) {
		printf("lcd error: no panel_type use defult lcd config\n ");
		return 0;
	}
	sprintf(propname, "/lcd/%s", panel_type);
	child_offset = fdt_path_offset(dt_addr, propname);
	if (child_offset < 0) {
		printf("lcd error: dts: not find /lcd/%s  node %s\n",panel_type,fdt_strerror(child_offset));
		return 0;
	} else {
		lcd_printf("lcd: load panel_type: %s\n", panel_type);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "interface", NULL);
	if (propdata == NULL) {
		printf("lcd error: failed to get interface!\n");
		return 0;
	} else {
		for (i = 0; i < LCD_TYPE_MAX; i++) {
			if (!strncmp(propdata, lcd_type_table[i], 3))
				break;
		}
		pConf->lcd_basic.lcd_type = i;
	}
	lcd_printf("dtd_lcd: lcd_type = %s(%u)\n", lcd_type_table[pConf->lcd_basic.lcd_type], pConf->lcd_basic.lcd_type);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "basic_setting", NULL);
	if (propdata == NULL) {
		printf("lcd error: failed to get basic_setting\n");
		return 0;
	} else {
		pConf->lcd_basic.h_active = be32_to_cpup((u32*)propdata);
		pConf->lcd_basic.v_active = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_basic.h_period = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_basic.v_period = be32_to_cpup((((u32*)propdata)+3));
		/* version 0 */
		pConf->lcd_basic.video_on_pixel = be32_to_cpup((((u32*)propdata)+4));
		pConf->lcd_basic.video_on_line  = be32_to_cpup((((u32*)propdata)+5));
	}
	lcd_printf("dtd_lcd:h_active = %d \n",pConf->lcd_basic.h_active);
	lcd_printf("dtd_lcd:v_active = %d \n",pConf->lcd_basic.v_active);
	lcd_printf("dtd_lcd:h_period = %d \n",pConf->lcd_basic.h_period);
	lcd_printf("dtd_lcd:v_period = %d \n",pConf->lcd_basic.v_period);
	if (pConf->version == 0) {
		lcd_printf("dtd_lcd:video_on_pixel = %d \n",pConf->lcd_basic.video_on_pixel);
		lcd_printf("dtd_lcd:video_on_line = %d \n",pConf->lcd_basic.video_on_line);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "lcd_timing", NULL);
	if (propdata == NULL) {
		printf("lcd error: failed to get lcd_timing\n");
		return 0;
	} else {
		pConf->lcd_timing.hpll_clk = be32_to_cpup((u32*)propdata);
		pConf->lcd_timing.hpll_od  = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_timing.hdmi_pll_cntl5 = be32_to_cpup((((u32*)propdata)+2));
		/* version 1 */
		pConf->lcd_timing.hsync_width    = be32_to_cpup((((u32*)propdata)+3));
		pConf->lcd_timing.hsync_bp       = be32_to_cpup((((u32*)propdata)+4));
		pConf->lcd_timing.vsync_width    = be32_to_cpup((((u32*)propdata)+5));
		pConf->lcd_timing.vsync_bp       = be32_to_cpup((((u32*)propdata)+6));
		/* version 0 */
		pConf->lcd_timing.sth1_hs_addr   = be32_to_cpup((((u32*)propdata)+3));
		pConf->lcd_timing.sth1_he_addr   = be32_to_cpup((((u32*)propdata)+4));
		pConf->lcd_timing.sth1_vs_addr   = be32_to_cpup((((u32*)propdata)+5));
		pConf->lcd_timing.sth1_ve_addr   = be32_to_cpup((((u32*)propdata)+6));
		pConf->lcd_timing.stv1_hs_addr   = be32_to_cpup((((u32*)propdata)+7));
		pConf->lcd_timing.stv1_he_addr   = be32_to_cpup((((u32*)propdata)+8));
		pConf->lcd_timing.stv1_vs_addr   = be32_to_cpup((((u32*)propdata)+9));
		pConf->lcd_timing.stv1_ve_addr   = be32_to_cpup((((u32*)propdata)+10));
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "frame_rate_adjust_type", NULL);
	if (propdata == NULL) {
		lcd_printf("lcd: failed to get frame_rate_adjust_type\n");
		pConf->lcd_timing.frame_rate_adj_type = 0;
	} else {
		pConf->lcd_timing.frame_rate_adj_type = (unsigned char)(be32_to_cpup((u32*)propdata));
	}
	lcd_printf("dtd_lcd:frame_rate_adj_type = %d \n",pConf->lcd_timing.frame_rate_adj_type);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "clk_att", NULL);
	if (propdata == NULL) {
		lcd_printf("lcd: failed to get clk_att\n");
		pConf->lcd_timing.clk_auto = 1;
		pConf->lcd_timing.ss_level = 0;
	} else {
		pConf->lcd_timing.clk_auto = (unsigned char)(be32_to_cpup((u32*)propdata));
		pConf->lcd_timing.ss_level = (unsigned char)(be32_to_cpup((((u32*)propdata)+1)));
	}
	lcd_printf("dtd_lcd:clk_auto = %d \n",pConf->lcd_timing.clk_auto);
	lcd_printf("dtd_lcd:ss_level = %d \n",pConf->lcd_timing.ss_level);

	lcd_printf("dtd_lcd:clk_auto = %d\n",pConf->lcd_timing.clk_auto);
	if (pConf->lcd_timing.clk_auto == 0) {
		lcd_printf("dtd_lcd:hpll_clk = %x \n",pConf->lcd_timing.hpll_clk);
		lcd_printf("dtd_lcd:hpll_od = %x \n",pConf->lcd_timing.hpll_od);
		lcd_printf("dtd_lcd:hdmi_pll_cntl5 = %x \n",pConf->lcd_timing.hdmi_pll_cntl5);
	}
	if (pConf->version) {
		lcd_printf("dtd_lcd:hsync_width = %d \n",pConf->lcd_timing.hsync_width);
		lcd_printf("dtd_lcd:hsync_bp = %d \n",   pConf->lcd_timing.hsync_bp);
		lcd_printf("dtd_lcd:vsync_width = %d \n",pConf->lcd_timing.vsync_width);
		lcd_printf("dtd_lcd:vsync_bp = %d \n",   pConf->lcd_timing.vsync_bp);
	} else {
		lcd_printf("dtd_lcd:sth1_hs_addr = %d \n",pConf->lcd_timing.sth1_hs_addr);
		lcd_printf("dtd_lcd:sth1_he_addr = %d \n",pConf->lcd_timing.sth1_he_addr);
		lcd_printf("dtd_lcd:sth1_vs_addr = %d \n",pConf->lcd_timing.sth1_vs_addr);
		lcd_printf("dtd_lcd:sth1_ve_addr = %d \n",pConf->lcd_timing.sth1_ve_addr);
		lcd_printf("dtd_lcd:stv1_hs_addr = %d \n",pConf->lcd_timing.stv1_hs_addr);
		lcd_printf("dtd_lcd:stv1_he_addr = %d \n",pConf->lcd_timing.stv1_he_addr);
		lcd_printf("dtd_lcd:stv1_vs_addr = %d \n",pConf->lcd_timing.stv1_vs_addr);
		lcd_printf("dtd_lcd:stv1_ve_addr = %d \n",pConf->lcd_timing.stv1_ve_addr);
	}

	if (((char *)fdt_getprop(dt_addr, child_offset, "lvds_att", NULL))) {
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "lvds_att", NULL);
		pConf->lcd_control.lvds_config->lvds_bits   = be32_to_cpup((u32*)propdata);
		pConf->lcd_control.lvds_config->lvds_repack = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_control.lvds_config->pn_swap     = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_control.lvds_config->dual_port   = be32_to_cpup((((u32*)propdata)+3));
		temp = be32_to_cpup((((u32*)propdata)+4));
		if (pConf->version)
			pConf->lcd_control.lvds_config->port_swap   = temp ? 1 : 0;
		else
			pConf->lcd_control.lvds_config->port_swap   = temp ? 0 : 1;
		//pConf->lcd_control.lvds_config->port_reverse= be32_to_cpup((((u32*)propdata)+4));
		//pConf->lcd_control.lvds_config->lvds_fifo_wr_mode = be32_to_cpup((((u32*)propdata)+5));

		lcd_printf("dtd_lcd:lvds_bits = %d \n",pConf->lcd_control.lvds_config->lvds_bits);
		lcd_printf("dtd_lcd:lvds_repack = %d \n",pConf->lcd_control.lvds_config->lvds_repack);
		lcd_printf("dtd_lcd:pn_swap = %d \n",pConf->lcd_control.lvds_config->pn_swap);
		lcd_printf("dtd_lcd:dual_port = %d \n",pConf->lcd_control.lvds_config->dual_port);
		lcd_printf("dtd_lcd:port_swap = %d \n",pConf->lcd_control.lvds_config->port_swap);
		//lcd_printf("dtd_lcd:port_reverse = %d \n",pConf->lcd_control.lvds_config->port_reverse);
		//lcd_printf("dtd_lcd:lvds_fifo_wr_mode = %d \n",pConf->lcd_control.lvds_config->lvds_fifo_wr_mode);
	} else if (((char *)fdt_getprop(dt_addr, child_offset, "vbyone_att", NULL))) {
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "vbyone_att", NULL);
		pConf->lcd_control.vbyone_config->lane_count	= be32_to_cpup((u32*)propdata);
		pConf->lcd_control.vbyone_config->byte		= be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_control.vbyone_config->region	= be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_control.vbyone_config->color_fmt	= be32_to_cpup((((u32*)propdata)+3));

		lcd_printf("dtd_lcd:lane_count = %d \n",pConf->lcd_control.vbyone_config->lane_count);
		lcd_printf("dtd_lcd:byte_mode = %d \n",pConf->lcd_control.vbyone_config->byte);
		lcd_printf("dtd_lcd:region_num = %d \n",pConf->lcd_control.vbyone_config->region);
		lcd_printf("dtd_lcd:color_fmt = %d \n",pConf->lcd_control.vbyone_config->color_fmt);
	} else if (((char *)fdt_getprop(dt_addr, child_offset, "ttl_att", NULL))) {
		lcd_printf("this is ttl att \n");
	} else {
		printf("lcd error: failed to get basic_setting\n");
		return 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "panel_power_pin", NULL);
	if (propdata == NULL) {
		printf("lcd error: failed to get panel_power_pin\n");
		return 0;
	} else {
		temp = gpioname_to_pin(propdata);
		if (temp < 0) {
			printf("lcd error: wrong gpio number %s\n", propdata);
			return 0;
		}
		pConf->lcd_power_ctrl.panel_power->gpio = temp;
	}
	lcd_printf("dtd_lcd:panel_power_pin: %s--%d \n",propdata,pConf->lcd_power_ctrl.panel_power->gpio);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "panel_power_att", NULL);
	if (propdata == NULL) {
		printf("lcd error: failed to get basic_setting\n");
		return 0;
	} else {
		pConf->lcd_power_ctrl.panel_power->on_value  = be32_to_cpup((u32*)propdata);
		pConf->lcd_power_ctrl.panel_power->off_value = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_power_ctrl.panel_power->panel_on_delay = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_power_ctrl.panel_power->panel_off_delay = be32_to_cpup((((u32*)propdata)+3));
	}
	lcd_printf("dtd_lcd:on_value = %d \n",pConf->lcd_power_ctrl.panel_power->on_value);
	lcd_printf("dtd_lcd:off_value = %d \n",pConf->lcd_power_ctrl.panel_power->off_value);
	lcd_printf("dtd_lcd:panel_on_delay = %d \n",pConf->lcd_power_ctrl.panel_power->panel_on_delay);
	lcd_printf("dtd_lcd:panel_off_delay = %d \n",pConf->lcd_power_ctrl.panel_power->panel_off_delay);

#ifdef CONFIG_AML_LCD_EXTERN
	propdata = (char *)fdt_getprop(dt_addr, child_offset, "lcd_extern_att", NULL);
	if (propdata == NULL) {
		printf("lcd: no lcd_extern_att\n");
		return 0;
	} else {
		pConf->lcd_control.ext_config->index = be32_to_cpup((u32*)propdata);
		pConf->lcd_control.ext_config->on_delay = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_control.ext_config->off_delay = be32_to_cpup((((u32*)propdata)+3));
	}
	lcd_printf("dtd_lcd:lcd_extern index = %d \n",pConf->lcd_control.ext_config->index);
	lcd_printf("dtd_lcd:lcd_extern power_on_delay = %d \n",pConf->lcd_control.ext_config->on_delay);
	lcd_printf("dtd_lcd:lcd_extern power_off_delay = %d \n",pConf->lcd_control.ext_config->off_delay);
#endif
#endif

	return 0;
}

extern Ext_Lcd_Config_t ext_lcd_config[LCD_TYPE_MAX];

static int _load_lcd_config_from_bsp(Lcd_Config_t *pConf)
{
	Ext_Lcd_Config_t *ext_lcd = NULL;
	char *panel_type = getenv("panel_type");
	unsigned int i = 0;
	unsigned int temp;

	if (panel_type == NULL) {
		printf("lcd error: no panel_type use defult lcd config\n ");
		return 0;
	}
	for (i = 0 ; i < LCD_TYPE_MAX ; i++) {
		ext_lcd = &ext_lcd_config[i];
		if (strcmp(ext_lcd->panel_type, panel_type) == 0) {
			lcd_printf("ext_lcd:use panel_type = %s \n",ext_lcd->panel_type);
			break ;
		}
	}
	if ( i >= LCD_TYPE_MAX ) {
		printf("lcd error: out of range use defult lcd config\n ");
		return 0;
	} else {
		lcd_printf("lcd: load panel_type: %s\n", panel_type);
	}

	pConf->lcd_basic.lcd_type = ext_lcd->lcd_type;
	pConf->lcd_basic.h_active = ext_lcd->h_active;
	pConf->lcd_basic.v_active = ext_lcd->v_active;
	pConf->lcd_basic.h_period = ext_lcd->h_period;
	pConf->lcd_basic.v_period = ext_lcd->v_period;
	/* version 0 */
	pConf->lcd_basic.video_on_pixel = ext_lcd->video_on_pixel;
	pConf->lcd_basic.video_on_line  = ext_lcd->video_on_line;

	pConf->lcd_timing.hpll_clk = ext_lcd->hpll_clk;
	pConf->lcd_timing.hpll_od  = ext_lcd->hpll_od;
	pConf->lcd_timing.hdmi_pll_cntl5 = ext_lcd->hdmi_pll_cntl5;

	if (pConf->version) { /* version 1 */
		pConf->lcd_timing.hsync_width    = ext_lcd->timing_val_0;
		pConf->lcd_timing.hsync_bp       = ext_lcd->timing_val_1;
		pConf->lcd_timing.vsync_width    = ext_lcd->timing_val_2;
		pConf->lcd_timing.vsync_bp       = ext_lcd->timing_val_3;
	} else { /* version 0 */
		pConf->lcd_timing.sth1_hs_addr   = ext_lcd->timing_val_0;
		pConf->lcd_timing.sth1_he_addr   = ext_lcd->timing_val_1;
		pConf->lcd_timing.sth1_vs_addr   = ext_lcd->timing_val_2;
		pConf->lcd_timing.sth1_ve_addr   = ext_lcd->timing_val_3;
		pConf->lcd_timing.stv1_hs_addr   = ext_lcd->timing_val_4;
		pConf->lcd_timing.stv1_he_addr   = ext_lcd->timing_val_5;
		pConf->lcd_timing.stv1_vs_addr   = ext_lcd->timing_val_6;
		pConf->lcd_timing.stv1_ve_addr   = ext_lcd->timing_val_7;
	}

	/* fr_adjust_type */
	temp = ext_lcd->customer_val_0;
	if (temp == Rsv_val)
		pConf->lcd_timing.frame_rate_adj_type = 0;
	else
		pConf->lcd_timing.frame_rate_adj_type = (unsigned char)temp;
	/* clk_auto_generate */
	temp = ext_lcd->customer_val_1;
	if (temp == Rsv_val)
		pConf->lcd_timing.clk_auto = 1;
	else
		pConf->lcd_timing.clk_auto = (unsigned char)temp;
	/* ss_level */
	temp = ext_lcd->customer_val_2;
	if (temp == Rsv_val)
		pConf->lcd_timing.ss_level = 0;
	else
		pConf->lcd_timing.ss_level = (unsigned char)temp;

	if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_VBYONE) {
		pConf->lcd_control.vbyone_config->lane_count	= ext_lcd->lcd_spc_val0;
		pConf->lcd_control.vbyone_config->byte		= ext_lcd->lcd_spc_val1;
		pConf->lcd_control.vbyone_config->region	= ext_lcd->lcd_spc_val2;
		pConf->lcd_control.vbyone_config->color_fmt	= ext_lcd->lcd_spc_val3;
	} else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_TTL) {
		printf("this is ttl att \n");
	} else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_LVDS) {
		pConf->lcd_control.lvds_config->lvds_bits   = ext_lcd->lcd_spc_val0;
		pConf->lcd_control.lvds_config->lvds_repack = ext_lcd->lcd_spc_val1;
		pConf->lcd_control.lvds_config->pn_swap     = ext_lcd->lcd_spc_val2;
		pConf->lcd_control.lvds_config->dual_port   = ext_lcd->lcd_spc_val3;
		temp = ext_lcd->lcd_spc_val4;
		if (pConf->version)
			pConf->lcd_control.lvds_config->port_swap = temp ? 1 : 0;
		else
			pConf->lcd_control.lvds_config->port_swap = temp ? 0 : 1;
		//pConf->lcd_control.lvds_config->port_reverse= ext_lcd->lcd_spc_val4;
		//pConf->lcd_control.lvds_config->lvds_fifo_wr_mode = ext_lcd->lcd_spc_val5;
	}

	pConf->lcd_power_ctrl.panel_power->gpio			= ext_lcd->panel_gpio;
	pConf->lcd_power_ctrl.panel_power->on_value		= ext_lcd->panel_on_value;
	pConf->lcd_power_ctrl.panel_power->off_value		= ext_lcd->panel_off_value;
	pConf->lcd_power_ctrl.panel_power->panel_on_delay 	= ext_lcd->panel_on_delay;
	pConf->lcd_power_ctrl.panel_power->panel_off_delay	= ext_lcd->panel_off_delay;

#ifdef CONFIG_AML_LCD_EXTERN
	pConf->lcd_control.ext_config->index  = ext_lcd->extern_index;
	pConf->lcd_control.ext_config->on_delay = ext_lcd->extern_on_delay;
	pConf->lcd_control.ext_config->off_delay = ext_lcd->extern_off_delay;
#endif

	lcd_printf("ext_lcd:h_active = %d \n",pConf->lcd_basic.h_active);
	lcd_printf("ext_lcd:v_active = %d \n",pConf->lcd_basic.v_active);
	lcd_printf("ext_lcd:h_period = %d \n",pConf->lcd_basic.h_period);
	lcd_printf("ext_lcd:v_period = %d \n",pConf->lcd_basic.v_period);
	lcd_printf("ext_lcd:lcd_type = %d \n",pConf->lcd_basic.lcd_type);
	if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_VBYONE) {
		lcd_printf("ext_lcd:lane_count = %d \n", pConf->lcd_control.vbyone_config->lane_count);
		lcd_printf("ext_lcd:byte_mode = %d \n", pConf->lcd_control.vbyone_config->byte_mode);
		lcd_printf("ext_lcd:region_num = %d \n", pConf->lcd_control.vbyone_config->region_num);
		lcd_printf("ext_lcd:color_fmt = %d \n", pConf->lcd_control.vbyone_config->color_fmt);
	} else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_LVDS) {
		lcd_printf("ext_lcd:lcd_bits = %d \n", pConf->lcd_control.lvds_config->lvds_bits);
		lcd_printf("ext_lcd:lvds_repack = %d \n", pConf->lcd_control.lvds_config->lvds_repack);
		lcd_printf("ext_lcd:pn_swap = %d \n", pConf->lcd_control.lvds_config->pn_swap);
		lcd_printf("ext_lcd:dual_port = %d \n", pConf->lcd_control.lvds_config->dual_port);
		lcd_printf("ext_lcd:port_swap = %d \n", pConf->lcd_control.lvds_config->port_swap);
	}
	lcd_printf("ext_lcd:lcd_bits = %d \n",pConf->lcd_control.lvds_config->lvds_bits);

#ifdef CONFIG_AML_LCD_EXTERN
	lcd_printf("ext_lcd:extern_index = %d \n",pConf->lcd_control.ext_config->index);
	lcd_printf("ext_lcd:extern_on_delay = %d \n",pConf->lcd_control.ext_config->on_delay);
	lcd_printf("ext_lcd:extern_off_delay = %d \n",pConf->lcd_control.ext_config->off_delay);
#endif

	return 0;
}

static int _load_bl_config_from_dtd(char *dt_addr, Lcd_Bl_Config_t *bl_config)
{
#ifdef CONFIG_OF_LIBFDT
	int parent_offset;
	char* propdata;
	char *sel;
	char propname[30];
	int child_offset;

	lcd_printf("\n");
	parent_offset = fdt_path_offset(dt_addr, "/backlight");
	if (parent_offset < 0) {
		printf("lcd error: backlight init: not find /backlight node %s.\n",fdt_strerror(parent_offset));
		return 0;
	}

	char *panel_type = getenv("panel_type");
	if (panel_type == NULL) {
		printf("lcd error: no panel_type use defult lcd config\n ");
		return 0;
	}
	sel = strchr(panel_type,'_');
	sprintf(propname,"/backlight/%s%s","backlight", sel);
	child_offset = fdt_path_offset(dt_addr, propname);
	if (child_offset < 0) {
		printf("lcd error: dts: not find /backlight/%s  node %s.\n",panel_type,fdt_strerror(child_offset));
		return 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_en_gpio", NULL);
	int blacklight_power_pin;
	blacklight_power_pin = gpioname_to_pin(propdata);
	if (blacklight_power_pin<0) {
		printf("lcd error: wrong gpio number %s\n",propdata);	 //----------------//
		return 0;
	}
	if (propdata == NULL) {
		printf("lcd error: faild to get panel_power_pin\n");
		return 0;
	} else {
		bl_config->bl_power.gpio = blacklight_power_pin;

	}
	lcd_printf("dtd_bl:panel_power_pin: %s--%d \n",propdata,bl_config->bl_power.gpio);


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_en_gpio_on", NULL);
	if (propdata == NULL) {
		printf("lcd error: faild to get backlight/bl_en_gpio_on \n");
		return 0;
	} else {
		bl_config->bl_power.on_value	 = be32_to_cpup((u32*)propdata);
		bl_config->bl_power.off_value  = !( be32_to_cpup((u32*)propdata));
	}
	lcd_printf("dtd_bl:on_value = %d \n",bl_config->bl_power.on_value);
	lcd_printf("dtd_bl:off_value = %d \n",bl_config->bl_power.off_value);


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_power_on_delay", NULL);
	if (propdata == NULL) {
		printf("lcd error: faild to get backlight/bl_power_on_delay \n");
		return 0;
	} else {
		bl_config->bl_power.bl_on_delay = be32_to_cpup((u32*)propdata);
	}
	lcd_printf("dtd_bl:bl_on_delay = %d \n",bl_config->bl_power.bl_on_delay);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_power_off_delay", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_power_off_delay \n");
		 return 0;
	 } else {
		 bl_config->bl_power.bl_off_delay = be32_to_cpup((u32*)propdata);
	 }
	 lcd_printf("dtd_bl:bl_off_delay = %d \n",bl_config->bl_power.bl_off_delay);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_gpio", NULL);
	 unsigned int pwm_pin;
	 pwm_pin = gpioname_to_pin(propdata);
	 if (pwm_pin<0) {
		 printf("lcd error: wrong gpio number %s\n",propdata);	  //----------------//
		 return 0;
	 }
	 if (propdata == NULL) {
		 printf("lcd error: faild to get bl_pwm_gpio\n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.pwm_gpio = pwm_pin;
	 }
	 lcd_printf("dtd_bl:bl_pwm_gpio: %s--%d \n",propdata,bl_config->bl_pwm.pwm_gpio);

	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_on_delay", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_pwm_on_delay \n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.pwm_on_delay = be32_to_cpup((u32*)propdata);
	 }
	 lcd_printf("dtd_bl:pwm_on_delay = %d \n",bl_config->bl_pwm.pwm_on_delay);

	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_off_delay", NULL);
	 if (propdata == NULL) {
		printf("lcd error: faild to get backlight/bl_pwm_off_delay \n");
		return 0;
	 } else {
		bl_config->bl_pwm.pwm_off_delay = be32_to_cpup((u32*)propdata);
	 }
	 lcd_printf("dtd_bl:pwm_off_delay = %d \n",bl_config->bl_pwm.pwm_off_delay);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_port", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_pwm_port \n");
		 return 0;
	 } else {
		 if (strcmp(propdata, "PWM_A") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_A;
		 else if (strcmp(propdata, "PWM_B") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_B;
		 else if (strcmp(propdata, "PWM_C") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_C;
		 else if (strcmp(propdata, "PWM_D") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_D;
		 else if (strcmp(propdata, "PWM_E") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_E;
		 else if (strcmp(propdata, "PWM_F") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_F;
		 else if (strcmp(propdata, "PWM_VS") == 0)
			 bl_config->bl_pwm.pwm_port = BL_PWM_VS;
	 }
	 lcd_printf("dtd_bl:pwm_port = %d \n",bl_config->bl_pwm.pwm_port);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_positive", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/pwm_positive\n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.pwm_positive = be32_to_cpup((u32*)propdata);
	 }
	 lcd_printf("dtd_bl:pwm_positive = %d \n",bl_config->bl_pwm.pwm_positive);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_freq", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_pwm_freq \n");
		 if (bl_config->bl_pwm.pwm_port == BL_PWM_VS)
			 bl_config->bl_pwm.pwm_freq = AML_BL_FREQ_VS_DEF;
		 else
			 bl_config->bl_pwm.pwm_freq = AML_BL_FREQ_DEF;
	 } else {
		 bl_config->bl_pwm.pwm_freq = be32_to_cpup((u32*)propdata);
	 }
	 if (bl_config->bl_pwm.pwm_port == BL_PWM_VS) {
		 if (bl_config->bl_pwm.pwm_freq > 4) {
			 printf("lcd error: faild to get backlight/pwm_positive\n");
			 bl_config->bl_pwm.pwm_freq = AML_BL_FREQ_VS_DEF;
		 }
		 lcd_printf("dtd_bl:pwm_vs_freq = %d x vfreq\n", bl_config->bl_pwm.pwm_freq);
	 } else {
		 if (bl_config->bl_pwm.pwm_freq > XTAL_HALF_FREQ_HZ)
			bl_config->bl_pwm.pwm_freq = XTAL_HALF_FREQ_HZ;
		 lcd_printf("dtd_bl:pwm_freq = %d \n", bl_config->bl_pwm.pwm_freq);
	 }

	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_pwm_duty_max_min", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_pwm_duty_max_min \n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.pwm_duty_max = be32_to_cpup((u32*)propdata);
		 bl_config->bl_pwm.pwm_duty_min = be32_to_cpup((((u32*)propdata)+1));
	 }
	 lcd_printf("dtd_bl:pwm_duty_max = %d \n",bl_config->bl_pwm.pwm_duty_max);
	 lcd_printf("dtd_bl:pwm_duty_min = %d \n",bl_config->bl_pwm.pwm_duty_min);

	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_level_default_uboot", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_level_default_uboot\n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.level_default = be32_to_cpup((u32*)propdata);
	 }
	 lcd_printf("dtd_bl:level_default = %d \n",bl_config->bl_pwm.level_default);


	 propdata = (char *)fdt_getprop(dt_addr, child_offset, "bl_level_max_min", NULL);
	 if (propdata == NULL) {
		 printf("lcd error: faild to get backlight/bl_level_max_min \n");
		 return 0;
	 } else {
		 bl_config->bl_pwm.level_max = be32_to_cpup((u32*)propdata);
		 bl_config->bl_pwm.level_min = be32_to_cpup((((u32*)propdata)+1));
	 }
	 lcd_printf("dtd_bl:level_max = %d \n",bl_config->bl_pwm.level_max);
	 lcd_printf("dtd_bl:level_min = %d \n",bl_config->bl_pwm.level_min);

	int len = 0;
	unsigned int i = 0;
	if (bl_config->bl_pwm.pwm_port == BL_PWM_VS)
		parent_offset = fdt_path_offset(dt_addr, "/pinmux/bl_pwm_vs_pins");
	else
		parent_offset = fdt_path_offset(dt_addr, "/pinmux/bl_pwm_pins");
	if (propdata == NULL) {
		printf("lcd error: backlight init: not find /pinmux/%s node %s\n",
			((bl_config->bl_pwm.pwm_port == BL_PWM_VS) ? "bl_pwm_vs_pins" : "bl_pwm_pins"),	fdt_strerror(parent_offset));
		return 0;
	} else {

		propdata = (char *)fdt_getprop(dt_addr, parent_offset, "amlogic,setmask", &len);
		if (propdata == NULL) {
			printf("lcd error: faild to get amlogic,setmask\n");
			return 0;
		} else {
			bl_config->bl_pwm.pinmux_set_num = len / 8;
			for (i=0; i<bl_config->bl_pwm.pinmux_set_num; i++) {
				bl_config->bl_pwm.pinmux_set[i][0] = be32_to_cpup((((u32*)propdata)+2*i));
				bl_config->bl_pwm.pinmux_set[i][1] = be32_to_cpup((((u32*)propdata)+2*i+1));
			}
		}

		propdata = (char *)fdt_getprop(dt_addr, parent_offset, "amlogic,clrmask", &len);
		if (propdata == NULL) {
			printf("lcd error: faild to get amlogic,clrmask\n");
			return 0;
		} else {
			bl_config->bl_pwm.pinmux_clr_num = len / 8;
			for (i=0; i<bl_config->bl_pwm.pinmux_clr_num; i++) {
				bl_config->bl_pwm.pinmux_clr[i][0] = be32_to_cpup((((u32*)propdata)+2*i));
				bl_config->bl_pwm.pinmux_clr[i][1] = be32_to_cpup((((u32*)propdata)+2*i+1));
			}

			for (i=0; i<bl_config->bl_pwm.pinmux_set_num; i++) {
				lcd_printf("backlight pinmux set %d: mux_num = %d, mux_mask = 0x%08x\n", i+1, bl_config->bl_pwm.pinmux_set[i][0], bl_config->bl_pwm.pinmux_set[i][1]);
			}
			for (i=0; i<bl_config->bl_pwm.pinmux_clr_num; i++) {
				lcd_printf("backlight pinmux clr %d: mux_num = %d, mux_mask = 0x%08x\n", i+1, bl_config->bl_pwm.pinmux_clr[i][0], bl_config->bl_pwm.pinmux_clr[i][1]);
			}

			return 1;
		}
	}

#endif

	return 0;
}


static int _load_bl_config_from_bsp(Lcd_Bl_Config_t *bl_config)
{
	Ext_Lcd_Config_t *ext_lcd = NULL;
	char *panel_type = getenv("panel_type");
	unsigned int i = 0;

	if (panel_type == NULL) {
		printf("lcd error: no panel_type use defult lcd config\n ");
		return 0;
	}
	for (i = 0 ; i < LCD_TYPE_MAX ; i++) {
		ext_lcd = &ext_lcd_config[i];
		if (strcmp(ext_lcd->panel_type, panel_type) == 0) {
			lcd_printf("ext_lcd:use panel_type = %s \n",ext_lcd->panel_type);
			break ;
		}
	}
	if ( i >= LCD_TYPE_MAX ) {
		printf("lcd error: out of range use defult bl config\n ");
		return 0;
	}else{
		bl_config->bl_power.gpio		 = ext_lcd->bl_gpio;
		bl_config->bl_power.on_value	 = ext_lcd->bl_on_value;
		bl_config->bl_power.off_value	 = ext_lcd->bl_off_value;
		bl_config->bl_power.bl_on_delay = ext_lcd->bl_on_delay;
		bl_config->bl_power.bl_off_delay= ext_lcd->bl_off_delay;

		bl_config->bl_pwm.pwm_port = ext_lcd->pwm_port;
		bl_config->bl_pwm.pwm_freq = ext_lcd->pwm_freq;
		bl_config->bl_pwm.pwm_duty_max = ext_lcd->pwm_duty_max;
		bl_config->bl_pwm.pwm_duty_min = ext_lcd->pwm_duty_min;
		bl_config->bl_pwm.pwm_positive = ext_lcd->pwm_positive;

		bl_config->bl_pwm.level_default = ext_lcd->level_default;
		bl_config->bl_pwm.level_min = ext_lcd->level_min;
		bl_config->bl_pwm.level_max = ext_lcd->level_max;

		lcd_printf("bl_config:bl_gpio = %d \n",bl_config->bl_power.gpio);
		lcd_printf("bl_config:on_value = %d \n",bl_config->bl_power.on_value);
		lcd_printf("bl_config:off_value = %d \n",bl_config->bl_power.off_value);
		lcd_printf("bl_config:bl_on_delay = %d \n",bl_config->bl_power.bl_on_delay);
		lcd_printf("bl_config:bl_off_delay = %d \n",bl_config->bl_power.bl_off_delay);

		lcd_printf("bl_config:pwm_port = %d \n",bl_config->bl_pwm.pwm_port);
		lcd_printf("bl_config:pwm_freq = %d \n",bl_config->bl_pwm.pwm_freq);
		lcd_printf("bl_config:pwm_duty_max = %d \n",bl_config->bl_pwm.pwm_duty_max);
		lcd_printf("bl_config:pwm_duty_min = %d \n",bl_config->bl_pwm.pwm_duty_min);
		lcd_printf("bl_config:pwm_positive = %d \n",bl_config->bl_pwm.pwm_positive);

		lcd_printf("bl_config:level_default = %d \n",bl_config->bl_pwm.level_default);
		lcd_printf("bl_config:level_min = %d \n",bl_config->bl_pwm.level_min);
		lcd_printf("bl_config:level_max = %d \n",bl_config->bl_pwm.level_max );

		return 1;
	}

	return 0;
}

static void lcd_tcon_config(Lcd_Config_t *pConf)
{
	unsigned short h_period, v_period, h_active, v_active;
	unsigned short hsync_bp, hsync_width, vsync_bp, vsync_width;
	unsigned short de_hstart, de_vstart;
	unsigned short hstart, hend, vstart, vend;

	h_period = pConf->lcd_basic.h_period;
	v_period = pConf->lcd_basic.v_period;
	h_active = pConf->lcd_basic.h_active;
	v_active = pConf->lcd_basic.v_active;
	hsync_bp = pConf->lcd_timing.hsync_bp;
	hsync_width = pConf->lcd_timing.hsync_width;
	vsync_bp = pConf->lcd_timing.vsync_bp;
	vsync_width = pConf->lcd_timing.vsync_width;

	de_hstart = h_period - h_active - 1;
	de_vstart = v_period - v_active;

	pConf->lcd_basic.video_on_pixel = de_hstart;
	pConf->lcd_basic.video_on_line = de_vstart;

	hstart = (de_hstart + h_period - hsync_bp - hsync_width) % h_period;
	hend = (de_hstart + h_period - hsync_bp) % h_period;
	pConf->lcd_timing.sth1_hs_addr = hstart;
	pConf->lcd_timing.sth1_he_addr = hend;
	pConf->lcd_timing.sth1_vs_addr = 0;
	pConf->lcd_timing.sth1_ve_addr = v_period - 1;

	pConf->lcd_timing.stv1_hs_addr = (hstart + h_period) % h_period;
	pConf->lcd_timing.stv1_he_addr = pConf->lcd_timing.stv1_hs_addr;
	vstart = (de_vstart + v_period - vsync_bp - vsync_width) % v_period;
	vend = (de_vstart + v_period - vsync_bp) % v_period;
	pConf->lcd_timing.stv1_vs_addr = vstart;
	pConf->lcd_timing.stv1_ve_addr = vend;

/*	lcd_print("hs_hs_addr=%d, hs_he_addr=%d, hs_vs_addr=%d, hs_ve_addr=%d\n"
		"vs_hs_addr=%d, vs_he_addr=%d, vs_vs_addr=%d, vs_ve_addr=%d\n",
		pConf->lcd_timing.sth1_hs_addr, pConf->lcd_timing.sth1_he_addr,
		pConf->lcd_timing.sth1_vs_addr, pConf->lcd_timing.sth1_ve_addr,
		pConf->lcd_timing.stv1_hs_addr, pConf->lcd_timing.stv1_he_addr,
		pConf->lcd_timing.stv1_vs_addr, pConf->lcd_timing.stv1_ve_addr);*/
}

/* change clock(frame_rate) for different vmode */
static int lcd_vmode_change(Lcd_Config_t *pConf, int index)
{
	unsigned int pclk;
	unsigned int h_period = pConf->lcd_basic.h_period;
	unsigned int v_period = pConf->lcd_basic.v_period;
	unsigned char type = pConf->lcd_timing.frame_rate_adj_type;
	unsigned int sync_duration_num = lcd_info[index].sync_duration_num;
	unsigned int sync_duration_den = lcd_info[index].sync_duration_den;

	/* init lcd pixel clock */
	pclk = h_period * v_period * 60;
	pConf->lcd_timing.lcd_clk = pclk;

	/* frame rate 60hz as default, no need adjust */
	if ((sync_duration_num / sync_duration_den) > 55)
		return 0;

	/* frame rate adjust */
	switch (type) {
	case 1: /* htotal adjust */
		h_period = ((pclk / v_period) * sync_duration_den * 10) / sync_duration_num;
		h_period = (h_period + 5) / 10; /* round off */
		printf("lcd: %s: adjust h_period %u -> %u\n",
			__func__, pConf->lcd_basic.h_period, h_period);
		pConf->lcd_basic.h_period = h_period;
		break;
	case 2: /* vtotal adjust */
		v_period = ((pclk / h_period) * sync_duration_den * 10) / sync_duration_num;
		v_period = (v_period + 5) / 10; /* round off */
		printf("lcd: %s: adjust v_period %u -> %u\n",
			__func__, pConf->lcd_basic.v_period, v_period);
		pConf->lcd_basic.v_period = v_period;
		break;
	case 0: /* pixel clk adjust */
	default:
		pclk = (h_period * v_period * sync_duration_num) / sync_duration_den;
		printf("lcd: %s: adjust pclk %u.%03uMHz -> %u.%03uMHz\n",
			__func__, (pConf->lcd_timing.lcd_clk / 1000000),
			((pConf->lcd_timing.lcd_clk / 1000) % 1000),
			(pclk / 1000000), ((pclk / 1000) % 1000));
		pConf->lcd_timing.lcd_clk = pclk;
		break;
	}

	return 0;
}

static void lcd_config_init(Lcd_Config_t *pConf)
{
	int vmode;

	vmode = _load_lcd_output_mode(pConf);
	lcd_vmode_change(pConf, vmode);

	if (pConf->version)
		lcd_tcon_config(pConf);
}

int get_lcd_config(Lcd_Config_t *pConf, Lcd_Bl_Config_t *bl_config)
{
	char *dt_addr = NULL;
	int load_id;
#ifdef CONFIG_AML_LCD_EXTERN
	int index;
#endif

	load_id = 0;
#ifdef CONFIG_OF_LIBFDT
#ifdef CONFIG_DT_PRELOAD
#ifdef CONFIG_DTB_LOAD_ADDR
	dt_addr = (char *)CONFIG_DTB_LOAD_ADDR;
#else
	dt_addr = (char *)0x0f000000;
#endif
	if (fdt_check_header(dt_addr) < 0)
		printf("lcd error: check dts: %s, load default lcd parameters\n", fdt_strerror(fdt_check_header(dt_addr)));
	else
		load_id = 1;
#endif
#endif

	if (load_id == 1 ) {
		printf("lcd: load config from dtd \n");
		_load_lcd_config_from_dtd(dt_addr, pConf);
		_load_bl_config_from_dtd(dt_addr, bl_config);
	} else {
		printf("lcd: load config from lcd.c \n");
		_load_lcd_config_from_bsp(pConf);
		_load_bl_config_from_bsp(bl_config);
	}
#ifdef CONFIG_AML_LCD_EXTERN
	index = pConf->lcd_control.ext_config->index;
	if (index < LCD_EXTERN_INDEX_INVALID)
		aml_lcd_extern_probe(dt_addr, index);
#endif
	lcd_config_init(pConf);

	return 0;
}
