#include "hdmi_tx_tvenc.h"
//#include <asm/arch/register.h>
#include <asm/arch/reg_addr.h>

// Below is ENC* registers configure

static struct enc_reg_set enc_480i60hz_set[] = {
    {P_VENC_VDAC_SETTING,            0xff,  },

    {P_ENCI_CFILT_CTRL,              0x12,},
    {P_ENCI_CFILT_CTRL2,              0x12,},
    {P_VENC_DVI_SETTING,             0,     },
    {P_ENCI_VIDEO_MODE,              0,     },
    {P_ENCI_VIDEO_MODE_ADV,          0,     },
    {P_ENCI_SYNC_HSO_BEGIN,          5,     },
    {P_ENCI_SYNC_HSO_END,            129,   },
    {P_ENCI_SYNC_VSO_EVNLN,          0x0003 },
    {P_ENCI_SYNC_VSO_ODDLN,          0x0104 },
    {P_ENCI_MACV_MAX_AMP,            0x810b },
    {P_VENC_VIDEO_PROG_MODE,         0xf0   },
    {P_ENCI_VIDEO_MODE,              0x08   },
    {P_ENCI_VIDEO_MODE_ADV,          0x26,  },
    {P_ENCI_VIDEO_SCH,               0x20,  },
    {P_ENCI_SYNC_MODE,               0x07,  },
    {P_ENCI_YC_DELAY,                0x333, },
    {P_ENCI_VFIFO2VD_PIXEL_START,    0xf3,  },
    {P_ENCI_VFIFO2VD_PIXEL_END,      0x0693,},
    {P_ENCI_VFIFO2VD_LINE_TOP_START, 0x12,  },
    {P_ENCI_VFIFO2VD_LINE_TOP_END,   0x102, },
    {P_ENCI_VFIFO2VD_LINE_BOT_START, 0x13,  },
    {P_ENCI_VFIFO2VD_LINE_BOT_END,   0x103, },
    {P_VENC_SYNC_ROUTE,              0,     },
    {P_ENCI_DBG_PX_RST,              0,     },
    {P_VENC_INTCTRL,                 0x2,   },
    {P_ENCI_VFIFO2VD_CTL,            0x4e01,},
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VENC_UPSAMPLE_CTRL0,          0x0061,},
    {P_VENC_UPSAMPLE_CTRL1,          0x4061,},
    {P_VENC_UPSAMPLE_CTRL2,          0x5061,},
    {P_VENC_VDAC_DACSEL0,            0x0000,},
    {P_VENC_VDAC_DACSEL1,            0x0000,},
    {P_VENC_VDAC_DACSEL2,            0x0000,},
    {P_VENC_VDAC_DACSEL3,            0x0000,},
    {P_VENC_VDAC_DACSEL4,            0x0000,},
    {P_VENC_VDAC_DACSEL5,            0x0000,},
    {P_VPU_VIU_VENC_MUX_CTRL,        0x0005,},
    {P_VENC_VDAC_FIFO_CTRL,          0x2000,},
    {P_ENCI_DACSEL_0,                0x0011 },
    {P_ENCI_DACSEL_1,                0x87   },
    {P_ENCP_VIDEO_EN,                0,     },
    {P_ENCI_VIDEO_EN,                1,     },
    {P_ENCI_VIDEO_SAT,               0x7        },
    {P_VENC_VDAC_DAC0_FILT_CTRL0,    0x1        },
    {P_VENC_VDAC_DAC0_FILT_CTRL1,    0xfc48     },
    {MREG_END_MARKER,              0      }
};

static struct enc_reg_set enc_480p60hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },

    //{P_HHI_VID_CLK_DIV,            0x01000100,},
    {P_ENCP_VIDEO_FILT_CTRL,       0x2052,},
    {P_VENC_DVI_SETTING,           0x21,  },
    {P_ENCP_VIDEO_MODE,            0x4000,},
    {P_ENCP_VIDEO_MODE_ADV,        9,     },
    {P_ENCP_VIDEO_YFP1_HTIME,      244,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      1630,  },
    {P_ENCP_VIDEO_YC_DLY,          0,     },
    {P_ENCP_VIDEO_MAX_PXCNT,       1715,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       524,   },
    {P_ENCP_VIDEO_HSPULS_BEGIN,    0x22,  },
    {P_ENCP_VIDEO_HSPULS_END,      0xa0,  },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   88,    },
    {P_ENCP_VIDEO_VSPULS_BEGIN,    0,     },
    {P_ENCP_VIDEO_VSPULS_END,      1589   },
    {P_ENCP_VIDEO_VSPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    5,     },
    {P_ENCP_VIDEO_HAVON_BEGIN,     249,   },
    {P_ENCP_VIDEO_HAVON_END,       1689,  },
    {P_ENCP_VIDEO_VAVON_BLINE,     42,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     521,   },
    {P_ENCP_VIDEO_SYNC_MODE,       0x07,  },
    {P_VENC_VIDEO_PROG_MODE,       0x0,   },
    {P_VENC_VIDEO_EXSRC,           0x0,   },
    {P_ENCP_VIDEO_HSO_BEGIN,       0x3,   },
    {P_ENCP_VIDEO_HSO_END,         0x5,   },
    {P_ENCP_VIDEO_VSO_BEGIN,       0x3,   },
    {P_ENCP_VIDEO_VSO_END,         0x5,   },
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },  //added by JZD. Switch Panel to 480p first time, movie video flicks if not set this to 0
    {P_ENCP_VIDEO_SY_VAL,          8,     },
    {P_ENCP_VIDEO_SY2_VAL,         0x1d8, },
    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VENC_UPSAMPLE_CTRL0,        0x9061,},
    {P_VENC_UPSAMPLE_CTRL1,        0xa061,},
    {P_VENC_UPSAMPLE_CTRL2,        0xb061,},
    {P_VENC_VDAC_DACSEL0,          0xf003,},
    {P_VENC_VDAC_DACSEL1,          0xf003,},
    {P_VENC_VDAC_DACSEL2,          0xf003,},
    {P_VENC_VDAC_DACSEL3,          0xf003,},
    {P_VENC_VDAC_DACSEL4,          0xf003,},
    {P_VENC_VDAC_DACSEL5,          0xf003,},
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,},
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_ENCI_VIDEO_EN,              0      },
    {P_ENCP_VIDEO_EN,              1      },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_576i50hz_set[] = {
    {P_VENC_VDAC_SETTING,               0xff,      },

    {P_VENC_DVI_SETTING,                0,         },
    {P_ENCI_VIDEO_MODE,                 0,         },
    {P_ENCI_VIDEO_MODE_ADV,             0,         },
    {P_ENCI_SYNC_HSO_BEGIN,             3,         },
    {P_ENCI_SYNC_HSO_END,               129,       },
    {P_ENCI_SYNC_VSO_EVNLN,             0x0003     },
    {P_ENCI_SYNC_VSO_ODDLN,             0x0104     },
    {P_ENCI_MACV_MAX_AMP,               0x8107     },
    {P_VENC_VIDEO_PROG_MODE,            0xff       },
    {P_ENCI_VIDEO_MODE,                 0x13       },
    {P_ENCI_VIDEO_MODE_ADV,             0x26,      },
    {P_ENCI_VIDEO_SCH,                  0x28,      },
    {P_ENCI_SYNC_MODE,                  0x07,      },
    {P_ENCI_YC_DELAY,                   0x333,     },
    {P_ENCI_VFIFO2VD_PIXEL_START,       0x010b     },
    {P_ENCI_VFIFO2VD_PIXEL_END,         0x06ab     },
    {P_ENCI_VFIFO2VD_LINE_TOP_START,    0x0016     },
    {P_ENCI_VFIFO2VD_LINE_TOP_END,      0x0136     },
    {P_ENCI_VFIFO2VD_LINE_BOT_START,    0x0017     },
    {P_ENCI_VFIFO2VD_LINE_BOT_END,      0x0137     },
    {P_VENC_SYNC_ROUTE,                 0,         },
    {P_ENCI_DBG_PX_RST,                 0,         },
    {P_VENC_INTCTRL,                    0x2,       },
    {P_ENCI_VFIFO2VD_CTL,               0x4e01,    },
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VENC_UPSAMPLE_CTRL0,             0x0061,    },
    {P_VENC_UPSAMPLE_CTRL1,             0x4061,    },
    {P_VENC_UPSAMPLE_CTRL2,             0x5061,    },
    {P_VENC_VDAC_DACSEL0,               0x0000,    },
    {P_VENC_VDAC_DACSEL1,               0x0000,    },
    {P_VENC_VDAC_DACSEL2,               0x0000,    },
    {P_VENC_VDAC_DACSEL3,               0x0000,    },
    {P_VENC_VDAC_DACSEL4,               0x0000,    },
    {P_VENC_VDAC_DACSEL5,               0x0000,    },
    {P_VPU_VIU_VENC_MUX_CTRL,           0x0005,    },
    {P_VENC_VDAC_FIFO_CTRL,             0x2000,    },
    {P_ENCI_DACSEL_0,                   0x0011     },
    {P_ENCI_DACSEL_1,                   0x87       },
    {P_ENCP_VIDEO_EN,                   0,         },
    {P_ENCI_VIDEO_EN,                   1,         },
    {P_ENCI_VIDEO_SAT,                  0x7        },
    {P_VENC_VDAC_DAC0_FILT_CTRL0,       0x1        },
    {P_VENC_VDAC_DAC0_FILT_CTRL1,       0xfc48     },
    {MREG_END_MARKER,                 0          }
};

static struct enc_reg_set enc_576p50hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,      },
    {P_ENCP_VIDEO_FILT_CTRL,       0x52,      },
    {P_VENC_DVI_SETTING,           0x21,      },
    {P_ENCP_VIDEO_MODE,            0x4000,    },
    {P_ENCP_VIDEO_MODE_ADV,        9,         },
    {P_ENCP_VIDEO_YFP1_HTIME,      235,       },
    {P_ENCP_VIDEO_YFP2_HTIME,      1674,      },
    {P_ENCP_VIDEO_YC_DLY,          0xf,       },
    {P_ENCP_VIDEO_MAX_PXCNT,       1727,      },
    {P_ENCP_VIDEO_MAX_LNCNT,       624,       },
    {P_ENCP_VIDEO_HSPULS_BEGIN,    0,         },
    {P_ENCP_VIDEO_HSPULS_END,      0x80,      },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   88,        },
    {P_ENCP_VIDEO_VSPULS_BEGIN,    0,         },
    {P_ENCP_VIDEO_VSPULS_END,      1599       },
    {P_ENCP_VIDEO_VSPULS_BLINE,    0,         },
    {P_ENCP_VIDEO_VSPULS_ELINE,    4,         },
    {P_ENCP_VIDEO_HAVON_BEGIN,     235,       },
    {P_ENCP_VIDEO_HAVON_END,       1674,      },
    {P_ENCP_VIDEO_VAVON_BLINE,     44,        },
    {P_ENCP_VIDEO_VAVON_ELINE,     619,       },
    {P_ENCP_VIDEO_SYNC_MODE,       0x07,      },
    {P_VENC_VIDEO_PROG_MODE,       0x0,       },
    {P_VENC_VIDEO_EXSRC,           0x0,       },
    {P_ENCP_VIDEO_HSO_BEGIN,       0x80,      },
    {P_ENCP_VIDEO_HSO_END,         0x0,       },
    {P_ENCP_VIDEO_VSO_BEGIN,       0x0,       },
    {P_ENCP_VIDEO_VSO_END,         0x5,       },
    {P_ENCP_VIDEO_VSO_BLINE,       0,         },
    {P_ENCP_VIDEO_SY_VAL,          8,         },
    {P_ENCP_VIDEO_SY2_VAL,         0x1d8,     },
    {P_VENC_SYNC_ROUTE,            0,         },
    {P_VENC_INTCTRL,               0x200,     },
    {P_ENCP_VFIFO2VD_CTL,               0,         },
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VENC_UPSAMPLE_CTRL0,        0x9061,    },
    {P_VENC_UPSAMPLE_CTRL1,        0xa061,    },
    {P_VENC_UPSAMPLE_CTRL2,        0xb061,    },
    {P_VENC_VDAC_DACSEL0,          0xf003,    },
    {P_VENC_VDAC_DACSEL1,          0xf003,    },
    {P_VENC_VDAC_DACSEL2,          0xf003,    },
    {P_VENC_VDAC_DACSEL3,          0xf003,    },
    {P_VENC_VDAC_DACSEL4,          0xf003,    },
    {P_VENC_VDAC_DACSEL5,          0xf003,    },
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,    },
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,    },
    {P_ENCP_DACSEL_0,              0x3102,    },
    {P_ENCP_DACSEL_1,              0x0054,    },
    {P_ENCI_VIDEO_EN,              0          },
    {P_ENCP_VIDEO_EN,              1          },
    {MREG_END_MARKER,            0          }
};

static struct enc_reg_set enc_720p50hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },
    {P_VENC_DVI_SETTING,           0x202d,},
    {P_ENCP_VIDEO_MAX_PXCNT,       3959,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       749,   },

     //analog vidoe position in horizontal
    {P_ENCP_VIDEO_HSPULS_BEGIN,    80,    },
    {P_ENCP_VIDEO_HSPULS_END,      240,   },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   80,    },

    //DE position in horizontal
    {P_ENCP_VIDEO_HAVON_BEGIN,     648,   },
    {P_ENCP_VIDEO_HAVON_END,       3207,  },

    //ditital hsync positon in horizontal
    {P_ENCP_VIDEO_HSO_BEGIN,       128 ,},
    {P_ENCP_VIDEO_HSO_END,         208 , },

    /* vsync horizontal timing */
    {P_ENCP_VIDEO_VSPULS_BEGIN,    688,   },
    {P_ENCP_VIDEO_VSPULS_END,      3248,  },

    /* vertical timing settings */
    {P_ENCP_VIDEO_VSPULS_BLINE,    4,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    8,     },
    {P_ENCP_VIDEO_EQPULS_BLINE,    4,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    8,     },

    //DE position in vertical
    {P_ENCP_VIDEO_VAVON_BLINE,     29,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     748,   },

    //adjust the vsync start point and end point
    {P_ENCP_VIDEO_VSO_BEGIN,       128,},  //168,   },
    {P_ENCP_VIDEO_VSO_END,         128, },  //256,   },

    //adjust the vsync start line and end line
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },

    /* filter & misc settings */
    {P_ENCP_VIDEO_YFP1_HTIME,      648,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      3207,  },


    {P_VENC_VIDEO_PROG_MODE,       0x100, },
    {P_ENCP_VIDEO_MODE,            0x4040,},  //Enable Hsync and equalization pulse switch in center
    {P_ENCP_VIDEO_MODE_ADV,        0x0019,},//bit6:swap PbPr; bit4:YPBPR gain as HDTV type;
                                                 //bit3:Data input from VFIFO;bit[2}0]:repreat pixel a time

     {P_ENCP_VIDEO_SYNC_MODE,       0x407,  },//Video input Synchronization mode ( bit[7:0] -- 4:Slave mode, 7:Master mode)
                                                 //bit[15:6] -- adjust the vsync vertical position
    {P_ENCP_VIDEO_YC_DLY,          0,     },      //Y/Cb/Cr delay
    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,},
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_ENCP_VIDEO_EN,              1,     },
    {P_ENCI_VIDEO_EN,              0,     },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_720p60hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },

    {P_ENCP_VIDEO_FILT_CTRL,       0x0052,},
    {P_VENC_DVI_SETTING,           0x2029,},
    {P_ENCP_VIDEO_MODE,            0x4040,},
    {P_ENCP_VIDEO_MODE_ADV,        0x0019,},
    {P_ENCP_VIDEO_YFP1_HTIME,      648,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      3207,  },
    {P_ENCP_VIDEO_MAX_PXCNT,       3299,  },
    {P_ENCP_VIDEO_HSPULS_BEGIN,    80,    },
    {P_ENCP_VIDEO_HSPULS_END,      240,   },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   80,    },
    {P_ENCP_VIDEO_VSPULS_BEGIN,    688,   },
    {P_ENCP_VIDEO_VSPULS_END,      3248,  },
    {P_ENCP_VIDEO_VSPULS_BLINE,    4,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    8,     },
    {P_ENCP_VIDEO_EQPULS_BLINE,    4,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    8,     },
    {P_ENCP_VIDEO_HAVON_BEGIN,     648,   },
    {P_ENCP_VIDEO_HAVON_END,       3207,  },
    {P_ENCP_VIDEO_VAVON_BLINE,     29,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     748,   },
    {P_ENCP_VIDEO_HSO_BEGIN,       256    },
    {P_ENCP_VIDEO_HSO_END,         168,   },
    {P_ENCP_VIDEO_VSO_BEGIN,       168,   },
    {P_ENCP_VIDEO_VSO_END,         256,   },
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },
    {P_ENCP_VIDEO_MAX_LNCNT,       749,   },
    {P_VENC_VIDEO_PROG_MODE,       0x100, },
    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VENC_UPSAMPLE_CTRL0,        0x9061,},
    {P_VENC_UPSAMPLE_CTRL1,        0xa061,},
    {P_VENC_UPSAMPLE_CTRL2,        0xb061,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,},
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_ENCP_VIDEO_EN,              1,     },
    {P_ENCI_VIDEO_EN,              0,     },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_1080i50hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },
    {P_VENC_DVI_SETTING,           0x202d,},
    {P_ENCP_VIDEO_MAX_PXCNT,       5279,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       1124,  },

    //analog vidoe position in horizontal
    {P_ENCP_VIDEO_HSPULS_BEGIN,    88,    },
    {P_ENCP_VIDEO_HSPULS_END,      264,   },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   88,    },

    //DE position in horizontal
    {P_ENCP_VIDEO_HAVON_BEGIN,     526,   },
    {P_ENCP_VIDEO_HAVON_END,       4365,  },

    //ditital hsync positon in horizontal
    {P_ENCP_VIDEO_HSO_BEGIN,       142,   },
    {P_ENCP_VIDEO_HSO_END,         230,   },

    /* vsync horizontal timing */
    {P_ENCP_VIDEO_EQPULS_BEGIN,    2728,  },
    {P_ENCP_VIDEO_EQPULS_END,      2904,  },
    {P_ENCP_VIDEO_VSPULS_BEGIN,    440,   },
    {P_ENCP_VIDEO_VSPULS_END,      2200,  },

    {P_ENCP_VIDEO_VSPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    4,     },
    {P_ENCP_VIDEO_EQPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    4,     },

    //DE position in vertical
    {P_ENCP_VIDEO_VAVON_BLINE,     20,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     559,   },

    //adjust vsync start point and end point
    {P_ENCP_VIDEO_VSO_BEGIN,       142,    },
    {P_ENCP_VIDEO_VSO_END,         142,    },

    //adjust the vsync start line and end line
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },

    /* filter & misc settings */
    {P_ENCP_VIDEO_YFP1_HTIME,      526,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      4365,  },

    {P_VENC_VIDEO_PROG_MODE,       0x100, },  // Select clk108 as DAC clock, progressive mode
    {P_ENCP_VIDEO_OFLD_VOAV_OFST,  0x11   },//bit[15:12]: Odd field VSO  offset begin,
                                                        //bit[11:8]: Odd field VSO  offset end,
                                                        //bit[7:4]: Odd field VAVON offset begin,
                                                        //bit[3:0]: Odd field VAVON offset end,
    {P_ENCP_VIDEO_MODE,            0x5ffc,},//Enable Hsync and equalization pulse switch in center
    {P_ENCP_VIDEO_MODE_ADV,        0x0019,}, //bit6:swap PbPr; bit4:YPBPR gain as HDTV type;
                                                 //bit3:Data input from VFIFO;bit[2}0]:repreat pixel a time
    {P_ENCP_VIDEO_SYNC_MODE,       0x7, }, //bit[15:8] -- adjust the vsync vertical position
    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_VENC_VDAC_SETTING,          0,     },
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_ENCI_VIDEO_EN,              0,     },
    {P_ENCP_VIDEO_EN,              1,     },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_1080i60hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },
    {P_VENC_DVI_SETTING,           0x2029,},
    {P_ENCP_VIDEO_MAX_PXCNT,       4399,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       1124,  },
    {P_ENCP_VIDEO_HSPULS_BEGIN,    88,    },
    {P_ENCP_VIDEO_HSPULS_END,      264,   },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   88,    },
    {P_ENCP_VIDEO_HAVON_BEGIN,     516,   },
    {P_ENCP_VIDEO_HAVON_END,       4355,  },
    {P_ENCP_VIDEO_HSO_BEGIN,       264,   },
    {P_ENCP_VIDEO_HSO_END,         176,   },
    {P_ENCP_VIDEO_EQPULS_BEGIN,    2288,  },
    {P_ENCP_VIDEO_EQPULS_END,      2464,  },
    {P_ENCP_VIDEO_VSPULS_BEGIN,    440,   },
    {P_ENCP_VIDEO_VSPULS_END,      2200,  },
    {P_ENCP_VIDEO_VSPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    4,     },
    {P_ENCP_VIDEO_EQPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    4,     },
    {P_ENCP_VIDEO_VAVON_BLINE,     20,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     559,   },
    {P_ENCP_VIDEO_VSO_BEGIN,       88,    },
    {P_ENCP_VIDEO_VSO_END,         88,    },
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },
    {P_ENCP_VIDEO_YFP1_HTIME,      516,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      4355,  },
    {P_VENC_VIDEO_PROG_MODE,       0x100, },
    {P_ENCP_VIDEO_OFLD_VOAV_OFST,  0x11   },
    {P_ENCP_VIDEO_MODE,            0x5ffc,},
    {P_ENCP_VIDEO_MODE_ADV,        0x0019,},
    {P_ENCP_VIDEO_SYNC_MODE,       0x207, },
    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_VENC_VDAC_SETTING,          0,     },
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_ENCI_VIDEO_EN,              0,     },
    {P_ENCP_VIDEO_EN,              1,     },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_1080p24hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },
    {P_VENC_DVI_SETTING,           0x000d,},
    {P_ENCP_VIDEO_MAX_PXCNT,       2749,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       1124,  },
    /* horizontal timing settings */
    {P_ENCP_VIDEO_HSPULS_BEGIN,    44,  },//1980
    {P_ENCP_VIDEO_HSPULS_END,      132,    },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   44,    },

    //DE position in horizontal
    {P_ENCP_VIDEO_HAVON_BEGIN,     271,   },
    {P_ENCP_VIDEO_HAVON_END,       2190,  },

    //ditital hsync positon in horizontal
    {P_ENCP_VIDEO_HSO_BEGIN,       79 ,    },
    {P_ENCP_VIDEO_HSO_END,         123,  },

    /* vsync horizontal timing */
    {P_ENCP_VIDEO_VSPULS_BEGIN,    220,   },
    {P_ENCP_VIDEO_VSPULS_END,      2140,  },

    /* vertical timing settings */
    {P_ENCP_VIDEO_VSPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    4,     },//35
    {P_ENCP_VIDEO_EQPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    4,     },//35
    {P_ENCP_VIDEO_VAVON_BLINE,     41,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     1120,  },

    //adjust the hsync & vsync start point and end point
    {P_ENCP_VIDEO_VSO_BEGIN,       79,  },
    {P_ENCP_VIDEO_VSO_END,         79,  },

    //adjust the vsync start line and end line
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },

    {P_ENCP_VIDEO_YFP1_HTIME,      271,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      2190,  },
    {P_VENC_VIDEO_PROG_MODE,       0x100, },
    {P_ENCP_VIDEO_MODE,            0x4040,},
    {P_ENCP_VIDEO_MODE_ADV,        0x0018,},

    {P_ENCP_VIDEO_SYNC_MODE,       0x7, }, //bit[15:8] -- adjust the vsync vertical position

    {P_ENCP_VIDEO_YC_DLY,          0,     },      //Y/Cb/Cr delay

    {P_ENCP_VIDEO_RGB_CTRL, 2,},       // enable sync on B

    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,},
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_ENCI_VIDEO_EN,              0,     },
    {P_ENCP_VIDEO_EN,              1,     },
    {MREG_END_MARKER,            0      }
};

static struct enc_reg_set enc_1080p50hz_set[] = {
    {P_VENC_VDAC_SETTING,          0xff,  },

    {P_ENCP_VIDEO_FILT_CTRL,       0x1052,},

    // bit 13    1          (delayed prog_vs)
    // bit 5:4:  2          (pixel[0])
    // bit 3:    1          invert vsync or not
    // bit 2:    1          invert hsync or not
    // bit1:     1          (select viu sync)
    // bit0:     1          (progressive)
    {P_VENC_DVI_SETTING,           0x000d,},
    {P_ENCP_VIDEO_MAX_PXCNT,       2639,  },
    {P_ENCP_VIDEO_MAX_LNCNT,       1124,  },
    /* horizontal timing settings */
    {P_ENCP_VIDEO_HSPULS_BEGIN,    44,  },//1980
    {P_ENCP_VIDEO_HSPULS_END,      132,    },
    {P_ENCP_VIDEO_HSPULS_SWITCH,   44,    },

    //DE position in horizontal
    {P_ENCP_VIDEO_HAVON_BEGIN,     271,   },
    {P_ENCP_VIDEO_HAVON_END,       2190,  },

    //ditital hsync positon in horizontal
    {P_ENCP_VIDEO_HSO_BEGIN,       79 ,    },
    {P_ENCP_VIDEO_HSO_END,         123,  },

    /* vsync horizontal timing */
    {P_ENCP_VIDEO_VSPULS_BEGIN,    220,   },
    {P_ENCP_VIDEO_VSPULS_END,      2140,  },

    /* vertical timing settings */
    {P_ENCP_VIDEO_VSPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_VSPULS_ELINE,    4,     },//35
    {P_ENCP_VIDEO_EQPULS_BLINE,    0,     },
    {P_ENCP_VIDEO_EQPULS_ELINE,    4,     },//35
    {P_ENCP_VIDEO_VAVON_BLINE,     41,    },
    {P_ENCP_VIDEO_VAVON_ELINE,     1120,  },

    //adjust the hsync & vsync start point and end point
    {P_ENCP_VIDEO_VSO_BEGIN,       79,  },
    {P_ENCP_VIDEO_VSO_END,         79,  },

    //adjust the vsync start line and end line
    {P_ENCP_VIDEO_VSO_BLINE,       0,     },
    {P_ENCP_VIDEO_VSO_ELINE,       5,     },

    {P_ENCP_VIDEO_YFP1_HTIME,      271,   },
    {P_ENCP_VIDEO_YFP2_HTIME,      2190,  },
    {P_VENC_VIDEO_PROG_MODE,       0x100, },
    {P_ENCP_VIDEO_MODE,            0x4040,},
    {P_ENCP_VIDEO_MODE_ADV,        0x0018,},

    {P_ENCP_VIDEO_SYNC_MODE,       0x7, }, //bit[15:8] -- adjust the vsync vertical position

    {P_ENCP_VIDEO_YC_DLY,          0,     },      //Y/Cb/Cr delay

    {P_ENCP_VIDEO_RGB_CTRL, 2,},       // enable sync on B

    {P_VENC_SYNC_ROUTE,            0,     },
    {P_VENC_INTCTRL,               0x200, },
    {P_ENCP_VFIFO2VD_CTL,               0,     },
    {P_VENC_VDAC_FIFO_CTRL,        0x1000,},
    {P_VENC_VDAC_SETTING,          0,     },
    {P_VPU_VIU_VENC_MUX_CTRL,      0x000a,},
    {P_ENCP_DACSEL_0,              0x3102,},
    {P_ENCP_DACSEL_1,              0x0054,},
    {P_VENC_VDAC_DACSEL0,          0x0001,},
    {P_VENC_VDAC_DACSEL1,          0x0001,},
    {P_VENC_VDAC_DACSEL2,          0x0001,},
    {P_VENC_VDAC_DACSEL3,          0x0001,},
    {P_VENC_VDAC_DACSEL4,          0x0001,},
    {P_VENC_VDAC_DACSEL5,          0x0001,},
    {P_ENCI_VIDEO_EN,              0,     },
    {P_ENCP_VIDEO_EN,              1,     },
    {MREG_END_MARKER,       MREG_END_MARKER},
};

static struct enc_reg_set enc_1080p60hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x0040 | (1<<14)}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140 + 1920},//2060},
    {P_ENCP_VIDEO_MAX_PXCNT,        2200 - 1},//2199},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       140 + 1920 - 1},//2059},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},
    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        148 + 1920 - 1},//2067},
    {P_ENCP_VIDEO_VAVON_BLINE,      45},
    {P_ENCP_VIDEO_VAVON_ELINE,      45 + 1080 - 1},//1124},
    {P_ENCP_VIDEO_HSO_BEGIN,        44},
    {P_ENCP_VIDEO_HSO_END,          2156},
    {P_ENCP_VIDEO_VSO_BEGIN,        2100},
    {P_ENCP_VIDEO_VSO_END,          2164},
    {P_ENCP_VIDEO_VSO_BLINE,        3},
    {P_ENCP_VIDEO_VSO_ELINE,        5},
    {P_ENCP_VIDEO_MAX_LNCNT,        1125 - 1},//1124},
    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {P_VENC_VDAC_DACSEL0,           0x9},
    {P_VENC_VDAC_DACSEL1,           0xa},
    {P_VENC_VDAC_DACSEL2,           0xb},
    {P_VENC_VDAC_DACSEL3,           0xc},
    {P_VENC_VDAC_DACSEL4,           0xd},
    {P_VENC_VDAC_DACSEL5,           0xe},
    {MREG_END_MARKER,       MREG_END_MARKER},
};

static struct enc_reg_set enc_4k2k24hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},

    {P_ENCP_VIDEO_MAX_PXCNT,        3840+1660-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter

    {MREG_END_MARKER,            0      },
};

static struct enc_reg_set enc_4k2ksmpte24hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840+256},

    {P_ENCP_VIDEO_MAX_PXCNT,        4096+1404-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987+256},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920+256},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920+256},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920+256},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {MREG_END_MARKER,            0      },
};

static struct enc_reg_set enc_4k2k25hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},

    {P_ENCP_VIDEO_MAX_PXCNT,        3840+1440-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {MREG_END_MARKER,            0      },
};

static struct enc_reg_set enc_4k2k30hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},
    {P_ENCP_VIDEO_MAX_PXCNT,        3840+560-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {MREG_END_MARKER,       MREG_END_MARKER},
};

static struct enc_reg_set enc_4k2k50hz_set[] = {
    {P_ENCP_VIDEO_EN,              0,     },
    {P_ENCI_VIDEO_EN,              0,     },
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},
    {P_ENCP_VIDEO_MAX_PXCNT,        3840+1440-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},
    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},
    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},
    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},
    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {P_ENCP_VIDEO_EN,              1,     },
    {P_ENCI_VIDEO_EN,              0,     },
    {MREG_END_MARKER,       MREG_END_MARKER},
};

static struct enc_reg_set enc_4k2k60hz_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},
    {P_ENCP_VIDEO_MAX_PXCNT,        3840+560-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {MREG_END_MARKER,       MREG_END_MARKER},
};

static struct enc_reg_set enc_4k2k5g_set[] = {
    {P_ENCP_VIDEO_MODE,             0x4040}, // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
    {P_ENCP_VIDEO_MODE_ADV,         0x0008}, // Sampling rate: 1
    {P_ENCP_VIDEO_YFP1_HTIME,       140},
    {P_ENCP_VIDEO_YFP2_HTIME,       140+3840},
    {P_ENCP_VIDEO_MAX_PXCNT,        3840+560-1},
    {P_ENCP_VIDEO_HSPULS_BEGIN,     2156+1920},
    {P_ENCP_VIDEO_HSPULS_END,       44},
    {P_ENCP_VIDEO_HSPULS_SWITCH,    44},
    {P_ENCP_VIDEO_VSPULS_BEGIN,     140},
    {P_ENCP_VIDEO_VSPULS_END,       2059+1920},
    {P_ENCP_VIDEO_VSPULS_BLINE,     0},
    {P_ENCP_VIDEO_VSPULS_ELINE,     4},

    {P_ENCP_VIDEO_HAVON_BEGIN,      148},
    {P_ENCP_VIDEO_HAVON_END,        3987},
    {P_ENCP_VIDEO_VAVON_BLINE,      89},
    {P_ENCP_VIDEO_VAVON_ELINE,      2248},

    {P_ENCP_VIDEO_HSO_BEGIN,	    44},
    {P_ENCP_VIDEO_HSO_END, 		    2156+1920},
    {P_ENCP_VIDEO_VSO_BEGIN,	    2100+1920},
    {P_ENCP_VIDEO_VSO_END, 		    2164+1920},

    {P_ENCP_VIDEO_VSO_BLINE,        51},
    {P_ENCP_VIDEO_VSO_ELINE,        53},
    {P_ENCP_VIDEO_MAX_LNCNT,        2249},

    {P_ENCP_VIDEO_FILT_CTRL,        0x1000}, //bypass filter
    {MREG_END_MARKER,       MREG_END_MARKER},
};

// Below is HDMI ENC*_DVI registers configure

const reg_t tvenc_regs_480i60hz[] = {
    {P_ENCI_DE_H_BEGIN,        0x000000f4},
    {P_ENCI_DE_H_END,          0x00000694},
    {P_ENCI_DE_V_BEGIN_EVEN,   0x00000012},
    {P_ENCI_DE_V_END_EVEN,     0x00000102},
    {P_ENCI_DE_V_BEGIN_ODD,    0x00000013},
    {P_ENCI_DE_V_END_ODD,      0x00000103},
    {P_ENCI_DVI_HSO_BEGIN,     0x00000006},
    {P_ENCI_DVI_HSO_END,       0x00000082},
    {P_ENCI_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCI_DVI_VSO_BLINE_ODD, 0x00000000},
    {P_ENCI_DVI_VSO_ELINE_EVN, 0x00000003},
    {P_ENCI_DVI_VSO_ELINE_ODD, 0x00000003},
    {P_ENCI_DVI_VSO_BEGIN_EVN, 0x00000006},
    {P_ENCI_DVI_VSO_BEGIN_ODD, 0x00000360},
    {P_ENCI_DVI_VSO_END_EVN,   0x00000006},
    {P_ENCI_DVI_VSO_END_ODD,   0x00000360},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_480p60hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000007},
    {P_ENCP_DVI_HSO_END      , 0x00000083},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000006},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x0000000c},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000007},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000007},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x000000fb},
    {P_ENCP_DE_H_END         , 0x0000069b},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x0000002a},
    {P_ENCP_DE_V_END_EVEN    , 0x0000020a},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

#if 0
const reg_t tvenc_regs_INTERLACE[] = {
    {P_ENCI_DE_H_BEGIN,        },
    {P_ENCI_DE_H_END,          },
    {P_ENCI_DE_V_BEGIN_EVEN,   },
    {P_ENCI_DE_V_END_EVEN,     },
    {P_ENCI_DE_V_BEGIN_ODD,    },
    {P_ENCI_DE_V_END_ODD,      },
    {P_ENCI_DVI_HSO_BEGIN,     },
    {P_ENCI_DVI_HSO_END,       },
    {P_ENCI_DVI_VSO_BLINE_EVN, },
    {P_ENCI_DVI_VSO_BLINE_ODD, },
    {P_ENCI_DVI_VSO_ELINE_EVN, },
    {P_ENCI_DVI_VSO_ELINE_ODD, },
    {P_ENCI_DVI_VSO_BEGIN_EVN, },
    {P_ENCI_DVI_VSO_BEGIN_ODD, },
    {P_ENCI_DVI_VSO_END_EVN,   },
    {P_ENCI_DVI_VSO_END_ODD,   },
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_PROGRESS[] = {
    {P_ENCP_DVI_HSO_BEGIN    , },
    {P_ENCP_DVI_HSO_END      , },
    {P_ENCP_DVI_VSO_BLINE_EVN, },
    {P_ENCP_DVI_VSO_BLINE_ODD, },
    {P_ENCP_DVI_VSO_ELINE_EVN, },
    {P_ENCP_DVI_VSO_ELINE_ODD, },
    {P_ENCP_DVI_VSO_BEGIN_EVN, },
    {P_ENCP_DVI_VSO_BEGIN_ODD, },
    {P_ENCP_DVI_VSO_END_EVN  , },
    {P_ENCP_DVI_VSO_END_ODD  , },
    {P_ENCP_DE_H_BEGIN       , },
    {P_ENCP_DE_H_END         , },
    {P_ENCP_DE_V_BEGIN_EVEN  , },
    {P_ENCP_DE_V_END_EVEN    , },
    {P_ENCP_DE_V_BEGIN_ODD   , },
    {P_ENCP_DE_V_END_ODD     , },
    {MREG_END_MARKER,     MREG_END_MARKER},
};
#endif

const reg_t tvenc_regs_576i50hz[] = {
    {P_ENCI_DE_H_BEGIN,        0x0000010c},
    {P_ENCI_DE_H_END,          0x000006ac},
    {P_ENCI_DE_V_BEGIN_EVEN,   0x00000016},
    {P_ENCI_DE_V_END_EVEN,     0x00000136},
    {P_ENCI_DE_V_BEGIN_ODD,    0x00000017},
    {P_ENCI_DE_V_END_ODD,      0x00000137},
    {P_ENCI_DVI_HSO_BEGIN,     0x00000004},
    {P_ENCI_DVI_HSO_END,       0x00000082},
    {P_ENCI_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCI_DVI_VSO_BLINE_ODD, 0x00000000},
    {P_ENCI_DVI_VSO_ELINE_EVN, 0x00000003},
    {P_ENCI_DVI_VSO_ELINE_ODD, 0x00000003},
    {P_ENCI_DVI_VSO_BEGIN_EVN, 0x00000004},
    {P_ENCI_DVI_VSO_BEGIN_ODD, 0x00000364},
    {P_ENCI_DVI_VSO_END_EVN,   0x00000004},
    {P_ENCI_DVI_VSO_END_ODD,   0x00000364},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_576p50hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x000006a5},
    {P_ENCP_DVI_HSO_END      , 0x00000065},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000270},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000004},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x000006a5},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x000006a5},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x000000ed},
    {P_ENCP_DE_H_END         , 0x0000068d},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x0000002c},
    {P_ENCP_DE_V_END_EVEN    , 0x0000026c},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_720p50hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000081},
    {P_ENCP_DVI_HSO_END      , 0x000000d1},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000004},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000009},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000081},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000081},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000289},
    {P_ENCP_DE_H_END         , 0x00000c89},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x0000001d},
    {P_ENCP_DE_V_END_EVEN    , 0x000002ed},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_720p60hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000081},
    {P_ENCP_DVI_HSO_END      , 0x000000d1},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000004},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000009},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000081},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000081},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000289},
    {P_ENCP_DE_H_END         , 0x00000c89},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x0000001d},
    {P_ENCP_DE_V_END_EVEN    , 0x000002ed},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_1080i50hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x0000008f},
    {P_ENCP_DVI_HSO_END      , 0x000000e7},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000232},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000005},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x00000237},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x0000008f},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000adf},
    {P_ENCP_DVI_VSO_END_EVN  , 0x0000008f},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000adf},
    {P_ENCP_DE_H_BEGIN       , 0x0000020f},
    {P_ENCP_DE_H_END         , 0x0000110f},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000014},
    {P_ENCP_DE_V_END_EVEN    , 0x00000230},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x00000247},
    {P_ENCP_DE_V_END_ODD     , 0x00000463},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_1080i60hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000085},
    {P_ENCP_DVI_HSO_END      , 0x000000dd},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000232},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000005},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x00000237},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000085},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x0000091d},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000085},
    {P_ENCP_DVI_VSO_END_ODD  , 0x0000091d},
    {P_ENCP_DE_H_BEGIN       , 0x00000205},
    {P_ENCP_DE_H_END         , 0x00001105},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000014},
    {P_ENCP_DE_V_END_EVEN    , 0x00000230},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x00000247},
    {P_ENCP_DE_V_END_ODD     , 0x00000463},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_1080p24hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000051},
    {P_ENCP_DVI_HSO_END      , 0x0000007d},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000005},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000051},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000051},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000111},
    {P_ENCP_DE_H_END         , 0x00000891},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000029},
    {P_ENCP_DE_V_END_EVEN    , 0x00000461},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_1080p50hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00000051},
    {P_ENCP_DVI_HSO_END      , 0x0000007d},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000000},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000005},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00000051},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00000051},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000111},
    {P_ENCP_DE_H_END         , 0x00000891},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000029},
    {P_ENCP_DE_V_END_EVEN    , 0x00000461},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_1080p60hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x0000086e},
    {P_ENCP_DVI_HSO_END      , 0x00000002},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000464},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000004},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x0000086e},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x0000086e},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000096},
    {P_ENCP_DE_H_END         , 0x00000816},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000029},
    {P_ENCP_DE_V_END_EVEN    , 0x00000461},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_4k2k24hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00001492},
    {P_ENCP_DVI_HSO_END      , 0x000014ea},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000006},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000010},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00001492},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00001492},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000096},
    {P_ENCP_DE_H_END         , 0x00000f96},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000059},
    {P_ENCP_DE_V_END_EVEN    , 0x000008c9},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_4k2ksmpte24hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00001492},
    {P_ENCP_DVI_HSO_END      , 0x000014ea},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000006},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000010},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00001492},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00001492},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000096},
    {P_ENCP_DE_H_END         , 0x00001096},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000059},
    {P_ENCP_DE_V_END_EVEN    , 0x000008c9},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_4k2k25hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x000013b6},
    {P_ENCP_DVI_HSO_END      , 0x0000140e},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000006},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000232},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000010},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x00000237},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x000013b6},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x0000091d},
    {P_ENCP_DVI_VSO_END_EVN  , 0x000013b6},
    {P_ENCP_DVI_VSO_END_ODD  , 0x0000091d},
    {P_ENCP_DE_H_BEGIN       , 0x00000096},
    {P_ENCP_DE_H_END         , 0x00000f96},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000059},
    {P_ENCP_DE_V_END_EVEN    , 0x000008c9},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x00000247},
    {P_ENCP_DE_V_END_ODD     , 0x00000463},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

const reg_t tvenc_regs_4k2k30hz[] = {
    {P_ENCP_DVI_HSO_BEGIN    , 0x00001046},
    {P_ENCP_DVI_HSO_END      , 0x0000109e},
    {P_ENCP_DVI_VSO_BLINE_EVN, 0x00000006},
    {P_ENCP_DVI_VSO_BLINE_ODD, 0x00000028},
    {P_ENCP_DVI_VSO_ELINE_EVN, 0x00000010},
    {P_ENCP_DVI_VSO_ELINE_ODD, 0x0000002a},
    {P_ENCP_DVI_VSO_BEGIN_EVN, 0x00001046},
    {P_ENCP_DVI_VSO_BEGIN_ODD, 0x00000010},
    {P_ENCP_DVI_VSO_END_EVN  , 0x00001046},
    {P_ENCP_DVI_VSO_END_ODD  , 0x00000020},
    {P_ENCP_DE_H_BEGIN       , 0x00000096},
    {P_ENCP_DE_H_END         , 0x00000f96},
    {P_ENCP_DE_V_BEGIN_EVEN  , 0x00000059},
    {P_ENCP_DE_V_END_EVEN    , 0x000008c9},
    {P_ENCP_DE_V_BEGIN_ODD   , 0x0000002a},
    {P_ENCP_DE_V_END_ODD     , 0x00000207},
    {MREG_END_MARKER,     MREG_END_MARKER},
};

// TODO, add other formats timing here
static struct enc_reg_map enc_regs_array[] = {
    {TVMODE_480I, &enc_480i60hz_set[0]},
    {TVMODE_480P, &enc_480p60hz_set[0]},
    {TVMODE_576I, &enc_576i50hz_set[0]},
    {TVMODE_576P, &enc_576p50hz_set[0]},
    {TVMODE_720P_50HZ, &enc_720p50hz_set[0]},
    {TVMODE_720P, &enc_720p60hz_set[0]},
    {TVMODE_1080I_50HZ, &enc_1080i50hz_set[0]},
    {TVMODE_1080I, &enc_1080i60hz_set[0]},
    {TVMODE_1080P_24HZ, &enc_1080p24hz_set[0]},
    {TVMODE_1080P_50HZ, &enc_1080p50hz_set[0]},
    {TVMODE_1080P, &enc_1080p60hz_set[0]},
    {TVMODE_4K2K_24HZ, &enc_4k2k24hz_set[0]},
    {TVMODE_4K2K_SMPTE, &enc_4k2ksmpte24hz_set[0]},
    {TVMODE_4K2K_25HZ, &enc_4k2k25hz_set[0]},
    {TVMODE_4K2K_30HZ, &enc_4k2k30hz_set[0]},
    {TVMODE_4K2K_FAKE_5G, &enc_4k2k5g_set[0]},
    {TVMODE_4K2K_60HZ, &enc_4k2k60hz_set[0]},
    {TVMODE_4K2K_50HZ, &enc_4k2k50hz_set[0]},
};

struct tv_hdmi_set_t hdmi_tvenc_regs_set[] = {
    {HDMI_720x480i60_16x9, tvenc_regs_480i60hz},
    {HDMI_720x480p60_16x9, tvenc_regs_480p60hz},
    {HDMI_720x576i50_16x9, tvenc_regs_576i50hz},
    {HDMI_720x576p50_16x9, tvenc_regs_576p50hz},
    {HDMI_1280x720p50_16x9, tvenc_regs_720p50hz},
    {HDMI_1280x720p60_16x9, tvenc_regs_720p60hz},
    {HDMI_1920x1080i50_16x9, tvenc_regs_1080i50hz},
    {HDMI_1920x1080i60_16x9, tvenc_regs_1080i60hz},
    {HDMI_1920x1080p24_16x9, tvenc_regs_1080p24hz},
    {HDMI_1920x1080p50_16x9, tvenc_regs_1080p50hz},
    {HDMI_1920x1080p60_16x9, tvenc_regs_1080p60hz},
    {HDMI_3840x2160p24_16x9, tvenc_regs_4k2k24hz},
    {HDMI_4096x2160p24_256x135, tvenc_regs_4k2ksmpte24hz},
    {HDMI_3840x2160p25_16x9, tvenc_regs_4k2k25hz},
    {HDMI_3840x2160p30_16x9, tvenc_regs_4k2k30hz},
    {HDMI_3840x2160p50_16x9, tvenc_regs_4k2k25hz},
    {HDMI_3840x2160p60_16x9, tvenc_regs_4k2k30hz},
};

// DON'T EDID BELOW TWO FUNCTIONS
void config_tv_enc(tvmode_t tvmode)
{
    int i;
    const struct enc_reg_set *reg;
    for(i = 0; i < sizeof(enc_regs_array) / sizeof(struct enc_reg_map); i++) {
        if(tvmode == enc_regs_array[i].tvmode) {
            reg = enc_regs_array[i].set;
            while(reg->addr != MREG_END_MARKER) {
                aml_write_reg32(reg->addr, reg->val);
                reg ++;
            }
            aml_write_reg32(P_VENC_VDAC_SETTING, 0xff);     // turn off vdac, hdmitx no need this
            return;
        }
    }
    aml_write_reg32(P_VENC_VDAC_SETTING, 0xff);     // turn off vdac, hdmitx no need this
    printk("not find output_type %d\n", tvmode);
} /* config_tv_enc */

void config_hdmi_tvenc(HDMI_Video_Codes_t vic)
{
    const reg_t *s = NULL;
    int i = 0;
    for(i = 0; i < ARRAY_SIZE(hdmi_tvenc_regs_set); i++) {
        if(vic == hdmi_tvenc_regs_set[i].vic) {
            s = hdmi_tvenc_regs_set[i].reg_setting;
            break;
        }
    }
    if(s) {
        while (MREG_END_MARKER != s->reg) {
            aml_write_reg32(s->reg, s->val);
            s++;
        }
    }
    else {
        printk("VIC %d REGS SETTING FAILED\n", vic);
    }
} /* config_hdmi_tvenc */
