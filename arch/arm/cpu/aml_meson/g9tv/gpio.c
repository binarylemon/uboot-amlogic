/*
Linux gpio.C

*/


#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/gpio.h>
#include <amlogic/gpio.h>
extern void *malloc (size_t len);
extern void free(void*);

int gpio_debug=0;

struct gpio_addr
{
	unsigned long mode_addr;
	unsigned long out_addr;
	unsigned long in_addr;
};
static struct gpio_addr gpio_addrs[]=
{

    [PREG_PAD_GPIO0]={P_PREG_PAD_GPIO0_EN_N,P_PREG_PAD_GPIO0_O,P_PREG_PAD_GPIO0_I},
    [PREG_PAD_GPIO1]={P_PREG_PAD_GPIO1_EN_N,P_PREG_PAD_GPIO1_O,P_PREG_PAD_GPIO1_I},
    [PREG_PAD_GPIO2]={P_PREG_PAD_GPIO2_EN_N,P_PREG_PAD_GPIO2_O,P_PREG_PAD_GPIO2_I},
    [PREG_PAD_GPIO3]={P_PREG_PAD_GPIO3_EN_N,P_PREG_PAD_GPIO3_O,P_PREG_PAD_GPIO3_I},
    [PREG_PAD_GPIO4]={P_PREG_PAD_GPIO4_EN_N,P_PREG_PAD_GPIO4_O,P_PREG_PAD_GPIO4_I},
	[PREG_PAD_GPIO5]={P_PREG_PAD_GPIO5_EN_N,P_PREG_PAD_GPIO5_O,P_PREG_PAD_GPIO5_I},
    [PREG_PAD_GPIOAO] = {P_AO_GPIO_O_EN_N, P_AO_GPIO_O_EN_N, P_AO_GPIO_I},
    [PREG_JTAG_GPIO]={PREG_JTAG_GPIO_ADDR,PREG_JTAG_GPIO_ADDR,PREG_JTAG_GPIO_ADDR},
    [PREG_PAD_GPIO6]={P_PREG_PAD_GPIO6_EN_N,P_PREG_PAD_GPIO6_O,P_PREG_PAD_GPIO6_I},
};

int set_gpio_mode(gpio_bank_t bank,int bit,gpio_mode_t mode)
{
	unsigned long addr=gpio_addrs[bank].mode_addr;
	clrsetbits_le32(addr,1<<bit,mode<<bit);
	return 0;
}
gpio_mode_t get_gpio_mode(gpio_bank_t bank,int bit)
{
	unsigned long addr=gpio_addrs[bank].mode_addr;
    return (readl(addr)&(1<<bit))?(GPIO_INPUT_MODE):(GPIO_OUTPUT_MODE);
//	return (READ_CBUS_REG_BITS(addr,bit,1)>0)?(GPIO_INPUT_MODE):(GPIO_OUTPUT_MODE);
}


int set_gpio_val(gpio_bank_t bank,int bit,unsigned long val)
{
	unsigned long addr=gpio_addrs[bank].out_addr;
	val=val?1:0;
	clrsetbits_le32(addr,1<<bit,val<<bit);

	return 0;
}

unsigned long  get_gpio_val(gpio_bank_t bank,int bit)
{
	unsigned long addr=gpio_addrs[bank].in_addr;
	return (readl(addr)&(1<<bit))?1:0;
}

/**
 * enable gpio edge interrupt
 *
 * @param [in] pin  index number of the chip, start with 0 up to 255
 * @param [in] flag rising(0) or falling(1) edge
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
void gpio_enable_edge_int(int pin , int flag, int group)
{
	group &= 7;
	unsigned ireg = GPIO_INTR_GPIO_SEL0 + (group>>2);
	SET_CBUS_REG_MASK(ireg, pin<<((group&3)<<3));
	SET_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, ((flag<<(16+group)) | (1<<group)));
}
/**
 * enable gpio level interrupt
 *
 * @param [in] pin  index number of the chip, start with 0 up to 255
 * @param [in] flag high(0) or low(1) level
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
void gpio_enable_level_int(int pin , int flag, int group)
{
	group &= 7;
	unsigned ireg = GPIO_INTR_GPIO_SEL0 + (group>>2);
	SET_CBUS_REG_MASK(ireg, pin<<((group&3)<<3));
	SET_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, ((flag<<(16+group)) | (0<<group)));
}

/**
 * enable gpio interrupt filter
 *
 * @param [in] filter from 0~7(*222ns)
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
void gpio_enable_int_filter(int filter, int group)
{
	group &= 7;
	unsigned ireg = GPIO_INTR_FILTER_SEL0;
	SET_CBUS_REG_MASK(ireg, filter<<(group<<2));
}

int gpio_is_valid(int number)
{
	return 1;
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

void gpio_free(unsigned gpio)
{
}

int gpio_direction_input(unsigned gpio)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;
	set_gpio_mode(bank, bit, GPIO_INPUT_MODE);
	//printf( "set gpio%d.%d input\n", bank, bit);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;
	set_gpio_val(bank, bit, value?1:0);
	set_gpio_mode(bank, bit, GPIO_OUTPUT_MODE);
	//printf( "set gpio%d.%d output\n", bank, bit);
	return 0;
}

void gpio_set_value(unsigned gpio, int value)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;
	set_gpio_val(bank, bit, value?1:0);
}

int gpio_get_value(unsigned gpio)
{
	gpio_bank_t bank = (gpio_bank_t)(gpio >> 16);
	int bit = gpio & 0xFFFF;
	return (get_gpio_val(bank, bit));
}


#define AO 10
unsigned p_gpio_oen_addr[]={
	P_PREG_PAD_GPIO0_EN_N,
	P_PREG_PAD_GPIO1_EN_N,
	P_PREG_PAD_GPIO2_EN_N,
	P_PREG_PAD_GPIO3_EN_N,
	P_PREG_PAD_GPIO4_EN_N,
	P_PREG_PAD_GPIO5_EN_N,
	P_AO_GPIO_O_EN_N,
};
static unsigned p_gpio_output_addr[]={
	P_PREG_PAD_GPIO0_O,
	P_PREG_PAD_GPIO1_O,
	P_PREG_PAD_GPIO2_O,
	P_PREG_PAD_GPIO3_O,
	P_PREG_PAD_GPIO4_O,
	P_PREG_PAD_GPIO5_O,
	P_AO_GPIO_O_EN_N,
};
static unsigned p_gpio_input_addr[]={
	P_PREG_PAD_GPIO0_I,
	P_PREG_PAD_GPIO1_I,
	P_PREG_PAD_GPIO2_I,
	P_PREG_PAD_GPIO3_I,
	P_PREG_PAD_GPIO4_I,
	P_PREG_PAD_GPIO5_I,
	P_AO_GPIO_I,
};

unsigned p_pull_up_addr[]={
	P_PAD_PULL_UP_REG0,
	P_PAD_PULL_UP_REG1,
	P_PAD_PULL_UP_REG2,
	P_PAD_PULL_UP_REG3,
	P_PAD_PULL_UP_REG4,
	P_AO_RTI_PULL_UP_REG,
};
unsigned int p_pin_mux_reg_addr[]=
{
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
unsigned p_pull_upen_addr[]={
	P_PAD_PULL_UP_EN_REG0,
	P_PAD_PULL_UP_EN_REG1,
	P_PAD_PULL_UP_EN_REG2,
	P_PAD_PULL_UP_EN_REG3,
	P_PAD_PULL_UP_EN_REG4,
	P_AO_RTI_PULL_UP_REG,
};

int g9tv_pin_to_pullup(unsigned int pin ,unsigned int *reg,unsigned int *bit,unsigned int *bit_en)
{
	if(pin<=GPIOX_27)
	{
		*reg=4;
		*bit=pin-GPIOX_0;
		*bit_en=*bit;
	}
	else if(pin<=BOOT_18)
	{
		*reg=2;
		*bit=pin-BOOT_0;
		*bit_en=*bit;
	}
	else if(pin<=GPIOH_10) 
	{
		*reg=1;
		*bit=pin-GPIOH_0+16;
		*bit_en=*bit;
	}
	else if(pin<=GPIOZ_20)
	{
		*reg=3;
		*bit=pin-GPIOZ_0;
		*bit_en=*bit;
	}
	else if(pin<=GPIOW_20)
	{
		*reg=0;
		*bit=pin-GPIOW_0;
		*bit_en=*bit;
	}
	else if(pin<=GPIOAO_13)
	{
		*reg=5;
		*bit=pin;
		*bit_en=pin;
	}
	else if(pin<=CARD_8)
	{
		*reg=2;
		*bit=pin-CARD_0 + 20;
		*bit_en=*bit;
	}
	else if(pin == GPIOY_13)
	{
		*reg=1;
		*bit=pin-GPIOY_0;
		*bit_en=*bit;
	}
	else if(pin == GPIO_TEST_N)
	{
		*reg=5;
		*bit=pin-GPIO_TEST_N+14;
		*bit_en=pin-GPIO_TEST_N+30;
	}
	else
		return -1;
	return 0;

}

int g9tv_pin_map_to_direction(unsigned int pin,unsigned int *reg,unsigned int *bit)
{
	if(pin<=GPIOX_27)
	{
		*reg=4;
		*bit=pin-GPIOX_0;
	}
	else if(pin<=BOOT_18)
	{
		*reg=2;
		*bit=pin-BOOT_0;
	}
	else if(pin<=GPIOH_10) 
	{
		*reg=1;
		*bit=pin-GPIOH_0+16;
	}
	else if(pin<=GPIOZ_20)
	{
		*reg=3;
		*bit=pin-GPIOZ_0;
	}
	else if(pin<=GPIOW_20)
	{
		*reg=0;
		*bit=pin-GPIOW_0;
	}
	else if(pin<=GPIOAO_13)
	{
		*reg=5;
		*bit=pin+16;
	}
	else if(pin<=CARD_8)
	{
		*reg=2;
		*bit=pin-CARD_0 + 20;
	}
	else if(pin == GPIOY_13)
	{
		*reg=1;
		*bit=pin-GPIOY_0;
	}
	else if(pin == GPIO_TEST_N)
	{
		*reg=5;
		*bit=pin-GPIO_TEST_N+30;
	}
	else
		return -1;
	return 0;
}

extern struct amlogic_set_pullup pullup_ops;
extern int gpio_irq;
extern int gpio_flag;
#define NONE 0xFFFF
//#define debug
#ifdef debug
	#define gpio_print(...) printf(__VA_ARGS__)
#else 
	#define gpio_print(...)
#endif

#define printk printf
#define kzalloc malloc
#define kzfree free
//gpio subsystem set pictrl subsystem gpio owner
enum gpio_reg_type
{
	INPUT_REG,
	OUTPUT_REG,
	OUTPUTEN_REG
};

#define PIN_MAP(pin,reg,bit) \
{ \
	.num=pin, \
	.name=#pin, \
	.out_en_reg_bit=GPIO_REG_BIT(reg,bit), \
	.out_value_reg_bit=GPIO_REG_BIT(reg,bit), \
	.input_value_reg_bit=GPIO_REG_BIT(reg,bit), \
}
#define PIN_AOMAP(pin,en_reg,en_bit,out_reg,out_bit,in_reg,in_bit) \
{ \
	.num=pin, \
	.name=#pin, \
	.out_en_reg_bit=GPIO_REG_BIT(en_reg,en_bit), \
	.out_value_reg_bit=GPIO_REG_BIT(out_reg,out_bit), \
	.input_value_reg_bit=GPIO_REG_BIT(in_reg,in_bit), \
	.gpio_owner=NULL, \
}

#define PMUX(reg, bit)		((reg << 5) | bit)
#define PMUX_NONE		(0xFFFF)

static unsigned int gpio_to_pin[][9]={
	[GPIOX_0]	=	{PMUX(5,14),	PMUX(8,5),	PMUX(0,1),	PMUX(0,6),	PMUX(6,17),	PMUX(7,0),	PMUX(8,27),	PMUX(3,17),	PMUX(9,18)},
	[GPIOX_1]	=	{PMUX(5,13),	PMUX(8,4),	PMUX(0,1),	PMUX(0,6),	PMUX(6,16),	PMUX(7,1),	PMUX_NONE,	PMUX(3,16),	PMUX(9,17)},
	[GPIOX_2]	=	{PMUX(5,13),	PMUX(8,3),	PMUX(0,0),	PMUX(0,6),	PMUX_NONE,	PMUX(7,2),	PMUX_NONE,	PMUX(3,15),	PMUX_NONE},
	[GPIOX_3]	=	{PMUX(5,13),	PMUX(8,2),	PMUX(0,0),	PMUX(0,6),	PMUX_NONE,	PMUX(7,3),	PMUX_NONE,	PMUX(3,14),	PMUX_NONE},
	[GPIOX_4]	=	{PMUX(5,12),	PMUX_NONE,	PMUX(0,0),	PMUX(0,6),	PMUX(3,30),	PMUX(7,4),	PMUX(4,17),	PMUX(3,13),	PMUX_NONE},
	[GPIOX_5]	=	{PMUX(5,12),	PMUX_NONE,	PMUX(0,0),	PMUX(0,6),	PMUX(3,29),	PMUX(7,5),	PMUX(4,16),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_6]	=	{PMUX(5,12),	PMUX_NONE,	PMUX(0,0),	PMUX(0,6),	PMUX(3,28),	PMUX(7,6),	PMUX(4,15),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_7]	=	{PMUX(5,12),	PMUX_NONE,	PMUX(0,0),	PMUX(0,6),	PMUX(3,27),	PMUX(7,7),	PMUX(4,14),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_8]	=	{PMUX(5,11),	PMUX(8,1),	PMUX(0,3),	PMUX(0,6),	PMUX_NONE,	PMUX(7,8),	PMUX(8,26),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_9]	=	{PMUX(5,10),	PMUX(8,0),	PMUX(0,3),	PMUX(0,6),	PMUX_NONE,	PMUX(7,9),	PMUX(9,14),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_10]	=	{PMUX_NONE,	PMUX(3,22),	PMUX(0,2),	PMUX(0,6),	PMUX_NONE,	PMUX(7,10),	PMUX(7,31),	PMUX(3,12),	PMUX(9,19)},
	[GPIOX_11]	=	{PMUX_NONE,	PMUX(3,18),	PMUX(0,2),	PMUX(0,6),	PMUX_NONE,	PMUX(7,11),	PMUX(2,3),	PMUX(3,12),	PMUX_NONE},
	[GPIOX_12]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,2),	PMUX(0,6),	PMUX(3,7),	PMUX(7,12),	PMUX_NONE,	PMUX_NONE,	PMUX(4,13)},
	[GPIOX_13]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,2),	PMUX(0,6),	PMUX(3,11),	PMUX(7,13),	PMUX_NONE,	PMUX_NONE,	PMUX(4,12)},
	[GPIOX_14]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,2),	PMUX(0,6),	PMUX(3,11),	PMUX(7,14),	PMUX_NONE,	PMUX_NONE,	PMUX(4,11)},
	[GPIOX_15]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,2),	PMUX(0,6),	PMUX(3,11),	PMUX(7,15),	PMUX_NONE,	PMUX_NONE,	PMUX(4,10)},
	[GPIOX_16]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,5),	PMUX(0,6),	PMUX(3,11),	PMUX(7,16),	PMUX(8,25),	PMUX_NONE,	PMUX_NONE},
	[GPIOX_17]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,5),	PMUX(0,6),	PMUX(3,11),	PMUX(7,18),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOX_18]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,11),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOX_19]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,11),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOX_20]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,9),	PMUX_NONE,	PMUX(1,17),	PMUX(1,0),	PMUX_NONE},
	[GPIOX_21]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,7),	PMUX(1,3),	PMUX(1,16),	PMUX(1,4),	PMUX_NONE},
	[GPIOX_22]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,10),	PMUX(1,1),	PMUX_NONE,	PMUX(1,2),	PMUX_NONE},
	[GPIOX_23]	=	{PMUX_NONE,	PMUX(9,20),	PMUX(0,4),	PMUX(0,6),	PMUX(3,6),	PMUX_NONE,	PMUX_NONE,	PMUX(1,6),	PMUX_NONE},
	[GPIOX_24]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,19),	PMUX(0,9),	PMUX_NONE,	PMUX(1,7),	PMUX(8,24),	PMUX(1,9),	PMUX(4,21)},
	[GPIOX_25]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(0,18),	PMUX(0,8),	PMUX_NONE,	PMUX(1,8),	PMUX(8,23),	PMUX(1,5),	PMUX(4,20)},
	[GPIOX_26]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(0,7),	PMUX(7,30),	PMUX(3,25),	PMUX(8,22),	PMUX_NONE,	PMUX(4,19)},
	[GPIOX_27]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(8,29),	PMUX(0,10),	PMUX_NONE,	PMUX(8,19),	PMUX(8,28),	PMUX_NONE,	PMUX(4,18)},

	[BOOT_0]	=	{PMUX_NONE,	PMUX(4,30),	PMUX_NONE,	PMUX(6,29),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_1]	=	{PMUX_NONE,	PMUX(4,29),	PMUX_NONE,	PMUX(6,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_2]	=	{PMUX_NONE,	PMUX(4,29),	PMUX_NONE,	PMUX(6,27),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_3]	=	{PMUX_NONE,	PMUX(4,29),	PMUX_NONE,	PMUX(6,25),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_4]	=	{PMUX_NONE,	PMUX(4,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_5]	=	{PMUX_NONE,	PMUX(4,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_6]	=	{PMUX_NONE,	PMUX(4,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_7]	=	{PMUX_NONE,	PMUX(4,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_8]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_9]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_10]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_11]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(5,1),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_12]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(5,3),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_13]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(5,2),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_14]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_15]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_16]	=	{PMUX_NONE,	PMUX(4,27),	PMUX_NONE,	PMUX(6,25),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_17]	=	{PMUX_NONE,	PMUX(4,26),	PMUX_NONE,	PMUX(6,24),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[BOOT_18]	=	{PMUX_NONE,	PMUX_NONE,	PMUX(5,0),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},

	[GPIOH_0]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(7,20),	PMUX_NONE},
	[GPIOH_1]	=	{PMUX_NONE,	PMUX(4,9),	PMUX_NONE,	PMUX(3,26),	PMUX(7,29),	PMUX_NONE,	PMUX_NONE,	PMUX(7,19),	PMUX_NONE},
	[GPIOH_2]	=	{PMUX_NONE,	PMUX(4,8),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(7,28),	PMUX_NONE},
	[GPIOH_3]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(1,25),	PMUX_NONE},
	[GPIOH_4]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(1,24),	PMUX_NONE},
	[GPIOH_5]	=	{PMUX_NONE,	PMUX(4,7),	PMUX_NONE,	PMUX(4,3),	PMUX_NONE,	PMUX(1,19),	PMUX_NONE,	PMUX(1,26),	PMUX_NONE},
	[GPIOH_6]	=	{PMUX_NONE,	PMUX(4,6),	PMUX_NONE,	PMUX(4,2),	PMUX_NONE,	PMUX(1,18),	PMUX_NONE,	PMUX(1,23),	PMUX_NONE},
	[GPIOH_7]	=	{PMUX_NONE,	PMUX(6,23),	PMUX_NONE,	PMUX(5,8),	PMUX_NONE,	PMUX(3,24),	PMUX_NONE,	PMUX(7,24),	PMUX_NONE},
	[GPIOH_8]	=	{PMUX_NONE,	PMUX(6,22),	PMUX_NONE,	PMUX(5,9),	PMUX_NONE,	PMUX(9,15),	PMUX_NONE,	PMUX(7,23),	PMUX_NONE},
	[GPIOH_9]	=	{PMUX_NONE,	PMUX(9,12),	PMUX_NONE,	PMUX(9,13),	PMUX_NONE,	PMUX(9,16),	PMUX_NONE,	PMUX(7,22),	PMUX_NONE},
	[GPIOH_10]	=	{PMUX_NONE,	PMUX(9,10),	PMUX_NONE,	PMUX(9,11),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},

	[GPIOZ_0]	=	{PMUX_NONE,	PMUX(2,23),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_1]	=	{PMUX_NONE,	PMUX(2,22),	PMUX_NONE,	PMUX(3,19),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_2]	=	{PMUX_NONE,	PMUX(2,21),	PMUX_NONE,	PMUX(2,20),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_3]	=	{PMUX_NONE,	PMUX(2,19),	PMUX_NONE,	PMUX(2,18),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_4]	=	{PMUX_NONE,	PMUX(2,17),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_5]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(2,16),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_6]	=	{PMUX_NONE,	PMUX(6,14),	PMUX_NONE,	PMUX(3,21),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_7]	=	{PMUX_NONE,	PMUX(6,13),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_8]	=	{PMUX_NONE,	PMUX(6,12),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_9]	=	{PMUX_NONE,	PMUX(6,11),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_10]	=	{PMUX_NONE,	PMUX(6,10),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_11]	=	{PMUX_NONE,	PMUX(6,9),	PMUX_NONE,	PMUX(1,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_12]	=	{PMUX_NONE,	PMUX(6,8),	PMUX_NONE,	PMUX(1,27),	PMUX_NONE,	PMUX_NONE,	PMUX(2,2),	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_13]	=	{PMUX_NONE,	PMUX(6,7),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_14]	=	{PMUX_NONE,	PMUX(6,6),	PMUX_NONE,	PMUX(3,20),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_15]	=	{PMUX_NONE,	PMUX(6,5),	PMUX_NONE,	PMUX_NONE,	PMUX(11,16),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_16]	=	{PMUX_NONE,	PMUX(6,4),	PMUX_NONE,	PMUX_NONE,	PMUX(11,15),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_17]	=	{PMUX_NONE,	PMUX(6,3),	PMUX_NONE,	PMUX(2,27),	PMUX_NONE,	PMUX(7,28),	PMUX(2,1),	PMUX(5,31),	PMUX_NONE},
	[GPIOZ_18]	=	{PMUX_NONE,	PMUX(6,2),	PMUX_NONE,	PMUX(2,26),	PMUX_NONE,	PMUX(7,27),	PMUX(2,0),	PMUX(5,30),	PMUX_NONE},
	[GPIOZ_19]	=	{PMUX_NONE,	PMUX(6,1),	PMUX_NONE,	PMUX(2,25),	PMUX(11,14),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOZ_20]	=	{PMUX_NONE,	PMUX(6,0),	PMUX_NONE,	PMUX(2,24),	PMUX(11,13),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},

	[GPIOW_0]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(7,26),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(6,31),	PMUX_NONE},
	[GPIOW_1]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(7,25),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(6,30),	PMUX_NONE},
	[GPIOW_2]	=	{PMUX_NONE,	PMUX(8,16),	PMUX_NONE,	PMUX(8,15),	PMUX(11,18),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_3]	=	{PMUX_NONE,	PMUX(8,13),	PMUX_NONE,	PMUX(8,12),	PMUX(11,17),	PMUX(7,21),	PMUX(9,21),	PMUX_NONE,	PMUX_NONE},
	[GPIOW_4]	=	{PMUX_NONE,	PMUX(10,12),	PMUX_NONE,	PMUX_NONE,	PMUX(11,12),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_5]	=	{PMUX_NONE,	PMUX(10,11),	PMUX_NONE,	PMUX_NONE,	PMUX(11,11),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_6]	=	{PMUX_NONE,	PMUX(10,10),	PMUX_NONE,	PMUX_NONE,	PMUX(11,10),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_7]	=	{PMUX_NONE,	PMUX(10,9),	PMUX_NONE,	PMUX(11,22),	PMUX(11,9),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_8]	=	{PMUX_NONE,	PMUX(10,9),	PMUX_NONE,	PMUX(11,21),	PMUX(11,8),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_9]	=	{PMUX_NONE,	PMUX(10,8),	PMUX_NONE,	PMUX_NONE,	PMUX(11,7),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_10]	=	{PMUX_NONE,	PMUX(10,7),	PMUX_NONE,	PMUX_NONE,	PMUX(11,6),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_11]	=	{PMUX_NONE,	PMUX(10,6),	PMUX_NONE,	PMUX(11,20),	PMUX(11,5),	PMUX(10,13),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_12]	=	{PMUX_NONE,	PMUX(10,6),	PMUX_NONE,	PMUX(11,19),	PMUX(11,4),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_13]	=	{PMUX_NONE,	PMUX(10,5),	PMUX_NONE,	PMUX_NONE,	PMUX(11,3),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_14]	=	{PMUX_NONE,	PMUX(10,4),	PMUX_NONE,	PMUX_NONE,	PMUX(11,2),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_15]	=	{PMUX_NONE,	PMUX(10,3),	PMUX_NONE,	PMUX(11,24),	PMUX(11,2),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_16]	=	{PMUX_NONE,	PMUX(10,3),	PMUX_NONE,	PMUX(11,23),	PMUX(11,0),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOW_17]	=	{PMUX_NONE,	PMUX(10,2),	PMUX_NONE,	PMUX_NONE,	PMUX(6,21),	PMUX_NONE,	PMUX_NONE,	PMUX(10,25),	PMUX_NONE},
	[GPIOW_18]	=	{PMUX_NONE,	PMUX(10,1),	PMUX_NONE,	PMUX_NONE,	PMUX(6,20),	PMUX_NONE,	PMUX_NONE,	PMUX(10,24),	PMUX_NONE},
	[GPIOW_19]	=	{PMUX_NONE,	PMUX(10,0),	PMUX_NONE,	PMUX_NONE,	PMUX(6,19),	PMUX(10,29),	PMUX(10,28),	PMUX(10,23),	PMUX_NONE},
	[GPIOW_20]	=	{PMUX_NONE,	PMUX(10,0),	PMUX_NONE,	PMUX_NONE,	PMUX(6,18),	PMUX(10,27),	PMUX(10,26),	PMUX(10,22),	PMUX_NONE},

	[GPIOAO_0]	=	{PMUX_NONE,	PMUX(10,12),	PMUX_NONE,	PMUX(10,26),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_1]	=	{PMUX_NONE,	PMUX(10,11),	PMUX_NONE,	PMUX(10,25),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_2]	=	{PMUX_NONE,	PMUX(10,10),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_3]	=	{PMUX_NONE,	PMUX(10,9),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_4]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,24),	PMUX(10,2),	PMUX(10,6),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_5]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,23),	PMUX(10,1),	PMUX(10,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_6]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_7]	=	{PMUX_NONE,	PMUX(10,0),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_8]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,17),	PMUX(11,26),	PMUX(11,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_9]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,27),	PMUX(10,25),	PMUX(11,27),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_10]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_11]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,22),	PMUX(10,28),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_12]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,21),	PMUX(10,29),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOAO_13]	=	{PMUX_NONE,	PMUX(10,31),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},

	[CARD_0]	=	{PMUX_NONE,	PMUX(2,14),	PMUX_NONE,	PMUX(2,7),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_1]	=	{PMUX_NONE,	PMUX(2,15),	PMUX_NONE,	PMUX(2,6),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_2]	=	{PMUX_NONE,	PMUX(2,11),	PMUX_NONE,	PMUX(2,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_3]	=	{PMUX_NONE,	PMUX(2,10),	PMUX_NONE,	PMUX(2,4),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_4]	=	{PMUX_NONE,	PMUX(2,12),	PMUX_NONE,	PMUX(2,7),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(8,10),	PMUX_NONE},
	[CARD_5]	=	{PMUX_NONE,	PMUX(2,13),	PMUX_NONE,	PMUX(2,7),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(8,9),	PMUX_NONE},
	[CARD_6]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_7]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[CARD_8]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},

	[GPIOY_0]	=	{PMUX_NONE,	PMUX(3,3),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(1,15),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE},
	[GPIOY_1]	=	{PMUX_NONE,	PMUX(3,2),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(1,14),	PMUX_NONE,	PMUX_NONE,	PMUX(9,3)},
	[GPIOY_2]	=	{PMUX_NONE,	PMUX(3,1),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(1,13),	PMUX_NONE,	PMUX_NONE,	PMUX(9,2)},
	[GPIOY_3]	=	{PMUX_NONE,	PMUX(3,0),	PMUX_NONE,	PMUX(9,4),	PMUX(9,5),	PMUX(1,12),	PMUX_NONE,	PMUX_NONE,	PMUX(9,1)},
	[GPIOY_4]	=	{PMUX_NONE,	PMUX(3,4),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(5,29),	PMUX_NONE,	PMUX_NONE,	PMUX(9,0)},
	[GPIOY_5]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(5,28),	PMUX_NONE,	PMUX_NONE,	PMUX(9,0)},
	[GPIOY_6]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(5,27),	PMUX_NONE,	PMUX_NONE,	PMUX(9,0)},
	[GPIOY_7]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX(9,26),	PMUX(9,25),	PMUX(5,26),	PMUX(10,19),	PMUX(10,21),	PMUX(9,0)},
	[GPIOY_8]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX_NONE,	PMUX(9,24),	PMUX(5,25),	PMUX(10,18),	PMUX(10,20),	PMUX(9,0)},
	[GPIOY_9]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX(9,22),	PMUX(9,23),	PMUX(5,24),	PMUX_NONE,	PMUX_NONE,	PMUX(9,0)},
	[GPIOY_10]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,17),	PMUX(9,9),	PMUX(9,0)},
	[GPIOY_11]	=	{PMUX_NONE,	PMUX(3,5),	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(10,16),	PMUX(9,8),	PMUX(9,0)},
	[GPIOY_12]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(9,31),	PMUX(9,30),	PMUX_NONE,	PMUX(10,15),	PMUX(9,7),	PMUX_NONE},
	[GPIOY_13]	=	{PMUX_NONE,	PMUX_NONE,	PMUX_NONE,	PMUX(9,29),	PMUX(9,28),	PMUX_NONE,	PMUX(10,14),	PMUX(9,6),	PMUX_NONE},

	[GPIO_TEST_N]=	{PMUX_NONE, PMUX_NONE,  PMUX_NONE, PMUX(10,19),PMUX(10,3),PMUX(10,4), PMUX_NONE,      PMUX_NONE,  PMUX_NONE},
};
struct amlogic_gpio_desc amlogic_pins[]=
{
	PIN_MAP(GPIOX_0,	4,	0),
	PIN_MAP(GPIOX_1,	4,	1),
	PIN_MAP(GPIOX_2,	4,	2),
	PIN_MAP(GPIOX_3,	4,	3),
	PIN_MAP(GPIOX_4,	4,	4),
	PIN_MAP(GPIOX_5,	4,	5),
	PIN_MAP(GPIOX_6,	4,	6),
	PIN_MAP(GPIOX_7,	4,	7),
	PIN_MAP(GPIOX_8,	4,	8),
	PIN_MAP(GPIOX_9,	4,	9),
	PIN_MAP(GPIOX_10,	4,	10),
	PIN_MAP(GPIOX_11,	4,	11),
	PIN_MAP(GPIOX_12,	4,	12),
	PIN_MAP(GPIOX_13,	4,	13),
	PIN_MAP(GPIOX_14,	4,	14),
	PIN_MAP(GPIOX_15,	4,	15),
	PIN_MAP(GPIOX_16,	4,	16),
	PIN_MAP(GPIOX_17,	4,	17),
	PIN_MAP(GPIOX_18,	4,	18),
	PIN_MAP(GPIOX_19,	4,	19),
	PIN_MAP(GPIOX_20,	4,	20),
	PIN_MAP(GPIOX_21,	4,	21),
	PIN_MAP(GPIOX_22,	4,	22),
	PIN_MAP(GPIOX_23,	4,	23),
	PIN_MAP(GPIOX_24,	4,	24),
	PIN_MAP(GPIOX_25,	4,	25),
	PIN_MAP(GPIOX_26,	4,	26),
	PIN_MAP(GPIOX_27,	4,	27),

	PIN_MAP(BOOT_0,		2,	0),
	PIN_MAP(BOOT_1,		2,	1),
	PIN_MAP(BOOT_2,		2,	2),
	PIN_MAP(BOOT_3,		2,	3),
	PIN_MAP(BOOT_4,		2,	4),
	PIN_MAP(BOOT_5,		2,	5),
	PIN_MAP(BOOT_6,		2,	6),
	PIN_MAP(BOOT_7,		2,	7),
	PIN_MAP(BOOT_8,		2,	8),
	PIN_MAP(BOOT_9,		2,	9),
	PIN_MAP(BOOT_10,	2,	10),
	PIN_MAP(BOOT_11,	2,	11),
	PIN_MAP(BOOT_12,	2,	12),
	PIN_MAP(BOOT_13,	2,	13),
	PIN_MAP(BOOT_14,	2,	14),
	PIN_MAP(BOOT_15,	2,	15),
	PIN_MAP(BOOT_16,	2,	16),
	PIN_MAP(BOOT_17,	2,	17),
	PIN_MAP(BOOT_18,	2,	18),

	PIN_MAP(GPIOH_0,	1,	16),
	PIN_MAP(GPIOH_1,	1,	17),
	PIN_MAP(GPIOH_2,	1,	18),
	PIN_MAP(GPIOH_3,	1,	19),
	PIN_MAP(GPIOH_4,	1,	20),
	PIN_MAP(GPIOH_5,	1,	21),
	PIN_MAP(GPIOH_6,	1,	22),
	PIN_MAP(GPIOH_7,	1,	23),
	PIN_MAP(GPIOH_8,	1,	24),
	PIN_MAP(GPIOH_9,	1,	25),
	PIN_MAP(GPIOH_10,	1,	26),

	PIN_MAP(GPIOZ_0,	3,	0),
	PIN_MAP(GPIOZ_1,	3,	1),
	PIN_MAP(GPIOZ_2,	3,	2),
	PIN_MAP(GPIOZ_3,	3,	3),
	PIN_MAP(GPIOZ_4,	3,	4),
	PIN_MAP(GPIOZ_5,	3,	5),
	PIN_MAP(GPIOZ_6,	3,	6),
	PIN_MAP(GPIOZ_7,	3,	7),
	PIN_MAP(GPIOZ_8,	3,	8),
	PIN_MAP(GPIOZ_9,	3,	9),
	PIN_MAP(GPIOZ_10,	3,	10),
	PIN_MAP(GPIOZ_11,	3,	11),
	PIN_MAP(GPIOZ_12,	3,	12),
	PIN_MAP(GPIOZ_13,	3,	13),
	PIN_MAP(GPIOZ_14,	3,	14),
	PIN_MAP(GPIOZ_15,	3,	15),
	PIN_MAP(GPIOZ_16,	3,	16),
	PIN_MAP(GPIOZ_17,	3,	17),
	PIN_MAP(GPIOZ_18,	3,	18),
	PIN_MAP(GPIOZ_19,	3,	19),
	PIN_MAP(GPIOZ_20,	3,	20),

	PIN_MAP(GPIOW_0,	0,	0),
	PIN_MAP(GPIOW_1,	0,	1),
	PIN_MAP(GPIOW_2,	0,	2),
	PIN_MAP(GPIOW_3,	0,	3),
	PIN_MAP(GPIOW_4,	0,	4),
	PIN_MAP(GPIOW_5,	0,	5),
	PIN_MAP(GPIOW_6,	0,	6),
	PIN_MAP(GPIOW_7,	0,	7),
	PIN_MAP(GPIOW_8,	0,	8),
	PIN_MAP(GPIOW_9,	0,	9),
	PIN_MAP(GPIOW_10,	0,	10),
	PIN_MAP(GPIOW_11,	0,	11),
	PIN_MAP(GPIOW_12,	0,	12),
	PIN_MAP(GPIOW_13,	0,	13),
	PIN_MAP(GPIOW_14,	0,	14),
	PIN_MAP(GPIOW_15,	0,	15),
	PIN_MAP(GPIOW_16,	0,	16),
	PIN_MAP(GPIOW_17,	0,	17),
	PIN_MAP(GPIOW_18,	0,	18),
	PIN_MAP(GPIOW_19,	0,	19),
	PIN_MAP(GPIOW_20,	0,	20),

	PIN_AOMAP(GPIOAO_0,	6,	0,	6,	16,	6,	0),
	PIN_AOMAP(GPIOAO_1,	6,	1,	6,	17,	6,	1),
	PIN_AOMAP(GPIOAO_2,	6,	2,	6,	18,	6,	2),
	PIN_AOMAP(GPIOAO_3,	6,	3,	6,	19,	6,	3),
	PIN_AOMAP(GPIOAO_4,	6,	4,	6,	20,	6,	4),
	PIN_AOMAP(GPIOAO_5,	6,	5,	6,	21,	6,	5),
	PIN_AOMAP(GPIOAO_6,	6,	6,	6,	22,	6,	6),
	PIN_AOMAP(GPIOAO_7,	6,	7,	6,	23,	6,	7),
	PIN_AOMAP(GPIOAO_8,	6,	8,	6,	24,	6,	8),
	PIN_AOMAP(GPIOAO_9,	6,	9,	6,	25,	6,	9),
	PIN_AOMAP(GPIOAO_10,	6,	10,	6,	26,	6,	10),
	PIN_AOMAP(GPIOAO_11,	6,	11,	6,	27,	6,	11),
	PIN_AOMAP(GPIOAO_12,	6,	12,	6,	28,	6,	12),
	PIN_AOMAP(GPIOAO_13,	6,	13,	6,	29,	6,	13),

	PIN_MAP(CARD_0,		2,	20),
	PIN_MAP(CARD_1,		2,	21),
	PIN_MAP(CARD_2,		2,	22),
	PIN_MAP(CARD_3,		2,	23),
	PIN_MAP(CARD_4,		2,	24),
	PIN_MAP(CARD_5,		2,	25),
	PIN_MAP(CARD_6,		2,	26),
	PIN_MAP(CARD_7,		2,	27),
	PIN_MAP(CARD_8,		2,	28),

	PIN_MAP(GPIOY_0,	1,	0),
	PIN_MAP(GPIOY_1,	1,	1),
	PIN_MAP(GPIOY_2,	1,	2),
	PIN_MAP(GPIOY_3,	1,	3),
	PIN_MAP(GPIOY_4,	1,	4),
	PIN_MAP(GPIOY_5,	1,	5),
	PIN_MAP(GPIOY_6,	1,	6),
	PIN_MAP(GPIOY_7,	1,	7),
	PIN_MAP(GPIOY_8,	1,	8),
	PIN_MAP(GPIOY_9,	1,	9),
	PIN_MAP(GPIOY_10,	1,	10),
	PIN_MAP(GPIOY_11,	1,	11),
	PIN_MAP(GPIOY_12,	1,	12),
	PIN_MAP(GPIOY_13,	1,	13),
	
	PIN_AOMAP(GPIO_TEST_N,0,0,4,31,0,0),
};
int gpio_amlogic_requst(struct gpio_chip *chip,unsigned offset)
{
	int ret=0;
	unsigned int i,reg,bit;
	unsigned int *gpio_reg=&gpio_to_pin[offset][0];
	if(!ret){
		for(i=0;i<sizeof(gpio_to_pin[offset])/sizeof(gpio_to_pin[offset][0]);i++){
			if(gpio_reg[i]!=NONE)
			{
				reg=GPIO_REG(gpio_reg[i]);
				bit=GPIO_BIT(gpio_reg[i]);
				aml_clr_reg32_mask(p_pin_mux_reg_addr[reg],1<<bit);
				if(gpio_debug)
					printf("clear pinmux reg%d[%d]=%d\n",reg,bit,aml_get_reg32_bits(p_pin_mux_reg_addr[reg],bit,1));
				
			}
		}
	}
	return ret;
}
/* amlogic request gpio interface*/

void	 gpio_amlogic_free(unsigned offset)
{	
	return;
}

int gpio_amlogic_direction_input(struct gpio_chip *chip,unsigned offset)
{
	unsigned int reg,bit;
	reg=GPIO_REG(amlogic_pins[offset].out_en_reg_bit);
	bit=GPIO_BIT(amlogic_pins[offset].out_en_reg_bit);
	aml_set_reg32_mask(p_gpio_oen_addr[reg],1<<bit);
	if(gpio_debug)
		printf("set output en 0x%x[%d]=%d\n",p_gpio_oen_addr[reg],bit,aml_get_reg32_bits(p_gpio_oen_addr[reg],bit,1));
	return 0;
}

int gpio_amlogic_get(struct gpio_chip *chip,unsigned offset)
{
	unsigned int reg,bit;
	reg=GPIO_REG(amlogic_pins[offset].input_value_reg_bit);
	bit=GPIO_BIT(amlogic_pins[offset].input_value_reg_bit);
	return aml_get_reg32_bits(p_gpio_input_addr[reg],bit,1);
}

int gpio_amlogic_direction_output(struct gpio_chip *chip,unsigned offset, int value)
{
	unsigned int reg,bit;
	if(offset==GPIO_TEST_N){
		if(value)
			aml_set_reg32_mask(P_AO_GPIO_O_EN_N,1<<31);//out put high
		else
			aml_clr_reg32_mask(P_AO_GPIO_O_EN_N,1<<31);//out put low
		aml_set_reg32_mask(P_AO_SECURE_REG0,1);// out put enable
		return 0;
	}
	if(value){
		reg=GPIO_REG(amlogic_pins[offset].out_value_reg_bit);
		bit=GPIO_BIT(amlogic_pins[offset].out_value_reg_bit);
		aml_set_reg32_mask(p_gpio_output_addr[reg],1<<bit);
		gpio_print("out reg=%x,value=%x\n",p_gpio_output_addr[reg],aml_read_reg32(p_gpio_output_addr[reg]));
	}
	else{
		reg=GPIO_REG(amlogic_pins[offset].out_value_reg_bit);
		bit=GPIO_BIT(amlogic_pins[offset].out_value_reg_bit);
		aml_clr_reg32_mask(p_gpio_output_addr[reg],1<<bit);
		gpio_print("out reg=%x,value=%x\n",p_gpio_output_addr[reg],aml_read_reg32(p_gpio_output_addr[reg]));
	}
	reg=GPIO_REG(amlogic_pins[offset].out_en_reg_bit);
	bit=GPIO_BIT(amlogic_pins[offset].out_en_reg_bit);
	aml_clr_reg32_mask(p_gpio_oen_addr[reg],1<<bit);
	if(gpio_debug){
		printf("set output en 0x%x[%d]=%d\n",p_gpio_oen_addr[reg],bit,aml_get_reg32_bits(p_gpio_oen_addr[reg],bit,1));
		printf("set output val 0x%x[%d]=%d\n",p_gpio_output_addr[reg],bit,aml_get_reg32_bits(p_gpio_output_addr[reg],bit,1));
	}
	return 0;
}
void	gpio_amlogic_set(struct gpio_chip *chip,unsigned offset, int value)
{
	unsigned int reg,bit;
	reg=GPIO_REG(amlogic_pins[offset].out_value_reg_bit);
	bit=GPIO_BIT(amlogic_pins[offset].out_value_reg_bit);
	gpio_print("==%s==%d\n",__FUNCTION__,__LINE__);
	if(value)
		aml_set_reg32_mask(p_gpio_output_addr[reg],1<<bit);
	else
		aml_clr_reg32_mask(p_gpio_output_addr[reg],1<<bit);
}
int gpio_amlogic_name_to_num(const char *name)
{
	int i,tmp=100,num=0;
	int len=0;
	char *p=NULL;
	char *start=NULL;
	if(!name)
		return -1;
	
	if(!strcmp(name,"GPIO_TEST_N"))
		return GPIO_TEST_N;

	len=strlen(name);
	p=kzalloc(len+1);
	if(!p)
	{
		printk("%s:malloc error\n",__func__);
		return -1;
	}
	start=p;
	p=strcpy(p,name);
	for(i=0;i<len;p++,i++){		
		if(*p=='_'){
			*p='\0';
			tmp=i;
		}
		if(i>tmp&&*p>='0'&&*p<='9')
			num=num*10+*p-'0';
	}
	p=start;
	if(!strcmp(p,"GPIOX"))
		num=num+0;
	else if(!strcmp(p,"BOOT"))
		num=num+28;
	else if(!strcmp(p,"GPIOH"))
		num=num+47;
	else if(!strcmp(p,"GPIOZ"))
		num=num+58;
	else if(!strcmp(p,"GPIOW"))
		num=num+79;
	else if(!strcmp(p,"GPIOAO"))
		num=num+100;
	else if(!strcmp(p,"CARD"))
		num=num+114;
	else if(!strcmp(p,"GPIOY"))
		num=num+123;
	else
		num= -1;	
	kzfree(start);
	return num;
}

static int g9tv_set_pullup(unsigned pin, int val,unsigned int pullen)
{
	unsigned int reg=0,bit=0,bit_en=0,ret;
	ret=g9tv_pin_to_pullup(pin,&reg,&bit,&bit_en);
	if(!ret)
	{
		if(pullen){
			if(!ret)
			{
				if(val)
					aml_set_reg32_mask(p_pull_up_addr[reg],1<<bit);
				else
					aml_clr_reg32_mask(p_pull_up_addr[reg],1<<bit);
			}
			aml_set_reg32_mask(p_pull_upen_addr[reg],1<<bit_en);
		}
		else
			aml_clr_reg32_mask(p_pull_upen_addr[reg],1<<bit_en);
	}
	return ret;
}

static int g9tv_set_highz(unsigned int pin)
{
	g9tv_set_pullup((unsigned)pin,0,0);
	gpio_amlogic_direction_input(NULL,pin);
	return 0;
}

struct gpio_chip amlogic_gpio_chip={
	.request=gpio_amlogic_requst,
	.free=gpio_amlogic_free,
	.direction_input=gpio_amlogic_direction_input,
	.get=gpio_amlogic_get,
	.direction_output=gpio_amlogic_direction_output,
	.set=gpio_amlogic_set,
	.set_pullup=g9tv_set_pullup,
	.set_highz=g9tv_set_highz,
	.name_to_pin=gpio_amlogic_name_to_num,
};

