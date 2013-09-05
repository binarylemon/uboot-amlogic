/*
 * Amlogic M1 
 * frame buffer driver-----------HDMI_TX
 * Copyright (C) 2010 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "ubi_uboot.h"
#include <common.h>
#include <asm/arch/io.h>
#include <amlogic/aml_tv.h>

#include "hdmi_info_global.h"
#include "hdmi_tx_module.h"

#define DEVICE_NAME "amhdmitx"
#define HDMI_TX_COUNT 32
#define HDMI_TX_POOL_NUM  6
#define HDMI_TX_RESOURCE_NUM 4

#ifdef DEBUG
#define pr_dbg(fmt, args...) printk(KERN_DEBUG "amhdmitx: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif
#define pr_error(fmt, args...) printk(KERN_ERR "amhdmitx: " fmt, ## args)


static hdmitx_dev_t hdmitx_device;

//static HDMI_TX_INFO_t hdmi_info;
#define INIT_FLAG_VDACOFF        0x1
#define INIT_FLAG_POWERDOWN      0x2

#define INIT_FLAG_NOT_LOAD 0x80

static unsigned char init_flag=0;
static unsigned char init_powermode=0;



//#undef DISABLE_AUDIO

void init_hdmi(void)
{
#if 0
    printf("hdmi init\n");
	 HDMITX_M1B_Init(&hdmitx_device);

    	if(hdmitx_device.HWOp.Cntl){
        if(init_flag&INIT_FLAG_VDACOFF){
            hdmitx_device.HWOp.Cntl(&hdmitx_device, HDMITX_HWCMD_VDAC_OFF, 0);    
        }
        if(init_powermode&0x80){
//            hdmitx_device.HWOp.Cntl(&hdmitx_device, HDMITX_HWCMD_LOWPOWER_SWITCH, init_powermode&0x1f);    
        }
    }
#endif
    printf("HDMI Init\n");
    C_Entry();
}

#define PRINT_TEMP_BUF_SIZE 512

int set_disp_mode_auto(int mode)
{
#if 0
    int ret=-1;
   char  mode_name[][16]=
   {
   	"480i","480i","480p","576i","576i","576p","720p","1080i","1080p","invalid"	
   };
    HDMI_Video_Codes_t vic;
    if(mode>=9)
    {
    	printf("invalid tvout mode \n");
	return -1;
    }
	
    vic = hdmitx_edid_get_VIC(&hdmitx_device,mode_name[mode]/* info->name*/, (hdmitx_device.disp_switch_config==DISP_SWITCH_FORCE)?1:0);
    hdmitx_device.cur_VIC = HDMI_Unkown;
    ret = hdmitx_set_display(&hdmitx_device, vic); //if vic is HDMI_Unkown, hdmitx_set_display will disable HDMI
    if(ret>=0){
        hdmitx_device.cur_VIC = vic;    
    }
    C_Entry();
#endif
    return 1;
}    


