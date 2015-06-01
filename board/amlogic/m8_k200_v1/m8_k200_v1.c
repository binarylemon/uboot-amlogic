#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <malloc.h>
#ifdef CONFIG_UART_A_FUNCTION_ADD
#include <asm/arch/uart.h>
#endif

#if defined(CONFIG_CMD_NET)
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/io.h>
#endif /*(CONFIG_CMD_NET)*/

#if defined(CONFIG_AML_I2C)
#include <aml_i2c.h>
#include <asm/arch/io.h>
#endif /*CONFIG_AML_I2C*/

#ifdef CONFIG_PLATFORM_HAS_PMU
#include <amlogic/aml_pmu_common.h>
#endif

#ifdef CONFIG_MESON_TRUSTZONE
#include <asm/arch/trustzone.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
#include <amlogic/battery_parameter.h>
/*
 * add board battery parameters, this is a backup option if uboot process
 * battery parameters failed, each board shoud use your own battery parameters
 */
int config_battery_rdc = 87;
struct battery_curve config_battery_curve[] = {
    {3132,      0,      0},
    {3273,      0,      0},
    {3414,      0,      0},
    {3555,      0,      0},
    {3625,      1,      3},
    {3660,      2,      8},
    {3696,      3,     16},
    {3731,     10,     24},
    {3766,     15,     38},
    {3801,     26,     48},
    {3836,     42,     56},
    {3872,     52,     63},
    {3942,     66,     74},
    {4012,     79,     85},
    {4083,     90,     94},
    {4153,    100,    100}
};
#endif

const char * env_args_reserve[]=
{
"480i_x",
"480i_y",
"480i_w",
"480i_h",
"480p_x",
"480p_y",
"480p_w",
"480p_h",
"576i_x",
"576i_y",
"576i_w",
"576i_h",
"576p_x",
"576p_y",
"576p_w",
"576p_h",
"720p_x",
"720p_y",
"720p_w",
"720p_h",
"1080i_x",
"1080i_y",
"1080i_w",
"1080i_h",
"1080p_x",
"1080p_y",
"1080p_w",
"1080p_h",
"4k2k24hz_x",
"4k2k24hz_y",
"4k2k24hz_w",
"4k2k24hz_h",
"4k2k25hz_x",
"4k2k25hz_y",
"4k2k25hz_w",
"4k2k25hz_h",
"4k2k30hz_x",
"4k2k30hz_y",
"4k2k30hz_w",
"4k2k30hz_h",
"4k2ksmpte_x",
"4k2ksmpte_y",
"4k2ksmpte_w",
"4k2ksmpte_h",
"digitaudiooutput",
"defaulttvfrequency",
"has.accelerometer",
"cecconfig",
"cvbsmode",
"hdmimode",
"outputmode",
"auto_update_enable",
"disp.fromleft",
NULL
};

#if defined(CONFIG_CMD_NET)
/*************************************************
  * Amlogic Ethernet controller operation
  *
  * Note: The LAN chip LAN8720 need to be reset
  *
  *************************************************/
static void setup_net_chip(void)
{
	//m8 only use externel clock
	/* setup ethernet clk */
#ifdef IP101PHY
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0xf00); // clock Input 50 inverted : bit14 =1 Div : 6:0 = 0 En : bit8 = 1  Sel : bit 11:9 = 7
#else
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x4f00); // clock Input 50 inverted : bit14 =1 Div : 6:0 = 0 En : bit8 = 1  Sel : bit 11:9 = 7
#endif
	/* setup ethernet pinmux use gpioz(5-14) */
#ifdef KSZ8091
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0xf00); // clock Input 50 inverted : bit14 =1 Div : 6:0 = 0 En : bit8 = 1  Sel : bit 11:9 = 7
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1<<15)|(1 << 14) | (1 << 13) | (1 << 12) |
	(1 << 11) | (1 << 8 ) | (1 << 7 ) | (1 << 10 ) | (1 << 6 ) | (1 << 5 ));
#else
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1 << 14) | (1 << 13) | (1 << 12) |
	(1 << 11) | (1 << 8 ) | (1 << 7 ) | (1 << 10 ) | (1 << 6 ) | (1 << 5 ));
#endif
	/* setup ethernet mode */
	WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x241);//bit6-4 :001 rmii mode
	CLEAR_CBUS_REG_MASK(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
	/* hardware reset ethernet phy : gpioz14 connect phyreset pin*/
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_EN_N, 1 << 31);
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 31);
	udelay(10000);
	SET_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 31);
}
static void setup_new_net_chip(void)
{
        eth_aml_reg0_t eth_reg0;
         //setup ethernet clk need calibrate to configre
#ifdef RMII_PHY_INTERFACE
        /* setup ethernet pinmux use gpioz(5-14) */
        SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1 << 14) | (1 << 13) | (1 << 12) |
        (1 << 11) | (1 << 8 ) | (1 << 7 ) | (1 << 10 ) | (1 << 6 ) | (1 << 5 ));
        eth_reg0.d32 = 0;
        eth_reg0.b.phy_intf_sel = 0;
        eth_reg0.b.data_endian = 0;
        eth_reg0.b.desc_endian = 0;
        eth_reg0.b.rx_clk_rmii_invert = 0;
        eth_reg0.b.rgmii_tx_clk_src = 0;
        eth_reg0.b.rgmii_tx_clk_phase = 0;
        eth_reg0.b.rgmii_tx_clk_ratio = 1;
        eth_reg0.b.phy_ref_clk_enable = 1;
        eth_reg0.b.clk_rmii_i_invert = 1;
        eth_reg0.b.clk_en = 1;
        eth_reg0.b.adj_enable = 1;
        eth_reg0.b.adj_setup = 0;
        eth_reg0.b.adj_delay = 18;
        eth_reg0.b.adj_skew = 0;
        eth_reg0.b.cali_start = 0;
        eth_reg0.b.cali_rise = 0;
        eth_reg0.b.cali_sel = 0;
        eth_reg0.b.rgmii_rx_reuse = 0;
        eth_reg0.b.eth_urgent = 0;
        WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, eth_reg0.d32 );//1          //rmii mode
        WRITE_CBUS_REG(0x2050,0x1000);//1          //rmii mode
#elif RGMII_PHY_INTERFACE
        SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0xffef);
        eth_reg0.d32 = 0;
        eth_reg0.b.phy_intf_sel = 1;
        eth_reg0.b.data_endian = 0;
        eth_reg0.b.desc_endian = 0;
        eth_reg0.b.rx_clk_rmii_invert = 0;
        eth_reg0.b.rgmii_tx_clk_src = 0;
        eth_reg0.b.rgmii_tx_clk_phase = 0;
        eth_reg0.b.rgmii_tx_clk_ratio = 2;
        eth_reg0.b.phy_ref_clk_enable = 1;
        eth_reg0.b.clk_rmii_i_invert = 1;
        eth_reg0.b.clk_en = 1;
        eth_reg0.b.adj_enable = 1;
        eth_reg0.b.adj_setup = 1;
        eth_reg0.b.adj_delay = 4;
        eth_reg0.b.adj_skew = 0xc;
        eth_reg0.b.cali_start = 0;
        eth_reg0.b.cali_rise = 0;
        eth_reg0.b.cali_sel = 0;
        eth_reg0.b.rgmii_rx_reuse = 0;
        eth_reg0.b.eth_urgent = 0;
        WRITE_CBUS_REG(0x2050, eth_reg0.d32);// rgmii mode
        SET_CBUS_REG_MASK(0x10a5,1<<27);
        WRITE_CBUS_REG(0x2050,0x7d21);// rgmii mode
        SET_CBUS_REG_MASK(0x108a,0xb803);
        SET_CBUS_REG_MASK(HHI_MPLL_CNTL9,(1638<<0)| (0<<14)|(1<<15) | (1<<14) | (5<<16) | (0<<25) | (0<<26) |(0<<30) | (0<<31));
#endif

#ifdef RGMII_PHY_INTERFACE
        /* setup ethernet mode */
        CLEAR_CBUS_REG_MASK(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
        /* hardware reset ethernet phy : gpioz14 connect phyreset pin*/
        CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_EN_N, 1 << 28);
        CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO2_O, 1 << 28);
        udelay(10000);
        SET_CBUS_REG_MASK(PREG_PAD_GPIO2_O, 1 << 28);
#else
        /* setup ethernet mode */
        CLEAR_CBUS_REG_MASK(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
        /* hardware reset ethernet phy : gpioz14 connect phyreset pin*/
        CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_EN_N, 1 << 31);
        CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 31);
        udelay(10000);
        SET_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 31);
#endif
}
int board_eth_init(bd_t *bis)
{
	if(IS_MESON_M8M2_CPU){//m8m2
        //printf("m8m2 ethernet\n");
        setup_new_net_chip();
	}
	else{//m8
        //printf("m8 ethernet\n");
		setup_net_chip();
	}
    udelay(1000);
	extern int aml_eth_init(bd_t *bis);
    aml_eth_init(bis);
	return 0;
}
#endif /* (CONFIG_CMD_NET) */

u32 get_board_rev(void)
{

	return 0x20;
}

#if CONFIG_AML_HDMI_TX
/*
 * Init hdmi related power configuration
 * Refer to your board SCH, power including HDMI5V, HDMI1.8V, AVDD18_HPLL, etc
 */
extern void hdmi_tx_power_init(void);
void hdmi_tx_power_init(void)
{
    //
//    printf("hdmi tx power init\n");
}
#endif

#if CONFIG_CMD_MMC
#include <mmc.h>
#include <asm/arch/sdio.h>
static int  sdio_init(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            //todo add card detect
            setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
            break;
        case SDIO_PORT_C:
            //enable pull up
            clrbits_le32(P_PAD_PULL_UP_REG3, 0xff<<0);
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

extern unsigned sdio_debug_1bit_flag;

static int  sdio_detect(unsigned port)
{
    int ret;
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
            ret=readl(P_PREG_PAD_GPIO5_I)&(1<<29)?0:1;
            printf( " %s return %d\n",__func__,ret);

			if(!(readl(P_PREG_PAD_GPIO0_I)&(1<<26))){ //sd_d3 low, debug board in
				if(!(readl(P_PREG_PAD_GPIO0_I)&(1<<22))){
					printf("sdio debug board detected, sd card with 1bit mode\n");
		 			sdio_debug_1bit_flag = 1;
		 		}
		 		else{
		 			printf("sdio debug board detected, no sd card in\n");
		 			sdio_debug_1bit_flag = 0;
		 			return 1;
		 		}
		 	}

            break;
        case SDIO_PORT_C:
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

    return 0;
}

static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
	cpu_sdio_pwr_prepare(port);
}

static void sdio_pwr_on(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            clrbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
			/// @todo NOT FINISH
            break;
        case SDIO_PORT_C:
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
    return;
}
static void sdio_pwr_off(unsigned port)
{
    /// @todo NOT FINISH
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            setbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
            break;
        case SDIO_PORT_C:
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
    return;
}

// #define CONFIG_TSD      1
static void board_mmc_register(unsigned port)
{
    struct aml_card_sd_info *aml_priv=cpu_sdio_get(port);

    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    if(aml_priv==NULL||mmc==NULL)
        return;
    memset(mmc,0,sizeof(*mmc));
    aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;

// #ifdef CONFIG_TSD
    // // if(mmc->block_dev.dev > 0)//tsd
          // mmc->block_dev.if_type = IF_TYPE_SD;
// #else
    // // if(mmc->block_dev.dev > 0)//emmc
          // mmc->block_dev.if_type = IF_TYPE_MMC;
// #endif

	sdio_register(mmc, aml_priv);

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
//board_mmc_register(SDIO_PORT_A);
	board_mmc_register(SDIO_PORT_B);
	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif


#ifdef CONFIG_USB_DWC_OTG_HCD
#include <asm/arch/usb.h>
#include <asm/arch/gpio.h>
static int usb_charging_detect_call_back(char bc_mode)
{
#ifdef CONFIG_PLATFORM_HAS_PMU
    struct aml_pmu_driver *driver = aml_pmu_get_driver();
#endif
	switch(bc_mode){
		case BC_MODE_DCP:
		case BC_MODE_CDP:
			//Pull up chargging current > 500mA
        #ifdef CONFIG_PLATFORM_HAS_PMU
            if (driver && driver->pmu_set_usb_current_limit) {
                driver->pmu_set_usb_current_limit(0);                  // do not limit usb current
            }
        #endif
			break;

		case BC_MODE_UNKNOWN:
		case BC_MODE_SDP:
		default:
			//Limit chargging current <= 500mA
			//Or detet dec-charger
        #ifdef CONFIG_PLATFORM_HAS_PMU
            if (driver && driver->pmu_set_usb_current_limit) {
                driver->pmu_set_usb_current_limit(500);
            }
        #endif
			break;
	}
	return 0;
}
//note: try with some M3 pll but only following can work
//USB_PHY_CLOCK_SEL_M3_XTAL @ 1 (24MHz)
//USB_PHY_CLOCK_SEL_M3_XTAL_DIV2 @ 0 (12MHz)
//USB_PHY_CLOCK_SEL_M3_DDR_PLL @ 27(336MHz); @Rev2663 M3 SKT board DDR is 336MHz
//                                                            43 (528MHz); M3 SKT board DDR not stable for 528MHz
struct amlogic_usb_config g_usb_config_m6_skt_a={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M8_USBPORT_BASE_A,
	USB_ID_MODE_SW_HOST,
	NULL,//gpio_set_vbus_power, //set_vbus_power
	NULL,
};
struct amlogic_usb_config g_usb_config_m6_skt_b={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M8_USBPORT_BASE_B,
	USB_ID_MODE_SW_HOST,
	NULL,//gpio_set_vbus_power, //set_vbus_power
	NULL,
};
struct amlogic_usb_config g_usb_config_m6_skt_h={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M8_USBPORT_BASE_A,
	USB_ID_MODE_HARDWARE,
	NULL,//gpio_set_vbus_power, //set_vbus_power
	usb_charging_detect_call_back,
};
#endif /*CONFIG_USB_DWC_OTG_HCD*/
#ifdef CONFIG_IR_REMOTE
//#define DEBUG_IR

#define msleep(a) udelay(a * 1000)

#define IR_POWER_KEY    0xe51afb04
#define IR_MENU_KEY		0xac53fb04
#define IR_POWER_KEY_MASK 0xffffffff

typedef struct reg_remote
{
        int reg;
        unsigned int val;
}reg_remote;

typedef enum
{
        DECODEMODE_NEC = 0,
        DECODEMODE_DUOKAN = 1,
        DECODEMODE_RCMM ,
        DECODEMODE_SONYSIRC,
        DECODEMODE_SKIPLEADER ,
        DECODEMODE_MITSUBISHI,
        DECODEMODE_THOMSON,
        DECODEMODE_TOSHIBA,
        DECODEMODE_RC5,
        DECODEMODE_RC6,
        DECODEMODE_COMCAST,
        DECODEMODE_SANYO,
        DECODEMODE_MAX
}ddmode_t;
#define CONFIG_END 0xffffffff
/*
  bit0 = 1120/31.25 = 36
  bit1 = 2240 /31.25 = 72
  2500 /31.25  = 80
  ldr_idle = 4500  /31.25 =144
  ldr active = 9000 /31.25 = 288
*/
#ifndef CONFIG_NON_32K
static const reg_remote RDECODEMODE_NEC[] ={
        {P_AO_MF_IR_DEC_LDR_ACTIVE,477<<16 |400<<0},
        {P_AO_MF_IR_DEC_LDR_IDLE, 248<<16 | 202<<0},
        {P_AO_MF_IR_DEC_LDR_REPEAT,130<<16 |110<<0},
        {P_AO_MF_IR_DEC_BIT_0,60<<16|48<<0 },
        {P_AO_MF_IR_DEC_REG0,3<<28|(0xFA0<<12)|0x13},
        {P_AO_MF_IR_DEC_STATUS,(111<<20)|(100<<10)},
        {P_AO_MF_IR_DEC_REG1,0x9f40},
        {P_AO_MF_IR_DEC_REG2,0x0},
        {P_AO_MF_IR_DEC_DURATN2,0},
        {P_AO_MF_IR_DEC_DURATN3,0},
        {CONFIG_END,            0 }
};
static const reg_remote RDECODEMODE_DUOKAN[] =
{
	{P_AO_MF_IR_DEC_LDR_ACTIVE,477<<16 | 400<<0}, // NEC leader 9500us,max 477: (477* timebase = 31.25) = 9540 ;min 400 = 8000us
	{P_AO_MF_IR_DEC_LDR_IDLE, 248<<16 | 202<<0}, // leader idle
	{P_AO_MF_IR_DEC_LDR_REPEAT,130<<16|110<<0},  // leader repeat
	{P_AO_MF_IR_DEC_BIT_0,60<<16|48<<0 }, // logic '0' or '00'
	{P_AO_MF_IR_DEC_REG0,3<<28|(0xFA0<<12)|0x13},  // sys clock boby time.base time = 20 body frame 108ms
	{P_AO_MF_IR_DEC_STATUS,(111<<20)|(100<<10)},  // logic '1' or '01'
	{P_AO_MF_IR_DEC_REG1,0x9f40}, // boby long decode (8-13)
	{P_AO_MF_IR_DEC_REG2,0x0},  // hard decode mode
	{P_AO_MF_IR_DEC_DURATN2,0},
	{P_AO_MF_IR_DEC_DURATN3,0},
	{CONFIG_END,            0      }
};
#else
static const reg_remote RDECODEMODE_NEC[] ={
        {P_AO_MF_IR_DEC_LDR_ACTIVE,477<<16 |400<<0},
        {P_AO_MF_IR_DEC_LDR_IDLE, 248<<16 | 202<<0},
        {P_AO_MF_IR_DEC_LDR_REPEAT,130<<16 |110<<0},
        {P_AO_MF_IR_DEC_BIT_0,60<<16|48<<0 },
        {P_AO_MF_IR_DEC_REG0,3<<28|(0xFA0<<12)|0x13},
        {P_AO_MF_IR_DEC_STATUS,(111<<20)|(100<<10)},
        {P_AO_MF_IR_DEC_REG1,0x9f40},
        {P_AO_MF_IR_DEC_REG2,0x0},
        {P_AO_MF_IR_DEC_DURATN2,0},
        {P_AO_MF_IR_DEC_DURATN3,0},
        {CONFIG_END,            0 }
};
static const reg_remote RDECODEMODE_DUOKAN[] =
{
	{P_AO_MF_IR_DEC_LDR_ACTIVE,53<<16 | 50<<0}, // NEC leader 9500us,max 477: (477* timebase = 31.25) = 9540 ;min 400 = 8000us
	{P_AO_MF_IR_DEC_LDR_IDLE, 31<<16 | 25<<0}, // leader idle
	{P_AO_MF_IR_DEC_LDR_REPEAT,30<<16|26<<0},  // leader repeat
	{P_AO_MF_IR_DEC_BIT_0,61<<16|55<<0 }, // logic '0' or '00'
	{P_AO_MF_IR_DEC_REG0,3<<28|(0x5DC<<12)|0x13},  // sys clock boby time.base time = 20 body frame 108ms
	{P_AO_MF_IR_DEC_STATUS,(76<<20)|(69<<10)},  // logic '1' or '01'
	{P_AO_MF_IR_DEC_REG1,0x9300}, // boby long decode (8-13)
	{P_AO_MF_IR_DEC_REG2,0x10b},  // hard decode mode
	{P_AO_MF_IR_DEC_DURATN2,91<<16| 79<<0},
	{P_AO_MF_IR_DEC_DURATN3,111<<16 | 99<<0},
	{CONFIG_END,            0      }
};
#endif
static const reg_remote *remoteregsTab[] =
{
	RDECODEMODE_NEC,
	RDECODEMODE_DUOKAN,
};
void setremotereg(const reg_remote *r)
{
	writel(r->val, r->reg);
#ifdef DEBUG_IR
	printf(">>>write[0x%x] = 0x%x\n",r->reg, r->val);
	msleep(50);
	printf("    read<<<<<< = 0x%x\n",readl(r->reg));
#endif
}
int set_remote_mode(int mode)
{
	const reg_remote *reg;
	reg = remoteregsTab[mode];

	while (CONFIG_END != reg->reg)
		setremotereg(reg++);

	return 0;
}

void board_ir_init(void)
{

	unsigned int status,data_value;
	int val = readl(P_AO_RTI_PIN_MUX_REG);
	writel((val  | (1<<0)), P_AO_RTI_PIN_MUX_REG);
	set_remote_mode(DECODEMODE_NEC);
	status = readl(P_AO_MF_IR_DEC_STATUS);
	data_value = readl(P_AO_MF_IR_DEC_FRAME);

	printf("IR init done!\n");

}
#endif
#ifdef CONFIG_AML_I2C
/*I2C module is board depend*/
static void board_i2c_set_pinmux(void){
	/*@AML9726-MX-MAINBOARD_V1.0.pdf*/
	/*@AL5631Q+3G_AUDIO_V1.pdf*/
    /*********************************************/
    /*                | I2C_Master_AO        |I2C_Slave            |       */
    /*********************************************/
    /*                | I2C_SCK                | I2C_SCK_SLAVE  |      */
    /* GPIOAO_4  | [AO_PIN_MUX: 6]     | [AO_PIN_MUX: 2]   |     */
    /*********************************************/
    /*                | I2C_SDA                 | I2C_SDA_SLAVE  |     */
    /* GPIOAO_5  | [AO_PIN_MUX: 5]     | [AO_PIN_MUX: 1]   |     */
    /*********************************************/

	//disable all other pins which share with I2C_SDA_AO & I2C_SCK_AO
    clrbits_le32(P_AO_RTI_PIN_MUX_REG, ((1<<2)|(1<<24)|(1<<1)|(1<<23)));
    //enable I2C MASTER AO pins
	setbits_le32(P_AO_RTI_PIN_MUX_REG,
	(MESON_I2C_MASTER_AO_GPIOAO_4_BIT | MESON_I2C_MASTER_AO_GPIOAO_5_BIT));

    udelay(10);

};

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
#endif

#ifdef CONFIG_PLATFORM_HAS_PMU
static void board_pmu_init(void)
{
    struct aml_pmu_driver *driver = aml_pmu_get_driver();
    if (driver && driver->pmu_init) {
        driver->pmu_init();
    }
}
#endif

inline void key_init(void)
{
    setbits_le32(P_AO_GPIO_O_EN_N, (1 << 3));                           // GPIOAO_3 as power key input
    clrbits_le32(P_AO_RTI_PIN_MUX_REG, (1 << 7) | (1 << 9) | (1 << 22));                       // clear pinmux as gpio function
    //setbits_le32(P_AO_RTI_PULL_UP_REG, ((1 << 3) | (1 << 19)));         // enable pull up/down of gpio3
    clrbits_le32(P_AO_RTI_PULL_UP_REG, (1 << 19));         // enable pull up/down of gpio3
}

inline int get_key(void)
{
    return (readl(P_AO_GPIO_I) & (1 << 3)) ? 1 : 0;
}

static void board_i2c_init(void)
{
	//set I2C pinmux with PCB board layout
	/*@AML9726-MX-MAINBOARD_V1.0.pdf*/
	/*@AL5631Q+3G_AUDIO_V1.pdf*/
	board_i2c_set_pinmux();

	//Amlogic I2C controller initialized
	//note: it must be call before any I2C operation
	aml_i2c_init();

	//must call aml_i2c_init(); before any I2C operation
	/*M6 board*/
	//udelay(10000);

	udelay(10);
#ifdef CONFIG_PLATFORM_HAS_PMU
    board_pmu_init();
#endif
}

#ifdef CONFIG_NET_WIFI
void wifi_power_init(void)
{
	printf("mcli -- wifi_power_init in uboot --\n");
	clrbits_le32(P_AO_GPIO_O_EN_N,1<<22);
	clrbits_le32(P_AO_RTI_PIN_MUX_REG,1<<18);
	clrbits_le32(P_AO_GPIO_O_EN_N,1<<6);
}
#endif

int board_init(void)
{
#ifdef CONFIG_UART_A_FUNCTION_ADD
	unsigned a_uart = (159375000/(115200*4) -1)
        | UART_STP_BIT 
        | UART_PRTY_BIT
        | UART_CHAR_LEN 
        | UART_CNTL_MASK_TX_EN
        | UART_CNTL_MASK_RX_EN
        | UART_CNTL_MASK_RST_TX
        | UART_CNTL_MASK_RST_RX
        | UART_CNTL_MASK_CLR_ERR ;
  serial_init_uart_a(a_uart);
#endif

#if KSZ8091
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_EN_N, 1 << 31);
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 31);
#endif
	gd->bd->bi_arch_number=MACH_TYPE_MESON6_SKT;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;
#if CONFIG_JERRY_NAND_TEST //temp test
    nand_init();

#endif

    // LED
    clrbits_le32(P_AO_GPIO_O_EN_N, (1 << 15));
    clrbits_le32(P_AO_GPIO_O_EN_N, (1 << 31));

#ifdef CONFIG_AML_I2C
	board_i2c_init();
#endif /*CONFIG_AML_I2C*/
#ifdef CONFIG_IR_REMOTE
	board_ir_init();
#endif
#ifdef CONFIG_USB_DWC_OTG_HCD
	board_usb_init(&g_usb_config_m6_skt_b,BOARD_USB_MODE_HOST);
	board_usb_init(&g_usb_config_m6_skt_h,BOARD_USB_MODE_CHARGER);
#endif /*CONFIG_USB_DWC_OTG_HCD*/

#ifdef CONFIG_NET_WIFI
	wifi_power_init();
#endif
        key_init();
	return 0;
}

#ifdef CONFIG_NAND_AML_M3 //temp test
//#include <amlogic/nand/platform.h>
#include <asm/arch/nand.h>
#include <linux/mtd/partitions.h>


static struct aml_nand_platform aml_nand_mid_platform[] = {
#if defined CONFIG_SPI_NAND_COMPATIBLE || defined CONFIG_SPI_NAND_EMMC_COMPATIBLE
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE),
            },
        },
        .rbpin_mode=1,
        .short_pgsz=384,
        .ran_mode=0,
        .T_REA = 20,
        .T_RHOH = 15,
    },
#endif
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0) | (AML_NAND_CE1 << 4),// | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
        .ready_busy_pad = (AML_NAND_CE0) | (AML_NAND_CE1 << 4),// | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 2,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE | NAND_TWO_PLANE_MODE),
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
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};
#endif

static int do_msr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc > 2)
		goto usage;

	int nIndex = 0;
	int nCounter = 64;

	if( 2 == argc)
	{
		cmd = argv[1];
		char *endp;
		nIndex = simple_strtoul(argv[1], &endp, 10);
		if(nIndex < 0 || nIndex > 63)
			goto usage;
		nCounter = 1;
	}

	extern unsigned long    clk_util_clk_msr(unsigned long clk_mux);

	//printf("\n");
	for(;((nIndex < 64) && nCounter);nCounter--,nIndex++)
		printf("MSR clock[%d] = %dMHz\n",nIndex,(int)clk_util_clk_msr(nIndex));

	return 0;

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	msr,	2, 	1,	do_msr,
	"Meson msr sub-system",
	" [0...63] - measure clock frequency\n"
	"          - no clock index will measure all clock"
);

static int select_m8_dtd(unsigned int pID1)
{
	unsigned int ddr_size;
	int i;
	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		ddr_size += gd->bd->bi_dram[i].size;
	}
	if(0x80000000==ddr_size){/*2G ddr*/
		switch(pID1){
			case 0x25e2:   //chip version A
				//printf("chip version A, emmc use sdio controller\n");
				setenv("aml_dt", "m8_k200_2gsdio");
				break;
			case 0x27ed:   //chip version B
				//printf("chip version B, emmc use sdhc controller\n");
				setenv("aml_dt", "m8_k200_2gsdhc");
				break;
			default:
				printf("bad chip version!!!");
				return 1;
		}
	}
	else if(0x40000000==ddr_size){/*1G ddr*/
		switch(pID1){
			case 0x25e2:   //chip version A
				//printf("chip version A, emmc use sdio controller\n");
				setenv("aml_dt", "m8_k200_sdio");
				break;
			case 0x27ed:   //chip version B
				//printf("chip version B, emmc use sdhc controller\n");
				setenv("aml_dt", "m8_k200_sdhc");
				break;
			default:
				printf("bad chip version!!!");
				return 1;
		}
	}
	else{/*others doesn't support fo now*/
		printf("DDR size: 0x%x, multi-dt doesn't support\n", ddr_size);
		return -1;
	}
	return 0;
}

static int select_m8m2_dtd(unsigned int pID1)
{
	unsigned int ddr_size;
	int i;
	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		ddr_size += gd->bd->bi_dram[i].size;
	}
	if(0x80000000==ddr_size){/*2G ddr*/
		setenv("aml_dt", "m8m2_n200_2G");
	}
	else if(0x40000000==ddr_size){/*1G ddr*/
		setenv("aml_dt", "m8m2_n200_1G");
	}
	else{/*others doesn't support fo now*/
		printf("DDR size: 0x%x, multi-dt doesn't support\n", ddr_size);
		return -1;
	}
	return 0;
}
static int do_checkhw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifndef CONFIG_MESON_TRUSTZONE
       unsigned int *pID1 =(unsigned int *)0xd9040004;
       //unsigned int *pID2 =(unsigned int *)0xd904002c;
#else
       extern uint32_t meson_trustzone_read_socrev1(void);
       unsigned int ID1 = meson_trustzone_read_socrev1();
       //unsigned int ID2 = meson_trustzone_read_socrev2();
       unsigned int *pID1 = &ID1;
       //unsigned int *pID2 = &ID2;
#endif
	int ret;
	if(IS_MESON_M8M2_CPU)
		ret=select_m8m2_dtd(*pID1);
	else
		ret=select_m8_dtd(*pID1);

  return ret;
}

U_BOOT_CMD(
        checkhw, 1, 1, do_checkhw,
        "Get the hardware revsion","[<string>]\n"
);
