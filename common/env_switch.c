#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <command.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>
#include <asm/arch/nand.h>
#include <linux/err.h>

#ifdef CONFIG_SPI_NAND_COMPATIBLE

DECLARE_GLOBAL_DATA_PTR;

/* references to names in env_common.c */
extern uchar default_environment[];

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */

char * env_name_spec;


int env_init(void)
{
	printk("env_init %s %d\n",__func__,__LINE__);
	if( (((BOOT_DEVICE_FLAG & 7) == 7) || ((BOOT_DEVICE_FLAG & 7) == 6))){
		printk("NAND BOOT, %s %d \n",__func__,__LINE__);
		env_name_spec = "NAND";	
		nand_env_init();
	}else if( (((BOOT_DEVICE_FLAG & 7) == 5) || ((BOOT_DEVICE_FLAG & 7) == 4))){
		printk("SPI BOOT, %s %d \n",__func__,__LINE__);
		env_name_spec = "SPI Flash";
		spi_env_init();
	}else {
		printk("MMC BOOT, %s %d \n",__func__,__LINE__);
		return -1;
	}

	return 0;
}

void env_relocate_spec(void)
{
	printk(" %s %d\n",__func__,__LINE__);
	if( (((BOOT_DEVICE_FLAG & 7) == 7) || ((BOOT_DEVICE_FLAG & 7) == 6))){
		printk("NAND BOOT,nand_env_relocate_spec : %s %d \n",__func__,__LINE__);
		env_name_spec = "NAND";
		nand_env_relocate_spec();
	}else if( (((BOOT_DEVICE_FLAG & 7) == 5) || ((BOOT_DEVICE_FLAG & 7) == 4))){
		printk("SPI BOOT,spi_env_relocate_spec : %s %d \n",__func__,__LINE__);
		env_name_spec = "SPI Flash";
		spi_env_relocate_spec();
	}else {
		printk("MMC BOOT, %s %d \n",__func__,__LINE__);
	}
}

int saveenv(void)
{
	int ret = 0;
	
	printk("saveenv %s %d\n",__func__,__LINE__);
	if( (((BOOT_DEVICE_FLAG & 7) == 7) || ((BOOT_DEVICE_FLAG & 7) == 6))){
		printk("NAND BOOT,nand_saveenv :%s %d \n",__func__,__LINE__);
		ret = nand_saveenv();
	}else if( (((BOOT_DEVICE_FLAG & 7) == 5) || ((BOOT_DEVICE_FLAG & 7) == 4))){
		printk("SPI BOOT,spi_saveenv : %s %d \n",__func__,__LINE__);
		ret = spi_saveenv();
	}else {
		printk("MMC BOOT, %s %d \n",__func__,__LINE__);
		return -1;
	}

	return ret;
}

#endif


