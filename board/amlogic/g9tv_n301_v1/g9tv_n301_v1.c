#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <malloc.h>

#if defined(CONFIG_CMD_NET)
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/io.h>
#endif /*(CONFIG_CMD_NET)*/

#if defined(CONFIG_AML_I2C)
#include <aml_i2c.h>
#include <asm/arch/io.h>
#endif /*CONFIG_AML_I2C*/

#ifdef CONFIG_AUTO_UPDATE_ENV
#include <version.h>
#include <timestamp.h>
#endif /*CONFIG_AUTO_UPDATE_ENV*/

#include <asm/arch/reboot.h>
#include "build_version.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET)
/*************************************************
  * Amlogic Ethernet controller operation
  * 
  * Note: RTL8211F gbit_phy use RGMII interface
  *
  *************************************************/
static void setup_net_chip(void) 
{
  	eth_aml_reg0_t eth_reg0;
	/*m8b mac clock use externel phy clock(125m/25m/2.5m)
	 setup ethernet clk need calibrate to configre
	 setup ethernet pinmux use DIF_TTL_0N/P 1N/P 2N/P 3N/P 4N/P GPIOH(3-9) */
#ifdef RMII_PHY_INTERFACE
	/* setup ethernet pinmux use gpioz(5-14) */
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6,0xff7f);
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_7,0xf00000);
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
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0xffff);
	eth_reg0.d32 = 0;
	eth_reg0.b.phy_intf_sel = 1;
	eth_reg0.b.data_endian = 0;
	eth_reg0.b.desc_endian = 1;
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
WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, eth_reg0.d32);// rgmii mode
#endif
	/* setup ethernet mode */
	CLEAR_CBUS_REG_MASK(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
	/* hardware reset ethernet phy : gpioh_4 connect phyreset pin*/
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO3_EN_N, 1 << 23);
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO3_O, 1 << 23);
	udelay(2000);
	SET_CBUS_REG_MASK(PREG_PAD_GPIO3_O, 1 << 23);
}

int board_eth_init(bd_t *bis)
{   	
    setup_net_chip();
    udelay(1000);
	extern int aml_eth_init(bd_t *bis);
    aml_eth_init(bis);
	return 0;
}
#endif /* (CONFIG_CMD_NET) */

#ifdef CONFIG_AUTO_UPDATE_ENV
void auto_update_env(void)
{
	char *last_ver = getenv("ubootversion");
	char *cur_ver = U_BOOT_VERSION "("U_BOOT_DATE "-" U_BOOT_TIME")";
	//printf("last = %s \n",last_ver);
	//printf("cur = %s \n",cur_ver);
	if(strcmp(cur_ver, last_ver) != 0 ) {
		run_command("defenv", 0);
		saveenv();
	}
}
#endif

#ifdef CONFIG_SWITCH_BOOT_MODE
int switch_boot_mode_power(void)
{
    u32 reboot_mode_current = reboot_mode;
	//unsigned int suspend_status_current = readl(P_AO_RTI_STATUS_REG0);	
	unsigned int suspend_status_current2 = readl(P_AO_RTI_STATUS_REG2);

	char *suspend = getenv("suspend");
	printf("STATUS_REG1=%x STATUS_REG2=%x  suspend=%s  \n",reboot_mode_current,suspend_status_current2,suspend);

    switch(reboot_mode_current) {
        case AMLOGIC_NORMAL_BOOT: {
            printf("bootload normal boot mode\n");
            return 0;
        }
        break;
        case AMLOGIC_UPDATE_REBOOT: {
            printf("bootload update mode\n");
            run_command("run update",0);
            return 0;
        }
        break;
        case AMLOGIC_FACTORY_RESET_REBOOT: {
            printf("bootload factory reset mode\n");
            run_command("run recovery",0);
            return 0;
        }
        break;
        case MESON_USB_BURNER_REBOOT: {
            printf("bootload burnimg mode\n");
            run_command("burnimg",0);
            //burn_img_package();
            reboot_mode = 0;
            run_command("reset",0);
            return 0;
        }
        break;
        case AMLOGIC_CHARGING_REBOOT:
        break;
        case AMLOGIC_LOCK_REBOOT:
        break;
    }

#ifdef CONFIG_POWER_MODE	
	char *powermode = getenv("powermode");// standby,on,last,;	
	if(!strcmp(powermode, "standby")){		 //--------standby------
		char *pstandby = getenv("pstandby");
		if(!strcmp(pstandby, "on")){
			if( PWRKEY_WAKEUP_FLAGE == suspend_status_current2 ){
				writel(0,P_AO_RTI_STATUS_REG2);
				setenv("pstandby","off");			
				saveenv();
				return 0; //boot linux
			}else{	
				run_command("suspend", 0);
			}	
		}else{
			setenv("pstandby","on");		
			saveenv();
			run_command("suspend", 0);
		}
	}else if(!strcmp(powermode, "last")){	//---------last------	
		if(PWRKEY_WAKEUP_FLAGE == suspend_status_current2){		
			writel(0,P_AO_RTI_STATUS_REG2);
			setenv("suspend","off");//reboot			
			saveenv();			
			return 0;		
		}
		if(!strcmp(suspend, "off")){
			return 0;
		}else if(!strcmp(suspend,"on")){
			run_command("suspend",0);
		}
		return 0;
	}else if(!strcmp(powermode, "on")){ 	//---------on------
		if(PWRKEY_WAKEUP_FLAGE == suspend_status_current2){		
			writel(0,P_AO_RTI_STATUS_REG2);
			setenv("suspend","off");//power up		
			saveenv();	
			return 0;		
		}
		if(!strcmp(suspend,"off")) {
			return 0;  //boot linux
		}
	    if(!strcmp(suspend,"on")) {
			setenv("suspend","off");		
			saveenv();	
       		run_command("suspend",0);  // press power , reboot
        }
		return 0;
	}
#endif

return 0;
}

int switch_boot_mode(void)
{
	printf("######### switch_boot_mode ##########\n");
#ifdef CONFIG_AUTO_UPDATE_ENV
	auto_update_env();
#endif
	switch_boot_mode_power();
	return 0;
}

#endif


u32 get_board_rev(void)
{
 
	return 0x20;
}

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
            setbits_le32(P_PREG_PAD_GPIO2_EN_N,1<<26);//CARD_6
           ret=readl(P_PREG_PAD_GPIO2_I)&(1<<26)?0:1;
            	
			if((readl(P_PERIPHS_PIN_MUX_8)&(3<<9))){ //if uart pinmux set, debug board in
				if(!(readl(P_PREG_PAD_GPIO2_I)&(1<<24))){
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
#ifdef CONFIG_VLSI_EMULATOR
	board_mmc_register(SDIO_PORT_A);
#else
	board_mmc_register(SDIO_PORT_B);
#endif
	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif

#ifdef CONFIG_USB_XHCI_AMLOGIC
#include <asm/arch/usb.h>
#include <asm/arch/gpio.h>
struct amlogic_usb_config g_usb_config_g9TV_skt={
	CONFIG_G9TV_XHCI_BASE,
	USB_ID_MODE_HARDWARE,
	NULL,//gpio_set_vbus_power, //set_vbus_power
	CONFIG_G9TV_USB_PHY2_BASE,
	CONFIG_G9TV_USB_PHY3_BASE,
};
#endif

#ifdef CONFIG_IR_REMOTE
void board_ir_init(void)
{
	writel(0x00005801,P_AO_RTI_PIN_MUX_REG);
	writel(0x30fa0013,P_AO_IR_DEC_REG0);
	writel(0x0ee8be40,P_AO_IR_DEC_REG1);
	writel(0x01d801ac,P_AO_IR_DEC_LDR_ACTIVE);
	writel(0x00f800ca,P_AO_IR_DEC_LDR_IDLE);
	writel(0x0044002c,P_AO_IR_DEC_BIT_0);
	printf("IR init done!\n");

}
#endif
int board_init(void)
{
	gd->bd->bi_arch_number=MACH_TYPE_MESON6_SKT;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

#ifdef CONFIG_UBOOT_BUILD_VERSION_INFO
    print_build_version_info();
#endif

#if CONFIG_JERRY_NAND_TEST //temp test	
    nand_init();
    
#endif    
    
#ifdef CONFIG_AML_I2C  
	board_i2c_init();
#endif /*CONFIG_AML_I2C*/
#ifdef CONFIG_IR_REMOTE
	board_ir_init();
#endif

#ifdef CONFIG_USB_XHCI_AMLOGIC
	board_usb_init(&g_usb_config_g9TV_skt,BOARD_USB_MODE_HOST);
#endif /*CONFIG_USB_XHCI_AMLOGIC*/

#if defined(CONFIG_VLSI_EMULATOR)
		   run_command("video dev open 1080p", 0);
#endif

#ifdef CONFIG_PWM_E_OUT_32K
    printf("init pwm_e out 32k clock.\n");
    writel(readl(P_PERIPHS_PIN_MUX_9) | (0x1 << 19), P_PERIPHS_PIN_MUX_9); //set mode GPIOX_10-->CLK_OUT3
    writel(0x16d016d, P_PWM_PWM_E);
    writel((readl(P_PWM_MISC_REG_EF) & ~(0xFF << 8)) | 0x8001, P_PWM_MISC_REG_EF);
#endif

    //default power on 24M
    writel(readl(P_PERIPHS_PIN_MUX_10)&~(1 << 11),P_PERIPHS_PIN_MUX_10);
    writel(readl(P_AO_GPIO_O_EN_N)&~(1 << 18),P_AO_GPIO_O_EN_N);
    writel(readl(P_AO_GPIO_O_EN_N)&~(1 << 2),P_AO_GPIO_O_EN_N);

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

#ifdef CONFIG_SARADC
#include <asm/saradc.h>
/*following key value are test with board 
  [M6_SKT_V_1.0 20120112]
  ref doc:
  1. M6_SKT_V1.pdf
*/
/* adc_init(&g_adc_info, ARRAY_SIZE(g_adc_info)); */
/*following is test code to test ADC & key pad*/
static int do_adc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if(argc > 2)
		goto usage;
	
	u32 nDelay = 0xffff;
	int nKeyVal = 0;
	int nCnt = 0;
	char *endp;
	int nMaxCnt;
	int adc_chan = 0; //m8 adc channel 0;m6 adc channel 4
	if(2 == argc)
		nMaxCnt	= simple_strtoul(argv[1], &endp, 10);
	else
		nMaxCnt = 10;

	saradc_enable();
	while(nCnt < nMaxCnt)
	{
		udelay(nDelay);
		nKeyVal = get_adc_sample(adc_chan);
		if(nKeyVal > 1021)
			continue;
		
		printf("SARADC CH-4 Get key : %d [%d]\n", nKeyVal,(100*nKeyVal)/1024);
		nCnt++;
	}
	saradc_disable();

	return 0;
	
usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	adc,	2,	1,	do_adc,
	"M6 ADC test",		
	"[times] -  read `times' adc key through channel-4, default to read 10 times\n"
	"		10bit ADC. key value: min=0; max=1024\n"
	"		SKT BOARD #20: Key1=13 Key2=149 key3=274 key4=393 key5=514\n"
);

#endif //CONFIG_SARADC

