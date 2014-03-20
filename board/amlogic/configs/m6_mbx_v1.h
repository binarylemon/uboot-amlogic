#ifndef __CONFIG_M6_MBX_V1_H__
#define __CONFIG_M6_MBX_V1_H__

#define CONFIG_AML_MESON_6 1
#define CONFIG_MACH_MESON6_MBX
//#define CONFIG_MESON_ARM_GIC_FIQ

//#define TEST_UBOOT_BOOT_SPEND_TIME
 
/*
 *  write to efuse/nand when usb_burning 
 *  WRITE_TO_EFUSE_ENABLE and WRITE_TO_NAND_ENABLE should not be both existed
 */
#define CONFIG_AML_MESON6
//#define WRITE_TO_EFUSE_ENABLE        
#define WRITE_TO_NAND_ENABLE

#define CONFIG_IR_REMOTE

#define CONFIG_SECURITYKEY
#ifdef CONFIG_SECURITYKEY
#define CONFIG_AML_NAND_KEY
#define CONFIG_AML_EMMC_KEY	1
#endif

//#define CONFIG_SWITCH_BOOT_MODE 1
#define CONFIG_HDCP_PREFETCH 1

#if defined(WRITE_TO_EFUSE_ENABLE) && defined(WRITE_TO_NAND_ENABLE)
#error You should only select one of WRITE_TO_EFUSE_ENABLE and WRITE_TO_NAND_ENABLE
#endif

#define CONFIG_ACS
#ifdef CONFIG_ACS
/*pass memory size, spl->uboot, can not use 0xD9000000 - 0xD900C000, or secure boot will fail*/
#define CONFIG_DDR_SIZE_IND_ADDR 0xd901ff7c
#endif

// cart type of each port
#define PORT_A_CARD_TYPE            CARD_TYPE_UNKNOWN
#define PORT_B_CARD_TYPE            CARD_TYPE_UNKNOWN
#define PORT_C_CARD_TYPE            CARD_TYPE_UNKNOWN // CARD_TYPE_MMC/CARD_TYPE_SD

#define CONFIG_AML_TINY_USBTOOL

//UART Sectoion
#define CONFIG_CONS_INDEX   2

//support "boot,bootd"
//#define CONFIG_CMD_BOOTD 1
//#define CONFIG_AML_I2C      1

#define HAS_AO_MODULE
#define CONFIG_AML_I2C	//add by Elvis Yu
//#define CONFIG_AW_AXP20

//Enable storage devices
//#ifndef CONFIG_JERRY_NAND_TEST
#define CONFIG_NEXT_NAND 

#define CONFIG_CMD_NAND  1
//#endif
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

#define CONFIG_L2_OFF			1
//#define CONFIG_ICACHE_OFF	1
//#define CONFIG_DCACHE_OFF	1


#define CONFIG_CMD_NET   1

#if defined(CONFIG_CMD_NET)
#define CONFIG_AML_ETHERNET 1
#define CONFIG_NET_MULTI 1
#define CONFIG_CMD_PING 1
#define CONFIG_CMD_DHCP 1
#define CONFIG_CMD_RARP 1

//#define CONFIG_NET_RGMII
#define CONFIG_NET_RMII_CLK_EXTERNAL //use external 50MHz clock source

#define CONFIG_AML_ETHERNET    1                   /*to link /driver/net/aml_ethernet.c*/
#define CONFIG_HOSTNAME        arm_m6
#define CONFIG_ETHADDR         00:15:18:01:81:31   /* Ethernet address */
#define CONFIG_IPADDR          10.18.9.97          /* Our ip address */
#define CONFIG_GATEWAYIP       10.18.9.1           /* Our getway ip address */
#define CONFIG_SERVERIP        10.18.9.113         /* Tftp server ip address */
#define CONFIG_NETMASK         255.255.255.0
#endif /* (CONFIG_CMD_NET) */


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
#define CONFIG_M6_USBPORT_BASE	0xC90C0000
#define CONFIG_M6_USBPORT_BASE_A	0xC9040000
#define CONFIG_USB_STORAGE      1
#define CONFIG_USB_DWC_OTG_HCD  1
#define CONFIG_USB_DWC_OTG_294	1
#define CONFIG_CMD_USB 1

//#define CONFIG_USB_ETHER
#ifdef CONFIG_USB_ETHER
#define IO_USB_A_BASE			0xc9040000
#define CONFIG_USBPORT_BASE IO_USB_A_BASE
#define CONFIG_SYS_CACHELINE_SIZE       64 
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED
#endif



#define CONFIG_UCL 1
#define CONFIG_SELF_COMPRESS

//#define CONFIG_IMPROVE_UCL_DEC   1

#ifdef CONFIG_IMPROVE_UCL_DEC
#define UCL_DEC_EN_IDCACHE        1
#define UCL_DEC_EN_IDCACHE_FINE_TUNE  1
#endif


#define CONFIG_CMD_AUTOSCRIPT
//#define CONFIG_CMD_AML 1
#define CONFIG_CMD_IMGPACK 1
#define CONFIG_CMD_REBOOT 1
#define CONFIG_CMD_MATH 1

#define CONFIG_ANDROID_IMG 1

/* Environment information */
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE		boot.img


#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
		"loadaddr_logo=0x83000000\0" \
		"testaddr=0x82400000\0" \
		"console=ttyS0,115200n8\0" \
		"mmcargs=setenv bootargs console=${console} " \
		"boardname=m6_mbx\0" \
		"chipname=8726m6\0" \
		"machid=4e27\0" \
		"hdmimode=1080p\0" \
		"cvbsmode=576cvbs\0" \
		"outputmode=1080p\0" \
		"bootargs=init=/init console=ttyS0,115200n8 no_console_suspend\0" \
		"preloaddtb=imgread dtb boot ${loadaddr}\0" \
		"video_dev=tvout\0" \
		"display_width=1920\0" \
		"display_height=1080\0" \
		"display_bpp=24\0" \
		"display_color_format_index=24\0" \
		"display_layer=osd2\0" \
		"display_color_fg=0xffff\0" \
		"display_color_bg=0\0" \
		"fb_addr=0x85100000\0" \
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
		"bootsize=60000\0" \
		"bootpath=u-boot.bin\0" \
		"sdcburncfg=aml_sdc_burn.ini\0"\
		"normalstart=1000000\0" \
		"normalsize=400000\0" \
		"upgrade_step=0\0" \
		"firstboot=1\0" \
		"store=0\0"\
		"preboot="\
			"echo preboot...;" \
			"if itest ${upgrade_step} == 3; then run prepare; run storeargs; run update; fi; "\
			"if itest ${upgrade_step} == 1; then  "\
				"defenv; setenv upgrade_step 2; saveenv;"\
			"fi; "\
			"run prepare;"\
			"run storeargs;"\
			"get_rebootmode; clear_rebootmode; echo reboot_mode=${reboot_mode};" \
			"run update_key; " \
			"run switch_bootmode\0" \
		\
		"update_key="\
			"saradc open 0; " \
			"if saradc get_in_range 0 0x50; then " \
				"msleep 400; " \
				"if saradc get_in_range 0 0x50; then echo update by key...; run update; fi;" \
			"fi\0" \
		\
		"update="\
			/*first try usb burning, second sdc_burn, third autoscr, last recovery*/\
			"echo update...; "\
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
			"setenv bootargs ${bootargs} logo=osd1,loaded,${fb_addr},${outputmode},full hdmimode=${hdmimode} cvbsmode=${cvbsmode} androidboot.firstboot=${firstboot} hdmitx=${cecconfig}\0"\
		\
		"switch_bootmode="\
			"echo switch_bootmode...;"\
			"if test ${reboot_mode} = normal; then "\
			"else if test ${reboot_mode} = charging; then "\
			"else if test ${reboot_mode} = factory_reset; then "\
				"run recovery;"\
			"else if test ${reboot_mode} = update; then "\
				"run update;"\
			"else if test ${reboot_mode} = usb_burning; then "\
				"run usb_burning;"\
			"else " \
				"  "\
			"fi;fi;fi;fi;fi\0" \
		\
		"prepare="\
			"logo size ${outputmode}; video open; video clear; video dev open ${outputmode};"\
			"imgread res logo ${loadaddr_logo}; "\
			"unpackimg ${loadaddr_logo}; "\
			"logo source ${outputmode}; bmp display ${bootup_offset}; bmp scale;"\
			"\0"\
		\
		"storeboot="\
			"echo Booting...; "\
			"if unifykey get usid; then  "\
				"setenv bootargs ${bootargs} androidboot.serialno=${usid};"\
			"fi;"\
			"imgread kernel boot ${loadaddr};"\
			"bootm;"\
			"run recovery\0" \
		\
		"recovery="\
			"echo enter recovery;"\
			"if mmcinfo; then "\
				"if fatload mmc 0 ${loadaddr} recovery.img; then bootm;fi;"\
			"fi; "\
			"imgread kernel recovery ${loadaddr}; "\
			"bootm\0" \
		\
		"usb_burning=update 1000\0" \
		"sdc_burning=sdc_burn ${sdcburncfg}\0"
		
#define CONFIG_BOOTCOMMAND   "saveenv;run storeboot"
/*
#define CONFIG_BOOTCOMMAND \
 "mmcinfo 0;fatload mmc 0 82000000 aml_logo480.bmp;video dev open 480P;bmp display 82000000;video open"
*/

//\\temp above

#define CONFIG_AUTO_COMPLETE	1

#define CONFIG_ENV_SIZE         0x8000

#define CONFIG_STORE_COMPATIBLE

//spi
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_SECT_SIZE 0x1000
 #define CONFIG_ENV_IN_SPI_OFFSET 0x80000
//nand
#define CONFIG_ENV_IN_NAND_OFFSET 0x400000
#define CONFIG_ENV_BLOCK_NUM 2
//emmc
#define CONFIG_SYS_MMC_ENV_DEV 1
#define CONFIG_ENV_IN_EMMC_OFFSET 0x80000





/*
 * Size of malloc() pool
 */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 26))
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
						/* initial data */


#define BOARD_LATE_INIT
#define CONFIG_PREBOOT
#define CONFIG_VIDEO_AML
/* config TV output */
#define CONFIG_VIDEO_AMLTVOUT
#define CONFIG_AML_HDMI_TX 1
/* config LCD output */
//#define CONFIG_VIDEO_AMLLCD
//#define CONFIG_VIDEO_AMLLCD_M3
#define CONFIG_CMD_BMP
#define LCD_BPP LCD_COLOR24
#define TV_BPP LCD_BPP
#define LCD_TEST_PATTERN
#ifndef CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#endif
/*end config LCD output*/

/*POST support*/
/*
#define CONFIG_POST (CONFIG_SYS_POST_CACHE	| CONFIG_SYS_POST_BSPEC1 |	\
										CONFIG_SYS_POST_RTC | CONFIG_SYS_POST_ADC | \
										CONFIG_SYS_POST_PLL)
*/
//CONFIG_SYS_POST_MEMORY

#undef CONFIG_POST
#ifdef CONFIG_POST
#define CONFIG_POST_AML
#define CONFIG_POST_ALT_LIST
#define CONFIG_SYS_CONSOLE_IS_IN_ENV  /* Otherwise it catches logbuffer as output */
#define CONFIG_LOGBUFFER
#define CONFIG_CMD_DIAG

#define SYSTEST_INFO_L1 1
#define SYSTEST_INFO_L2 2
#define SYSTEST_INFO_L3 3

#define CONFIG_POST_BSPEC1 {    \
	"L2CACHE test", \
	"l2cache", \
	"This test verifies the L2 cache operation.", \
	POST_RAM | POST_MANUAL,   \
	&l2cache_post_test,		\
	NULL,		\
	NULL,		\
	CONFIG_SYS_POST_BSPEC1 	\
	}

#define CONFIG_POST_BSPEC2 {  \
	"BIST test", \
	"bist", \
	"This test checks bist test", \
	POST_RAM | POST_MANUAL, \
	&bist_post_test, \
	NULL, \
	NULL, \
	CONFIG_SYS_POST_BSPEC1  \
	}
#endif   /*end ifdef CONFIG_POST*/

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
//Please just define M6 DDR clock here only
//current DDR clock range (300~600)MHz
#define M6_DDR_CLK (516)

#define CONFIG_DDR_LOW_POWER


//#define DDR3_9_9_9
#define DDR3_7_7_7
//above setting must be set for ddr_set __ddr_setting in file
//board/amlogic/m6_ref_v1/firmware/timming.c

//note: please DO NOT remove following check code
#if !defined(DDR3_9_9_9) && !defined(DDR3_7_7_7)
	#error "Please set DDR3 property first in file m6_ref_v1.h\n"
#endif

#define M6_DDR3_1GB
//#define M6_DDR3_512M
//above setting will affect following:
//board/amlogic/m6_ref_v1/firmware/timming.c
//arch/arm/cpu/aml_meson/m6/mmutable.s

//note: please DO NOT remove following check code
#if !defined(M6_DDR3_1GB) && !defined(M6_DDR3_512M)
	#error "Please set DDR3 capacity first in file m6_ref_v1.h\n"
#endif


#define CONFIG_NR_DRAM_BANKS    1   /* CS1 may or may not be populated */

#define PHYS_MEMORY_START    0x80000000 // from 500000
#if defined(M6_DDR3_1GB)
	#define PHYS_MEMORY_SIZE     0x40000000 // 1GB
#elif defined(M6_DDR3_512M)
	#define PHYS_MEMORY_SIZE     0x20000000 // 512M
#else
	#error "Please define DDR3 memory capacity in file m6_ref_v1.h\n"
#endif

#define CONFIG_SYS_MEMTEST_START    0x80000000  /* memtest works on */
#define CONFIG_SYS_MEMTEST_END      0x07000000  /* 0 ... 120 MB in DRAM */
#define CONFIG_ENABLE_MEM_DEVICE_TEST 1
#define CONFIG_NR_DRAM_BANKS	1	/* CS1 may or may not be populated */

#define DDR_DETECT168_NAND_D7
//////////////////////////////////////////////////////////////////////////


//M6 security boot enable
//#define CONFIG_M6_SECU_BOOT		 1
//M6 2-RSA signature enable, enable CONFIG_M6_SECU_BOOT
//first before use this feature
//#define CONFIG_M6_SECU_BOOT_2RSA   1

//M6 Auth-key build to uboot
//#define CONFIG_M6_SECU_AUTH_KEY 1


//enable CONFIG_M6_SECU_BOOT_2K must enable CONFIG_M6_SECU_BOOT first
#if defined(CONFIG_M6_SECU_BOOT_2K)
	#if !defined(CONFIG_M6_SECU_BOOT)
		#define CONFIG_M6_SECU_BOOT 1
	#endif //!defined(CONFIG_M6_SECU_BOOT)
#endif //defined(CONFIG_M6_SECU_BOOT_2K)


//M6 L1 cache enable for uboot decompress speed up
//#define CONFIG_AML_SPL_L1_CACHE_ON	1

//////////////////////////////////////////////////////////////////////////

//M6 secure firmware
//#define CONFIG_AML_SECURE 1

/* Pass open firmware flat tree*/
#define CONFIG_OF_LIBFDT	1
#define CONFIG_SYS_BOOTMAPSZ   PHYS_MEMORY_SIZE       /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * power down
 */
//#define CONFIG_CMD_RUNARC 1 /* runarc */
//#define CONFIG_CMD_SUSPEND 1
#define CONFIG_AML_SUSPEND 1
#define CONFIG_CEC_WAKE_UP 1

#endif //__CONFIG_M6_REF_V1_H__
