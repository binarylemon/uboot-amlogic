#ifndef __CONFIG_M8_K200_V1_H__
#define __CONFIG_M8_K200_V1_H__

//#define AML_UBOOT_LOG_PROFILE 1

#define CONFIG_AML_MESON_8      1
#define CONFIG_MACH_MESON8_K200_V1  // generate M8 K200 machid number

#define CONFIG_SECURITYKEY

//#define CONFIG_INSTABOOT
//#define CONFIG_MUTE_PRINT

#ifndef CONFIG_M8
#define CONFIG_M8
#endif // ifndef CONFIG_M8
//#define	CONFIG_VLSI_EMULATOR 1
//#define TEST_UBOOT_BOOT_SPEND_TIME

// cart type of each port
#define PORT_A_CARD_TYPE            CARD_TYPE_UNKNOWN
#define PORT_B_CARD_TYPE            CARD_TYPE_UNKNOWN
#define PORT_C_CARD_TYPE            CARD_TYPE_MMC // otherwise CARD_TYPE_SD

//UART Sectoion
#define CONFIG_CONS_INDEX   2

//UART A Section
//#define CONFIG_UART_A_FUNCTION_ADD


//#define CONFIG_SECURESTORAGEKEY
#ifdef CONFIG_SECURESTORAGEKEY
#ifndef CONFIG_RANDOM_GENERATE
#define CONFIG_RANDOM_GENERATE
#endif
#endif

//#define CONFIG_SECURITYKEY
#ifdef CONFIG_SECURITYKEY
#define CONFIG_AML_NAND_KEY
#endif

#define  CONFIG_AML_GATE_INIT	1
#define CONFIG_NEXT_NAND
//#define CONFIG_SECURE_NAND  1
//support "boot,bootd"
//#define CONFIG_CMD_BOOTD 1

//Enable HDMI Tx
//#define CONFIG_VIDEO_AMLTVOUT 1
//Enable LCD output
//#define CONFIG_VIDEO_AMLLCD
#define LCD_BPP LCD_COLOR16

#define CONFIG_ACS
#ifdef CONFIG_ACS
#define CONFIG_DDR_SIZE_IND_ADDR 0xD9000000	//pass memory size, spl->uboot
#endif

#ifdef CONFIG_NEXT_NAND
#define CONFIG_CMD_IMGREAD  1   //read the actual size of boot.img/recovery.img/logo.img use cmd 'imgread'
#define CONFIG_AML_V2_USBTOOL 1
#endif//#ifdef CONFIG_NEXT_NAND

#define CONFIG_CMD_CPU_TEMP
#if CONFIG_AML_V2_USBTOOL
#define CONFIG_SHA1
#define CONFIG_AUTO_START_SD_BURNING     1//1 then auto detect whether or not jump into sdc_burning when boot from external mmc card 
#define CONFIG_SD_BURNING_SUPPORT_LED    1//1 then using led flickering states changing to show burning states when sdcard burning
#define CONFIG_POWER_KEY_NOT_SUPPORTED_FOR_BURN 1//power key and poweroff can't work
#define CONFIG_SD_BURNING_SUPPORT_UI     1//have bmp display to indicate burning state when sdcard burning
#ifdef CONFIG_ACS
#define CONFIG_TPL_BOOT_ID_ADDR       		(0xD9000000U + 4)//pass boot_id, spl->uboot
#else
#define CONFIG_TPL_BOOT_ID_ADDR       		(&reboot_mode)//pass boot_id, spl->uboot
#endif// #ifdef CONFIG_ACS
#endif// #if CONFIG_AML_V2_USBTOOL

#define CONFIG_UNIFY_KEY_MANAGE 1
#define CONFIG_CMD_PWM  1
//#define CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2 1  //open this macro if need read encrypted kernel/dtb with whole part size

//Enable storage devices
#define CONFIG_CMD_NAND  1
#define CONFIG_VIDEO_AML 1
#define CONFIG_CMD_BMP 1
#define CONFIG_VIDEO_AMLTVOUT 1
#define CONFIG_AML_HDMI_TX  1
#define CONFIG_OSD_SCALE_ENABLE 1

#if defined(CONFIG_VIDEO_AMLTVOUT)
#define CONFIG_CVBS_PERFORMANCE_COMPATIBILITY_SUPPORT	1

#define CONFIG_CVBS_CHINASARFT		0x0
#define CONFIG_CVBS_CHINATELECOM	0x1
#define CONFIG_CVBS_CHINAMOBILE		0x2
#define CONFIG_CVBS_PERFORMANCE_ACTIVED	CONFIG_CVBS_CHINASARFT

#endif

//Enable storage devices
#define CONFIG_CMD_SF    1

#if defined(CONFIG_CMD_SF)
	#define SPI_WRITE_PROTECT  1
	#define CONFIG_CMD_MEMORY  1
#endif /*CONFIG_CMD_SF*/

//Amlogic SARADC support
#define CONFIG_SARADC 1
#define CONFIG_CMD_SARADC
#define CONFIG_EFUSE 1
//#define CONFIG_MACHID_CHECK 1
#ifndef CONFIG_BL31
#define CONFIG_CMD_SUSPEND 1
#endif
//#define CONFIG_IR_REMOTE 1
#define CONFIG_L2_OFF	 1

#define CONFIG_CMD_NET   1
#if defined(CONFIG_CMD_NET)
	#define CONFIG_AML_ETHERNET 1
	#define CONFIG_NET_MULTI 1
	#define CONFIG_CMD_PING 1
	#define CONFIG_CMD_DHCP 1
	#define CONFIG_CMD_RARP 1
	#define RMII_PHY_INTERFACE 1
	//#define CONFIG_NET_RGMII
	//#define CONFIG_NET_RMII_CLK_EXTERNAL //use external 50MHz clock source
	#define CONFIG_AML_ETHERNET    1                   /*to link /driver/net/aml_ethernet.c*/
	#define IP101PHY    1                   /*to link /driver/net/aml_ethernet.c*/
	//#define KSZ8091    1                   /*to link /driver/net/aml_ethernet.c*/
	#define CONFIG_HOSTNAME        arm_m8
	#define CONFIG_ETHADDR         00:15:18:01:81:31   /* Ethernet address */
	#define CONFIG_IPADDR          10.18.9.97          /* Our ip address */
	#define CONFIG_GATEWAYIP       10.18.9.1           /* Our getway ip address */
	#define CONFIG_SERVERIP        10.18.9.113         /* Tftp server ip address */
	#define CONFIG_NETMASK         255.255.255.0
#endif /* (CONFIG_CMD_NET) */

//I2C definitions
#define CONFIG_AML_I2C			1
#ifdef CONFIG_AML_I2C
#define CONFIG_CMD_I2C			1
#define HAS_AO_MODULE
#define CONFIG_SYS_I2C_SPEED	400000
#endif	//#ifdef CONFIG_AML_I2C

#define CONFIG_CMD_AML
#define CONFIG_CMD_CPU_TEMP
/*
 * PMU definitions, all PMU devices must be include involved
 * in CONFIG_PLATFORM_HAS_PMU
 */
#define CONFIG_PLATFORM_HAS_PMU
#ifdef CONFIG_PLATFORM_HAS_PMU

#define CONFIG_RN5T618
#ifdef CONFIG_RN5T618
//#define CONFIG_UBOOT_BATTERY_PARAMETER_TEST         // uboot can do battery curve test
//#define CONFIG_UBOOT_BATTERY_PARAMETERS             // uboot can get battery parameters from dts 
#define CONFIG_ALWAYS_POWER_ON                      // if platform without battery, must have
#define CONFIG_DISABLE_POWER_KEY_OFF                // disable power off PMU by long press power key

/*
 * under some cases default voltage of PMU output is 
 * not suitable for application, so you should take care
 * of the following macros which defined initial voltage
 * of each power domain when in SPL stage of uboot.
 */
#define CONFIG_POWER_SPL                            // init power for all domians, must have
#define CONFIG_VCCK_VOLTAGE             1100        // CPU core voltage when boot, must have
#define CONFIG_VDDAO_VOLTAGE            1150        // VDDAO voltage when boot, must have
#define CONFIG_DDR_VOLTAGE              1500        // DDR voltage when boot, must have

#define CONFIG_VDDIO_AO28               2900        // VDDIO_AO28 voltage when boot, option
#define CONFIG_VDDIO_AO18               1800        // VDDIO_AO18 voltage when boot, option
#define CONFIG_RTC_0V9                   900        // RTC_0V9 voltage when boot, option
#define CONFIG_VDD_LDO                  2700        // VDD_LDO voltage when boot, option
#define CONFIG_VCC1V8                   1800        // VCC1.8v voltage when boot, option
#define CONFIG_VCC2V8                   2850        // VCC2.8v voltage when boot, option
#define CONFIG_AVDD1V8                  1800        // AVDD1.8V voltage when boot, option

/*
 * set to 1 if you want decrease voltage of VDDAO when suspend
 */
#define CONFIG_VDDAO_VOLTAGE_CHANGE     1
#ifdef CONFIG_VDDAO_VOLTAGE_CHANGE
#define CONFIG_VDDAO_SUSPEND_VOLTAGE    825         // voltage of VDDAO when suspend
#endif /* CONFIG_VDDAO_VOLTAGE_CHANGE */

/*
 * DCDC mode switch when suspend 
 */
#define CONFIG_DCDC_PFM_PMW_SWITCH      1
#define CONFIG_POWEROFF_VCCX2
#endif /* CONFIG_RN5T618 */

#endif /* CONFIG_PLATFORM_HAS_PMU */

#define CONFIG_SDIO_B1   1
#define CONFIG_SDIO_A    1
#define CONFIG_SDIO_B    1
#define CONFIG_SDIO_C    1
#define CONFIG_ENABLE_EXT_DEVICE_RETRY 1


#define CONFIG_MMU                    1
#define CONFIG_PAGE_OFFSET 	0xc0000000
#define CONFIG_SYS_LONGHELP	1

/* USB
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
/* #define CONFIG_MUSB_UDC		1 */
#define CONFIG_CMD_USB 1
#if defined(CONFIG_CMD_USB)
	#define CONFIG_M8_USBPORT_BASE_A	0xC9040000
	#define CONFIG_M8_USBPORT_BASE_B	0xC90C0000
	#define CONFIG_USB_STORAGE      1
	#define CONFIG_USB_DWC_OTG_HCD  1
	#define CONFIG_USB_DWC_OTG_294	1
#endif //#if defined(CONFIG_CMD_USB)

#define CONFIG_ENABLE_CVBS 1
 
#define CONFIG_UCL 1
#define CONFIG_SELF_COMPRESS 
//#define CONFIG_PREBOOT "mw da004004 80000510;mw c81000014 4000;mw c1109900 0"

#define CONFIG_CMD_AUTOSCRIPT

#define CONFIG_CMD_REBOOT 1
#define CONFIG_PREBOOT 


/* Environment information */
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE		boot.img

#define CONFIG_EXTRA_ENV_SETTINGS \
	"us_delay_step=1\0" \
	"loadaddr=0x12000000\0" \
	"loadaddr_logo=0x13000000\0" \
	"testaddr=0x12400000\0" \
	"console=ttyS0,115200n8\0" \
	"bootm_low=0x00000000\0" \
	"bootm_size=0x80000000\0" \
	"boardname=m8_board\0" \
	"chipname=8726m8\0" \
	"get_dt=checkhw\0" \
	"initrd_high=60000000\0" \
	"hdmimode=1080p\0" \
	"cvbsmode=576cvbs\0" \
	"outputmode=1080p\0" \
	"vdac_config=0x10\0" \
	"initargs=rootfstype=ramfs init=/init console=ttyS0,115200n8 no_console_suspend ramoops.mem_address=0x04e00000 ramoops.mem_size=0x100000 ramoops.record_size=0x8000 ramoops.console_size=0x4000\0" \
	"preloaddtb=imgread dtb boot ${loadaddr}\0" \
	"video_dev=tvout\0" \
	"display_width=1920\0" \
	"display_height=1080\0" \
	"display_bpp=16\0" \
	"display_color_format_index=16\0" \
	"display_layer=osd2\0" \
	"display_color_fg=0xffff\0" \
	"display_color_bg=0\0" \
	"fb_addr=0x07900000\0" \
	"fb_width=1280\0"\
	"fb_height=720\0"\
	"partnum=2\0" \
	"p0start=1000000\0" \
	"p0size=400000\0" \
	"p0path=uImage\0" \
	"p1start=1400000\0" \
	"p1size=8000000\0" \
	"p1path=android.rootfs\0" \
	"bootstart=0\0" \
	"bootsize=100000\0" \
	"bootpath=u-boot.bin\0" \
	"sdcburncfg=aml_sdc_burn.ini\0"\
	"normalstart=1000000\0" \
	"normalsize=400000\0" \
	"upgrade_step=0\0" \
	"firstboot=1\0" \
	"store=0\0"\
	"wipe_data=successful\0"\
	"wipe_cache=successful\0"\
	"cvbs_drv=0\0"\
	"preboot="\
        "run factory_reset_poweroff_protect;"\
        "if itest ${upgrade_step} == 3; then run prepare; run storeargs; run update; fi; "\
        "if itest ${upgrade_step} == 1; then  "\
            "defenv_reserve_env; setenv upgrade_step 2; saveenv;"\
        "fi; "\
        "run check_rebootmode;"\
        "run prepare;"\
        "run storeargs;"\
        "run update_key; " \
        "run switch_bootmode\0" \
    \
    "update_key="\
        "saradc open 0; " \
        "if saradc get_in_range 0 0x50; then " \
            "msleep 50; " \
            "if saradc get_in_range 0 0x50; then echo update by key...; run update; fi;" \
        "fi\0" \
    \
   	"update="\
        /*first try usb burning, second sdc_burn, third autoscr, last recovery*/\
        "run usb_burning; "\
        "if mmcinfo; then "\
            "if fatexist mmc 0 ${sdcburncfg}; then "\
                "run sdc_burning; "\
            "else "\
                "if fatload mmc 0 ${loadaddr} aml_autoscript; then autoscr ${loadaddr}; fi;"\
                "run recovery;"\
            "fi;"\
        "else "\
            "run recovery;"\
        "fi;\0"\
    \
   	"storeargs="\
        "setenv bootargs ${initargs} cvbsdrv=${cvbs_drv} vdaccfg=${vdac_config} logo=osd1,loaded,${fb_addr},${outputmode},full hdmimode=${hdmimode} cvbsmode=${cvbsmode} androidboot.firstboot=${firstboot} hdmitx=${cecconfig}\0"\
    \
	"switch_bootmode="\
        "if test ${reboot_mode} = factory_reset; then "\
                "run recovery;"\
        "else if test ${reboot_mode} = update; then "\
        	"run update;"\
        "else if test ${reboot_mode} = usb_burning; then "\
        	"run usb_burning;"\
        "else if test ${wipe_data} = failed; then "\
        	"echo wipe_data=${wipe_data}; run recovery;"\
        "else " \
        	"  "\
        "fi;fi;fi;fi\0" \
    \
    "prepare="\
        "logo size ${outputmode}; video open; video clear; video dev open ${outputmode};"\
        "imgread pic logo bootup ${loadaddr_logo}; "\
        "bmp display ${bootup_offset}; bmp scale;"\
        "\0"\
	\
	"storeboot="\
                "if test ${reboot_mode} = charging; then run try_auto_burn; fi;"\
		"secukey auto;" \
		"secukey write keyexample 1234567890; "\
        "echo Booting...; "\
        "if unifykey get usid; then  "\
            "setenv bootargs ${bootargs} androidboot.serialno=${usid};"\
        "fi;"\
        "if unifykey get mac; then  "\
            "setenv bootargs ${bootargs} mac=${mac};"\
        "fi;"\
        "imgread kernel boot ${loadaddr};"\
        "bootm;"\
        "run recovery\0" \
    \
	"recovery="\
        "echo enter recovery;"\
        "setenv bootargs ${bootargs} wipeinstaboot;"\
        "if mmcinfo; then "\
            "if fatload mmc 0 ${loadaddr} recovery.img; then bootm;fi;"\
        "fi; "\
        "if usb start 0; then "\
                "if fatload usb 0 ${loadaddr} recovery.img; then bootm; fi;"\
        "fi;"\
	      "if imgread kernel recovery ${loadaddr}; then "\
	        "bootm; "\
				"else "\
					"echo no recovery in flash; "\
				"fi;\0" \
    \
    "check_rebootmode="\
		"get_rebootmode; clear_rebootmode; echo reboot_mode=${reboot_mode};"\
		"if test ${reboot_mode} = factory_reset; then defenv; saveenv; fi\0" \
    \
    "factory_reset_poweroff_protect="\
        "echo wipe_data=${wipe_data}; echo wipe_cache=${wipe_cache};"\
        "if test ${wipe_data} = failed; then "\
            "run prepare; run storeargs; run recovery;"\
        "fi; "\
        "if test ${wipe_cache} = failed; then "\
            "run prepare; run storeargs; run recovery;"\
        "fi; \0" \
    \
	"usb_burning=update 1000\0" \
        "try_auto_burn=update 700 750;\0"\
    "sdc_burning=sdc_burn ${sdcburncfg}\0"


#define CONFIG_BOOTCOMMAND   "run storeboot"

#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_ENV_SIZE         (64*1024)

#define CONFIG_STORE_COMPATIBLE

#ifdef  CONFIG_STORE_COMPATIBLE
//spi
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_SECT_SIZE 0x1000
 #define CONFIG_ENV_IN_SPI_OFFSET 0x100000
//nand
#define CONFIG_ENV_IN_NAND_OFFSET 0x400000
#define CONFIG_ENV_BLOCK_NUM 2
//emmc
#define CONFIG_SYS_MMC_ENV_DEV 1
#define CONFIG_ENV_IN_EMMC_OFFSET 0x80000

#else

#define CONFIG_SPI_BOOT 1
//#define CONFIG_MMC_BOOT
//#define CONFIG_NAND_BOOT 1

#ifdef CONFIG_NAND_BOOT
	#define CONFIG_AMLROM_NANDBOOT 1
#endif 


#ifdef CONFIG_SPI_BOOT
	#define CONFIG_ENV_OVERWRITE
	#define CONFIG_ENV_IS_IN_SPI_FLASH
	#define CONFIG_CMD_SAVEENV	
	#define CONFIG_ENV_SECT_SIZE		0x10000
	#define CONFIG_ENV_OFFSET           0x1f0000
#elif defined CONFIG_NAND_BOOT
	#define CONFIG_ENV_IS_IN_AML_NAND
	#define CONFIG_CMD_SAVEENV
	#define CONFIG_ENV_OVERWRITE	
	#define CONFIG_ENV_OFFSET       0x400000
	#define CONFIG_ENV_BLOCK_NUM    2
#elif defined CONFIG_MMC_BOOT
	#define CONFIG_ENV_IS_IN_MMC
	#define CONFIG_CMD_SAVEENV
    #define CONFIG_SYS_MMC_ENV_DEV        0	
	#define CONFIG_ENV_OFFSET       0x1000000		
#else
	#define CONFIG_ENV_IS_NOWHERE    1
#endif

#endif


//wifi wake up
#define CONFIG_WIFI_WAKEUP 1
#define CONFIG_NET_WIFI

//----------------------------------------------------------------------
//Please set the M8 CPU clock(unit: MHz)
//legal value: 600, 792, 996, 1200,1440
#if defined(CONFIG_CPU_792M)
#define M8_CPU_CLK          (792)
#else
#define M8_CPU_CLK 		    (1200)
#endif
#define CONFIG_SYS_CPU_CLK	(M8_CPU_CLK)
//----------------------------------------------------------------------

//-----------------------------------------------------------------------
//DDR setting
//For DDR PUB training not check the VT done flag
#define CONFIG_NO_DDR_PUB_VT_CHECK 1

//For DDR clock gating disable
#define CONFIG_GATEACDDRCLK_DISABLE 1

//For DDR low power feature disable
//#define CONFIG_DDR_LOW_POWER_DISABLE 1

//For DDR PUB WL/WD/RD/RG-LVT, WD/RD-BVT disable
#define CONFIG_PUB_WLWDRDRGLVTWDRDBVT_DISABLE 1

//current DDR clock range (408~804)MHz with fixed step 12MHz
#define CONFIG_DDR_CLK           792 //696 //768  //792// (636)
#define CONFIG_DDR_MODE          CFG_DDR_BUS_WIDTH_32BIT //m8 doesn't support
#define CONFIG_DDR_CHANNEL_SET   CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12

//On board DDR capacity
/*DDR capactiy support 512MB, 1GB, 1.5GB, 2GB, 3GB*/
#define CONFIG_DDR_SIZE          1024 //MB. Legal value: 512, 1024, 1536, 2048, 3072

#ifdef CONFIG_ACS
//#define CONFIG_DDR_CHANNEL_AUTO_DETECT	//ddr channel setting auto detect
//#define CONFIG_DDR_MODE_AUTO_DETECT	//ddr bus-width auto detection. m8 doesn't support.
#define CONFIG_DDR_SIZE_AUTO_DETECT	//ddr size auto detection
#endif

#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
#define CONFIG_AUTO_SET_MULTI_DT_ID    //if wanna pass mem=xx to kernel, pls disable this config
#ifndef CONFIG_AUTO_SET_MULTI_DT_ID
#define CONFIG_AUTO_SET_BOOTARGS_MEM
#endif
#endif

#define CONFIG_DUMP_DDR_INFO 1
#define CONFIG_ENABLE_WRITE_LEVELING 1
//#define DDR_SCRAMBE_ENABLE  1

#define CONFIG_SYS_MEMTEST_START      0x10000000  /* memtest works on */      
#define CONFIG_SYS_MEMTEST_END        0x18000000  /* 0 ... 128 MB in DRAM */  
//#define CONFIG_ENABLE_MEM_DEVICE_TEST 1
#define CONFIG_NR_DRAM_BANKS	      1	          /* CS1 may or may not be populated */

/* Pass open firmware flat tree*/
#define CONFIG_OF_LIBFDT	1
#define CONFIG_DT_PRELOAD	1
#define CONFIG_SYS_BOOTMAPSZ   PHYS_MEMORY_SIZE       /* Initial Memory map for Linux */
#define CONFIG_ANDROID_IMG	1

#define CONFIG_CMD_IMGPACK 1

//M8 secure boot disable
#ifdef CONFIG_BL31
#define CONFIG_AML_DISABLE_CRYPTO_UBOOT 1
#endif

//M8 L1 cache enable for uboot decompress speed up
#define CONFIG_AML_SPL_L1_CACHE_ON	1


/*-----------------------------------------------------------------------
 * power down
 */
//#define CONFIG_CMD_RUNARC 1 /* runarc */
#ifndef CONFIG_BL31
#define CONFIG_AML_SUSPEND 1
#endif

#define CONFIG_CMD_LOGO


/*
* CPU switch test for uboot
*/
//#define CONFIG_M8_TEST_CPU_SWITCH 1


/*
 * Secure OS
 */
#ifdef CONFIG_MESON_TRUSTZONE

#ifdef CONFIG_BL31
#define CONFIG_JOIN_UBOOT_SECUREOS 1
#define SECUREOS_KEY_BASE_ADDR 0x06100000
#define SECURE_OS_DECOMPRESS_ADDR 0x06200000

#else  // CONFIG_BL31
#define CONFIG_MESON_SECUREARGS  1
//#define CONFIG_MESON_SECURE_HDCP 1
#define CONFIG_JOIN_UBOOT_SECUREOS 1
#define SECUREOS_KEY_BASE_ADDR 0x06100000
#define SECURE_OS_DECOMPRESS_ADDR 0x06200000
#define CONFIG_SECURE_STORAGE_BURNED
#ifdef CONFIG_SECURE_STORAGE_BURNED
#define CONFIG_MESON_STORAGE_BURN 1
#define CONFIG_MESON_STORAGE_DEBUG
#define CONFIG_SECURESTORAGEKEY
#define CONFIG_RANDOM_GENERATE
#define CONFIG_CMD_SECURESTORE
#define CONFIG_CMD_RANDOM
/* secure storage support both spi and emmc */
#define CONFIG_SECURE_MMC
#define CONFIG_SPI_NOR_SECURE_STORAGE
#endif // CONFIG_SECURE_STORAGE_BURNED

#endif  // CONFIG_BL31
#endif //CONFIG_MESON_TRUSTZONE

#endif //__CONFIG_M8_K200_V1_H__
