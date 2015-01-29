
#ifndef __AMLOGIC_LCD_GPIO_H_
#define __AMLOGIC_LCD_GPIO_H_

#include <asm/arch/gpio.h>

typedef enum
{
	LCD_GPIO_OUTPUT_LOW = 0,
	LCD_GPIO_OUTPUT_HIGH,
	LCD_GPIO_INPUT,
} Lcd_Gpio_t;

extern int aml_lcd_gpio_name_map_num(const char *name);
extern int aml_lcd_gpio_set(int gpio, int flag);
extern int aml_lcd_gpio_input_get(int gpio);

extern int aml_lcd_pinmux_set(unsigned int mux_index, unsigned int mux_mask);
extern int aml_lcd_pinmux_clr(unsigned int mux_index, unsigned int mux_mask);

#endif

