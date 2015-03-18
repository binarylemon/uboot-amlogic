
void init_dmc(struct ddr_set * timing_set)
{
	//serial_puts("timing_set->t_mmc_ddr_ctrl: ");
	//serial_put_hex(timing_set->t_mmc_ddr_ctrl, 32);
	//serial_puts("\n");
	writel(timing_set->t_mmc_ddr_ctrl, P_DMC_DDR_CTRL);

	writel(0xffff0000, DMC_SEC_RANGE0_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE1_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE2_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE3_CTRL);
	writel(0xffffffff, DMC_SEC_AXI_PORT_CTRL);
	writel(0xffffffff, DMC_SEC_AM_PORT_CTRL);
	writel(0xffffffff, DMC_DEV_RANGE_CTRL);
	writel(0xffffffff, DMC_DEV_RANGE_CTRL1);
	writel(0x80000000, DMC_SEC_CTRL);

	//changed for AXI and AMBUS arbiter weight control.
	writel((0x1f | (0xf << 6)), P_DMC_2ARB_CTRL);

	//enable the DMC auto refresh function
	writel(0x20109a27, P_DMC_REFR_CTRL2);
	writel(0x80389f, P_DMC_REFR_CTRL1);

	//enable the dc_reqs.
	writel(0xffff, P_DMC_REQ_CTRL);
	writel(0x7, P_DMC_N_CLK_CTRL);

	//put some code here to try to stop bus traffic
	asm("NOP");
	asm("DMB");
	asm("ISB");

	//change PL310 address filtering to allow DRAM reads to go to M1
	writel(0xbff00000, 0xc4200c04);
	writel(0x00000001, 0xc4200c00);
}

#ifdef DDR_SCRAMBE_ENABLE
void ddr_scramble(void){
	unsigned int dmc_sec_ctrl_value;
	unsigned int ddr_key;

	ddr_key = readl(P_RAND64_ADDR0);
	writel(ddr_key, DMC_SEC_KEY);
	dmc_sec_ctrl_value = 0x80000000 | (1<<0);
	writel(dmc_sec_ctrl_value, DMC_SEC_CTRL);
	while( readl(DMC_SEC_CTRL) & 0x80000000 ) {}
}
#endif

int ddr_init_hw(struct ddr_set * timing_set)
{
	int ret = 0;
	ret = timing_set->init_pctl(timing_set);

	if(ret)
	{
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
		//serial_puts("PUB init fail!\n");
		return -1;
#else
		serial_puts("\nPUB init fail! Reset...\n");
		__udelay(10000);
		AML_WATCH_DOG_START();
#endif
	}

	init_dmc(timing_set);

#ifdef DDR_SCRAMBE_ENABLE
	ddr_scramble();
#endif

	return 0;
}

void ddr_info_dump(struct ddr_set * timing_set)
{
#ifdef CONFIG_DUMP_DDR_INFO
	int nPLL = readl(AM_DDR_PLL_CNTL);
	int nDDRCLK = 2*((24 / ((nPLL>>9)& 0x1F) ) * (nPLL & 0x1FF))/ (1<<((nPLL>>16) & 0x3));
	//serial_puts("DDR clock: ");
	serial_puts(" @ ");
	serial_put_dec(nDDRCLK);
	serial_puts("MHz(");
#ifdef CONFIG_DDR_LOW_POWER
	serial_puts("LP&");
#endif
	if((timing_set->t_pctl_mcfg) & (1<<3)) //DDR0, DDR1 same setting?
		serial_puts("2T)");
	else
		serial_puts("1T)");
#endif
#if defined(DDR_SCRAMBE_ENABLE) || defined(CONFIG_DUMP_DDR_INFO)
	unsigned int dmc_sec_ctrl_value;
	dmc_sec_ctrl_value = readl(DMC_SEC_CTRL);
	if(dmc_sec_ctrl_value & (1<<0)){
		serial_puts("+Scramb EN");
	}
#endif
	serial_puts("\n");
}
