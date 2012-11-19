#define reg(a) (*(volatile unsigned*)(a))

static inline int dtu_enable(void)
{
    unsigned timeout = 10000;
    reg(P_UPCTL_DTUECTL_ADDR) = 0x1;  // start wr/rd
    while((reg(P_UPCTL_DTUECTL_ADDR)&1) && timeout)
        --timeout;
    return timeout;
}

static inline int start_ddr_config(void)
{
    unsigned timeout = -1;
    reg(P_UPCTL_SCTL_ADDR) = 0x1;
    while((reg(P_UPCTL_STAT_ADDR) != 0x1) && timeout)
        --timeout;

    return timeout;
}

static inline int end_ddr_config(void)
{
    unsigned timeout = 10000;
    reg(P_UPCTL_SCTL_ADDR) = 0x2;
    while((readl(P_UPCTL_STAT_ADDR) != 0x3) && timeout)
        --timeout;

    return timeout;
}

static void dtu_test_for_debug_training_result(struct ddr_set * timing_reg)
{
    int i;
    
    reg(P_UPCTL_DTUWD0_ADDR) = 0xdd22ee11;
    reg(P_UPCTL_DTUWD1_ADDR) = 0x7788bb44;
    reg(P_UPCTL_DTUWD2_ADDR) = 0xdd22ee11;
    reg(P_UPCTL_DTUWD3_ADDR) = 0x7788bb44;
    reg(P_UPCTL_DTUWACTL_ADDR) = 0;
    reg(P_UPCTL_DTURACTL_ADDR) = 0;
    for(i = 0; i < ((timing_reg->ddr_ctrl&(1<<7))?0x2:0x4); i++)
    {
        serial_puts("\n\n");
        serial_put_hex(i, 8);
        serial_puts(" byte lane:\n");
        start_ddr_config();
        reg(P_UPCTL_DTUCFG_ADDR) = (i << 10) | 1;
        end_ddr_config();

        dtu_enable();
        serial_puts("\n");
        serial_put_hex(reg(P_UPCTL_DTURD0_ADDR), 32);
        serial_puts("\n");
        serial_put_hex(reg(P_UPCTL_DTURD1_ADDR), 32);
        serial_puts("\n");
        serial_put_hex(reg(P_UPCTL_DTURD2_ADDR), 32);
        serial_puts("\n");
        serial_put_hex(reg(P_UPCTL_DTURD3_ADDR), 32);
        serial_puts("\n");
        serial_put_hex(reg(P_UPCTL_DTUPDES_ADDR), 32);

    }
}

static void display_training_result(struct ddr_set * timing_reg)
{
    serial_puts("\nDX0DLLCR:");
    serial_put_hex(reg(P_PUB_DX0DLLCR_ADDR), 32);
    serial_puts("\nDX0DQTR:");
    serial_put_hex(reg(P_PUB_DX0DQTR_ADDR), 32);
    serial_puts("\nDX0DQSTR:");
    serial_put_hex(reg(P_PUB_DX0DQSTR_ADDR), 32);
    serial_puts("\nDX1DLLCR:");
    serial_put_hex(reg(P_PUB_DX1DLLCR_ADDR), 32);
    serial_puts("\nDX1DQTR:");
    serial_put_hex(reg(P_PUB_DX1DQTR_ADDR), 32);
    serial_puts("\nDX1DQSTR:");
    serial_put_hex(reg(P_PUB_DX1DQSTR_ADDR), 32);
    if(!(timing_reg->ddr_ctrl&(1<<7))){
        serial_puts("\nDX2DLLCR:");
        serial_put_hex(reg(P_PUB_DX2DLLCR_ADDR), 32);
        serial_puts("\nDX2DQTR:");
        serial_put_hex(reg(P_PUB_DX2DQTR_ADDR), 32);
        serial_puts("\nDX2DQSTR:");
        serial_put_hex(reg(P_PUB_DX2DQSTR_ADDR), 32);
        serial_puts("\nDX3DLLCR:");
        serial_put_hex(reg(P_PUB_DX3DLLCR_ADDR), 32);
        serial_puts("\nDX3DQTR:");
        serial_put_hex(reg(P_PUB_DX3DQTR_ADDR), 32);
        serial_puts("\nDX3DQSTR:");
        serial_put_hex(reg(P_PUB_DX3DQSTR_ADDR), 32);
    }
}

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
	while( readl(P_DMC_SEC_CTRL) & 0x800000000 ) {}


#ifdef CONFIG_DDR_LOW_POWER	
	writel( (0x4c )|
	(0xf5 << 8) |
	(0xf5 <<16) |
	( 0 << 30)  |
	( 0 << 31), P_MMC_LP_CTRL4 );   //tREFI        
	writel( (0xf  <<  0) | //t_PWD_WAIT  wait for Power down pipeline finish, 
	(0xf  <<  8) | //t_SELF_WAIT.  wait for self refresh pipeline finish.               
	(0x2f << 16) | // APB command hold time.                
	(0x28 << 24)   //t100ns                
	,P_MMC_LP_CTRL3 );	
	writel( (0x20 << 0 )|  //tRFC, disable ddr command after auto refresh command issued.                 
	(0x100 << 8 ) |  //idle cycle numbers for MMC enter self refresh mode                
	(1 << 28 )    |  //enable MMC auto power down mode.               
	(1 << 29 )    |  //enable hardwire wakeup enable. use c_active_in for ddr wakeup.                
	(1 << 30 )       //enable MMC auto self refresh mode.                 
	,P_MMC_LP_CTRL1 );
#endif


	writel(0xff, P_MMC_REQ_CTRL);

	//re read write DDR SDRAM several times to make sure the AXI2DDR bugs dispear.
	//refer from arch\arm\cpu\aml_meson\m6\firmware\kreboot.s	
	int nCnt,nMax,nVal;
	for(nCnt=0,nMax= 9;nCnt<nMax;++nCnt)
	{
		//asm volatile ("LDR  r0, =0x55555555");
		//asm volatile ("LDR  r1, =0x9fffff00");
		//asm volatile ("STR  r0, [r1]");
		writel(0x55555555, 0x9fffff00);
	}		
	for(nCnt=0,nMax= 12;nCnt<nMax;++nCnt)
	{
		//asm volatile ("LDR  r1, =0x9fffff00");
		//asm volatile ("LDR  r0, [r1]");		
		nVal = readl(0x9fffff00);
	}	
	asm volatile ("dmb");
	asm volatile ("isb");	
	//

}
int ddr_init_hw(struct ddr_set * timing_reg)
{
    int ret = 0;
    
    ret = timing_reg->init_pctl(timing_reg);
    if(ret){
        dtu_test_for_debug_training_result(timing_reg);
        __udelay(10);        
		serial_puts("\nPUB init fail! Reset...\n");
		__udelay(10000); 
		//writel((1<<22) | (3<<24), P_WATCHDOG_TC);
		//while(1);		
        return ret;
    }
    
    display_training_result(timing_reg);
    
    init_dmc(timing_reg);

    return 0;
}
