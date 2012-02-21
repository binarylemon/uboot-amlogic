
#include <config.h>
#include <amlogic/efuse.h>

/** efuse layout
http://wiki-sh.amlogic.com/index.php/How_To_burn_the_info_into_E-Fuse
0~3					licence					1 check byte				4 bytes(in total)
4~10				mac						1 check byte				7 bytes(in total)
12~322			hdcp					10 check byte				310 bytes(in total)
322~328 			mac_bt				1 check byte				7 bytes(in total)
330~336 			mac_wifi				1 check byte				7 bytes(in total)
337~384  		usid						2 check byte		 		48 bytes(in total)
*/

/**
	new efuse version 0.0 (for exist MP production, compitable version)
*/
/** efuse layout before M3 (for M1, M2, A3)
0~3					licence					1 check byte				4 bytes(in total)
4~10				mac						1 check byte				7 bytes(in total)
12~322			hdcp					10 check byte				310 bytes(in total)
322~328 			mac_bt				1 check byte				7 bytes(in total)
330~336 			mac_wifi				1 check byte				7 bytes(in total)
337~379  		usid						2 check byte		 		44 bytes(in total)
380~384			version				1 check byte 				4 bytes(in total) (it should be zero)
*/

/**
 * efuse version 0.1 (for M3 ) 
 * M3 efuse: read all free efuse data maybe fail on addr 0 and addr 0x40
 * so M3 EFUSE layout avoid using 0 and 0x40
 */
/** 
1~38				usid						2 check byte				38 bytes(in total)
39~45	 			mac_wifi				1 check byte				7 bytes(in total)
46~52	 			mac_bt				1 check byte				7 bytes(in total)
53~59				mac						1 check byte				7 bytes(in total)
60~63				licence					1 check byte				4 bytes(in total)
68~377			hdcp					10 check byte				310 bytes(in total)
380~383			version 				1 check byte				4 bytes(in total) (version=1)
* 		3 valid byte is 1byte for version and 2byte for customerid . version is 1
* 		
*/

/**
 * efuse version 0.2 (after M6 )
*/
/** 
0~2					licence					3bytes ( some basic feature control)
3						version 				1byte  ( must be 2)
4~7					customerid			4bytes (0 , keep , should not be 0)
8~135				rsa key					128bytes (if Secure Boot enable )
136~435			hdcp 					300bytes			
436~441			mac0					6bytes for the main network interface
442~447			mac1					6bytes for the second network interface or bt 
448~453			mac2					6bytes for the second network interface or bt 
454~511			userid 					58bytes (not used)
*/
	
static efuseinfo_item_t efuseinfo_v0[] = 
{
	{
		.title = "licence",
		.offset = 0,
		.enc_len = 4,
		.data_len = 3,
		.we = 0,
		.bch_en = 1,
		.bch_reverse = 1,
	},
	{
		.title = "mac",
		.offset = 4,
		.enc_len = 7,
		.data_len = 6,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 1,
	},
	{
		.title = "hdcp",
		.offset = 12,
		.enc_len = 310,
		.data_len = 300,
		.we = 0,
		.bch_en = 1,
		.bch_reverse = 1,
	},
	{
		.title = "mac_bt",
		.offset = 322,
		.enc_len = 7,
		.data_len = 6,
		.we=1,
		.bch_en = 1,
		.bch_reverse = 1,
	},
	{
		.title = "mac_wifi",
		.offset = 330,
		.enc_len = 7,
		.data_len = 6,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 1,
	},
	{
		.title = "usid",
		.offset = 337,
		.enc_len = 43,
		.data_len = 41,
		.we = 0,
		.bch_en = 1,
		.bch_reverse = 1,
	},	
	{
		.title= "version",     //1B(version=0)+2B(machid)
		.offset=380,
		.enc_len = 4,
		.data_len = 3,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 0,
	},
};

static efuseinfo_item_t efuseinfo_v1[] =
{
	{
		.title = "usid",
		.offset = 1,
		.enc_len = 38,
		.data_len = 36,
		.we = 0,
		.bch_en = 1,
		.bch_reverse = 0,
	},	
	{
		.title = "mac_wifi",
		.offset = 39,
		.enc_len = 7,
		.data_len = 6,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 0,
	},
	{
		.title = "mac_bt",
		.offset = 46,
		.enc_len = 7,
		.data_len = 6,
		.we=1,
		.bch_en = 1,
		.bch_reverse = 0,
	},
	{
		.title = "mac",
		.offset = 53,
		.enc_len = 7,
		.data_len = 6,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 0,
	},
	{
		.title = "licence",
		.offset = 60,
		.enc_len = 4,
		.data_len = 3,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 1,
	},	
	{
		.title = "hdcp",
		.offset = 68,
		.enc_len = 310,
		.data_len = 300,
		.we = 0,
		.bch_en = 1,
		.bch_reverse = 0,
	},
	{
		.title= "version",     //1B(version=1)+2B(machid)
		.offset=380,
		.enc_len = 4,
		.data_len = 3,
		.we = 1,
		.bch_en = 1,
		.bch_reverse = 0,
	},
};

// after M6
static efuseinfo_item_t efuseinfo_v2[] = 
{
	{
		.title = "licence",
		.offset = 0,
		.enc_len = 3,
		.data_len = 3,
		.we = 0,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "version",   // include machid
		.offset = 3,
		.enc_len = 5,
		.data_len = 5,
		.we = 1,
		.bch_en = 0,	
		.bch_reverse = 0,
	},
	{
		.title = "rsakey",
		.offset = 8,
		.enc_len = 128,
		.data_len = 128,
		.we = 0,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "hdcp",
		.offset=136,
		.enc_len = 300,
		.data_len = 300,
		.we = 0,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "mac",    //for the main network interface
		.offset = 436,
		.enc_len = 6,
		.data_len = 6,
		.we=1,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "mac_bt",  //for the second network interface or bt
		.offset = 442,
		.enc_len = 6,
		.data_len = 6,
		.we=1,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "mac_wifi", //for the second network interface or bt
		.offset = 448,
		.enc_len = 6,
		.data_len = 6,
		.we = 1,
		.bch_en = 0,
		.bch_reverse = 0,
	},
	{
		.title = "usid",   
		.offset = 454,
		.enc_len = 58,
		.data_len = 58,
		.we=0,
		.bch_en = 0,
		.bch_reverse = 0,
	},
};

efuseinfo_t efuseinfo[] = 
{
	{
		.efuseinfo_version = efuseinfo_v0,
		.size = sizeof(efuseinfo_v0)/sizeof(efuseinfo_item_t),
		.version = 0,
	},
	{
		.efuseinfo_version = efuseinfo_v1,
		.size = sizeof(efuseinfo_v1)/sizeof(efuseinfo_item_t),
		.version =1,
	},
	{
		.efuseinfo_version = efuseinfo_v2,
		.size = sizeof(efuseinfo_v2)/sizeof(efuseinfo_item_t),
		.version = 2,
	},
};

int efuseinfo_num = sizeof(efuseinfo)/sizeof(efuseinfo_t);
int efuse_active_version = -1;
unsigned efuse_active_customerid = 0;
pfn efuse_getinfoex = 0;

