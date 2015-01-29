/*
 * AMLOGIC backlight external driver.
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <amlogic/aml_bl_extern.h>
#include <amlogic/lcdoutc.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif
#ifdef CONFIG_PLATFORM_HAS_PMU
#include <amlogic/aml_pmu_common.h>
#endif

#ifdef CONFIG_AML_BL_EXTERN
//#define BL_EXT_DEBUG_INFO

#define BL_EXTERN_NAME			"bl_pmu_aml1218"
#define BL_EXTERN_TYPE			BL_EXTERN_OTHER

static unsigned int bl_status = 0;
static unsigned int bl_level = 0;
static struct aml_bl_extern_driver_t bl_ext_driver;

static struct bl_extern_config_t bl_ext_config = {
    .name = BL_EXTERN_NAME,
    .type = BL_EXTERN_TYPE,
    .gpio_used = 1,
    .gpio = GPIODV_28,
    .gpio_on = 1,
    .gpio_off = 0,
    .dim_min = 0x1b,
    .dim_max = 0x1,
};

static int bl_extern_set_level(unsigned int level)
{
#ifdef CONFIG_PLATFORM_HAS_PMU
    struct aml_pmu_driver *pmu_driver;
    unsigned char temp;
#endif
    int ret = 0;

    bl_level = level;
    if (bl_status) {
        get_bl_level(&bl_ext_config);
        level = bl_ext_config.dim_min - ((level - bl_ext_config.level_min) * (bl_ext_config.dim_min - bl_ext_config.dim_max)) / (bl_ext_config.level_max - bl_ext_config.level_min);
        level &= 0x1f;

#ifdef CONFIG_PLATFORM_HAS_PMU
        pmu_driver = aml_pmu_get_driver();
        if (pmu_driver == NULL) {
            printf("no pmu driver\n");
            return -1;
        }
        else {
            if ((pmu_driver->pmu_reg_write) && (pmu_driver->pmu_reg_read)) {
                ret = pmu_driver->pmu_reg_read(0x005f, &temp);
                temp &= ~(0x3f << 2);
                temp |= (level << 2);
                ret = pmu_driver->pmu_reg_write(0x005f, temp);
            }
            else {
                printf("no pmu_reg_read/write\n");
                return -1;
            }
        }
#endif
    }
    return ret;
}

static int bl_extern_power_on(void)
{
#ifdef CONFIG_PLATFORM_HAS_PMU
    struct aml_pmu_driver *pmu_driver;
    unsigned char temp;
#endif
    int ret = 0;

#ifdef CONFIG_PLATFORM_HAS_PMU
    pmu_driver = aml_pmu_get_driver();
    if (pmu_driver == NULL) {
        printf("no pmu driver\n");
        return -1;
    }
    else {
        if ((pmu_driver->pmu_reg_write) && (pmu_driver->pmu_reg_read)) {
            ret = pmu_driver->pmu_reg_read(0x005e, &temp);
            temp |= (1 << 7);
            ret = pmu_driver->pmu_reg_write(0x005e, temp);//DCEXT_IREF_ADJLV2_EN
        }
        else {
            printf("no pmu_reg_read/write\n");
            return -1;
        }
    }
#endif
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
    return ret;
}

static int bl_extern_power_off(void)
{
#ifdef CONFIG_AMLOGIC_BOARD_HAS_PMU
    struct aml_pmu_driver *pmu_driver;
    unsigned char temp;
#endif
    int ret = 0;

    bl_status = 0;
    if (bl_ext_config.gpio_used > 0) {
        if(bl_ext_config.gpio_off == 2)
            bl_extern_gpio_direction_input(bl_ext_config.gpio);
        else
            bl_extern_gpio_direction_output(bl_ext_config.gpio, bl_ext_config.gpio_off);
    }
#ifdef CONFIG_AMLOGIC_BOARD_HAS_PMU
    pmu_driver = aml_pmu_get_driver();
    if (pmu_driver == NULL) {
        printf("no pmu driver\n");
        return -1;
    }
    else {
        if ((pmu_driver->pmu_reg_write) && (pmu_driver->pmu_reg_read)) {
            ret = pmu_driver->pmu_reg_read(0x005e, &temp);
            temp &= ~(1 << 7);
            ret = pmu_driver->pmu_reg_write(0x005e, temp);//DCEXT_IREF_ADJLV2_EN
        }
        else {
            printf("no pmu_reg_read/write\n");
            return -1;
        }
    }
#endif

    printf("%s\n", __FUNCTION__);
    return ret;
}

#ifdef CONFIG_OF_LIBFDT
static int get_bl_extern_dt_data(char *dt_addr)
{
	int ret=0;
	int nodeoffset;
	char * propdata;
	struct fdt_property *prop;
	char *p;
	const char * str;
	struct bl_extern_config_t *bl_extern = &bl_ext_config;

	nodeoffset = fdt_path_offset(dt_addr, "/bl_extern_pmu_aml1218");
	if(nodeoffset < 0) {
		printf("dts: not find /bl_extern_pmu_aml1218 node %s.\n",fdt_strerror(nodeoffset));
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
	printf("bl_extern_gpio = %d, gpio_on = %d, gpio_off = %d\n", bl_extern->gpio,bl_extern->gpio_on,bl_extern->gpio_off);
	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "dim_max_min", NULL);
	if (propdata == NULL) {
		printf("faild to get dim_max_min\n");
		bl_extern->dim_min = 0x1b;
		bl_extern->dim_max = 0x1;
	}
	else {
		bl_extern->dim_max = (be32_to_cpup((u32*)propdata));
		bl_extern->dim_min = (be32_to_cpup((((u32*)propdata)+1)));
	}
	printf("bl_extern_dim_min =%x, bl_extern_dim_max =%x\n", bl_extern->dim_min,bl_extern->dim_max);
	return ret;
}
#endif

static int get_bl_extern_config(void)
{
    int ret = 0;

#ifdef CONFIG_OF_LIBFDT
    if (bl_ext_driver.dt_addr) {
        ret = fdt_check_header(bl_ext_driver.dt_addr);
        if(ret < 0) {
            printf("check dts: %s, load bl_ext_config failed\n", fdt_strerror(ret));
        }
        else {
            get_bl_extern_dt_data(bl_ext_driver.dt_addr);
        }
    }
#endif

    if (bl_ext_config.dim_min > 0x1f)
        bl_ext_config.dim_min = 0x1f;
    if (bl_ext_config.dim_max > 0x1f)
        bl_ext_config.dim_max = 0x1f;

    return ret;
}

static struct aml_bl_extern_driver_t bl_ext_driver = {
    .name = BL_EXTERN_NAME,
    .type = BL_EXTERN_TYPE,
    .power_on = bl_extern_power_on,
    .power_off = bl_extern_power_off,
    .set_level = bl_extern_set_level,
    .dt_addr = NULL,
    .get_bl_ext_config = get_bl_extern_config,
};

struct aml_bl_extern_driver_t* aml_bl_extern_get_driver(void)
{
    return &bl_ext_driver;
}

#endif
