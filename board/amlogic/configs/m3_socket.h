#ifndef __CONFIG_M3_SOCKET_H_
#define __CONFIG_M3_SOCKET_H_


//UART Sectoion
#define CONFIG_CONS_INDEX   1

//Enable storage devices
//#define CONFIG_CMD_NAND  1
//#define CONFIG_CMD_SF    1

#define CONFIG_SDIO_B1   1
#define CONFIG_SDIO_A    1
#define CONFIG_SDIO_B    1
#define CONFIG_SDIO_C    1
#define CONFIG_ENABLE_EXT_DEVICE_RETRY 1


#define IO_REGION_BASE                0xe0000000
#define CONFIG_MMU                    1
#define CONFIG_PAGE_OFFSET 	0xc0000000
#define CONFIG_SYS_LONGHELP	1

#define CONFIG_SYS_CACHE_LINE_SIZE 32
#define CONFIG_CMD_CACHE	1
//#define CONFIG_L2_OFF
#ifndef CONFIG_L2_OFF
#define CONFIG_CACHE_L2X0       1
#endif

//#define CONFIG_DCACHE_OFF    
//#define CONFIG_ICACHE_OFF    
#ifndef CONFIG_DCACHE_OFF
#define CONFIG_CACHE_ENABLE     1
#else
#ifndef CONFIG_ICACHE_OFF
#define CONFIG_CACHE_ENABLE     1
#endif
#endif

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

#endif
