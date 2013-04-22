#include <asm/arch/register.h>
#include <asm/arch/reg_addr.h>
//#include "ethernet.h" 
#include <asm/arch/aml_eth_reg.h>
#include <common.h>
#include <command.h>

#define  SMI_ADDR_TSTCNTL     20
#define  SMI_ADDR_TSTREAD1    21
#define  SMI_ADDR_TSTREAD2    22
#define  SMI_ADDR_TSTWRITE    23

#define  WR_ADDR_A1CFG        0x12
#define  WR_ADDR_A2CFG        0x13
#define  WR_ADDR_A3CFG        0x14
#define  WR_ADDR_A4CFG        0x15
#define  WR_ADDR_A5CFG        0x16
#define  WR_ADDR_A6CFG        0x17
#define  WR_ADDR_A7CFG        0x18
#define  WR_ADDR_A8CFG        0x1a
#define  WR_ADDR_A9CFG        0x1b
#define  WR_ADDR_A10CFG       0x1c
#define  WR_ADDR_A11CFG       0x1d

#define  RD_ADDR_A3CFG        (0x14 << 5)
#define  RD_ADDR_A4CFG        (0x15 << 5)
#define  RD_ADDR_A5CFG        (0x16 << 5)
#define  RD_ADDR_A6CFG        (0x17 << 5)

#define  TSTCNTL_RD           ((1 << 15) | (1 << 10))
#define  TSTCNTL_WR           ((1 << 14) | (1 << 10))
#define  PA						8
#define  PRINT_FLAG				1

void writeSMIRegister(int pa, int gr, int data, int flag);
int readSMIRegister(int pa, int gr, int expected, int flag);

// -----------------------------------------------
// The following is used establish a mechanism
// for the ARC to communicate with stimulus.v
// You can add defines as needed (to here and
// in stimulus.v)
// -----------------------------------------------

#define TMP_TIMERE   (volatile unsigned long *)0xc1109954
void delay_us(int  us )
{
    (*TMP_TIMERE) = 0;
    while( (*TMP_TIMERE) < us ) {}
}

void mdio_init_regs(void)
{
	printf("Init regs!\n");
	writel(0x128488bf,0xc11040f0);	//0x103c
	printf("0x128488bf->0xc11040f0(0x103c)\n");
	writel(0x1ed396e1,0xc11040f4);	//0x103d
	printf("0x1ed396e1->0xc11040f4(0x103d)\n");
	writel(0x4500187d,0xc11040f8);	//0x103e
	printf("0x4500187d->0xc11040f8(0x103e)\n");
	writel(0x3dea4000,0xc11040fc);	//0x103f
	printf("0x3dea4000->0xc11040fc(0x103f)\n");
	writel(0xf,0xc1104100);			//0x1040
	printf("0x0000000f->0xc1104100(0x1040)\n");
	writel(0,0xc1104104);			//0x1041
	printf("0x00000000->0xc1104104(0x1041)\n");
	writel(0x2a855008,0xc1104108);	//0x1042
	printf("0x2a855008->0xc1104108(0x1042)\n");
	run_command("mdio write 0 0 11 1",0);		//0x00
	run_command("mdio write 0 0x12 0 13",0);	//0x12
	run_command("mdio write 0x1f08 0x13 1 15",0);	//0x13
	run_command("mdio write 0 0x14 0 4",0);		//0x14
	run_command("mdio write 0x89 0x14 8 4",0);	//0x14
	run_command("mdio write 0x209 0x15 1 10",0);//0x15
	run_command("mdio write 0x3 0x15 12 3",0);	//0x15
	run_command("mdio write 0x111b 0x16 1 14",0);	//0x16
	run_command("mdio write 0x0 0x17 1 1",0);	//0x17
	run_command("mdio write 0x3 0x18 0 5",0);	//0x18
	run_command("mdio write 0x0 0x18 6 10",0);	//0x18
	run_command("mdio write 0x0 0x1a 5 5",0);	//0x1a
	run_command("mdio write 0x52 0x1b 1 8",0);	//0x1b
	run_command("mdio write 0x0 0x1b 13 2",0);	//0x1b
	run_command("mdio write 0x0 0x1c 3 6",0);	//0x1c
	run_command("mdio write 0x6d03 0x1d 1 15",0);	//0x1d
}

void mdio_test_mode(void)
{
	int pa = PA;
	//enter test mode
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, (1 << 10), 0);
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, 0, 0);
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, (1 << 10), 0);
   printf(">>> Enter test mode!\n");
}

void mdio_init_gpio(void)
{
	//printf(">>> Disabling GPIO connections to external PHY\n");
    writel(readl(P_PERIPHS_PIN_MUX_6)& ~((1 << 31) |
                               (1 << 30) |
                               (1 << 19) |
                               (1 << 17) |
                               (1 << 14) |
                               (1 << 13) |
                               (1 << 11) |
                               (1 << 8)  |
                               (1 << 7)  |
                               (1 << 6)  |
                               (1 << 5)),P_PERIPHS_PIN_MUX_6);
	printf(">>> GPIO initialized!\n");
}

void mdio_init_phy(void)
{
	//---------------------------------
	// configure clocks
	//---------------------------------
	//printf(">>> Configuring clocks for RMII operation\n");
//   (*P_HHI_ETH_CLK_CNTL) = 0x113;
   writel(0x113, P_HHI_ETH_CLK_CNTL);
   delay_us(5);

   printf(">>> Configuring 10Base-T full duplex\n");
   int pa = PA;

	// [26:24] - eth_phy_co_st_mode
    //           000 - 10Base-T Half Duplex, auto neg disabled
    //           001 - 10Base-T Full Duplex, auto neg disabled
    //           010 - 100Base-TX Half Duplex, auto neg disabled
    //           011 - 100Base-TX Full Duplex, auto neg disabled
    //           100 - 100Base-TX Half Duplex, auto neg enabled
    //           101 - Repeater mode, auto neg enabled
    //           110 - Power Down Mode
    //           111 - All capable, auto neg enabled, automdix enabled
   writel(((pa << 27) |
           (1 << 24) | // 10BT full duplex
           (1 << 14) | // bypass, else wait ~2.6 ms
           (1 << 13) |
           (1 << 12) |
           (1 << 1)), P_PREG_ETHERNET_ADDR0);
   printf(">>> Ethernet PHY init finished!\n");
}

void mdio_init_reset(void)
{
   printf("Resetting TranSwitch PHY to latch OUI values\n");
//   (*P_RESET1_REGISTER) &= 0xf7ff;
   writel((readl(P_RESET1_REGISTER)&0xf7ff), P_RESET1_REGISTER);
   delay_us(5);
//   (*P_RESET1_REGISTER) |= 0x0800;
   writel((readl(P_RESET1_REGISTER)|0x0800), P_RESET1_REGISTER);
   delay_us(5);
}

void mdio_init(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if(2 == argc){
		mdio_init_reset();
		mdio_init_gpio();
		mdio_init_phy();
		mdio_test_mode();
		mdio_init_regs();
	}
	else{
		if(strncmp(argv[2], "gpio", 4) == 0)
			mdio_init_gpio();
		else if(strncmp(argv[2], "phy", 3) == 0)
			mdio_init_phy();
		else if(strncmp(argv[2], "testmode", 8) == 0)
			mdio_test_mode();
		else if(strncmp(argv[2], "reset", 8) == 0)
			mdio_init_reset();
		else if(strncmp(argv[2], "regs", 4) == 0)
			mdio_init_regs();
		else{
			printf("%s arg error\n", argv[2]);
			cmd_usage(cmdtp);
		}
	}
}

void mdio_test(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	printf("\\\ Test start!\n");
//	mdio_init();
	int pa = PA; // PA: Physical Layer Address (select 1 of 32 possible PHYs)
	int reg2_oui_in;
	int reg3_oui_in;
#if 1
   //---------------------------------
   // SMI Testing
   //---------------------------------
   printf("Creating random values for reg2_oui_in and reg3_oui_in\n");
   reg2_oui_in = 0x1234 & 0xffff;
   reg3_oui_in = 0x8765 & 0xffff;
//   (*P_PREG_ETHERNET_ADDR1) = (reg3_oui_in << 16) | reg2_oui_in;
   writel(((reg3_oui_in << 16) | reg2_oui_in),P_PREG_ETHERNET_ADDR1);
#endif


   // assert reset to latch OUI values into TranSwitch PHY
   printf("Resetting TranSwitch PHY to latch OUI values\n");
//   (*P_RESET1_REGISTER) &= 0xf7ff;
   writel((readl(P_RESET1_REGISTER)&0xf7ff), P_RESET1_REGISTER);
   delay_us(5);
//   (*P_RESET1_REGISTER) |= 0x0800;
   writel((readl(P_RESET1_REGISTER)|0x0800), P_RESET1_REGISTER);
   delay_us(5);

   printf("Begin SMI read test\n");
   writeSMIRegister(pa, SMI_ADDR_TSTWRITE, 0x0123, 0);
   readSMIRegister(pa, 0, 0x0100, 0);
   readSMIRegister(pa, 1, 0x7809, 0);
   readSMIRegister(pa, 16, 0x0040, 0);
   readSMIRegister(pa, 2, reg2_oui_in, 0);
   readSMIRegister(pa, 3, reg3_oui_in, 0);
   readSMIRegister(pa, SMI_ADDR_TSTWRITE, 0x0123, 0);

   // enable the testability mechanism of the TranSwitch PHY
   // NOTE: examining phyt100/src/regfile_m001.v tst_state logic shows
   //       the following sequence is required to stay in test mode
   printf("Enabling testability mechanism of the TranSwitch PHY\n");
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, (1 << 10), 0);
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, 0, 0);
   writeSMIRegister(pa, SMI_ADDR_TSTCNTL, (1 << 10), 0);

   printf("Begin A4CFG TSTCNTL test\n");
   int i;
   for (i=0; i<16; i++) {
      writeSMIRegister(pa, SMI_ADDR_TSTWRITE, (1 << i), 0);
      writeSMIRegister(pa, SMI_ADDR_TSTCNTL, (TSTCNTL_WR | WR_ADDR_A4CFG), 0);
   }

	printf("/// Test end!\n");
   // end test
}

//-------------------------------------
// SMI register access functions
//-------------------------------------
void writeSMIRegister(int pa, int gr, int data, int flag) {
   int tmp;
   int busy;

   if(1 != flag)
		printf(">>> Writing SMI Register on PA: 0x%02x, GR[0x%02x] = 0x%04x\n", pa, gr, data);
   
//   (*ETH_MAC_5_GMII_Data) = data;
//   (*ETH_MAC_4_GMII_Addr) = ((pa << 11) | (gr << 6) | (0x13));
   writel(data, ETH_MAC_5_GMII_Data);
   writel(((pa << 11) | (gr << 6) | (0x13)), ETH_MAC_4_GMII_Addr);
//   printf("ETH_MAC_4_GMII_Addr=%d",readl(ETH_MAC_4_GMII_Addr));
   busy = 1;
   while (busy) {
      tmp = readl(ETH_MAC_4_GMII_Addr);
      busy = tmp & 0x1;
   }
//      printf("ETH_MAC_4_GMII_Addr=%d",readl(ETH_MAC_4_GMII_Addr));
//   printf("writeSMIRegister called!\n");
}

int readSMIRegister(int pa, int gr, int expected, int flag) {
   int tmp;
   int busy;
   
//   (*ETH_MAC_4_GMII_Addr) = ((pa << 11) | (gr << 6) | (0x11));
   writel(((pa << 11) | (gr << 6) | (0x11)), ETH_MAC_4_GMII_Addr);
   busy = 1;
   while (busy) {
      tmp = readl(ETH_MAC_4_GMII_Addr);
      busy = tmp & 0x1;
   }
   
   tmp = readl(ETH_MAC_5_GMII_Data);
   if(1 != flag)
	   printf(">>> Reading SMI Register on PA: 0x%02x, GR[0x%02x] = 0x%04x\n", pa, gr, tmp);
   
   if (tmp != expected) {
	  if(1 != flag)
      	printf("--- ERROR: Register read mismatch (expected: 0x4x)\n", expected);
   }

	return tmp;
//   printf("readSMIRegister called!\n");
}

void mdio_read(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int pa = PA;
	if(3 <= argc){
		int addr = simple_strtoul(argv[2], NULL, 16) & 0xff;	//8bit

		int length;
		int reg;
		if(3 == argc){
			reg = 0;
			length = 16;
		}
		else{
			reg = simple_strtoul(argv[3], NULL, 10) & 0xf;		//0-15
			length = simple_strtoul(argv[4], NULL, 10);	//1-16
		}
		if(length > 16)
			length = 16;
		if(length < 0)
			length = 0;
		int expect = 0xffff;
		#if 0
		if(6 == argc)
			expect = simple_strtoul(argv[5], NULL, 16) & 0xffff;	//16bit
		else
			printf("Not define expect value\n");
		#endif
		if(16 < (reg+length)){
			printf("Warning! Read range over 15:0\n");
			return;
		}
		
		int read_val = readSMIRegister(pa, addr, expect, PRINT_FLAG);
		read_val = read_val >> reg;
		int i,read_val2 = 0;
		for(i = 0; i<length; i++){
			read_val2 = read_val2 + (((read_val >> i)&0x1) << i);
		}
		printf("Read PA:0x%02x-GR[0x%02x]-[%d:%d] : 0x%x\n",pa,addr,(reg+length-1),reg,read_val2);
	}
	else{
		cmd_usage(cmdtp);
	}
}

void mdio_write(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int pa = PA;
	if(4 <= argc){
		int value = simple_strtoul(argv[2], NULL, 16) & 0xffff;
		int addr = simple_strtoul(argv[3], NULL, 16) & 0xff;	//8bit
		int reg, length;
		if(4 == argc){
			reg = 0;
			length = 16;
		}
		else{
			reg = simple_strtoul(argv[4], NULL, 10) & 0xf;		//0-15
			length = simple_strtoul(argv[5], NULL, 10);	//1-16
		}
		if(length > 16)
			length = 16;
		if(length < 0)
			length = 0;

		if(16 < (reg+length)){
			printf("Warning! Write range over 15:0\n");
			return;
		}
		int expect = 0xffff;
		int read_val = readSMIRegister(pa, addr, expect, PRINT_FLAG);
		int i,write_val = 0;
		for(i = 0; i<length; i++){
			read_val = (read_val & (~(1<<(reg+i)))) | (((value>>i)&0x1)<<(reg+i));
		}
		write_val = read_val;
		writeSMIRegister(pa, addr, write_val, 1);
		printf("Write PA:0x%02x-GR[0x%02x]-[%d:%d] : 0x%x\n",pa,addr,(reg+length-1),reg,value);
	}
	else{
		cmd_usage(cmdtp);
	}
}

int do_mdio(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if(strncmp(argv[1], "read", 4) == 0)
		mdio_read(cmdtp, flag, argc, argv);
	else if(strncmp(argv[1], "write", 5) == 0)
		mdio_write(cmdtp, flag, argc, argv);
	else if(strncmp(argv[1], "init", 4) == 0)
		mdio_init(cmdtp, flag, argc, argv);
	else if(strncmp(argv[1], "test", 4) == 0)
		mdio_test(cmdtp, flag, argc, argv);
	else{
		cmd_usage(cmdtp);
		return -1;
	}
	return 0;
}

U_BOOT_CMD(
	mdio,	CONFIG_SYS_MAXARGS,	1,	do_mdio,
	"mdio",
	"	- m6tvlite ethernet regs related function\n"
	"	mdio init [reset/gpio/phy/testmode/regs]\n"
	"		-- init all or separately\n"
	"	mdio read ADDR{hex} [REG]{dec} [length]{dec}\n"
	"		-- read [length/16] bits from TSTCNTL[ADDR,REG(lowest)/0]\n"
	"	mdio write value{hex} ADDR{hex} [REG]{dec} [length]{dec}\n"
	"		-- write [length] bits [value] to TSTCNTL[ADDR,REG(lowest)]\n"
	"	mdio test\n"
	"		-- test ethernet phy\n"
);
