/*
 * \file        aml_v2_burning.c
 * \brief       common interfaces for version 2 burning
 *
 * \version     1.0.0
 * \date        09/15/2013
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2013 Amlogic. All Rights Reserved.
 *
 */
#include <common.h>
#include <environment.h>
#include <amlogic/aml_v2_burning.h>
#include <asm/arch/reboot.h>
#include <asm/arch/romboot.h>
#include <mmc.h>
#include <amlogic/aml_lcd.h>
#include "optimus_download.h"

extern int optimus_burn_package_in_sdmmc(const char* sdc_cfg_file);

//check ${sdcburncfg} exist in external mmc and internal flash not burned yet!
int aml_check_is_ready_for_sdc_produce(void)
{
    char* sdc_cfg_file = NULL;
    const char* cmd = NULL;
    int ret = 0;

    if(!is_the_flash_first_burned())//check if upgrade_step == 0
    {
        DWN_MSG("Boot from external mmc but upgrade_step not matched\n");
        return 0;//not ready
    }

    cmd = "mmcinfo 0";
    ret = run_command(cmd, 0);
    if(ret){
        DWN_MSG("mmcinfo failed!\n");
        return 0;//not ready
    }

    sdc_cfg_file = getenv("sdcburncfg");
    if(!sdc_cfg_file) {
        sdc_cfg_file = "aml_sdc_burn.ini";
        setenv("sdcburncfg", sdc_cfg_file);
    }

    cmd = "fatexist mmc 0 ${sdcburncfg}";
    ret = run_command(cmd, 0);
    if(ret){
        DWN_DBG("%s not exist in external mmc\n", sdc_cfg_file);
        return 0;
    }
    
    return 1;//is ready for sdcard producing
}


//is the uboot loaded from usb otg
int is_tpl_loaded_from_usb(void)
{
    return (MESON_USB_BURNER_REBOOT == readl(CONFIG_TPL_BOOT_ID_ADDR));
}

//is the uboot loaded from sdcard mmc 0
//note only sdmmc supported by romcode when external device boot
int is_tpl_loaded_from_ext_sdmmc(void)
{
    /*return (MESON_SDC_BURNER_REBOOT == readl(CONFIG_TPL_BOOT_ID_ADDR));*/
    //see loaduboot.c, only boot from sdcard when "boot_id == 1"
    return (1 == C_ROM_BOOT_DEBUG->boot_id);
}

//Check if uboot loaded from external sdmmc or usb otg
int check_uboot_loaded_for_burn(int flag)
{
    int usb_boot = is_tpl_loaded_from_usb();
    int sdc_boot = is_tpl_loaded_from_ext_sdmmc();

    return usb_boot || sdc_boot;
}

int aml_v2_usb_producing(int flag, bd_t* bis)
{
    flag = flag; bis = bis;//avoid compile warning

    set_default_env("!for usb burn");

    return v2_usbburning(20000);
}

int aml_v2_sdc_producing(int flag, bd_t* bis)
{
    return optimus_burn_package_in_sdmmc(getenv("sdcburncfg"));
}

