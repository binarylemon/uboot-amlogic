
/***************************************************************************
  *  Author:  Wenwu Zhang <wenwu.zhang@amlogic.com>
  *
  *
  * Comment: headder file for board WA-AML8726-M3_REF_V1.0.pdf
  *
  *
  * Remark: copy from m3_socket.h by Hisun Bao 2011.08.05
  * 
  ***************************************************************************
  */

#ifndef __M3_WA_REF_V1_H__
#define __M3_WA_REF_V1_H__

/*board ID*/
/*@WA-AML8726-M3_REF_V1.0.pdf*/
#define WA_AML8726_M3_REF_V10 1

//UART Sectoion
#define CONFIG_CONS_INDEX   2


/*@WA-AML8726-M3_REF_V1.0.pdf*/
#define CONFIG_AML_I2C      1


/*use AO I2C to access ACT8942 for verify*/
#ifdef CONFIG_AML_I2C
  /*board has AO I2C connection*/
  #ifndef HAS_AO_MODULE
    #define HAS_AO_MODULE    1
  #endif /*HAS_AO_MODULE*/
#endif /*CONFIG_AML_I2C*/


//Enable storage devices
#define CONFIG_CMD_NAND  1
#define CONFIG_CMD_SF        1


#if defined(CONFIG_CMD_SF)
  #define CONFIG_AML_MESON_3 1
  #define SPI_WRITE_PROTECT  1
  #define CONFIG_CMD_MEMORY  1
#endif /*CONFIG_CMD_SF*/

//Amlogic SARADC support
#define CONFIG_SARADC    1

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

#endif /*__M3_WA_REF_V1_H__*/
