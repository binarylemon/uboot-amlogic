#include <asm/arch/reboot.h>

#include "platform.h"
#include "usb_pcd.h"

#include "usb_pcd.c"
#include "platform.c"
#include "dwc_pcd.c"
#include "dwc_pcd_irq.c"

int usb_boot(int clk_cfg)
{
	int cfg = INT_CLOCK;
	if(clk_cfg)
		cfg = EXT_CLOCK;
	set_usb_phy_config(cfg);

	usb_parameter_init();
		
	if(usb_pcd_init())
		return 0;

	while(1)
	{
		//watchdog_clear();		//Elvis Fool
		if(usb_pcd_irq())
			break;
	}
	return 0;
}

void relocate_init(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{
	writel(0,P_WATCHDOG_TC);//disable Watchdog
	reboot_mode = MESON_USB_BURNER_REBOOT;
	while(1)
	{
		//usb_boot(0);	//Elvis
		usb_boot(1);
	}
}


