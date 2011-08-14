#ifndef __AMLOGIC_NAND_PLATFORM_H_
#define __AMLOGIC_NAND_PLATFORM_H_
typedef void (* claim_bus_t)(uint32_t get);
struct aml_nand_platform{
    uint32_t        reg_base;
    uint32_t        delay;
    uint32_t        rbmod;
    uint32_t        t_rea;
    uint32_t        t_rhoh;
    uint32_t        ce_num;
    uint32_t        clk_src;
    claim_bus_t     claim_bus;
};

#endif
