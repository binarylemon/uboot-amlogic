/*
 * sue_fwupdate_common.h
 *
 * This file contains the common U-Boot script for the firmware upgrade process.
 */

#ifndef __SUE_FWUPDATE_COMMON_H
#define __SUE_FWUPDATE_COMMON_H

/*
 * UPDATE PROCESS is fully described at: https://extern.streamunlimited.com:8443/display/Stream800/Firmware+Update+Architecture
 *
 * Update methods:
 *
 * METHODS:
 *              readfitUimage                   - loads uImage to RAM
 *              readswufitUimage                - loads swupdate uImage to RAM
 *              swu_boot                        - main method
 *
 * VARIABLES:
 *              bootcount                       - actual count of uncorrect reboots
 *              bootlimit                       - limit of uncorrcet reboots, if reached, board does not boot anymore
 *              swu_load_addr                   - base RAM address for u-boot operations
 *              factory_state                   - whether the board is in a factory state
 *              usb_update_req                  - whether the USB update request is active
 */

#if 0
    "fdt_addr=0x83000000\0" \
    "fdt_high=0xffffffff\0" \
    "console=ttymxc0,115200\0" \
    "bootfile=uImage\0" \
    "optargs=mwifiex.driver_mode=0x3\0" \
    "swu_load_addr=0x80800000\0" \
    "wdtargs=imx2_wdt.timeout=120 imx2_wdt.early_disable=0\0" 
#endif

#define SUE_FWUPDATE_EXTRA_ENV_SETTINGS \
    "bootargs_defaults=setenv bootargs " \
    "console=${console}\0" \
    "mtdparts=\0" \
    "factory_state=0\0" \
    "usb_update_req=0\0" \
\
\
    "readfitimage=" \
        "if nand read fit ${fdt_addr};" \
            "then " \
            "echo \"INFO: fit partition load successful\"; " \
        "else " \
            "echo \"ERROR: cannot load kernel image from nand\"; " \
            "reset; " \
        "fi;\0" \
\
\
    "readswufitimage=" \
	"if nand read swufit ${fdt_addr};" \
            "then " \
            "echo \"INFO: swupdate kernel partition load successful\"; " \
        "else " \
            "echo \"ERROR: cannot load swupdate kernel image from nand\"; " \
            "reset; " \
        "fi;\0" \
\
\
    "bootfitimage=" \
        "if env exists fit_config; " \
            "then " \
            "echo \"INFO: will boot fit config ${fit_config}@1\"; " \
            "bootm ${fdt_addr}#${fit_config}@1; " \
        "fi; " \
        "echo \"INFO: will try to boot the default fit config\"; " \
        "bootm ${fdt_addr}; " \
        "echo \"INFO: fit boot failed...\"; " \
        "echo \"INFO: resetting...\"; " \
        "reset;\0" \
\
\
    "nandroot=ubi0:nsdk-rootfs rw\0" \
    "nandrootfstype=ubifs rootwait=1\0" \
    "nandargs=const toenv eth_int_addr; " \
        "setenv bootargs console=${console} " \
        "ubi.mtd=5 root=${nandroot} noinitrd ${wdtargs} " \
        "rootfstype=${nandrootfstype} " \
        "${mtdparts} ${optargs}\0" \
    "nand_boot=echo \"Booting from nand ...\"; " \
        "run nandargs; " \
        "echo \"INFO: loading fit image into RAM...\"; " \
        "bstate booting; " \
        "run readfitimage; " \
        "echo \"INFO: booting fit image...\"; " \
        "run bootfitimage;\0" \
\
\
    "swunandargs=const toenv eth_int_addr; " \
        "setenv bootargs console=${console} " \
        "factory_state=${factory_state} usb_update_req=${usb_update_req} " \
        "secure_board=${secure_board} " \
        "${mtdparts} ${optargs}\0" \
    "swu_nand_boot=echo \"Booting swu from nand ...\"; " \
        "run swunandargs; " \
        "echo \"INFO: loading swu fit image into RAM...\"; " \
        "bstate dontunplug; " \
        "run readswufitimage; " \
        "echo \"INFO: booting swu fit image...\"; " \
        "run bootfitimage;\0" \
\
\
    "check_factory_state=" \
       "echo \"INFO: Checking if fit and u-boot-env partitions are empty.\"; " \
       "setenv target_addr ${swu_load_addr}; " \
       "setenv factory_state 1; " \
       "mw ${swu_load_addr} 0xffffffff; " \
       "setexpr target_addr ${target_addr} + 4; " \
       "setexpr target_addr ${target_addr} + 4; " \
       "for part in fit u-boot-env; " \
           "do; " \
           "nand read $part ${target_addr} 0 4; " \
           "cmp.l ${swu_load_addr} ${target_addr} 1; " \
               "if test $? -eq 1; " \
                   "then; " \
                   "setenv factory_state 0; " \
                   "echo \"INFO: partition $part is not empty.\"; " \
               "fi; " \
       "done; " \
       "if test ${factory_state} -eq 0; " \
           "then " \
           "echo \"INFO: Board is NOT in factory state.\"; " \
           "ubi part data;" \
           "if ubifsmount nsdk-rootfs; then " \
		"echo \"Regular image detected.\"; " \
           "else " \
		"echo \"Factory image detected.\"; " \
		"setenv nandroot 'ubi0 rw'; " \
		"setenv fit_config factory; " \
	   "fi; " \
       "else " \
           "echo \"INFO: Board is in factory state.\"; " \
       "fi;\0" \
\
\
    "check_usb_update_request=" \
        "if fwup usb_update_req; then " \
            "echo \"INFO: USB update request is active\"; " \
            "setenv usb_update_req 1; " \
        "else " \
            "echo \"INFO: USB update request is NOT active\"; " \
            "setenv usb_update_req 0; " \
        "fi;\0" \
\
\
    "swu_boot=" \
        "if fwup fail; " \
            "then " \
            "if test ${bootcount} -gt ${bootlimit}; " \
                "then " \
                "echo \"INFO: bootcount(${bootcount}) greater than bootlimit(${bootlimit})\"; " \
                "bstate hardfailure; " \
                "echo \"ERROR: Maximum boot count reached!\"; " \
                "while true; do sleep 100; done; " \
            "fi; " \
        "fi; " \
        "boot_swupdate=no; " \
        "if test ${bootcount} -eq 1; " \
            "then " \
            "echo \"INFO: Bootcount = 1, checking if board is in factory state or USB update request is active\"; " \
            "run check_factory_state; " \
            "if test ${factory_state} = 1; " \
                "then " \
                "boot_swupdate=yes; " \
            "else " \
                "" \
                "if test ${usb_update_req} = 1; " \
                    "then " \
                    "boot_swupdate=yes; " \
                "fi; " \
            "fi; " \
        "else " \
            "echo \"INFO: Bootcount != 1, not checking factory state or USB update request\"; " \
        "fi; " \
        "if fwup fail; " \
            "then " \
            "echo \"INFO: Fail flag is set\"; " \
            "boot_swupdate=yes; " \
        "fi; " \
        "if fwup update; " \
            "then " \
            "echo \"INFO: Update flag is set\"; " \
            "boot_swupdate=yes; " \
        "fi; " \
        "if test ${boot_swupdate} = yes; " \
            "then " \
            "echo \"INFO: Booting swupdate image\"; " \
            "bstate dontunplug; " \
            "run swu_nand_boot; " \
        "else " \
            "echo \"INFO: Booting regular image\"; " \
            "echo \"INFO: Setting fail flag...\"; " \
            "fwup set fail; " \
            "bstate normal; " \
            "run nand_boot; " \
        "fi;\0" \

#define SUE_FWUPDATE_BOOTCOMMAND \
        "echo \"INFO: attempting SWU boot...\"; " \
        "fwup flags; "\
        "run swu_boot;" \

#define SUE_FWUPDATE_ALTBOOTCOMMAND \
        "echo \"ERROR: Maximum boot count reached!\"; while true; do sleep 100; done; "

#endif /* __SUE_FWUPDATE_COMMON_H */
