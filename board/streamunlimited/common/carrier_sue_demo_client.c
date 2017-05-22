#include <common.h>
//#include <asm/gpio.h>
//#include <asm/arch/mx7-pins.h>

#include "device_interface.h"

#if 0
#define FWUP_GPIO IMX_GPIO_NR(6, 20)

#define ALT_FWUP_TIMEOUT	3000	/* in ms */
#define ALT_FWUP_GPIO IMX_GPIO_NR(3, 21)
#endif

//static int alt_fwup_request = 0;

#if 0
static const iomux_v3_cfg_t fwupdate_pads[] = {
	MX7D_PAD_SAI2_TX_BCLK__GPIO6_IO20 | MUX_PAD_CTRL(PAD_CTL_PUE | PAD_CTL_PUS_PU5KOHM),
	MX7D_PAD_LCD_DATA15__GPIO3_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif

static int demo_client_init(const struct sue_device_info *device)
{
#if 0
	imx_iomux_v3_setup_multiple_pads(fwupdate_pads, ARRAY_SIZE(fwupdate_pads));
	gpio_direction_input(FWUP_GPIO);
	gpio_direction_input(ALT_FWUP_GPIO);
#endif
	return 0;
}

static int demo_client_late_init(const struct sue_device_info *device)
{
#if 0
	int totaldelay = 0;

	while (gpio_get_value(ALT_FWUP_GPIO)) {
		if (totaldelay > ALT_FWUP_TIMEOUT) {
			printf("Alt fwup request set\n");
			alt_fwup_request = 1;
			break;
		}

		mdelay(100);
		totaldelay += 100;
	}
#endif
	return 0;
}

static int demo_client_get_usb_update_request(const struct sue_device_info *device)
{
#if 0
	return alt_fwup_request || !gpio_get_value(FWUP_GPIO);
#endif
	return 0;
}

struct sue_carrier_ops demo_client_ops = {
	.init = demo_client_init,
	.late_init = demo_client_late_init,
	.get_usb_update_request = demo_client_get_usb_update_request,
};
