#ifndef _LCD_EXTERN_H_
#define _LCD_EXTERN_H_

//#define LCD_EXT_DEBUG_INFO
#ifdef LCD_EXT_DEBUG_INFO
#define DBG_PRINT(format, arg...)        printf("lcd extern: " format, ## arg)
#else
#define DBG_PRINT(format, arg...)
#endif
#define LCD_EXT_PR(format, arg...)        printf("lcd extern: " format, ## arg)

#define LCD_EXTERN_DRIVER		"lcd_extern"

struct aml_lcd_extern_pinmux_t {
	unsigned reg;
	unsigned mux;
};

extern void udelay(unsigned long usec);
extern void mdelay(unsigned long msec);

#ifdef CONFIG_OF_LIBFDT
extern char *aml_lcd_extern_get_dt_prop(int nodeoffset, char *propname);
extern int aml_lcd_extern_get_dt_child(int index);
#endif

#ifdef CONFIG_AML_LCD_EXTERN_I2C_T5800Q
extern int aml_lcd_extern_i2c_T5800Q_get_default_index(void);
extern int aml_lcd_extern_i2c_T5800Q_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_TC101
extern int aml_lcd_extern_i2c_tc101_get_default_index(void);
extern int aml_lcd_extern_i2c_tc101_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_ANX6345
extern int aml_lcd_extern_i2c_anx6345_get_default_index(void);
extern int aml_lcd_extern_i2c_anx6345_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_SPI_LD070WS2
extern int aml_lcd_extern_spi_LD070WS2_get_default_index(void);
extern int aml_lcd_extern_spi_LD070WS2_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_N070ICN
extern int aml_lcd_extern_mipi_N070ICN_get_default_index(void);
extern int aml_lcd_extern_mipi_N070ICN_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_KD080D13
extern int aml_lcd_extern_mipi_KD080D13_get_default_index(void);
extern int aml_lcd_extern_mipi_KD080D13_probe(struct aml_lcd_extern_driver_t *ext_drv);
#endif

#endif

