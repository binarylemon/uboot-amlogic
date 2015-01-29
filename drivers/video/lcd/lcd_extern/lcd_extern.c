#include <common.h>
#include <malloc.h>
#include <asm/arch/io.h>
#include <asm/arch/vinfo.h>
#include <asm/arch/lcdoutc.h>
#include <amlogic/lcdoutc.h>
#include <asm/arch/i2c.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif
#ifdef CONFIG_AML_LCD_EXTERN
#include <amlogic/aml_lcd_extern.h>
#endif

//#define LCD_EXT_DEBUG_INFO
#ifdef LCD_EXT_DEBUG_INFO
#define DBG_PRINT(...)		printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif
#define LCD_EXTERN_DEVICE  "lcd_extern_device"

#ifdef CONFIG_OF_LIBFDT
int get_lcd_extern_dt_data(char * dt_addr, char *of_node, struct lcd_extern_config_t *pdata)
{
	int val;
	int nodeoffset;
	char * propdata;
	int ret;
	
	nodeoffset = fdt_path_offset(dt_addr, of_node);
	if(nodeoffset < 0) {
		printf("dts: not find  node %s.\n",of_node);
		return ret;
	}
	propdata =(char *)fdt_getprop(dt_addr, nodeoffset, "dev_name", NULL);
	if (propdata == NULL) {
		printf("faild to get dev_name, use default name: %s\n", pdata->name);
	}
	else {
		pdata->name = (char *)malloc(sizeof(char)*LCD_EXT_NAME_LEN_MAX);
		if (pdata->name == NULL) {
			printf("[get_lcd_extern_dt_data]: Not enough memory\n");
		}
		else {
			memset(pdata->name, 0, LCD_EXT_NAME_LEN_MAX);
			strcpy(pdata->name, propdata);
			printf("load lcd_extern in dtb: %s\n", pdata->name);
		}
	}
	
	propdata =(char *)fdt_getprop(dt_addr, nodeoffset, "type", NULL);
	if(propdata == NULL){
		pdata->type = LCD_EXTERN_MAX;
		printf("faild to get %s type\n", pdata->name);
	}
	else {
		pdata->type = (unsigned short)(be32_to_cpup((u32*)propdata));
	}
	DBG_PRINT("%s: type =%d\n", pdata->name, pdata->type);
	switch (pdata->type) {
		case LCD_EXTERN_I2C:
			propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "i2c_address", NULL);
			if (propdata == NULL) {
				printf("%s warning: get i2c_address failed\n", pdata->name);
				pdata->i2c_addr = 0;
			}
			else {
				pdata->i2c_addr = (unsigned short)(be32_to_cpup((u32*)propdata));
			}
			DBG_PRINT("%s: i2c_address=0x%02x\n", pdata->name, pdata->i2c_addr);
		  
			propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "i2c_bus", NULL);
			if (propdata == NULL) {
				printf("%s warning: get i2c_bus failed, use default i2c bus\n", pdata->name);
				propdata = "i2c_bus_a";
			}
			else {
				if (strncmp(propdata, "i2c_bus_a", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_A;
				else if (strncmp(propdata, "i2c_bus_b", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_B;
				else if (strncmp(propdata, "i2c_bus_ao", 10) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_AO;
				else
					pdata->i2c_bus = AML_I2C_MASTER_A; 
			}
			DBG_PRINT("%s: i2c_bus=%s[%d]\n", pdata->name, propdata, pdata->i2c_bus);
			break;
		case LCD_EXTERN_SPI:
			propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "gpio_spi_cs", NULL);
			if (propdata == NULL) {
				printf("%s warning: get spi gpio_spi_cs failed\n", pdata->name);
				pdata->spi_cs = -1;
			}
			else {
			    val = aml_lcd_gpio_name_map_num(propdata);
				if (val > 0) {
					pdata->spi_cs = val;
					DBG_PRINT("spi_cs gpio = %s(%d)\n", propdata, pdata->spi_cs);
				}
				else {
					pdata->spi_cs = -1;
				}
			}
			
			propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "gpio_spi_clk", NULL);
			if (propdata == NULL) {
				printf("%s warning: get spi gpio_spi_clk failed\n", pdata->name);
				pdata->spi_clk = -1;
			}
			else {
			    val = aml_lcd_gpio_name_map_num(propdata);
				if (val > 0) {
					pdata->spi_clk = val;
					DBG_PRINT("spi_clk gpio = %s(%d)\n", propdata, pdata->spi_clk);
				}
				else {
					pdata->spi_clk = -1;
				}
			}
			
			propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "gpio_spi_data", NULL);
			if (propdata == NULL) {
				printf("%s warning: get spi gpio_spi_data failed\n", pdata->name);
				pdata->spi_data = -1;
			}
			else {
			    val = aml_lcd_gpio_name_map_num(propdata);
				if (val > 0) {
					pdata->spi_data = val;
					DBG_PRINT("spi_data gpio = %s(%d)\n", propdata, pdata->spi_data);
				}
				else {
					pdata->spi_data = -1;
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
