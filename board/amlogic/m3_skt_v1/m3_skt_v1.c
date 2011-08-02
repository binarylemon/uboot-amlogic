#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>

#if defined(CONFIG_CMD_NET)
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/io.h>
#endif /*(CONFIG_CMD_NET)*/

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_arch_number=MACH_TYPE_MESON_8626M;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;
	return 0;
}

#if defined(CONFIG_CMD_NET)

/*************************************************
  * Amlogic Ethernet controller operation
  * 
  * Note: The LAN chip LAN8720 need to be reset by GPIOA_23
  *
  *************************************************/
static void setup_net_chip(void)
{
	//disable all other pins which share the GPIOA_23
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0,(1<<6)); //LCDin_B7 R0[6]
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7,(1<<11));//ENC_11 R7[11]
	//GPIOA_23 -> 0
    CLEAR_CBUS_REG_MASK(PREG_EGPIO_O,1<<23);    //RST -> 0
    //GPIOA_23 output enable
    CLEAR_CBUS_REG_MASK(PREG_EGPIO_EN_N,1<<23); //OUTPUT enable	
    udelay(2000);
	//GPIOA_23 -> 1
    SET_CBUS_REG_MASK(PREG_EGPIO_O,1<<23);      //RST -> 1
    udelay(2000);	
}

int board_eth_init(bd_t *bis)
{   	
	//set clock
    eth_clk_set(ETH_CLKSRC_MISC_PLL_CLK,800*CLK_1M,50*CLK_1M);	

	//set pinmux
    aml_eth_set_pinmux(ETH_BANK0_GPIOY1_Y9,ETH_CLK_OUT_GPIOY0_REG6_17,0);

	//ethernet pll control
    writel(readl(ETH_PLL_CNTL) & ~(0xF << 0), ETH_PLL_CNTL); // Disable the Ethernet clocks        
    writel(readl(ETH_PLL_CNTL) | (0 << 3), ETH_PLL_CNTL);    // desc endianess "same order"   
    writel(readl(ETH_PLL_CNTL) | (0 << 2), ETH_PLL_CNTL);    // data endianess "little"    
    writel(readl(ETH_PLL_CNTL) | (1 << 1), ETH_PLL_CNTL);    // divide by 2 for 100M     
    writel(readl(ETH_PLL_CNTL) | (1 << 0), ETH_PLL_CNTL);    // enable Ethernet clocks   
    
    udelay(1000);

	//reset LAN8720 with GPIOA_23
    setup_net_chip();

    udelay(1000);
	
extern int aml_eth_init(bd_t *bis);

    aml_eth_init(bis);

	return 0;
}
#endif /* (CONFIG_CMD_NET) */

#if CONFIG_CMD_MMC
#include <mmc.h>
#include <asm/arch/sdio.h>
static int  sdio_init(unsigned port)
{
	//setbits_le32(P_PREG_CGPIO_EN_N,1<<5);
	setbits_le32(P_PREG_GGPIO_EN_N,1<<11);//GPIOD13

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
	//return (readl(P_PREG_CGPIO_I)&(1<<5))?1:0;
	return (readl(P_PREG_GGPIO_I)&(1<<11))?1:0;//GPIOD13
}
static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
}
static void sdio_pwr_on(unsigned port)
{
//	clrbits_le32(P_PREG_CGPIO_O,(1<<5));
//	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
	clrbits_le32(P_PREG_GGPIO_O,(1<<11));
	clrbits_le32(P_PREG_GGPIO_EN_N,(1<<11));//GPIOD13
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
//	setbits_le32(P_PREG_CGPIO_O,(1<<5));
//	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
	setbits_le32(P_PREG_GGPIO_O,(1<<11));
	clrbits_le32(P_PREG_GGPIO_EN_N,(1<<11));//GPIOD13

	/// @todo NOT FINISH
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
	board_mmc_register(SDIO_PORT_A);
//	board_mmc_register(SDIO_PORT_B);
//	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif