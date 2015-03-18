#ifndef __HDMI_TX_TVENC_H__
#define __HDMI_TX_TVENC_H__

#include <common.h>
#include <amlogic/aml_tv.h>
#include <amlogic/hdmitx/hdmi.h>

struct enc_reg_set {
    unsigned int addr;
    unsigned int val;
};

struct enc_reg_map {
    tvmode_t tvmode;
    struct enc_reg_set *set;
};

void config_hdmi_tvenc(HDMI_Video_Codes_t vic);
void config_tv_enc(tvmode_t tvmode);

#ifndef printk
#define printk printf
#endif

#endif
