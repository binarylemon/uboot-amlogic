#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/lcdoutc.h>
#include <asm/arch/i2c.h>
#include <asm/arch/aml_lcd_gpio.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif
#include <amlogic/aml_lcd_extern.h>
#include "lcd_extern.h"

static char *dt_addr = NULL;

/* only probe one extern driver for uboot */
static struct aml_lcd_extern_driver_t *lcd_ext_driver;

struct aml_lcd_extern_driver_t *aml_lcd_extern_get_driver(void)
{
	if (lcd_ext_driver == NULL)
		LCD_EXT_PR("invalid driver\n");
	return lcd_ext_driver;
}

#ifdef CONFIG_OF_LIBFDT
char *aml_lcd_extern_get_dt_prop(int nodeoffset, char *propname)
{
	char *propdata;

	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, propname, NULL);
	return propdata;
}

int aml_lcd_extern_get_dt_child(int index)
{
	int nodeoffset;
	char chlid_node[30];
	char *propdata;

	sprintf(chlid_node, "/lcd_extern/extern_%d", index);
	nodeoffset = fdt_path_offset(dt_addr, chlid_node);
	if (nodeoffset < 0) {
		LCD_EXT_PR("dts: not find  node %s\n", chlid_node);
		return nodeoffset;
	}

	propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "index", NULL);
	if (propdata == NULL) {
		LCD_EXT_PR("get index failed, exit\n");
		return -1;
	} else {
		if (be32_to_cpup((u32*)propdata) != index) {
			LCD_EXT_PR("index not match, exit\n");
			return -1;
		}
	}
	return nodeoffset;
}

static int aml_lcd_extern_get_dt_config(char *dtaddr, int index, struct lcd_extern_config_t *econfig)
{
	int val;
	int nodeoffset;
	char *propdata;
	const char *str;

	nodeoffset = aml_lcd_extern_get_dt_child(index);
	if (nodeoffset < 0)
		return -1;

	propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "index", NULL);
	if (propdata == NULL) {
		econfig->index = LCD_EXTERN_INDEX_INVALID;
		LCD_EXT_PR("get index failed, exit\n");
		return -1;
	} else {
		econfig->index = be32_to_cpup((u32*)propdata);
	}

	propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "extern_name", NULL);
	if (propdata == NULL) {
		str = "invalid_name";
		strcpy(econfig->name, str);
		LCD_EXT_PR("get extern_name failed\n");
	} else {
		memset(econfig->name, 0, LCD_EXTERN_NAME_LEN_MAX);
		strcpy(econfig->name, propdata);
	}
	LCD_EXT_PR("load config in dtb: %s[%d]\n", econfig->name, econfig->index);

	propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "type", NULL);
	if (propdata == NULL) {
		econfig->type = LCD_EXTERN_MAX;
		LCD_EXT_PR("error: get type failed, exit\n");
		return -1;
	} else {
		econfig->type = be32_to_cpup((u32*)propdata);
	}
	DBG_PRINT("%s: type = %d\n", econfig->name, econfig->type);

	switch (econfig->type) {
	case LCD_EXTERN_I2C:
		propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "i2c_address", NULL);
		if (propdata == NULL) {
			LCD_EXT_PR("get %s i2c_address failed, exit\n", econfig->name);
			econfig->i2c_addr = 0;
			return -1;
		} else {
			econfig->i2c_addr = be32_to_cpup((u32*)propdata);
		}
		DBG_PRINT("%s i2c_address=0x%02x\n", econfig->name, econfig->i2c_addr);

		propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "i2c_bus", NULL);
		if (propdata == NULL) {
			LCD_EXT_PR("get %s i2c_bus failed, exit\n", econfig->name);
			econfig->i2c_bus = AML_I2C_MASTER_A;
			return -1;
		} else {
			if (strncmp(propdata, "i2c_bus_ao", 10) == 0)
				econfig->i2c_bus = AML_I2C_MASTER_AO;
			else if (strncmp(propdata, "i2c_bus_a", 9) == 0)
				econfig->i2c_bus = AML_I2C_MASTER_A;
			else if (strncmp(propdata, "i2c_bus_b", 9) == 0)
				econfig->i2c_bus = AML_I2C_MASTER_B;
#ifdef AML_I2C_MASTER_C
			else if (strncmp(propdata, "i2c_bus_c", 9) == 0)
				econfig->i2c_bus = AML_I2C_MASTER_C;
#endif
#ifdef AML_I2C_MASTER_D
			else if (strncmp(propdata, "i2c_bus_d", 9) == 0)
				econfig->i2c_bus = AML_I2C_MASTER_D;
#endif
			else {
				econfig->i2c_bus = AML_I2C_MASTER_A;
				LCD_EXT_PR("%s invalid i2c bus, exit\n", econfig->name);
				return -1;
			}
		}
		DBG_PRINT("%s i2c_bus=%s[%d]\n", econfig->name, propdata, econfig->i2c_bus);
		break;
	case LCD_EXTERN_SPI:
		propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "gpio_spi_cs", NULL);
		if (propdata == NULL) {
			LCD_EXT_PR("get %s gpio_spi_cs failed, exit\n", econfig->name);
			econfig->spi_cs = -1;
			return -1;
		} else {
			val = aml_lcd_gpio_name_map_num(propdata);
			if (val > 0) {
				econfig->spi_cs = val;
				DBG_PRINT("spi_cs gpio = %s(%d)\n", propdata, econfig->spi_cs);
			} else {
				econfig->spi_cs = -1;
			}
		}

		propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "gpio_spi_clk", NULL);
		if (propdata == NULL) {
			LCD_EXT_PR("get %s gpio_spi_clk failed, exit\n", econfig->name);
			econfig->spi_clk = -1;
			return -1;
		} else {
			val = aml_lcd_gpio_name_map_num(propdata);
			if (val > 0) {
				econfig->spi_clk = val;
				DBG_PRINT("spi_clk gpio = %s(%d)\n", propdata, econfig->spi_clk);
			} else {
				econfig->spi_clk = -1;
			}
		}

		propdata = (char *)fdt_getprop(dtaddr, nodeoffset, "gpio_spi_data", NULL);
		if (propdata == NULL) {
			LCD_EXT_PR("get %s gpio_spi_data failed, exit\n", econfig->name);
			econfig->spi_data = -1;
			return -1;
		} else {
			val = aml_lcd_gpio_name_map_num(propdata);
			if (val > 0) {
				econfig->spi_data = val;
				DBG_PRINT("spi_data gpio = %s(%d)\n", propdata, econfig->spi_data);
			} else {
				econfig->spi_data = -1;
			}
		}
		break;
	case LCD_EXTERN_MIPI:
		break;
	default:
		break;
	}

	return 0;
}
#endif

static int aml_lcd_extern_add_i2c(struct aml_lcd_extern_driver_t *ext_drv)
{
	int ret = 0;

	if (strcmp(ext_drv->config.name, "i2c_T5800Q") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_I2C_T5800Q
		ret = aml_lcd_extern_i2c_T5800Q_probe(ext_drv);
#endif
	} else if (strcmp(ext_drv->config.name, "i2c_tc101") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_I2C_TC101
		ret = aml_lcd_extern_i2c_tc101_probe(ext_drv);
#endif
	} else if (strcmp(ext_drv->config.name, "i2c_anx6345") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_I2C_ANX6345
		ret = aml_lcd_extern_i2c_anx6345_probe(ext_drv);
#endif
	} else {
		LCD_EXT_PR("invalid driver name: %s\n", ext_drv->config.name);
		ret = -1;
	}
	return ret;
}

static int aml_lcd_extern_add_spi(struct aml_lcd_extern_driver_t *ext_drv)
{
	int ret = 0;

	if (strcmp(ext_drv->config.name, "spi_LD070WS2") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_SPI_LD070WS2
		ret = aml_lcd_extern_spi_LD070WS2_probe(ext_drv);
#endif
	} else {
		LCD_EXT_PR("invalid driver name: %s\n", ext_drv->config.name);
		ret = -1;
	}
	return ret;
}

static int aml_lcd_extern_add_mipi(struct aml_lcd_extern_driver_t *ext_drv)
{
	int ret = 0;

	if (strcmp(ext_drv->config.name, "mipi_N070ICN") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_N070ICN
		ret = aml_lcd_extern_mipi_N070ICN_probe(ext_drv);
#endif
	} else if (strcmp(ext_drv->config.name, "mipi_KD080D13") == 0) {
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_KD080D13
		ret = aml_lcd_extern_mipi_KD080D13_probe(ext_drv);
#endif
	} else {
		LCD_EXT_PR("invalid driver name: %s\n", ext_drv->config.name);
		ret = -1;
	}
	return ret;
}

static int aml_lcd_extern_add_invalid(struct aml_lcd_extern_driver_t *ext_drv)
{
	return -1;
}

static int aml_lcd_extern_add_driver(struct lcd_extern_config_t *econfig)
{
	struct aml_lcd_extern_driver_t *ext_drv;
	int ret = 0;

	lcd_ext_driver = (struct aml_lcd_extern_driver_t *)malloc(sizeof(struct aml_lcd_extern_driver_t));
	if (lcd_ext_driver == NULL) {
		LCD_EXT_PR("failed to alloc driver %s[%d], not enough memory\n", econfig->name, econfig->index);
		return -1;
	}

	ext_drv = lcd_ext_driver;
	/* fill config parameters */
	ext_drv->config.index = econfig->index;
	strcpy(ext_drv->config.name, econfig->name);
	ext_drv->config.type = econfig->type;

	/* fill config parameters by different type */
	switch (ext_drv->config.type) {
	case LCD_EXTERN_I2C:
		ext_drv->config.i2c_addr = econfig->i2c_addr;
		ext_drv->config.i2c_bus = econfig->i2c_bus;
		ret = aml_lcd_extern_add_i2c(ext_drv);
		break;
	case LCD_EXTERN_SPI:
		ext_drv->config.spi_cs = econfig->spi_cs;
		ext_drv->config.spi_clk = econfig->spi_clk;
		ext_drv->config.spi_data = econfig->spi_data;
		ret = aml_lcd_extern_add_spi(ext_drv);
		break;
	case LCD_EXTERN_MIPI:
		ret = aml_lcd_extern_add_mipi(ext_drv);
		break;
	default:
		ret = aml_lcd_extern_add_invalid(ext_drv);
		LCD_EXT_PR("don't support type %d\n", ext_drv->config.type);
		break;
	}
	if (ret) {
		LCD_EXT_PR("add driver failed\n");
		free(lcd_ext_driver);
		lcd_ext_driver = NULL;
		return -1;
	}
	LCD_EXT_PR("add driver %s(%d)\n", ext_drv->config.name, ext_drv->config.index);
	return ret;
}

static int aml_lcd_extern_add_driver_default(int index)
{
	int drv_index;
	int ret;
	struct aml_lcd_extern_driver_t *ext_drv;

	lcd_ext_driver = (struct aml_lcd_extern_driver_t *)malloc(sizeof(struct aml_lcd_extern_driver_t));
	if (lcd_ext_driver == NULL) {
		LCD_EXT_PR("failed to alloc driver %d, not enough memory\n", index);
		return -1;
	}

	ext_drv = lcd_ext_driver;
#ifdef CONFIG_AML_LCD_EXTERN_I2C_T5800Q
	drv_index = aml_lcd_extern_i2c_T5800Q_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_i2c_T5800Q_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_TC101
	drv_index = aml_lcd_extern_i2c_tc101_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_i2c_tc101_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_ANX6345
	drv_index = aml_lcd_extern_i2c_anx6345_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_i2c_anx6345_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif
#ifdef CONFIG_AML_LCD_EXTERN_SPI_LD070WS2
	drv_index = aml_lcd_extern_spi_LD070WS2_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_spi_LD070WS2_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_N070ICN
	drv_index = aml_lcd_extern_mipi_N070ICN_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_mipi_N070ICN_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_KD080D13
	drv_index = aml_lcd_extern_mipi_KD080D13_get_default_index();
	if (drv_index == index) {
		ret = aml_lcd_extern_mipi_KD080D13_probe(ext_drv);
		goto add_driver_default_end;
	}
#endif

add_driver_default_end:
	if (ret) {
		LCD_EXT_PR("add driver failed\n");
		free(lcd_ext_driver);
		lcd_ext_driver = NULL;
		return -1;
	}
	LCD_EXT_PR("add default driver %d\n", index);
	return ret;
}

int aml_lcd_extern_probe(char *dtaddr, int index)
{
	struct lcd_extern_config_t ext_config;
	int ret, dt_ready = 0;

	if (index >= LCD_EXTERN_INDEX_INVALID) {
		LCD_EXT_PR("invalid index, %s exit\n", __func__);
		return -1;
	}

	dt_addr = NULL;
	ext_config.index = LCD_EXTERN_INDEX_INVALID;
	ext_config.type = LCD_EXTERN_MAX;
#ifdef CONFIG_OF_LIBFDT
	if (dtaddr)
		dt_addr = dtaddr;
	if (fdt_check_header(dtaddr) < 0) {
		LCD_EXT_PR("check dts: %s, load default parameters\n", fdt_strerror(fdt_check_header(dt_addr)));
	} else {
		ret = aml_lcd_extern_get_dt_config(dtaddr, index, &ext_config);
		if (ret == 0)
			dt_ready = 1;
	}
#endif
	if (dt_ready)
		ret = aml_lcd_extern_add_driver(&ext_config);
	else
		ret = aml_lcd_extern_add_driver_default(index);

	DBG_PRINT("%s %s\n", __func__, (ret ? "failed" : "ok"));
	return ret;
}

int aml_lcd_extern_remove(void)
{
	if (lcd_ext_driver)
		free(lcd_ext_driver);
	lcd_ext_driver = NULL;
	return 0;
}

