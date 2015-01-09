/*
 * Driver for the amlogic vpu controller
 *
 *
 */
#include <config.h>
#include <asm/arch/io.h> 
#include <asm/arch/clock.h>

#define VPU_VERION	"v01"

extern void udelay(unsigned long usec);
extern void mdelay(unsigned long msec);
extern int printf(const char *fmt, ...);
#ifdef CONFIG_OF_LIBFDT
extern int fdt_path_offset(const void *fdt, const char *path);
extern const char *fdt_strerror(int errval);
extern const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp);
extern int fdt_check_header(const void *fdt);
#endif

typedef struct {
	unsigned int clk_level_dft;
	unsigned int clk_level_max;
	unsigned int clk_level;
}VPU_Conf_t;

static char * dt_addr;
static int dts_ready = 0;

//!!! Only for g9tv vpu gp1 clock test, please double check these settings!!!!!
#define GP1_CLK_TEST

#ifdef GP1_CLK_TEST
  #define CLK_LEVEL_DFT		10   // also need to check the dtd setting
  #define CLK_LEVEL_MAX		11  // only for gp1_pll_clk test
#else
  #define CLK_LEVEL_DFT		7
  #define CLK_LEVEL_MAX		10	//limit max clk to 637M
#endif
static unsigned int vpu_clk_setting[][3] = {
	//frequency		clk_mux		div
	{106250000,		1,			7},	//0
	{127500000,		2,			3},	//1
	{159375000,		0,			3},	//2
	{212500000,		1,			3},	//3
	{255000000,		2,			1},	//4
	{283333000,		1,			2},	//5
	{318750000,		0,			1},	//6
	{425000000,		1,			1},	//7
	{510000000,		2,			0},	//8
	{637500000,		0,			0},	//9
#ifdef GP1_CLK_TEST
	{696000000,		7,			0},	//10
	{850000000,		1,			0},	//11
#else
	{850000000,		1,			0},	//10
#endif
};

static VPU_Conf_t vpu_config = {
	.clk_level_dft = CLK_LEVEL_DFT,
	.clk_level_max = CLK_LEVEL_MAX,
	.clk_level = CLK_LEVEL_DFT,
};

static unsigned int get_vpu_clk_level(unsigned int video_clk)
{
	unsigned int video_bw;
	unsigned clk_level;
	int i;

	video_bw = video_clk + 1000000;

	for (i=0; i<vpu_config.clk_level_max; i++) {
		if (video_bw <= vpu_clk_setting[i][0])
			break;
	}
	clk_level = i;

	return clk_level;
}

static unsigned int get_vpu_clk(void)
{
	unsigned int clk_freq;
	unsigned int clk_source, clk_div;
	
	switch ((readl(P_HHI_VPU_CLK_CNTL) >> 9) & 0x7) {
		case 0:
			clk_source = 637500000;
			break;
		case 1:
			clk_source = 850000000;
			break;
		case 2:
			clk_source = 510000000;
			break;
		case 3:
			clk_source = 364300000;
			break;
#ifdef GP1_CLK_TEST
		case 7:
			clk_source = 696000000;
			break;
#endif
		default:
			clk_source = 0;
			break;
	}
	
	clk_div = ((readl(P_HHI_VPU_CLK_CNTL) >> 0) & 0x7f) + 1;
	clk_freq = clk_source / clk_div;
	
	return clk_freq;
}

#ifdef GP1_CLK_TEST
static int switch_gp1_pll(int flag)
{
	int cnt = 100;
	int ret = 0;
	
	if (flag) { //enable gp1_pll
		/* GP1 DPLL 696MHz output*/
		writel(0x6a01023a, P_HHI_GP1_PLL_CNTL);
		writel(0x69c80000, P_HHI_GP1_PLL_CNTL2);
		writel(0x0a5590c4, P_HHI_GP1_PLL_CNTL3); //0x0a674a21
		writel(0x0000500d, P_HHI_GP1_PLL_CNTL4); //0x0000000d
		writel(0x4a01023a, P_HHI_GP1_PLL_CNTL);
		do{
			udelay(10);
			setbits_le32(P_HHI_GP1_PLL_CNTL, (1 << 29)); //reset
			udelay(50);
			clrbits_le32(P_HHI_GP1_PLL_CNTL, (1 << 29)); //release reset
			udelay(50);
		}while(((readl(P_HHI_GP1_PLL_CNTL)&(1<<31)) == 0) && (cnt > 0));
		if (cnt == 0) {
			ret = 1;
			clrbits_le32(P_HHI_GP1_PLL_CNTL, (1 << 30));
			printf("[error]: GP_PLL lock failed, can't use the clk source!\n");
		}
	}
	else { //disable gp_pll
		clrbits_le32(P_HHI_GP1_PLL_CNTL, (1 << 30));
	}
	
	return ret;
}
#endif

static int set_vpu_clk(unsigned int vclk)
{
	int ret = 0;
	unsigned clk_level;
	
	if (vclk >= 100) {	//regard as vpu_clk
		clk_level = get_vpu_clk_level(vclk);
	}
	else {	//regard as clk_level
		clk_level = vclk;
	}

	if (clk_level >= vpu_config.clk_level_max) {
		ret = 1;
		clk_level = vpu_config.clk_level_dft;
		printf("vpu clk out of supported range, set to default\n");
	}
	
	vpu_config.clk_level = clk_level;
	
#ifdef GP1_CLK_TEST
	if (clk_level == 10) {
		ret = switch_gp1_pll(1);
		if (ret)
			clk_level = 9;
	}
	else
		ret = switch_gp1_pll(0);
	
	writel(((readl(P_HHI_VPU_CLK_CNTL) & (~(0x7 << 25))) | (vpu_clk_setting[0][1] << 25)), P_HHI_VPU_CLK_CNTL);
	writel(((readl(P_HHI_VPU_CLK_CNTL) & (~(0x7f << 16))) | (vpu_clk_setting[0][2] << 16)), P_HHI_VPU_CLK_CNTL);
	setbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 24));
	setbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 31));
	udelay(10);
	clrbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 8));
	writel(((readl(P_HHI_VPU_CLK_CNTL) & (~(0x7 << 9))) | (vpu_clk_setting[clk_level][1] << 9)), P_HHI_VPU_CLK_CNTL);
	writel(((readl(P_HHI_VPU_CLK_CNTL) & (~(0x7f << 0))) | (vpu_clk_setting[clk_level][2] << 0)), P_HHI_VPU_CLK_CNTL);
	setbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 8));
	udelay(20);
	clrbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 31));
	clrbits_le32(P_HHI_VPU_CLK_CNTL, (1 << 24));
#else
	writel(((vpu_clk_setting[clk_level][1] << 9) | (vpu_clk_setting[clk_level][2] << 0)), P_HHI_VPU_CLK_CNTL);
	writel(readl(P_HHI_VPU_CLK_CNTL) | (1<<8), P_HHI_VPU_CLK_CNTL);
#endif
	
	printf("set vpu clk: %uHz, readback: %uHz(0x%x)\n", vpu_clk_setting[clk_level][0], get_vpu_clk(), (readl(P_HHI_VPU_CLK_CNTL)));

	return ret;
}

static void power_switch_to_vpu_hdmi(int pwr_ctrl)
{
    unsigned int i;
    if(pwr_ctrl == 1) {
        // Powerup VPU_HDMI
        clrbits_le32(P_AO_RTI_GEN_PWR_SLEEP0, (1 << 8));

        // power up memories
        for(i = 0; i < 32; i++) {
            clrbits_le32(P_HHI_VPU_MEM_PD_REG0, (1 << i));
            mdelay(2);
        }
        for(i = 0; i < 32; i++) {
            clrbits_le32(P_HHI_VPU_MEM_PD_REG1, (1 << i));
            mdelay(2);
        }
        for(i = 8; i < 16; i++) {
            clrbits_le32(P_HHI_MEM_PD_REG0, (1 << i));
            mdelay(2);
        }
        // Remove VPU_HDMI ISO
        clrbits_le32(P_AO_RTI_GEN_PWR_SLEEP0, (1 << 9));
    } else {
        // Add isolations
        setbits_le32(P_AO_RTI_GEN_PWR_SLEEP0, (1 << 9));
        // Power off VPU_HDMI domain
        for(i = 0; i < 32; i++) {
            setbits_le32(P_HHI_VPU_MEM_PD_REG0, (1 << i));
            mdelay(2);
        }
        for(i = 0; i < 32; i++) {
            setbits_le32(P_HHI_VPU_MEM_PD_REG1, (1 << i));
            mdelay(2);
        }
        for(i = 8; i < 16; i++) {
            setbits_le32(P_HHI_MEM_PD_REG0, (1 << i));
            mdelay(2);
        }
        setbits_le32(P_AO_RTI_GEN_PWR_SLEEP0, (1 << 8));
    }
}

static void set_vpu_defconf(void)
{
    power_switch_to_vpu_hdmi(1);
}

static void vpu_driver_disable(void)
{
    power_switch_to_vpu_hdmi(0);
}

static int get_vpu_config(void)
{
	//int ret=0;
	int nodeoffset;
	char * propdata;
	
	if (dts_ready == 1) {
#ifdef CONFIG_OF_LIBFDT
		nodeoffset = fdt_path_offset(dt_addr, "/vpu");
		if(nodeoffset < 0) {
			printf("vpu preset: not find /vpu node in dts %s.\n",fdt_strerror(nodeoffset));
			return 0;
		}
		
		propdata = (char *)fdt_getprop(dt_addr, nodeoffset, (const char *)("clk_level"), NULL);
		if(propdata == NULL){
			vpu_config.clk_level = vpu_config.clk_level_dft;
			printf("don't find to match clk_level in dts, use default setting.\n");
		}
		else {
			vpu_config.clk_level = (unsigned short)(be32_to_cpup((u32*)propdata));
			printf("vpu clk_level in dts: %u\n", vpu_config.clk_level);
		}
#endif
	}
	else {
		vpu_config.clk_level = vpu_config.clk_level_dft;
		printf("vpu clk_level = %u\n", vpu_config.clk_level);
	}
	return 0;
}

int vpu_probe(void)
{
	dts_ready = 0;
#ifdef CONFIG_OF_LIBFDT
#ifdef CONFIG_DT_PRELOAD
#ifdef CONFIG_DTB_LOAD_ADDR
	dt_addr = (char *)CONFIG_DTB_LOAD_ADDR;
#else
	dt_addr = (char *)0x0f000000;
#endif
	int ret;
	ret = fdt_check_header(dt_addr);
	if(ret < 0) {
		printf("check dts: %s, load default vpu parameters\n", fdt_strerror(ret));
	}
	else {
		dts_ready = 1;
	}
#endif
#endif
	
	get_vpu_config();
	set_vpu_defconf();
	set_vpu_clk(vpu_config.clk_level);

	return 0;
}

int vpu_remove(void)
{
	vpu_driver_disable();
	return 0;
}
