
/*
 * arch/arm/cpu/aml_meson/g9tv/firmware/arch_init.c
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
	//enable watchdog for 5s
	//if bootup failed, switch to next boot device
	AML_WATCH_DOG_SET(5000); //5s
	writel(readl(P_AO_RTI_STATUS_REG0), SKIP_BOOT_REG_BACK_ADDR); //[By Sam.Wu]backup the skip_boot flag to sram for v2_burning

	//setbits_le32(0xda004000,(1<<0));	//TEST_N enable: This bit should be set to 1 as soon as possible during the Boot process to prevent board changes from placing the chip into a production test mode

	writel((readl(0xDA000004)|0x08000000), 0xDA000004);	//set efuse PD=1

	//A9 JTAG enable
	writel(0x102,0xda004004);

	//TDO enable
	writel(readl(0xc8100014)|0x4000,0xc8100014);

	//detect sdio debug board
	unsigned pinmux_2 = readl(P_PERIPHS_PIN_MUX_2);

	// clear sdio pinmux
	setbits_le32(P_PREG_PAD_GPIO2_O,0x3f<<20);
	setbits_le32(P_PREG_PAD_GPIO2_EN_N,0x3f<<20);
	clrbits_le32(P_PERIPHS_PIN_MUX_2,7<<12);  //clear sd d1~d3 pinmux

	if (!(readl(P_PREG_PAD_GPIO2_I)&(1<<24))) {  //sd_d3 low, debug board in
		clrbits_le32(P_AO_RTI_PIN_MUX_REG,3<<11);   //clear AO uart pinmux
		setbits_le32(P_PERIPHS_PIN_MUX_8,3<<9);
		serial_puts("\nsdio debug board detected ");
		if ((readl(P_PREG_PAD_GPIO2_I)&(1<<20)))
			writel(0x220,P_AO_SECURE_REG1);  //enable sdio jtag
	}
	else{
		serial_puts("\nno sdio debug board detected ");
		writel(pinmux_2,P_PERIPHS_PIN_MUX_2);
	}
}
