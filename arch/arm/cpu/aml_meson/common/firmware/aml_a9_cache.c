
#ifndef  __AML_A9_CACHE__
#define  __AML_A9_CACHE__

#include <asm/system.h>

//code from \arch\arm\lib\cache-cp15.c
static inline void cp_delay (void)
{
	volatile int i;
	/* copro seems to need some delay between reading and writing */
	for (i = 0; i < 100; i++)
		nop();
	asm volatile("" : : : "memory");
}

/* to activate the MMU we need to set up virtual memory: use 1M areas */
static inline void mmu_setup(void)
{	

	/*
	 * Invalidate L1 I/D
	 */
	asm volatile("mov	r0, #0");//			@ set up for MCR
	asm volatile("mcr	p15, 0, r0, c8, c7, 0");//	@ invalidate TLBs
	asm volatile("mcr	p15, 0, r0, c7, c5, 0");//	@ invalidate icache

	/*
	 * disable MMU stuff and caches
	 */
	asm volatile("mrc	p15, 0, r0, c1, c0, 0");//
	asm volatile("bic	r0, r0, #0x00002000");//	@ clear bits 13 (--V-)
	asm volatile("bic	r0, r0, #0x00000007");//	@ clear bits 2:0 (-CAM)
	asm volatile("orr	r0, r0, #0x00000002");//	@ set bit 1 (--A-) Align
	asm volatile("orr	r0, r0, #0x00000800");//	@ set bit 12 (Z---) BTB
	asm volatile("mcr	p15, 0, r0, c1, c0, 0");//

	/////////////////////////////////////////////////////////////	
	unsigned int *pVMMUTable = (0xd9000000 + 32 * 1024);
	int i = 0;
	uint nVal = 0;
	for(i = 0 ; i < 0x1000;++i)
	{
		if(i< 0x10)
			nVal = (i<<20)|(SEC_PROT_RW_RW | SEC_SO_MEM);
		if((i>= 0x10 && i< 0x800) ||
			(i>= 0xd91 && i< 0xda0) ||
			(i>= 0xC13 && i< 0xC42) ||
			(i>= 0xC44 && i< 0xC80) ||
			(i>= 0xC00 && i< 0xC11) ||
			(i>= 0xd01 && i< 0xd90) ||
			(i>= 0xda1 && i< 0xe00))
			nVal = 0;
		
		if((i>= 0x800 && i< 0xa00) || (i>= 0xd90 && i< 0xd91))
			nVal = (i<<20)|(SEC_PROT_RW_RW | SEC_WB);

		if(i>= 0xA00 && i< 0xC00)
			nVal = (i<<20)|(SEC_PROT_RW_RW | SEC_SO_MEM);

		if(i>= 0xC11 && i< 0xC13)
			nVal = (i<<20)|(SEC_PROT_RW_NA | SEC_XN | SEC_SO_MEM);

		if((i>= 0xC42 && i< 0xC44) || (0xda0 == i))
			nVal = (i<<20)|(SEC_PROT_RW_NA |  SEC_XN | SEC_DEVICE);

		if(i>= 0xC80 && i< 0xd01)
			nVal = (i<<20)|(SEC_PROT_RW_NA | SEC_XN | SECTION);
				 
		if(i>= 0xe00 && i<= 0xfff)
			nVal = (i<<20)|(SEC_PROT_RW_RW | SEC_SO_MEM);
	
		*(pVMMUTable+i) = nVal;//(i << 20 | (SEC_PROT_RW_RW | SEC_WB));
		
	}
	u32 *page_table = (u32 *)(pVMMUTable);
	//int i;
	u32 reg;

	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (page_table) : "memory");
	/* Set the access control to client */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (0x55555555));
		     	
 	asm volatile("mcr p15, 0, %0, c7, c5, 6"   : : "r" (0));   // invalidate BTAC    				   	
 	asm volatile("mcr p15, 0, %0, c7, c5, 0"   : : "r" (0));   // invalidate ICache
    asm volatile("dsb");
    asm volatile("mcr p15, 0, %0, c8, c7, 0"	  : : "r" (0));    // invalidate TLBs
	asm volatile("dsb");
 	asm volatile("isb");
	
	/* and enable the mmu */
	reg = get_cr();	/* get control reg. */
	cp_delay();
	set_cr(reg | CR_M);
}

/* cache_bit must be either CR_I or CR_C */
static void cache_enable(uint32_t cache_bit)
{
	uint32_t reg;

	/* The data cache is not active unless the mmu is enabled too */
	if (cache_bit == CR_C)
		mmu_setup();
	reg = get_cr();	/* get control reg. */
	cp_delay();
	set_cr(reg | cache_bit);
}

static int dcache_status(void)
{
	return (get_cr() & CR_C) != 0;
}

//code from \arch\arm\lib\cache_v7.S
//@void _clean_invd_dcache(void);
static void _clean_invd_dcache()
{
    //asm volatile("push    {r4,r5,r6,lr}");
    asm volatile("push    {r4,r5,r6}");
    asm volatile("MOV     r2,#0");
    asm volatile("MOV     r3,#0x40000000");
    asm volatile("MOV     r4,#0x80000000");
    asm volatile("MOV     r5,#0xc0000000");
    //asm volatile("MOVEQ   r6,#0x2000");
    //asm volatile("MOVNE   r6,#0x4000");
    asm volatile("MOV   r6,#0x4000");
	asm volatile("clean_invd_loop:");
    asm volatile("MCR     p15,0,r2,c7,c14,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r3,c7,c14,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r4,c7,c14,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r5,c7,c14,2");//  @ invalidate dcache by set / way
    asm volatile("ADD     r2,r2,#0x20");
    asm volatile("ADD     r3,r3,#0x20");
    asm volatile("ADD     r4,r4,#0x20");
    asm volatile("ADD     r5,r5,#0x20");
    asm volatile("CMP     r2,r6");
    asm volatile("BNE     clean_invd_loop");
    asm volatile("DSB");
    //asm volatile("pop {r4, r5,r6, pc}");
    asm volatile("pop {r4, r5,r6}");
}

//code from \arch\arm\lib\cache_v7.S
//@void _clean_dcache(void);
static void _clean_dcache()
{
    asm volatile("push    {r4,r5,r6}");
    asm volatile("MOV     r2,#0     ");
    asm volatile("MOV     r3,#0x40000000");
    asm volatile("MOV     r4,#0x80000000");
    asm volatile("MOV     r5,#0xc0000000");
    //asm volatile("MOVEQ   r6,#0x2000");
    //asm volatile("MOVNE   r6,#0x4000");
    asm volatile("MOV   r6,#0x4000");
	asm volatile("clean_loop:");
    asm volatile("MCR     p15,0,r2,c7,c10,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r3,c7,c10,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r4,c7,c10,2");//  @ invalidate dcache by set / way
    asm volatile("MCR     p15,0,r5,c7,c10,2");//  @ invalidate dcache by set / way
    asm volatile("ADD     r2,r2,#0x20");//
    asm volatile("ADD     r3,r3,#0x20");//
    asm volatile("ADD     r4,r4,#0x20");//
    asm volatile("ADD     r5,r5,#0x20");//
    asm volatile("CMP     r2,r6");//
    asm volatile("BNE     clean_loop");//
    asm volatile("DSB");//
    asm volatile("pop {r4, r5,r6}");//
}
    
static void dcache_flush(void)
{
    if(dcache_status())
    {    	
        _clean_invd_dcache();
    }
}


/* cache_bit must be either CR_I or CR_C */
static void cache_disable(uint32_t cache_bit)
{
	uint32_t reg;

	if (cache_bit == CR_C) {
		/* if cache isn;t enabled no need to disable */
		reg = get_cr();
		if ((reg & CR_C) != CR_C)
			return;
		/* if disabling data cache, disable mmu too */
		cache_bit |= CR_M;
		//flush_cache(0, ~0);
		dcache_flush();
	}
	reg = get_cr();
	cp_delay();
	set_cr(reg & ~cache_bit);
}

static void aml_cache_disable()
{
	_clean_dcache();
	cache_disable(CR_I);
	cache_disable(CR_C);
	memset((0xd9000000 + 32 * 1024),0,0x4000);
}

static void aml_cache_enable()
{
	//cache_disable(CR_I);
	//cache_disable(CR_C);
	
	aml_cache_disable();
	cp_delay();	
	cache_enable(CR_I);
	cache_enable(CR_C);
}


#endif // __AML_A9_CACHE__
