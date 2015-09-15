/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * SPI
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/timing.h>
#include <asm/arch/aml_lcd_gpio.h>
#include <amlogic/aml_lcd_extern.h>
#include "lcd_extern.h"

#define LCD_EXTERN_INDEX	0
#define LCD_EXTERN_NAME		"spi_LD070WS2"
#define LCD_EXTERN_TYPE		LCD_EXTERN_SPI

#define GPIO_SPI_CS		GPIOY_5
#define GPIO_SPI_CLK		GPIOZ_13
#define GPIO_SPI_DATA		GPIOZ_14

#define SPI_DELAY		30 //unit: us

static struct lcd_extern_config_t *ext_config;

static unsigned char spi_init_table[][2] = {
	{0x00,0x21},  //reset
	{0x00,0xa5},  //standby
	{0x01,0x30},  //enable FRC/Dither
	{0x02,0x40},  //enable normally black
	{0x0e,0x5f},  //enable test mode1
	{0x0f,0xa4},  //enable test mode2
	{0x0d,0x00},  //enable SDRRS, enlarge OE width
	{0x02,0x43},  //adjust charge sharing time
	{0x0a,0x28},  //trigger bias reduction
	{0x10,0x41},  //adopt 2 line/1 dot
	{0xff,50},    //delay 50ms
	{0x00,0xad},  //display on
	{0xff,0xff},  //ending flag
};

static unsigned char spi_off_table[][2] = {
	{0x00,0xa5},  //standby
	{0xff,0xff},
};

static void lcd_extern_set_csb(unsigned v)
{
	aml_lcd_gpio_set(ext_config->spi_cs, v);
	udelay(SPI_DELAY);
}

static void lcd_extern_set_scl(unsigned v)
{
	aml_lcd_gpio_set(ext_config->spi_clk, v);
	udelay(SPI_DELAY);
}

static void lcd_extern_set_sda(unsigned v)
{
	aml_lcd_gpio_set(ext_config->spi_data, v);
	udelay(SPI_DELAY);
}

static void spi_gpio_init(void)
{
	lcd_extern_set_csb(1);
	lcd_extern_set_scl(1);
	lcd_extern_set_sda(1);
}

static void spi_gpio_off(void)
{
	lcd_extern_set_sda(0);
	lcd_extern_set_scl(0);
	lcd_extern_set_csb(0);
}

static void spi_write_8(unsigned char addr, unsigned char data)
{
	int i;
	unsigned int sdata;

	sdata = (unsigned int)(addr & 0x3f);
	sdata <<= 10;
	sdata |= (data & 0xff);
	sdata &= ~(1<<9); //write flag

	lcd_extern_set_csb(1);
	lcd_extern_set_scl(1);
	lcd_extern_set_sda(1);

	lcd_extern_set_csb(0);
	for (i = 0; i < 16; i++) {
		lcd_extern_set_scl(0);
		if (sdata & 0x8000)
			lcd_extern_set_sda(1);
		else
			lcd_extern_set_sda(0);
		sdata <<= 1;
		lcd_extern_set_scl(1);
	}

	lcd_extern_set_csb(1);
	lcd_extern_set_scl(1);
	lcd_extern_set_sda(1);
	udelay(SPI_DELAY);
}

static int lcd_extern_spi_init(void)
{
	int ending_flag = 0;
	int i=0;

	spi_gpio_init();

	while (ending_flag == 0) {
		if (spi_init_table[i][0] == 0xff) {
			if (spi_init_table[i][1] == 0xff)
				ending_flag = 1;
			else
				mdelay(spi_init_table[i][1]);
		} else {
			spi_write_8(spi_init_table[i][0], spi_init_table[i][1]);
		}
		i++;
	}

	LCD_EXT_PR("%s: %s\n", __func__, ext_config->name);
	return 0;
}

static int lcd_extern_spi_off(void)
{
	int ending_flag = 0;
	int i=0;

	spi_gpio_init();

	while (ending_flag == 0) {
		if (spi_off_table[i][0] == 0xff) {
			if (spi_off_table[i][1] == 0xff)
				ending_flag = 1;
			else
				mdelay(spi_off_table[i][1]);
		} else {
			spi_write_8(spi_off_table[i][0], spi_off_table[i][1]);
		}
		i++;
	}

	LCD_EXT_PR("%s: %s\n", __func__, ext_config->name);
	mdelay(10);
	spi_gpio_off();
	return 0;
}

static int lcd_extern_power_on(void)
{
	int ret;

	ret = lcd_extern_spi_init();
	return ret;
}

static int lcd_extern_power_off(void)
{
	int ret;

	ret = lcd_extern_spi_off();
	return ret;
}

static int lcd_extern_driver_update(struct aml_lcd_extern_driver_t *ext_drv)
{
	if (ext_drv == NULL) {
		LCD_EXT_PR("%s driver is null\n", LCD_EXTERN_NAME);
		return -1;
	}

	if (ext_drv->config.type == LCD_EXTERN_MAX) { //default for no dt
		ext_drv->config.index = LCD_EXTERN_INDEX;
		ext_drv->config.type = LCD_EXTERN_TYPE;
		strcpy(ext_drv->config.name, LCD_EXTERN_NAME);
		ext_drv->config.spi_cs = GPIO_SPI_CS;
		ext_drv->config.spi_clk = GPIO_SPI_CLK;
		ext_drv->config.spi_data = GPIO_SPI_DATA;
	}
	ext_drv->power_on  = lcd_extern_power_on;
	ext_drv->power_off = lcd_extern_power_off;

	return 0;
}

int aml_lcd_extern_spi_LD070WS2_get_default_index(void)
{
	return LCD_EXTERN_INDEX;
}

int aml_lcd_extern_spi_LD070WS2_probe(struct aml_lcd_extern_driver_t *ext_drv)
{
	int ret = 0;

	ext_config = &ext_drv->config;
	ret = lcd_extern_driver_update(ext_drv);

	DBG_PRINT("%s: %d\n", __func__, ret);
	return ret;
}
