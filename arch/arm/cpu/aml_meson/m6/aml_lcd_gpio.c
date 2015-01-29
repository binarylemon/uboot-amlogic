#include <common.h>
#include <asm/arch/reg_addr.h>
#include <asm/arch/gpio.h>
#include <asm/arch/aml_lcd_gpio.h>

//#define LCD_GPIO_USE_SYS_API

#ifdef LCD_GPIO_USE_SYS_API
extern int gpio_amlogic_name_to_num(const char *name);
int aml_lcd_gpio_name_map_num(const char *name)
{
	//printf("gpio name: %s\n", name);
	return gpio_amlogic_name_to_num(name);
}

int aml_lcd_gpio_set(int gpio, int flag)
{
	//printf("gpio: %d = %d\n", gpio, flag);
	switch (flag) {
		case LCD_GPIO_OUTPUT_LOW:
		case LCD_GPIO_OUTPUT_HIGH:
			gpio_direction_output(gpio, flag);
			break;
		case LCD_GPIO_INPUT:
			gpio_direction_input(gpio);
			break;
		default:
			printf("Wrong GPIO value: %d\n", flag);
	}
	return 0;
}

int aml_lcd_gpio_input_get(int gpio)
{
	//printf("gpio: %d\n", gpio);
	return gpio_get_value(gpio);
}
#else
typedef struct aml_lcd_gpio_s {
	unsigned int bank;
	unsigned int offset;
} aml_lcd_gpio_t;

#define LCD_GPIO_NAME_MAP_USE_SYS_API

#ifdef LCD_GPIO_NAME_MAP_USE_SYS_API
extern int gpio_amlogic_name_to_num(const char *name);
#else
static const char* aml_lcd_gpio_type_table[]={
	"GPIOZ_0",
	"GPIOZ_1",
	"GPIOZ_2",
	"GPIOZ_3",
	"GPIOZ_4",
	"GPIOZ_5",
	"GPIOZ_6",
	"GPIOZ_7",
	"GPIOZ_8",
	"GPIOZ_9",
	"GPIOZ_10",
	"GPIOZ_11",
	"GPIOZ_12",
	"GPIOE_0",
	"GPIOE_1",
	"GPIOE_2",
	"GPIOE_3",
	"GPIOE_4",
	"GPIOE_5",
	"GPIOE_6",
	"GPIOE_7",
	"GPIOE_8",
	"GPIOE_9",
	"GPIOE_10",
	"GPIOE_11",
	"GPIOY_0",
	"GPIOY_1",
	"GPIOY_2",
	"GPIOY_3",
	"GPIOY_4",
	"GPIOY_5",
	"GPIOY_6",
	"GPIOY_7",
	"GPIOY_8",
	"GPIOY_9",
	"GPIOY_10",
	"GPIOY_11",
	"GPIOY_12",
	"GPIOY_13",
	"GPIOY_14",
	"GPIOY_15",
	"GPIOX_0",
	"GPIOX_1",
	"GPIOX_2",
	"GPIOX_3",
	"GPIOX_4",
	"GPIOX_5",
	"GPIOX_6",
	"GPIOX_7",
	"GPIOX_8",
	"GPIOX_9",
	"GPIOX_10",
	"GPIOX_11",
	"GPIOX_12",
	"GPIOX_13",
	"GPIOX_14",
	"GPIOX_15",
	"GPIOX_16",
	"GPIOX_17",
	"GPIOX_18",
	"GPIOX_19",
	"GPIOX_20",
	"GPIOX_21",
	"GPIOX_22",
	"GPIOX_23",
	"GPIOX_24",
	"GPIOX_25",
	"GPIOX_26",
	"GPIOX_27",
	"GPIOX_28",
	"GPIOX_29",
	"GPIOX_30",
	"GPIOX_31",
	"GPIOX_32",
	"GPIOX_33",
	"GPIOX_34",
	"GPIOX_35",
	"BOOT_0",
	"BOOT_1",
	"BOOT_2",
	"BOOT_3",
	"BOOT_4",
	"BOOT_5",
	"BOOT_6",
	"BOOT_7",
	"BOOT_8",
	"BOOT_9",
	"BOOT_10",
	"BOOT_11",
	"BOOT_12",
	"BOOT_13",
	"BOOT_14",
	"BOOT_15",
	"BOOT_16",
	"BOOT_17",
	"GPIOD_0",
	"GPIOD_1",
	"GPIOD_2",
	"GPIOD_3",
	"GPIOD_4",
	"GPIOD_5",
	"GPIOD_6",
	"GPIOD_7",
	"GPIOD_8",
	"GPIOD_9",
	"GPIOC_0",
	"GPIOC_1",
	"GPIOC_2",
	"GPIOC_3",
	"GPIOC_4",
	"GPIOC_5",
	"GPIOC_6",
	"GPIOC_7",
	"GPIOC_8",
	"GPIOC_9",
	"GPIOC_10",
	"GPIOC_11",
	"GPIOC_12",
	"GPIOC_13",
	"GPIOC_14",
	"GPIOC_15",
	"CARD_0",
	"CARD_1",
	"CARD_2",
	"CARD_3",
	"CARD_4",
	"CARD_5",
	"CARD_6",
	"CARD_7",
	"CARD_8",
	"GPIOB_0",
	"GPIOB_1",
	"GPIOB_2",
	"GPIOB_3",
	"GPIOB_4",
	"GPIOB_5",
	"GPIOB_6",
	"GPIOB_7",
	"GPIOB_8",
	"GPIOB_9",
	"GPIOB_10",
	"GPIOB_11",
	"GPIOB_12",
	"GPIOB_13",
	"GPIOB_14",
	"GPIOB_15",
	"GPIOB_16",
	"GPIOB_17",
	"GPIOB_18",
	"GPIOB_19",
	"GPIOB_20",
	"GPIOB_21",
	"GPIOB_22",
	"GPIOB_23",
	"GPIOA_0",
	"GPIOA_1",
	"GPIOA_2",
	"GPIOA_3",
	"GPIOA_4",
	"GPIOA_5",
	"GPIOA_6",
	"GPIOA_7",
	"GPIOA_8",
	"GPIOA_9",
	"GPIOA_10",
	"GPIOA_11",
	"GPIOA_12",
	"GPIOA_13",
	"GPIOA_14",
	"GPIOA_15",
	"GPIOA_16",
	"GPIOA_17",
	"GPIOA_18",
	"GPIOA_19",
	"GPIOA_20",
	"GPIOA_21",
	"GPIOA_22",
	"GPIOA_23",
	"GPIOA_24",
	"GPIOA_25",
	"GPIOA_26",
	"GPIOA_27",
	"GPIOAO_0",
	"GPIOAO_1",
	"GPIOAO_2",
	"GPIOAO_3",
	"GPIOAO_4",
	"GPIOAO_5",
	"GPIOAO_6",
	"GPIOAO_7",
	"GPIOAO_8",
	"GPIOAO_9",
	"GPIOAO_10",
	"GPIOAO_11",
	"GPIO_MAX",
};
#endif

static unsigned int aml_lcd_gpio_group_ctrl[][6] = {
	//oen_reg,              oen_bit,  out_reg,            out_bit,  in_reg,             in_bit
	{P_PREG_PAD_GPIO6_EN_N, 16,       P_PREG_PAD_GPIO6_O, 16,       P_PREG_PAD_GPIO6_I, 16,}, //GPIOZ
	{P_PREG_PAD_GPIO6_EN_N, 0,        P_PREG_PAD_GPIO6_O, 0,        P_PREG_PAD_GPIO6_I, 0, }, //GPIOE
	{P_PREG_PAD_GPIO5_EN_N, 0,        P_PREG_PAD_GPIO5_O, 0,        P_PREG_PAD_GPIO5_I, 0, }, //GPIOY
	{P_PREG_PAD_GPIO4_EN_N, 0,        P_PREG_PAD_GPIO4_O, 0,        P_PREG_PAD_GPIO4_I, 0, }, //GPIOX(0~31)
	{P_PREG_PAD_GPIO3_EN_N, 20,       P_PREG_PAD_GPIO3_O, 20,       P_PREG_PAD_GPIO3_I, 20,}, //GPIOX(32~35)
	{P_PREG_PAD_GPIO3_EN_N, 0,        P_PREG_PAD_GPIO3_O, 0,        P_PREG_PAD_GPIO3_I, 0, }, //BOOT
	{P_PREG_PAD_GPIO2_EN_N, 16,       P_PREG_PAD_GPIO2_O, 16,       P_PREG_PAD_GPIO2_I, 16,}, //GPIOD
	{P_PREG_PAD_GPIO2_EN_N, 7,        P_PREG_PAD_GPIO2_O, 7,        P_PREG_PAD_GPIO2_I, 7, }, //GPIOC
	{P_PREG_PAD_GPIO5_EN_N, 23,       P_PREG_PAD_GPIO5_O, 23,       P_PREG_PAD_GPIO5_I, 23,}, //CARD
	{P_PREG_PAD_GPIO1_EN_N, 0,        P_PREG_PAD_GPIO1_O, 0,        P_PREG_PAD_GPIO1_I, 0, }, //GPIOB
	{P_PREG_PAD_GPIO0_EN_N, 0,        P_PREG_PAD_GPIO0_O, 0,        P_PREG_PAD_GPIO0_I, 0, }, //GPIOA
	{P_AO_GPIO_O_EN_N,      0,        P_AO_GPIO_O_EN_N,   16,       P_AO_GPIO_I,        0, }, //GPIOAO
};

int aml_lcd_gpio_name_map_num(const char *name)
{
#ifdef LCD_GPIO_NAME_MAP_USE_SYS_API
	return gpio_amlogic_name_to_num(name);
#else
	int i;
	
	for(i = 0; i < GPIO_MAX; i++) {
		if(!strcmp(name, aml_lcd_gpio_type_table[i]))
			break;
	}
	if (i == GPIO_MAX) {
		printf("wrong gpio name %s, i=%d\n", name, i);
		i = -1;
	}
	return i;
#endif
}

static int aml_lcd_gpio_bank_offset(int gpio, struct aml_lcd_gpio_s *gpio_s)
{
	int ret = 0;
	
	if ((gpio>=GPIOZ_0) && (gpio<=GPIOZ_12)) { //GPIOZ_0~12
		gpio_s->bank = 0;
		gpio_s->offset = gpio - GPIOZ_0;
	}
	else if ((gpio>=GPIOE_0) && (gpio<=GPIOE_11)) { //GPIOE_0~11
		gpio_s->bank = 1;
		gpio_s->offset = gpio - GPIOE_0;
	}
	else if ((gpio>=GPIOY_0) && (gpio<=GPIOY_15)) { //GPIOY_0~15
		gpio_s->bank = 2;
		gpio_s->offset = gpio - GPIOY_0;
	}
	else if ((gpio>=GPIOX_0) && (gpio<=GPIOX_31)) { //GPIOX_0~31
		gpio_s->bank = 3;
		gpio_s->offset = gpio - GPIOX_0;
	}
	else if ((gpio>=GPIOX_32) && (gpio<=GPIOX_35)) { //GPIOX_32~35
		gpio_s->bank = 4;
		gpio_s->offset = gpio - GPIOX_32;
	}
	else if ((gpio>=BOOT_0) && (gpio<=BOOT_17)) { //BOOT_0~17
		gpio_s->bank = 5;
		gpio_s->offset = gpio - BOOT_0;
	}
	else if ((gpio>=GPIOD_0) && (gpio<=GPIOD_9)) { //GPIOD_0~9
		gpio_s->bank = 6;
		gpio_s->offset = gpio - GPIOD_0;
	}
	else if ((gpio>=GPIOC_0) && (gpio<=GPIOC_15)) { //GPIOC_0~15
		gpio_s->bank = 7;
		gpio_s->offset = gpio - GPIOC_0;
	}
	else if ((gpio>=CARD_0) && (gpio<=CARD_8)) { //CARD_0~8
		gpio_s->bank = 8;
		gpio_s->offset = gpio - CARD_0;
	}
	else if ((gpio>=GPIOB_0) && (gpio<=GPIOB_23)) { //GPIOB_0~23
		gpio_s->bank = 9;
		gpio_s->offset = gpio - GPIOB_0;
	}
	else if ((gpio>=GPIOA_0) && (gpio<=GPIOA_27)) { //GPIOA_0~27
		gpio_s->bank = 10;
		gpio_s->offset = gpio - GPIOA_0;
	}
	else if ((gpio>=GPIOAO_0) && (gpio<=GPIOAO_11)) { //GPIOAO_0~11
		gpio_s->bank = 11;
		gpio_s->offset = gpio - GPIOAO_0;
	}
	else {
		printf("Wrong GPIO Port number: %d\n", gpio);
		ret = -1;
	}
	
	return ret;
}

int aml_lcd_gpio_set(int gpio, int flag)
{
	struct aml_lcd_gpio_s gpio_s;
	int ret = 0;
	
	ret = aml_lcd_gpio_bank_offset(gpio, &gpio_s);
	if (ret == 0) {
		if (flag == LCD_GPIO_OUTPUT_LOW) {
			aml_clr_reg32_mask(aml_lcd_gpio_group_ctrl[gpio_s.bank][2], (1 << (aml_lcd_gpio_group_ctrl[gpio_s.bank][3] + gpio_s.offset)));
			aml_clr_reg32_mask(aml_lcd_gpio_group_ctrl[gpio_s.bank][0], (1 << (aml_lcd_gpio_group_ctrl[gpio_s.bank][1] + gpio_s.offset)));
		}
		else if (flag == LCD_GPIO_OUTPUT_HIGH) {
			aml_set_reg32_mask(aml_lcd_gpio_group_ctrl[gpio_s.bank][2], (1 << (aml_lcd_gpio_group_ctrl[gpio_s.bank][3] + gpio_s.offset)));
			aml_clr_reg32_mask(aml_lcd_gpio_group_ctrl[gpio_s.bank][0], (1 << (aml_lcd_gpio_group_ctrl[gpio_s.bank][1] + gpio_s.offset)));
		}
		else {
			aml_set_reg32_mask(aml_lcd_gpio_group_ctrl[gpio_s.bank][0], (1 << (aml_lcd_gpio_group_ctrl[gpio_s.bank][1] + gpio_s.offset)));
		}
	}
	return 0;
}

int aml_lcd_gpio_input_get(int gpio)
{
	struct aml_lcd_gpio_s gpio_s;
	int ret = 0;
	
	ret = aml_lcd_gpio_bank_offset(gpio, &gpio_s);
	if (ret == 0)
		ret = aml_get_reg32_bits(aml_lcd_gpio_group_ctrl[gpio_s.bank][4], (aml_lcd_gpio_group_ctrl[gpio_s.bank][5] + gpio_s.offset), 1);
	return ret;
}
#endif

#define AML_LCD_PINMUX_NUM    11
static unsigned int aml_lcd_pinmux_reg[] = {
	P_PERIPHS_PIN_MUX_0,
	P_PERIPHS_PIN_MUX_1,
	P_PERIPHS_PIN_MUX_2,
	P_PERIPHS_PIN_MUX_3,
	P_PERIPHS_PIN_MUX_4,
	P_PERIPHS_PIN_MUX_5,
	P_PERIPHS_PIN_MUX_6,
	P_PERIPHS_PIN_MUX_7,
	P_PERIPHS_PIN_MUX_8,
	P_PERIPHS_PIN_MUX_9,
	P_AO_RTI_PIN_MUX_REG,
};

int aml_lcd_pinmux_set(unsigned int mux_index, unsigned int mux_mask)
{
	if (mux_index < AML_LCD_PINMUX_NUM) {
		aml_set_reg32_mask(aml_lcd_pinmux_reg[mux_index], mux_mask);
		return 0;
	}
	else {
		printf("[error]: wrong pinmux index %d\n", mux_index);
		return -1;
	}
}

int aml_lcd_pinmux_clr(unsigned int mux_index, unsigned int mux_mask)
{
	if (mux_index < AML_LCD_PINMUX_NUM) {
		aml_clr_reg32_mask(aml_lcd_pinmux_reg[mux_index], mux_mask);
		return 0;
	}
	else {
		printf("[error]: wrong pinmux index %d\n", mux_index);
		return -1;
	}
}

