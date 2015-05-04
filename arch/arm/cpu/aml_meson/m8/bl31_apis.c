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
#include <asm/arch/bl31_apis.h>

#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

static unsigned sharemem_input_base;
static unsigned sharemem_output_base;

/**
 * @brief meson_trustzone_sram_read_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_sram_read_reg32(uint32_t addr)
{
	register uint32_t r0 asm("r0") = SRAM_READ;
	register uint32_t r1 asm("r1") = addr;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    __asmeq("%2", "r1")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0), "r"(r1), "r"(r1));
	} while (0);

	return r0;
}

uint32_t meson_trustzone_read_socrev1(void)
{
	register uint32_t r0 asm("r0") = CORE_RD_REV1;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

uint32_t meson_trustzone_get_sharemem_input_base(void)
{
	register uint32_t r0 asm("r0") = GET_SHARE_MEM_INPUT_BASE;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}
uint32_t	meson_trustzone_get_sharemem_output_base(void)
{
	register uint32_t r0 asm("r0") = GET_SHARE_MEM_OUTPUT_BASE;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

#ifdef CONFIG_ACS
uint32_t meson_trustzone_acs_addr(uint32_t addr)
{
	uint32_t cmd = 0;
	uint32_t len = SECURE_OS_ACS_LEN;
	uint32_t dest = SECURE_OS_ACS_DRAM_ADDR;
	if (!sharemem_input_base)
		sharemem_input_base = meson_trustzone_get_sharemem_input_base();
	if (!sharemem_output_base)
		sharemem_output_base =
			meson_trustzone_get_sharemem_output_base();

	if ((addr < SECURE_OS_ACS_SRAM_ADDR)
		|| (addr > (SECURE_OS_ACS_SRAM_ADDR + SECURE_OS_ACS_LEN))) {
		cmd = SRAM_ACS_READ;
		addr = *((unsigned int *)addr);
	} else
		cmd = SRAM_ACS_INDIRECT_READ;
	asm __volatile__("" : : : "memory");

	register uint32_t r0 asm("r0") = cmd;
	register uint32_t r1 asm("r1") = addr;
	register uint32_t r2 asm("r2") = len;
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
	if (len > r0)
		len = r0;
	memcpy((void *)dest, (const void *)sharemem_output_base, len);

	return dest;
}
#endif

#ifdef CONFIG_EFUSE
int32_t meson_trustzone_efuse(struct efuse_hal_api_arg *arg)
{
	int ret;
	unsigned cmd, offset, size;
	unsigned int *retcnt = (unsigned int *)(arg->retcnt_phy);

	if (!sharemem_input_base)
		sharemem_input_base = meson_trustzone_get_sharemem_input_base();
	if (!sharemem_output_base)
		sharemem_output_base =
			meson_trustzone_get_sharemem_output_base();

	if (arg->cmd == EFUSE_HAL_API_READ)
		cmd = EFUSE_READ;
	else if (arg->cmd == EFUSE_HAL_API_WRITE)
		cmd = EFUSE_WRITE;
	else
		cmd = EFUSE_WRITE_PATTERN;
	offset = arg->offset;
	size = arg->size;

	if (arg->cmd == EFUSE_HAL_API_WRITE)
		memcpy((void *)sharemem_input_base,
					(const void *)arg->buffer_phy, size);
	asm __volatile__("" : : : "memory");

	register int32_t r0 asm("r0") = cmd;
	register uint32_t r1 asm("r1") = offset;
	register uint32_t r2 asm("r2") = size;
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
	ret = r0;
	*retcnt = r0;

	if ((arg->cmd == EFUSE_HAL_API_READ) && (ret != 0))
		memcpy((void *)arg->buffer_phy,
				(const void *)sharemem_output_base, ret);

	if (!ret)
		return -1;
	else
		return 0;
}

ssize_t meson_trustzone_efuse_writepattern(const char *buf, size_t count)
{
	struct efuse_hal_api_arg arg;
	unsigned int retcnt;

	if (count != EFUSE_BYTES)
		return 0;	/* Past EOF */

	arg.cmd = EFUSE_HAL_API_WRITE_PATTERN;
	arg.offset = 0;
	arg.size = count;
	arg.buffer_phy = (unsigned int)buf;
	arg.retcnt_phy = (unsigned int)&retcnt;
	int ret;
	ret = meson_trustzone_efuse(&arg);
	return ret;
}

#endif

/**
 * @brief meson_trustzone_rtc_read_reg32
 *
 * @param addr
 * @param value
 */
uint32_t meson_trustzone_rtc_read_reg32(uint32_t addr)
{
	register uint32_t r0 asm("r0") = RTC_READ;
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
	register uint32_t r0 asm("r0") = RTC_WRITE;
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
uint32_t meson_trustzone_suspend(void)
{
	register uint32_t r0 asm("r0") = SUSPEND_FIRMWARE;

	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

uint32_t meson_trustzone_suspend_uboot(void)
{
	register uint32_t r0 asm("r0") = SUSPEND_FIRMWARE_UBOOT;

	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

uint32_t meson_trustzone_suspend_init(void)
{
	register uint32_t r0 asm("r0") = SUSPEND_INIT;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

uint32_t meson_get_share_mem_base(void)
{
	register uint32_t r0 asm("r0") = GET_SHARE_MEM_INPUT_BASE;
	do {
		asm volatile(
		    __asmeq("%0", "r0")
		    __asmeq("%1", "r0")
		    "smc    #0  @switch to secure world\n"
		    : "=r"(r0)
		    : "r"(r0));
	} while (0);

	return r0;
}

#endif /* CONFIG_AML_SUSPEND */


uint32_t meson_trustzone_boot_check(unsigned char *addr)
{
	unsigned int ret = 0;
	struct sram_hal_api_arg arg = {};

	arg.cmd = SRAM_HAL_API_CHECK;
	arg.req_len = 0x1000000;
	arg.res_len = 0;
	arg.req_phy_addr = (unsigned int)addr;
	arg.res_phy_addr = (unsigned int)NULL;

	asm __volatile__("" : : : "memory");

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

	ret = r0;

	return ret;
}

uint32_t meson_trustzone_efuse_check(unsigned char *addr)
{
	unsigned int ret = 0;
	struct sram_hal_api_arg arg = {};

	arg.cmd = SRAM_HAL_API_CHECK_EFUSE;
	arg.req_len = 0x1000000;
	arg.res_len = 0;
	arg.req_phy_addr = (unsigned int)addr;
	arg.res_phy_addr = (unsigned int)NULL;

	asm __volatile__("" : : : "memory");

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

	ret = r0;

	return ret;
}

#ifdef CONFIG_MESON_SECURE_HDCP
int32_t meson_trustzone_hdcp(struct hdcp_hal_api_arg *arg)
{
	int ret;
	register int32_t r0 asm("r0") = CALL_TRUSTZONE_HAL_API;
	register uint32_t r1 asm("r1") = TRUSTZONE_HAL_API_HDCP;
	register uint32_t r2 asm("r2") = (unsigned int)arg;
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

	ret = r0;
	return ret;
}

#endif
