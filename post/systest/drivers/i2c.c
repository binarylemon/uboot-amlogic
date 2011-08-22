#include <aml_i2c.h>
#include <common.h>


#ifdef CONFIG_AML_I2C

extern char systest_info_line[];
extern struct aml_i2c_device aml_i2c_devices;

//=============================================================================
static int i2c_xfer(unsigned int addr)
{
    unsigned char cmd = 0x0;
	
	struct i2c_msg msgs[] = {
	    {
	        .addr = addr,
	        .flags = 0,
	        .len = 1,
	        .buf = &cmd,
	    }
	};

	if(aml_i2c_xfer(msgs, 1) < 0) 
		return -1;

	return 0;
}
#endif

//=============================================================================
static int i2c_sub_test(struct i2c_board_info *ptests,  int num)
{
	int i, ret;
	struct i2c_board_info *ptest = NULL;
	
	if(!ptests){
		sprintf(systest_info_line, "%s:%d: I2C: test fail: no test case found. \n", __FILE__, __LINE__);
		systest_log(systest_info_line, SYSTEST_INFO_L2);	
		return -1;
	}
	
	ret = 0;	
	for(i=0; i<num; i++){	
		ptest = ptests+i;
				
		if(ptest->device_init)
			ptest->device_init();
				
		if(i2c_xfer(ptest->addr) < 0){		
			sprintf(systest_info_line, "%s:%d: I2C[board: %s addr: 0x%x]: test fail. \n", __FILE__, __LINE__, ptest->type, ptest->addr);
			systest_log(systest_info_line, SYSTEST_INFO_L2);	
			ret = -1;
		}
		else{			
			sprintf(systest_info_line, "%s:%d: I2C[board: %s addr: 0x%x]: test pass. \n", __FILE__, __LINE__, ptest->type, ptest->addr);
			systest_log(systest_info_line, SYSTEST_INFO_L2);	
		}
		
		if(ptest->device_uninit)
			ptest->device_uninit();					
	}
	
	return ret;	
}

//=============================================================================
// systest run i2c [addr]
int i2c_systest(int argc, char* argv[])
{
	struct i2c_board_info board;	
	int ret;
		
	switch(argc)
	{
		case 0:
		case 1:							
				ret = i2c_sub_test(aml_i2c_devices.aml_i2c_boards,aml_i2c_devices.dev_num)	;
				break;
		case 2:				
			if(test_isnum(argv[1] >= 0)){				
				board.addr = simple_strtoul(argv[1], NULL, 16);				
				strcpy(board.type, "i2cboard");				
				board.device_init = NULL;
				board.device_uninit = NULL;
				ret = i2c_sub_test(&board, 1);				
			}
			else{
				ret = -1;
				sprintf(systest_info_line, "%s:%d: I2C address %s is error.\n", __FUNCTION__, __LINE__, argv[1]);
				systest_log(systest_info_line, SYSTEST_INFO_L2);
			}
			break;			
			
		default:
			sprintf(systest_info_line, "%s:%d: I2C test fail: %s\n", __FILE__, __LINE__, "parameters too much.\n");
			systest_log(systest_info_line, SYSTEST_INFO_L1);			
			return 0;
	}
	
	if(ret < 0)
		systest_log("i2c: test fail. \n", SYSTEST_INFO_L1);
	else
		systest_log("i2c: test pass. \n", SYSTEST_INFO_L1);			
	
	return 0;
}

//=============================================================================


