/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * MIPI 
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/gpio.h>
#include <amlogic/aml_lcd_extern.h>

//#define LCD_EXT_DEBUG_INFO

#define LCD_EXTERN_NAME			"lcd_mipi_LD070WX4"
#define LCD_EXTERN_TYPE			LCD_EXTERN_MIPI

static unsigned char mipi_init_table[] = {
    2,0x01,0x0,
    0xff,0x20,//mdelay flag
    2,0xAE,0x0B,
    2,0xEE,0xEA,
    2,0xEF,0x5F,
    2,0xF2,0x68,
    2,0xEE,0x0,
    2,0xEF,0x0,
    0xff,0xff,//ending flag
};

static struct aml_lcd_extern_driver_t lcd_ext_driver = {
    .name = LCD_EXTERN_NAME,
    .type = LCD_EXTERN_TYPE,
    .reg_read = NULL,
    .reg_write = NULL,
    .power_on = NULL,
    .power_off = NULL,
    .init_on_cmd_8 = &mipi_init_table[0],
    .init_off_cmd_8 = NULL,
};

struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void)
{
    return &lcd_ext_driver;
}
