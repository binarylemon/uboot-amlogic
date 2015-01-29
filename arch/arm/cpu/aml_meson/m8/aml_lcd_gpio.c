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
	"GPIOAO_12",
	"GPIOAO_13",
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
	"GPIOZ_13",
	"GPIOZ_14",
	"GPIOH_0",
	"GPIOH_1",
	"GPIOH_2",
	"GPIOH_3",
	"GPIOH_4",
	"GPIOH_5",
	"GPIOH_6",
	"GPIOH_7",
	"GPIOH_8",
	"GPIOH_9",
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
	"BOOT_18",
	"CARD_0",
	"CARD_1",
	"CARD_2",
	"CARD_3",
	"CARD_4",
	"CARD_5",
	"CARD_6",
	"GPIODV_0",
	"GPIODV_1",
	"GPIODV_2",
	"GPIODV_3",
	"GPIODV_4",
	"GPIODV_5",
	"GPIODV_6",
	"GPIODV_7",
	"GPIODV_8",
	"GPIODV_9",
	"GPIODV_10",
	"GPIODV_11",
	"GPIODV_12",
	"GPIODV_13",
	"GPIODV_14",
	"GPIODV_15",
	"GPIODV_16",
	"GPIODV_17",
	"GPIODV_18",
	"GPIODV_19",
	"GPIODV_20",
	"GPIODV_21",
	"GPIODV_22",
	"GPIODV_23",
	"GPIODV_24",
	"GPIODV_25",
	"GPIODV_26",
	"GPIODV_27",
	"GPIODV_28",
	"GPIODV_29",
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
	"GPIOY_16",
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
	"GPIO_TEST_N",
	"GPIO_MAX",
};
#endif

static unsigned int aml_lcd_gpio_group_ctrl[][6] = {
	//oen_reg,             oen_bit,  out_reg,            out_bit,  in_reg,            in_bit
	{P_AO_GPIO_O_EN_N,      0,       P_AO_GPIO_O_EN_N,   16,       P_AO_GPIO_I,        0, }, //GPIOAO
	{P_PREG_PAD_GPIO1_EN_N, 17,      P_PREG_PAD_GPIO1_O, 17,       P_PREG_PAD_GPIO1_I, 17,}, //GPIOZ
	{P_PREG_PAD_GPIO3_EN_N, 19,      P_PREG_PAD_GPIO3_O, 19,       P_PREG_PAD_GPIO3_I, 19,}, //GPIOH
	{P_PREG_PAD_GPIO3_EN_N, 0,       P_PREG_PAD_GPIO3_O, 0,        P_PREG_PAD_GPIO3_I, 0, }, //BOOT
	{P_PREG_PAD_GPIO0_EN_N, 22,      P_PREG_PAD_GPIO0_O, 22,       P_PREG_PAD_GPIO0_I, 22,}, //CARD
	{P_PREG_PAD_GPIO2_EN_N, 0,       P_PREG_PAD_GPIO2_O, 0,        P_PREG_PAD_GPIO2_I, 0, }, //GPIODV
	{P_PREG_PAD_GPIO1_EN_N, 0,       P_PREG_PAD_GPIO1_O, 0,        P_PREG_PAD_GPIO1_I, 0, }, //GPIOY
	{P_PREG_PAD_GPIO0_EN_N, 0,       P_PREG_PAD_GPIO0_O, 0,        P_PREG_PAD_GPIO0_I, 0, }, //GPIOX
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
	
	if ((gpio>=GPIOAO_0) && (gpio<=GPIOAO_13)) {
		gpio_s->bank = 0;
		gpio_s->offset = gpio - GPIOAO_0;
	}
	else if ((gpio>=GPIOZ_0) && (gpio<=GPIOZ_14)) {
		gpio_s->bank = 1;
		gpio_s->offset = gpio - GPIOZ_0;
	}
	else if ((gpio>=GPIOH_0) && (gpio<=GPIOH_9)) {
		gpio_s->bank = 2;
		gpio_s->offset = gpio - GPIOH_0;
	}
	else if ((gpio>=BOOT_0) && (gpio<=BOOT_18)) {
		gpio_s->bank = 3;
		gpio_s->offset = gpio - BOOT_0;
	}
	else if ((gpio>=CARD_0) && (gpio<=CARD_6)) {
		gpio_s->bank = 4;
		gpio_s->offset = gpio - CARD_0;
	}
	else if ((gpio>=GPIODV_0) && (gpio<=GPIODV_29)) {
		gpio_s->bank = 5;
		gpio_s->offset = gpio - GPIODV_0;
	}
	else if ((gpio>=GPIOY_0) && (gpio<=GPIOY_16)) {
		gpio_s->bank = 6;
		gpio_s->offset = gpio - GPIOY_0;
	}
	else if ((gpio>=GPIOX_0) && (gpio<=GPIOX_21)) {
		gpio_s->bank = 7;
		gpio_s->offset = gpio - GPIOX_0;
	}
	else if (gpio==GPIO_TEST_N) {
		printf("don't support GPIO_TEST_N Port\n");
		ret = -2;
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

