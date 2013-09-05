#ifndef _GIC_H_
#define _GIC_H_


#define  INT_TYPE_FIQ       0
#define  INT_TYPE_IRQ       1

// SPI Interrupt Vector
#define INT_VEC_VPU_ISP                 6*32+25
#define INT_VEC_EDP_TX                  6*32+24
#define INT_VEC_MALI_PPPMU7             6*32+19
#define INT_VEC_MALI_PP7                6*32+18
#define INT_VEC_MALI_PPPMU6             6*32+17
#define INT_VEC_MALI_PP6                6*32+16
#define INT_VEC_MALI_PPPMU5             6*32+15
#define INT_VEC_MALI_PP5                6*32+14
#define INT_VEC_MALI_PPPMU4             6*32+13
#define INT_VEC_MALI_PP4                6*32+12
#define INT_VEC_MALI_PPPMU3             6*32+11
#define INT_VEC_MALI_PP3                6*32+10
#define INT_VEC_MALI_PPPMU2             6*32+9
#define INT_VEC_MALI_PP2                6*32+8
#define INT_VEC_MALI_PPPMU1             6*32+7
#define INT_VEC_MALI_PP1                6*32+6
#define INT_VEC_MALI_PPPMU0             6*32+5
#define INT_VEC_MALI_PP0                6*32+4
#define INT_VEC_MALI_PMU                6*32+3
#define INT_VEC_MALI_PP                 6*32+2
#define INT_VEC_MALI_GPMMU              6*32+1
#define INT_VEC_MALI_GP                 6*32+0

#define INT_VEC_VIU2_LINE               5*32+31
#define INT_VEC_A9_DGB_COMRX3           5*32+30
#define INT_VEC_A9_DGB_COMRX2           5*32+29
#define INT_VEC_A9_DGB_COMTX3           5*32+28
#define INT_VEC_A9_DGB_COMTX2           5*32+27
#define INT_VEC_A9_PMU3                 5*32+26
#define INT_VEC_A9_PMU2                 5*32+25
#define INT_VEC_VIU1_LINE               5*32+24
#define INT_VEC_AO_CEC                  5*32+23
#define INT_VEC_GE2D                    5*32+22
#define INT_VEC_CUSAD                   5*32+21
#define INT_VEC_ASSIST_MBOX3            5*32+20
#define INT_VEC_ASSIST_MBOX2            5*32+19
#define INT_VEC_ASSIST_MBOX1            5*32+18
#define INT_VEC_ASSIST_MBOX0            5*32+17
#define INT_VEC_RESV4_0                 5*32+16
#define INT_VEC_L2CACHE_CNTR            5*32+15
#define INT_VEC_A9_DGB_COMRX1           5*32+14
#define INT_VEC_A9_DGB_COMRX0           5*32+13
#define INT_VEC_A9_DGB_COMTX1           5*32+12
#define INT_VEC_A9_DGB_COMTX0           5*32+11
#define INT_VEC_A9_PMU1                 5*32+10
#define INT_VEC_A9_PMU0                 5*32+9
#define INT_VEC_DEMOD                   5*32+8
#define INT_VEC_IR_BLASTER              5*32+7
#define INT_VEC_MIPI_DSI_TEAR           5*32+6
#define INT_VEC_MIPI_DSI_ERR            5*32+5
#define INT_VEC_RESV4_2                 5*32+4
#define INT_VEC_RESV4_3                 5*32+3
#define INT_VEC_SEC                     5*32+2
#define INT_VEC_M_I2C_2                 5*32+1
#define INT_VEC_M_I2C_1                 5*32+0 


#define INT_VEC_VDEC2DOS_17             4*32+31
#define INT_VEC_VDEC2DOS_16             4*32+30
#define INT_VEC_VDEC2DOS_15             4*32+29
#define INT_VEC_VDEC2DOS_14             4*32+28
#define INT_VEC_VDEC2DOS_13             4*32+27
#define INT_VEC_VDEC2DOS_12             4*32+26
#define INT_VEC_VDEC2DOS_11             4*32+25
#define INT_VEC_VDEC2DOS_10             4*32+24
#define INT_VEC_VDEC2DOS_9              4*32+23
#define INT_VEC_DOS_13                  4*32+22
#define INT_VEC_VDEC2DOS_8              4*32+21
#define INT_VEC_VDEC2DOS_7              4*32+20
#define INT_VEC_DOS_12                  4*32+19
#define INT_VEC_DOS_11                  4*32+18
#define INT_VEC_VDEC2DOS_6              4*32+17
#define INT_VEC_VDEC2DOS_5              4*32+16
#define INT_VEC_DOS_10                  4*32+15
#define INT_VEC_VDEC2DOS_4              4*32+14
#define INT_VEC_VDEC2DOS_3              4*32+13
#define INT_VEC_DOS_9                   4*32+12
#define INT_VEC_DOS_8                   4*32+11
#define INT_VEC_DOS_7                   4*32+10
#define INT_VEC_VDEC2DOS_2              4*32+9
#define INT_VEC_VDEC2DOS_1              4*32+8
//#define INT_VEC_DOS_6                   4*32+7 //refer to INT_VEC_AI_IEC958
#define INT_VEC_DOS_5                   4*32+6
#define INT_VEC_DOS_4                   4*32+5
#define INT_VEC_VDEC2DOS_0              4*32+4
#define INT_VEC_DOS_3                   4*32+3
#define INT_VEC_DOS_2                   4*32+2
#define INT_VEC_DOS_1                   4*32+1
#define INT_VEC_DOS_0                   4*32+0 


#define INT_VEC_HCODEC2DOS_17           4*32+31
#define INT_VEC_HCODEC2DOS_16           4*32+30
#define INT_VEC_HCODEC2DOS_15           4*32+29
#define INT_VEC_HCODEC2DOS_14           4*32+28
#define INT_VEC_HCODEC2DOS_13           4*32+27
#define INT_VEC_HCODEC2DOS_12           4*32+26
#define INT_VEC_HCODEC2DOS_11           4*32+25
#define INT_VEC_HCODEC2DOS_10           4*32+24
#define INT_VEC_HCODEC2DOS_9            4*32+23
//~ #define INT_VEC_DOS_13              4*32+22
#define INT_VEC_HCODEC2DOS_8            4*32+21
#define INT_VEC_HCODEC2DOS_7            4*32+20
//~ #define INT_VEC_DOS_12              4*32+19
//~ #define INT_VEC_DOS_11              4*32+18
#define INT_VEC_HCODEC2DOS_6            4*32+17
#define INT_VEC_HCODEC2DOS_5            4*32+16
//~ #define INT_VEC_DOS_10              4*32+15
#define INT_VEC_HCODEC2DOS_4            4*32+14
#define INT_VEC_HCODEC2DOS_3            4*32+13
//~ #define INT_VEC_DOS_9               4*32+12
//~ #define INT_VEC_DOS_8               4*32+11
//~ #define INT_VEC_DOS_7               4*32+10
#define INT_VEC_HCODEC2DOS_2            4*32+9
#define INT_VEC_HCODEC2DOS_1            4*32+8
//~ #define INT_VEC_DOS_6               4*32+7
//~ #define INT_VEC_DOS_5               4*32+6
//~ #define INT_VEC_DOS_4               4*32+5
#define INT_VEC_HCODEC2DOS_0            4*32+4
//~ #define INT_VEC_DOS_3               4*32+3
//~ #define INT_VEC_DOS_2               4*32+2
//~ #define INT_VEC_DOS_1               4*32+1
//~ #define INT_VEC_DOS_0               4*32+0


#define INT_VEC_CSI2_ADAPT              3*32+31
#define INT_VEC_UART3                   3*32+30
#define INT_VEC_UART2                   3*32+29
#define INT_VEC_AO_M_I2C                3*32+28
#define INT_VEC_AO_S_I2C                3*32+27
#define INT_VEC_AO_UART                 3*32+26
#define INT_VEC_RDMA_DONE               3*32+25
#define INT_VEC_I2S_CBUS_DDR            3*32+24
#define INT_VEC_CSI2_HOST_2             3*32+23
#define INT_VEC_VID1_WR                 3*32+22
#define INT_VEC_VDIN1_VSYNC             3*32+21
#define INT_VEC_VDIN1_HSYNC             3*32+20
#define INT_VEC_VDIN0_VSYNC             3*32+19
#define INT_VEC_VDIN0_HSYNC             3*32+18
#define INT_VEC_SPI2                    3*32+17
#define INT_VEC_SPI                     3*32+16
#define INT_VEC_VID0_WR                 3*32+15
#define INT_VEC_SDHC                    3*32+14
#define INT_VEC_ACODEC                  3*32+13
#define INT_VEC_LED_PWM                 3*32+12
#define INT_VEC_UART1                   3*32+11
#define INT_VEC_CSI2_HOST_1             3*32+10
#define INT_VEC_SAR_ADC                 3*32+9
#define INT_VEC_AO_RTC                  3*32+8
#define INT_VEC_GPIO_I_NOW_7            3*32+7
#define INT_VEC_GPIO_I_NOW_6            3*32+6
#define INT_VEC_GPIO_I_NOW_5            3*32+5
#define INT_VEC_GPIO_I_NOW_4            3*32+4
#define INT_VEC_GPIO_I_NOW_3            3*32+3
#define INT_VEC_GPIO_I_NOW_2            3*32+2
#define INT_VEC_GPIO_I_NOW_1            3*32+1
#define INT_VEC_GPIO_I_NOW_0            3*32+0 

#define INT_VEC_TIMERI                  2*32+31
#define INT_VEC_TIMERH                  2*32+30
#define INT_VEC_TIMERG                  2*32+29
#define INT_VEC_TIMERF                  2*32+28
//#define INT_VEC_MALI_PPMMU1             2*32+27
//#define INT_VEC_MALI_PP1                2*32+26
#define INT_VEC_HDMI_TX                 2*32+25
#define INT_VEC_HDMI_RX                 2*32+24
#define INT_VEC_HDMI_CEC                2*32+23
#define INT_VEC_AUTO_MEDIA_CPU_CLK      2*32+22
#define INT_VEC_DEMUX_2                 2*32+21
#define INT_VEC_DMC                     2*32+20
#define INT_VEC_DMC_SEC                  2*32+19
#define INT_VEC_AI_IEC958               2*32+18
#define INT_VEC_IEC958_DDR              2*32+17
#define INT_VEC_I2S                 2*32+16
//#define INT_VEC_DMC                     2*32+15
#define INT_VEC_RESV2_0                 2*32+14
#define INT_VEC_MBOX_SLOW2              2*32+13
#define INT_VEC_MBOX_SLOW1              2*32+12
#define INT_VEC_MBOX_SLOW0              2*32+11
#define INT_VEC_MBOX_FAST2              2*32+10
#define INT_VEC_MBOX_FAST1              2*32+9
#define INT_VEC_MBOX_FAST0              2*32+8
#define INT_VEC_S_I2C                   2*32+7
#define INT_VEC_AO_UART2                2*32+6
#define INT_VEC_SMARTCARD               2*32+5
#define INT_VEC_NDMA                    2*32+4
#define INT_VEC_SPDIF                   2*32+3
#define INT_VEC_PNAND                   2*32+2
#define INT_VEC_VIFF_EMPTY              2*32+1
#define INT_VEC_PARSER                  2*32+0 


#define INT_VEC_USB1                    1*32+31
#define INT_VEC_USB0                    1*32+30
#define INT_VEC_TIMERD                  1*32+29
#define INT_VEC_ARC_SDIO                1*32+28
#define INT_VEC_ETH_PMT                 1*32+27
#define INT_VEC_UART0                   1*32+26
#define INT_VEC_ASYNC_FIFO2_FLUSH       1*32+25
#define INT_VEC_ASYNC_FIFO2_FILL        1*32+24
#define INT_VEC_DEMUX                   1*32+23
#define INT_VEC_ENCIF                   1*32+22
#define INT_VEC_M_I2C_0                 1*32+21
#define INT_VEC_BT656                   1*32+20
#define INT_VEC_ASYNC_FIFO_FLUSH        1*32+19
#define INT_VEC_ASYNC_FIFO_FILL         1*32+18
#define INT_VEC_ABUF_RD                 1*32+17
#define INT_VEC_ABUF_WR                 1*32+16
#define INT_VEC_AO_IR_DEC               1*32+15
#define INT_VEC_MIPI_PHY                1*32+14
#define INT_VEC_VIU2_VSYNC              1*32+13
#define INT_VEC_VIU2_HSYNC              1*32+12
#define INT_VEC_TIMERB                  1*32+11
#define INT_VEC_TIMERA                  1*32+10
#define INT_VEC_AUTO_SYS_CPU_CLK        1*32+9
#define INT_VEC_ETH_GMAC                1*32+8
#define INT_VEC_AUDIN                   1*32+7
#define INT_VEC_TIMERC                  1*32+6
#define INT_VEC_DEMUX_1                 1*32+5
#define INT_VEC_ETH_LPI                 1*32+4
#define INT_VEC_VIU1_VSYNC              1*32+3
#define INT_VEC_VIU1_HSYNC              1*32+2
#define INT_VEC_MAILBOX                 1*32+1
#define INT_VEC_WD_TIMER                1*32+0 


// PPI interrupt
#define INT_VEC_PPI5_NIRQ               31      // low-level sensitive
#define INT_VEC_PPI4_WD_TIMER           30      // rising-edge sensitive
#define INT_VEC_PPI3_PRIV_TIMER         29      // rising-edge sensitive
#define INT_VEC_PPI1_NFIQ               28      // low-level sensitive
#define INT_VEC_PPI0_GLOBAL_TIMER       27      // rising-edge sensitive

// software generated interrupt
#define INT_VEC_SGI_15                  15
#define INT_VEC_SGI_14                  14
#define INT_VEC_SGI_13                  13
#define INT_VEC_SGI_12                  12
#define INT_VEC_SGI_11                  11
#define INT_VEC_SGI_10                  10
#define INT_VEC_SGI_9                   9
#define INT_VEC_SGI_8                   8
#define INT_VEC_SGI_7                   7
#define INT_VEC_SGI_6                   6
#define INT_VEC_SGI_5                   5
#define INT_VEC_SGI_4                   4
#define INT_VEC_SGI_3                   3
#define INT_VEC_SGI_2                   2
#define INT_VEC_SGI_1                   1
#define INT_VEC_SGI_0                   0 

// ----------------------------------------
// registers
// ----------------------------------------
// SCU registers
#define SCU_CTRL         0xc4300000
#define SCU_CONF         0xc4300004
#define SCU_PWR_STUS     0xc4300008
#define SCU_INVALID_RAM  0xc430000c
#define SCU_SSAC         0xc4300054

// CPU interface registers
#define GICI_CNTL        0xc4300100
#define GICI_PRI_MSK     0xc4300104
#define GICI_ACK         0xc430010C
#define GICI_END_INT     0xc4300110

// distributer registers
#define GICD_CNTL        0xc4301000     // whether respond to change in the status (ns or s banked)
                                                                  // 0: don't change state from external stimulus (SPI or PPI), but no impact to internal
                                                                  // 1: enable update state
#define GICD_ICTR        0xc4301004     // Interrupt control type
#define GICD_DIST_ID     0xc4301008     // distributer implementer identification
#define GICD_INT_TYPE    0xc4301080     // 80~9c Interrupt Security Registers (each processor have its own bank registers)
#define GICD_SET_EN      0xc4301100
#define GICD_CLR_EN      0xc4301180
#define GICD_SET_PEND    0xc4301200
#define GICD_CLR_PEND    0xc4301280
#define GICD_ACTIVE      0xc4301300     // 300~31c active status (RO)
#define GICD_PERI        0xc4301400     // 400~4fc priority level
#define GICD_TARGET      0xc4301800     // 800~8fc SPI target register
#define GICD_CONF        0xc4301c00     // c00~c3c interrupt configuration register (high level or posedge)
#define GICD_PPI_STUS    0xc4301D00     // PPI status
#define GICD_SPI_STUS    0xc4301D04     // d04~d1c SPI status
#define GICD_SGI         0xc4301F00     // f00 Software generated interrupt


// external functions

void cpu0_gic_init(void);
void cpu123_gic_init(void);
void set_priority_mask(int priority);
void set_int_priority(int intVec, int priority);
void set_int_sensitive(int intVec, int sensitive);
int check_pending_state(int intVec);
void set_int_type(int intVec, int type);  // INT_TYPE_FIQ: security, 0, FIQ   INT_TYPE_IRQ: non-security, 1, IRQ
void set_int_enable(int intvec);
void set_int_disable(int intVec);
void send_sgi(unsigned int id, unsigned int target_list, unsigned int filter_list, unsigned int non_sec);


#endif
