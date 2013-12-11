/***************************************************************************************************************
 * Amlogic M8 secure-boot solution
 *
 * Feature: decrypt & check image (uboot/kernel/efuse)
 *
 * Author: Haixiang.bao <haixiang.bao@amlogic.com>
 * 
 * Function:
 *           1. decrypt & check TPL/kernel image for secure boot
 *           2. decrypt & check EFUSE pattern for secure boot
 *
 * Copyright (c) 2013 Amlogic Inc.
 * 
 * Create date: 2013.12.10
 *
 * IMPORTANT : All the feature is chip dependent, please DO NOT update
 *             any of this file before verified.
 * Remark: 
 *			 1. 2013.12.10 v0.1 for decrypt only
 *
 **************************************************************************************************************/

#if !defined(__AMLOGIC_M8_SECURE_C_BFD0A6CA_8E97_47E1_9DDD_2E4544A831AE__)
#define __AMLOGIC_M8_SECURE_C_BFD0A6CA_8E97_47E1_9DDD_2E4544A831AE__

//help info show config
#define AML_SECURE_PROCESS_MSG_SHOW 1

unsigned int g_action[3][16]={
	{0xd904706c,0xd904b294,0xd904038c,0xd904441c,0xd9047458,0xd9044518,
	 0xd904455c,0xd904459c,0xd9046e44,0xd904d014,0xd904cf60,0xd904a4f4},
	{0xd9046f90,0xd904b080,0xd904038c,0xd9044340,0xd904737c,0xd904443c,
	 0xd9044480,0xd90444c0,0xd9046d68,0xd904cde0,0xd904cd2c,0xd904a2e0},
	{0xd904716c,0xd904b394,0xd904038c,0xd934441c,0xd9047258,0xd9044578, 
	 0xd904438c,0xd90444c4,0xd9046d98,0xd904cfe0,0xd904ce2c,0xd904a280},
	 };

typedef  void (*t_func_v1)( int a);
typedef  void (*t_func_v2)( int a, int b);
typedef  void (*t_func_v3)( int a, int b, int c);
typedef  void (*t_func_v4)( int a, int b, int c, int d);

typedef  int  (*t_func_r1)( int a);
typedef  int  (*t_func_r2)( int a, int b);
typedef  int  (*t_func_r3)( int a, int b, int c);

static int aml_m8_sec_boot_check(unsigned char *pSRC,char *pkey1,char *pkey2)
{	

#if defined(AML_SECURE_PROCESS_MSG_SHOW)

#if defined(CONFIG_AMLROM_SPL)
	#define AML_MSG_FAIL ("Aml log : ERROR! TPL secure check fail!\n")
	#define AML_MSG_PASS ("Aml log : TPL secure check pass!\n")	
	#define MSG_SHOW serial_puts	
#else
	#define AML_MSG_FAIL ("Aml log : ERROR! Image secure check fail!\n")
	#define AML_MSG_PASS ("Aml log : Image secure check pass!\n")
	#define MSG_SHOW printf
#endif

#endif //#if defined(AML_SECURE_PROCESS_MSG_SHOW)
		
#define AMLOGIC_CHKBLK_ID  (0x434C4D41) //414D4C43 AMLC
#define AMLOGIC_CHKBLK_VER (1)

	typedef struct {
		unsigned int	nSizeH; 	   ////4@0
		unsigned int	nLength1;      ////4@4
		unsigned int	nLength2;      ////4@8
		unsigned int	nLength3;      ////4@12
		unsigned int	nLength4;      ////4@16
		unsigned char	szkey1[116];   ////116@20
		unsigned char	szkey2[108];   ////108@136
		unsigned int	nSizeT;        ////4@244
		unsigned int	nVer;          ////4@248
		unsigned int	unAMLID;       ////4@252
	}st_aml_chk_blk; //256

	typedef struct{
		int ver; int len;
		unsigned char szBuf1[12];
		unsigned char szBuf2[188];
	} st_crypto_blk1;

	typedef struct{
		int nr;
		unsigned int buff[80];
	} st_crypto_blk2;

	int i;
	int nRet = -1;
	st_crypto_blk1 cb1_ctx;	
	st_crypto_blk2 cb2_ctx;
	st_aml_chk_blk chk_blk;	
	unsigned char szkey[32+16];	
	unsigned int *ct32 = (unsigned int *)(pSRC);
	unsigned char *pBuf = (unsigned char *)&chk_blk;
	int nStep = 0;	
	
	switch(* (unsigned int *)0xd9040004)
	{
	case 0x25e2: break;
	case 0x27ed: nStep = 1 ; break;
	default: goto exit;break;
	}
		
	t_func_v3 fp_00 = (t_func_v3)g_action[nStep][0]; //void rsa_init(1,2,3)
	t_func_r3 fp_01 = (t_func_r3)g_action[nStep][1]; //int mpi_read_string(1,2,3)
	t_func_v3 fp_02 = (t_func_v3)g_action[nStep][2]; //void efuse_read(1,2,3)
	t_func_r2 fp_03 = (t_func_r2)g_action[nStep][3]; //int boot_rsa_read_puk(a,b)
	t_func_r3 fp_04 = (t_func_r3)g_action[nStep][4]; //int rsa_public(1,2,3)
	t_func_v2 fp_05 = (t_func_v2)g_action[nStep][5]; //void boot_aes_setkey_dec(1,2)
	t_func_v1 fp_06 = (t_func_v1)g_action[nStep][6]; //void boot_aes_setiv_init(1)
	t_func_v4 fp_07 = (t_func_v4)g_action[nStep][7]; //void boot_aes_crypt_cbc(1,2,3,4)
	t_func_v4 fp_08 = (t_func_v4)g_action[nStep][8]; //void sha2(1,2,3,4)
	t_func_r3 fp_09 = (t_func_r3)g_action[nStep][9]; //int memcpy(1,2,3)
	t_func_r3 fp_10 = (t_func_r3)g_action[nStep][10];//int memcmp(1,2,3)
	t_func_r1 fp_11 = (t_func_r1)g_action[nStep][11];//int mpi_msb(1)

	fp_00(&cb1_ctx,0,0);
	if(pkey1 && pkey2)
	{	
		if(fp_01(cb1_ctx.szBuf1,16,pkey1) ||	fp_01(cb1_ctx.szBuf2,16,pkey2))
			goto exit;
		cb1_ctx.len = ( fp_11( cb1_ctx.szBuf1 ) + 7 ) >> 3;			
	}
	else
	{
		unsigned int nState  = 0;
		fp_02(&nState,0,4);		
		fp_03(&cb1_ctx,(nState & (1<<23)) ? 1 : 0);
		cb1_ctx.len = (nState & (1<<23)) ? 256 : 128;
	}
	
	fp_09((unsigned char*)&chk_blk,(unsigned char*)pSRC,sizeof(chk_blk));

	for(i = 0;i< sizeof(chk_blk);i+=cb1_ctx.len)
		if(fp_04(&cb1_ctx, pBuf+i, pBuf+i ))
			goto exit;

	if(AMLOGIC_CHKBLK_ID != chk_blk.unAMLID ||
		AMLOGIC_CHKBLK_VER < chk_blk.nVer)
		goto exit;

	if(sizeof(st_aml_chk_blk) != chk_blk.nSizeH ||
		sizeof(st_aml_chk_blk) != chk_blk.nSizeT ||
		chk_blk.nSizeT != chk_blk.nSizeH)
		goto exit;

	if((sizeof(st_aml_chk_blk) != chk_blk.nLength2))
		goto exit;

	if(chk_blk.nLength2)
		fp_09((void*)pSRC,(void*)(pSRC+chk_blk.nLength1),
			chk_blk.nLength2);

	fp_09((void*)szkey,(void*)chk_blk.szkey2,sizeof(szkey));
	fp_05( &cb2_ctx, szkey );
	fp_06(&szkey[32]);
	for (i=0; i<(chk_blk.nLength4)/16; i++)
		fp_07 ( &cb2_ctx, &szkey[32], &ct32[i*4], &ct32[i*4] );
	
	fp_08( pSRC,chk_blk.nLength3, szkey, 0 );	
	if(fp_10(szkey,chk_blk.szkey1,32))
		goto exit;

	nRet = 0;

exit:

#if defined(AML_SECURE_PROCESS_MSG_SHOW)
	MSG_SHOW (nRet ? AML_MSG_FAIL : AML_MSG_PASS);
		
	#undef AML_MSG_FAIL
	#undef AML_MSG_PASS
	#undef MSG_SHOW		
	
#endif //#if defined(AML_SECURE_PROCESS_MSG_SHOW)	

	return nRet;
	
}


int aml_sec_boot_check(unsigned char *pSRC)
{
	return aml_m8_sec_boot_check(pSRC,0,0);
}

#if !defined(CONFIG_AMLROM_SPL)
int aml_sec_boot_check_efuse(unsigned char *pSRC)
{
	char sz1[] = "A38DE0396359BE3AA3E8615D325F91CE7EAB601352BAFB1006E0AED67AB3E30D58D4AA6019DE066BACE68DAC26507457D1008862FE45349C2BD2D71401FA0AFAA848939905B40146CB9577890F3CCF5D073612017260AB83D3176B1EAFB81390D0BE7075DEC9D3F62BB48DA64D7EDCD7622118CE4C1DCA159EC58BB12D7914EB";
	char sz2[] = "010001";

	return aml_m8_sec_boot_check(pSRC,sz1,sz2);
}
#endif //

//here can add more feature like encrypt...

#endif //__AMLOGIC_M8_SECURE_C_BFD0A6CA_8E97_47E1_9DDD_2E4544A831AE__