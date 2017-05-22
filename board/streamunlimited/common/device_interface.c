#include <common.h>
#include <asm/errno.h>
#include <asm/saradc.h>
#include "device_interface.h"

struct adc_map_entry {
	u8 adc_code;
	u16 adc_value_min;
	u16 adc_value_max;
};

/*
 * This is based on the table from:
 * 	https://extern.streamunlimited.com:8443/display/Stream800/StreamVienna+Hardware+Configuration
 */
static const struct adc_map_entry adc_map[] = {
	{ 0x01, 0x0000, 0x0088 },
	{ 0x02, 0x0088, 0x0111 },
	{ 0x03, 0x0111, 0x0199 },
	{ 0x04, 0x0199, 0x0222 },
	{ 0x05, 0x0222, 0x02AA },
	{ 0x06, 0x02AA, 0x0333 },
	{ 0x07, 0x0333, 0x03BB },
	{ 0x08, 0x03BB, 0x0444 },
	{ 0x09, 0x0444, 0x04CC },
	{ 0x0A, 0x04CC, 0x0555 },
	{ 0x0B, 0x0555, 0x05DD },
	{ 0x0C, 0x05DD, 0x0666 },
	{ 0x0D, 0x0666, 0x06EE },
	{ 0x0E, 0x06EE, 0x0777 },
	{ 0x0F, 0x0777, 0x07FF },
	{ 0x10, 0x07FF, 0x0888 },
	{ 0x11, 0x0888, 0x0910 },
	{ 0x12, 0x0910, 0x0999 },
	{ 0x13, 0x0999, 0x0A21 },
	{ 0x14, 0x0A21, 0x0AAA },
	{ 0x15, 0x0AAA, 0x0B32 },
	{ 0x16, 0x0B32, 0x0BBB },
	{ 0x17, 0x0BBB, 0x0C43 },
	{ 0x18, 0x0C43, 0x0CCC },
	{ 0x19, 0x0CCC, 0x0D54 },
	{ 0x1A, 0x0D54, 0x0DDD },
	{ 0x1B, 0x0DDD, 0x0E65 },
	{ 0x1C, 0x0E65, 0x0EEE },
	{ 0x1D, 0x0EEE, 0x0F76 },
	{ 0x1E, 0x0F76, 0x0FFF },
};

/*
 * These names are more human friendly and can be used for printing.
 */
static const char *module_names[] = {
	"unknown",
	"stream770 basic",
};

/*
 * These names can be used where no spaces or other special charactar are allowed,
 * e.g. for fit configurations.
 */
static const char *canonical_module_names[] = {
	"unknown",
	"stream770b",
	"stream770x"
};

struct module_map_entry {
	enum sue_module module;
	u8 module_version;
	u8 lsb_code;
};

static const struct module_map_entry module_map[] = {
	{ SUE_MODULE_S770_BASIC,	1, 0x0a },
	{ SUE_MODULE_S770_EXTENDED,	1, 0x15 },
};

static const char *carrier_names[] = {
	"unknown",
	"s770 reference kit go",
};

static const char *canonical_carrier_names[] = {
	"unknown",
	"streamkitgo",
	"factory",
};

extern struct sue_carrier_ops demo_client_ops;

static const struct sue_carrier_ops *sue_carrier_ops[] = {
	NULL,				/* unknown */
	&demo_client_ops,		/* demo client */
	&demo_client_ops,		/* highend demo client, it is also handled by the demo client board file */
	&demo_client_ops,		/* ref kit, still handled by demo client board file */
};

struct carrier_map_entry {
	enum sue_carrier carrier;
	u8 carrier_version;
	u8 msb_code;
	u8 lsb_code;
};

/*
 * NOTE: currently the demo client has the same resistors for all revisions,
 * so we just set it to zero here.
 *
 * NOTE: on the stream810 with the basic interface the LSB will always read
 * 0x01, so we always assume it's a normal demo client.
 */
static const struct carrier_map_entry carrier_map[] = {
	{ SUE_CARRIER_S770_REF_KIT,	1, 0x15, 0x01 },
	{ SUE_CARRIER_S770_TEST,	1, 0x19, 0x01 },
};

static int get_adc_code(u16 adc_value)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(adc_map); i++) {
		if (adc_value >= adc_map[i].adc_value_min && adc_value < adc_map[i].adc_value_max)
			return adc_map[i].adc_code;
	}

	return -EINVAL;
}

static int fill_device_info(struct sue_device_info *device, u16 module_msb_adc_value, u16 module_lsb_adc_value,
			u16 carrier_msb_adc_value, u16 carrier_lsb_adc_value)
{
	int i;
	int module_msb_code, module_lsb_code;
	int carrier_msb_code, carrier_lsb_code;

	module_msb_code = get_adc_code(module_msb_adc_value);
	if (module_msb_code < 0)
		return module_msb_code;

	module_lsb_code = get_adc_code(module_lsb_adc_value);
	if (module_lsb_code < 0)
		return module_lsb_code;

	carrier_msb_code = get_adc_code(carrier_msb_adc_value);
	if (carrier_msb_code < 0)
		return carrier_msb_code;

	carrier_lsb_code = get_adc_code(carrier_lsb_adc_value);
	if (carrier_lsb_code < 0)
		return carrier_lsb_code;


	for (i = 0; i < ARRAY_SIZE(module_map); i++) {
		if (module_map[i].lsb_code == module_lsb_code) {
			device->module = module_map[i].module;
			device->module_version = module_map[i].module_version;
			break;
		}
	}

	if (SUE_MODULE_S770_BASIC == device->module) {
		/* basic module (Stream770b) support only board MSB pin */
		for (i = 0; i < ARRAY_SIZE(carrier_map); i++) {
			if (carrier_map[i].msb_code == carrier_msb_code) {
				device->carrier = carrier_map[i].carrier;
				device->carrier_version = carrier_map[i].carrier_version;
				break;
			}
		}
	}
	else {
		/* extended module (Stream770x) support all board detection pins */
		for (i = 0; i < ARRAY_SIZE(carrier_map); i++) {
			if (carrier_map[i].msb_code == carrier_msb_code && carrier_map[i].lsb_code == carrier_lsb_code) {
				device->carrier = carrier_map[i].carrier;
				device->carrier_version = carrier_map[i].carrier_version;
				break;
			}
		}
	}

	device->carrier_ops = NULL;
	device->fec2_phy_addr = -1;

	printf("module_msb_code=0x%x, module_lsb_code=0x%x, carrier_msb_code=0x%x, carrier_lsb_code=0x%x\n", module_msb_code, module_lsb_code, carrier_msb_code, carrier_lsb_code);
	printf("device->module=%d, device->module_version=%d, device->carrier=%d, device->carrier_version=%d\n", device->module, device->module_version, device->carrier, device->carrier_version);

	return 0;
}

int sue_device_detect(struct sue_device_info *device)
{
	int ret;
	unsigned int module_msb_adc_value, module_lsb_adc_value;
	unsigned int carrier_msb_adc_value, carrier_lsb_adc_value;

	//TODO
	saradc_enable();
	module_lsb_adc_value = get_adc_sample(0) << 2;
	module_msb_adc_value = get_adc_sample(1) << 2;
	carrier_lsb_adc_value = get_adc_sample(2) << 2;
	carrier_msb_adc_value = get_adc_sample(3) << 2;
	saradc_disable();

	printf("ADC: m1=0x%x, m0=0x%x, c1=0x%x, c0=0x%x\n",module_msb_adc_value, module_lsb_adc_value, carrier_msb_adc_value, carrier_lsb_adc_value);

	ret = fill_device_info(device, module_msb_adc_value, module_lsb_adc_value, carrier_msb_adc_value, carrier_lsb_adc_value);

	return ret;
}

int sue_print_device_info(const struct sue_device_info *device)
{
	printf("Module: '%s (L%d)', Carrier board: '%s (L%d)'\n", module_names[device->module], device->module_version, carrier_names[device->carrier], device->carrier_version);
	return 0;
}

const char *sue_device_get_canonical_module_name(const struct sue_device_info *device)
{
	return canonical_module_names[device->module];
}

const char *sue_device_get_canonical_carrier_name(const struct sue_device_info *device)
{
	return canonical_carrier_names[device->carrier];
}

int sue_carrier_ops_init(struct sue_device_info *device)
{
	device->carrier_ops = sue_carrier_ops[device->carrier];

	return 0;
}

int sue_carrier_init(const struct sue_device_info *device)
{
	if (device == NULL || device->carrier_ops == NULL || device->carrier_ops->init == NULL)
		return -EIO;

	return device->carrier_ops->init(device);
}

int sue_carrier_late_init(const struct sue_device_info *device)
{
	if (device == NULL || device->carrier_ops == NULL || device->carrier_ops->late_init == NULL)
		return -EIO;

	return device->carrier_ops->late_init(device);
}

int sue_carrier_get_usb_update_request(const struct sue_device_info *device)
{
	if (device == NULL || device->carrier_ops == NULL || device->carrier_ops->get_usb_update_request == NULL)
		return -EIO;

	return device->carrier_ops->get_usb_update_request(device);
}
