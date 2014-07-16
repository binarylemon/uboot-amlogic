/*

 *
 */

#ifndef _CPU_H
#define _CPU_H
#include <config.h>
#include <asm/plat-cpu.h>
#if CONFIG_AML_MESON==0
#error please define CONFIG_AML_MESON
#endif
//U boot code control

//DDR channel setting
#define CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12			0
#define CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8			1 //don't use this one
#define CFG_DDR_TWO_CHANNEL_DDR0_LOW				2
#define CFG_DDR_TWO_CHANNEL_DDR1_LOW				3
#define CFG_DDR_ONE_CHANNEL_DDR0_ONLY				4
#define CFG_DDR_ONE_CHANNEL_DDR1_ONLY				5

//DDR mode. Once considering this value stored in efuse, 0=NotSet is necessary
#define CFG_DDR_BUS_WIDTH_NOT_SET					0
#define CFG_DDR_BUS_WIDTH_32BIT						1
//#define CFG_DDR_BUS_WIDTH_16BIT_LANE02			2	//DDR lane0+lane2 //m8m2 doesn't support this mode.
#define CFG_DDR_BUS_WIDTH_16BIT_LANE01				2 //3	//DDR lane0+lane1
//following 2 defines used for dtar calculate
#define CONFIG_DDR_BIT_MODE_32BIT					0
#define CONFIG_DDR_BIT_MODE_16BIT					1

//DDR bank setting
#define CFG_DDR_BANK_SET_2_BANKS_SWITCH_BIT12		0x0
#define CFG_DDR_BANK_SET_4_BANKS_SWITCH_BIT12_13	0x1
#define CFG_DDR_BANK_SET_2_BANKS_SWITCH_BIT8		0x2
#define CFG_DDR_BANK_SET_4_BANKS_SWITCH_BIT8_9		0x3

//check necessary defines
#if !defined(CONFIG_DDR_CHANNEL)
	#define CONFIG_DDR_CHANNEL CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12
#endif //CONFIG_DDR_CHANNEL
#if !defined(CONFIG_DDR_BANK_SET)
	#define CONFIG_DDR_BANK_SET CFG_DDR_BANK_SET_4_BANKS_SWITCH_BIT12_13
#endif //CONFIG_DDR_BANK_SET

#if (CONFIG_DDR_CHANNEL == CFG_DDR_ONE_CHANNEL_DDR0_ONLY)
	#define CONFIG_DDR_CHANNEL_SETTING				0x1
#elif (CONFIG_DDR_CHANNEL == CFG_DDR_ONE_CHANNEL_DDR1_ONLY)
	#define CONFIG_DDR_CHANNEL_SETTING				0x2
#else
	#define CONFIG_DDR_CHANNEL_SETTING				0x3 //2 channels enabled
#endif

//DDR training address, DO NOT modify
#if (CONFIG_DDR_CHANNEL == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(1<<12)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x0) //for dmc_ctrl bit[27:26]
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0) //for dmc_ctrl bit24
#elif (CONFIG_DDR_CHANNEL == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(1<<8)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x1)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#elif (CONFIG_DDR_CHANNEL == CFG_DDR_TWO_CHANNEL_DDR0_LOW)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(PHYS_MEMORY_SIZE / 2)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x2)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#elif (CONFIG_DDR_CHANNEL == CFG_DDR_TWO_CHANNEL_DDR1_LOW)
	#define DDR0_DTAR_ADDR_OFFSET					(PHYS_MEMORY_SIZE / 2)
	#define DDR1_DTAR_ADDR_OFFSET					(0-PHYS_MEMORY_SIZE/2)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x2)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(1)
#elif (CONFIG_DDR_CHANNEL == CFG_DDR_ONE_CHANNEL_DDR0_ONLY)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(0)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x3)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#else	//CFG_DDR_ONE_CHANNEL_DDR1_ONLY
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(0)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x3)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(1)
#endif
//128Bytes each channel. COL[2:0] must be b'000. In addition of 128Bytes, set DTAR ADDR[6:0]=b'00,0000 for safe.
//Please make sure DTAR ROW[10]!=1. Or please use MPR mode for pctl init. use 0x3fffef80 for test.
#define CONFIG_DDR0_DTAR_ADDR (0x3000000 + DDR0_DTAR_ADDR_OFFSET)	/*don't modify*/
#define CONFIG_DDR1_DTAR_ADDR (CONFIG_DDR0_DTAR_ADDR) + DDR1_DTAR_ADDR_OFFSET

#if (CONFIG_DDR_MODE > CFG_DDR_BUS_WIDTH_32BIT)	//not 32bit mode
	#define CONFIG_DDR_BIT_MODE (CONFIG_DDR_BIT_MODE_16BIT)
#else
	#define CONFIG_DDR_BIT_MODE (CONFIG_DDR_BIT_MODE_32BIT)
#endif

#define DDR_32BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set) \
/*get bank[0] or bank[1:0]*/	((((addr)>>(2+(((bank_set>>1)&0x1)?(6):(10+((channel_set>0)?0:1)))))&((bank_set&0x1)?(0x3):(0x1))) | \
/*get bank[2] or bank[2:1]*/	(((addr>>(2+row_bits+col_bits+((bank_set&0x1)?(2):(1))+((channel_set>0)?0:1)))&((bank_set&0x1)?(0x1):(0x3)))<<((bank_set&0x1)?(2):(1))))
#define DDR_32BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set) \
/*get row[row_bits-1:0]*/	(((addr)>>(2+col_bits+((channel_set>0)?0:1)+((bank_set&0x1)?(2):(1)))&((1<<row_bits)-1)))
#define DDR_32BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set) \
					(((bank_set>>1)&0x1) ? \
/*get col[5:0]*/	((((addr)>>2)&(0x3f)) | \
						((channel_set>0) ? \
	/* | col[9:6]*/				((((addr)>>(2+6+((bank_set&0x1)?(2):(1))))&(0xf))<<6) : \
	/* | col[9:6]*/				((((addr)>>(2+6+((bank_set&0x1)?(2):(1)))&((bank_set&0x1)?0x3:0x7))<<6)|((addr)>>(2+6+5)&((bank_set&0x1)?0x3:0x1))<<((bank_set&0x1)?8:9)))) : \
/*get col[9:0]*/	(((addr)>>2)&(0x3ff)))

#define DDR_16BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set) 0
#define DDR_16BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set) 0
#define DDR_16BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set) 0

#define DDR_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set)))
#define DDR_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set)))
#define DDR_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set)))

#define CONFIG_DDR0_DTAR_DTBANK  DDR_DTAR_BANK_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR_ROW_BITS,CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR0_DTAR_DTROW   DDR_DTAR_DTROW_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR_ROW_BITS,CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR0_DTAR_DTCOL   DDR_DTAR_DTCOL_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)

#define CONFIG_DDR1_DTAR_DTBANK  DDR_DTAR_BANK_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR_ROW_BITS,CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR1_DTAR_DTROW   DDR_DTAR_DTROW_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR_ROW_BITS,CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR1_DTAR_DTCOL   DDR_DTAR_DTCOL_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//NOTE: IMPORTANT! DO NOT TRY TO MODIFY FOLLOWING CODE!
//           It is used to get DDR0 training address from PUB_DTAR0.
//
//How to fetch the DDR training address:
//           1. enable PCTL clock before access
//           2. load DMC DDR setting from P_DMC_DDR_CTRL
//           3. load the DTAR0 value from DDR0 PUB register according to the channel setting from DMC_DDR_CTRL
//           4. disable PCTL clock for power saving
//
//Demo code: 
/*
	//enable clock
	writel(readl(P_DDR0_CLK_CTRL)|(1), P_DDR0_CLK_CTRL);

	printf("training address: %x\n", M8BABY_GET_DT_ADDR(readl(P_DDR0_PUB_DTAR0), readl(P_DMC_DDR_CTRL)));

	//disable clock
	writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);
*/

#if 0
/*temp functions*/
bank_set=(((dmc)>>(channel?(13):(5)))&0x3)
bit_mode=(((dmc)>>(channel?(15):(7)))&0x1)
row_bits=((((dmc)>>(channel?(10):(2)))&0x3)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16))
col_bits=((((dmc)>>(channel?(8):(0)))&0x3)+8)
channel_set=(((dmc)>>26)&0x3)
channel_set_ext=(((dmc)>>24)&0x1)
ext_addr=((channel_set==0x2)?(channel_set_ext?(channel?(0):(OTHER)):(channel?(OTHER):(0))):((channel_set==0x0)?(channel?(1<<12):(0)):((channel_set==0x1)?(1<<8):(0))))
mem_size_of_current_channel=(1<<(((((dmc)>>(channel?(10):(2)))&0x3)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16))+((((dmc)>>(channel?(8):(0)))&0x3)+8)+5))
mem_size_of_another_channel=(1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5))
ext_addr=(((((dmc)>>26)&0x3)==0x2)?((((dmc)>>24)&0x1)?(channel?(0):(1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5)):(channel?(1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5)):(0)):(0)):(0)))
#else
/*bank1*/
#define GET_32BIT_DT_ADDR_BANK1(dtar, dmc, channel) (((((dtar)>>28)&0x7)&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(0x3):(0x1)))<<((2)+((((((dmc)>>(channel?(13):(5)))&0x3)>>1)&0x1)?(6):(10+(((((dmc)>>26)&0x3)>0)?0:1)))))
/*bank2*/
#define GET_32BIT_DT_ADDR_BANK2(dtar, dmc, channel) ((((((dtar)>>28)&0x7)>>(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1)))&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(0x1):(0x3)))<<((2)+(((((dmc)>>(channel?(10):(2)))&0x3)>0)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16))+((((dmc)>>(channel?(8):(0)))&0x3)+8)+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))+(((((dmc)>>26)&0x3)>0)?0:1)))
/*row  */
#define GET_32BIT_DT_ADDR_ROW(dtar, dmc, channel) (((((dtar)>>12)&0xffff)&((1<<((((dmc)>>(channel?(10):(2)))&0x3)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16)))-1))<<((2)+((((dmc)>>(channel?(8):(0)))&0x3)+8)+(((((dmc)>>26)&0x3)>0)?0:1)+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))))
/*col  */
#define GET_32BIT_DT_ADDR_COL(dtar, dmc, channel)   ((((((dmc)>>(channel?(13):(5)))&0x3)>>1)&0x1)?(((((dtar)&0xfff)&0x3f)<<(2))|(((((dmc)>>26)&0x3)>0)?(((((dtar)&0xfff)>>6)&0xf)<<(2+6+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1)))):((((((dtar)&0xfff)>>6)&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?0x3:0x7))<<(2+6+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))))|(((((dtar)&0xfff)>>(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?8:9))&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?0x3:0x1))<<(2+6+5))))):((((dtar)&0xfff)&0x3ff)<<(2)))
/*ext_addr*/
#define GET_32BIT_DT_ADDR_EXT(dtar, dmc, channel) (((((dmc)>>26)&0x3)==0x2)?((((dmc)>>24)&0x1)?(channel?(0):((1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5)))):(channel?((1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5))):(0))):(((((dmc)>>26)&0x3)==0x0)?(channel?(1<<12):(0)):(((((dmc)>>26)&0x3)==0x1)?(1<<8):(0))))
#define GET_32BIT_DT_ADDR(dtar, dmc, channel) \
			((GET_32BIT_DT_ADDR_BANK1(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_BANK2(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_ROW(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_COL(dtar, dmc, channel)) + \
			GET_32BIT_DT_ADDR_EXT(dtar, dmc, channel))

#define GET_16BIT_DT_ADDR_BANK1(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_BANK2(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_ROW(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_COL(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_EXT(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR(dtar, dmc, channel) \
			((GET_16BIT_DT_ADDR_BANK1(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_BANK2(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_ROW(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_COL(dtar, dmc, channel)) + \
			GET_16BIT_DT_ADDR_EXT(dtar, dmc, channel))

#define GET_DT_ADDR(dtar, dmc, channel) \
			(((dmc>>(channel?15:7))&0x1)?(GET_16BIT_DT_ADDR(dtar, dmc, channel)):(GET_32BIT_DT_ADDR(dtar, dmc, channel)))
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//timer
#define CONFIG_SYS_HZ 1000

#define CONFIG_SYS_NO_FLASH 1
#define CONFIG_NR_DRAM_BANKS 1

#define CONFIG_BAUDRATE                 115200
#define CONFIG_SYS_BAUDRATE_TABLE       { 9600, 19200, 38400, 57600, 115200}
#define CONFIG_SERIAL_MULTI             1

#if 0
//no need to keep but for develop & verify
#define CONFIG_SYS_SDRAM_BASE   0x80000000
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE+0xF00000)
#define CONFIG_SYS_TEXT_BASE    0x8F800000
#define CONFIG_MMU_DDR_SIZE     (0xc00)
#define CONFIG_SYS_LOAD_ADDR    0x82000000
#define CONFIG_DTB_LOAD_ADDR    0x83000000
#else
#define CONFIG_SYS_SDRAM_BASE   0x00000000
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE+0xF00000)
#define CONFIG_SYS_TEXT_BASE    0x10000000
#define CONFIG_MMU_DDR_SIZE     ((PHYS_MEMORY_SIZE)>>20)
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
#undef CONFIG_MMU_DDR_SIZE
#define CONFIG_MMU_DDR_SIZE    ((0x80000000)>>20)	//max 2GB
#endif
#define CONFIG_SYS_LOAD_ADDR    0x12000000
#define CONFIG_DTB_LOAD_ADDR    0x0f000000
#endif

#define CONFIG_SECURE_UBOOT_SIZE     0x100000

#define CONFIG_SYS_MALLOC_LEN   (12<<20)

#define CONFIG_SYS_MAXARGS      16
#define CONFIG_SYS_CBSIZE          1024
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */

#define CONFIG_VPU_PRESET		1

/** Timer relative Configuration **/
#define CONFIG_CRYSTAL_MHZ  24
/** Internal storage setting **/
//size Limitation
//#include "romboot.h"
//#warning todo implement CONFIG_BOARD_SIZE_LIMIT 
//#define CONFIG_BOARD_SIZE_LIMIT 600000
#define IO_REGION_BASE                0xe0000000
#define CONFIG_SYS_CACHE_LINE_SIZE 32
#define CONFIG_CMD_CACHE	1
//#define CONFIG_SYS_NO_CP15_CACHE	1
//#define CONFIG_DCACHE_OFF    		1
//#define CONFIG_ICACHE_OFF    		1

//#define CONFIG_EFUSE 1

#ifdef CONFIG_CMD_NAND
	#define CONFIG_NAND_AML_M3 1
	#define CONFIG_NAND_AML  1	
	#define CONFIG_NAND_AML_M8
	//#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
	#define CONFIG_SYS_NAND_MAX_CHIPS	4
	#ifndef CONFIG_NAND_SP_BLOCK_SIZE
		#define CONFIG_NAND_SP_BLOCK_SIZE 32
	#endif
	//#warning todo implement nand driver later
	#define CONFIG_SYS_MAX_NAND_DEVICE  2  //make uboot happy
	#define CONFIG_SYS_NAND_BASE_LIST   {0}//make uboot happy
	//#define CONFIG_SYS_NAND_BASE 0 //make uboot happy
#endif

#ifdef CONFIG_CMD_SF
	#define CONFIG_AMLOGIC_SPI_FLASH    1
	#define CONFIG_SPI_FLASH            1
	#define SPI_FLASH_CACHELINE         64 //amlogic special setting. in M1 , SPI_A for SPI flash
	#define CONFIG_SPI_FLASH_MACRONIX   1
	#define CONFIG_SPI_FLASH_EON        1
	#define CONFIG_SPI_FLASH_SPANSION   1
	#define CONFIG_SPI_FLASH_SST        1
	#define CONFIG_SPI_FLASH_STMICRO    1
	#define CONFIG_SPI_FLASH_WINBOND    1
	#define CONFIG_SPI_FLASH_GIGADEVICE     1
#endif


#if CONFIG_SDIO_B1 || CONFIG_SDIO_A || CONFIG_SDIO_B || CONFIG_SDIO_C
	#define CONFIG_CMD_MMC          1
	#define CONFIG_MMC              1
	#define CONFIG_DOS_PARTITION    1
	#define CONFIG_AML_SDIO         1
	#define CONFIG_GENERIC_MMC      1
#endif

#if CONFIG_NAND_AML_M3 || CONFIG_AMLOGIC_SPI_FLASH
	#define CONFIG_MTD_DEVICE     1
	#define CONFIG_MTD_PARTITIONS 1
	#define CONFIG_CMD_MTDPARTS   1
#endif

/*
 * File system
 */
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/


#define CONFIG_AML_ROMBOOT    1
#define SPI_MEM_BASE                                0xcc000000
#define AHB_SRAM_BASE                               0xd9000000  // AHB-SRAM-BASE
#define CONFIG_USB_SPL_ADDR                         (CONFIG_SYS_TEXT_BASE - (32<<10)) //here need update when support 64KB SPL
#define CONFIG_DDR_INIT_ADDR                        (0xd9000000) //usb driver limit, bit4 must 1, change 0xd9000000 as ACS hard coded to 0xd9000200

#if !defined(CONFIG_AML_DISABLE_CRYPTO_UBOOT)
	#define CONFIG_AML_SECU_BOOT_V2		1
	#define CONFIG_AML_CRYPTO_UBOOT		1
#endif //CONFIG_AML_DISABLE_CRYPTO_UBOOT


#ifdef CONFIG_AML_ROMBOOT_SPL
	#define SPL_STATIC_FUNC     static
	#define SPL_STATIC_VAR      static
#else
	#define SPL_STATIC_FUNC     
	#define SPL_STATIC_VAR      
#endif

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1
#define CONFIG_CMD_KGDB			1
////#define CONFIG_SERIAL_TAG       1*/

//#define CONFIG_AML_RTC 
//#define CONFIG_RTC_DAY_TEST 1  // test RTC run 2 days

#define CONFIG_LZMA  1
#define CONFIG_LZO
#define CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK
/*default command select*/
#define CONFIG_CMD_MEMORY	1 /* md mm nm mw cp cmp crc base loop mtest */
//support "bdinfo" 
#define CONFIG_CMD_BDI 1
//support "coninfo"
#define CONFIG_CMD_CONSOLE 1
//support "echo"
#define CONFIG_CMD_ECHO 1
//support "loadb,loads,loady"
#define CONFIG_CMD_LOADS 1
#define CONFIG_CMD_LOADB 1
//support "run"
#define CONFIG_CMD_RUN 1
//support "true,false,test"
//#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
//#define CONFIG_SYS_PROMPT		"8726M_ref # "
//#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
//support "imxtract"
#define CONFIG_CMD_XIMG 1
//support "itest"
#define CONFIG_CMD_ITEST 1
//support "sleep"
#define CONFIG_CMD_MISC 1
//support "source"
#define CONFIG_SOURCE 1
#define CONFIG_CMD_SOURCE 1
//support "editenv"
#define CONFIG_CMD_EDITENV 1
#define CONFIG_CMD_CALINFO 1
/*default command select end*/

//support gpio cmd
#define CONFIG_AML_GPIO_CMD 1
#define CONFIG_AML_GPIO 1

//max watchdog timer: 8.388s
#define AML_WATCHDOG_TIME_SLICE				128	//us
#define AML_WATCHDOG_ENABLE_OFFSET			19
#define AML_WATCHDOG_CPU_RESET_CNTL			0xf	//qual-core
#define AML_WATCHDOG_CPU_RESET_OFFSET		24

#define MESON_CPU_TYPE	MESON_CPU_TYPE_MESON8M2
#define CONFIG_AML_MESON_8		1
#define CONFIG_AML_SMP

#endif /* _CPU_H */
