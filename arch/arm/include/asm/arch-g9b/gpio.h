#ifndef  __MESON_GPIO_H__
#define	 __MESON_GPIO_H__

#include "io.h"
//#include "gpio_name.h"
typedef enum gpio_bank {
    PREG_PAD_GPIO0 = 0,
    PREG_PAD_GPIO1,
    PREG_PAD_GPIO2,
    PREG_PAD_GPIO3,
    PREG_PAD_GPIO4,
    PREG_PAD_GPIO5,
	PREG_PAD_GPIOAO,
	PREG_JTAG_GPIO,
	PREG_PAD_GPIO6,
} gpio_bank_t;


typedef enum gpio_mode {
    GPIO_OUTPUT_MODE,
    GPIO_INPUT_MODE,
} gpio_mode_t;

int set_gpio_mode(gpio_bank_t bank, int bit, gpio_mode_t mode);
gpio_mode_t get_gpio_mode(gpio_bank_t bank, int bit);

int set_gpio_val(gpio_bank_t bank, int bit, unsigned long val);
unsigned long  get_gpio_val(gpio_bank_t bank, int bit);

#define GPIOX_bank_bit0_27(bit)     (PREG_PAD_GPIO4)
#define GPIOX_bit_bit0_27(bit)      (bit)

#define GPIOBOOT_bank_bit0_18(bit)  (PREG_PAD_GPIO2)
#define GPIOBOOT_bit_bit0_18(bit)   (bit)

#define GPIOH_bank_bit0_10(bit)     (PREG_PAD_GPIO1)
#define GPIOH_bit_bit0_10(bit)      (bit + 16)

#define GPIOZ_bank_bit0_20(bit)     (PREG_PAD_GPIO3)
#define GPIOZ_bit_bit0_20(bit)      (bit)

#define GPIOW_bank_bit0_20(bit)     (PREG_PAD_GPIO0)
#define GPIOW_bit_bit0_20(bit)      (bit)

#define GPIOAO_bank_bit0_13(bit)    (PREG_PAD_GPIOAO)
#define GPIOAO_bit_bit0_13(bit)     (bit)

#define GPIOCARD_bank_bit0_8(bit)   (PREG_PAD_GPIO2)
#define GPIOCARD_bit_bit0_8(bit)    (bit+20)

#define GPIOY_bank_bit0_13(bit)     (PREG_PAD_GPIO1)
#define GPIOY_bit_bit0_13(bit)      (bit)


/**
 * enable gpio edge interrupt
 *
 * @param [in] pin  index number of the chip, start with 0 up to 255
 * @param [in] flag rising(0) or falling(1) edge
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_edge_int(int pin , int flag, int group);
/**
 * enable gpio level interrupt
 *
 * @param [in] pin  index number of the chip, start with 0 up to 255
 * @param [in] flag high(0) or low(1) level
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_level_int(int pin , int flag, int group);

/**
 * enable gpio interrupt filter
 *
 * @param [in] filter from 0~7(*222ns)
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_int_filter(int filter, int group);

extern int gpio_is_valid(int number);
extern int gpio_request(unsigned gpio, const char *label);
extern void gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);
typedef enum {
	GPIOX_0		=	0,
	GPIOX_1		=	1,
	GPIOX_2		=	2,
	GPIOX_3		=	3,
	GPIOX_4		=	4,
	GPIOX_5		=	5,
	GPIOX_6		=	6,
	GPIOX_7		=	7,
	GPIOX_8		=	8,
	GPIOX_9		=	9,
	GPIOX_10	=	10,
	GPIOX_11	=	11,
	GPIOX_12	=	12,
	GPIOX_13	=	13,
	GPIOX_14	=	14,
	GPIOX_15	=	15,
	GPIOX_16	=	16,
	GPIOX_17	=	17,
	GPIOX_18	=	18,
	GPIOX_19	=	19,
	GPIOX_20	=	20,
	GPIOX_21	=	21,
	GPIOX_22	=	22,
	GPIOX_23	=	23,
	GPIOX_24	=	24,
	GPIOX_25	=	25,
	GPIOX_26	=	26,
	GPIOX_27	=	27,

	BOOT_0		=	28,
	BOOT_1		=	29,
	BOOT_2		=	30,
	BOOT_3		=	31,
	BOOT_4		=	32,
	BOOT_5		=	33,
	BOOT_6		=	34,
	BOOT_7		=	35,
	BOOT_8		=	36,
	BOOT_9		=	37,
	BOOT_10		=	38,
	BOOT_11		=	39,
	BOOT_12		=	40,
	BOOT_13		=	41,
	BOOT_14		=	42,
	BOOT_15		=	43,
	BOOT_16		=	44,
	BOOT_17		=	45,
	BOOT_18		=	46,

	GPIOH_0		=	47,
	GPIOH_1		=	48,
	GPIOH_2		=	49,
	GPIOH_3		=	50,
	GPIOH_4		=	51,
	GPIOH_5		=	52,
	GPIOH_6		=	53,
	GPIOH_7		=	54,
	GPIOH_8		=	55,
	GPIOH_9		=	56,
	GPIOH_10	=	57,

	GPIOZ_0		=	58,
	GPIOZ_1		=	59,
	GPIOZ_2		=	60,
	GPIOZ_3		=	61,
	GPIOZ_4		=	62,
	GPIOZ_5		=	63,
	GPIOZ_6		=	64,
	GPIOZ_7		=	65,
	GPIOZ_8		=	66,
	GPIOZ_9		=	67,
	GPIOZ_10	=	68,
	GPIOZ_11	=	69,
	GPIOZ_12	=	70,
	GPIOZ_13	=	71,
	GPIOZ_14	=	72,
	GPIOZ_15	=	73,
	GPIOZ_16	=	74,
	GPIOZ_17	=	75,
	GPIOZ_18	=	76,
	GPIOZ_19	=	77,
	GPIOZ_20	=	78,

	GPIOW_0		=	79,
	GPIOW_1		=	80,
	GPIOW_2		=	81,
	GPIOW_3		=	82,
	GPIOW_4		=	83,
	GPIOW_5		=	84,
	GPIOW_6		=	85,
	GPIOW_7		=	86,
	GPIOW_8		=	87,
	GPIOW_9		=	88,
	GPIOW_10	=	89,
	GPIOW_11	=	90,
	GPIOW_12	=	91,
	GPIOW_13	=	92,
	GPIOW_14	=	93,
	GPIOW_15	=	94,
	GPIOW_16	=	95,
	GPIOW_17	=	96,
	GPIOW_18	=	97,
	GPIOW_19	=	98,
	GPIOW_20	=	99,

	GPIOAO_0	=	100,
	GPIOAO_1	=	101,
	GPIOAO_2	=	102,
	GPIOAO_3	=	103,
	GPIOAO_4	=	104,
	GPIOAO_5	=	105,
	GPIOAO_6	=	106,
	GPIOAO_7	=	107,
	GPIOAO_8	=	108,
	GPIOAO_9	=	109,
	GPIOAO_10	=	110,
	GPIOAO_11	=	111,
	GPIOAO_12	=	112,
	GPIOAO_13	=	113,

	CARD_0		=	114,
	CARD_1		=	115,
	CARD_2		=	116,
	CARD_3		=	117,
	CARD_4		=	118,
	CARD_5		=	119,
	CARD_6		=	120,
	CARD_7		=	121,
	CARD_8		=	122,

	GPIOY_0		=	123,
	GPIOY_1		=	124,
	GPIOY_2		=	125,
	GPIOY_3		=	126,
	GPIOY_4		=	127,
	GPIOY_5		=	128,
	GPIOY_6		=	129,
	GPIOY_7		=	130,
	GPIOY_8		=	131,
	GPIOY_9		=	132,
	GPIOY_10	=	133,
	GPIOY_11	=	134,
	GPIOY_12	=	135,
	GPIOY_13	=	136,

	GPIO_TEST_N =   137,
	GPIO_MAX	=	138,

}gpio_t;

#endif
