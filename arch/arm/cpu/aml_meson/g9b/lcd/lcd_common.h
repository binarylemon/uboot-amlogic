
#define VPP_OUT_SATURATE            (1 << 0)

#define CLK_UTIL_VID_PLL_DIV_1      0
#define CLK_UTIL_VID_PLL_DIV_2      1
#define CLK_UTIL_VID_PLL_DIV_3      2
#define CLK_UTIL_VID_PLL_DIV_3p5    3
#define CLK_UTIL_VID_PLL_DIV_3p75   4
#define CLK_UTIL_VID_PLL_DIV_4      5
#define CLK_UTIL_VID_PLL_DIV_5      6
#define CLK_UTIL_VID_PLL_DIV_6      7
#define CLK_UTIL_VID_PLL_DIV_6p25   8
#define CLK_UTIL_VID_PLL_DIV_7      9
#define CLK_UTIL_VID_PLL_DIV_7p5    10
#define CLK_UTIL_VID_PLL_DIV_12     11
#define CLK_UTIL_VID_PLL_DIV_14     12
#define CLK_UTIL_VID_PLL_DIV_15     13
#define CLK_UTIL_VID_PLL_DIV_2p5    14


void clocks_set_vid_clk_div(int div_sel);
int set_hpll_pll(int freq, int od1,int hdmi_pll_cntl5);
void set_crt_video_enc(int vIdx, int inSel, int DivN);
void enable_crt_video_encl(int enable, int inSel);
void vpp_set_matrix_ycbcr2rgb (int vd1_or_vd2_or_post, int mode);
