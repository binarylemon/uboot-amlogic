#ifndef __HDMI_H__
#define __HDMI_H__
#include "../aml_tv.h"

// CEA TIMING STRUCT DEFINITION
struct hdmi_cea_timing {
    unsigned int pixel_freq;            // Unit: 1000
    unsigned int h_freq;              // Unit: Hz
    unsigned int v_freq;              // Unit: 0.001 Hz
    unsigned int vsync_polarity : 1;    // 1: positive  0: negative
    unsigned int hsync_polarity : 1;
    unsigned short h_active;
    unsigned short h_total;
    unsigned short h_blank;
    unsigned short h_front;
    unsigned short h_sync;
    unsigned short h_back;
    unsigned short v_active;
    unsigned short v_total;
    unsigned short v_blank;
    unsigned short v_front;
    unsigned short v_sync;
    unsigned short v_back;
    unsigned short v_sync_ln;
};

// get hdmi cea timing
// t: struct hdmi_cea_timing *
#define GET_TIMING(name)      (t->name)

struct hdmi_format_para {
    HDMI_Video_Codes_t vic;
    char * name;
    unsigned int pixel_repetition_factor;
    unsigned int progress_mode : 1;         // 0: Interlace mode  1: Progressive Mode
    unsigned int scrambler_en : 1;
    unsigned int tmds_clk_div40 : 1;
    unsigned int tmds_clk;            // Unit: 1000
    struct hdmi_cea_timing timing;
};

struct hdmi_format_para * hdmi_get_fmt_paras(HDMI_Video_Codes_t vic);

enum hdmi_color_depth {
    HDMI_COLOR_DEPTH_24B = 4,
    HDMI_COLOR_DEPTH_30B = 5,
    HDMI_COLOR_DEPTH_36B = 6,
    HDMI_COLOR_DEPTH_48B = 7,
};

enum hdmi_color_format {
    HDMI_COLOR_FORMAT_RGB,
    HDMI_COLOR_FORMAT_444,
    HDMI_COLOR_FORMAT_422,
    HDMI_COLOR_FORMAT_420,
};

enum hdmi_color_range {
    HDMI_COLOR_RANGE_LIM,
    HDMI_COLOR_RANGE_FUL,
};

enum hdmi_audio_packet {
    HDMI_AUDIO_PACKET_SMP = 0x02,
    HDMI_AUDIO_PACKET_1BT = 0x07,
    HDMI_AUDIO_PACKET_DST = 0x08,
    HDMI_AUDIO_PACKET_HBR = 0x09,
};

#endif
