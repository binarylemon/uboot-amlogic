#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/romboot.h>
#include <romboot.c>
#ifndef CONFIG_AML_UBOOT_MAGIC
#define CONFIG_AML_UBOOT_MAGIC 0x12345678
#endif
#ifndef CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK
SPL_STATIC_FUNC short check_sum(unsigned * addr)
{
    int i;
    unsigned short * p=(unsigned short *)addr;
    serial_put_dword(addr[15]);
    if(addr[15]!=CONFIG_AML_UBOOT_MAGIC)
        return -1;
#if 0        
    for(i=0;i<size>>1;i++)
        check_sum^=p[i];
#endif        
    return 0;
}
#endif
SPL_STATIC_FUNC int load_uboot(unsigned __TEXT_BASE,unsigned __TEXT_SIZE)
{

	unsigned por_cfg=romboot_info->por_cfg;
	unsigned boot_id=romboot_info->boot_id;
	unsigned size;
	int i;
	unsigned * mem;
	int rc=0;

	size=__TEXT_SIZE;
	if(boot_id>1)
        boot_id=0;
	if(boot_id==0)
    {
       rc=fw_load_intl(por_cfg,__TEXT_BASE,size);
	}else{
	   rc=fw_load_extl(por_cfg,__TEXT_BASE,size); 
	}
#ifndef CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK	
	if(!rc&&check_sum(__TEXT_BASE)==0)
	{
	    fw_print_info(por_cfg,boot_id);
        return ;
    }
#else
    if(rc==0)
	{
	    fw_print_info(por_cfg,boot_id);
        return ;
    }
#endif    
	while(rc)
	{
        debug_rom(__FILE__,__LINE__);	        
#if CONFIG_ENABLE_EXT_DEVICE_RETRY	
        
	    rc=fw_init_extl(por_cfg);//INTL device  BOOT FAIL
	    if(rc)
	        continue;
	    rc=fw_load_extl(por_cfg,__TEXT_BASE,size);
	    if(rc)
	        continue;
#ifndef CONFIG_DISABLE_INTERNAL_U_BOOT_CHECK	        
	    rc=check_sum(__TEXT_BASE);
#endif	    
#endif	    
	}
	fw_print_info(por_cfg,1);
    return ;
}


