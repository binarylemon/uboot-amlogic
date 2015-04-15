
#if defined(CONFIG_AML_EXT_PGM)

void print_ddr_size(unsigned int size){}
void print_ddr_mode(void){}

#else
void print_ddr_size(unsigned int size)
{
	serial_puts("DDR size: ");
	unsigned int mem_size = size >> 20; //MB
	(mem_size) >= 1024 ? serial_put_dec(mem_size >> 10):serial_put_dec(mem_size);
	(mem_size) >= 1024 ? serial_puts("GB"):serial_puts("MB");
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
	serial_puts(" (auto)\n");
#else
	serial_puts("\n");
#endif
}

void print_ddr_mode(void){
	serial_puts("DDR mode: ");
	switch(cfg_ddr_mode){
		case CFG_DDR_NOT_SET:
			serial_puts("Not Set"); break;
		case CFG_DDR_32BIT:
			serial_puts("32 bit mode"); break;
		case CFG_DDR_16BIT_LANE02:
			serial_puts("16 bit mode lane0+2"); break;
		case CFG_DDR_16BIT_LANE01:
			serial_puts("16 bit mode lane0+1"); break;
	}
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
	serial_puts(" (auto)\n");
#else
	serial_puts("\n");
#endif
}
#endif

/*Following code is for DDR MODE AUTO DETECT*/
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
static inline unsigned ddr_init(struct ddr_set * timing_reg);

int ddr_mode_auto_detect(struct ddr_set * timing_reg){
	int ret = 0;
#ifdef CONFIG_DDR_MODE_AUTO_DETECT_SKIP_32BIT
	int try_times = 2;
#else
	int try_times = 1;
#endif
	for(; try_times <= 3; try_times++){
		cfg_ddr_mode = try_times;
		ret = ddr_init(timing_reg);
		if(!ret){
			print_ddr_mode();
			return 0;
		}
	}
	serial_puts("\nAll ddr mode test failed, reset...\n");
	__udelay(10000); 
	AML_WATCH_DOG_START();
}
#endif

#if defined(CONFIG_DDR_MODE_AUTO_DETECT) || defined(CONFIG_DDR_SIZE_AUTO_DETECT)
unsigned int set_dmc_row(unsigned int dmc, unsigned int channel0, unsigned int channel1){
	dmc = dmc & ( 0xfffff3f3 ); /*clear row size bit*/
	dmc = dmc | ((channel0 & 0x3) << 2) | ((channel1 & 0x3) << 10);
	return dmc;
}
#endif

/*Following code is for DDR SIZE AUTO DETECT*/
#define DDR_SIZE_AUTO_DETECT_DEBUG 0
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
#define DDR_SIZE_AUTO_DETECT_PATTERN	0xABCDEF00
#define MEMORY_ROW_BITS(mem_size, mode_16bit) ((mem_size >> 29) + (mode_16bit ? 14:13))
#define MEMORY_ROW_BITS_DMC_REG(mem_size, mode_16bit) \
	(MEMORY_ROW_BITS(mem_size, mode_16bit) > 15)?(0):(MEMORY_ROW_BITS(mem_size, mode_16bit) - 12)
#define MEMORY_ROW_BITS_DMC_REG_SIMP(row_bits) ((row_bits > 15)?(0):(row_bits- 12))
void ddr_size_auto_detect(struct ddr_set * timing_reg){
/*
memory size auto detect function support:
32bit max 2GB
16bit max 1GB
*/
	/*enable pctl clk*/
	writel(readl(P_DDR0_CLK_CTRL) | 0x1, P_DDR0_CLK_CTRL);
	__udelay(1000);
#if DDR_SIZE_AUTO_DETECT_DEBUG //debug info
	serial_puts("timing_reg->t_pub0_dtar: ");
	serial_put_hex(timing_reg->t_pub0_dtar, 32);
	serial_puts("\n");
	serial_puts("P_DDR0_PUB_DTAR0: ");
	serial_put_hex(readl(P_DDR0_PUB_DTAR0), 32);
	serial_puts("\n");
	serial_puts("timing_reg->t_mmc_ddr_ctrl: ");
	serial_put_hex(timing_reg->t_mmc_ddr_ctrl, 32);
	serial_puts("\n");
	serial_puts("P_DMC_DDR_CTRL: ");
	serial_put_hex(readl(P_DMC_DDR_CTRL), 32);
	serial_puts("\n");
	serial_puts("DT_ADDR: ");
	serial_put_hex(CONFIG_M8B_RANK0_DTAR_ADDR, 32);
	serial_puts("\n");
	serial_puts("M8BABY_GET_DT_ADDR: ");
	serial_put_hex(M8BABY_GET_DT_ADDR(readl(P_DDR0_PUB_DTAR0), readl(P_DMC_DDR_CTRL)), 32);
	serial_puts("\n");
#endif

	//row_size of 16bit and 32bit mode are different
	unsigned int mem_mode = 0;
	if (cfg_ddr_mode > CFG_DDR_32BIT)
		mem_mode = 1;

	/*set max row size, then use "ROW ADDRESS MASK" to detect memory size*/
	timing_reg->t_mmc_ddr_ctrl = set_dmc_row(timing_reg->t_mmc_ddr_ctrl, 0x0, 0x0);
	unsigned int dmc_reg_setting = timing_reg->t_mmc_ddr_ctrl;//readl(P_MMC_DDR_CTRL);
	writel(dmc_reg_setting, P_DMC_DDR_CTRL);

#if DDR_SIZE_AUTO_DETECT_DEBUG //debug info
	serial_puts("new P_DMC_DDR_CTRL: ");
	serial_put_hex(readl(P_DMC_DDR_CTRL), 32);
	serial_puts("\n");
#endif

	/*start detection*/
	unsigned int row_mask_bit_offset = 25; /*just start from a little row size*/
	unsigned int cur_mask_addr = 0;
	int loop = 0;
	for (loop = 0; ; loop++) {
		cur_mask_addr = (1 << (row_mask_bit_offset + loop));
		writel(0, PHYS_MEMORY_START);
		writel(DDR_SIZE_AUTO_DETECT_PATTERN, cur_mask_addr);
#if (DDR_SIZE_AUTO_DETECT_DEBUG)
		serial_puts("	write address: ");
		serial_put_hex(cur_mask_addr, 32);
		serial_puts("	with 0xABCDEF00\n	read(0): ");
		serial_put_hex(readl(0), 32);
		serial_puts("\n");
#endif
		asm volatile("DSB"); /*sync ddr data*/
		__udelay(1);
		if (readl(PHYS_MEMORY_START) == DDR_SIZE_AUTO_DETECT_PATTERN) {
#if (DDR_SIZE_AUTO_DETECT_DEBUG)
			serial_puts("	find match address(not ddr size): ");
			serial_put_hex(cur_mask_addr, 32);
			serial_puts("\n");
#endif
			break;
		}
		if (cur_mask_addr >= (0x20000000 >> mem_mode)) {
			/*32bit max 2GB, 16bit max 1GB*/
			break;
		}
	}

	cur_mask_addr = cur_mask_addr << 2; /*it's a fixed multiplier according to the phy*/
	print_ddr_size(cur_mask_addr);

	/*Get corresponding row_bits setting and write back to DMC reg*/
	unsigned int cur_row_bits = MEMORY_ROW_BITS((cur_mask_addr), mem_mode);
	unsigned int cur_row_bit_dmc = MEMORY_ROW_BITS_DMC_REG_SIMP(cur_row_bits);
	timing_reg->t_mmc_ddr_ctrl = ((timing_reg->t_mmc_ddr_ctrl & (~(3<<2))) | (cur_row_bit_dmc<<2) | ((cur_row_bit_dmc<<10)));
	writel(timing_reg->t_mmc_ddr_ctrl, P_DMC_DDR_CTRL);
	__udelay(10);

	timing_reg->t_pub0_dtar	= ((0x0 + (M8BABY_DDR_DTAR_DTCOL_GET(CONFIG_M8B_RANK0_DTAR_ADDR,CONFIG_DDR_COL_BITS,CONFIG_M8B_DDR_BANK_SET,mem_mode)))|\
		((M8BABY_DDR_DTAR_DTROW_GET(CONFIG_M8B_RANK0_DTAR_ADDR,cur_row_bits,CONFIG_DDR_COL_BITS,CONFIG_M8B_DDR_BANK_SET,mem_mode)) << 12)|\
		((M8BABY_DDR_DTAR_BANK_GET(CONFIG_M8B_RANK0_DTAR_ADDR,cur_row_bits,CONFIG_DDR_COL_BITS,CONFIG_M8B_DDR_BANK_SET,mem_mode)) << 28));
	writel((0x0  + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR0);
	writel((0x08 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR1);
	writel((0x10 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR2);
	writel((0x18 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR3);
	__udelay(10);
#if DDR_SIZE_AUTO_DETECT_DEBUG	//debug info
	serial_puts("cur_row_bits: ");
	serial_put_hex(cur_row_bits, 32);
	serial_puts("\n");
	serial_puts("cur_row_bit_dmc: ");
	serial_put_hex(cur_row_bit_dmc, 32);
	serial_puts("\n");
	serial_puts("P_DMC_DDR_CTRL: ");
	serial_put_hex(readl(P_DMC_DDR_CTRL), 32);
	serial_puts("\n");
	serial_puts("timing_reg->t_mmc_ddr_ctrl: ");
	serial_put_hex(timing_reg->t_mmc_ddr_ctrl, 32);
	serial_puts("\n");
	serial_puts("timing_reg->t_pub0_dtar: ");
	serial_put_hex(timing_reg->t_pub0_dtar, 32);
	serial_puts("\n");
	serial_puts("M8BABY_GET_DT_ADDR: ");
	serial_put_hex(M8BABY_GET_DT_ADDR((readl(P_DDR0_PUB_DTAR0)), (readl(P_DMC_DDR_CTRL))), 32);
	serial_puts("\n");
#endif

	timing_reg->phy_memory_size = (cur_mask_addr);

	/*disable pctl clk*/
	writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);
}
#endif
