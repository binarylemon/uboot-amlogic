
#ifndef __AMLOGIC_LCD_EXTERN_H_
#define __AMLOGIC_LCD_EXTERN_H_

typedef enum {
	LCD_EXTERN_I2C = 0,
	LCD_EXTERN_SPI,
	LCD_EXTERN_MAX,
} Lcd_Extern_Type_t;

struct aml_lcd_extern_pinmux_t {
	unsigned reg;
	unsigned mux;
};

struct aml_lcd_extern_driver_t {
	char *name;
	Lcd_Extern_Type_t type;
	int  (*reg_read)  (unsigned char reg, unsigned char *buf);
    int  (*reg_write) (unsigned char reg, unsigned char value);
	int  (*power_on)(void);
	int  (*power_off)(void);
	// void  (*fun1_no_para)(void);
	// void  (*fun2_no_para)(void);
	// void  (*fun1_para)(unsigned para);
	// void  (*fun2_para)(unsigned para); 
};

extern struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void);

#endif

