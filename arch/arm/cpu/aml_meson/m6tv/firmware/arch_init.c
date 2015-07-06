
/*
 * arch/arm/cpu/aml_meson/m6tv/firmware/arch_init.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

void arch_init(void) {
	writel((readl(0xDA000004)|0x08000000), 0xDA000004);	//set efuse PD=1

	//write ENCI_MACV_N0 (CBUS 0x1b30) to 0, disable Macrovision
	writel(0, CBUS_REG_ADDR(ENCI_MACV_N0));

	//Default to open ARM JTAG for M6
	#define AML_M6_JTAG_ENABLE
	#define AML_M6_JTAG_SET_ARM

	//for M6 only. And it will cause M3 fail to boot up.
	//TEST_N enable: This bit should be set to 1 as soon as possible during the
	//Boot process to prevent board changes from placing the chip into a production test mode
	setbits_le32(0xda004000,(1<<0));

	// set bit [12..14] to 1 in AO_RTI_STATUS_REG0
	// This disables boot device fall back feature in MX Rev-D
	// This still enables bootloader to detect which boot device
	// is selected during boot time.
	switch (readl(P_AO_RTI_STATUS_REG0))
	{
	case 0x6b730001:
	case 0x6b730002: writel(readl(P_AO_RTI_STATUS_REG0) |(0x70<<8),P_AO_RTI_STATUS_REG0);break;
	}

	#ifdef AML_M6_JTAG_SET_ARM
		//A9 JTAG enable
		writel(0x80000510,0xda004004);
		//TDO enable
		writel(readl(0xc8100014)|0x4000,0xc8100014);
	#elif AML_M6_JTAG_SET_ARC
		//ARC JTAG enable
		writel(0x80051001,0xda004004);
		//ARC bug fix disable
		writel((readl(0xc8100040)|1<<24),0xc8100040);
	#endif	//AML_M6_JTAG_SET_ARM

	//Watchdog disable
	//writel(0,0xc1109900);
}
