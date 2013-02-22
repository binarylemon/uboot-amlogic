/*
 * Command for suspend.
 *
 * Copyright (C) 2012 Amlogic.
 * Elvis Yu <elvis.yu@amlogic.com>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/ao_reg.h>
extern void meson_pm_suspend(void);
#define readl(addr) (*(volatile unsigned int*)(addr))
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))

static int do_suspend (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	//run_command("video dev bl_off",1);
	//run_command("video dev disable",1);
	//char *c=#cc;

	
	writel(0x00005801,P_AO_RTI_PIN_MUX_REG);
	writel(0x30fa0013,P_AO_IR_DEC_REG0);
	writel(0x001ebe50,P_AO_IR_DEC_REG1);
	writel(0x00f800ca,P_AO_IR_DEC_LDR_ACTIVE);
	writel(0x00f800ca,P_AO_IR_DEC_LDR_IDLE);
	writel(0x0044002c,P_AO_IR_DEC_BIT_0);

	printf("IR init done! %s\n");
	
	meson_pm_suspend();
	return 0;
}



U_BOOT_CMD(
	suspend1,	1,	0,	do_suspend,
	"suspend",
	"/N\n"
	"This command will into suspend\n"
);

