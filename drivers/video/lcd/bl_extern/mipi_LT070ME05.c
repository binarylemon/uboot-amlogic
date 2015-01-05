/*
 * AMLOGIC backlight external driver.
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/aml_bl_extern.h>
#include <amlogic/lcdoutc.h>
#ifdef CONFIG_AML_BL_EXTERN
#ifdef CONFIG_LCD_IF_MIPI_VALID

//#define BL_EXT_DEBUG_INFO

#define BL_EXTERN_NAME			"bl_mipi_LT070ME05"
#define BL_EXTERN_TYPE			BL_EXTERN_OTHER

static unsigned int bl_status = 0;
static unsigned int bl_level = 0;

static struct bl_extern_config_t bl_ext_config = {
    .name = BL_EXTERN_NAME,
    .type = BL_EXTERN_TYPE,
    .gpio_used = 1,
    .gpio = GPIODV_28,
    .gpio_on = 1,
    .gpio_off = 0,
    .dim_min = 10,
    .dim_max = 255,
};

#ifdef CONFIG_OF_LIBFDT
static int get_bl_ext_config (char *dt_addr)
{
		int ret=0;
		int nodeoffset;
		char * propdata;
		int i;
		struct fdt_property *prop;
		char *p;
		const char * str;
		struct bl_extern_config_t *bl_extern = &bl_ext_config;

		nodeoffset = fdt_path_offset(dt_addr, "/bl_extern_mipi_LT070ME05");
		if(nodeoffset < 0) {
			printf("dts: not find /bl_extern_mipi_LT070ME05 node %s.\n",fdt_strerror(nodeoffset));
		return ret;
	}
	
		propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "gpio_enable_on_off", NULL);
		if (propdata == NULL) {
			printf("faild to get gpio_enable_on_off\n");
			bl_extern->gpio_used = 1;
#ifdef GPIODV_28
			bl_extern->gpio = GPIODV_28;
#endif
#ifdef GPIOD_1
			bl_extern->gpio = GPIOD_1;
#endif
			bl_extern->gpio_on = LCD_POWER_GPIO_OUTPUT_HIGH;
			bl_extern->gpio_off = LCD_POWER_GPIO_OUTPUT_LOW;
		}
		else {
			prop = container_of(propdata, struct fdt_property, data);
			p = prop->data;
			str = p;
			bl_extern->gpio_used = 1;
			bl_extern->gpio = aml_lcd_gpio_name_map_num(p);
			p += strlen(p) + 1;
			str = p;
			if (strncmp(str, "2", 1) == 0)
				bl_extern->gpio_on = LCD_POWER_GPIO_INPUT;
			else if(strncmp(str, "0", 1) == 0)
				bl_extern->gpio_on = LCD_POWER_GPIO_OUTPUT_LOW;
			else
				bl_extern->gpio_on = LCD_POWER_GPIO_OUTPUT_HIGH;	
			p += strlen(p) + 1;
			str = p;
			if (strncmp(str, "2", 1) == 0)
				bl_extern->gpio_off = LCD_POWER_GPIO_INPUT;
			else if(strncmp(str, "1", 1) == 0)
				bl_extern->gpio_off = LCD_POWER_GPIO_OUTPUT_HIGH;
			else
				bl_extern->gpio_off = LCD_POWER_GPIO_OUTPUT_LOW;
		}
		printf("bl_extern_gpio = %d, bl_extern_gpio_on = %d ,bl_extern_gpio_off= %d\n", bl_extern->gpio,bl_extern->gpio_on,bl_extern->gpio_off);
		propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "dim_max_min", NULL);
		if (propdata == NULL) {
			printf("faild to get dim_max_min\n");
			bl_extern->dim_min = 10;
			bl_extern->dim_max = 255;
		}
		else {
			bl_extern->dim_max = (be32_to_cpup((u32*)propdata));
			bl_extern->dim_min = (be32_to_cpup((((u32*)propdata)+1)));
		}
		printf("bl_extern_dim_min =%x, bl_extern_dim_max =%x\n", bl_extern->dim_min,bl_extern->dim_max);
		return ret;
}
#endif

//******************** mipi command ********************//
//format:  data_type, num, data....
//special: data_type=0xff, num<0xff means delay ms, num=0xff means ending.
//******************************************************//
static int bl_extern_set_level(unsigned int level)
{
    unsigned char payload[6]={0x15,2,0x51,0x00,0xff,0xff};

    bl_level = level;
    if (bl_status) {
        get_bl_level(&bl_ext_config);
        level = bl_ext_config.dim_min - ((level - bl_ext_config.level_min) * (bl_ext_config.dim_min - bl_ext_config.dim_max)) / (bl_ext_config.level_max - bl_ext_config.level_min);
        level &= 0xff;

        payload[3] = level;
        dsi_write_cmd(payload);
    }
    return 0;
}

static int bl_extern_power_on(void)
{
    unsigned char temp;
    int ret;

    if (bl_ext_config.gpio_used > 0) {
        if(bl_ext_config.gpio_on == 2)
    	  		bl_extern_gpio_direction_input(bl_ext_config.gpio);
    	  else
        		bl_extern_gpio_direction_output(bl_ext_config.gpio, bl_ext_config.gpio_on);
    }
    bl_status = 1;

    if (bl_level > 0) {
        bl_extern_set_level(bl_level);
    }

    printf("%s\n", __FUNCTION__);
    return 0;
}

static int bl_extern_power_off(void)
{
    bl_status = 0;
    if (bl_ext_config.gpio_used > 0) {
        if(bl_ext_config.gpio_off == 2)
    	  		bl_extern_gpio_direction_input(bl_ext_config.gpio);
    	  else		
        		bl_extern_gpio_direction_output(bl_ext_config.gpio, bl_ext_config.gpio_off);
    }

    printf("%s\n", __FUNCTION__);
    return 0;
}

static struct aml_bl_extern_driver_t bl_ext_driver = {
    .name = BL_EXTERN_NAME,
    .type = BL_EXTERN_TYPE,
    .power_on = bl_extern_power_on,
    .power_off = bl_extern_power_off,
    .set_level = bl_extern_set_level,
#ifdef CONFIG_OF_LIBFDT
    .get_bl_ext_config = get_bl_ext_config,
#endif
};

struct aml_bl_extern_driver_t* aml_bl_extern_get_driver(void)
{
    return &bl_ext_driver;
}

#endif
#endif
