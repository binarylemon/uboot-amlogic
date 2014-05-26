/*Following code is for DDR MODE AUTO DETECT*/
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
#include "efuse_basic.c"
static inline unsigned ddr_init(struct ddr_set * timing_reg);

int read_ddr_mode(void){
	return ((efuse_read_byte(CFG_DDR_MODE_STO_ADDR) >> CFG_DDR_MODE_STO_OFFSET) & 0x3);
}

int write_ddr_mode(void){
	//serial_puts("write ddr mode\n");
	//efuse_write_byte(CFG_DDR_MODE_STO_ADDR, ((cfg_ddr_mode & 0x3) << CFG_DDR_MODE_STO_OFFSET));
	return 0;
}

int print_ddr_mode(void){
	switch(cfg_ddr_mode){
		case CFG_DDR_NOT_SET:
			serial_puts("Not Set. "); break;
		case CFG_DDR_32BIT:
			serial_puts("32 bit mode. "); break;
		case CFG_DDR_16BIT_LANE02:
			serial_puts("16 bit mode lane0+2. "); break;
		case CFG_DDR_16BIT_LANE01:
			serial_puts("16 bit mode lane0+1. "); break;
	}
}

int ddr_mode_auto_detect(struct ddr_set * timing_reg){
	int ret = 0;
	int tmp_mode = read_ddr_mode();
	int try_times = 0;
	for(try_times = 1; try_times <= 3; try_times++){
		cfg_ddr_mode = try_times;
		//if(tmp_mode == cfg_ddr_mode) /*skip preset mode*/
			//continue;
		serial_puts("Try ddr mode : ");
		print_ddr_mode();
		ret = ddr_init(timing_reg);
		if(!ret){
			serial_puts("Pass!!\n");
			if(tmp_mode == CFG_DDR_NOT_SET) /*first init, store ddr mode*/
				write_ddr_mode();
			return 0;
		}
	}
	serial_puts("\nAll ddr mode test failed, reset...\n");
	__udelay(10000); 
	AML_WATCH_DOG_START();
}
#endif

