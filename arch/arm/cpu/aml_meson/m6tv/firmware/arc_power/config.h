#ifndef _CONFIG_H_
#define _CONFIG_H_
#define RAM_START       0x000000
#define RAM_SIZE        16*1024
#define RAM_END         (RAM_START+RAM_SIZE)
#define _STACK_END      RAM_END
#define ROMBOOT_START  RAM_START


#define CONFIG_BAUDRATE 115200
#define CONFIG_CRYSTAL_MHZ 24

/*
 * PMU device select, CONFIG_AML_PMU and CONFIG_AW_AXP20 should not be both existed
 */
//#define CONFIG_AML_PMU 
//#define CONFIG_AW_AXP20

#ifdef CONFIG_AW_AXP20
#define CONFIG_CONST_PWM_FOR_DCDC
#define CONFIG_DISABLE_LDO3_UNDER_VOLTAGE_PROTECT
#define CONFIG_DCDC2_VOLTAGE	1500
#define CONFIG_DCDC3_VOLTAGE	1100
#define CONFIG_LDO2_VOLTAGE	3000
#define CONFIG_LDO3_VOLTAGE	2500
#define CONFIG_LDO4_VOLTAGE	3300
#endif

#if defined(CONFIG_AML_PMU) && defined(CONFIG_AW_AXP20)
#error You should only select one of CONFIG_AML_PMU and CONFIG_AW_AXP20
#endif

#ifndef __ASSEMBLY__
void serial_init_with_clk(unsigned clk);
void serial_put_dword(unsigned int data);
void serial_put_dec(unsigned int data);
void serial_puts(const char *s);
void serial_putc(const char c);
int serial_tstc(void);
int serial_getc(void);
void serial_init(unsigned set,unsigned tag);
void serial_put_hex(unsigned int data,unsigned bitlen);
void f_serial_puts(const char *s);
void k_delay(void);
unsigned get_uart_ctrl(void);
unsigned long    clk_util_clk_msr(   unsigned long   clk_mux, unsigned long   uS_gate_time );
void init_pctl(void);
void init_dmc(void);
void mmc_wakeup(void);
void mmc_sleep(void);
void disable_mmc_req(void);
void enable_mmc_req(void);
void reset_mmc();
unsigned delay_tick(unsigned count);
void disp_pctl(void);
void disable_retention(void);
void enable_retention(void);
void save_ddr_settings();
#define OUT_INIT()      
#define OUT_PUT(a)      
#endif
#endif
