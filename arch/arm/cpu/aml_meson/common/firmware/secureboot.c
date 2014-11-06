/*
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *  Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Secure loader
 *
 * Copyright (C) 2012 Amlogic, Inc.
 *
 * Author: Platform-SH@amlogic.com
 *
 */
 
#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/romboot.h>
#include <asm/arch/trustzone.h>
#include <asm/cpu_id.h>

extern int uclDecompress(char* op, unsigned* o_len, char* ip);

int load_secureos(void)
{
	int rc = 0;
	unsigned len;
	unsigned *psecureargs = NULL;
	unsigned memsize;

	psecureargs = (unsigned*)(AHB_SRAM_BASE + READ_SIZE-SECUREARGS_ADDRESS_IN_SRAM);
	*psecureargs = (unsigned)NULL;	
	memsize = 0;
#ifdef CONFIG_MESON_SECUREARGS
#if (MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON8)
	memsize = readl(CONFIG_DDR_SIZE_IND_ADDR);
	if(IS_MESON_M8M2_CPU) {
		if (memsize > 1024) {
			*psecureargs = (unsigned)__secureargs_m8m2_2g;
		} else {
			*psecureargs = (unsigned)__secureargs_m8m2;
		}
	} else {
		if (memsize > 1024) {
			*psecureargs = (unsigned)__secureargs_m8_2g;
		} else {
			*psecureargs = (unsigned)__secureargs_m8;
		}
	}
	serial_puts("\nDDR size : ");
	serial_put_dec(memsize);
	serial_puts(" MB\n");
#else
	*psecureargs = (unsigned)__secureargs;
#endif
#endif

	/* UCL decompress */
	/* enable cache to speed up decompress */
#if CONFIG_AML_SPL_L1_CACHE_ON
	aml_cache_enable();
#endif
	rc=uclDecompress((char*)(SECURE_OS_DECOMPRESS_ADDR), &len,(char*)(SECURE_OS_COMPRESS_ADDR)); 	
#if CONFIG_AML_SPL_L1_CACHE_ON
	aml_cache_disable();
#endif

	return rc;
}
