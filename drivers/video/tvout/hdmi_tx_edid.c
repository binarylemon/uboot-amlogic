
#include "ubi_uboot.h"
#include <asm/arch/io.h>
#include <amlogic/aml_tv.h>

#include "hdmi_tx_module.h"
#include "hdmi_info_global.h"


#define CEA_DATA_BLOCK_COLLECTION_ADDR_1StP 0x04
#define VIDEO_TAG 0x40
#define AUDIO_TAG 0x20
#define VENDOR_TAG 0x60
#define SPEAKER_TAG 0x80

#define HDMI_EDID_BLOCK_TYPE_RESERVED     0
#define HDMI_EDID_BLOCK_TYPE_AUDIO        1
#define HDMI_EDID_BLOCK_TYPE_VIDEO        2
#define HDMI_EDID_BLOCK_TYPE_VENDER       3
#define HDMI_EDID_BLOCK_TYPE_SPEAKER      4
#define HDMI_EDID_BLOCK_TYPE_VESA         5
#define HDMI_EDID_BLOCK_TYPE_EXTENDED_TAG 7

static struct{
    const char* disp_mode;
    HDMI_Video_Codes_t VIC;
}dispmode_VIC_tab[]=
{
    {"480i", HDMI_480i60}, 
    {"480i", HDMI_480i60_16x9},
    {"480p", HDMI_480p60},
    {"480p", HDMI_480p60_16x9},
    {"576i", HDMI_576i50},
    {"576i", HDMI_576i50_16x9},
    {"576p", HDMI_576p50},
    {"576p", HDMI_576p50_16x9},
    {"720p", HDMI_720p60},
    {"1080i", HDMI_1080i60},
    {"1080p", HDMI_1080p60},
    {"1080P30", HDMI_1080p30},
    {"1080P24", HDMI_1080p24},
    {"720p50hz", HDMI_720p50},
    {"1080i50hz", HDMI_1080i50},
    {"1080p50hz", HDMI_1080p50},
};    

HDMI_Video_Codes_t hdmitx_edid_get_VIC(hdmitx_dev_t* hdmitx_device, const char* disp_mode, char force_flag)
{
    rx_cap_t* pRXCap = &(hdmitx_device->RXCap);
	  int  i,j,count=ARRAY_SIZE(dispmode_VIC_tab);
	  HDMI_Video_Codes_t vic=HDMI_Unkown;
	  HDMI_Video_Codes_t vic16x9=HDMI_Unkown;
	  HDMI_Video_Codes_t vicret=HDMI_Unkown;
    int mode_name_len=0;
    //printk("disp_mode is %s\n", disp_mode);
    for(i=0;i<count;i++)
    { /* below code assumes "16x9 mode" follows it's "normal mode" in dispmode_VIC_tab[] */
        if(strncmp(disp_mode, dispmode_VIC_tab[i].disp_mode, strlen(dispmode_VIC_tab[i].disp_mode))==0)
        {
            if((vic!=HDMI_Unkown)||(strlen(dispmode_VIC_tab[i].disp_mode)==mode_name_len)){
                vic16x9 = dispmode_VIC_tab[i].VIC;
            }
            else if((vic==HDMI_Unkown)||(strlen(dispmode_VIC_tab[i].disp_mode)>mode_name_len)){
                vic = dispmode_VIC_tab[i].VIC;
                vic16x9 = HDMI_Unkown;
                mode_name_len = strlen(dispmode_VIC_tab[i].disp_mode);
            }
        }
    }
    if(vic!=HDMI_Unkown){
        /* normal mode has high priority */
            for( j = 0 ; j < pRXCap->VIC_count ; j++ ){
            if(pRXCap->VIC[j]==vic){ 
                vicret = vic;
                    break;    
            }
        }
        if((j>=pRXCap->VIC_count)&&(vic16x9!=HDMI_Unkown)){
            for( j = 0 ; j < pRXCap->VIC_count ; j++ ){
                if(pRXCap->VIC[j]==vic16x9){
                    vicret = vic16x9;
                    break;
                }
            }
        }
        if(force_flag){ 
            if((vicret==HDMI_Unkown)&&(vic!=HDMI_Unkown)){
                vicret = vic;    
            }
        }
    }    
    return vicret;
}    

char* hdmitx_edid_get_native_VIC(hdmitx_dev_t* hdmitx_device)
{
    rx_cap_t* pRXCap = &(hdmitx_device->RXCap);
	  int  i,count=ARRAY_SIZE(dispmode_VIC_tab);
	  char* disp_mode_ret=NULL;
    for(i=0;i<count;i++){
        if(pRXCap->native_VIC==dispmode_VIC_tab[i].VIC){
            disp_mode_ret = (char*)(dispmode_VIC_tab[i].disp_mode);
            break;    
        }
    }    
    return disp_mode_ret;
}    
