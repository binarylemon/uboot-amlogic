#ifndef __CONFIG_M3_SOCKET_H_
#define __CONFIG_M3_SOCKET_H_

#define M3_SKT_V1_20110622 1

//UART Sectoion
#define CONFIG_CONS_INDEX   2


//#define CONFIG_AML_I2C      1

//Enable storage devices
//#define CONFIG_CMD_NAND  1
#define CONFIG_CMD_SF    1

#if defined(CONFIG_CMD_SF)
#define CONFIG_AML_MESON_3 1
#define SPI_WRITE_PROTECT  1
#define CONFIG_CMD_MEMORY  1
#endif /*CONFIG_CMD_SF*/

//Amlogic SARADC support
#define CONFIG_SARADC 1

#define CONFIG_CMD_NET   1

#if defined(CONFIG_CMD_NET)
#define CONFIG_AML_ETHERNET 1
#define CONFIG_NET_MULTI 1
#define CONFIG_CMD_PING 1
#define CONFIG_CMD_DHCP 1
#define CONFIG_CMD_RARP 1

#define CONFIG_AML_ETHERNET    1                   /*to link /driver/net/aml_ethernet.c*/
#define CONFIG_HOSTNAME        arm_m1
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


#define IO_REGION_BASE                0xe0000000
#define CONFIG_MMU                    1
#define CONFIG_PAGE_OFFSET 	0xc0000000
#define CONFIG_SYS_LONGHELP	1

#define CONFIG_MEMSIZE	512	/*unit is MB*/ 
#if(CONFIG_MEMSIZE == 512)
#define BOARD_INFO_ENV  " mem=512M"
#define UBOOTPATH		"u-boot-512M-UartB.bin"
#else
#define BOARD_INFO_ENV ""
#define UBOOTPATH		"u-boot-aml.bin"
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	/* CS1 may or may not be populated */
#define PHYS_MEMORY_START    0x80000000 // from 500000
#if(CONFIG_MEMSIZE == 128)
#define PHYS_MEMORY_SIZE      0x8000000 // 128M
#elif(CONFIG_MEMSIZE == 256)
#define CONFIG_DDR_TYPE DDR_K4T1G164QE //128M/PCS DDR
#define PHYS_MEMORY_SIZE     0x10000000 // 256M
#elif(CONFIG_MEMSIZE == 512)
#define CONFIG_DDR_TYPE DDR_W972GG6JB	//256M/PCS DDR
#define PHYS_MEMORY_SIZE     0x20000000 // 512M
#else
#ERROR: Must config CONFIG_MEMSIZE
#endif
#define CONFIG_SYS_MEMTEST_START	0x80000000	/* memtest works on	*/      
#define CONFIG_SYS_MEMTEST_END		0x87000000	/* 0 ... 120 MB in DRAM	*/  

/*-----------------------------------------------------------------------
 * define commands 
 */
#define CONFIG_CMD_LOADB	1 /* loadb			*/
#define CONFIG_CMD_LOADS	1 /* loads			*/
#define CONFIG_CMD_MEMORY	1 /* md mm nm mw cp cmp crc base loop mtest */

#define CONFIG_CMD_RUNARC 1 /* runarc */

#define CONFIG_AML_RTC
#endif
