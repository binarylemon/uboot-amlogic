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
 * Trustzone API
 *
 * Copyright (C) 2012 Amlogic, Inc.
 *
 * Author: Platform-BJ@amlogic.com
 *
 */


#include <asm/arch/io.h>
#include <amlogic/efuse.h>
#include <asm/cache.h>
#include <asm/arch/trustzone.h>

#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

typedef struct trustzone_cmd {
    unsigned int id;
    unsigned int context;
    unsigned int enc_id;

    unsigned int src_id;
    unsigned int src_context;

    unsigned int req_buf_len;
    unsigned int resp_buf_len;
    unsigned int ret_resp_buf_len;
    unsigned int cmd_status;
    unsigned int req_buf_phys;
    unsigned int resp_buf_phys;
    unsigned int meta_data_phys;
    unsigned int dev_file_id;
} trustzone_cmd_t;


/**
 * @brief meson_cpu_ctrl_reg_set
 *
 * @param para
 */
void meson_trustzone_smc(uint32_t para)
{
#define CALL_TRUSTZONE_API 0x1
#define OTZ_SVC 0x6
#define OTZ_CMD_ID 0x1
#define OTZ_CMD_TYPE_NS_TO_SECURE 0x1
    trustzone_cmd_t boot_cmd = {};
    boot_cmd.id = ((OTZ_SVC << 10) | OTZ_CMD_ID);
    boot_cmd.src_id = ((OTZ_SVC << 10) | OTZ_CMD_ID);
    boot_cmd.context = para;
    register uint32_t r0 asm("r0") = CALL_TRUSTZONE_API;
    register uint32_t r1 asm("r1") = virt_to_phys(&boot_cmd);
    register uint32_t r2 asm("r2") = OTZ_CMD_TYPE_NS_TO_SECURE;
    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1), "r"(r2));
    } while (0);
}

/**
 * @brief meson_trustzone_rtc_read_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_rtc_read_reg32(uint32_t addr)
{
    register uint32_t r0 asm("r0") = 0x2;
    register uint32_t r1 asm("r1") = addr;
    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1));
    } while (0);

	return r0;
}


/**
 * @brief meson_trustzone_rtc_write_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_rtc_write_reg32(uint32_t addr, uint32_t value)
{
    register uint32_t r0 asm("r0") = 0x3;
    register uint32_t r1 asm("r1") = addr;
    register uint32_t r2 asm("r2") = value;
    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1), "r"(r2));
    } while (0);

    return r0;
}


/**
 * @brief meson_trustzone_sram_read_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_sram_read_reg32(uint32_t addr)
{
    register uint32_t r0 asm("r0") = 0x2;
    register uint32_t r1 asm("r1") = addr;
    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1));
    } while (0);

    return r0;
}


/**
 * @brief meson_trustzone_sram_write_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_sram_write_reg32(uint32_t addr, uint32_t value)
{
    register uint32_t r0 asm("r0") = 0x3;
    register uint32_t r1 asm("r1") = addr;
    register uint32_t r2 asm("r2") = value;
    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1), "r"(r2));
    } while (0);

    return r0;
}

#ifdef CONFIG_AML_SUSPEND
/**
 * @brief meson_trustzone_suspend
 *
 * @suspend cmd smc entry
 */
#define TRUSTZONE_MON_SUSPNED_FIRMWARE          0x300
#define TRUSTZONE_MON_SUSPNED_FIRMWARE_INIT     0x301
#define TRUSTZONE_MON_SUSPNED_FIRMWARE_UBOOT     0x302


uint32_t meson_trustzone_suspend()
{
    register uint32_t r0 asm("r0") = 0x4;
    register uint32_t r1 asm("r1") = TRUSTZONE_MON_SUSPNED_FIRMWARE;

    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1));
    } while (0);

    return r0;
}

uint32_t meson_trustzone_suspend_uboot()
{
    register uint32_t r0 asm("r0") = 0x4;
    register uint32_t r1 asm("r1") = TRUSTZONE_MON_SUSPNED_FIRMWARE_UBOOT;

    do {
        asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1));
    } while (0);

    return r0;
}


uint32_t meson_trustzone_suspend_init(void)
{
	register uint32_t r0 asm("r0") = 0x4;
	register uint32_t r1 asm("r1") = TRUSTZONE_MON_SUSPNED_FIRMWARE_INIT;

	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    __asmeq("%2", "r1")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0), "r"(r1));
	} while (0);

	return r0;
}
#endif

#ifdef CONFIG_EFUSE
int32_t meson_trustzone_efuse(struct efuse_hal_api_arg* arg)
{	
	int ret;
	dcache_flush_range(arg->buffer_phy, (arg->size));
	dcache_flush_range(arg->retcnt_phy, (sizeof(unsigned int)));
	dcache_flush_range(arg, sizeof(struct efuse_hal_api_arg));
	
	register int32_t r0 asm("r0") = CALL_TRUSTZONE_HAL_API;
	register uint32_t r1 asm("r1") = TRUSTZONE_HAL_API_EFUSE;
	register uint32_t r2 asm("r2") = (unsigned int)arg;
	do{
		asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1), "r"(r2));
	}while(0);
	
	ret=r0;
	if(arg->cmd == EFUSE_HAL_API_READ)
		ov_dcache_invalid_range(arg->buffer_phy, (arg->size));		
	ov_dcache_invalid_range(arg->retcnt_phy, (sizeof(unsigned int)));
		
	return ret;
}
	
#endif


#ifdef CONFIG_MESON_STORAGE_BURN
int32_t meson_trustzone_storage(struct storage_hal_api_arg* arg)
{
	int ret;
	dcache_flush_range(arg->name_phy_addr, arg->namelen);
	dcache_flush_range(arg->data_phy_addr, arg->datalen);
	dcache_flush_range(arg, sizeof(struct storage_hal_api_arg));
		
	register int32_t r0 asm("r0") = CALL_TRUSTZONE_HAL_API;
	register int32_t r1 asm("r1") = TRUSTZONE_HAL_API_STORAGE;
	register int32_t r2 asm("r2") = (unsigned int)arg;
	
	do{
		asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            "smc    #0  @switch to secure world\n"
            : "=r"(r0)
            : "r"(r0), "r"(r1), "r"(r2));
	}while(0);
	
	ret=r0;
#ifdef CONFIG_MESON_STORAGE_DEBUG	
	if(arg->cmd == STORAGE_HAL_API_READ)
		ov_dcache_invalid_range(arg->data_phy_addr, (arg->datalen));		
#endif		
	ov_dcache_invalid_range(arg->retval_phy_addr, (sizeof(unsigned int)));			
	
	return ret;
}

#endif

uint32_t meson_trustzone_read_socrev1()
{
	register uint32_t r0 asm("r0") = CALL_TRUSTZONE_MON;
	register uint32_t r1 asm("r1") = TRUSTZONE_MON_CORE_RD_SOC_REV1;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    __asmeq("%2", "r1")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0), "r"(r1));
	} while (0);

	return r0;
}

uint32_t meson_trustzone_read_socrev2()
{
	register uint32_t r0 asm("r0") = CALL_TRUSTZONE_MON;
	register uint32_t r1 asm("r1") = TRUSTZONE_MON_CORE_RD_SOC_REV2;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    __asmeq("%2", "r1")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0), "r"(r1));
	} while (0);

	return r0;
}

#ifdef CONFIG_ACS
uint32_t meson_trustzone_acs_addr(uint32_t addr)
{
	unsigned int ret = 0;
	struct sram_hal_api_arg arg = {};

	arg.cmd = SRAM_HAL_API_CAS;
	arg.req_len = SECURE_OS_ACS_LEN;
	arg.res_len = SECURE_OS_ACS_LEN;

	if ((addr < SECURE_OS_ACS_SRAM_ADDR) || (addr > (SECURE_OS_ACS_SRAM_ADDR + SECURE_OS_ACS_LEN)))
		arg.req_phy_addr = *((volatile unsigned int*)addr);
	else
		arg.req_phy_addr = meson_trustzone_sram_read_reg32(addr);
	arg.res_phy_addr = SECURE_OS_ACS_DRAM_ADDR;
	dcache_flush_range(&arg, sizeof(struct sram_hal_api_arg));

	register uint32_t r0 asm("r0") = CALL_TRUSTZONE_HAL_API;
	register uint32_t r1 asm("r1") = TRUSTZONE_HAL_API_SRAM;
	register uint32_t r2 asm("r2") = (unsigned int)(&arg);
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    __asmeq("%2", "r1")
		    __asmeq("%3", "r2")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0), "r"(r1), "r"(r2));
	} while (0);

	ov_dcache_invalid_range(arg.res_phy_addr, (arg.res_len));
	ret = arg.res_phy_addr;

	return ret;
}
#endif

