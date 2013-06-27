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

extern int uclDecompress(char* op, unsigned* o_len, char* ip);

int load_secureos(void)
{
 	int rc=0;
 	unsigned len;    
/* 	
 	unsigned por_cfg=romboot_info->por_cfg;
	unsigned boot_id=romboot_info->boot_id;
 	
 	// load from internal flash
 	// load from external flash
 	rc=fw_init_extl(por_cfg);
 	if(rc)
 		goto exit;
 	rc=secure_load_extl(por_cfg);
 	if(rc)
 		goto exit;
*/ 	
 	// verify secureOS validation 		
#ifndef CONFIG_MESON_SECUREBOOT_WITHOUT_DECRYPT	
	rc=spldecrypt(SECURE_OS_ENCRYPTED_ADDR);
	if(rc)
		goto exit;	
#endif		
 	
 	// UCL decompress 	 	
#ifdef CONFIG_MESON_SECUREBOOT_WITHOUT_DECRYPT
	rc=uclDecompress((char*)(SECURE_OS_DECRYPTED_ADDR), &len,(char*)(SECURE_OS_ENCRYPTED_ADDR)); 	
#else 	
 	rc=uclDecompress((char*)(SECURE_OS_DECRYPTED_ADDR), &len,(char*)(SECURE_OS_ENCRYPTED_ADDR+SECUREOS_HEAD_SIZE)); 	
#endif 	 	
exit:
	return rc; 	 	
 }
 
 