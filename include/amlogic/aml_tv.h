#ifndef TVOUT_H
#define TVOUT_H

typedef enum {
    VMODE_480I  = 0,
    VMODE_480CVBS,
    VMODE_480P  ,

    VMODE_576I   ,
    VMODE_576CVBS   ,
    VMODE_576P  ,
    VMODE_720P  ,
    VMODE_1080I ,
    VMODE_1080P ,
    VMODE_720P_50HZ ,
    VMODE_1080I_50HZ ,
    VMODE_1080P_50HZ ,
    VMODE_1080P_24HZ ,
    VMODE_4K2K_30HZ ,
    VMODE_4K2K_25HZ ,
    VMODE_4K2K_24HZ ,
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

typedef enum {
    TVMODE_480I  = 0,
    TVMODE_480CVBS,
    TVMODE_480P  ,

    TVMODE_576I  ,
    TVMODE_576CVBS,
    TVMODE_576P  ,
    TVMODE_720P  ,
    TVMODE_1080I ,
    TVMODE_1080P ,
    TVMODE_720P_50HZ ,
    TVMODE_1080I_50HZ ,
    TVMODE_1080P_50HZ ,
    TVMODE_1080P_24HZ ,
    TVMODE_4K2K_30HZ ,
    TVMODE_4K2K_25HZ ,
    TVMODE_4K2K_24HZ ,
    TVMODE_4K2K_SMPTE ,
    TVMODE_4K2K_FAKE_5G ,
    TVMODE_4K2K_60HZ,
    TVMODE_4K2K_60HZ_Y420,
    TVMODE_4K2K_50HZ,
    TVMODE_4K2K_50HZ_Y420,
    TVMODE_VGA ,
    TVMODE_SVGA,
    TVMODE_XGA,
    TVMODE_SXGA,
    TVMODE_WSXGA,
    TVMODE_FHDVGA,
    TVMODE_MAX    
} tvmode_t;

// HDMI VIC definitions
typedef enum HDMI_Video_Type_ {
// Refer to CEA 861-D
    HDMI_Unkown = 0,
    HDMI_640x480p60_4x3 = 1,
    HDMI_720x480p60_4x3 = 2,
    HDMI_720x480p60_16x9 = 3,
    HDMI_1280x720p60_16x9 = 4,
    HDMI_1920x1080i60_16x9 = 5,
    HDMI_720x480i60_4x3 = 6,
    HDMI_720x480i60_16x9 = 7,
    HDMI_720x240p60_4x3 = 8,
    HDMI_720x240p60_16x9 = 9,
    HDMI_2880x480i60_4x3 = 10,
    HDMI_2880x480i60_16x9 = 11,
    HDMI_2880x240p60_4x3 = 12,
    HDMI_2880x240p60_16x9 = 13,
    HDMI_1440x480p60_4x3 = 14,
    HDMI_1440x480p60_16x9 = 15,
    HDMI_1920x1080p60_16x9 = 16,
    HDMI_720x576p50_4x3 = 17,
    HDMI_720x576p50_16x9 = 18,
    HDMI_1280x720p50_16x9 = 19,
    HDMI_1920x1080i50_16x9 = 20,
    HDMI_720x576i50_4x3 = 21,
    HDMI_720x576i50_16x9 = 22,
    HDMI_720x288p_4x3 = 23,
    HDMI_720x288p_16x9 = 24,
    HDMI_2880x576i50_4x3 = 25,
    HDMI_2880x576i50_16x9 = 26,
    HDMI_2880x288p50_4x3 = 27,
    HDMI_2880x288p50_16x9 = 28,
    HDMI_1440x576p_4x3 = 29,
    HDMI_1440x576p_16x9 = 30,
    HDMI_1920x1080p50_16x9 = 31,
    HDMI_1920x1080p24_16x9 = 32,
    HDMI_1920x1080p25_16x9 = 33,
    HDMI_1920x1080p30_16x9 = 34,
    HDMI_2880x480p60_4x3 = 35,
    HDMI_2880x480p60_16x9 = 36,
    HDMI_2880x576p50_4x3 = 37,
    HDMI_2880x576p50_16x9 = 38,
    HDMI_1920x1080i_t1250_50_16x9 = 39,
    HDMI_1920x1080i100_16x9 = 40,
    HDMI_1280x720p100_16x9 = 41,
    HDMI_720x576p100_4x3 = 42,
    HDMI_720x576p100_16x9 = 43,
    HDMI_720x576i100_4x3 = 44,
    HDMI_720x576i100_16x9 = 45,
    HDMI_1920x1080i120_16x9 = 46,
    HDMI_1280x720p120_16x9 = 47,
    HDMI_720x480p120_4x3 = 48,
    HDMI_720x480p120_16x9 = 49,
    HDMI_720X480i120_4x3 = 50,
    HDMI_720X480i120_16x9 = 51,
    HDMI_720x576p200_4x3 = 52,
    HDMI_720x576p200_16x9 = 53,
    HDMI_720x576i200_4x3 = 54,
    HDMI_720x576i200_16x9 = 55,
    HDMI_720x480p240_4x3 = 56,
    HDMI_720x480p240_16x9 = 57,
    HDMI_720x480i240_4x3 = 58,
    HDMI_720x480i240_16x9 = 59,
// Refet to CEA 861-F
    HDMI_1280x720p24_16x9 = 60,
    HDMI_1280x720p25_16x9 = 61,
    HDMI_1280x720p30_16x9 = 62,
    HDMI_1920x1080p120_16x9 = 63,
    HDMI_1920x1080p100_16x9 = 64,
    HDMI_1280x720p24_64x27 = 65,
    HDMI_1280x720p25_64x27 = 66,
    HDMI_1280x720p30_64x27 = 67,
    HDMI_1280x720p50_64x27 = 68,
    HDMI_1280x720p60_64x27 = 69,
    HDMI_1280x720p100_64x27 = 70,
    HDMI_1280x720p120_64x27 = 71,
    HDMI_1920x1080p24_64x27 = 72,
    HDMI_1920x1080p25_64x27 = 73,
    HDMI_1920x1080p30_64x27 = 74,
    HDMI_1920x1080p50_64x27 = 75,
    HDMI_1920x1080p60_64x27 = 76,
    HDMI_1920x1080p100_64x27 = 77,
    HDMI_1920x1080p120_64x27 = 78,
    HDMI_1680x720p24_64x27 = 79,
    HDMI_1680x720p25_64x27 = 80,
    HDMI_1680x720p30_64x27 = 81,
    HDMI_1680x720p50_64x27 = 82,
    HDMI_1680x720p60_64x27 = 83,
    HDMI_1680x720p100_64x27 = 84,
    HDMI_1680x720p120_64x27 = 85,
    HDMI_2560x1080p24_64x27 = 86,
    HDMI_2560x1080p25_64x27 = 87,
    HDMI_2560x1080p30_64x27 = 88,
    HDMI_2560x1080p50_64x27 = 89,
    HDMI_2560x1080p60_64x27 = 90,
    HDMI_2560x1080p100_64x27 = 91,
    HDMI_2560x1080p120_64x27 = 92,
    HDMI_3840x2160p24_16x9 = 93,
    HDMI_3840x2160p25_16x9 = 94,
    HDMI_3840x2160p30_16x9 = 95,
    HDMI_3840x2160p50_16x9 = 96,
    HDMI_3840x2160p60_16x9 = 97,
    HDMI_4096x2160p24_256x135 = 98,
    HDMI_4096x2160p25_256x135 = 99,
    HDMI_4096x2160p30_256x135 = 100,
    HDMI_4096x2160p50_256x135 = 101,
    HDMI_4096x2160p60_256x135 = 102,
    HDMI_3840x2160p24_64x27 = 103,
    HDMI_3840x2160p25_64x27 = 104,
    HDMI_3840x2160p30_64x27 = 105,
    HDMI_3840x2160p50_64x27 = 106,
    HDMI_3840x2160p60_64x27 = 107,
    HDMI_RESERVED = 108,
} HDMI_Video_Codes_t;

// Compliance with old definitions
#define HDMI_640x480p60         HDMI_640x480p60_4x3
#define HDMI_480p60             HDMI_720x480p60_4x3
#define HDMI_480p60_16x9        HDMI_720x480p60_16x9
#define HDMI_720p60             HDMI_1280x720p60_16x9
#define HDMI_1080i60            HDMI_1920x1080i60_16x9
#define HDMI_480i60             HDMI_720x480i60_4x3
#define HDMI_480i60_16x9        HDMI_720x480i60_16x9
#define HDMI_480i60_16x9_rpt    HDMI_2880x480i60_16x9
#define HDMI_1440x480p60        HDMI_1440x480p60_4x3
#define HDMI_1440x480p60_16x9   HDMI_1440x480p60_16x9
#define HDMI_1080p60            HDMI_1920x1080p60_16x9
#define HDMI_576p50             HDMI_720x576p50_4x3
#define HDMI_576p50_16x9        HDMI_720x576p50_16x9
#define HDMI_720p50             HDMI_1280x720p50_16x9
#define HDMI_1080i50            HDMI_1920x1080i50_16x9
#define HDMI_576i50             HDMI_720x576i50_4x3
#define HDMI_576i50_16x9        HDMI_720x576i50_16x9
#define HDMI_576i50_16x9_rpt    HDMI_2880x576i50_16x9
#define HDMI_1080p50            HDMI_1920x1080p50_16x9
#define HDMI_1080p24            HDMI_1920x1080p24_16x9
#define HDMI_1080p25            HDMI_1920x1080p25_16x9
#define HDMI_1080p30            HDMI_1920x1080p30_16x9
#define HDMI_480p60_16x9_rpt    HDMI_2880x480p60_16x9
#define HDMI_576p50_16x9_rpt    HDMI_2880x576p50_16x9
#define HDMI_4k2k_24            HDMI_3840x2160p24_16x9
#define HDMI_4k2k_25            HDMI_3840x2160p25_16x9
#define HDMI_4k2k_30            HDMI_3840x2160p30_16x9
#define HDMI_4k2k_50            HDMI_3840x2160p50_16x9
#define HDMI_4k2k_60            HDMI_3840x2160p60_16x9
#define HDMI_4k2k_smpte_24      HDMI_4096x2160p24_256x135
#define HDMI_4k2k_smpte         HDMI_4k2k_smpte_24
#define HDMI_4k2k_smpte_50      HDMI_4096x2160p50_256x135
#define HDMI_4k2k_smpte_60      HDMI_4096x2160p60_256x135

#define TVMODE_VALID(m) (m < TVMODE_MAX)

extern HDMI_Video_Codes_t tvmode_to_vic(int mode);
extern tvmode_t vic_to_tvmode(HDMI_Video_Codes_t vic);
extern vmode_t vic_to_vmode(HDMI_Video_Codes_t vic);

int tv_out_open(int mode);
int tv_out_close(void);
int tv_out_cur_mode(void);
int tv_out_get_info(int mode, unsigned *width, unsigned *height);

typedef struct tv_operations {
	void  (*enable)(void);
	void  (*disable)(void);
	void  (*power_on)(void);
	void  (*power_off)(void);
} tv_operations_t;

extern tv_operations_t tv_oper;

typedef struct reg_s {
    unsigned int reg;
    unsigned int val;
} reg_t;

struct tvregs_set_t {
    tvmode_t tvmode;
    const reg_t *reg_setting;
};

struct tv_hdmi_set_t {
    HDMI_Video_Codes_t vic;
    const reg_t *reg_setting;
};

typedef struct tvinfo_s {
    tvmode_t tvmode;
    unsigned int xres;
    unsigned int yres;
    const char *id;
} tvinfo_t;

#ifndef MREG_END_MARKER
#define MREG_END_MARKER 0xffff
#endif

#endif
