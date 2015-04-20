/*
 * Amlogic Apollo
 * frame buffer driver
 *
 * Copyright (C) 2009 Amlogic, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:	Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef VINFO_H
#define VINFO_H

//the MSB is represent vmode set by logo
#define	VMODE_LOGO_BIT_MASK	0x8000	
#define	VMODE_MODE_BIT_MASK	0xff		
typedef enum {
    VMODE_480I  = 0,
    VMODE_480CVBS,		
    VMODE_480P  ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
    VMODE_480P_59HZ,// for framerate automation 480p 59.94hz
#endif
    VMODE_576I   ,
    VMODE_576CVBS   ,
    VMODE_576P  ,
    VMODE_720P  ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
	VMODE_720P_59HZ , // for framerate automation 720p 59.94hz
#endif
    VMODE_1080I ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
    VMODE_1080I_59HZ , // for framerate automation 1080i 59.94hz
#endif
    VMODE_1080P ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
	VMODE_1080P_59HZ , // for framerate automation 1080p 59.94hz
#endif
    VMODE_720P_50HZ ,
    VMODE_1080I_50HZ ,
    VMODE_1080P_50HZ ,
    VMODE_1080P_24HZ ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
	VMODE_1080P_23HZ , // for framerate automation 1080p 23.97hz
#endif
    VMODE_4K2K_30HZ ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
	VMODE_4K2K_29HZ , // for framerate automation 4k2k 29.97hz
#endif
    VMODE_4K2K_25HZ ,
    VMODE_4K2K_24HZ ,
#ifdef CONFIG_AML_VOUT_FRAMERATE_AUTOMATION
	VMODE_4K2K_23HZ , // for framerate automation 4k2k 23.97hz
#endif
    VMODE_4K2K_SMPTE,
    VMODE_4K2K_FAKE_5G,   // timing same as 4k2k30hz, Vsync from 30hz to 50hz
    VMODE_4K2K_60HZ,	  // timing same as 4k2k30hz, Vsync from 30hz to 60hz
    VMODE_4K2K_60HZ_Y420,
    VMODE_4K2K_50HZ,	  // timing same as 4k2k25hz, Vsync from 25hz to 50hz
    VMODE_4K2K_50HZ_Y420,
    VMODE_4K2K_5G,
    VMODE_VGA,
    VMODE_SVGA,
    VMODE_XGA,
    VMODE_SXGA,
    VMODE_WSXGA,
    VMODE_FHDVGA,
    VMODE_LCD,
    VMODE_LVDS_1080P,
    VMODE_LVDS_1080P_50HZ,
    VMODE_LVDS_768P,
    VMODE_MAX,
    VMODE_INIT_NULL,
    VMODE_MASK = 0xFF,
} vmode_t;

typedef struct {
	char  		*name;
	vmode_t		mode;
	u32			width;
	u32			height;
	u32			field_height;
	u32			aspect_ratio_num;
	u32			aspect_ratio_den;
	u32			sync_duration_num;
	u32			sync_duration_den;
	u32			screen_real_width;
	u32			screen_real_height;
} vinfo_t;

#endif /* TVMODE_H */
