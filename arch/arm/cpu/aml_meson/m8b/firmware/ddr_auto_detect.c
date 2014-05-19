#ifdef CFG_DDR_MODE_AUTO_DETECT
#include "efuse_basic.c"
unsigned ddr_init(int arg);

int read_ddr_mode(void){
	return ((efuse_read_byte(CFG_DDR_MODE_STO_ADDR) >> CFG_DDR_MODE_STO_OFFSET) & 0x3);
}

int write_ddr_mode(void){
	serial_puts("write ddr mode\n");
	//efuse_write_byte(CFG_DDR_MODE_STO_ADDR, ((cfg_ddr_mode & 0x3) << CFG_DDR_MODE_STO_OFFSET));
}

int print_ddr_mode(void){
	switch(cfg_ddr_mode){
		case 0:
			serial_puts("Not Set\n"); break;
		case CFG_DDR_32BIT:
			serial_puts("32 bit mode.\n"); break;
		case CFG_DDR_16BIT_LANE02:
			serial_puts("16 bit mode lane0+2.\n"); break;
		case CFG_DDR_16BIT_LANE01:
			serial_puts("16 bit mode lane0+1.\n"); break;
	}
}

int ddr_auto_switch(int ddr_test){
	int ret = 0;
	serial_puts("Get ddr mode : ");
	cfg_ddr_mode = read_ddr_mode();
	print_ddr_mode();
	int tmp_mode = cfg_ddr_mode;
	if(cfg_ddr_mode != 0){ /*get preset ddr mode*/
		if(ddr_init(ddr_test) == 0){ /*ddr pass with preset mode*/
			serial_puts("DDR init test pass with : ");
			print_ddr_mode();
			return 0;
		}
	}
	int try_times = 0;
	for(try_times = 1; try_times <= 3; try_times++){
		cfg_ddr_mode = try_times;
		if(tmp_mode == cfg_ddr_mode) /*skip preset mode*/
			continue;
		ret = ddr_init(ddr_test);
		serial_puts("Try ddr mode : ");
		print_ddr_mode();
		if(ret)
			serial_puts("Failed\n\n");
		else{
			serial_puts("Pass\n\n");
			if(tmp_mode == 0) /*first init, store ddr mode*/
				write_ddr_mode();
			return 0;
		}
	}
	serial_puts("\nAll ddr mode test failed, reset...\n");
	__udelay(10000); 
	AML_WATCH_DOG_START();
}
#endif

