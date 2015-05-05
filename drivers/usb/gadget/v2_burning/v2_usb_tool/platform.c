/* platform dirver header */
/*
 * (C) Copyright 2010 Amlogic, Inc
 *
 * Victor Wan, victor.wan@amlogic.com, 
 * 2010-03-24 @ Shanghai
 *
 */
 #include "platform.h"
#include <asm/arch/power_gate.h>

/*CONFIG_AML_MESON_8 include m8, m8baby, m8m2, etc... defined in cpu.h*/
#if !( defined(CONFIG_AML_MESON_6) || defined(CONFIG_AML_MESON_8) || defined (CONFIG_AML_G9TV))
#error "platform is not m6 or m8!!"
#endif//#if 

#if (MESON_CPU_TYPE == MESON_CPU_TYPE_G9TV)
#define PREI_USB_PHY_2_REG_BASE 0xc8022020
#define PREI_USB_PHY_3_REG_BASE 0xc8022080

typedef struct u2p_aml_regs {
	volatile uint32_t u2p_r0; 
	volatile uint32_t u2p_r1;      
	volatile uint32_t u2p_r2; 
} u2p_aml_regs_t;

typedef union u2p_r0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned bypass_sel:1;   // 0
		unsigned bypass_dm_en:1; // 1 
		unsigned bypass_dp_en:1; // 2
		unsigned txbitstuffenh:1;// 3
  		unsigned txbitstuffen:1; // 4
		unsigned dmpulldown:1;   // 5
		unsigned dppulldown:1;   // 6
		unsigned vbusvldextsel:1;// 7
		unsigned vbusvldext:1;   // 8
		unsigned adp_prb_en:1;   // 9
		unsigned adp_dischrg:1;  // 10
		unsigned adp_chrg:1;     // 11
		unsigned drvvbus:1;      // 12
		unsigned idpullup:1;     // 13
		unsigned loopbackenb:1;  // 14
		unsigned otgdisable:1;   // 15
		unsigned commononn:1;    // 16
 		unsigned fsel:3;         // 17
		unsigned refclksel:2;    // 20
		unsigned por:1;          // 22
		unsigned vatestenb:2;    // 23
		unsigned set_iddq:1;     // 25
		unsigned ate_reset:1;    // 26
		unsigned fsv_minus:1;    // 27
		unsigned fsv_plus:1;     // 28
		unsigned bypass_dm_data:1; // 29 
		unsigned bypass_dp_data:1; // 30
        	unsigned not_used:1;
	} b;
} u2p_r0_t;

typedef struct usb_aml_regs {
	volatile uint32_t usb_r0; 
	volatile uint32_t usb_r1;      
	volatile uint32_t usb_r2; 
	volatile uint32_t usb_r3; 
	volatile uint32_t usb_r4; 
	volatile uint32_t usb_r5; 
	volatile uint32_t usb_r6; 
} usb_aml_regs_t;

typedef union usb_r0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned p30_fsel:6; // 0
		unsigned p30_phy_reset:1; // 6
		unsigned p30_test_powerdown_hsp:1; // 7
		unsigned p30_test_powerdown_ssp:1; // 8
		unsigned p30_acjt_level:5;         // 9
		unsigned p30_tx_vboost_lvl:3;      // 14
  		unsigned p30_lane0_tx2rx_loopbk:1; // 17
		unsigned p30_lane0_ext_pclk_req:1; // 18
		unsigned p30_pcs_rx_los_mask_val:10; // 19
		unsigned u2d_ss_scaledown_mode:2;  // 29
		unsigned u2d_act:1; // 31
	} b;
} usb_r0_t;

typedef union usb_r4 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned p21_PORTRESET0:1; // 0
		unsigned p21_SLEEPM0:1; // 1
		unsigned mem_pd:2;
		unsigned reserved4:28; // 31        
	} b;
} usb_r4_t;

static void set_usb_phy_config(int cfg)
{
    const int time_dly = 500;
    u2p_aml_regs_t * u2p_aml_regs = (u2p_aml_regs_t * )PREI_USB_PHY_2_REG_BASE;
    usb_aml_regs_t * usb_aml_regs = (usb_aml_regs_t * )PREI_USB_PHY_3_REG_BASE;
    
    u2p_r0_t u2p_r0;
    usb_r0_t usb_r0;
    usb_r4_t usb_r4;

    if (!IS_CLK_GATE_ON(USB0)) {
            SET_CBUS_REG_MASK(GCLK_REG_USB0, GCLK_MASK_USB0);
    }
    /*printf("%s %d\n", __func__, __LINE__);*/
    cfg = cfg;//avoid compiler warning
    /**P_RESET1_REGISTER = (1<<2);//usb reset*/
	  writel((1 << 2),P_RESET1_REGISTER);	//usb reset
    udelay(time_dly);//by Sam: delay after reset

		u2p_r0.d32 = u2p_aml_regs->u2p_r0;
		u2p_r0.b.fsel = 5;
    u2p_r0.b.por = 1;
		u2p_aml_regs->u2p_r0 = u2p_r0.d32;

		u2p_r0.d32 = u2p_aml_regs->u2p_r0;
		u2p_r0.b.por = 0;
		u2p_aml_regs->u2p_r0 = u2p_r0.d32;		
		
  	usb_r0.d32 = usb_aml_regs->usb_r0;	
		usb_r0.b.u2d_act = 1;
		usb_aml_regs->usb_r0 = usb_r0.d32;	
		
    usb_r4.d32 = usb_aml_regs->usb_r4;
    usb_r4.b.p21_SLEEPM0 = 1;
    usb_aml_regs->usb_r4 = usb_r4.d32;	
    
    udelay(time_dly);	
    return;
}

void close_usb_phy_clock(int cfg)
{
    cfg = cfg;//avoid compiler warning

    dwc_otg_pullup(0);//disconnect
    __udelay(20);
    /*dwc_otg_power_off_phy();*///Don't call this as it may cause pull-down failed!!!!
    run_command("sleep 1", 0);//sleep sometime to improve pc compatibility!!

    return;
}

#else
/*
   cfg = 0 : EXT clock
   cfg = 1 : INT clock
  */

#if (MESON_CPU_TYPE == MESON_CPU_TYPE_G9BABY)
#define PREI_USB_PHY_A_REG_BASE       0xC8022000  //0x2100
#define PREI_USB_PHY_B_REG_BASE       0xC1108420	//0X2108
#else
#define PREI_USB_PHY_A_REG_BASE       P_USB_ADDR0
#define PREI_USB_PHY_B_REG_BASE       P_USB_ADDR1
#endif//#if 0

#ifdef __USE_PORT_B
#define PREI_USB_PHY_REG_BASE   PREI_USB_PHY_B_REG_BASE
#else
#define PREI_USB_PHY_REG_BASE   PREI_USB_PHY_A_REG_BASE
#endif
//#define P_RESET1_REGISTER                           (volatile unsigned long *)0xc1104408
#define USB_CLK_SEL_XTAL				0
#define USB_CLK_SEL_XTAL_DIV_2	1
#define USB_CLK_SEL_DDR_PLL			2
#define USB_CLK_SEL_MPLL_OUT0		3
#define USB_CLK_SEL_MPLL_OUT1		4
#define USB_CLK_SEL_MPLL_OUT2		5
#define USB_CLK_SEL_FCLK_DIV2		6
#define USB_CLK_SEL_FCLK_DIV3		7

typedef struct usb_aml_regs {
    volatile uint32_t config; 
    volatile uint32_t ctrl;      
    volatile uint32_t endp_intr; 
    volatile uint32_t adp_bc;    
    volatile uint32_t dbg_uart;  
    volatile uint32_t test;
    volatile uint32_t tune; 
} usb_aml_regs_t;

typedef union usb_config_data {
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct {
        unsigned clk_en     :1;
        unsigned clk_sel    :3;
        unsigned clk_div    :7;
        unsigned reserved0  :1;
        unsigned clk_32k_alt_sel:1;
        unsigned reserved1  :15;
        unsigned test_trig  :1;
    } b;
} usb_config_data_t;

typedef union usb_ctrl_data {
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct {
        unsigned soft_prst:1;
        unsigned soft_hreset:1;
        unsigned ss_scaledown_mode:2;
        unsigned clk_det_rst:1;
        unsigned intr_sel:1;
        unsigned reserved:2;
        unsigned clk_detected:1;
        unsigned sof_sent_rcvd_tgl:1; 
        unsigned sof_toggle_out:1; 
        unsigned not_used:4;
        unsigned por:1;
        unsigned sleepm:1;
        unsigned txbitstuffennh:1;
        unsigned txbitstuffenn:1;
        unsigned commononn:1; 
        unsigned refclksel:2; 
        unsigned fsel:3;
        unsigned portreset:1;
        unsigned thread_id:6;
    } b;
} usb_ctrl_data_t;

#if defined(CONFIG_AML_MESON_8) || (MESON_CPU_TYPE == MESON_CPU_TYPE_G9BABY)
static void set_usb_phy_config(int cfg)
{
    const int time_dly = 500;
    usb_aml_regs_t * usb_aml_regs = (usb_aml_regs_t * )PREI_USB_PHY_REG_BASE;
    usb_config_data_t config;
    usb_ctrl_data_t control;

    /*CLK_GATE_ON(USB0);*/
    if (!IS_CLK_GATE_ON(USB0)) {
            SET_CBUS_REG_MASK(GCLK_REG_USB0, GCLK_MASK_USB0);
    }
    /*printf("%s %d\n", __func__, __LINE__);*/
    cfg = cfg;//avoid compiler warning
    /**P_RESET1_REGISTER = (1<<2);//usb reset*/
	writel((1 << 2),P_RESET1_REGISTER);	//usb reset
    udelay(time_dly);//by Sam: delay after reset

  	config.d32 = usb_aml_regs->config;

//    config.b.clk_sel    = 0;
//    config.b.clk_div    = 1; 
//    config.b.clk_32k_alt_sel = 1;
  	usb_aml_regs->config = config.d32;

    control.d32 = usb_aml_regs->ctrl;
    control.b.fsel = 5;
    control.b.por = 1;
    usb_aml_regs->ctrl = control.d32;
    udelay(time_dly);

    control.b.por = 0;
    usb_aml_regs->ctrl = control.d32;
    udelay(time_dly);//by Sam: delay 0.5s to wait usb clam down

    control.d32 = usb_aml_regs->ctrl;
    if(!control.b.clk_detected){
        printf("Error, usb phy clock not detected!\n");
    }

    return;
}

void close_usb_phy_clock(int cfg)
{
    cfg = cfg;//avoid compiler warning

    dwc_otg_pullup(0);//disconnect
    __udelay(20);
    /*dwc_otg_power_off_phy();*///Don't call this as it may cause pull-down failed!!!!
    run_command("sleep 1", 0);//sleep sometime to improve pc compatibility!!

    return;
}

#else//MX

static void set_usb_phy_config(int cfg)
{

    const int time_dly = 500;
    usb_aml_regs_t * usb_aml_regs = (usb_aml_regs_t * )PREI_USB_PHY_REG_BASE;
    usb_config_data_t config;
    usb_ctrl_data_t control;

    /**P_RESET1_REGISTER = (1<<2);//usb reset*/
	writel((1 << 2),P_RESET1_REGISTER);	//usb reset
    udelay(time_dly);//by Sam: delay after reset

  	config.d32 = usb_aml_regs->config;

  	if(cfg == EXT_CLOCK)
  	{/* crystal == 24M */
  		config.b.clk_sel = USB_CLK_SEL_XTAL_DIV_2;	// 12M, Phy default setting is 12Mhz
  		config.b.clk_div = 0; // 24M /(0 + 1) = 12M
  	}
  	else
  	{
#ifndef M3_SIM
  		config.b.clk_sel = USB_CLK_SEL_DDR_PLL; // ddr_pll = 600M
#else
  		config.b.clk_sel = USB_CLK_SEL_MPLL_OUT0; // ddr_pll = 600M
#endif
  		config.b.clk_div = 49; // (600 / (49 + 1)) = 12M
  	}

    config.b.clk_en = 1;
		usb_aml_regs->config = config.d32;
		control.d32 = usb_aml_regs->ctrl;
		control.b.fsel = 2;
		control.b.por = 1;
		usb_aml_regs->ctrl = control.d32;
		udelay(time_dly);
		control.b.por = 0;
		usb_aml_regs->ctrl = control.d32;
		udelay(time_dly*100);//by Sam: delay 0.5s to wait usb clam down

}

void close_usb_phy_clock(int cfg)
{
    dwc_otg_pullup(0);//disconnect
    __udelay(20);

    //dwc_otg_power_off_phy();//close phy
    run_command("sleep 1", 0);//sleep sometime to improve pc compatibility!!
    return;
}

#endif//#if defined(CONFIG_M6) 
#endif //if define CONFIG G9TV
#if 0
int chip_watchdog(void)
{
	watchdog_clear();
	return 0;
};

void usb_memcpy(char * dst,char * src,int len)
{
	while(len--)
	{
		*(unsigned char*)dst = *(unsigned char*)src;
		(unsigned char*)dst++;
		(unsigned char*)src++;
	}
}
void usb_memcpy_32bits(int *dst,int *src,int len)
{
	while(len--)
	{
		*dst = *src;
		dst++;
		src++;
	}
}

#endif

