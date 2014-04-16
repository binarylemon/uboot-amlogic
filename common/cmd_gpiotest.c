#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <amlogic/gpio.h>

int do_gpiotest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int gpio_amlogic_requst(struct gpio_chip *chip ,unsigned offset);

U_BOOT_CMD(
	gpiotest, 8, 1,  do_gpiotest,
	"gpiotest gpio_group_name [-m group_mask] [-t repeat_read_times] [-d dealy_time]\n",
	"	gpio_group_name: should be x/y/dv/card/boot/h/z/ao\n"
	"	[group mask]: mask witch pins need to be tested\n "
	"	[repeat_read_times]: reading back times in one test\n"
	"	[delay_time]: delay time(us) between the two reading operation\n"
);

#define GPIO_PIN_MUX_REG0		0xc11080b0
#define GPIO_PIN_MUX_REG1		0xc11080b4
#define GPIO_PIN_MUX_REG2		0xc11080b8
#define GPIO_PIN_MUX_REG3		0xc11080bc			
#define GPIO_PIN_MUX_REG4		0xc11080c0
#define GPIO_PIN_MUX_REG5		0xc11080c4
#define GPIO_PIN_MUX_REG6		0xc11080c8
#define GPIO_PIN_MUX_REG7		0xc11080cc
#define GPIO_PIN_MUX_REG8		0xc11080d0
#define GPIO_PIN_MUX_REG9		0xc11080d4
#define GPIO_PIN_MUX_REG10		0xc11080d8
#define GPIO_PIN_MUX_REG11		0xc11080dc
#define GPIO_PIN_MUX_REG12		0xc11080e0

#define PINMUX_NUM			12
static unsigned pinmux_reg_org_val[PINMUX_NUM];

#define MAX_GPIOX_PIN_NUM		22
#define MAX_GPIOY_PIN_NUM		17
#define MAX_GPIODV_PIN_NUM		30
#define MAX_CARD_PIN_NUM		7
#define MAX_BOOT_PIN_NUM		19
#define MAX_GPIOH_PIN_NUM		10
#define MAX_GPIOZ_PIN_NUM		15
#define MAX_GPIOAO_PIN_NUM		14

struct gpio_group_info_s {
	const char* name;
	const unsigned start_num;
	const unsigned pins_num;
	const unsigned mask;
	const unsigned oen_reg_addr;
	const unsigned oen_start_bit;
	const unsigned output_reg_addr;
	const unsigned output_start_bit;
	const unsigned input_reg_addr;
	const unsigned input_start_bit;
	unsigned oen_org_val;
	unsigned output_org_val;
};

#define GPIO_X_OEN_REG			0xc1108030
#define GPIO_X_OUTPUT_REG		0xc1108034
#define GPIO_X_INPUT_REG		0xc1108038

#define GPIO_Y_OEN_REG			0xc110803C
#define GPIO_Y_OUTPUT_REG		0xc1108040
#define GPIO_Y_INPUT_REG		0xc1108044

#define GPIO_DV_OEN_REG			0xc1108048
#define GPIO_DV_OUTPUT_REG		0xc110804C
#define GPIO_DV_INPUT_REG		0xc1108050

#define GPIO_CARD_OEN_REG		0xc1108030
#define GPIO_CARD_OUTPUT_REG		0xc1108034
#define GPIO_CARD_INPUT_REG		0xc1108038

#define GPIO_BOOT_OEN_REG		0xc1108054
#define GPIO_BOOT_OUTPUT_REG		0xc1108058
#define GPIO_BOOT_INPUT_REG		0xc110805C

#define GPIO_H_OEN_REG			0xc1108054
#define GPIO_H_OUTPUT_REG		0xc1108058
#define GPIO_H_INPUT_REG		0xc110805C

#define GPIO_Z_OEN_REG			0xc110803C
#define GPIO_Z_OUTPUT_REG		0xc1108040
#define GPIO_Z_INPUT_REG		0xc1108044

#define GPIO_AO_OEN_REG			0xc8100024
#define GPIO_AO_OUTPUT_REG		0xc8100024
#define GPIO_AO_INPUT_REG		0xc8100028


struct gpio_group_info_s g_group_info[] = {
	{
		.name = "x",
		.start_num = GPIOX_0,
		.pins_num = 22,
		.mask = (1 << 22) - 1,
		.oen_reg_addr = GPIO_X_OEN_REG,
		.oen_start_bit = 0,
		.output_reg_addr = GPIO_X_OUTPUT_REG,
		.output_start_bit = 0,
		.input_reg_addr = GPIO_X_INPUT_REG,
		.input_start_bit = 0,
	}, {
		.name = "y",
		.start_num = GPIOY_0,
		.pins_num = 17,
		.mask = (1 << 17) - 1,
		.oen_reg_addr = GPIO_Y_OEN_REG,
		.oen_start_bit = 0,
		.output_reg_addr = GPIO_Y_OUTPUT_REG,
		.output_start_bit = 0,
		.input_reg_addr = GPIO_Y_INPUT_REG,
		.input_start_bit = 0,
	}, {
		.name = "dv",
		.start_num = GPIODV_0,
		.pins_num = 30,
		.mask = (1 << 30) - 1,
		.oen_reg_addr = GPIO_DV_OEN_REG,
		.oen_start_bit = 0,
		.output_reg_addr = GPIO_DV_OUTPUT_REG,
		.output_start_bit = 0,
		.input_reg_addr = GPIO_DV_INPUT_REG,
		.input_start_bit = 0,
	}, {
		.name = "card",
		.start_num = CARD_0,
		.pins_num = 7,
		.mask = (1 << 7) - 1,
		.oen_reg_addr = GPIO_CARD_OEN_REG,
		.oen_start_bit = 22,
		.output_reg_addr = GPIO_CARD_OUTPUT_REG,
		.output_start_bit = 22,
		.input_reg_addr = GPIO_CARD_INPUT_REG,
		.input_start_bit = 22,
	}, {
		.name = "boot",
		.start_num = BOOT_0,
		.pins_num = 22,
		.mask = (1 << 22) - 1,
		.oen_reg_addr = GPIO_BOOT_OEN_REG,
		.oen_start_bit = 0,
		.output_reg_addr = GPIO_BOOT_OUTPUT_REG,
		.output_start_bit = 0,
		.input_reg_addr = GPIO_BOOT_INPUT_REG,
		.input_start_bit = 0,
	}, {
		.name = "h",
		.start_num = GPIOH_0,
		.pins_num = 10,
		.mask = (1 << 10) - 1,
		.oen_reg_addr = GPIO_H_OEN_REG,
		.oen_start_bit = 19,
		.output_reg_addr = GPIO_H_OUTPUT_REG,
		.output_start_bit = 19,
		.input_reg_addr = GPIO_H_INPUT_REG,
		.input_start_bit = 19,
	}, {
		.name = "z",
		.start_num = GPIOZ_0,
		.pins_num = 15,
		.mask = (1 << 15) - 1,
		.oen_reg_addr = GPIO_Z_OEN_REG,
		.oen_start_bit = 17,
		.output_reg_addr = GPIO_Z_OUTPUT_REG,
		.output_start_bit = 17,
		.input_reg_addr = GPIO_Z_INPUT_REG,
		.input_start_bit = 17,
	}, {
		.name = "ao",
		.start_num = GPIOAO_0,
		.pins_num = 14,
		.mask = (1 << 14) - 1,
		.oen_reg_addr = GPIO_AO_OEN_REG,
		.oen_start_bit = 0,
		.output_reg_addr = GPIO_AO_OUTPUT_REG,
		.output_start_bit = 16,
		.input_reg_addr = GPIO_AO_INPUT_REG,
		.input_start_bit = 0,
	}
};

static struct gpio_group_info_s* get_group_info(char* name)
{
	struct gpio_group_info_s* group_info = NULL;
	int i;
	for (i = 0; i < ARRAY_SIZE(g_group_info); i++) {
		struct gpio_group_info_s* tmp_info;
		tmp_info = &g_group_info[i];
		if(!strcmp(tmp_info->name, name)) {
			group_info = tmp_info;
			break;
		}
	}
	return group_info;
}

#define GPIO_PIN(group_info, i) (group_info->start_num + i)


#define GPIO_MAX_PIN_NUM		22
#define GPIO_PIN_MASK			0x3ff7ff

#define TYPE_INT	0
#define TYPE_HEX	1

static int ssscanf(char *str, int type, unsigned *value)
{
    char *p;
    char c;
    int val = 0;
    p = str;

    c = *p;
    while (!((c >= '0' && c <= '9') || 
             (c >= 'a' && c <= 'f') ||
             (c >= 'A' && c <= 'F'))) {                     // skip other characters 
        p++;
        c = *p;
    }
    switch (type) {
    case TYPE_INT:
        c = *p;
        while (c >= '0' && c <= '9') {
            val *= 10;
            val += c - '0';   
            p++;
            c = *p;
        }
        break;

    case TYPE_HEX:
        if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
            p += 2;                         // skip '0x' '0X'
        }
        c = *p;
        while ((c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F')) {
            val = val * 16;
            if (c >= '0' && c <= '9') {
                val += c - '0';
            }
            if (c >= 'a' && c <= 'f') {
                val += (c - 'a' + 10);
            }
            if (c >= 'A' && c <= 'F') {
                val += (c - 'a' + 10);
            }
            p++;
            c = *p;
        }
        break;

    default:
        break;
    }

    *value = val;
    return p - str; 
}

static void record_org_pinmux(void)
{
	int i;
	for (i = 0; i < PINMUX_NUM; i++)
		pinmux_reg_org_val[i] = __raw_readl(GPIO_PIN_MUX_REG0+(i<<2));
}

static void set_gpio_pinmux(struct gpio_group_info_s* g_info, unsigned mask)
{
	int i;
	for (i = 0; i < g_info->pins_num; i++) {
		if (mask & (1 << i))
			gpio_amlogic_requst(NULL, GPIO_PIN(g_info, i));
	}
		
}

static void revert_gpiotest_pinmux(void)
{
	int i;
	for (i = 0; i < PINMUX_NUM; i++)
		__raw_writel(pinmux_reg_org_val[i],
				 GPIO_PIN_MUX_REG0+(i<<2));
}	

static int cmd_gpiotest(int argc, char * const argv[])
{
	int i, ret;
	unsigned delay_us = 0, repeat_time, mask, total_times = 1;
	unsigned oen_org, output_org;
	unsigned readbak_v;
	unsigned err_val = 0;
	struct gpio_group_info_s* g_info = NULL;
	
	g_info = get_group_info(argv[1]);
	if (g_info == NULL) {
		printf("gpio group name error!\n\n");
		return -1;
	}
	
	mask =  g_info->mask;
	
	for (i = 2; i < argc; i += 2) {
		if (!strcmp(argv[i], "-m"))
			ssscanf(argv[i+1], TYPE_HEX, &mask);
		else if (!strcmp(argv[i], "-t"))
			ssscanf(argv[i+1], TYPE_INT, &total_times);
		else if (!strcmp(argv[i], "-d"))
			ssscanf(argv[i+1], TYPE_INT, &delay_us);
		else {
			printf("args error\n\n");
			return -1;
		}
	}
	
	mask &= g_info->mask;
	
	//printf("mask %x, delay time %d, repeat count %d\n", mask, delay_us, total_times);
	
	//record the org pinmux value
	record_org_pinmux();
	
	//set the pinmux of gpiotest
	set_gpio_pinmux(g_info, mask);
		
	oen_org = __raw_readl(g_info->oen_reg_addr);
	output_org = __raw_readl(g_info->output_reg_addr);
	
	//printf("oen_org = %x, output_org = %x\n", oen_org, output_org);
	
	__raw_writel(oen_org & (~(mask)), g_info->oen_reg_addr);
	
	__raw_writel(output_org | mask, g_info->output_reg_addr);
	//printf("dir_cur = %x, val_cur = %x\n", 
	//		__raw_readl(gpiotest_PIN_DIR_REG), __raw_readl(gpiotest_PIN_VAL_REG));
	printf("\ntest high level\n");
	repeat_time = 0;
	ret = 0;
	do {
		if (delay_us > 0)
			udelay(delay_us);
		readbak_v = __raw_readl(g_info->input_reg_addr) & mask;
		
		//printf("readbak_v = %lx\n", readbak_v);
		
		//for test
		 //   readbak_v &= (~(1 << 5));
		
		for (i = 0; i < g_info->pins_num; i++) {
			if ((mask & (1 << i)) && !(readbak_v& (1 << i))) {
				//printf("gpiotest%d high level error\n", i);
				if (ret == 0) {
					err_val = readbak_v;
					printf( "error_val(right_val:0x%x)    time\n"     
						"0x%x                         %d\n", 
							mask, err_val, repeat_time);
				} 
				ret = -1;
			}
			if (readbak_v != err_val && ret != 0) {
				err_val = readbak_v;
				printf("0x%x                         %d\n", 
					err_val, repeat_time);
			}
		}
	} while (++repeat_time < total_times);
	
	if (ret != -1)
		printf("always ok!\n");
	
			
	__raw_writel(output_org & (~mask), g_info->output_reg_addr);	
	//printf("dir_cur = %x, val_cur = %x\n", 
	//		 __raw_readl(gpiotest_PIN_DIR_REG), __raw_readl(gpiotest_PIN_VAL_REG));
	
	printf("\ntest low level\n");
	
	repeat_time = 0;
	ret = 0;	
	do {		 
		if (delay_us > 0)
			udelay(delay_us);
			
		readbak_v = __raw_readl(g_info->input_reg_addr) & mask;
	
		//printf("readbak_v = %lx\n", readbak_v);
		
		//for test
		//    readbak_v |= (1 << 5);
		
		for (i = 0; i < g_info->pins_num; i++) {
			if ((mask & (1 << i)) && (readbak_v & (1 << i))) {
				if (ret == 0) {
					err_val = readbak_v;
					printf( "error_val(right_val:0x%x)    time\n"     
						"0x%x                         %d\n", 
						(~mask) & g_info->mask, err_val, repeat_time);
				} 
				ret = -1;
			}
			if (readbak_v != err_val && ret != 0) {
				err_val = readbak_v;
				printf("0x%x                           %d\n", 
					err_val, repeat_time);
			}
		}
		
	} while(++repeat_time < total_times);
	if (ret != -1)
		printf("always ok!\n");
		
	//revert the pinmux value	
	revert_gpiotest_pinmux();
		
	//write back all the gpio setting after the test.
	__raw_writel(oen_org, g_info->oen_reg_addr);
	__raw_writel(output_org, g_info->output_reg_addr);
	
	return 0;
}

int do_gpiotest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if(argc > 8 || argc & 1){
		cmd_usage(cmdtp);
		return -1;
	}
	
	return cmd_gpiotest(argc, argv);
}