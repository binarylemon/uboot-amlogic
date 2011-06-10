#ifndef __CONFIG_ARM_8726M_TARGET_H_
#define __CONFIG_ARM_8726M_TARGET_H_


//UART Sectoion
#define CONFIG_CONS_INDEX   1

//Enable storage devices
#define CONFIG_CMD_NAND  1
#define CONFIG_CMD_SF    1

#define CONFIG_SDIO_B1   1
#define CONFIG_SDIO_A    1
#define CONFIG_SDIO_B    1
#define CONFIG_SDIO_C    1
#define CONFIG_ENABLE_EXT_DEVICE_RETRY 1

#endif