/*
static int ddr_phy_data_traning(void)
{   
    int  data_temp;
    clrbits_le32(UPCTL_DTUAWDT_ADDR,3<<9);
        APB_Wr(UPCTL_DTUWD0_ADDR, 0xdd22ee11);
        APB_Wr(UPCTL_DTUWD1_ADDR, 0x7788bb44);
        APB_Wr(UPCTL_DTUWD2_ADDR, 0xdd22ee11);
        APB_Wr(UPCTL_DTUWD3_ADDR, 0x7788bb44);
        APB_Wr(UPCTL_DTUWACTL_ADDR, 0x300 |    // col addr
                                   (0x7<<10) |  //bank addr
                                   (0x1fff <<13) |  // row addr
                                   (0 <<30 ));    // rank addr
        APB_Wr(UPCTL_DTURACTL_ADDR, 0x300 |    // col addr
                                   (0x7<<10) |  //bank addr
                                   (0x1fff <<13) |  // row addr
                                   (0 <<30 ));    // rank addr
 
       // hardware build in  data training.
       APB_Wr(PCTL_PHYCR_ADDR, APB_Rd(PCTL_PHYCR_ADDR) | (1<<31));
       APB_Wr(UPCTL_SCTL_ADDR, 1); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4         rddata = 1;        // init: 0, cfg: 1, cfg_req: 2, access: 3, access_req: 4, low_power: 5, low_power_enter_req: 6, low_power_exit_req: 7
       while ((APB_Rd(UPCTL_STAT_ADDR) & 0x7 ) != 1 ) {}
       APB_Wr(UPCTL_SCTL_ADDR, 2); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4
       data_temp = APB_Rd(PCTL_PHYCR_ADDR);
       while (data_temp & 0x80000000 ) {
          data_temp = APB_Rd(PCTL_PHYCR_ADDR);
       }  // waiting the data trainning finish.  
       data_temp = APB_Rd(PCTL_PHYSR_ADDR);
       //printf("PCTL_PHYSR_ADDR=%08x\n",data_temp& 0x00340000);
       serial_puts("PHY trainning Result=");
       serial_put_dword(data_temp);
       if ( data_temp & 0x00340000 ) {       // failed.
           return (1); 
       } else {
           return (0);                      //passed.
       }
}*/

void init_dmc(struct ddr_set * ddr_setting)
{
	//APB_Wr(MMC_DDR_CTRL, ddr_setting->ddr_ctrl); //hisun 2012.02.08
	MMC_Wr(MMC_DDR_CTRL, 0xff236a); //MMC_DDR_CTRL: c8006000 hisun 2010.02.10
	sec_mmc_wr(DMC_SEC_PORT0_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT1_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT2_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT3_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT4_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT5_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT6_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_PORT7_RANGE0, 0xffff);
    sec_mmc_wr(DMC_SEC_CTRL,         0x80000000);
	
	//APB_Wr(MMC_REQ_CTRL,0xff); //hisun 2012.02.08
	MMC_Wr(MMC_REQ_CTRL,0xff);   //hisun 2012.02.08
}
int ddr_init_hw(struct ddr_set * timing_reg)
{
    if(timing_reg->init_pctl(timing_reg))
        return 1;
    //if(ddr_phy_data_traning())
    //    return 1;
    init_dmc(timing_reg);
    return 0;
}
