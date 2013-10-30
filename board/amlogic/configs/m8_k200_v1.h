#ifndef __CONFIG_M8_K200_V1_H__
#define __CONFIG_M8_K200_V1_H__

#define CONFIG_AML_MESON_8      1
#define CONFIG_MACH_MESON8_K200_V1  // generate M8 K200 machid number

#ifndef CONFIG_M8
#define CONFIG_M8
#endif // ifndef CONFIG_M8
//#define	CONFIG_VLSI_EMULATOR 1
//#define TEST_UBOOT_BOOT_SPEND_TIME

//UART Sectoion
#define CONFIG_CONS_INDEX   2

//#define CONFIG_UNIFY_KEY_MANAGE

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

//#define CONFIG_NEXT_NAND
//#define CONFIG_SECURE_NAND  1
//support "boot,bootd"
//#define CONFIG_CMD_BOOTD 1

//Enable HDMI Tx
//#define CONFIG_VIDEO_AMLTVOUT 1
//Enable LCD output
//#define CONFIG_VIDEO_AMLLCD
//#define LCD_BPP LCD_COLOR16

#define CONFIG_ACS
#ifdef CONFIG_ACS
#define CONFIG_DDR_SIZE_IND_ADDR 0xD9000000	//pass memory size, spl->uboot
#endif

#ifdef CONFIG_NEXT_NAND
#define CONFIG_AML_V2_USBTOOL 1
#endif//#ifdef CONFIG_NEXT_NAND

#if CONFIG_AML_V2_USBTOOL
#define CONFIG_SHA1
#ifdef CONFIG_ACS
#define CONFIG_TPL_BOOT_ID_ADDR       		(0xD9000000U + 4)//pass boot_id, spl->uboot
#else
#define CONFIG_TPL_BOOT_ID_ADDR       		(&reboot_mode)//pass boot_id, spl->uboot
#endif// #ifdef CONFIG_ACS
#endif// #if CONFIG_AML_V2_USBTOOL


//Enable storage devices
//#define CONFIG_CMD_NAND  1
//#define CONFIG_VIDEO_AML 1
//#define CONFIG_CMD_BMP 1
//#define CONFIG_VIDEO_AMLTVOUT 1

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
#define CONFIG_CMD_SUSPEND 1
//#define CONFIG_IR_REMOTE 1
#define CONFIG_L2_OFF	 1

//#define CONFIG_CMD_NET   1
#if defined(CONFIG_CMD_NET)
	#define CONFIG_AML_ETHERNET 1
	#define CONFIG_NET_MULTI 1
	#define CONFIG_CMD_PING 1
	#define CONFIG_CMD_DHCP 1
	#define CONFIG_CMD_RARP 1
	//#define CONFIG_NET_RGMII
	//#define CONFIG_NET_RMII_CLK_EXTERNAL //use external 50MHz clock source
	#define CONFIG_AML_ETHERNET    1                   /*to link /driver/net/aml_ethernet.c*/
	#define IP101PHY    1                   /*to link /driver/net/aml_ethernet.c*/
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
/*
 * PMU definitions, all PMU devices must be include involved
 * in CONFIG_PLATFORM_HAS_PMU
 */
#define CONFIG_PLATFORM_HAS_PMU
#ifdef CONFIG_PLATFORM_HAS_PMU

#define CONFIG_RN5T618
#ifdef CONFIG_RN5T618
#define CONFIG_UBOOT_BATTERY_PARAMETER_TEST
#define CONFIG_UBOOT_BATTERY_PARAMETERS 

#define CONFIG_POWER_SPL
#define CONFIG_VCCK_VOLTAGE             1050
#define CONFIG_VDDAO_VOLTAGE            1100
#define CONFIG_DDR_VOLTAGE              1500

#define CONFIG_DCDC_PFM_PMW_SWITCH      1 
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
//#define CONFIG_CMD_USB 1
#if defined(CONFIG_CMD_USB)
	#define CONFIG_M8_USBPORT_BASE_A	0xC9040000
	#define CONFIG_M8_USBPORT_BASE_B	0xC90C0000
	#define CONFIG_USB_STORAGE      1
	#define CONFIG_USB_DWC_OTG_HCD  1
	#define CONFIG_USB_DWC_OTG_294	1
#endif //#if defined(CONFIG_CMD_USB)


#define CONFIG_UCL 1
#define CONFIG_SELF_COMPRESS 
//#define CONFIG_PREBOOT "mw da004004 80000510;mw c81000014 4000;mw c1109900 0"

#define CONFIG_CMD_AUTOSCRIPT

#define CONFIG_CMD_REBOOT 1
#define CONFIG_PREBOOT 


/* Environment information */
#define CONFIG_BOOTDELAY	10
#define CONFIG_BOOTFILE		boot.img

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x12000000\0" \
	"loadaddr_logo=0x13000000\0" \
	"testaddr=0x12400000\0" \
	"console=ttyS0,115200n8\0" \
	"bootm_low=0x00000000\0" \
	"bootm_size=0x80000000\0" \
	"mmcargs=setenv bootargs console=${console} " \
	"boardname=m8_board\0" \
	"chipname=8726m8\0" \
	"initrd_high=60000000\0" \
	"hdmimode=1080p\0" \
	"cvbsmode=576cvbs\0" \
	"bootargs=init=/init console=ttyS0,115200n8 no_console_suspend\0" \
	"display_width=1920\0" \
	"display_height=1080\0" \
	"fb_addr=0x15100000\0" \
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
		"get_rebootmode; clear_rebootmode; echo reboot_mode=${reboot_mode};" \
        "run update_key; " \
        "run switch_bootmode\0" \
    \
    "update_key="\
        "echo update by key...; " \
        "saradc open 0; " \
        "if saradc get_in_range 0 0x50; then " \
            "msleep 400; " \
            "if saradc get_in_range 0 0x50; then run update; fi;" \
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
        "store read logo ${loadaddr_logo} 0 800000;unpackimg ${loadaddr_logo}; "\
        "cp ${bootup_offset} ${fb_addr} ${bootup_size};"\
        "setenv bootargs init=/init console=ttyS0,115200n8 no_console_suspend logo=osd1,${fb_addr},${hdmimode},full,needscaler hdmimode=${hdmimode} cvbsmode=${cvbsmode} storage=${store} androidboot.firstboot=${firstboot}\0"\
    \
	"switch_bootmode="\
		"echo switch_bootmode...;" \
		"if test ${reboot_mode} = factory_reset; then run recovery; fi;"\
        "if test ${reboot_mode} = update; then run update; fi;"\
        "if test ${reboot_mode} = usb_burning; then run usb_burning; fi;"\
        "\0" \
    \
	"storeboot="\
        "echo Booting...; "\
        "run storeargs;"\
        "store read boot ${loadaddr} 0 600000;"\
        "bootm;"\
        "run recovery\0" \
    \
	"recovery="\
        "echo enter recovery;"\
        "run storeargs;"\
        "if mmcinfo; then "\
            "if fatload mmc 0 ${loadaddr} recovery.img; then bootm;fi;"\
        "fi; "\
        "store read recovery ${loadaddr} 0 600000; bootm\0" \
    \
	"usb_burning=update 2000\0" \
    "sdc_burning=sdc_burn ${sdcburncfg}\0"


#define CONFIG_BOOTCOMMAND   "run storeboot"

#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_ENV_SIZE         (64*1024)

//#define CONFIG_STORE_COMPATIBLE

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


//----------------------------------------------------------------------
//Please set the M8 CPU clock(unit: MHz)
//legal value: 600, 792, 996, 1200
#define M8_CPU_CLK 		    (792)
#define CONFIG_SYS_CPU_CLK	(M8_CPU_CLK)
//----------------------------------------------------------------------

//-----------------------------------------------------------------------
//DDR setting
#define CONFIG_M8_DDR_CHANNEL_SET (CONFIG_M8_DDRX2_S12)
#define CONFIG_M8_DDR_AMBM_SET    (CONFIG_M8_DDR_ADDR_MAP_BANK_MODE_4_BNK)


//Please just define m8 DDR clock here only
//current DDR clock range (408~804)MHz with fixed step 12MHz
#define CFG_M8_DDR_CLK    696 //768  //792// (636)

//#define CONFIG_DDR_LOW_POWER 1

//On board DDR capactiy
//#define CFG_M8_DDR3_1GB
#define CFG_M8_DDR3_2GB
//#define CFG_M8_DDR3_4GB
//above setting will affect following:
//board/amlogic/m8_k200_v1/firmware/timming.c
//arch/arm/cpu/aml_meson/m8/mmutable.s

//DDR row/col size
//row size.  2'b01 : A0~A12.   2'b10 : A0~A13.  2'b11 : A0~A14.  2'b00 : A0~A15.
//col size.   2'b01 : A0~A8,      2'b10 : A0~A9  
#if   defined(CFG_M8_DDR3_1GB)
	//2Gb(X16) x 4pcs
	#define CONFIG_M8_DDR3_ROW_SIZE (2)
	#define CONFIG_M8_DDR3_COL_SIZE (2)
	#define CONFIG_M8_DDR_ROW_BITS  (14)
#elif defined(CFG_M8_DDR3_2GB)
	//4Gb(X16) x 4pcs
	#define CONFIG_M8_DDR3_ROW_SIZE (3)
	#define CONFIG_M8_DDR3_COL_SIZE (2)
	#define CONFIG_M8_DDR_ROW_BITS  (15)
#elif defined(CFG_M8_DDR3_4GB)
	//8Gb(X16) x 4pcs
	#define CONFIG_M8_DDR3_ROW_SIZE (0)
	#define CONFIG_M8_DDR3_COL_SIZE (2)
	#define CONFIG_M8_DDR_ROW_BITS  (16)
#elif !defined(CFG_M8_DDR3_1GB) && !defined(CFG_M8_DDR3_2GB) && !defined(CFG_M8_DDR3_4GB)
	#error "Please set DDR3 capacity first in file m8_k200_v1.h\n"
#endif

#define CONFIG_M8_DUMP_DDR_INFO 1

#define CONFIG_ENABLE_WRITE_LEVELING 1

#define PHYS_MEMORY_START        (0x00000000) // ???
#if defined(CFG_M8_DDR3_1GB)
	#define PHYS_MEMORY_SIZE     (0x40000000) // 1GB
#elif defined(CFG_M8_DDR3_2GB)
	#define PHYS_MEMORY_SIZE     (0x80000000) // 2GB
#elif defined(CFG_M8_DDR3_4GB)
	#define PHYS_MEMORY_SIZE     (0x100000000) // 4GB ??
#else
	#error "Please define DDR3 memory capacity in file m8_k200_v1.h\n"
#endif

#define CONFIG_SYS_MEMTEST_START      0x10000000  /* memtest works on */      
#define CONFIG_SYS_MEMTEST_END        0x07000000  /* 0 ... 120 MB in DRAM */  
#define CONFIG_ENABLE_MEM_DEVICE_TEST 1
#define CONFIG_NR_DRAM_BANKS	      1	          /* CS1 may or may not be populated */

/* Pass open firmware flat tree*/
#define CONFIG_OF_LIBFDT	1
#define CONFIG_SYS_BOOTMAPSZ   PHYS_MEMORY_SIZE       /* Initial Memory map for Linux */
#define CONFIG_ANDROID_IMG	1

#define CONFIG_CMD_IMGPACK 1
//M8 security boot
//#define CONFIG_M8_SECU_BOOT	1

//M8 L1 cache enable for uboot decompress speed up
//#define CONFIG_AML_SPL_L1_CACHE_ON	1


/*-----------------------------------------------------------------------
 * power down
 */
//#define CONFIG_CMD_RUNARC 1 /* runarc */
#define CONFIG_AML_SUSPEND 1



/*
* CPU switch test for uboot
*/
//#define CONFIG_M8_TEST_CPU_SWITCH 1

#endif //__CONFIG_M8_K200_V1_H__
