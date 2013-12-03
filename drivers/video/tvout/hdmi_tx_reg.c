/*
 * Amlogic M1 
 * frame buffer driver-----------HDMI_TX
 * Copyright (C) 2010 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <asm/arch/io.h>
#include <common.h>
#include "hdmi_tx_reg.h"

// if the following bits are 0, then access HDMI IP Port will cause system hungup
#define GATE_NUM    2
Hdmi_Gate_s hdmi_gate[GATE_NUM] =   {   {HHI_HDMI_CLK_CNTL, 8},
                                        {HHI_GCLK_MPEG2   , 4},
                                    };

// In order to prevent system hangup, add check_cts_hdmi_sys_clk_status() to check 
static void check_cts_hdmi_sys_clk_status(void)
{
    int i;

    for(i = 0; i < GATE_NUM; i++){
        if(!(READ_CBUS_REG(hdmi_gate[i].cbus_addr) & (1<<hdmi_gate[i].gate_bit))){
            printf("HDMI Gate Clock is off, turn on now\n");
            WRITE_CBUS_REG_BITS(hdmi_gate[i].cbus_addr, 1, hdmi_gate[i].gate_bit, 1);
        }
    }
}

unsigned long hdmi_rd_reg(unsigned long addr)
{
    unsigned long data;
    check_cts_hdmi_sys_clk_status();
#ifdef CONFIG_AML_MESON_8
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    data = READ_APB_HDMI_REG(HDMI_DATA_PORT);
#else
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);
    data = READ_APB_REG(HDMI_DATA_PORT);
#endif
  
    return (data);
}


void hdmi_wr_only_reg(unsigned long addr, unsigned long data)
{
    check_cts_hdmi_sys_clk_status();
#ifdef CONFIG_AML_MESON_8
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    
    WRITE_APB_HDMI_REG(HDMI_DATA_PORT, data);
#else
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);

    WRITE_APB_REG(HDMI_DATA_PORT, data);
#endif
}

void hdmi_wr_reg(unsigned long addr, unsigned long data)
{
    unsigned long rd_data;
    
    check_cts_hdmi_sys_clk_status();
#ifdef CONFIG_AML_MESON_8
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_HDMI_REG(HDMI_ADDR_PORT, addr);
    
    WRITE_APB_HDMI_REG(HDMI_DATA_PORT, data);
#else
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);
    WRITE_APB_REG(HDMI_ADDR_PORT, addr);

    WRITE_APB_REG(HDMI_DATA_PORT, data);
#endif
    rd_data = hdmi_rd_reg (addr);
    if (rd_data != data) 
    {
        //printk("hdmi_wr_reg(%x,%x) fails to write: %x\n",addr, data, rd_data);
       //while(1){};      
    }
}



