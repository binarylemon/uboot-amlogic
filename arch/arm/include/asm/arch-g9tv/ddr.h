/*All ddr defines*/

/*DDR capacity hex define*/
#define CFG_DDR_SIZE_512M			0x20000000
#define CFG_DDR_SIZE_1G				0x40000000
#define CFG_DDR_SIZE_1P5G			0x60000000
#define CFG_DDR_SIZE_2G				0x80000000
#define CFG_DDR_SIZE_3G				0xC0000000

//DDR channel setting
#define CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12			(0)
#define CFG_DDR_ONE_CHANNEL_DDR1_ONLY				(1)
#define CFG_DDR_ONE_CHANNEL_DDR0_ONLY				(3)
#define CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8			(5) //don't use this one
#define CFG_DDR_TWO_CHANNEL_DDR0_LOW				(6)
#define CFG_DDR_TWO_CHANNEL_DDR1_LOW				(7)

//DDR mode. Once considering this value stored in efuse, 0=NotSet is necessary.
#define CFG_DDR_BUS_WIDTH_NOT_SET					0
#define CFG_DDR_BUS_WIDTH_32BIT						1
//#define CFG_DDR_BUS_WIDTH_16BIT_LANE02			2	//DDR lane0+lane2 //m8m2(g9tv?) doesn't support
#define CFG_DDR_BUS_WIDTH_16BIT_LANE01				2 //3	//DDR lane0+lane1
//following 2 defines used for dtar calculate
#define CONFIG_DDR_BIT_MODE_32BIT					0
#define CONFIG_DDR_BIT_MODE_16BIT					1

//DDR bank setting
#define CFG_DDR_BANK_SET_2_BANKS					(0)
#define CFG_DDR_BANK_SET_4_BANKS					(1)
//#define CFG_DDR_BANK_SET_2_BANKS_SWITCH_BIT12		0x0
//#define CFG_DDR_BANK_SET_4_BANKS_SWITCH_BIT12_13	0x1
//#define CFG_DDR_BANK_SET_2_BANKS_SWITCH_BIT8		0x2
//#define CFG_DDR_BANK_SET_4_BANKS_SWITCH_BIT8_9	0x3

#define GET_DDR_ROW_BITS(mem_size, bus_width) (13 + (((unsigned int)mem_size)>>29) - ((((unsigned int)mem_size)>>29)==0x4?(1):(0)) + \
	((bus_width>=CFG_DDR_BUS_WIDTH_16BIT_LANE01)?(1):(0)))
#define GET_DDR_ROW_SIZE(row_bits) ((row_bits>=16)?(0):(row_bits-12))

/*CONFIG_DDR_ALIGN: DDR align define, 0x0: ddr1=ddr0, 0x1: ddr0>ddr1, 0x2: ddr0<ddr1*/
#if ((CONFIG_DDR_SIZE & 0x1FF) != 0)
	#error "Please define correct DDR capacity in board config file!\n"
#endif
#define PHYS_MEMORY_START							(0x00000000)
#define PHYS_MEMORY_SIZE							(CONFIG_DDR_SIZE << 20)
#if (CONFIG_DDR_SIZE == 1536) || (CONFIG_DDR_SIZE == 3072)
	#if (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_DDR0_LOW)
		#define CONFIG_DDR_ALIGN					0x1
	#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_DDR1_LOW)
		#define CONFIG_DDR_ALIGN					0x2
	#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_30)
		#if (CONFIG_DDR_SIZE == 1536)
			#define CONFIG_DDR_ALIGN				0x1
		#elif (CONFIG_DDR_SIZE == 3072)
			#define CONFIG_DDR_ALIGN				0x2
		#endif
	#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12)
		#define CONFIG_DDR_ALIGN				0x1
	#endif
#endif
#if (CONFIG_DDR_SIZE == 1536) || (CONFIG_DDR_SIZE == 3072)
	#define CONFIG_DDR_SIZE_0   ((CONFIG_DDR_SIZE+(((CONFIG_DDR_ALIGN&0x1)-(CONFIG_DDR_ALIGN&0x2))*((CONFIG_DDR_SIZE==1536)?0x200:0x400)))<<19)/*divide 2*/
	#define CONFIG_DDR_SIZE_1   ((CONFIG_DDR_SIZE+(((CONFIG_DDR_ALIGN&0x2)-(CONFIG_DDR_ALIGN&0x1))*((CONFIG_DDR_SIZE==1536)?0x200:0x400)))<<19)/*divide 2*/
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_ONE_CHANNEL_DDR1_ONLY)
	#define CONFIG_DDR_SIZE_0   (0x0)
	#define CONFIG_DDR_SIZE_1   (CONFIG_DDR_SIZE<<20)
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_ONE_CHANNEL_DDR0_ONLY)
	#define CONFIG_DDR_SIZE_0   (CONFIG_DDR_SIZE<<20)
	#define CONFIG_DDR_SIZE_1   (0x0)
#else
	#define CONFIG_DDR_SIZE_0   (CONFIG_DDR_SIZE<<19)/*divide 2*/
	#define CONFIG_DDR_SIZE_1   (CONFIG_DDR_SIZE<<19)/*divide 2*/
#endif

/*DON'T MODIFY FOLLOWING ROW/COL SETTINGS*/
//row size.  2'b01 : A0~A12.   2'b10 : A0~A13.  2'b11 : A0~A14.  2'b00 : A0~A15.
//col size.   2'b01 : A0~A8,      2'b10 : A0~A9
#define CONFIG_DDR0_COL_BITS     (10)
#define CONFIG_DDR0_COL_SIZE     (2)
#define CONFIG_DDR1_COL_BITS     (10)
#define CONFIG_DDR1_COL_SIZE     (2)
#define CONFIG_DDR0_ROW_BITS     GET_DDR_ROW_BITS(CONFIG_DDR_SIZE_0, CONFIG_DDR_MODE)
#define CONFIG_DDR0_ROW_SIZE     GET_DDR_ROW_SIZE(CONFIG_DDR0_ROW_BITS)
#define CONFIG_DDR1_ROW_BITS     GET_DDR_ROW_BITS(CONFIG_DDR_SIZE_1, CONFIG_DDR_MODE)
#define CONFIG_DDR1_ROW_SIZE     GET_DDR_ROW_SIZE(CONFIG_DDR1_ROW_BITS)

//check necessary defines
#if !defined(CONFIG_DDR_CHANNEL_SET)
	#define CONFIG_DDR_CHANNEL_SET CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12
#endif
#if !defined(CONFIG_DDR_BANK_SET)
	#define CONFIG_DDR_BANK_SET CFG_DDR_BANK_SET_4_BANKS
#endif
#if !defined(CONFIG_DDR_MODE)
	#define CONFIG_DDR_MODE CFG_DDR_BUS_WIDTH_32BIT
#endif

//DDR training address, DO NOT modify.
#if (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(1<<12)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x0) //for dmc_ctrl bit[27:26]
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0) //for dmc_ctrl bit24
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(1<<8)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x1)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_SWITCH_BIT_30)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
#if !defined(CONFIG_DDR_SIZE_AUTO_DETECT)
#if (CONFIG_DDR_SIZE < 1024)
#error "Channel switch bit30, doesn't support ddr capacity lower than 1GB.\n"
#endif
#endif
	#define DDR1_DTAR_ADDR_OFFSET					(0x40000000)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x1)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_DDR0_LOW)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(CONFIG_DDR_SIZE_0)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x2)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_TWO_CHANNEL_DDR1_LOW)
	#define DDR0_DTAR_ADDR_OFFSET					(CONFIG_DDR_SIZE_1)
	#define DDR1_DTAR_ADDR_OFFSET					(0-DDR0_DTAR_ADDR_OFFSET)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x2)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(1)
#elif (CONFIG_DDR_CHANNEL_SET == CFG_DDR_ONE_CHANNEL_DDR0_ONLY)
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(0)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x3)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(0)
#else	//CFG_DDR_ONE_CHANNEL_DDR1_ONLY
	#define DDR0_DTAR_ADDR_OFFSET					(0)
	#define DDR1_DTAR_ADDR_OFFSET					(0)
	#define CONFIG_DDR_CHANNEL_SWITCH				(0x3)
	#define CONFIG_DDR_CHANNEL_SUB_SWITCH			(1)
#endif
//128Bytes each channel. COL[2:0] must be b'000. In addition of 128Bytes, set DTAR ADDR[6:0]=b'00,0000 for safe.
//Please make sure DTAR ROW[10]!=1. Or please use MPR mode for pctl init. use 0x3fffef80 for test.
#define CONFIG_DDR_DTAR_ADDR_BASE 0x3000000
#define CONFIG_DDR0_DTAR_ADDR (CONFIG_DDR_DTAR_ADDR_BASE + DDR0_DTAR_ADDR_OFFSET)	/*don't modify*/
#define CONFIG_DDR1_DTAR_ADDR (CONFIG_DDR0_DTAR_ADDR + DDR1_DTAR_ADDR_OFFSET)

#if (CONFIG_DDR_MODE > CFG_DDR_BUS_WIDTH_32BIT)	//not 32bit mode
	#define CONFIG_DDR_BIT_MODE (CONFIG_DDR_BIT_MODE_16BIT)
#else
	#define CONFIG_DDR_BIT_MODE (CONFIG_DDR_BIT_MODE_32BIT)
#endif

#define DDR_32BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set) \
/*get bank[0] or bank[1:0]*/	((((addr)>>(2+(((bank_set>>1)&0x1)?(6):(10+((channel_set>0)?0:1)))))&((bank_set&0x1)?(0x3):(0x1))) | \
/*get bank[2] or bank[2:1]*/	(((addr>>(2+row_bits+col_bits+((bank_set&0x1)?(2):(1))+((channel_set>0)?0:1)))&((bank_set&0x1)?(0x1):(0x3)))<<((bank_set&0x1)?(2):(1))))
#define DDR_32BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set) \
/*get row[row_bits-1:0]*/	(((addr)>>(2+col_bits+((channel_set>0)?0:1)+((bank_set&0x1)?(2):(1)))&((1<<row_bits)-1)))
#define DDR_32BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set) \
					(((bank_set>>1)&0x1) ? \
/*get col[5:0]*/	((((addr)>>2)&(0x3f)) | \
						((channel_set>0) ? \
	/* | col[9:6]*/				((((addr)>>(2+6+((bank_set&0x1)?(2):(1))))&(0xf))<<6) : \
	/* | col[9:6]*/				((((addr)>>(2+6+((bank_set&0x1)?(2):(1)))&((bank_set&0x1)?0x3:0x7))<<6)|((addr)>>(2+6+5)&((bank_set&0x1)?0x3:0x1))<<((bank_set&0x1)?8:9)))) : \
/*get col[9:0]*/	(((addr)>>2)&(0x3ff)))

#define DDR_16BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set) 0
#define DDR_16BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set) 0
#define DDR_16BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set) 0

#define DDR_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_BANK_GET(addr,row_bits,col_bits,bank_set,channel_set)))
#define DDR_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_DTROW_GET(addr,row_bits,col_bits,bank_set,channel_set)))
#define DDR_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set,bit_mode) \
		((bit_mode)?(DDR_16BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set)):(DDR_32BIT_DTAR_DTCOL_GET(addr,col_bits,bank_set,channel_set)))

#define CONFIG_DDR0_DTAR_DTBANK  DDR_DTAR_BANK_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR0_ROW_BITS,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR0_DTAR_DTROW   DDR_DTAR_DTROW_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR0_ROW_BITS,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR0_DTAR_DTCOL   DDR_DTAR_DTCOL_GET(CONFIG_DDR0_DTAR_ADDR, \
	CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)

#define CONFIG_DDR1_DTAR_DTBANK  DDR_DTAR_BANK_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR1_ROW_BITS,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR1_DTAR_DTROW   DDR_DTAR_DTROW_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR1_ROW_BITS,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)
#define CONFIG_DDR1_DTAR_DTCOL   DDR_DTAR_DTCOL_GET(CONFIG_DDR1_DTAR_ADDR, \
	CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,CONFIG_DDR_CHANNEL_SWITCH,CONFIG_DDR_BIT_MODE)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*bank1*/
#define GET_32BIT_DT_ADDR_BANK1(dtar, dmc, channel) (((((dtar)>>28)&0x7)&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(0x3):(0x1)))<<((2)+((((((dmc)>>(channel?(13):(5)))&0x3)>>1)&0x1)?(6):(10+(((((dmc)>>26)&0x3)>0)?0:1)))))
/*bank2*/
#define GET_32BIT_DT_ADDR_BANK2(dtar, dmc, channel) ((((((dtar)>>28)&0x7)>>(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1)))&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(0x1):(0x3)))<<((2)+(((((dmc)>>(channel?(10):(2)))&0x3)>0)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16))+((((dmc)>>(channel?(8):(0)))&0x3)+8)+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))+(((((dmc)>>26)&0x3)>0)?0:1)))
/*row  */
#define GET_32BIT_DT_ADDR_ROW(dtar, dmc, channel) (((((dtar)>>12)&0xffff)&((1<<((((dmc)>>(channel?(10):(2)))&0x3)?((((dmc)>>(channel?(10):(2)))&0x3)+12):(16)))-1))<<((2)+((((dmc)>>(channel?(8):(0)))&0x3)+8)+(((((dmc)>>26)&0x3)>0)?0:1)+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))))
/*col  */
#define GET_32BIT_DT_ADDR_COL(dtar, dmc, channel)   ((((((dmc)>>(channel?(13):(5)))&0x3)>>1)&0x1)?(((((dtar)&0xfff)&0x3f)<<(2))|(((((dmc)>>26)&0x3)>0)?(((((dtar)&0xfff)>>6)&0xf)<<(2+6+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1)))):((((((dtar)&0xfff)>>6)&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?0x3:0x7))<<(2+6+(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?(2):(1))))|(((((dtar)&0xfff)>>(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?8:9))&(((((dmc)>>(channel?(13):(5)))&0x3)&0x1)?0x3:0x1))<<(2+6+5))))):((((dtar)&0xfff)&0x3ff)<<(2)))
/*ext_addr*/
#define GET_32BIT_DT_ADDR_EXT(dtar, dmc, channel) (((((dmc)>>26)&0x3)==0x2)?((((dmc)>>24)&0x1)?(channel?(0):((1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5)))):(channel?((1<<(((((dmc)>>(channel?(2):(10)))&0x3)?((((dmc)>>(channel?(2):(10)))&0x3)+12):(16))+((((dmc)>>(channel?(0):(8)))&0x3)+8)+5))):(0))):(((((dmc)>>26)&0x3)==0x0)?(channel?(1<<12):(0)):(((((dmc)>>26)&0x3)==0x1)?(1<<8):(0))))
#define GET_32BIT_DT_ADDR(dtar, dmc, channel) \
			((GET_32BIT_DT_ADDR_BANK1(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_BANK2(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_ROW(dtar, dmc, channel) | \
			GET_32BIT_DT_ADDR_COL(dtar, dmc, channel)) + \
			GET_32BIT_DT_ADDR_EXT(dtar, dmc, channel))

#define GET_16BIT_DT_ADDR_BANK1(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_BANK2(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_ROW(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_COL(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR_EXT(dtar, dmc, channel) 0
#define GET_16BIT_DT_ADDR(dtar, dmc, channel) \
			((GET_16BIT_DT_ADDR_BANK1(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_BANK2(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_ROW(dtar, dmc, channel) | \
			GET_16BIT_DT_ADDR_COL(dtar, dmc, channel)) + \
			GET_16BIT_DT_ADDR_EXT(dtar, dmc, channel))

#define GET_DT_ADDR(dtar, dmc, channel) \
			(((dmc>>(channel?15:7))&0x1)?(GET_16BIT_DT_ADDR(dtar, dmc, channel)):(GET_32BIT_DT_ADDR(dtar, dmc, channel)))

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//NOTE: IMPORTANT! DO NOT TRY TO MODIFY FOLLOWING CODE!
//           It is used to get DDR0/1 training address from PUB_DTAR0. The DDR training address is just the 
//           start address and it will occupy 128bytes for each channel.
//          
//How to fetch the DDR training address for M8:
//           1. enable PCTL clock before access
//           2. load MMC DDR setting from P_MMC_DDR_CTRL  (#define P_MMC_DDR_CTRL        0xc8006000 + (0x00 << 2 ) )
//           3. load the DTAR0 value from DDR0/DDR1 PUB register according to the channel setting from MMC_DDR_CTRL
//           4. disable PCTL clock for power saving
//
//Demo code: 
/*          
	//enable clock
	writel(readl(P_DDR0_CLK_CTRL)|(1), P_DDR0_CLK_CTRL);
	writel(readl(P_DDR1_CLK_CTRL)|(1), P_DDR1_CLK_CTRL);

	unsigned int nDDR0_BaseAddr,nDDR1_BaseAddr;
	unsigned int nMMC_DDR_SET  = readl(0xc8006000);
	unsigned int nPUB0_DTAR0   = readl(0xc80010b4);
	unsigned int nPUB1_DTAR0   = readl(0xc80030b4);
	serial_puts("\nAml log : nMMC_DDR_SET is 0x");
	serial_put_hex(nMMC_DDR_SET,32);
	serial_puts("\nAml log : nPUB0_DTAR0 is 0x");
	serial_put_hex(nPUB0_DTAR0,32);
	serial_puts("\nAml log : nPUB1_DTAR0 is 0x");
	serial_put_hex(nPUB1_DTAR0,32);
	
	switch( ((nMMC_DDR_SET >> 24) & 3))
	{
	case CONFIG_M8_DDR1_ONLY: 
		{ 
			nDDR1_BaseAddr = CFG_M8_GET_DDR1_TA_FROM_DTAR0(nPUB1_DTAR0,nMMC_DDR_SET);	
			serial_puts("\nAml log : DDR1 DTAR is 0x");
			serial_put_hex(nDDR1_BaseAddr,32);			
			serial_puts("\n");

		}break;
	default: 
		{
			nDDR0_BaseAddr = CFG_M8_GET_DDR0_TA_FROM_DTAR0(nPUB0_DTAR0,nMMC_DDR_SET);
			nDDR1_BaseAddr = CFG_M8_GET_DDR1_TA_FROM_DTAR0(nPUB1_DTAR0,nMMC_DDR_SET);

			serial_puts("\nAml log : DDR0 DTAR is 0x");
			serial_put_hex(nDDR0_BaseAddr,32);
			
			serial_puts("\nAml log : DDR1 DTAR is 0x");
			serial_put_hex(nDDR1_BaseAddr,32);
			serial_puts("\n");

		}break;
	}

	//disable clock
	writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);  
	writel(readl(P_DDR1_CLK_CTRL) & (~1), P_DDR1_CLK_CTRL);

*/
