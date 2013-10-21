/*
 * \file        optimus_sdc_burn.c
 * \brief       burning itself from Pheripheral tf/sdmmc card
 *
 * \version     1.0.0
 * \date        2013-7-11
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2013 Amlogic. All Rights Reserved.
 *
 */
#include "optimus_sdc_burn_i.h"

static int is_bootloader_old(void)
{
    int sdc_boot = is_tpl_loaded_from_ext_sdmmc();

    return !sdc_boot;
}

#define VERIFY_FILE_NOT_EXIST   0x55 

int optimus_verify_partition(const char* partName, HIMAGE hImg, char* _errInfo)
{
#define MaxSz (64 - 7) //verify file to at most 64B to corresponding to USB burn, strlen("verify ") == 7

    char* argv[4];
    int ret = 0;
    HIMAGEITEM hImgItem = NULL;
    int imgItemSz = 0;
    char CmdVerify[MaxSz + 7] = {0};

    hImgItem = image_item_open(hImg, "VERIFY", partName);
    if(!hImgItem){
        DWN_ERR("Fail to open verify file for part (%s)\n", partName);
        return VERIFY_FILE_NOT_EXIST;
    }

    imgItemSz = (int)image_item_get_size(hImgItem);
    if(imgItemSz > MaxSz || !imgItemSz){
        DWN_ERR("verify file size %d for part %s invalid, max is %d\n", imgItemSz, partName, MaxSz);
        ret = __LINE__; goto _finish;
    }
    DWN_DBG("item sz %u\n", imgItemSz);

    ret = image_item_read(hImg, hImgItem, CmdVerify, imgItemSz);
    if(ret){
        DWN_ERR("Fail to read verify item for part %s\n", partName);
        goto _finish;
    }
    CmdVerify[imgItemSz] = 0;
    DWN_DBG("verify[%s]\n", CmdVerify);

    argv[0] = "verify";
    ret = parse_line(CmdVerify, argv + 1);
    if(ret != 2){
        DWN_ERR("verify cmd argc must be 2, but %d\n", ret);
        return __LINE__;
    }

    ret = optimus_media_download_verify(3, argv, _errInfo);
    if(ret){
        DWN_ERR("Fail when verify\n");
        return __LINE__;
    }

_finish:
    image_item_close(hImgItem);
    return ret;
}

int optimus_burn_one_partition(const char* partName, HIMAGE hImg, __hdle hUiProgress)
{
    int rcode = 0;
    s64 imgItemSz       = 0;
    s64 leftItemSz      = 0;
    u32 thisReadLen     = 0;
    __hdle hImgItem     = NULL;
    char* downTransBuf  = NULL;//get buffer from optimus_buffer_manager
    const unsigned ItemReadBufSz = OPTIMUS_DOWNLOAD_SLOT_SZ;//read this size from image item each time
    unsigned sequenceNo = 0;
    const char* fileFmt = NULL;
    /*static */char _errInfo[512];

    printf("\n");
    DWN_MSG("=====>To burn part [%s]\n", partName);
    hImgItem = image_item_open(hImg, "PARTITION", partName);
    if(!hImgItem){
        DWN_ERR("Fail to open item for part (%s)\n", partName);
        return __LINE__;
    }

    imgItemSz = leftItemSz = image_item_get_size(hImgItem);
    if(!imgItemSz){
        DWN_ERR("image size is 0 , image of part (%s) not exist ?\n", partName);
        return __LINE__;
    }

    fileFmt = (IMAGE_ITEM_TYPE_SPARSE == image_item_get_type(hImgItem)) ? "sparse" : "normal";

    rcode = sdc_burn_buf_manager_init(partName, imgItemSz, fileFmt);
    if(rcode){
        DWN_ERR("fail in sdc_burn_buf_manager_init, rcode %d\n", rcode);
        return __LINE__;
    }

    //for each loop: 
    //1, get buffer from buffer_manager, 
    //2, read item data to buffer, 
    //3, report data ready to buffer_manager
    for(; leftItemSz > 0; leftItemSz -= thisReadLen, sequenceNo++)
    {
        thisReadLen = leftItemSz > ItemReadBufSz ? ItemReadBufSz: (u32)leftItemSz;

        rcode = optimus_buf_manager_get_buf_for_bulk_transfer(&downTransBuf, thisReadLen, sequenceNo, _errInfo);
        if(rcode){
            DWN_ERR("fail in get buf, msg[%s]\n", _errInfo);
            goto _finish;
        }

        rcode = image_item_read(hImg, hImgItem, downTransBuf, thisReadLen);
        if(rcode){
            DWN_ERR("fail in read data from item,rcode %d\n", rcode);
            goto _finish;
        }

        rcode = optimus_buf_manager_report_transfer_complete(thisReadLen, _errInfo);
        if(rcode){
            DWN_ERR("fail in report data ready, rcode %d\n", rcode);
            goto _finish;
        }
        if(hUiProgress)optimus_progress_ui_update_by_bytes(hUiProgress, thisReadLen);
    }

    DWN_DBG("BURN part %s %s!\n", partName, leftItemSz ? "FAILED" : "SUCCESS");

_finish:
    image_item_close(hImgItem);

    if(rcode){
        DWN_ERR("Fail to burn part(%s) with in format (%s) before verify\n", partName, fileFmt);
        return rcode;
    }

#if 1
    rcode = optimus_verify_partition(partName, hImg, _errInfo);
    if(VERIFY_FILE_NOT_EXIST == rcode)
    {
        printf("WRN:part(%s) NOT verified\n", partName);
        return 0;
    }
    if(rcode) {
        printf("Fail in verify part(%s)\n", partName);
        return __LINE__;
    }
#endif//#fi 0

    return rcode;

}

int optimus_sdc_burn_partitions(ConfigPara_t* pCfgPara, HIMAGE hImg, __hdle hUiProgress)
{
    BurnParts_t* cfgParts = &pCfgPara->burnParts;
    int burnNum       = cfgParts->burn_num;
    int i = 0;
    int rcode = 0;

    //update burn_parts para if burnNum is 0, i.e, not configured
    if(!burnNum)
    {
        rcode = get_burn_parts_from_img(hImg, pCfgPara);
        if(rcode){
            DWN_ERR("Fail to get burn parts from image\n");
            return __LINE__;
        }
        burnNum = cfgParts->burn_num;
        DWN_MSG("Data part num %d\n", burnNum);
    }
    if(!burnNum){
        DWN_ERR("Data part num is 0!!\n");
        return __LINE__;
    }

    for (i = 0; i < burnNum; i++) 
    {
        const char* partName = cfgParts->burnParts[i];

        rcode = optimus_burn_one_partition(partName, hImg, hUiProgress);
        if(rcode){
            DWN_ERR("Fail in burn part %s\n", partName);
            return __LINE__;
        }
    }

    return rcode;
}

//not need to verify as not config ??
int optimus_sdc_burn_media_partition(const char* mediaImgPath, const char* verifyFile)
{
    return optimus_burn_partition_image("media", mediaImgPath, "normal", verifyFile);
}

int optimus_burn_bootlader(HIMAGE hImg)
{
    int rcode = 0;

    rcode = optimus_burn_one_partition("bootloader", hImg, NULL);
    if(rcode){
        DWN_ERR("Fail when burn bootloader\n");
        return __LINE__;
    }
    
    return rcode;
}

//flag, 0 is burn completed, else burn failed
int optimus_report_burn_complete_sta(int flag)
{
    if(flag)
    {
        DWN_MSG("======sdc burn Failed!!!!!\n");
        DWN_MSG("PLS long-press power key to shut down\n");
        while(1){
            /*if(ctrlc())*/
        }

        return __LINE__;
    }

    DWN_MSG("======sdc burn SUCCESS.\n");
    optimus_burn_complete(2);//set complete flag and poweroff if burn successful
    return flag;
}

int optimus_burn_with_cfg_file(const char* cfgFile)
{
    extern ConfigPara_t g_sdcBurnPara ;

    int ret = 0;
    s64 fileSz = 0;
    HIMAGE hImg = NULL;
    ConfigPara_t* pSdcCfgPara = &g_sdcBurnPara;
    const char* pkgPath = pSdcCfgPara->burnEx.pkgPath;
    __hdle hUiProgress = NULL;

    ret = parse_ini_cfg_file(cfgFile);
    if(ret){
        DWN_ERR("Fail to parse file %s\n", cfgFile);
        return __LINE__;
    }

    fileSz = do_fat_get_fileSz(pkgPath);
    if(!fileSz){
        DWN_ERR("package [%s] not exist in external sdcard\n", pkgPath);
        return __LINE__;
    }
    DWN_MSG("image sz 0x%llx\n", fileSz);

    if(pSdcCfgPara->custom.eraseBootloader)
    {
        if(is_bootloader_old())
        {
            DWN_MSG("To erase OLD bootloader in Internal flash!\n");
            ret = optimus_erase_bootloader(NULL);
            if(ret){
                DWN_ERR("Fail to erase bootloader\n");
                return __LINE__;
            }

            DWN_MSG("Clear env upgrade_step to before reset\n");
            ret = run_command("defenv;saveenv", 0);
            if(ret){
                DWN_ERR("Fail to defenv;saveenv to flash, before reset\n");
                return __LINE__;
            }

            DWN_MSG("Reset to load NEW bootloader from external flash!\n");
            optimus_reset();
            return __LINE__;//should never reach here!!
        }
    }

    hImg = image_open(pkgPath);
    if(!hImg){
        DWN_ERR("Fail to open image %s\n", pkgPath);
        return __LINE__;
    }

    ret = show_logo_to_report_burning(hImg);
    if(ret){
        DWN_ERR("Fail to show burning logo, ret %d\n", ret);
        ret = __LINE__; goto _finish;
    }
    hUiProgress = optimus_progress_ui_request_for_sdc_burn();
    if(!hUiProgress){
        DWN_ERR("request progress handle failed!\n");
        ret = __LINE__; goto _finish;
    }
    optimus_progress_ui_direct_update_progress(hUiProgress, UPGRADE_STEPS_AFTER_IMAGE_OPEN_OK);

    ret = optimus_storage_init(pSdcCfgPara->custom.eraseFlash);
    if(ret){
        DWN_ERR("Fail to init stoarge for sdc burn\n");
        return __LINE__;
    }

    optimus_progress_ui_direct_update_progress(hUiProgress, UPGRADE_STEPS_AFTER_DISK_INIT_OK);
    ret = optimus_progress_ui_set_smart_mode(hUiProgress, get_data_parts_size(hImg), 
                UPGRADE_STEPS_FOR_BURN_DATA_PARTS_IN_PKG(!pSdcCfgPara->burnEx.bitsMap.mediaPath));
    if(ret){
        DWN_ERR("Fail to set smart mode\n");
        ret = __LINE__; goto _finish;
    }
    
    ret = optimus_sdc_burn_partitions(pSdcCfgPara, hImg, hUiProgress);
    if(ret){
        DWN_ERR("Fail when burn partitions\n");
        ret = __LINE__; goto _finish;
    }

    if(pSdcCfgPara->burnEx.bitsMap.mediaPath)//burn media image
    {
        const char* mediaPath = pSdcCfgPara->burnEx.mediaPath;

        ret = optimus_sdc_burn_media_partition(mediaPath, NULL);//no progress bar info if have partition image not in package
        if(ret){
            DWN_ERR("Fail to burn media partition with image %s\n", mediaPath);
            optimus_storage_exit();
            return __LINE__;
        }
    }
    optimus_progress_ui_direct_update_progress(hUiProgress, UPGRADE_STPES_AFTER_BURN_DATA_PARTS_OK);

#if 1
    //burn bootloader
    ret = optimus_burn_bootlader(hImg);
    if(ret){
        DWN_ERR("Fail in burn bootloader\n");
        goto _finish;
    }
#endif
    optimus_progress_ui_direct_update_progress(hUiProgress, UPGRADE_STEPS_AFTER_BURN_BOOTLOADER_OK);

_finish:
    image_close(hImg);
    optimus_progress_ui_report_upgrade_stat(hUiProgress, !ret);
    optimus_report_burn_complete_sta(ret);
    //optimus_storage_exit();//temporary not exit storage driver when failed as may continue burning after burn
    return ret;
}

int optimus_burn_package_in_sdmmc(const char* sdc_cfg_file)
{
    int rcode = 0;

    DWN_MSG("mmcinfo\n");
    rcode = run_command("mmcinfo", 0);
    if(rcode){
        DWN_ERR("Fail in init mmc, Does sdcard not plugged in?\n");
        return __LINE__;
    }

    rcode = do_fat_get_fileSz(sdc_cfg_file);
    if(!rcode){
        printf("The [%s] not exist in bootable mmc card\n", sdc_cfg_file);
        return __LINE__;
    }

    rcode = optimus_burn_with_cfg_file(sdc_cfg_file);

    return rcode;
}

int do_sdc_burn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rcode = 0;
    const char* sdc_cfg_file = argv[1];

    if(argc < 2 ){
        cmd_usage(cmdtp);
        return __LINE__;
    }

    rcode = optimus_burn_package_in_sdmmc(sdc_cfg_file);

    return rcode;
}

U_BOOT_CMD(
   sdc_burn,      //command name
   5,               //maxargs
   0,               //repeatable
   do_sdc_burn,   //command function
   "Burning with amlogic format package in sdmmc ",           //description
   "argv: [sdc_burn_cfg_file]\n"//usage
   "    -aml_sdc_burn.ini is usually used configure file\n"
);

