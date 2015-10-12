#include <asm/arch/lcdoutc.h>

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

#define CRT_VID_DIV_MAX			255
#define CRT_VID_MAX_CLK_IN		(3000 * 1000)

/* g9tv, g9bb divider */
#define CLK_DIV_I2O     0
#define CLK_DIV_O2I     1

struct pll_para_s {
	unsigned int m;
	unsigned int n;
	unsigned int frac;
	unsigned int od1_sel;
	unsigned int od2_sel;
	unsigned int od3_sel;
};

enum div_sel_e {
	CLK_DIV_SEL_1 = 0,
	CLK_DIV_SEL_2,    /* 1 */
	CLK_DIV_SEL_3,    /* 2 */
	CLK_DIV_SEL_3p5,  /* 3 */
	CLK_DIV_SEL_3p75, /* 4 */
	CLK_DIV_SEL_4,    /* 5 */
	CLK_DIV_SEL_5,    /* 6 */
	CLK_DIV_SEL_6,    /* 7 */
	CLK_DIV_SEL_6p25, /* 8 */
	CLK_DIV_SEL_7,    /* 9 */
	CLK_DIV_SEL_7p5,  /* 10 */
	CLK_DIV_SEL_12,   /* 11 */
	CLK_DIV_SEL_14,   /* 12 */
	CLK_DIV_SEL_15,   /* 13 */
	CLK_DIV_SEL_2p5,  /* 14 */
	CLK_DIV_SEL_MAX,
};


/* ********************************************
// for clk parameter auto generation
// ********************************************* */
/**** clk parameters bit ***/
/* pll_ctrl */
#define PLL_CTRL_LOCK			31
#define PLL_CTRL_EN			30
#define PLL_CTRL_RST			28
#define PLL_CTRL_OD3			22 /* [23:22] */
#define PLL_CTRL_OD2			18 /* [19:18] */
#define PLL_CTRL_OD1			16 /* [17:16] */
#define PLL_CTRL_N			9 /* [13:9] */
#define PLL_CTRL_M			0 /* [8:0] */

/* div_ctrl */
#define DIV_CTRL_CLK_DIV		0 /* [7:0] */

/* clk_ctrl */
#define CLK_CTRL_FRAC			16 /* [27:16] */
#define CLK_CTRL_XD 			0  //[7:0]

#define PLL_WAIT_LOCK_CNT		500

/* freq: unit in kHz */
#define FIN_FREQ			(24 * 1000)
#define PLL_VCO_MIN 			(3000 * 1000)
#define PLL_VCO_MAX 			(6000 * 1000)
/* VIDEO */
#define CLK_DIV_MAX_CLK_IN		(3000 * 1000)
#define CRT_VID_MAX_CLK_IN		(3000 * 1000)
#define ENCL_MAX_CLK_IN 		(666 * 1000)

#define OD_SEL_MAX			3


extern void set_vclk_lcd(Lcd_Config_t *pconf);
extern void clocks_set_vid_clk_div(int div_sel);
extern int set_hpll_pll(int freq, int od1,int hdmi_pll_cntl5);
extern void set_crt_video_enc(int vIdx, int inSel, int DivN);
extern void enable_crt_video_encl(int enable, int inSel);
extern void _enable_vsync_interrupt(void);
extern void vpp_set_matrix_ycbcr2rgb (int vd1_or_vd2_or_post, int mode);

extern void lcd_test(unsigned int num, Lcd_Config_t *pconf);

extern void generate_clk_parameter(Lcd_Config_t *pConf);
