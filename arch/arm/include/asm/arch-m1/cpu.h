/*

 *
 */

#ifndef _CPU_H
#define _CPU_H
#if CONFIG_AML_M1==0
#error please define CONFIG_AML_MEASON_1
#endif

/** Internal storage setting **/

#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_AML_M1 1
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
#define CONFIG_SYS_NAND_MAX_CHIPS	4
#endif

#ifdef CONFIG_CMD_SF
#define CONFIG_AMLOGIC_SPI_FLASH    1
#define CONFIG_SPI_FLASH            1
#define SPI_FLASH_CACHELINE         64 //amlogic special setting. in M1 , SPI_A for SPI flash
#define CONFIG_SPI_FLASH_MACRONIX   1
//#define CONFIG_SPI_FLASH_SPANSION   1
#define CONFIG_SPI_FLASH_SST        1
//#define CONFIG_SPI_FLASH_STMICRO    1
#define CONFIG_SPI_FLASH_WINBOND    1

#endif

#if CONFIG_NAND_AML_M1 || CONFIG_AMLOGIC_SPI_FLASH
#define CONFIG_MTD_DEVICE     1
#define CONFIG_MTD_PARTITIONS 1
#define CONFIG_CMD_MTDPARTS   1
#endif

#define CONFIG_AML_ROMBOOT    1
#define SPI_MEM_BASE                                0x40000000
#define AHB_SRAM_BASE                               0x49000000  // AHB-SRAM-BASE


#ifdef CONFIG_AML_ROMBOOT_SPL
#define SPL_STATIC_FUNC     static
#define SPL_STATIC_VAR      static
#else
#define SPL_STATIC_FUNC     
#define SPL_STATIC_VAR      
#endif
#endif /* _CPU_H */
