#include <common.h>

DECLARE_GLOBAL_DATA_PTR;
int dram_init(void)
{
    /**
    @todo : implement it
    **/
/*    unsigned int size0 = 0, size1 = 0;
    gd->ram_size = size0 + size1;*/
    gd->ram_size = PHYS_MEMORY_SIZE;
    return 0;
}
