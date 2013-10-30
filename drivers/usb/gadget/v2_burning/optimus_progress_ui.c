/*
 * \file        optimus_progress_ui.c
 * \brief       Show progress info to UI
 *
 * \version     1.0.0
 * \date        2013/10/13
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2013 Amlogic Inc.. All Rights Reserved.
 *
 */
#include <common.h>
#include <config.h>
#include <malloc.h>
#include <bmp_layout.h>
#include "optimus_download.h"
#include "amlImage_if.h"
#include "optimus_progress_ui.h"

#include <lcd.h>
#include <video_font.h>
extern int lcd_drawchars (ushort x, ushort y, uchar *str, int count);

//default env for display UI
const char* env_video_prepare_for_upgrade = "echo video prepare for upgrade; "\
                            "video open; "\
                            "video clear; "\
                            "video dev bl_on; "\
                            /* twice clear */ "video clear; ";

const char* env_ui_report_burning = "bmp display ${upgrade_upgrading_offset}";

const char* env_ui_report_burn_failed = "bmp display ${upgrade_fail_offset}";
const char* env_ui_report_burn_error = "bmp display ${upgrade_error_offset}";
const char* env_ui_report_burn_success = "bmp display ${upgrade_success_offset}";

//if env upgrade_logo_offset exist, use bmp resources existed in memory
int video_res_prepare_for_upgrade(HIMAGE hImg)
{
    char* env_name  = NULL;
    const char* env_value = NULL;
    int ret = 0;

    env_value = "unpackimg ${loadaddr_misc}";
    ret = run_command(env_value, 0);
    if(ret)
    {//Failed to load logo resources from memory, then Load it from package
        void* resAddr      = (void*)simple_strtoul(getenv("loadaddr_misc"), NULL, 0);
        unsigned imgItemSz = 0;
        HIMAGEITEM hItem = NULL;

        DWN_MSG("Use upgrade res in pkg\n");
        hItem = image_item_open(hImg, "PARTITION", "logo");
        if(!hItem){
            DWN_ERR("Fail to get logo.PARTITION for display logo\n");
            return __LINE__;
        }

        imgItemSz = (unsigned)image_item_get_size(hItem);
        ret = image_item_read(hImg, hItem, resAddr, imgItemSz);
        if(ret){
            DWN_ERR("Fail to read item logo\n");
            image_item_close(hItem); return __LINE__;
        }
        image_item_close(hItem);

        ret = run_command(env_value, 0);
        if(ret){
            DWN_ERR("Exception: Fail to unpack image in the package.\n");
            return __LINE__;
        }
    }

    //video prepare to show upgrade bmp
    {
        char env_buf[32];

        env_name = "burn_logo_prepare";
        if(setenv(env_name, (char*)env_video_prepare_for_upgrade)){
            DWN_ERR("Fail to set env(%s=%s)\n", env_name, env_video_prepare_for_upgrade);
            return __LINE__;
        }
        sprintf(env_buf, "run %s", env_name);
        ret = run_command(env_buf, 0);
        if(ret){
            DWN_ERR("Fail in run_command[%s]\n", env_buf);
            return __LINE__;
        }

    }

    return 0;
}

//Display logo to report platform is in burning state
int show_logo_to_report_burning(void)
{
    int ret = 0;

    ret = run_command(env_ui_report_burning, 0);
    if(ret){
        DWN_ERR("Fail in display burning logo\n");
        return __LINE__;
    }

    return 0;
}

static int _show_burn_logo(const char* bmpCmd) //Display logo to report burning result is failed
{
    int ret = 0;

    ret = run_command(bmpCmd, 0);
    if(ret){
        DWN_ERR("Fail in run[%s]\n", bmpCmd);
        return __LINE__;
    }

    return 0;
}

int show_logo_to_report_burn_success(void)
{
    int ret = 0;

    ret = run_command(env_ui_report_burn_success, 0);
    if(ret){
        DWN_ERR("Fail in display logo upgrade_success_offset\n");
        return __LINE__;
    }
    lcd_printf("PLS short-press the power key to shut down\n");

    return 0;
}

static int show_logo_report_burn_ui_error(void)
{
    int ret = 0;

    ret = run_command(env_ui_report_burn_error, 0);
    if(ret){
        DWN_ERR("Fail in display logo upgrade_error\n");
        return __LINE__;
    }

    return 0;
}

//Horizontal progress bar
//_f means not changed after initialized
typedef struct _uiProgress{
    //For smart mode
    u64         smartModeTotalBytes_f;//fixed
    u64         smartModeLeftBytes;

    unsigned    nDownBytesOnePercent_f;//update 1% if .nDownBytesOnePercent, (==.totalDownloadBytes/totalPercents)

//For total
    int         totalPercents_f; //Total width for display the progress bar, should < 'display_width'

    int         curPercent;//curent percent of the progress
    int         endPercent_f;//.endPercent + 1 - .startPerCent = .totalPercents, this field is fixed

    int         progressBarWidth_f;//pixel width of the progress bar bmp
    int         progressBarHeight_f;//pixel height of the progress bar bmp

    unsigned    totalProgressBarWidth_f;//total pixel width of the progress bar, use 'display_width'/100*100 ?
    unsigned    nProgressBarOnePercent_f;//times to display bar.if .totalProgressBarWidth=2000, .progressBarWidth=2, .totalPercents=100, then .nProgressBarOnePercent=10

    int         nextProgressBarX;
    int         progressBarY_f;//As this is a horizon progress bar, y coordinate is fixed.

    int         upgradeStepX_f;
    int         upgradeStepY_f;

    unsigned    bmpAddr_f;
    unsigned    reservToAlign64;
}UiProgress_t;


__hdle optimus_progress_ui_request(int totalPercents_f, int startPercent, unsigned bmpBarAddr,       
                                    int display_width,  int progressBarY_f )
{
    UiProgress_t* pUiProgress = NULL;
    bmp_header_t* bmpHeadInfo = (bmp_header_t*)bmpBarAddr;
    const unsigned commonMultiple = totalPercents_f * bmpHeadInfo->width;

    if(display_width < commonMultiple){
        DWN_ERR("pic width too large!! width(%d) * totalPercents_f(%d) >= display_width(%d)\n", 
                bmpHeadInfo->width, totalPercents_f, display_width);
        show_logo_report_burn_ui_error(); return NULL;
    }
    pUiProgress = (UiProgress_t*)malloc(sizeof(UiProgress_t));
    if(!pUiProgress){
        DWN_ERR("Fail when malloc for UiProgress_t\n");
        show_logo_report_burn_ui_error(); return NULL;
    }

    pUiProgress->totalPercents_f            = totalPercents_f;
    pUiProgress->curPercent                 = startPercent;
    pUiProgress->endPercent_f               = startPercent + totalPercents_f;

    
    pUiProgress->bmpAddr_f                  = bmpBarAddr;
    pUiProgress->progressBarWidth_f         = bmpHeadInfo->width;
    pUiProgress->progressBarHeight_f        = bmpHeadInfo->height;
    DWN_MSG("w,h[%u,%u]\n", pUiProgress->progressBarWidth_f, pUiProgress->progressBarHeight_f);

    pUiProgress->totalProgressBarWidth_f    = /* common multiple of totalPercents_f and progress bar width */
                     (display_width / commonMultiple) * commonMultiple;
    pUiProgress->nProgressBarOnePercent_f   = pUiProgress->totalProgressBarWidth_f / totalPercents_f;

    pUiProgress->nextProgressBarX            = display_width - pUiProgress->totalProgressBarWidth_f;
    pUiProgress->nextProgressBarX           /= 2;

    pUiProgress->progressBarY_f             = progressBarY_f;
    DWN_DBG("barYCor %d\n", progressBarY_f);

    pUiProgress->upgradeStepX_f              = pUiProgress->nextProgressBarX;
    pUiProgress->upgradeStepY_f              = progressBarY_f - VIDEO_FONT_HEIGHT;
    DWN_DBG("upgradeStepX_f %d, upgradeStepY_f %d\n", pUiProgress->upgradeStepX_f, pUiProgress->upgradeStepY_f);

    DWN_DBG("totalProgressBarWidth_f %d, nProgressBarOnePercent_f %d, nextProgressBarX %d\n", 
            pUiProgress->totalProgressBarWidth_f, pUiProgress->nProgressBarOnePercent_f, pUiProgress->nextProgressBarX);

    return (__hdle)pUiProgress;
}

int optimus_progress_ui_set_smart_mode(__hdle hUiProgress, const u64 smartModeTotalBytes_f, const unsigned smartModePercents)
{
    UiProgress_t* pUiProgress   = (UiProgress_t*)hUiProgress;

    if(!smartModeTotalBytes_f || !smartModePercents){
        DWN_ERR("arg error!, smartModeTotalBytes_f %lld, smartModePercents %u\n", smartModeTotalBytes_f, smartModePercents);
        show_logo_report_burn_ui_error(); 
        return __LINE__;
    }
    pUiProgress->smartModeTotalBytes_f     = smartModeTotalBytes_f;
    pUiProgress->nDownBytesOnePercent_f     = smartModeTotalBytes_f/smartModePercents;
    DWN_DBG("nDownBytesOnePercent_f %d\n", pUiProgress->nDownBytesOnePercent_f);
    pUiProgress->smartModeLeftBytes     = 0;

    return 0;
}

int optimus_progress_ui_set_unfocus_bkg(__hdle hUiProgress, unsigned unfocusBmpAddr)
{
    UiProgress_t* pUiProgress   = (UiProgress_t*)hUiProgress;
    bmp_header_t* bkgBmpHead    = (bmp_header_t*)unfocusBmpAddr;
    const unsigned barHeight    = pUiProgress->progressBarHeight_f;
    const unsigned bkgHeight    = bkgBmpHead->height;
    const unsigned bkgWidth     = bkgBmpHead->width;
    const int nProgressBar      = pUiProgress->totalPercents_f
                    * pUiProgress->nProgressBarOnePercent_f / bkgWidth;
    int progressBarX            = pUiProgress->nextProgressBarX;
    const int progressBarY      = pUiProgress->progressBarY_f;

    int i = 0;

    //allow width not equal, but height must equal!
    if(barHeight != bkgHeight){
        DWN_ERR("barHeight %d != bkgHeight %d\n", barHeight, bkgHeight);
        show_logo_report_burn_ui_error(); 
        return __LINE__;
    }

    //show the progress bar to update progress to video device
    for(i=0; i < nProgressBar; ++i)
    {
        char cmd[64];

        sprintf(cmd, "bmp display %x %d %d", unfocusBmpAddr, progressBarX, progressBarY);
        if(run_command(cmd, 0)){
            DWN_ERR("Fail to in cmd[%s]\n", cmd);
            show_logo_report_burn_ui_error(); 
            return __LINE__;
        }
       
        progressBarX += bkgWidth;
    }

    return 0;
}

static int optimus_progress_ui_set_steps(__hdle hUiProgress, int steps)
{ 
    UiProgress_t* pUiProgress   = (UiProgress_t*)hUiProgress;
    const int curPercent        = pUiProgress->curPercent;
    char strStep[16];

    sprintf(strStep, "%d%%", steps);
    lcd_drawchars((ushort)pUiProgress->upgradeStepX_f, (ushort)pUiProgress->upgradeStepY_f, (uchar*)strStep, strlen(strStep));

    if(UPGRADE_STEPS_AFTER_IMAGE_OPEN_OK == curPercent)
    {
        lcd_printf("[OK]Open image\n");
    }
    if(UPGRADE_STEPS_AFTER_DISK_INIT_OK == curPercent)
    {
        lcd_printf("[OK]Disk initial\n");
    }
    else if(UPGRADE_STPES_AFTER_BURN_DATA_PARTS_OK == curPercent)
    {
        lcd_printf("[OK]Burn Data Partitons\n");
    }
    else if(UPGRADE_STEPS_AFTER_BURN_BOOTLOADER_OK == curPercent)
    {
        lcd_printf("[OK]Burn bootloader\n");
    }

    return 0;
}

int optimus_progress_ui_direct_update_progress(__hdle hUiProgress, const int percents)
{
    UiProgress_t* pUiProgress = (UiProgress_t*)hUiProgress;
    const int     nPercents   = percents - pUiProgress->curPercent;
    const int     nProgressBar= nPercents * pUiProgress->nProgressBarOnePercent_f / pUiProgress->progressBarWidth_f;
    int   i = 0;

    //if already up to this user percents, not need to update yet!
    if(percents > pUiProgress->endPercent_f){
        DWN_ERR("user percents(%d) beyond max (%d)\n", percents, pUiProgress->endPercent_f);
        show_logo_report_burn_ui_error(); 
        return __LINE__;
    }
    if(nPercents <= 0){
        DWN_MSG("curPercent(%d) >= percents(%d)\n", pUiProgress->curPercent, percents);
        show_logo_report_burn_ui_error(); 
        return 0;
    }
    
    DWN_DBG("curPercent %d, percents %d, nPercents %d\n", pUiProgress->curPercent, percents, nPercents);
    DWN_DBG("nProgressBar %d, %d\n", nProgressBar, pUiProgress->progressBarWidth_f);

    //show the progress bar to update progress to video device
    for(i=0; i < nProgressBar; ++i)
    {
        const int progressBarX          = pUiProgress->nextProgressBarX;
        const int progressBarY          = pUiProgress->progressBarY_f;
        const unsigned bmpAddr          = pUiProgress->bmpAddr_f;
        char cmd[64];

        sprintf(cmd, "bmp display %x %d %d", bmpAddr, progressBarX, progressBarY);
        if(run_command(cmd, 0)){
            DWN_ERR("Fail to in cmd[%s]\n", cmd);
            show_logo_report_burn_ui_error(); 
            return __LINE__;
        }
       
        pUiProgress->nextProgressBarX += pUiProgress->progressBarWidth_f;
    }

    pUiProgress->curPercent                 = percents;

    optimus_progress_ui_set_steps(hUiProgress, percents);

    return 0;
}

//intelligent mode, update progress by download bytes
int optimus_progress_ui_update_by_bytes(__hdle hUiPrgress, const unsigned nBytes)
{
    UiProgress_t* pUiProgress   = (UiProgress_t*)hUiPrgress;
    const unsigned nDownBytesOnePercent_f = pUiProgress->nDownBytesOnePercent_f;
    const unsigned bytesNotReport         = nBytes + pUiProgress->smartModeLeftBytes;
    int percentsIncreased       = 0;
    unsigned leftBytes          = 0;
    int         ret             = 0;
    int percentsToReport        = 0;

    //bytes not enghout to update one percent
    if(bytesNotReport < nDownBytesOnePercent_f)
    {
        pUiProgress->smartModeLeftBytes = bytesNotReport;
        return 0;
    }
    percentsIncreased   = bytesNotReport / nDownBytesOnePercent_f;
    leftBytes           = bytesNotReport - percentsIncreased * nDownBytesOnePercent_f;

    percentsToReport    = percentsIncreased + pUiProgress->curPercent;
    DWN_DBG("update ui to [%d%%]\n", percentsToReport);
    ret = optimus_progress_ui_direct_update_progress(hUiPrgress, percentsToReport);
    pUiProgress->smartModeLeftBytes = leftBytes;

    return ret;
}

int optimus_progress_ui_release(__hdle hUiPrgress)
{
    UiProgress_t* pUiProgress = (UiProgress_t*)hUiPrgress;

    DWN_MSG("Release prgress bar res\n");
    if(pUiProgress)
    {
        free(pUiProgress), pUiProgress = NULL;
    }

    return 0;
}

__hdle optimus_progress_ui_request_for_sdc_burn(void)
{
    __hdle hUiProgress = NULL;
    unsigned barAddr = simple_strtoul(getenv("upgrade_bar_offset"), NULL, 0);
    unsigned display_width = simple_strtoul(getenv("display_width"), NULL, 0);
    unsigned display_height = simple_strtoul(getenv("display_height"), NULL, 0);
    bmp_header_t* upgrading  = (bmp_header_t*)simple_strtoul(getenv("upgrade_upgrading_offset"), NULL, 0);
    const unsigned loadingHeight = upgrading->height;
    const unsigned barYCor       = 
        (3* display_height + loadingHeight)/4;//display_height - (display_height/2 - loadingHeight/2)/2;
    unsigned unfocusBmpAddr = simple_strtoul(getenv("upgrade_unfocus_offset"), NULL, 0);

    if(!barAddr){
        DWN_ERR("Fail to getenv[%s=%s]\n", 
                "upgrade_bar_offset", getenv("upgrade_bar_offset")); 
        show_logo_report_burn_ui_error(); 
        return NULL;
    }
    if(!display_width){
        DWN_ERR("Fail to getenv[%s=%s]\n",
                "display_width", getenv("display_width")); 
        show_logo_report_burn_ui_error(); return NULL;
    }
    if(!display_height){
        DWN_ERR("Fail to getenv[%s=%s]\n",
                "display_height", getenv("display_height")); return NULL;
    }
    if(!upgrading){
        DWN_ERR("Fail to getenv[%s=%s]\n",
                "upgrade_upgrading_offset", getenv("upgrade_upgrading_offset")); 
        show_logo_report_burn_ui_error(); return NULL;
    }
    if(!unfocusBmpAddr){
        DWN_ERR("Fail to getenv[%s=%s]\n",
                "upgrade_unfocus_offset", getenv("upgrade_unfocus_offset")); 
        show_logo_report_burn_ui_error(); return NULL;
    }

    hUiProgress = optimus_progress_ui_request(100, 0,barAddr, display_width, barYCor);
    if(!hUiProgress){
        DWN_ERR("Fail to request progress bar\n");
        return NULL;
    }

    if(optimus_progress_ui_set_unfocus_bkg(hUiProgress, unfocusBmpAddr)){
        DWN_ERR("Fail to set bkg\n");
        return NULL;;
    }

    return hUiProgress;
}

int optimus_progress_ui_report_upgrade_stat(__hdle hUiProgress, const int isSuccess)
{
    UiProgress_t* pUiProgress = (UiProgress_t*)hUiProgress;
    const int curPercent      = pUiProgress->curPercent;

    if(isSuccess)
    {
        optimus_progress_ui_direct_update_progress(hUiProgress, 100);
        _show_burn_logo(env_ui_report_burn_success);
        lcd_printf("Burning Success^^\nPLS SHORT-PRESS the power key to shut down\n");
        return 0;
    }

    _show_burn_logo(env_ui_report_burn_failed);
    lcd_printf("[Failed] at ");
    //Followings failure
    if(UPGRADE_STEPS_AFTER_IMAGE_OPEN_OK == curPercent)
    {
        lcd_printf("Disk initial!!\n");
    }
    else if(UPGRADE_STEPS_AFTER_DISK_INIT_OK < curPercent && UPGRADE_STPES_AFTER_BURN_DATA_PARTS_OK > curPercent)
    {
        lcd_printf("Burning Data Partitons[%d%%]\n", curPercent);
    }
    else if(UPGRADE_STPES_AFTER_BURN_DATA_PARTS_OK == curPercent)
    {
        lcd_printf("Burning bootloader\n");
    }
    else if(UPGRADE_STEPS_AFTER_BURN_BOOTLOADER_OK == curPercent)
    {
        lcd_printf("Burn complete\n");
    }
    lcd_printf("PLS LONG-PRESS the power key to shut down\n");

    return 0;
}

int optimus_progress_ui_printf(const char* fmt, ...)
{
	va_list args;
	char buf[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	lcd_printf(buf);
    return 0;
}

#define PROGRESS_BAR_TEST 0
#if PROGRESS_BAR_TEST
static int do_progress_bar_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    //bmp display $upgrade_bar_offset x,y,width
    static __hdle hProgressBar = NULL;
    int percents = 0;

    if(argc < 2){
        cmd_usage(cmdtp);
        return __LINE__;
    }

    if(!strcmp("rel", argv[1])){
        optimus_progress_ui_release(hProgressBar);
        hProgressBar = NULL;
        return 0;
    }

    if(!hProgressBar)
    {
        unsigned barAddr = simple_strtoul(getenv("upgrade_bar_offset"), NULL, 0);
        unsigned display_width = simple_strtoul(getenv("display_width"), NULL, 0);
        unsigned display_height = simple_strtoul(getenv("display_height"), NULL, 0);
        bmp_header_t* upgrading  = (bmp_header_t*)simple_strtoul(getenv("upgrade_upgrading_offset"), NULL, 0);
        const unsigned loadingHeight = upgrading->height;
        const unsigned barYCor       = 
            (3* display_height + loadingHeight)/4;//display_height - (display_height/2 - loadingHeight/2)/2;
        unsigned unfocusBmpAddr = simple_strtoul(getenv("upgrade_unfocus_offset"), NULL, 0);

        if(!barAddr){
            DWN_ERR("Fail to getenv[%s=%s]\n", 
                    "upgrade_bar_offset", getenv("upgrade_bar_offset")); return __LINE__;
        }
        if(!display_width){
            DWN_ERR("Fail to getenv[%s=%s]\n",
                   "display_width", getenv("display_width")); return __LINE__;
        }
        if(!display_height){
            DWN_ERR("Fail to getenv[%s=%s]\n",
                    "display_height", getenv("display_height")); return __LINE__;
        }
        if(!upgrading){
            DWN_ERR("Fail to getenv[%s=%s]\n",
                    "upgrade_upgrading_offset", getenv("upgrade_upgrading_offset")); return __LINE__;
        }
        if(!unfocusBmpAddr){
            DWN_ERR("Fail to getenv[%s=%s]\n",
                    "upgrade_unfocus_offset", getenv("upgrade_unfocus_offset")); return __LINE__;
        }

        hProgressBar = optimus_progress_ui_request(100, 0, barAddr, display_width, barYCor);
        if(!hProgressBar){
            DWN_ERR("Fail to request progress bar\n");
            return __LINE__;
        }

        if(optimus_progress_ui_set_unfocus_bkg(hProgressBar, unfocusBmpAddr)){
            DWN_ERR("Fail to set bkg\n");
            return __LINE__;
        }
        lcd_printf("-----25%%");
    }

    if(!strcmp("dir", argv[1]))
    {
        percents = simple_strtoul(argv[2], NULL, 10);
        optimus_progress_ui_direct_update_progress(hProgressBar, percents);
    }
    else if(!strcmp("nb", argv[1]))
    {
        static u64 dataBytes = 0;
        unsigned nBytes = simple_strtoul(argv[2], NULL, 10);

        if(!dataBytes){
            dataBytes = 2u*1024*1024*1024;
            if(optimus_progress_ui_set_smart_mode(hProgressBar, dataBytes, 90)){
                DWN_ERR("Fail to set smart mode\n");
                return __LINE__;
            }
        }
        optimus_progress_ui_update_by_bytes(hProgressBar, nBytes);
    }


    return 0;
}

U_BOOT_CMD(
   bar,      //command name
   5,               //maxargs
   0,               //repeatable
   do_progress_bar_test,   //command function
   "Test dynamic upgrade progress bar",           //description
   "argv: [percents]\n"//usage
   "    -10 means 10%\n"
);
#endif//#if PROGRESS_BAR_TEST

