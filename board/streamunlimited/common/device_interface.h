#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__

enum sue_module {
	SUE_MODULE_UNKNOWN,
	SUE_MODULE_S770_BASIC,
	SUE_MODULE_S770_EXTENDED,
};

enum sue_carrier {
	SUE_CARRIER_UNKNOWN,
	SUE_CARRIER_S770_REF_KIT,
	SUE_CARRIER_S770_TEST,
};

enum sue_reset_cause {
	SUE_RESET_CAUSE_UNKNOWN,
	SUE_RESET_CAUSE_POR,
	SUE_RESET_CAUSE_SOFTWARE,
	SUE_RESET_CAUSE_WDOG,
};

struct sue_device_info {
	enum sue_reset_cause reset_cause;

	enum sue_module module;
	u8 module_version;

	enum sue_carrier carrier;
	u8 carrier_version;

	const struct sue_carrier_ops *carrier_ops;

	/*
	 * If this is set to a value other than -1, the board.c file
	 * will patch the devicetree before booting the kernel. The
	 * patching will set the phy address of the fec2 MAC accordingly.
	 */
	int fec2_phy_addr;
};

struct sue_carrier_ops {
	int (*init)(const struct sue_device_info *device);
	int (*late_init)(const struct sue_device_info *device);
	int (*get_usb_update_request)(const struct sue_device_info *device);
};

int sue_carrier_init(const struct sue_device_info *device);
int sue_carrier_late_init(const struct sue_device_info *device);
int sue_carrier_get_usb_update_request(const struct sue_device_info *device);

int sue_device_detect(struct sue_device_info *device);
int sue_carrier_ops_init(struct sue_device_info *device);
int sue_print_device_info(const struct sue_device_info *device);

const char *sue_device_get_canonical_module_name(const struct sue_device_info *device);
const char *sue_device_get_canonical_carrier_name(const struct sue_device_info *device);

#endif /* __DEVICE_INTERFACE_H__ */
