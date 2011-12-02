
/***************************************************************************
  *  Author:  Wenwu Zhang <wenwu.zhang@amlogic.com>
  *
  *
  * Comment: headder file for board Q07CL_DSN_RB_0922A.pdf
  *
  *
  * Remark: copy from m3_wa_ref_v1.h by Ada Wang 2011.08.05
  * 
  ***************************************************************************
  */

#ifndef __M3_SHUTTLE_MID_V1_H__
#define __M3_SHUTTLE_MID_V1_H__

/*board ID*/
/*@Q07CL_DSN_RB_0922A.pdf*/
#define SHUTTLE_M3_MID_V1 1

//UART Sectoion
#define CONFIG_CONS_INDEX   2

//support "boot,bootd"
//#define CONFIG_CMD_BOOTD 1
#define CONFIG_SWITCH_BOOT_MODE
#define AMLOGIC_REBOOT_MODE

#define CONFIG_CMD_AUTOSCRIPT
#define CONFIG_AML_AUTOSCRIPT
#define AML_AUTOSCRIPT  "aml_autoscript"
#define SCAN_MMC_PARTITION  4

/*@Q07CL_DSN_RB_0922A.pdf*/
#define CONFIG_AML_I2C      1
#define CONFIG_PMU_ACT8942

/*use AO I2C to access ACT8942 for verify*/
#ifdef CONFIG_AML_I2C
  /*board has AO I2C connection*/
  #ifndef HAS_AO_MODULE
    #define HAS_AO_MODULE    1
  #endif /*HAS_AO_MODULE*/
#endif /*CONFIG_AML_I2C*/


//Enable storage devices
// M3_shuttle not support NAND
//#ifndef CONFIG_JERRY_NAND_TEST
//#define CONFIG_CMD_NAND  1
//#endif

//M3 shuttle MID not support SPI nor flash
//#define CONFIG_CMD_SF        1


#if defined(CONFIG_CMD_SF)
  #define CONFIG_AML_MESON_3 1
  #define SPI_WRITE_PROTECT  1
  #define CONFIG_CMD_MEMORY  1
#endif /*CONFIG_CMD_SF*/


/* USB
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
/* #define CONFIG_MUSB_UDC		1 */
#define CONFIG_M3_USBPORT_BASE	0xC90C0000
#define CONFIG_USB_STORAGE
#define CONFIG_USB_DWC_OTG_HCD
#define CONFIG_CMD_USB 1


//Amlogic SARADC support
#define CONFIG_SARADC    1
#define CONFIG_EFUSE 1
//#define CONFIG_L2_OFF			1
#define SCAN_DEVICE_PARTITION	4

/*board WA_AML8726_M3_REF_V1.0 NOT support Ethernet*/

#define CONFIG_SDIO_B1   1
#define CONFIG_SDIO_A    1
#define CONFIG_SDIO_B    1
#define CONFIG_SDIO_C    1
#define CONFIG_ENABLE_EXT_DEVICE_RETRY 1

#define IO_REGION_BASE      0xe0000000
#define CONFIG_MMU          1
#define CONFIG_PAGE_OFFSET 	0xc0000000
#define CONFIG_SYS_LONGHELP	1

#define CONFIG_MEMSIZE	512	/*unit is MB*/ 

#if(CONFIG_MEMSIZE == 512)
  #define BOARD_INFO_ENV  " mem=512M"
  #define UBOOTPATH		  "u-boot-512M-UartB.bin"
#else
  #define BOARD_INFO_ENV  ""
  #define UBOOTPATH		  "u-boot-aml.bin"
#endif

#define CONFIG_UCL 1
#define CONFIG_SELF_COMPRESS 

#define CONFIG_IMPROVE_UCL_DEC    1
#if CONFIG_IMPROVE_UCL_DEC
#define UCL_DEC_EN_IDCACHE        1
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
#endif
#define AML_CARD_SD_INFO_DETAILED

//#define CONFIG_UBI_SUPPORT
#ifdef	CONFIG_UBI_SUPPORT
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define MTDIDS_DEFAULT		"nand1=nandflash1\0"
#define MTDPARTS_DEFAULT	"mtdparts=nandflash1:256m@168m(system)\0"						
#endif

/* Environment information */
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE		uImage

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"testaddr=0x82400000\0" \
	"usbtty=cdc_acm\0" \
	"console=ttyS2,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
	"boardname=m1_mbox\0" \
	"chipname=8726m\0" \
	"machid=B8E\0" \
	"bootargs=init=/init console=ttyS0,115200n8 nohlt a9_clk=600M clk81=192M hdmitx=vdacoff,powermode1,unplug_powerdown mem=512m logo=osd1,0x84100000,lcd,full\0" \
	"bootloader_start=0\0" \
	"bootloader_size=60000\0" \
	"bootloader_path=u-boot.bin\0" \
	"normal_name=boot\0" \
	"normal_start=0x6000\0" \
	"normal_size=0x2000\0" \
	"recovery_name=recovery\0" \
	"recovery_start=0x3800\0" \
	"recovery_size=0x2800\0" \
	"recovery_path=uImage_recovery\0" \
	"logo_name=logo\0" \
	"logo_start=0x800\0" \
	"logo_size=0x1800\0" \
	"aml_logo_name=aml_logo\0" \
	"aml_logo_start=0x2000\0" \
	"aml_logo_size=0xc00\0"	

#define CONFIG_BOOTCOMMAND  "mmc read 1 84100000 ${logo_start} ${logo_size};mmc read 1 ${loadaddr} ${normal_start} ${normal_size};bootm"

#define CONFIG_AUTO_COMPLETE	1

//#define CONFIG_SPI_BOOT 1
//#ifndef CONFIG_JERRY_NAND_TEST
//#define CONFIG_NAND_BOOT 1
//#endif
#define CONFIG_EMMC_BOOT 1

#ifdef CONFIG_NAND_BOOT
#define CONFIG_AMLROM_NANDBOOT 1
#endif 

#ifdef CONFIG_SPI_BOOT
	#define CONFIG_ENV_OVERWRITE
	#define CONFIG_ENV_IS_IN_SPI_FLASH
	#define CONFIG_CMD_SAVEENV	
	#define CONFIG_ENV_SECT_SIZE        0x1000
	#define CONFIG_ENV_OFFSET           0x1f0000
#elif defined CONFIG_NAND_BOOT
	#define CONFIG_ENV_IS_IN_AML_NAND
	#define CONFIG_CMD_SAVEENV
	#define CONFIG_ENV_OVERWRITE	
	#define CONFIG_ENV_OFFSET       0x400000
	#define CONFIG_ENV_BLOCK_NUM    2
#elif defined CONFIG_EMMC_BOOT
	#define CONFIG_ENV_IS_IN_EMMC
	#define CONFIG_CMD_SAVEENV
    #define CONFIG_ENV_DEVICE_ID        1	
	#define CONFIG_ENV_OFFSET       0x60000	
#else
#define CONFIG_ENV_IS_NOWHERE    1
#endif
/* Monitor at start of flash */


/* config LCD output */ 
#define CONFIG_VIDEO_AML
#define CONFIG_VIDEO_AMLLCD
//#define CONFIG_VIDEO_AMLLCD_M3
#define CONFIG_CMD_BMP
#define LCD_BPP LCD_COLOR24
#define CURRENT_OSD OSD2
#define LCD_TEST_PATTERN
#ifndef CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#endif
/*end config LCD output*/

#define OSD_WIDTH   800
#define OSD_HEIGHT  480
#define OSD_BPP     OSD_COLOR24

#define CONFIG_AML_FONT
#define ENABLE_FONT_RESOURCE
#ifdef ENABLE_FONT_RESOURCE
#define DEFAULT_FONT &ISO_88591_32Font
#endif


/*POST support*/
#define CONFIG_POST (CONFIG_SYS_POST_MEMORY | CONFIG_SYS_POST_CACHE | \
										CONFIG_SYS_POST_BSPEC1 |  \
										CONFIG_SYS_POST_RTC | CONFIG_SYS_POST_ADC | \
										CONFIG_SYS_POST_PLL)										
										
#ifdef CONFIG_POST
#define CONFIG_POST_AML
#define CONFIG_POST_ALT_LIST
#ifndef CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_IS_IN_ENV  /* Otherwise it catches logbuffer as output */
#endif
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

//power down
//#define CONFIG_ENABLE_ARC //trunk def
#define CONFIG_CMD_RUNARC 1 /* runarc */
#define CONFIG_AML_SUSPEND 1

#define CONFIG_VOLTAGE_AO12  1
#define CONFIG_VOLTAGE_DDR15 1

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS   1              /* CS1 may or may not be populated */
#define PHYS_MEMORY_START      0x80000000     // from 500000
#if(CONFIG_MEMSIZE == 128)
  #define PHYS_MEMORY_SIZE     0x8000000      // 128M
#elif(CONFIG_MEMSIZE == 256)
  #define CONFIG_DDR_TYPE      DDR_K4T1G164QE //128M/PCS DDR
  #define PHYS_MEMORY_SIZE     0x10000000     // 256M
#elif(CONFIG_MEMSIZE == 512)
  #define CONFIG_DDR_TYPE      DDR_W972GG6JB  //256M/PCS DDR
  #define PHYS_MEMORY_SIZE     0x20000000     // 512M
#else
  #ERROR: Must config CONFIG_MEMSIZE
#endif

#define CONFIG_SYS_MEMTEST_START	0x80000000  /* memtest works on	*/      
#define CONFIG_SYS_MEMTEST_END		0x87000000  /* 0 ... 120 MB in DRAM	*/  

#endif /*__M3_SHUTTLE_MID_V1_H__*/
