#ifndef MIPI_DSI_UTIL_H
#define MIPI_DSI_UTIL_H

#include <asm/arch/lcdoutc.h>
#include <asm/arch/register.h>

// --------------------------------------------------------
// MIPI DSI Data Type/ MIPI DCS Command Type Definitions
// --------------------------------------------------------
typedef enum {DT_VSS                 = 0x01,
        DT_VSE                 = 0x11,
        DT_HSS                 = 0x21,
        DT_HSE                 = 0x31,
        DT_EOTP                = 0x08,
        DT_CMOFF               = 0x02,
        DT_CMON                = 0x12,
        DT_SHUT_DOWN           = 0x22,
        DT_TURN_ON             = 0x32,
        DT_GEN_SHORT_WR_0      = 0x03,
        DT_GEN_SHORT_WR_1      = 0x13,
        DT_GEN_SHORT_WR_2      = 0x23,
        DT_GEN_RD_0            = 0x04,
        DT_GEN_RD_1            = 0x14,
        DT_GEN_RD_2            = 0x24,
        DT_DCS_SHORT_WR_0      = 0x05,
        DT_DCS_SHORT_WR_1      = 0x15,
        DT_DCS_RD_0            = 0x06,
        DT_SET_MAX_RPS         = 0x37,
        DT_NULL_PKT            = 0x09,
        DT_BLANK_PKT           = 0x19,
        DT_GEN_LONG_WR         = 0x29,
        DT_DCS_LONG_WR         = 0x39,
        DT_20BIT_LOOSE_YCBCR   = 0x0c,
        DT_24BIT_YCBCR         = 0x1c,
        DT_16BIT_YCBCR         = 0x2c,
        DT_30BIT_RGB_101010    = 0x0d,
        DT_36BIT_RGB_121212    = 0x1d,
        DT_12BIT_YCBCR         = 0x3d,
        DT_16BIT_RGB_565       = 0x0e,
        DT_18BIT_RGB_666       = 0x1e,
        DT_18BIT_LOOSE_RGB_666 = 0x2e,
        DT_24BIT_RGB_888       = 0x3e
} mipi_dsi_data_type_t;

// DCS Command List
#define DCS_ENTER_IDLE_MODE          0x39
#define DCS_ENTER_INVERT_MODE        0x21
#define DCS_ENTER_NORMAL_MODE        0x13
#define DCS_ENTER_PARTIAL_MODE       0x12
#define DCS_ENTER_SLEEP_MODE         0x10
#define DCS_EXIT_IDLE_MODE           0x38
#define DCS_EXIT_INVERT_MODE         0x20
#define DCS_EXIT_SLEEP_MODE          0x11
#define DCS_GET_3D_CONTROL           0x3f
#define DCS_GET_ADDRESS_MODE         0x0b
#define DCS_GET_BLUE_CHANNEL         0x08
#define DCS_GET_DIAGNOSTIC_RESULT    0x0f
#define DCS_GET_DISPLAY_MODE         0x0d
#define DCS_GET_GREEN_CHANNEL        0x07
#define DCS_GET_PIXEL_FORMAT         0x0c
#define DCS_GET_POWER_MODE           0x0a
#define DCS_GET_RED_CHANNEL          0x06
#define DCS_GET_SCANLINE             0x45
#define DCS_GET_SIGNAL_MODE          0x0e
#define DCS_NOP                      0x00
#define DCS_READ_DDB_CONTINUE        0xa8
#define DCS_READ_DDB_START           0xa1
#define DCS_READ_MEMORY_CONTINUE     0x3e
#define DCS_READ_MEMORY_START        0x2e
#define DCS_SET_3D_CONTROL           0x3d
#define DCS_SET_ADDRESS_MODE         0x36
#define DCS_SET_COLUMN_ADDRESS       0x2a
#define DCS_SET_DISPLAY_OFF          0x28
#define DCS_SET_DISPLAY_ON           0x29
#define DCS_SET_GAMMA_CURVE          0x26
#define DCS_SET_PAGE_ADDRESS         0x2b
#define DCS_SET_PARTIAL_COLUMNS      0x31
#define DCS_SET_PARTIAL_ROWS         0x30
#define DCS_SET_PIXEL_FORMAT         0x3a
#define DCS_SET_SCROLL_AREA          0x33
#define DCS_SET_SCROLL_START         0x37
#define DCS_SET_TEAR_OFF             0x34
#define DCS_SET_TEAR_ON              0x35
#define DCS_SET_TEAR_SCANLINE        0x44
#define DCS_SET_VSYNC_TIMING         0x40
#define DCS_SOFT_RESET               0x01
#define DCS_WRITE_LUT                0x2d
#define DCS_WRITE_MEMORY_CONTINUE    0x3c
#define DCS_WRITE_MEMORY_START       0x2c

// --------------------------------------------------------
// MIPI DCS Pixel-to-Byte Format
// --------------------------------------------------------
// DCS pixel-to-byte format
#define DCS_PF_RSVD                  0x0
#define DCS_PF_3BIT                  0x1
#define DCS_PF_8BIT                  0x2
#define DCS_PF_12BIT                 0x3
#define DCS_PF_16BIT                 0x5
#define DCS_PF_18BIT                 0x6
#define DCS_PF_24BIT                 0x7

// --------------------------------------------------------
// MIPI DSI/VENC Color Format Definitions
// --------------------------------------------------------
#define MIPI_DSI_VENC_COLOR_30B 0x0
#define MIPI_DSI_VENC_COLOR_24B 0x1
#define MIPI_DSI_VENC_COLOR_18B 0x2
#define MIPI_DSI_VENC_COLOR_16B 0x3

#define COLOR_16BIT_CFG_1  0x0
#define COLOR_16BIT_CFG_2  0x1
#define COLOR_16BIT_CFG_3  0x2
#define COLOR_18BIT_CFG_1  0x3
#define COLOR_18BIT_CFG_2  0x4
#define COLOR_24BIT        0x5
#define COLOR_20BIT_LOOSE  0x6
#define COLOR_24_BIT_YCBCR 0x7
#define COLOR_16BIT_YCBCR  0x8
#define COLOR_30BIT        0x9
#define COLOR_36BIT        0xa
#define COLOR_12BIT        0xb
#define COLOR_RGB_111      0xc
#define COLOR_RGB_332      0xd
#define COLOR_RGB_444      0xe

// --------------------------------------------------------
// MIPI DSI Relative REGISTERs Definitions
// --------------------------------------------------------
// For MIPI_DSI_TOP_CNTL
#define BIT_DPI_COLOR_MODE    20
#define BIT_IN_COLOR_MODE     16
#define BIT_CHROMA_SUBSAMPLE  14
#define BIT_COMP2_SEL         12
#define BIT_COMP1_SEL         10
#define BIT_COMP0_SEL          8
#define BIT_DE_POL             6
#define BIT_HSYNC_POL          5
#define BIT_VSYNC_POL          4
#define BIT_DPICOLORM          3
#define BIT_DPISHUTDN          2
#define BIT_EDPITE_INTR_PULSE  1
#define BIT_ERR_INTR_PULSE     0

// For MIPI_DSI_DWC_CLKMGR_CFG_OS 
#define BIT_TO_CLK_DIV     8
#define BIT_TX_ESC_CLK_DIV 0

// For MIPI_DSI_DWC_PCKHDL_CFG_OS
#define BIT_CRC_RX_EN      4
#define BIT_ECC_RX_EN      3
#define BIT_BTA_EN         2
#define BIT_EOTP_RX_EN     1
#define BIT_EOTP_TX_EN     0

// For MIPI_DSI_DWC_VID_MODE_CFG_OS
#define BIT_LP_CMD_EN        15
#define BIT_FRAME_BTA_ACK_EN 14
#define BIT_LP_HFP_EN        13
#define BIT_LP_HBP_EN        12
#define BIT_LP_VCAT_EN       11
#define BIT_LP_VFP_EN        10
#define BIT_LP_VBP_EN         9
#define BIT_LP_VSA_EN         8
#define BIT_VID_MODE_TYPE     0

#define NON_BURST_SYNC_PULSE  0x0
#define NON_BURST_SYNC_EVENT  0x1
#define BURST_MODE            0x2

// For MIPI_DSI_DWC_PHY_STATUS_OS
#define BIT_PHY_ULPSACTIVENOT3LANE 12
#define BIT_PHY_STOPSTATE3LANE     11
#define BIT_PHY_ULPSACTIVENOT2LANE 10
#define BIT_PHY_STOPSTATE2LANE      9
#define BIT_PHY_ULPSACTIVENOT1LANE  8
#define BIT_PHY_STOPSTATE1LANE      7
#define BIT_PHY_RXULPSESC0LANE      6
#define BIT_PHY_ULPSACTIVENOT0LANE  5
#define BIT_PHY_STOPSTATE0LANE      4
#define BIT_PHY_ULPSACTIVENOTCLK    3
#define BIT_PHY_STOPSTATECLKLANE    2
#define BIT_PHY_DIRECTION           1
#define BIT_PHY_LOCK                0

// For MIPI_DSI_DWC_PHY_IF_CFG_OS
#define BIT_PHY_STOP_WAIT_TIME      8
#define BIT_N_LANES                 0

// For MIPI_DSI_DWC_DPI_COLOR_CODING_OS
#define BIT_LOOSELY18_EN            8
#define BIT_DPI_COLOR_CODING        0

// For MIPI_DSI_DWC_GEN_HDR_OS
#define BIT_GEN_WC_MSBYTE          16
#define BIT_GEN_WC_LSBYTE           8
#define BIT_GEN_VC                  6
#define BIT_GEN_DT                  0

// For MIPI_DSI_DWC_LPCLK_CTRL_OS
#define BIT_AUTOCLKLANE_CTRL        1
#define BIT_TXREQUESTCLKHS          0

// For MIPI_DSI_DWC_DPI_CFG_POL_OS
#define BIT_COLORM_ACTIVE_LOW       4
#define BIT_SHUTD_ACTIVE_LOW        3
#define BIT_HSYNC_ACTIVE_LOW        2
#define BIT_VSYNC_ACTIVE_LOW        1
#define BIT_DATAEN_ACTIVE_LOW       0

// For MIPI_DSI_DWC_CMD_MODE_CFG_OS
#define BIT_MAX_RD_PKT_SIZE        24
#define BIT_DCS_LW_TX              19 
#define BIT_DCS_SR_0P_TX           18
#define BIT_DCS_SW_1P_TX           17
#define BIT_DCS_SW_0P_TX           16
#define BIT_GEN_LW_TX              14
#define BIT_GEN_SR_2P_TX           13
#define BIT_GEN_SR_1P_TX           12
#define BIT_GEN_SR_0P_TX           11
#define BIT_GEN_SW_2P_TX           10
#define BIT_GEN_SW_1P_TX            9
#define BIT_GEN_SW_0P_TX            8
#define BIT_ACK_RQST_EN             1
#define BIT_TEAR_FX_EN              0

// For MIPI_DSI_DWC_CMD_PKT_STATUS_OS
#define BIT_DBI_RD_CMD_BUSY        14                                          // For DBI no usefull 
#define BIT_DBI_PLD_R_FULL         13                                          // For DBI no usefull 
#define BIT_DBI_PLD_R_EMPTY        12                                          // For DBI no usefull 
#define BIT_DBI_PLD_W_FULL         11                                          // For DBI no usefull 
#define BIT_DBI_PLD_W_EMPTY        10                                          // For DBI no usefull 
#define BIT_DBI_CMD_FULL            9                                          // For DBI no usefull 
#define BIT_DBI_CMD_EMPTY           8                                          // For DBI no usefull 

#define BIT_GEN_RD_CMD_BUSY         6                                          // For Generic interface 
#define BIT_GEN_PLD_R_FULL          5                                          // For Generic interface               
#define BIT_GEN_PLD_R_EMPTY         4                                          // For Generic interface    
#define BIT_GEN_PLD_W_FULL          3                                          // For Generic interface 
#define BIT_GEN_PLD_W_EMPTY         2                                          // For Generic interface 
#define BIT_GEN_CMD_FULL            1                                          // For Generic interface 
#define BIT_GEN_CMD_EMPTY           0                                          // For Generic interface 

// For MIPI_DSI_TOP_MEAS_CNTL
#define BIT_CNTL_MEAS_VSYNC        10                                          // measure vsync control
#define BIT_EDPITE_MEAS_EN          9                                          // tear measure enable
#define BIT_EDPITE_ACCUM_MEAS_EN    8                                          // not clear the counter
#define BIT_EDPITE_VSYNC_SPAN       0                                          // 

// For MIPI_DSI_TOP_STAT
#define BIT_STAT_EDPIHALT          31                                          // signal from halt
#define BIT_STAT_TE_LINE           16                                          // line number when edpite pulse
#define BIT_STAT_TE_PIXEL           0                                          // pixel number when edpite pulse

// For MIPI_DSI_TOP_INTR_CNTL_STAT
#define BIT_STAT_CLR_DWC_PIC_EOF   21                                          // State/Clear for pic_eof
#define BIT_STAT_CLR_DWC_DE_FALL   20                                          // State/Clear for de_fall
#define BIT_STAT_CLR_DWC_DE_RISE   19                                          // State/Clear for de_rise
#define BIT_STAT_CLR_DWC_VS_FALL   18                                          // State/Clear for vs_fall
#define BIT_STAT_CLR_DWC_VS_RISE   17                                          // State/Clear for vs_rise
#define BIT_STAT_CLR_DWC_EDPITE    16                                          // State/Clear for edpite
#define BIT_PIC_EOF                 5                                          // end of picture
#define BIT_DE_FALL                 4                                          // data enable fall
#define BIT_DE_RISE                 3                                          // data enable rise
#define BIT_VS_FALL                 2                                          // vsync fall
#define BIT_VS_RISE                 1                                          // vsync rise
#define BIT_EDPITE_INT_EN           0                                          // edpite int enable

// For MIPI_DSI_TOP_MEAS_CNTL
#define BIT_VSYNC_MEAS_EN          19                                          // vsync measure enable
#define BIT_VSYNC_ACCUM_MEAS_EN    18                                          // vsync accumulate measure
#define BIT_VSYNC_SPAN             10                                          // vsync span
#define BIT_TE_MEAS_EN              9                                          // tearing measure enable
#define BIT_TE_ACCUM_MEAS_EN        8                                          // tearing accumulate measure
#define BIT_TE_SPAN                 0                                          // tearing span

// For MIPI_DSI_DWC_INT_ST0_OS
#define BIT_DPHY_ERR_4             20                                          // LP1 contention error from lane0
#define BIT_DPHY_ERR_3             19                                          // LP0 contention error from lane0
#define BIT_DPHY_ERR_2             18                                          // ErrControl error from lane0
#define BIT_DPHY_ERR_1             17                                          // ErrSyncEsc error from lane0
#define BIT_DPHY_ERR_0             16                                          // ErrEsc escape error lane0
#define BIT_ACK_ERR_15             15
#define BIT_ACK_ERR_14             14
#define BIT_ACK_ERR_13             13
#define BIT_ACK_ERR_12             12
#define BIT_ACK_ERR_11             11
#define BIT_ACK_ERR_10             10
#define BIT_ACK_ERR_9               9
#define BIT_ACK_ERR_8               8
#define BIT_ACK_ERR_7               7
#define BIT_ACK_ERR_6               6
#define BIT_ACK_ERR_5               5
#define BIT_ACK_ERR_4               4
#define BIT_ACK_ERR_3               3
#define BIT_ACK_ERR_2               2
#define BIT_ACK_ERR_1               1
#define BIT_ACK_ERR_0               0

// Operation mode parameters
#define OPERATION_VIDEO_MODE        0
#define OPERATION_COMMAND_MODE      1

// Command transfer type in command mode
#define DCS_TRANS_HS                0
#define DCS_TRANS_LP                1

#define MIPI_DSI_DCS_NO_ACK         0
#define MIPI_DSI_DCS_REQ_ACK        1

// Video mode video timing parameters
#define SCLK_PERIOD                   40                                       // ns

#define MIPI_DSI_REFRESH_RATE_50HZ    50
#define MIPI_DSI_REFRESH_RATE_40HZ    40
#define MIPI_DSI_REFRESH_RATE_30HZ    30
#define MIPI_DSI_REFRESH_RATE_24HZ    24

#define MIPI_DSI_REFRESH_RATE         MIPI_DSI_REFRESH_RATE_40HZ

#define PCLK_PERIOD_BIG_50HZ          (4467/1000)    // 50Hz
#define PCLK_PERIOD_BIG_45HZ          (4964/1000)    // 45Hz
#define PCLK_PERIOD_BIG_40HZ          (5598/1000)    // 40Hz
#define PCLK_PERIOD_BIG_35HZ          (6382/1000)    // 35Hz
#define PCLK_PERIOD_BIG_30HZ          (75/10)      // 30Hz

#define PCLK_PERIOD_SMALL             16
#define BYTELANECLK_PERIOD_SMALL      8

#define PCLK_PERIOD_BIG               (                                                                               \
                (MIPI_DSI_REFRESH_RATE == MIPI_DSI_REFRESH_RATE_50HZ) ? PCLK_PERIOD_BIG_50HZ : \
                (MIPI_DSI_REFRESH_RATE == MIPI_DSI_REFRESH_RATE_40HZ) ? PCLK_PERIOD_BIG_40HZ : \
                (MIPI_DSI_REFRESH_RATE == MIPI_DSI_REFRESH_RATE_30HZ) ? PCLK_PERIOD_BIG_30HZ : \
                PCLK_PERIOD_BIG_30HZ   \
                )
#define BYTELANECLK_PERIOD_BIG        (9952/1000)

#define PCLK_PERIOD_1920X1200         (6488/1000)    // 60Hz
#define BYTELANECLK_PERIOD_1920X1200  (8651/1000)   //

// Different Resolution Settings
// Only 24bit color format will use the real clock setting and be overriden outside

#define MAX_PIXCNT_240X160            367
#define MAX_LNCNT_240X160             187
#define HFP_240X160                   (MAX_PIXCNT_240X160-HSA_240X160-HBP_240X160-HACT_240X160+1)    // 91
#define HSA_240X160                   (31-15)                                                        // 16
#define HBP_240X160                   (50-31+2)   // 21 add 2 because: data path is 2 clks later than sync
#define HACT_240X160                  (239+50-50+1)  // 240
#define VFP_240X160                   (MAX_LNCNT_240X160-VSA_240X160-VBP_240X160-VACT_240X160) 
#define VSA_240X160                   (12-10)
#define VBP_240X160                   (20-12)
#define VACT_240X160                  (159+20-20+1)
#define MIPI_HLINE_240X160            ((HFP_240X160+HSA_240X160+HBP_240X160+HACT_240X160)*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HSA_240X160              (HSA_240X160*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HBP_240X160              (HBP_240X160*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)

#define MAX_PIXCNT_240X160_dsi        (400-1)
#define MAX_LNCNT_240X160_dsi         (195-1)
#define HFP_240X160_dsi               48
#define HSA_240X160_dsi               32
#define HBP_240X160_dsi               80
#define HACT_240X160_dsi              240
#define VFP_240X160_dsi               3
#define VSA_240X160_dsi               6
#define VBP_240X160_dsi               26
#define VACT_240X160_dsi              160
#define MIPI_HLINE_240X160_dsi        ((HFP_240X160_dsi+HSA_240X160_dsi+HBP_240X160_dsi+HACT_240X160_dsi)*3/4)
#define MIPI_HSA_240X160_dsi          (HSA_240X160_dsi*3/4)
#define MIPI_HBP_240X160_dsi          (HBP_240X160_dsi*3/4)

#define MAX_PIXCNT_240X160_slow        (400-1)
#define MAX_LNCNT_240X160_slow         (195-1)
#define HFP_240X160_slow               48
#define HSA_240X160_slow               32
#define HBP_240X160_slow               80
#define HACT_240X160_slow              240
#define VFP_240X160_slow               3
#define VSA_240X160_slow               6
#define VBP_240X160_slow               26
#define VACT_240X160_slow              160
#define MIPI_HLINE_240X160_slow        ((HFP_240X160_slow+HSA_240X160_slow+HBP_240X160_slow+HACT_240X160_slow)*200/160)
#define MIPI_HSA_240X160_slow          (HSA_240X160_slow*200/160)
#define MIPI_HBP_240X160_slow          (HBP_240X160_slow*200/160)

#define MAX_PIXCNT_480X234            607     
#define MAX_LNCNT_480X234             261     
#define HFP_480X234                   (MAX_PIXCNT_480X234-HSA_480X234-HBP_480X234-HACT_480X234+1)
#define HSA_480X234                   (31-15)
#define HBP_480X234                   (50-31+2)                                  // add 2 because: data path is 2 clks later than sync
#define HACT_480X234                  (479+50-50+1)
#define VFP_480X234                   (MAX_LNCNT_480X234-VSA_480X234-VBP_480X234-VACT_480X234) 
#define VSA_480X234                   (12-10)
#define VBP_480X234                   (20-12)
#define VACT_480X234                  (233+20-20+1)
#define MIPI_HLINE_480X234            ((HFP_480X234+HSA_480X234+HBP_480X234+HACT_480X234)*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HSA_480X234              (HSA_480X234*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HBP_480X234              (HBP_480X234*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)

#define MAX_PIXCNT_720X480            857
#define MAX_LNCNT_720X480             524     
#define HFP_720X480                   (MAX_PIXCNT_720X480-HSA_720X480-HBP_720X480-HACT_720X480+1)
#define HSA_720X480                   (31-15)
#define HBP_720X480                   (50-31+2)                                  // add 2 because: data path is 2 clks later than sync
#define HACT_720X480                  (479+50-50+1)
#define VFP_720X480                   (MAX_LNCNT_720X480-VSA_720X480-VBP_720X480-VACT_720X480) 
#define VSA_720X480                   (12-10)
#define VBP_720X480                   (20-12)
#define VACT_720X480                  (233+20-20+1)
#define MIPI_HLINE_720X480            ((HFP_720X480+HSA_720X480+HBP_720X480+HACT_720X480)*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HSA_720X480              (HSA_720X480*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HBP_720X480              (HBP_720X480*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)

#define MAX_PIXCNT_720X576            863
#define MAX_LNCNT_720X576             624
#define HFP_720X576                   (MAX_PIXCNT_720X576-HSA_720X576-HBP_720X576-HACT_720X576+1)
#define HSA_720X573                   (5-3)
#define HBP_720X576                   (124-5+2)                                  // add 2 because: data path is 2 clks later than sync
#define HACT_720X576                  (843-124+1)
#define VFP_720X576                   (MAX_LNCNT_720X576-VSA_720X576-VBP_720X576-VACT_720X576) 
#define VSA_720X576                   (16-11)
#define VBP_720X576                   (44-16)
#define VACT_720X576                  (624-44+1)
#define MIPI_HLINE_720X576            ((HFP_720X576+HSA_720X576+HBP_720X576+HACT_720X576)*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HSA_720X576              (HSA_720X576*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)
#define MIPI_HBP_720X576              (HBP_720X576*PCLK_PERIOD_SMALL/BYTELANECLK_PERIOD_SMALL)

#define MAX_PIXCNT_1920x1200p         (2080-1)
#define MAX_LNCNT_1920x1200p          (1235-1)
#define HFP_1920x1200p                48
#define HSA_1920x1200p                32
#define HBP_1920x1200p                80
#define HACT_1920x1200p               1920
#define VFP_1920x1200p                3
#define VSA_1920x1200p                6
#define VBP_1920x1200p                26
#define VACT_1920x1200p               1200
#define MIPI_HLINE_1920x1200p         ((HFP_1920x1200p+HSA_1920x1200p+HBP_1920x1200p+HACT_1920x1200p)*3/4)
#define MIPI_HSA_1920x1200p           (HSA_1920x1200p*3/4)
#define MIPI_HBP_1920x1200p           (HBP_1920x1200p*3/4)

#define HFP_768X1024                60
#define HSA_768X1024                64
#define HBP_768X1024                56
#define HACT_768X1024               768
#define HLINE_768X1024              (HFP_768X1024+HSA_768X1024+HBP_768X1024+HACT_768X1024)
#define VFP_768X1024                36
#define VSA_768X1024                50
#define VBP_768X1024                30
#define VACT_768X1024               1024

#define MAX_PIXCNT_2560X1600          2719
#define MAX_LNCNT_2560X1600           1645    
#define HFP_2560X1600                 (MAX_PIXCNT_2560X1600-HSA_2560X1600-HBP_2560X1600-HACT_2560X1600+1)
#define HSA_2560X1600                 (59-27)
#define HBP_2560X1600                 (139-59+2)                                  // add 2 because: data path is 2 clks later than sync
#define HACT_2560X1600                (2559+139-139+1)
#define VFP_2560X1600                 (MAX_LNCNT_2560X1600-VSA_2560X1600-VBP_2560X1600-VACT_2560X1600) 
#define VSA_2560X1600                 (7-1)
#define VBP_2560X1600                 (43-7)
#define VACT_2560X1600                (1599+43-43+1)
#define MIPI_HLINE_2560X1600          ((HFP_2560X1600+HSA_2560X1600+HBP_2560X1600+HACT_2560X1600)*PCLK_PERIOD_BIG/BYTELANECLK_PERIOD_BIG)
#define MIPI_HSA_2560X1600            (HSA_2560X1600*PCLK_PERIOD_BIG/BYTELANECLK_PERIOD_BIG)
#define MIPI_HBP_2560X1600            (HBP_2560X1600*PCLK_PERIOD_BIG/BYTELANECLK_PERIOD_BIG)

#define MAX_PIXCNT_1920X1200          2079
#define MAX_LNCNT_1920X1200           1234  
#define HFP_1920X1200                 48 // (MAX_PIXCNT_1920X1200-HSA_1920X1200-HBP_1920X1200-HACT_1920X1200+1)
#define HSA_1920X1200                 32 // (59-27)
#define HBP_1920X1200                 80 // (148-59+2)                                  // add 2 because: data path is 2 clks later than sync
#define HACT_1920X1200                1920 // (2067-148+1)
#define VFP_1920X1200                 (MAX_LNCNT_1920X1200-VSA_1920X1200-VBP_1920X1200-VACT_1920X1200) 
#define VSA_1920X1200                 (7-1)
#define VBP_1920X1200                 (31-7)
#define VACT_1920X1200                (1230-31+1)
#define MIPI_HLINE_1920X1200          ((HFP_1920X1200+HSA_1920X1200+HBP_1920X1200+HACT_1920X1200)*PCLK_PERIOD_1920X1200/BYTELANECLK_PERIOD_1920X1200)
#define MIPI_HSA_1920X1200            (HSA_1920X1200*PCLK_PERIOD_1920X1200/BYTELANECLK_PERIOD_1920X1200)
#define MIPI_HBP_1920X1200            (HBP_1920X1200*PCLK_PERIOD_1920X1200/BYTELANECLK_PERIOD_1920X1200)

// DSI Tear Defines
#define MIPI_DCS_SET_TEAR_ON_MODE_0   0
#define MIPI_DCS_SET_TEAR_ON_MODE_1   1
#define MIPI_DCS_ENABLE_TEAR          1
#define MIPI_DCS_DISABLE_TEAR         0

// Pixel FIFO Depth
#define PIXEL_FIFO_DEPTH            1440

#define BYTE_PER_PIXEL_COLOR_16BIT_CFG_1  2
#define BYTE_PER_PIXEL_COLOR_16BIT_CFG_2  2
#define BYTE_PER_PIXEL_COLOR_16BIT_CFG_3  2
#define BYTE_PER_PIXEL_COLOR_18BIT_CFG_1  3
#define BYTE_PER_PIXEL_COLOR_18BIT_CFG_2  3
#define BYTE_PER_PIXEL_COLOR_24BIT        3
#define BYTE_PER_PIXEL_COLOR_20BIT_LOOSE  3
#define BYTE_PER_PIXEL_COLOR_24_BIT_YCBCR 3
#define BYTE_PER_PIXEL_COLOR_16BIT_YCBCR  2
#define BYTE_PER_PIXEL_COLOR_30BIT        4
#define BYTE_PER_PIXEL_COLOR_36BIT        5
#define BYTE_PER_PIXEL_COLOR_12BIT        3                                    // in fact it should be 1.5(12bit)

// Tearing Interrupt Bit
#define INT_TEARING                       6

typedef enum tv_enc_type_e{
        TV_ENC_480i = 0,
        TV_ENC_576i,
        TV_ENC_480p,
        TV_ENCP_480p,
        TV_ENC_576p,
        TV_ENC_1080i, 
        TV_ENC_720p, 
        TV_ENC_1080p, 
        TV_ENC_2205p,
        TV_ENC_2440p, 
        TV_ENC_3840x2160p_vic01, 
        TV_ENC_3840x2160p_vic03, 
        TV_ENC_4096x2160p_vic04, 
        TV_ENC_TYPE_MAX
} tv_enc_type_t;   /* tv encoder output format */

typedef enum tv_enc_lcd_type_e{
        TV_ENC_LCD480x234 = 0,
        TV_ENC_LCD480x234_dsi36b = 1,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD240x160 = 2,
        TV_ENC_LCD240x160_dsi36b = 3,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD720x480 = 4,
        TV_ENC_LCD720x480_dsi36b = 5,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD720x576 = 6,
        TV_ENC_LCD720x576_dsi36b = 7,       // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD1280x720 = 8,
        TV_ENC_LCD1280x720_dsi36b = 9,      // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD1920x1080 = 10,
        TV_ENC_LCD1920x1080_dsi36b = 11,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD1920x2205 = 12,
        TV_ENC_LCD1920x2205_dsi36b = 13,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD2560x1600 = 14,
        TV_ENC_LCD2560x1600_dsi36b = 15,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD3840x2440 = 16,
        TV_ENC_LCD3840x2440_dsi36b = 17,    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
        TV_ENC_LCD3840x2160p_vic03 = 18,
        TV_ENC_LCD4096x2160p_vic04 = 19,
        TV_ENC_LCD640x480 = 20,
        TV_ENC_LCD1920x1200p = 21, 
        TV_ENC_LCD240x160_dsi   = 22,
        TV_ENC_LCD240x160_slow   = 23,
        TV_ENC_LCD3840x2160p_vic01 = 24,
        TV_ENC_LCD2048x1536  = 25,
        TV_ENC_LCD768x1024p = 26,
        TV_ENC_LCD_TYPE_MAX
} tv_enc_lcd_type_t;   /* tv encoder output format */

// DCS COMMAND LIST
#define DCS_CMD_CODE_ENTER_IDLE_MODE      0x0
#define DCS_CMD_CODE_ENTER_INVERT_MODE    0x1  
#define DCS_CMD_CODE_ENTER_NORMAL_MODE    0x2  
#define DCS_CMD_CODE_ENTER_PARTIAL_MODE   0x3  
#define DCS_CMD_CODE_ENTER_SLEEP_MODE     0x4  
#define DCS_CMD_CODE_EXIT_IDLE_MODE       0x5  
#define DCS_CMD_CODE_EXIT_INVERT_MODE     0x6  
#define DCS_CMD_CODE_EXIT_SLEEP_MODE      0x7  
#define DCS_CMD_CODE_NOP                  0x8 
#define DCS_CMD_CODE_SET_DISPLAY_OFF      0x9 
#define DCS_CMD_CODE_SET_DISPLAY_ON       0xa 
#define DCS_CMD_CODE_SET_TEAR_OFF         0xb 
#define DCS_CMD_CODE_SOFT_RESET           0xc

extern void mipi_dsi_link_on(Lcd_Config_t *pConf);
extern void mipi_dsi_link_off(Lcd_Config_t *pConf);
extern void set_mipi_dsi_control(Lcd_Config_t *pConf);
extern void mipi_dsi_off(void);

#endif
