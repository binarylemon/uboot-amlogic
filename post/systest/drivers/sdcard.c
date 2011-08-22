#include <common.h>
#include <mmc.h>

extern char systest_info_line[];
//=======================================================================
int sdcard_sub_test()
{
	struct mmc *mmc;
	int dev_num;

	dev_num = simple_strtoul("mmcinfo", NULL, 0);
	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);		
	    	if((mmc->tran_speed == 0) || (mmc->read_bl_len == 0) || (mmc->capacity == 0)) {
			sprintf(systest_info_line, "%s:%d: SDCARD: %s\n", __FILE__, __LINE__, "no MMC device available.");
			systest_log(systest_info_line, SYSTEST_INFO_L2);
			return -1;
		}		
		
		sprintf(systest_info_line, "SDCARD: Device: %s\n", mmc->name);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		
		sprintf(systest_info_line, "SDCARD: Manufacturer ID: %x\n", mmc->cid[0] >> 24);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		
		sprintf(systest_info_line, "SDCARD: OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
	
		//sprintf(systest_info_line, "SDCARD: Name: %c%c%c%c%c\n", mmc->cid[0] & 0xff,
		//			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		//			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
		//systest_log(systest_info_line, SYSTEST_INFO_L2);
		
		sprintf(systest_info_line, "SDCARD: Tran Speed: %d\n", mmc->tran_speed);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		
		sprintf(systest_info_line, "SDCARD: Rd Block Len: %d\n", mmc->read_bl_len);
		systest_log(systest_info_line, SYSTEST_INFO_L2);

		sprintf(systest_info_line, "SDCARD: %s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
				(mmc->version >> 4) & 0xf, mmc->version & 0xf);
		systest_log(systest_info_line, SYSTEST_INFO_L2);

		sprintf(systest_info_line, "SDCARD: High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
		systest_log(systest_info_line, SYSTEST_INFO_L2);

		sprintf(systest_info_line, "SDCARD: Capacity: %lld\n", mmc->capacity);
		systest_log(systest_info_line, SYSTEST_INFO_L2);

		sprintf(systest_info_line, "SDCARD: Bus Width: %d-bit\n", mmc->bus_width);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
	}
	else {		
		sprintf(systest_info_line, "%s:%d: SDCARD: %s\n", __FILE__, __LINE__, "no MMC device available.");
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return -1;
	}
	return 1;

}

//=======================================================================
int sdcard_systest(int argc, char *argv[])
{
	if(argc < 2){
		if(sdcard_sub_test() < 0)
			systest_log("sdcard: test fail. \n", SYSTEST_INFO_L1);
		else
			systest_log("sdcard: test pass. \n", SYSTEST_INFO_L1);
		return 0;
		
	}
	else{
		systest_log("sdcard: test fail: parameter number error. \n", SYSTEST_INFO_L1);
		return 0;
	}
	
}
	
