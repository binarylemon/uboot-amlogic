/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include <asm/cache.h>

#include <asm/arch/reboot.h>
#include <partition_table.h>


#ifdef CONFIG_AML_RTC
#include <aml_rtc.h>
#endif

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

#ifdef CONFIG_POST
#include <post.h>
#endif
#ifdef CONFIG_LOGBUFFER
#include <logbuff.h>
#endif

#if defined(CONFIG_AML_V2_USBTOOL)
#include <amlogic/aml_v2_burning.h>
#endif// #if defined(CONFIG_AML_V2_USBTOOL)

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

#ifdef CONFIG_VIDEO_AMLLCD
extern int aml_lcd_init(void);
#endif

// Pre-clear hdmi hdcp ksv ram
extern int hdmi_hdcp_clear_ksv_ram(void);

#if defined(AML_UBOOT_LOG_PROFILE)
int __g_nTE1_4BC722B3__ = 0 ;
int __g_nTE2_4BC722B3__ = 0 ;
int __g_nTEFlag_4BC722B3__ = 0;
int __g_nTStep_4BC722B3__ = 0;
#endif //AML_UBOOT_LOG_PROFILE

/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
void inline __coloured_LED_init (void) {}
void coloured_LED_init (void) __attribute__((weak, alias("__coloured_LED_init")));
void inline __red_LED_on (void) {}
void red_LED_on (void) __attribute__((weak, alias("__red_LED_on")));
void inline __red_LED_off(void) {}
void red_LED_off(void) __attribute__((weak, alias("__red_LED_off")));
void inline __green_LED_on(void) {}
void green_LED_on(void) __attribute__((weak, alias("__green_LED_on")));
void inline __green_LED_off(void) {}
void green_LED_off(void) __attribute__((weak, alias("__green_LED_off")));
void inline __yellow_LED_on(void) {}
void yellow_LED_on(void) __attribute__((weak, alias("__yellow_LED_on")));
void inline __yellow_LED_off(void) {}
void yellow_LED_off(void) __attribute__((weak, alias("__yellow_LED_off")));
void inline __blue_LED_on(void) {}
void blue_LED_on(void) __attribute__((weak, alias("__blue_LED_on")));
void inline __blue_LED_off(void) {}
void blue_LED_off(void) __attribute__((weak, alias("__blue_LED_off")));

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif
static int init_baudrate (void)
{
#if !defined (CONFIG_VLSI_EMULATOR)
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_f("baudrate", tmp, sizeof (tmp));
	gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;
#else
	gd->baudrate = 1930;
#endif
	return (0);
}

static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _TEXT_BASE,
	       _bss_start_ofs+_TEXT_BASE, _bss_end_ofs+_TEXT_BASE);
#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void);

void __dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}
void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

#ifdef CONFIG_AML_EFUSE_INIT_PLUS
extern int efuse_aml_init_plus(void);
#endif //CONFIG_AML_EFUSE_INIT_PLUS

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	timer_init,		/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
#if !defined (CONFIG_VLSI_EMULATOR)
	//serial_init,		/* serial communications setup */
#endif //#if !defined (CONFIG_VLSI_EMULATOR)
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
#ifdef CONFIG_AML_RTC
	aml_rtc_init,
#endif
	dram_init,		/* configure available RAM banks */
#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
	arm_pci_init,
#endif
#if !defined(CONFIG_M3)
   hdmi_hdcp_clear_ksv_ram,
#endif
#ifdef CONFIG_AML_EFUSE_INIT_PLUS
    efuse_aml_init_plus,
#endif //CONFIG_AML_EFUSE_INIT_PLUS
	NULL,
};

#ifdef CONFIG_M8B
extern void init_neon(void);
#endif

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
#include <asm/arch/timer.h>
unsigned int spl_boot_end,lib_board_init_f_start,lib_board_init_f_end;
unsigned int lib_board_init_r_start,main_loop_start;
#endif
extern ulong __mmu_table;
void board_init_f (ulong bootflag)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd_t *id;
	ulong addr, addr_sp;

#ifdef CONFIG_M8B
	init_neon();
#endif
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));

	//gd->mon_len = _bss_end_ofs;
	gd->mon_len = _uboot_end_ofs;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	debug ("monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug ("ramsize: %08lX\n", gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif

	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;

#ifdef CONFIG_AML_SUSPEND
//init_suspend_firmware() function (used in main.c) will use 0x9ff00000 buffer
	if(addr > 0x9FF00000)
		addr = 0x9FF00000;
#endif
#ifdef CONFIG_AML_SECURE
//init_secure_firmware() function (used in main.c) will use 0x9fe00000 buffer
	if(addr > 0x9FE00000)
		addr = 0x9FE00000;
#endif

#ifdef AUDIO_DSP_START_PHY_ADDR
//arc run vsync code in 0x9FD00000 space,when kernel is kernel3.10,size is 1Mbyte
	#if (AUDIO_DSP_START_PHY_ADDR == 0x9FD00000)
	if(addr > 0x9FD00000)
		addr = 0x9FD00000;
	#endif
#endif


#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
	/* reserve kernel log buffer */
	addr -= (LOGBUFF_RESERVE);
	debug ("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN, addr);
#endif
#endif

#ifdef CONFIG_PRAM
	/*
	 * reserve protected RAM
	 */
	i = getenv_r ("pram", (char *)tmp, sizeof (tmp));
	reg = (i > 0) ? simple_strtoul ((const char *)tmp, NULL, 10) : CONFIG_PRAM;
	addr -= (reg << 10);		/* size is in kB */
	debug ("Reserving %ldk for protected RAM at %08lx\n", reg, addr);
#endif /* CONFIG_PRAM */

#if !(defined(CONFIG_ICACHE_OFF) && defined(CONFIG_DCACHE_OFF))
	/* reserve TLB table */
	addr -= (4096 * 4);

	/* round down to next 64 kB limit */
	addr &= ~(0x10000 - 1);

//	gd->tlb_addr = addr;
//	debug ("TLB table at: %08lx\n", addr);
#endif

	/* round down to next 4 kB limit */
	addr &= ~(4096 - 1);
	debug ("Top of RAM usable for U-Boot at: %08lx\n", addr);

#ifdef CONFIG_VFD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	addr -= vfd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_VFD */

#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO_AMLLCD)
	/* reserve memory for LCD display (always full pages) */
	addr = lcd_setmem (addr);
	gd->fb_base = addr;
	debug("fb_base=%x\n", gd->fb_base);
#endif /* CONFIG_LCD */

	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	addr -= gd->mon_len;
	//addr &= ~(4096 - 1);
	// relocate code include mmu table, mmu table start address need 14bits alignment
	// so relocate code start code align to 14bits to set correct mmu table start addr
	//addr &= ~(0x10000 - 1);  // round down to 64kB is ok
	addr -= ((__mmu_table + addr - _TEXT_BASE)&((1024 * 16 - 1)));

	debug ("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10, addr);

#ifndef CONFIG_PRELOADER
	/*
	 * reserve memory for malloc() arena
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	debug ("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof (bd_t);
	bd = (bd_t *) addr_sp;
	gd->bd = bd;
	debug ("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof (bd_t), addr_sp);
	addr_sp -= sizeof (gd_t);
	id = (gd_t *) addr_sp;
	debug ("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof (gd_t), addr_sp);

	/* setup stackpointer for exeptions */
	gd->irq_sp = addr_sp;
#ifdef CONFIG_USE_IRQ
	addr_sp -= (CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ);
	debug ("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ, addr_sp);
#endif
	/* leave 3 words for abort-stack    */
	addr_sp -= 3;

	/* 8-byte alignment for ABI compliance */
	addr_sp &= ~0x07;
#else
	addr_sp += 128;	/* leave 32 words for abort-stack   */
	gd->irq_sp = addr_sp;
#endif

	debug ("New Stack Pointer is: %08lx\n", addr_sp);

#ifdef CONFIG_POST
	post_bootmode_init();
	post_run (NULL, POST_ROM | post_bootmode_get(0));
#endif

	gd->bd->bi_baudrate = gd->baudrate;
	/* Ram ist board specific, so move it to board code ... */
	dram_init_banksize();
	display_dram_config();	/* and display it */

	gd->relocaddr = addr;
	gd->start_addr_sp = addr_sp;
	gd->reloc_off = addr - _TEXT_BASE;

#if !(defined(CONFIG_ICACHE_OFF) && defined(CONFIG_DCACHE_OFF))
	/* adjust mmu table address */
	gd->tlb_addr = __mmu_table + gd->reloc_off;
  debug ("TLB table at: %08lx\n", gd->tlb_addr);
#endif

	printf ("relocation Offset is: %08lx\n", gd->reloc_off);
	memcpy (id, (void *)gd, sizeof (gd_t));

	relocate_code (addr_sp, id, addr);

	/* NOTREACHED - relocate_code() does not return */
}

#if !defined(CONFIG_SYS_NO_FLASH)
static char *failed = "*** failed ***\n";
#endif
unsigned int emmc_init(void)
{
    int ret = -1;
    struct mmc *mmc = NULL;
	mmc = find_mmc_device(1);
	if (mmc) {
		ret = mmc_init(mmc); // init eMMC/tSD+
	}
	return ret;
}
#ifdef CONFIG_STORE_COMPATIBLE
extern int amlnf_init(unsigned flag);
extern int spi_env_relocate_spec(void);
extern int get_storage_device_flag(void);
void get_device_boot_flag(void)
{
	unsigned init_flag = 0;
	int ret=0;
	if(POR_NAND_BOOT()){
		printf("enter nand boot\n");
		//try nand first
		device_boot_flag = NAND_BOOT_FLAG;
		ret = amlnf_init(0x5);
		if(ret){  //failed, try spi and eMMC
			printf("NAND boot, but nand init failed\n");
			init_flag = spi_env_relocate_spec();
			if(init_flag){
				device_boot_flag = EMMC_BOOT_FLAG;
				printf("NAND boot,spi init failed\n");
			}
			else{
				device_boot_flag=SPI_BOOT_FLAG;
			}
			//try eMMC init
			ret = emmc_init();
			if(ret){
				printf("NAND boot, eMMC init failed\n");
			}

			if(ret && init_flag){
			//error case
				device_boot_flag = CARD_BOOT_FLAG;
			}
			else if(init_flag && (ret == 0)){
				device_boot_flag = EMMC_BOOT_FLAG;
			}
			else if((init_flag == 0) && ret){
				device_boot_flag = CARD_BOOT_FLAG;
			}
			else{
				device_boot_flag = SPI_EMMC_FLAG;
			}
		}
	}
	else if(POR_EMMC_BOOT()){
	//try nand first
		printf("enter emmc boot\n");
		device_boot_flag = EMMC_BOOT_FLAG;
		ret = emmc_init();
		if(ret){  //failed, try spi and eMMC
			printf("EMMC boot, but emmc init failed\n");
			init_flag = spi_env_relocate_spec();
			if(init_flag){
				device_boot_flag = NAND_BOOT_FLAG;
				printf("EMMC boot, spi init failed\n");
			}
			else{
				device_boot_flag = SPI_BOOT_FLAG;
			}

			ret = amlnf_init(0x5);
			if(ret){
				printf("EMMC boot, nand init failed\n");
			}

			if(ret && init_flag){
			//error case
				device_boot_flag = CARD_BOOT_FLAG;
			}
			else if(init_flag && (ret == 0)){
				device_boot_flag = NAND_BOOT_FLAG;
			}
			else if((init_flag == 0) && ret){
				device_boot_flag = CARD_BOOT_FLAG;
			}
			else{
				device_boot_flag = SPI_NAND_FLAG;
			}
		}
	}
	else{  //SPI boot or other case, shoulde init all the device here.
	//for SPI boot, init env first+
		printf("enter spi boot\n");
		ret = spi_env_relocate_spec();
		if(ret == 0){
			printf("spi success\n");
			device_boot_flag = get_storage_device_flag();
			if(device_boot_flag == SPI_EMMC_FLAG){
				ret = emmc_init();
				if(ret){
					device_boot_flag = CARD_BOOT_FLAG;
					printf("eMMC init failed for SPI boot\n");
				}
			}
			else if(device_boot_flag == SPI_NAND_FLAG){
				ret = amlnf_init(0x5);
				if (ret) {
					device_boot_flag = CARD_BOOT_FLAG;
					printf("nand init failed for SPI boot\n");
				}
			}
			else{
				// try nand first
				device_boot_flag = SPI_BOOT_FLAG;
				init_flag = amlnf_init(0x5);
				if(init_flag){
					//then try emmc
					init_flag = emmc_init();
					if(init_flag){
						device_boot_flag = CARD_BOOT_FLAG;
						printf("nand init failed for SPI boot\n");
					}
					else{
						device_boot_flag = SPI_EMMC_FLAG;
					}
				}
				else{
					device_boot_flag = SPI_NAND_FLAG;
				}
			}
		}
		else{
			//try nand first
			device_boot_flag = NAND_BOOT_FLAG;
			init_flag = amlnf_init(0x5);
			if(init_flag){
				//then try emmc
				device_boot_flag = EMMC_BOOT_FLAG;
				init_flag = emmc_init();
				if(init_flag){
					device_boot_flag = CARD_BOOT_FLAG;
					printf("nand init failed for spi boot\n");
				}
				else{
					device_boot_flag = EMMC_BOOT_FLAG;
				}
			}
			else{
				device_boot_flag = NAND_BOOT_FLAG;
			}
		}
	}
printf("device_boot_flag=%d\n",device_boot_flag);
}
#endif
/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */

void board_init_r (gd_t *id, ulong dest_addr)
{
	char *s;
	bd_t *bd;
	ulong malloc_start;
	//unsigned init_flag = 0;
/*#ifdef CONFIG_STORE_COMPATIBLE
	//int init_ret=0;
	unsigned init_flag = 0;
#endif
#ifdef  CONFIG_NEXT_NAND
	int ret = 0;
#endif*/
#if defined(CONFIG_PARTITIONS_STORE)
    struct mmc *mmc;
#endif
#if !defined(CONFIG_SYS_NO_FLASH)
	ulong flash_size;
#endif


	//2013.07.16
	//cache update after relocation
	dcache_flush();
	icache_invalid();
	//end 2013.07.16

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	lib_board_init_r_start = get_utimer(0);

	printf("\ntime: from powerup to board_init_r time(us):%d\n",(lib_board_init_r_start));
#endif

	gd = id;
	bd = gd->bd;

	gd->env_addr += gd->reloc_off;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	monitor_flash_len = _end_ofs;
	debug ("monitor flash len: %08lX\n", monitor_flash_len);

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	debug ("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

#ifdef CONFIG_LOGBUFFER
	logbuff_init_ptrs ();
#endif
#ifdef CONFIG_POST
	post_output_backlog ();
#endif

	/* The Malloc area is immediately below the monitor copy in DRAM */
	malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_init (malloc_start, TOTAL_MALLOC_LEN);
#ifdef CONFIG_ACS
	extern int  get_partition_table(void);
	get_partition_table();
#endif

#ifdef CONFIG_GENERIC_MMC
    puts("MMC:   ");
    mmc_initialize(bd);
#endif
#ifdef CONFIG_AML_I2C
    extern int aml_i2c_init(void);
    aml_i2c_init();
#endif
#if defined(CONFIG_AML_V2_USBTOOL)
	if(is_tpl_loaded_from_usb())//is uboot loaded from usb or bootable sdcard
	{
        aml_v2_usb_producing(0, bd);//would NOT return if 1)boot from usb, 2)boot from sdmmc and fatexist(aml_sdc_burn.ini)
	}
#endif// #if defined(CONFIG_AML_V2_USBTOOL)

	board_init();	/* Setup chipselects */
#if !defined(CONFIG_SYS_NO_FLASH)
	puts ("Flash: ");

	if ((flash_size = flash_init ()) > 0) {
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		print_size (flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		s = getenv ("flashchecksum");
		if (s && (*s == 'y')) {
			printf ("  CRC: %08X",
				crc32 (0, (const unsigned char *) CONFIG_SYS_FLASH_BASE, flash_size)
			);
		}
		putc ('\n');
# else	/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size (flash_size, "\n");
# endif /* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts (failed);
		hang ();
	}
#endif

	AML_LOG_INIT("board");
	AML_LOG_TE("board");
#ifdef CONFIG_STORE_COMPATIBLE
	get_device_boot_flag();
	amlnf_init(0x0);
#else
#if CONFIG_JERRY_NAND_TEST
	nand_init();
#endif
#if defined(CONFIG_CMD_NAND)
	puts ("NAND:  ");
#ifdef  CONFIG_NEXT_NAND
#ifdef AML_NAND_UBOOT
extern int amlnf_init(unsigned flag);
#else
struct platform_device;
extern int amlnf_init(struct platform_device *pdev);
#endif
	amlnf_init(0x0);
// flag = 0,indicate normal boot;
//flag = 1, indicate update;
//flag = 2, indicate need erase
#else
	nand_init();	/* go init the NAND */
#endif
#endif
#endif

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	unsigned int after_nand_init =  get_utimer(0);
	printf("\ntime: from powerup to nand init finished %d us \n", after_nand_init);
#endif

	AML_LOG_TE("board");
/*
#ifdef CONFIG_STORE_COMPATIBLE
	init_ret = ret;
	extern int get_storage_device_flag(int init_ret);
	get_storage_device_flag(init_ret);
#endif

	AML_LOG_TE("board");
*/
#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

	AML_LOG_TE("board");
/*
#if defined (CONFIG_GENERIC_MMC) && defined(CONFIG_STORE_COMPATIBLE)
    if((device_boot_flag == SPI_EMMC_FLAG) || (device_boot_flag == EMMC_BOOT_FLAG)) { // if eMMC/tSD is exist
        mmc = find_mmc_device(1);
        if (mmc) {
            mmc_init(mmc); // init eMMC/tSD
        }
    }
#endif

	AML_LOG_TE("board");
*/
#if defined (CONFIG_PARTITIONS_STORE)
        mmc = find_mmc_device(1);
        if (mmc) {
            mmc_init(mmc); // init eMMC/tSD
        }
#endif

	AML_LOG_TE("board");

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

	AML_LOG_TE("board");

	/* initialize environment */
	env_relocate ();

	AML_LOG_TE("board");

#ifdef CONFIG_STORE_COMPATIBLE
	extern void set_storage_device_flag(void);
	set_storage_device_flag();
#endif

	AML_LOG_TE("board");

//#ifdef MX_REVD
#if defined(CONFIG_M6) || defined(CONFIG_M6TV) || defined(CONFIG_M6TVD)
 		//if not clear, uboot command reset will fail -> blocked
 		*((volatile unsigned long *)P_AO_RTI_STATUS_REG0) = 0;
#endif

	AML_LOG_TE("board");

#ifdef CONFIG_DT_PRELOAD
	if ((s = getenv ("preloaddtb")) != NULL) {
		run_command(s, 0);
	}
#endif

	AML_LOG_TE("board");

#ifdef CONFIG_VPU_PRESET
	extern int vpu_probe(void);
	vpu_probe();
#endif

	AML_LOG_TE("board");

#ifdef CONFIG_VFD
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */

	AML_LOG_TE("board");

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	stdio_init ();	/* get the devices list going. */

	AML_LOG_TE("board");

	jumptable_init ();

	AML_LOG_TE("board");


#if defined(CONFIG_API)
	/* Initialize API */
	api_init ();
#endif

	AML_LOG_TE("board");

	console_init_r ();	/* fully init console as a device */

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init ();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

	AML_LOG_TE("board");

	 /* set up exceptions */
	interrupt_init ();
	/* enable exceptions */
	enable_interrupts ();

	/* Perform network card initialisation if necessary */
#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	AML_LOG_TE("board");

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

	AML_LOG_TE("board");

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif

	AML_LOG_TE("board");

	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug ("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

	AML_LOG_TE("board");

#ifdef CONFIG_CMD_ADC_POWER_KEY
	puts ("cold power up \n");
	char *data_sus = getenv("suspend");
	int sus_ret = strcmp(data_sus,"on");
	if  (sus_ret == 0)
		{
			printf("pls touch key_pad\n");
			/*add  install_halder_riq for keypad and remote*/
			//run_command("adc 2",0);

		}
#endif

	AML_LOG_TE("board");

#if CONFIG_AUTO_START_SD_BURNING
    if(is_tpl_loaded_from_ext_sdmmc())
    {
    AML_LOG_TE("board");
        if(aml_check_is_ready_for_sdc_produce())
        {
        AML_LOG_TE("board");
            aml_v2_sdc_producing(0, bd);
        }
    }
#endif// #if CONFIG_AUTO_START_SD_BURNING

	AML_LOG_TE("board");

#ifdef	CONFIG_VIDEO_AMLLCD
	#ifndef CONFIG_NO_LCD_INIT_IN_BOARDC
		puts("LCD Initialize:   \n");
		aml_lcd_init();
	#endif
#endif

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	unsigned int after_lcd_init =  get_utimer(0);
	printf("\ntime: from powerup to lcd init finished %d us \n", after_lcd_init );
#endif

#ifdef CONFIG_POST
	post_run (NULL, POST_RAM | post_bootmode_get(0));
#endif

	AML_LOG_TE("board");

#if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
	/*
	 * Export available size of memory for Linux,
	 * taking into account the protected RAM at top of memory
	 */
	{
		ulong pram;
#ifndef CONFIG_POST_AML
		uchar memsz[32];
#endif
#ifdef CONFIG_PRAM
		char *s;

		if ((s = getenv ("pram")) != NULL) {
			pram = simple_strtoul (s, NULL, 10);
		} else {
			pram = CONFIG_PRAM;
		}
#else
		pram=0;
#endif
#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
		/* Also take the logbuffer into account (pram is in kB) */
		pram += (LOGBUFF_LEN+LOGBUFF_OVERHEAD)/1024;
#endif
#endif
#ifndef CONFIG_POST_AML
		sprintf ((char *)memsz, "%ldk", (bd->bi_memsize / 1024) - pram);
		setenv ("mem", (char *)memsz);
#endif
	}
#endif
	char *uartb = getenv("uartb_enable");
	if (strcmp(uartb,"on") == 0) {
		run_command("uart init",0);
	}

	AML_LOG_TE("board");

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	main_loop_start = get_utimer(0);
	printf("\ntime: from powerup to start main_loop time(us):%d\n\n",main_loop_start);
#endif

	AML_LOG_TE("board");

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
