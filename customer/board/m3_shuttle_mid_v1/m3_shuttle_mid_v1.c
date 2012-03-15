#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <share_kernel.h>
#include <asm/arch/gpio.h>
#include <malloc.h>

#ifdef CONFIG_SARADC
#include <asm/saradc.h>
#endif /*CONFIG_SARADC*/

#ifdef CONFIG_AML_I2C
#include <aml_i2c.h>
#include <asm/arch/io.h>
#endif /*CONFIG_AML_I2C*/

#include <font/ISO_88591_18.h>
#include <font/ISO_88591_20.h>
#include <font/ISO_88591_24.h>
#include <font/ISO_88591_32.h>
#include <font/ISO_88591_40.h>

DECLARE_GLOBAL_DATA_PTR;
extern void mdelay(unsigned long msec);

#ifdef __GNUC__
#define IS_NOT_USED __attribute__ ((unused)) 
#else 
#define IS_NOT_USED 
#endif 

#ifdef CONFIG_PMU_ACT8942
#include <act8942.h>  

#define msleep(a) udelay(a * 1000)

IS_NOT_USED static void power_off(void)
{
    //Power hold down
    set_gpio_val(GPIOAO_bank_bit0_11(6), GPIOAO_bit_bit0_11(6), 0);
    set_gpio_mode(GPIOAO_bank_bit0_11(6), GPIOAO_bit_bit0_11(6), GPIO_OUTPUT_MODE);
}

/*
 *	DC_DET(GPIOA_20)	enable internal pullup
 *		High:		Disconnect
 *		Low:		Connect
 */
static inline int is_ac_online(void)
{
#if 0
	int val;
	
	SET_CBUS_REG_MASK(PAD_PULL_UP_REG0, (1<<20));	//enable internal pullup
	set_gpio_mode(GPIOA_bank_bit0_27(20), GPIOA_bit_bit0_27(20), GPIO_INPUT_MODE);
	val = get_gpio_val(GPIOA_bank_bit0_27(20), GPIOA_bit_bit0_27(20));
	
	printf("%s: get from gpio is %d.\n", __FUNCTION__, val);
	
	return !val;
#endif
	return 1;
}

//temporary
static inline int is_usb_online(void)
{
	//u8 val;

	return 0;
}


/*
 *	nSTAT OUTPUT(GPIOA_21)	enable internal pullup
 *		High:		Full
 *		Low:		Charging
 */
static inline int get_charge_status(void)
{
	int val;
	
	SET_CBUS_REG_MASK(PAD_PULL_UP_REG0, (1<<21));	//enable internal pullup
	set_gpio_mode(GPIOA_bank_bit0_27(21), GPIOA_bit_bit0_27(21), GPIO_INPUT_MODE);
	val = get_gpio_val(GPIOA_bank_bit0_27(21), GPIOA_bit_bit0_27(21));

	printf("%s: get from gpio is %d.\n", __FUNCTION__, val);
	
	return val;
}

/*
 *	When BAT_SEL(GPIOA_22) is High Vbat=Vadc*2
 */
static inline int measure_voltage(void)
{
	int val;
	msleep(2);
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 1);
	val = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: get from adc is %duV.\n", __FUNCTION__, val);
	return val;
}

/*
 *	Get Vhigh when BAT_SEL(GPIOA_22) is High.
 *	Get Vlow when BAT_SEL(GPIOA_22) is Low.
 *	I = Vdiff / 0.02R
 *	Vdiff = Vhigh - Vlow
 */
static inline int measure_current(void)
{
	int val, Vh, Vl, Vdiff;
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 1);
	msleep(2);
	Vl = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: Vl is %duV.\n", __FUNCTION__, Vl);
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 0);
	msleep(2);
	Vh = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: Vh is %duV.\n", __FUNCTION__, Vh);
	Vdiff = Vh - Vl;
	val = Vdiff * 50;
	printf("%s: get from adc is %duA.\n", __FUNCTION__, val);
	return val;
}

static inline int measure_capacity(void)
{
	int val, tmp;
	tmp = measure_voltage();
	if((tmp>4200000) || (get_charge_status() == 0x1))
	{
		printf("%s: get from PMU and adc is 100.\n", __FUNCTION__);
		return 100;
	}
	
	val = (tmp - 3600000) / (600000 / 100);
	printf("%s: get from adc is %d.\n", __FUNCTION__, val);
	return val;
}

//temporary
static void set_bat_off(void)
{
	return;
}

static struct act8942_operations act8942_pdata = {
	.is_ac_online = is_ac_online,
	.is_usb_online = is_usb_online,
	.set_bat_off = set_bat_off,
	.get_charge_status = NULL,
	.measure_voltage = measure_voltage,
	.measure_current = measure_current,
	.measure_capacity_charging = measure_capacity,
	.measure_capacity_battery = measure_capacity,
	.update_period = 2000,	//2S
};

#endif

#ifdef CONFIG_AML_I2C
static void board_i2c_set_pinmux(void)
{
    //refer to Q07CL_DSN_RB_0922A.pdf & AppNote-M3-CorePinMux.xlsx
    /*********************************************/
    /*          | AO I2C_Master | AO I2C_Slave |                 */
    /*********************************************/
    /*          | I2C_SCK       | I2C_SCK_SLAVE|*/
    /* GPIOAO_4 | [AO_REG:6]    | [AO_REG:2]   |*/
    /*********************************************/
    /*          | I2C_SDA       | I2C_SDA_SLAVE|*/
    /* GPIOAO_5 | [AO_REG:5]    | [AO_REG:1]   | */
    /*********************************************/
    //disable all other pins which share with I2C_SDA_B & I2C_SCK_B
    clrbits_le32(P_AO_RTI_PIN_MUX_REG,((1<<1)|(1<<2)));
    //enable I2C MASTER B pins
	setbits_le32(P_AO_RTI_PIN_MUX_REG,((1<<5)|(1<<6)));
	
    //udelay(10000);
}

#define I2C_ACT8942QJ133_ADDR   (0x5B)

void i2c_act8942_write(unsigned char reg, unsigned char val)
{
    unsigned char buff[2];
    buff[0] = reg;
    buff[1] = val;

	struct i2c_msg msg[] = {
        {
        .addr  = I2C_ACT8942QJ133_ADDR,
        .flags = 0,
        .len   = 2,
        .buf   = buff,
        }
    };

    if (aml_i2c_xfer(msg, 1) < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
    }
}

unsigned char i2c_act8942_read(unsigned char reg)
{
    unsigned char val = 0;
    struct i2c_msg msgs[] = {
        {
            .addr = I2C_ACT8942QJ133_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = I2C_ACT8942QJ133_ADDR,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &val,
        }
    };

    if ( aml_i2c_xfer(msgs, 2)< 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
    }

    return val;
}
void board_Q07CL_DSN_RB_0922A_i2c_test(void)
{
	/*@Q07CL_DSN_RB_0922A.pdf*/
	/*@DS_ACT8942_PrA_22Jun11_M.pdf*/
	unsigned char act8942_reg_id_lst[] = {
	0x00,0x01,0x20,0x21,0x22,0x30,
	0x31,0x32,0x40,0x41,0x42,0x50,
	0x51,0x54,0x55,0x60,0x61,0x64,
	0x65,0x70,0x71,0x78,0x79,0x7A,
	};
	int nIdx = 0;
	printf("[M3 shuttle MID]-[Q07CL_DSN_RB_0922A]-[AO-I2C]-[ACT8942QJ133-T] dump begin:\n");
	for(nIdx = 0;nIdx < sizeof(act8942_reg_id_lst)/sizeof(act8942_reg_id_lst[0]);++nIdx)
		printf("Reg addr=0x%02x Val=0x%02x\n",
		act8942_reg_id_lst[nIdx],
		i2c_act8942_read(act8942_reg_id_lst[nIdx]));

	printf("[M3 shuttle MID]-[Q07CL_DSN_RB_0922A]-[AO-I2C]-[ACT8942QJ133-T] dump end.\n\n");
}

//Amlogic I2C param setting for board "Q07CL_DSN_RB_0922A.pdf"
//will be used by function:  int aml_i2c_init(void) @ drivers\i2c\aml_i2c.c
//refer following doc for detail:
//board schematic: Q07CL_DSN_RB_0922A.pdf
//pinmux setting: AppNote-M3-CorePinMux.xlsx
struct aml_i2c_platform g_aml_i2c_plat = {
    .wait_count         = 1000000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no          = AML_I2C_MASTER_AO,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_400K,
    .master_ao_pinmux = {
        .scl_reg    = MESON_I2C_MASTER_AO_GPIOAO_4_REG,
        .scl_bit    = MESON_I2C_MASTER_AO_GPIOAO_4_BIT,
        .sda_reg    = MESON_I2C_MASTER_AO_GPIOAO_5_REG,
        .sda_bit    = MESON_I2C_MASTER_AO_GPIOAO_5_BIT,
    }
};

static void board_i2c_init(void)
{		
	//set I2C pinmux with PCB board layout
	//refer Q07CL_DSN_RB_0922A.pdf
	board_i2c_set_pinmux();

	//Amlogic I2C controller initialized
	//note: it must be call before any I2C operation
	aml_i2c_init();

	//must call aml_i2c_init(); before any I2C operation	

	/*M3 shuttle MID board*/
	//board_Q07CL_DSN_RB_0922A_i2c_test();	

	//udelay(10000);	
	
}
//for sys_test only, not check yet
static struct i2c_board_info aml_i2c_info[] = {
    {
        I2C_BOARD_INFO("I2C PMU(ACT8942)", I2C_ACT8942QJ133_ADDR),
        .device_init = board_i2c_init,
    },
};

struct aml_i2c_device aml_i2c_devices={
	.aml_i2c_boards = (struct i2c_borad_info *)aml_i2c_info,
	.dev_num = sizeof(aml_i2c_info)/sizeof(struct i2c_board_info)
};
#endif /*CONFIG_AML_I2C*/

#if CONFIG_JERRY_NAND_TEST //temp test
#include <amlogic/nand/platform.h>
#include <asm/arch/nand.h>
#include <asm/arch/clock.h>
#include <linux/mtd/partitions.h>
#include <amlogic/debug.h>
static void claim_bus(uint32_t get)
{
	if(get==NAND_BUS_RELEASE)
	{
		NAND_IO_DISABLE(0);
	}else{
		NAND_IO_ENABLE(1);
	}
}
static struct aml_nand_platform nand_plat={
/*
		uint32_t        reg_base;
		    uint32_t        delay;
		    uint32_t        rbmod;
		    uint32_t        t_rea;
		    uint32_t        t_rhoh;
		    uint32_t        ce_num;
		    uint32_t        clk_src;
		    claim_bus_t     claim_bus;
*/
		.ce_num=4,
		.rbmod=1,
		.clk_src=CLK81,
		.claim_bus=claim_bus
};
void    board_mynand_init(void)
{
	nanddebug(1,"NAND is inited\n");
	nand_plat.clk_src=CLK81;
	nand_probe(&nand_plat);
//	cntl_init(&nand_plat);
//	amlnand_probe();
}

#elif CONFIG_NAND_AML_M3 //temp test
//#include <amlogic/nand/platform.h>
#include <asm/arch/nand.h>
#include <linux/mtd/partitions.h>


static struct aml_nand_platform aml_nand_mid_platform[] = {
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_SHORT_MODE),
            },
        },
        .rbpin_mode=1,
        .short_pgsz=384,
        .ran_mode=0,
        .T_REA = 20,
        .T_RHOH = 15,
    },
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0) ,  //| (AML_NAND_CE1 << 4) | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
        .ready_busy_pad = (AML_NAND_CE0) ,  //| (AML_NAND_CE0 << 4) | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5| NAND_ECC_BCH30_1K_MODE),
            },
        },
        .rbpin_mode = 1,
        .short_pgsz = 0,
        .ran_mode = 0,
        .T_REA = 20,
        .T_RHOH = 15,
    }
    
};

struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = 2,
};
#endif


#ifdef CONFIG_USB_DWC_OTG_HCD
#include <asm/arch/usb.h>
//note: try with some M3 pll but only following can work
//USB_PHY_CLOCK_SEL_M3_XTAL @ 1 (24MHz)
//USB_PHY_CLOCK_SEL_M3_XTAL_DIV2 @ 0 (12MHz)
//USB_PHY_CLOCK_SEL_M3_DDR_PLL @ 43 (528MHz)
struct amlogic_usb_config g_usb_config_m3_shuttle={
	USB_PHY_CLOCK_SEL_M3_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M3_USBPORT_BASE,
	USB_ID_MODE_SW_HOST,
	0, //set_vbus_power
};
#endif


int board_init(void)
{
	gd->bd->bi_arch_number=2958; //MACH_TYPE_MESON_8626M;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

#ifdef CONFIG_AML_I2C  
	board_i2c_init();
#endif /*CONFIG_AML_I2C*/

#ifdef CONFIG_USB_DWC_OTG_HCD
	board_usb_init(&g_usb_config_m3_shuttle);
#endif
	
	return 0;
}

#ifdef CONFIG_SARADC
/*following key value are test with board 
  [M3 Shuttle MID project]
  ref doc:
  1. Q07CL_DSN_RB_0922A.pdf
  2. M3-Periphs-Registers.docx (Pg43-47)
*/
static struct adckey_info g_key_K1_info[] = {
    {"SW1", 180, 60},
};
static struct adckey_info g_key_K2_info[] = {
    {"SW2", 400, 60},
};

static struct adc_info g_adc_info[] = {
    {"Press Key SW1", AML_ADC_CHAN_4, ADC_KEY,&g_key_K1_info},
    {"Press Key SW2", AML_ADC_CHAN_4, ADC_KEY,&g_key_K2_info},
    {"Press Key N/A", AML_ADC_CHAN_5, ADC_OTHER, NULL},
};

struct adc_device aml_adc_devices={
	.adc_device_info = g_adc_info,
	.dev_num = sizeof(g_adc_info)/sizeof(struct adc_info)
};

/* adc_init(&g_adc_info, ARRAY_SIZE(g_adc_info)); */
/* void adc_init(struct adc_info *adc_info, unsigned int len) 
     @trunk/common/sys_test.c */

/*following is test code to test ADC & key pad*/
/*SW1: 180; SW2:400*/
/*
#include <asm/saradc.h>
static int do_adc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{	
	saradc_enable();	
	u32 nDelay = 0xffff;
	int nKeyVal = 0;
	int nCnt = 0;
	#define ADC_CHECK_MAX 3
	printf("\nM3 shuttle MID ADC check begin!\nTotal %d times to check, press key to start...\n",ADC_CHECK_MAX);
	while(nCnt < ADC_CHECK_MAX)
	{
		udelay(nDelay);
		nKeyVal = get_adc_sample(4);
		if(nKeyVal > 1000)
			continue;
		
		printf("%d : get_key(): %d",(nCnt+1), nKeyVal);
		int nKeyIndex;
		int nKeyMax = aml_adc_devices.dev_num;
		char *pInfo="";
		for(nKeyIndex = 0;nKeyIndex<nKeyMax;++nKeyIndex)
		{

			if(ADC_OTHER == aml_adc_devices.adc_device_info[nKeyIndex].adc_type)
			{
				pInfo = aml_adc_devices.adc_device_info[nKeyIndex].tint;
				break;
			}
			else
			{
			adckey_info_t * pKeyInfo = (adckey_info_t*)aml_adc_devices.adc_device_info[nKeyIndex].adc_data;
			if(pKeyInfo)
			{
				if(nKeyVal > (pKeyInfo->value - pKeyInfo->tolerance) && 
					nKeyVal < (pKeyInfo->value + pKeyInfo->tolerance))
					{
						pInfo = aml_adc_devices.adc_device_info[nKeyIndex].tint;
						break;
					}
			}
			else{
				pInfo = aml_adc_devices.adc_device_info[nKeyMax-1].tint;
				break;
				}
			}
		}
		printf("  -> %s!\n",pInfo);
		nCnt++;
	}
	
	saradc_disable();

	printf("M3 shuttle MID ADC check end!\n\n");
}

U_BOOT_CMD(
	adc,1,	1,	do_adc,
	"M3 ADC check",
	"\n"
);
*/
#endif

u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}

#if CONFIG_CMD_MMC
#include <mmc.h>
#include <asm/arch/sdio.h>
static int inand_reset_flag;

#ifdef AML_CARD_SD_INFO_DETAILED
static int  sdio_init(unsigned port,struct aml_card_sd_info *sdio)
#else
static int  sdio_init(unsigned port)
#endif
{
	//setbits_le32(P_PREG_CGPIO_EN_N,1<<5);
    //todo add card detect 	
	//setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
    switch(port)
    {
    case SDIO_PORT_A:
        break;
    case SDIO_PORT_B:
        break;
    case SDIO_PORT_C:    	
         if(inand_reset_flag)
        {
            inand_reset_flag = 0;
            printf("inand reset\n");
            //clrbits_le32(P_PERIPHS_PIN_MUX_2,(1<<24));
            clrbits_le32(P_PREG_PAD_GPIO3_EN_N,(1<<9));
            clrbits_le32(P_PREG_PAD_GPIO3_O,(1<<9));
            mdelay(5);
        //mdelay(50);
            setbits_le32(P_PREG_PAD_GPIO3_O,1<<9);
        }                
        break;
    case SDIO_PORT_XC_A:
        break;
    case SDIO_PORT_XC_B:
        break;
    case SDIO_PORT_XC_C:
        break;
    default:
        break;
    }
    return cpu_sdio_init(port);
}

#ifdef AML_CARD_SD_INFO_DETAILED
static int  sdio_detect(unsigned port,struct aml_card_sd_info *sdio)
#else
static int  sdio_detect(unsigned port)
#endif
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
	        setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
	        //return readl(P_PREG_PAD_GPIO5_I)&(1<<29)?1:0;//low detected card
	        return readl(P_PREG_PAD_GPIO5_I)&(1<<29)?0:1;//high detected card
	    break;
	    case SDIO_PORT_C:
	        return 0;//have card is inserted
	        break;
	    case SDIO_PORT_XC_A:
	        break;
	    case SDIO_PORT_XC_B:
	        break;
	    case SDIO_PORT_XC_C:
	        break;
	    default:
	        break;
    }
    return -1;//error 
}
#ifdef AML_CARD_SD_INFO_DETAILED
static void sdio_pwr_prepare(unsigned port,struct aml_card_sd_info *sdio)
#else
static void sdio_pwr_prepare(unsigned port)
#endif
{
    /// @todo NOT FINISH
	///do nothing here
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            break;
        case SDIO_PORT_C:
            break;
        default:
            break;
    }
}
#ifdef AML_CARD_SD_INFO_DETAILED
static void sdio_pwr_on(unsigned port,struct aml_card_sd_info *sdio)
#else
static void sdio_pwr_on(unsigned port)
#endif
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
        	clrbits_le32(P_PREG_PAD_GPIO5_O,(1<<31));
        	clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
            /// @todo NOT FINISH
            #ifdef AML_CARD_SD_INFO_DETAILED
        	sdio->sdio_pwr_flag |= CARD_SD_SDIO_PWR_ON;
        	#endif
            break;
        case SDIO_PORT_C:
        	inand_reset_flag = 1;
            break;
        default:
            break;
    }
}
#ifdef AML_CARD_SD_INFO_DETAILED
static void sdio_pwr_off(unsigned port,struct aml_card_sd_info *sdio)
#else
static void sdio_pwr_off(unsigned port)
#endif
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
        //	setbits_le32(P_PREG_CGPIO_O,(1<<5));
        //	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
        	setbits_le32(P_PREG_PAD_GPIO5_O,(1<<31));
        	clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));//GPIOD13
        	/// @todo NOT FINISH
        	#ifdef AML_CARD_SD_INFO_DETAILED
        	sdio->sdio_pwr_flag |= CARD_SD_SDIO_PWR_OFF;
        	#endif
	        break;
	    case SDIO_PORT_C:
	        inand_reset_flag = 0;	    	
	        break;
	    default:
	        break;
	}
}
static void board_mmc_register(unsigned port)
{
    struct aml_card_sd_info *aml_priv=cpu_sdio_get(port);
    
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    if(aml_priv==NULL||mmc==NULL)
        return;
    aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	#ifdef AML_CARD_SD_INFO_DETAILED
    aml_priv->sdio_pwr_flag = 0;
	#endif
	if(port == SDIO_PORT_C)
	{
	    inand_reset_flag = 1;
	}
	sdio_register(mmc,aml_priv);
#if 0    
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 300000;
	mmc->f_min = 200000;
	mmc->f_max = 50000000;
	mmc_register(mmc);
#endif	
}
int board_mmc_init(bd_t	*bis)
{
//	board_mmc_register(SDIO_PORT_A);
	board_mmc_register(SDIO_PORT_B);  //TF card
	board_mmc_register(SDIO_PORT_C);  //inand
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif

static int is_ac_connected(void)
{
extern int pmu_is_ac_online(void);	
	return pmu_is_ac_online();
}

int logo_display(void)
{
    int ret = 0;    
    run_command ("mmc read 1 ${loadaddr} ${aml_logo_start} ${aml_logo_size}", 0);
    ret = run_command ("bmp display ${loadaddr}", 0);    
    run_command ("video dev bl_on", 0);    
    return ret;
}

inline void display_messge(char *msg)
{
#ifdef ENABLE_FONT_RESOURCE    
    run_command ("video clear", 0);
    //AsciiPrintf(msg, 250, 200, 0x80ff80);
    AsciiPrintf(msg, 0, 0, 0x00ff00);
    run_command ("video dev bl_on", 0);
#else
	printf("%s\n",msg);
#endif    
}

//Power Function
inline void power_up(void)
{
	clrbits_le32(P_AO_GPIO_O_EN_N, ((1<<2)|(1<<6)));
	setbits_le32(P_AO_GPIO_O_EN_N,((1<<18)|(1<<22)));
}

inline void power_down(void)
{
	clrbits_le32(P_AO_GPIO_O_EN_N, ((1<<2)|(1<<6)));
	setbits_le32(P_AO_GPIO_O_EN_N,((1<<18)|(1<<22)));
}
//end of Power Function

//powerkey function
static  int powerkey_hold_count = -1; 

static inline int powerkey_init(void)
{
	clrbits_le32(P_RTC_ADDR0, (1<<11));
	clrbits_le32(P_RTC_ADDR1, (1<<3));
    return 0;
}

int powerkey_scan(void)
{
	return (((readl(P_RTC_ADDR1) >> 2) & 1) ? 0 : 1);
}

int powerkey_hold(unsigned long hold_time)
{
    unsigned long polling_time = 100, tmp;

    if(powerkey_hold_count == -1)
    {
        powerkey_init();
        
    }

    if(!hold_time)
    {
        tmp = powerkey_scan();
        printf("powerkey: %ld\n", tmp);
        if((!tmp) || (powerkey_hold_count < 0))
        {
            powerkey_hold_count = -2;
            return  0;
        }
        else
        {
            return  ++powerkey_hold_count;
        }
    }

    while(hold_time > 0)
    {
        mdelay(polling_time);
        tmp = powerkey_scan();
        //printf("powerkey: %d\n", tmp);
        if(!tmp)  break;
        if(hold_time <= polling_time)
        {
            hold_time = 0;
        }
        else
        {
            hold_time -= polling_time;
        }
    }
    if(hold_time > 0)
    {
        return  0;
    }
    return  1;
}
//end of powerkey function

//Upgrade Function
#define NAND_CLEAN

int upgrade_bootloader(void)
{
    int	i = 0;  //, j = 0 , retry = 2;
    char	str[128];
   // unsigned long   size, size1;
   // char    *filepath;
    
    printf("u-boot upgrading...\n");
    if(run_command ("mmcinfo", 0))
    {
        printf("##	ERROR: SD card not find!!!\n");
    }
    else
    {
        printf("Find SD card!!!\n");

        for(i = 0; i < SCAN_DEVICE_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d ${bootloader_path}", (i + 1));
            printf("command:    %s\n", str);
            if(!run_command (str, 0))
            {
				sprintf(str, "fatload mmc 0:%d ${loadaddr} ${bootloader_path}", (i + 1));
				printf("command:    %s\n", str);
				run_command (str, 0);
				run_command ("mw.b 820001ff 0 1", 0);	//added by Elvis, for broke FAT ID of BPB
				run_command ("mmc write 1 ${loadaddr} 0 300", 0);

				return 1;

            }
        }
    }
    return 0;
}

typedef unsigned short word;
typedef unsigned long int dword;

struct BITMAPFILEHEADER {
    word  Type;
    dword bfSize;
    word  bfReserved1;
    word  bfReserved2;
    dword bfOffBits;
};

struct BITMAPINFOHEADER {
    dword biSize;
    dword biWidth;
    dword biHeight;
    word  biPlanes;
    word  biBitCount;
    dword biCompression;
    dword biSizeImage;
    dword biXPelsPerMeter;
    dword biYPelsPerMeter;
    dword biClrUsed;
    dword biClrImportant;
};

struct bmp_head_t {
    struct BITMAPFILEHEADER file_head;
    struct BITMAPINFOHEADER bitmap_head;
};

static void prepare_black_bmp(char * addr, int w, int h)
{
    struct bmp_head_t bmp_head = {
        {
            .Type = 0x4D42,
            .bfSize = 800*600*3 + 54,
            .bfReserved1 = 0,
            .bfReserved2 = 0,
            .bfOffBits = 54
        },
        {
            .biSize = 40,
            .biWidth = 800,
            .biHeight = 600,
            .biPlanes = 1,
            .biBitCount = 24,
            .biCompression = 0,
            .biSizeImage = 800*600*3,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed = 0,
            .biClrImportant = 0,
        },
    };

    bmp_head.file_head.bfSize = w*h*3 + 54;
    bmp_head.bitmap_head.biWidth = w;
    bmp_head.bitmap_head.biHeight = h;
    bmp_head.bitmap_head.biSizeImage = w*h*3;

    memset(addr, 0, w*h*3+54);
    *addr++ = 'B';
    *addr++ = 'M';
    memcpy(addr, &bmp_head.file_head.bfSize, sizeof(bmp_head)-sizeof(bmp_head.file_head.Type));

}

void into_recovery(void)
{
    int	i = 0,ret = 0; // j = 0, 
    char	str[128];
    
    printf("Recovery Start...\n");
    run_command ("mmc read 1 8410000 ${logo_start} ${logo_size}", 0);
    ret = run_command ("bmp info 84100000", 0);
    printf("bmp info ret = %d.\n", ret);
    if (ret < 0) {
        prepare_black_bmp(((char*)0x84100000), OSD_WIDTH, OSD_HEIGHT);
    }
    
    if(!run_command ("mmcinfo", 0))
    {
        for(i = 0; i < SCAN_DEVICE_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d ${recovery_path}", (i + 1));
            if(!run_command (str, 0))
            {
                printf("recovery in SD Card!!!\n");
                sprintf(str, "fatload mmc 0:%d ${loadaddr} ${recovery_path}", (i + 1));
                run_command (str, 0);
                run_command ("bootm", 0);
            }
        }
    }

    printf("recovery in emmc!!!\n");
    run_command ("mmc read 1 ${loadaddr} ${recovery_start} ${recovery_size}", 0);

    run_command ("bootm", 0);
}
//end of Upgrade Function

#ifdef CONFIG_SWITCH_BOOT_MODE

inline int get_key(void)
{
    int adc_val = get_adc_sample(4);
    printf("get_adc_sample(4): 0x%x\n", adc_val);
    return(((adc_val >= 0x0) && (adc_val < 0x200)) ? 1 : 0);
}

#ifdef CONFIG_VOLTAGE_AO12
void vccao_1_2v(void)
{	//change vcc ao from 1.25v to 1.2v
	act8942_i2c_write(0x20,0x18);
	act8942_i2c_write(0x21,0x18);
}
#endif

#ifdef CONFIG_VOLTAGE_DDR15
void ddr_1_5_v(void)
{ //change ddr from 1.55v to 1.5v
	act8942_i2c_write(0x30,0x1E);
	act8942_i2c_write(0x31,0x1E);	
}
#endif

int switch_boot_mode(void)
{
	unsigned long hold_time = 50000, polling_time = 10000, tmp;
    unsigned long upgrade_step;
	act8942_init(&act8942_pdata);
	
 //	//act8942_dump();
//
	int pmu=0;	
	//act8942_init(&act8942_pdata);
//	//act8942_dump();
//	 //REG1 => 1.3V
	act8942_i2c_write(0x20, 0x1a);
// 	//REG1 => 1.35V
//	//act8942_i2c_write(0x20, 0x1b);
//	//REG3 => 3.3V
	act8942_i2c_write(0x40, 0x39);	
	pmu = act8942_i2c_read(0x20);
	printf("ACT8942_REG1_ADDR: %x \n\n",pmu);
		
/*#ifdef CONFIG_VOLTAGE_AO12
	vccao_1_2v();
#endif
#ifdef CONFIG_VOLTAGE_DDR15
	ddr_1_5_v();
#endif*/
	
	upgrade_step = simple_strtoul (getenv ("upgrade_step"), NULL, 16);
	printf("upgrade_step = %ld\n", upgrade_step);
		
	saradc_enable();
	
#ifdef ENABLE_FONT_RESOURCE
	RegisterFont(DEFAULT_FONT);
#endif

    //added by Elvis for added fool idle
	get_key();
	get_key();

    if(get_key())
    {
extern int aml_autoscript(void);    	
	    aml_autoscript();
    }
	powerkey_hold(0);
	
	if(upgrade_step == 2)
	{
		switch(reboot_mode)
		{
			case AMLOGIC_NORMAL_BOOT:
			{
				printf("AMLOGIC_NORMAL_BOOT...\n");
				power_up();
				logo_display();
				return	1;
			}
			case AMLOGIC_FACTORY_RESET_REBOOT:
			{
				printf("AMLOGIC_FACTORY_RESET_REBOOT...\n");
				power_up();
				logo_display();
				into_recovery();
				break;
			}
			case AMLOGIC_UPDATE_REBOOT:
			{
				printf("AMLOGIC_UPDATE_REBOOT...\n");
				power_up();
				logo_display();
				run_command ("set upgrade_step 0", 0);
				run_command ("save", 0);
				upgrade_step = 0;
				break;
			}
			default:
			{
				printf("AMLOGIC_CHARGING_REBOOT...\n");
					if(is_ac_connected())   //if(is_ac_online_sh() || measure_capacity_battery_sh())
				{
					power_up();
#ifdef CONFIG_BATTERY_CHARGING
					battery_charging();
#endif
					logo_display();
				}
				else
				{
					powerkey_hold(0);
#ifdef CONFIG_BATTERY_CHARGING
					if(get_powerkey_hold_count())
					{
						  logo_display();
						if(get_battery_percentage() < 10)
						{
							power_low_display();
							sdelay(2);
							power_down();
							printf("Low Power!!!\nPower Down!\n");
							hang();
						}
#else
					if(powerkey_hold(1000))
					{
#endif
						logo_display();
						power_up();
						printf("Power Up!\n");
					}
					else
					{
						power_down();
						printf("Power Down!\n");
						hang();
					}
				}
				break;
			}
		}
	}
	else
	{
		power_up();
		printf("Upgrade step %ld...\n", upgrade_step);
	}

	if(upgrade_step == 0)
	{
		display_messge("upgrade step 1! Don't Power Off!");
		if(upgrade_bootloader())
		{
			run_command ("set upgrade_step 1", 0);
			run_command ("save", 0);
			run_command ("reset", 0);
			hang();
		}
		else
		{
			printf("### ERROR:	u-boot write failed!!!\n");
			return	-1;
		}
	}
	else if(upgrade_step == 1)
	{
		display_messge("upgrade step 2! Don't Power Off!");
		run_command ("defenv", 0);
		run_command ("set upgrade_step 2", 0);
		run_command ("save", 0);
		into_recovery();
	}


	//added by Elvis for added fool idle
	//get_key();
	//get_key();
	
	while(hold_time > 0)
	{
		udelay(polling_time);
		tmp = get_key();
		printf("get_key(): %ld\n", tmp);
		if(!tmp)  break;
		hold_time -= polling_time;
	}

	if(hold_time > 0)
	{
		printf("Normal Start...\n");
		return	1;
	}
	else
	{
		display_messge("upgrading... please wait");
		if(upgrade_bootloader())
		{
			run_command ("set upgrade_step 1", 0);
			run_command ("save", 0);
			run_command ("reset", 0);
			hang();
		}
		run_command ("set upgrade_step 2", 0);
		run_command ("save", 0);
		into_recovery();
	}


return	0;
}
#endif


