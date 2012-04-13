void init_dmc(struct ddr_set * ddr_setting)
{	
	writel(ddr_setting->ddr_ctrl, P_MMC_DDR_CTRL);
	writel(0xffff, P_DMC_SEC_PORT0_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT1_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT2_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT3_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT4_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT5_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT6_RANGE0);
    writel(0xffff, P_DMC_SEC_PORT7_RANGE0);
    writel(0x80000000, P_DMC_SEC_CTRL);


#ifdef CONFIG_DDR_LOW_POWER		

	//move from ddr_init_pctl.c	
	writel(ddr_setting->t_refi_100ns,0xc8006434);//#define P_MMC_LP_CTRL4		0xc8006434

	writel(0x34400f03,0xc8006430);//#define P_MMC_LP_CTRL3 	  0xc8006430
		
	writel(0x8160203,0xc800642c); //#define P_MMC_LP_CTRL2	0xc800642c

	writel(0x78000030,0xc8006428);//#define P_MMC_LP_CTRL1 	  0xc8006428 
			
#endif

	writel(0xff, P_MMC_REQ_CTRL);

}
int ddr_init_hw(struct ddr_set * timing_reg)
{
    if(timing_reg->init_pctl(timing_reg))
        return 1;

    init_dmc(timing_reg);

    return 0;
}
