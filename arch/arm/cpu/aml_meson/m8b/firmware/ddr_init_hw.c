
void init_dmc(struct ddr_set * timing_set)
{
	writel(0xffffffff, P_DMC_SOFT_RST);
	writel(0xffffffff, P_DMC_SOFT_RST1);
	writel(0x20109a27, P_DMC_REFR_CTRL2);
	writel(0x80389d, P_DMC_REFR_CTRL1);
	//serial_put_hex(timing_set->t_mmc_ddr_ctrl, 32);
	//serial_puts("\n");
	//writel(timing_set->t_mmc_ddr_ctrl, P_DMC_DDR_CTRL);
if(cfg_ddr_mode == CFG_DDR_32BIT)
	writel(0x0013e3e, P_DMC_DDR_CTRL);
else
	writel(0xc01bebe, P_DMC_DDR_CTRL);

	writel(0xffff0000, DMC_SEC_RANGE0_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE1_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE2_CTRL);
	writel(0xffffffff, DMC_SEC_RANGE3_CTRL);
	writel(0xffffffff, DMC_SEC_AXI_PORT_CTRL);
	writel(0xffffffff, DMC_SEC_AM_PORT_CTRL);
	writel(0xffffffff, DMC_DEV_RANGE_CTRL);
	writel(0xffffffff, DMC_DEV_RANGE_CTRL1);
	writel(0x80000000, DMC_SEC_CTRL);
	//writel(0x12a, P_DDR0_CLK_CTRL);
	writel(0xffff, P_DMC_REQ_CTRL);

	//change PL310 address filtering to allow DRAM reads to go to M1
	writel(0xc0000000, 0xc4200c04);
	writel(0x00000001, 0xc4200c00);

	//put some code here to try to stop bus traffic
	asm("NOP");
	asm("DMB");
	asm("ISB");

	serial_put_hex(readl(P_DMC_DDR_CTRL), 32);
	serial_puts("\n");
	//if(timing_set->t_mmc_ddr_ctrl >> ):
}

int ddr_init_hw(struct ddr_set * timing_set)
{
	int ret = 0;
#ifdef DDR_SCRAMBE_ENABLE
	unsigned int dmc_sec_ctrl_value;
	unsigned int ddr_key;
#endif

	ret = timing_set->init_pctl(timing_set);

	if(ret)
	{
#ifdef CFG_DDR_AUTO_DETECT
		serial_puts("PUB init fail!\n");
		return -1;
#else
		serial_puts("\nPUB init fail! Reset...\n");
		__udelay(10000);
		AML_WATCH_DOG_START();
#endif
	}

	//asm volatile("wfi");
	//while(init_dmc(timing_set);){}
	init_dmc(timing_set);

#ifdef DDR_SCRAMBE_ENABLE
	ddr_key = readl(P_RAND64_ADDR0);
	dmc_sec_ctrl_value = 0x80000000 | (1<<0);
	writel(dmc_sec_ctrl_value, DMC_SEC_CTRL);
	while( readl(DMC_SEC_CTRL) & 0x80000000 ) {}
	writel(ddr_key &0x0000ffff, DMC_SEC_KEY0);
	writel((ddr_key >>16)&0x0000ffff, DMC_SEC_KEY1);

#ifdef CONFIG_DUMP_DDR_INFO
	dmc_sec_ctrl_value = readl(DMC_SEC_CTRL);
	if(dmc_sec_ctrl_value & (1<<0)){
		serial_puts("ddr scramb EN\n");
	}
#endif
#endif

#ifdef CONFIG_DUMP_DDR_INFO
	int nPLL = readl(AM_DDR_PLL_CNTL);
	int nDDRCLK = 2*((24 / ((nPLL>>9)& 0x1F) ) * (nPLL & 0x1FF))/ (1<<((nPLL>>16) & 0x3));
	serial_puts("\nDDR clock is ");
	serial_put_dec(nDDRCLK);
	serial_puts("MHz with ");
#ifdef CONFIG_DDR_LOW_POWER
	serial_puts("Low Power & ");
#endif
	if(readl(P_DDR0_PCTL_MCFG)& (1<<3)) //DDR0, DDR1 same setting?
		serial_puts("2T mode\n");
	
	else
		serial_puts("1T mode\n");
#endif
	
	return 0;
}
