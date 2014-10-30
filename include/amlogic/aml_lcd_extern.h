
#ifndef __AMLOGIC_LCD_EXTERN_H_
#define __AMLOGIC_LCD_EXTERN_H_

typedef enum {
	LCD_EXTERN_I2C = 0,
	LCD_EXTERN_SPI,
	LCD_EXTERN_MIPI,
	LCD_EXTERN_MAX,
} Lcd_Extern_Type_t;

struct aml_lcd_extern_pinmux_t {
	unsigned reg;
	unsigned mux;
};

struct aml_lcd_extern_driver_t {
	char *name;
	Lcd_Extern_Type_t type;
	int (*reg_read)  (unsigned char reg, unsigned char *buf);
	int (*reg_write) (unsigned char reg, unsigned char value);
	int (*power_on)(void);
	int (*power_off)(void);
	unsigned char *init_on_cmd_8;
	unsigned char *init_off_cmd_8;
	int (*get_lcd_ext_config)(char *dt_addr);
	//unsigned short *init_on_cmd_16;
	//unsigned short *init_off_cmd_16;
};

struct lcd_extern_config_t {
	char *name;
	Lcd_Extern_Type_t type;
	int status;
	int i2c_addr;
	int i2c_bus;
	int spi_cs;
	int spi_clk;
	int spi_data;
};

struct lcd_extern_edp_config {
	int lane_num;
	int bits;
	int link_rate;
};

extern void udelay(unsigned long usec);
extern void mdelay(unsigned long msec);

extern struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void);
extern int get_lcd_extern_dt_data(char * dt_addr, char *of_node, struct lcd_extern_config_t *pdata);

#endif

