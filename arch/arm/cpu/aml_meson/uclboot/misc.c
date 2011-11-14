// 
//  misc.c
//  u-boot self-decompress
//  
//  Created by Amlogic on 2009-07-21.
//  Copyright 2009 Amlogic. All rights reserved.
// 

extern int uclDecompress(char* op, unsigned o_len, char* ip);

//extern unsigned CONFIG_SYS_TEXT_BASE;
//extern void *UCL_TEXT_BASE;
extern void *input_data;
extern void *input_data_end;
extern void mmu_disable();
	
void start_arcboot_ucl(void)
{
	typedef void (* JumpAddr)(void);	
	unsigned len ;
	serial_puts("ucl decompress in TPL: \n");
	uclDecompress(CONFIG_SYS_TEXT_BASE,&len,&input_data);
    serial_puts("decompress finished\n");
    serial_wait_tx_empty();
        
    mmu_disable();
    dcache_disable();    
    icache_disable();
    
    JumpAddr target=(JumpAddr)(CONFIG_SYS_TEXT_BASE);
    target();
		
}
