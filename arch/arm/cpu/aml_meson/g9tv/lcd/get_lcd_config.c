#include <common.h>
//#include <malloc.h>
//#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>

#include <amlogic/gpio.h>


#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>

static const char* lcd_type_table[]={
	"lvds",
	"vbyone",
	"ttl",
	"invalid",
};
#endif

static int _load_lcd_config_from_dtd(Lcd_Config_t *pConf)
{
#ifdef CONFIG_OF_LIBFDT
	static char * dt_addr;
	int parent_offset;
	int child_offset;
	char propname[30];
	char* propdata;
	int i;

#ifdef CONFIG_DT_PRELOAD
#ifdef CONFIG_DTB_LOAD_ADDR
	dt_addr = (char *)CONFIG_DTB_LOAD_ADDR;
#else
	dt_addr = (char *)0x0f000000;
#endif
	if (fdt_check_header(dt_addr) < 0) {
		printf("check dts: %s, load default lcd parameters\n", fdt_strerror(fdt_check_header(dt_addr)));
		return 0;
	}
#endif

	parent_offset = fdt_path_offset(dt_addr, "/lcd");
	if (parent_offset < 0) {
		printf("dts: not find /lcd node %s.\n",fdt_strerror(parent_offset));
		return 0;
	}

	char *panel_type = getenv("panel_type");
	if (panel_type == NULL) {
		printf("no panel_type use defult lcd config\n ");
		return 0;
	}
	sprintf(propname, "/lcd/%s", panel_type);
	child_offset = fdt_path_offset(dt_addr, propname);
	if (child_offset < 0) {
		printf("dts: not find /lcd/%s  node %s.\n",panel_type,fdt_strerror(child_offset));
		return 0;
	}


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "interface", NULL);
	if (propdata == NULL) {
		printf("faild to get interface!\n");
		return 0;
	}
	else {
		for (i = 0; i < LCD_TYPE_MAX; i++) {
			if (!strncmp(propdata, lcd_type_table[i], 3))
				break;
		}
		pConf->lcd_basic.lcd_type = i;
	}
	printf("dtd_lcd: lcd_type = %s(%u)\n", lcd_type_table[pConf->lcd_basic.lcd_type], pConf->lcd_basic.lcd_type);


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "basic_setting", NULL);
	if (propdata == NULL) {
		printf("faild to get basic_setting\n");
		return 0;
	}
	else {
		pConf->lcd_basic.h_active = be32_to_cpup((u32*)propdata);
		pConf->lcd_basic.v_active = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_basic.h_period = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_basic.v_period = be32_to_cpup((((u32*)propdata)+3));
		pConf->lcd_basic.video_on_pixel = be32_to_cpup((((u32*)propdata)+4));
		pConf->lcd_basic.video_on_line  = be32_to_cpup((((u32*)propdata)+5));
	}
	printf("dtd_lcd:h_active = %d \n",pConf->lcd_basic.h_active);
	printf("dtd_lcd:v_active = %d \n",pConf->lcd_basic.v_active);
	printf("dtd_lcd:h_period = %d \n",pConf->lcd_basic.h_period);
	printf("dtd_lcd:v_period = %d \n",pConf->lcd_basic.v_period);
	printf("dtd_lcd:video_on_pixel = %d \n",pConf->lcd_basic.video_on_pixel);
	printf("dtd_lcd:video_on_line = %d \n",pConf->lcd_basic.video_on_line);


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "lcd_timing", NULL);
	if (propdata == NULL) {
		printf("faild to get basic_setting\n");
		return 0;
	}
	else {
		pConf->lcd_timing.hpll_clk = be32_to_cpup((u32*)propdata);
		pConf->lcd_timing.hpll_od  = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_timing.hdmi_pll_cntl5 = be32_to_cpup((((u32*)propdata)+2));

		pConf->lcd_timing.sth1_hs_addr   = be32_to_cpup((((u32*)propdata)+3));
		pConf->lcd_timing.sth1_he_addr   = be32_to_cpup((((u32*)propdata)+4));
		pConf->lcd_timing.sth1_vs_addr   = be32_to_cpup((((u32*)propdata)+5));
		pConf->lcd_timing.sth1_ve_addr   = be32_to_cpup((((u32*)propdata)+6));
		pConf->lcd_timing.stv1_hs_addr   = be32_to_cpup((((u32*)propdata)+7));
		pConf->lcd_timing.stv1_he_addr   = be32_to_cpup((((u32*)propdata)+8));
		pConf->lcd_timing.stv1_vs_addr   = be32_to_cpup((((u32*)propdata)+9));
		pConf->lcd_timing.stv1_ve_addr   = be32_to_cpup((((u32*)propdata)+10));
	}

	printf("dtd_lcd:hpll_clk = %x \n",pConf->lcd_timing.hpll_clk);
	printf("dtd_lcd:hpll_od = %x \n",pConf->lcd_timing.hpll_od);
	printf("dtd_lcd:hdmi_pll_cntl5 = %x \n",pConf->lcd_timing.hdmi_pll_cntl5);
	printf("dtd_lcd:sth1_hs_addr = %d \n",pConf->lcd_timing.sth1_hs_addr);
	printf("dtd_lcd:sth1_he_addr = %d \n",pConf->lcd_timing.sth1_he_addr);
	printf("dtd_lcd:sth1_vs_addr = %d \n",pConf->lcd_timing.sth1_vs_addr);
	printf("dtd_lcd:sth1_ve_addr = %d \n",pConf->lcd_timing.sth1_ve_addr);
	printf("dtd_lcd:stv1_hs_addr = %d \n",pConf->lcd_timing.stv1_hs_addr);
	printf("dtd_lcd:stv1_he_addr = %d \n",pConf->lcd_timing.stv1_he_addr);
	printf("dtd_lcd:stv1_vs_addr = %d \n",pConf->lcd_timing.stv1_vs_addr);
	printf("dtd_lcd:stv1_ve_addr = %d \n",pConf->lcd_timing.stv1_ve_addr);

	if (((char *)fdt_getprop(dt_addr, child_offset, "lvds_att", NULL)) ||
		((char *)fdt_getprop(dt_addr, child_offset, "vbyone_att", NULL))||
		((char *)fdt_getprop(dt_addr, child_offset, "ttl_att", NULL))) {
		if (((char *)fdt_getprop(dt_addr, child_offset, "lvds_att", NULL))) {
			propdata = (char *)fdt_getprop(dt_addr, child_offset, "lvds_att", NULL);
			pConf->lcd_control.lvds_config->lvds_bits	 = be32_to_cpup((u32*)propdata);
			pConf->lcd_control.lvds_config->lvds_repack = be32_to_cpup((((u32*)propdata)+1));
			pConf->lcd_control.lvds_config->pn_swap     = be32_to_cpup((((u32*)propdata)+2));
			pConf->lcd_control.lvds_config->dual_port   = be32_to_cpup((((u32*)propdata)+3));
			pConf->lcd_control.lvds_config->port_reverse= be32_to_cpup((((u32*)propdata)+4));
			pConf->lcd_control.lvds_config->lvds_fifo_wr_mode = be32_to_cpup((((u32*)propdata)+5));

			printf("dtd_lcd:lvds_bits = %d \n",pConf->lcd_control.lvds_config->lvds_bits);
			printf("dtd_lcd:lvds_repack = %d \n",pConf->lcd_control.lvds_config->lvds_repack);
			printf("dtd_lcd:pn_swap = %d \n",pConf->lcd_control.lvds_config->pn_swap);
			printf("dtd_lcd:dual_port = %d \n",pConf->lcd_control.lvds_config->dual_port);
			printf("dtd_lcd:port_reverse = %d \n",pConf->lcd_control.lvds_config->port_reverse);
			printf("dtd_lcd:lvds_fifo_wr_mode = %d \n",pConf->lcd_control.lvds_config->lvds_fifo_wr_mode);
		}else if (((char *)fdt_getprop(dt_addr, child_offset, "vbyone_att", NULL))) {
			propdata = (char *)fdt_getprop(dt_addr, child_offset, "vbyone_att", NULL);
			pConf->lcd_control.vbyone_config->lane_count	= be32_to_cpup((u32*)propdata);
			pConf->lcd_control.vbyone_config->byte			= be32_to_cpup((((u32*)propdata)+1));
			pConf->lcd_control.vbyone_config->region		= be32_to_cpup((((u32*)propdata)+2));
			pConf->lcd_control.vbyone_config->color_fmt		= be32_to_cpup((((u32*)propdata)+3));

			printf("dtd_lcd:lane_count = %d \n",pConf->lcd_control.vbyone_config->lane_count);
			printf("dtd_lcd:byte = %d \n",pConf->lcd_control.vbyone_config->byte);
			printf("dtd_lcd:region = %d \n",pConf->lcd_control.vbyone_config->region);
			printf("dtd_lcd:color_fmt = %d \n",pConf->lcd_control.vbyone_config->color_fmt);
		}else{
			printf("this is ttl att \n");


		}
	}else{
		printf("faild to get basic_setting\n");
		return 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "panel_power_pin", NULL);
	int panel_power_pin;
	panel_power_pin = gpioname_to_pin(propdata);
	if (panel_power_pin<0) {
		printf("wrong gpio number %s\n",propdata);   //----------------//
		return 0;
	}
	if (propdata == NULL) {
		printf("faild to get panel_power_pin\n");
		return 0;
	}
	else {
		pConf->lcd_power_ctrl.panel_power->gpio	= panel_power_pin;
	}
	printf("dtd_lcd:panel_power_pin: %s--%d \n",propdata,pConf->lcd_power_ctrl.panel_power->gpio);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "panel_power_att", NULL);
	if (propdata == NULL) {
		printf("faild to get basic_setting\n");
		return 0;
	}
	else {
		pConf->lcd_power_ctrl.panel_power->on_value	 = be32_to_cpup((u32*)propdata);
		pConf->lcd_power_ctrl.panel_power->off_value = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_power_ctrl.panel_power->panel_on_delay = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_power_ctrl.panel_power->panel_off_delay = be32_to_cpup((((u32*)propdata)+3));
	}
	printf("dtd_lcd:on_value = %d \n",pConf->lcd_power_ctrl.panel_power->on_value);
	printf("dtd_lcd:off_value = %d \n",pConf->lcd_power_ctrl.panel_power->off_value);
	printf("dtd_lcd:panel_on_delay = %d \n",pConf->lcd_power_ctrl.panel_power->panel_on_delay);
	printf("dtd_lcd:panel_off_delay = %d \n",pConf->lcd_power_ctrl.panel_power->panel_off_delay);


	propdata = (char *)fdt_getprop(dt_addr, child_offset, "blacklight_power_pin", NULL);
	int blacklight_power_pin;
	blacklight_power_pin = gpioname_to_pin(propdata);
	if (blacklight_power_pin<0) {
		printf("wrong gpio number %s\n",propdata);	 //----------------//
		return 0;
	}
	if (propdata == NULL) {
		printf("faild to get panel_power_pin\n");
		return 0;
	}
	else {
		pConf->lcd_power_ctrl.bl_power->gpio = blacklight_power_pin;
	}
	printf("dtd_lcd:panel_power_pin: %s--%d \n",propdata,pConf->lcd_power_ctrl.bl_power->gpio);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "blacklight_power_att", NULL);
	if (propdata == NULL) {
		printf("faild to get basic_setting\n");
		return 0;
	}
	else {
		pConf->lcd_power_ctrl.bl_power->on_value  = be32_to_cpup((u32*)propdata);
		pConf->lcd_power_ctrl.bl_power->off_value = be32_to_cpup((((u32*)propdata)+1));
		pConf->lcd_power_ctrl.bl_power->bl_on_delay = be32_to_cpup((((u32*)propdata)+2));
		pConf->lcd_power_ctrl.bl_power->bl_off_delay = be32_to_cpup((((u32*)propdata)+3));

		printf("dtd_lcd:on_value = %d \n",pConf->lcd_power_ctrl.bl_power->on_value);
		printf("dtd_lcd:off_value = %d \n",pConf->lcd_power_ctrl.bl_power->off_value);
		printf("dtd_lcd:bl_on_delay = %d \n",pConf->lcd_power_ctrl.bl_power->bl_on_delay);
		printf("dtd_lcd:bl_off_delay = %d \n",pConf->lcd_power_ctrl.bl_power->bl_off_delay);
		return 1;
	}
#endif

	return 0;
}

extern Ext_Lcd_Config_t ext_lcd_config[LCD_TYPE_MAX];

static int _load_lcd_config_from_bsp(Lcd_Config_t *pConf)
{
	Ext_Lcd_Config_t *ext_config = NULL;
	char *panel_type = getenv("panel_type");
	unsigned int i = 0;

	if (panel_type == NULL) {
		printf("no panel_type use defult lcd config\n ");
		return 0;
	}
	for (i = 0 ; i < LCD_TYPE_MAX ; i++) {
		ext_config = &ext_lcd_config[i];
		if (strcmp(ext_config->panel_type, panel_type) == 0) {
			printf("ext_config:use panel_type = %s \n",ext_config->panel_type);
			break ;
		}
	}
	if ( i >= LCD_TYPE_MAX ) {
		printf("error: out of range use defult lcd config\n ");
		return 0;
	}else{
		pConf->lcd_basic.lcd_type = ext_config->lcd_type;
		pConf->lcd_basic.h_active = ext_config->h_active;
		pConf->lcd_basic.v_active = ext_config->v_active;
		pConf->lcd_basic.h_period = ext_config->h_period;
		pConf->lcd_basic.v_period = ext_config->v_period;
		pConf->lcd_basic.video_on_pixel = ext_config->video_on_pixel;
		pConf->lcd_basic.video_on_line  = ext_config->video_on_line;

		pConf->lcd_timing.hpll_clk = ext_config->hpll_clk;
		pConf->lcd_timing.hpll_od  = ext_config->hpll_od;
		pConf->lcd_timing.hdmi_pll_cntl5 = ext_config->hdmi_pll_cntl5;

		pConf->lcd_timing.sth1_hs_addr   = ext_config->sth1_hs_addr;
		pConf->lcd_timing.sth1_he_addr   = ext_config->sth1_he_addr;
		pConf->lcd_timing.sth1_vs_addr   = ext_config->sth1_vs_addr;
		pConf->lcd_timing.sth1_ve_addr   = ext_config->sth1_ve_addr;
		pConf->lcd_timing.stv1_hs_addr   = ext_config->stv1_hs_addr;
		pConf->lcd_timing.stv1_he_addr   = ext_config->stv1_he_addr;
		pConf->lcd_timing.stv1_vs_addr   = ext_config->stv1_vs_addr;
		pConf->lcd_timing.stv1_ve_addr   = ext_config->stv1_ve_addr;

		pConf->lcd_power_ctrl.panel_power->gpio				= ext_config->panel_gpio;
		pConf->lcd_power_ctrl.panel_power->on_value			= ext_config->panel_on_value;
		pConf->lcd_power_ctrl.panel_power->off_value		= ext_config->panel_off_value;
		pConf->lcd_power_ctrl.panel_power->panel_on_delay 	= ext_config->panel_on_delay;
		pConf->lcd_power_ctrl.panel_power->panel_off_delay	= ext_config->panel_off_delay;

		pConf->lcd_power_ctrl.bl_power->gpio        = ext_config->bl_gpio;
		pConf->lcd_power_ctrl.bl_power->on_value    = ext_config->bl_on_value;
		pConf->lcd_power_ctrl.bl_power->off_value   = ext_config->bl_off_value;
		pConf->lcd_power_ctrl.bl_power->bl_on_delay = ext_config->bl_on_delay;
		pConf->lcd_power_ctrl.bl_power->bl_off_delay= ext_config->bl_off_delay;

		if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_VBYONE) {
			pConf->lcd_control.vbyone_config->lane_count	= ext_config->lcd_spc_val0;
			pConf->lcd_control.vbyone_config->byte			= ext_config->lcd_spc_val1;
			pConf->lcd_control.vbyone_config->region		= ext_config->lcd_spc_val2;
			pConf->lcd_control.vbyone_config->color_fmt		= ext_config->lcd_spc_val3;
		}else if (pConf->lcd_basic.lcd_type == LCD_DIGITAL_TTL) {
			printf("this is ttl att \n");
		}else{
			pConf->lcd_control.lvds_config->lvds_bits	 = ext_config->lcd_spc_val0;
			pConf->lcd_control.lvds_config->lvds_repack = ext_config->lcd_spc_val1;
			pConf->lcd_control.lvds_config->pn_swap     = ext_config->lcd_spc_val2;
			pConf->lcd_control.lvds_config->dual_port   = ext_config->lcd_spc_val3;
			pConf->lcd_control.lvds_config->port_reverse= ext_config->lcd_spc_val4;
			pConf->lcd_control.lvds_config->lvds_fifo_wr_mode = ext_config->lcd_spc_val5;
		}

	/*
		printf("ext_config:h_active = %d \n",pConf->lcd_basic.h_active);
		printf("ext_config:v_active = %d \n",pConf->lcd_basic.v_active);
		printf("ext_config:h_period = %d \n",pConf->lcd_basic.h_period);
		printf("ext_config:v_period = %d \n",pConf->lcd_basic.v_period);
		printf("ext_config:lcd_type = %d \n",pConf->lcd_basic.lcd_type);
		printf("ext_config:lcd_bits = %d \n",pConf->lcd_control.lvds_config->lvds_bits);
		printf("ext_config:panel_gpio = %d \n",pConf->lcd_power_ctrl.panel_power->gpio);
		printf("ext_config:bl_gpio = %d  \n",pConf->lcd_power_ctrl.bl_power->gpio);
	*/

		return 1;
	}

	return 0;
}

static int _find_lcd_config(Lcd_Config_t *pConf)
{
	static int load_id = 1;

	if (load_id == 1 ) {
		return _load_lcd_config_from_dtd(pConf); //usr dtd lcd config
	}else if (load_id == 2) {
		return _load_lcd_config_from_bsp(pConf); //usr lcd.c lcd config
	}else
		return 0; //usr defult lcd config
}

int reset_lcd_config(Lcd_Config_t *pConf)
{
	if (_find_lcd_config(pConf))
		return 1;
	else
		return 0; //usr defult lcd config
}
