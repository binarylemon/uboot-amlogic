/*
 * Amlogic Meson driver-----------HDMI_TX
 * Copyright (C) 2013 Amlogic, Inc.
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

#if MESON_CPU_TYPE == MESON_CPU_TYPE_G9TV
#include <asm/arch/osd_hw_def.h>
#endif

#if MESON_CPU_TYPE >= MESON_CPU_TYPE_G9TV
extern void hdmi_tx_set(HDMI_Video_Codes_t vic, int mode420, int colord);
#else
extern void hdmi_tx_set(HDMI_Video_Codes_t vic);
#endif
/*
 * Init hdmi power
 */
void init_hdmi(void)
{
    extern void hdmi_tx_power_init(void);
    hdmi_tx_power_init();
}

/*
 * the parameters mode from tv setting should be convert to HDMI VIC
 */
static unsigned int tvmode_vmode_vic_map[][3] = {
    {TVMODE_480I, VMODE_480I, HDMI_480i60_16x9},
    {TVMODE_480I, VMODE_480I, HDMI_480i60},
    {TVMODE_480CVBS, VMODE_480I, HDMI_480i60},
    {TVMODE_480P, VMODE_480P, HDMI_480p60_16x9},
    {TVMODE_480P, VMODE_480P, HDMI_480p60},
    {TVMODE_576I, VMODE_576I, HDMI_576i50_16x9},
    {TVMODE_576I, VMODE_576I, HDMI_576i50},
    {TVMODE_576CVBS, VMODE_576I, HDMI_576i50},
    {TVMODE_576P, VMODE_576P, HDMI_576p50_16x9},
    {TVMODE_576P, VMODE_576P, HDMI_576p50},
    {TVMODE_720P, VMODE_720P, HDMI_720p60},
    {TVMODE_1080I, VMODE_1080I, HDMI_1080i60},
    {TVMODE_1080P, VMODE_1080P, HDMI_1080p60},
    {TVMODE_720P_50HZ, VMODE_720P_50HZ, HDMI_720p50},
    {TVMODE_1080I_50HZ, VMODE_1080I_50HZ, HDMI_1080i50},
    {TVMODE_1080P_50HZ, VMODE_1080P_50HZ, HDMI_1080p50},
    {TVMODE_1080P_24HZ, VMODE_1080P_24HZ, HDMI_1080p24},
    {TVMODE_4K2K_30HZ, VMODE_4K2K_30HZ, HDMI_4k2k_30},
    {TVMODE_4K2K_25HZ, VMODE_4K2K_25HZ, HDMI_4k2k_25},
    {TVMODE_4K2K_24HZ, VMODE_4K2K_24HZ, HDMI_4k2k_24},
    {TVMODE_4K2K_SMPTE, VMODE_4K2K_SMPTE, HDMI_4k2k_smpte_24},
    {TVMODE_4K2K_60HZ_Y420, VMODE_4K2K_60HZ_Y420, HDMI_3840x2160p60_16x9},
    {TVMODE_4K2K_50HZ_Y420, VMODE_4K2K_50HZ_Y420, HDMI_3840x2160p50_16x9},
    {TVMODE_MAX, VMODE_MAX, HDMI_Unkown},
};

HDMI_Video_Codes_t tvmode_to_vic(int mode)
{
    HDMI_Video_Codes_t vic = HDMI_Unkown;
    int i = 0;
    while(tvmode_vmode_vic_map[i][0] != TVMODE_MAX) {
        if(tvmode_vmode_vic_map[i][0] == mode) {
            vic = tvmode_vmode_vic_map[i][2];
            break;
        }
        i ++;
    }
    return vic;
}

tvmode_t vic_to_tvmode(HDMI_Video_Codes_t vic)
{
    tvmode_t tvmode = TVMODE_MAX;
    int i = 0;
    while(tvmode_vmode_vic_map[i][2] != HDMI_Unkown) {
        if(tvmode_vmode_vic_map[i][2] == vic) {
            tvmode = tvmode_vmode_vic_map[i][0];
            break;
        }
        i ++;
    }
    return tvmode;
}

vmode_t vic_to_vmode(HDMI_Video_Codes_t vic)
{
    vmode_t vmode = VMODE_INIT_NULL;
    int i = 0;
    while(tvmode_vmode_vic_map[i][2] != HDMI_Unkown) {
        if(tvmode_vmode_vic_map[i][2] == vic) {
            vmode = tvmode_vmode_vic_map[i][1];
            break;
        }
        i ++;
    }
    return vmode;
}

/*
 * set hdmi format
 */
int set_disp_mode(int mode)
{
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_G9TV
    int mode420 = 0;
    int colord = 0;
#endif
    HDMI_Video_Codes_t vic;
    if(mode >= TVMODE_MAX) {
        printf("Invalid hdmi mode %d\n", mode);
        return 0;
    }
    vic = tvmode_to_vic(mode);
    printf("mode = %d  vic = %d\n", mode, vic);
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_G9TV
    if((mode == TVMODE_4K2K_60HZ_Y420) || (mode == TVMODE_4K2K_50HZ_Y420))
        mode420 = 1;
    else
        mode420 = 0;
    hdmi_tx_set(vic, mode420, colord);
#else
    hdmi_tx_set(vic);
#endif
    return 1;
}    

//TODO Delete later
extern void osd1_update_coef(void){}
extern void osd1_update_disp_freescale_enable(void){}
extern void osd2_update_coef(void){}
extern void osd2_update_disp_freescale_enable(void){}
