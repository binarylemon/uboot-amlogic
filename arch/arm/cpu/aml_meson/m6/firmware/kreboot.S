/*
 * aml8726m M3
 * @author wenwu.zhang
 *
 */
#include <config.h>
#include <asm/arch/romboot.h>

.globl pwr_reboot
pwr_reboot: 
	b	pwr_restart
	b pwr_undefined_instruction
	b pwr_software_interrupt
	b pwr_prefetch_abort
	b pwr_data_abort
	b pwr_not_used
	b pwr_irq
	b pwr_fiq
reset_addr:
	.word 0
pwr_restart:
    dmb
    isb
    mov sp,#0x4000
    
 		ldr r0,=0xda002020
 		ldr r1,=0xFFFF
 		str r1,[r0]      /*DMC_SEC_PORT0_RANGE0*/
 		str r1,[r0,#4]   /*DMC_SEC_PORT1_RANGE0*/
 		str r1,[r0,#8]   /*DMC_SEC_PORT2_RANGE0*/
 		str r1,[r0,#12]  /*DMC_SEC_PORT3_RANGE0*/
 		str r1,[r0,#16]  /*DMC_SEC_PORT4_RANGE0*/
 		str r1,[r0,#20]  /*DMC_SEC_PORT5_RANGE0*/
 		str r1,[r0,#24]  /*DMC_SEC_PORT6_RANGE0*/
 		str r1,[r0,#28]  /*DMC_SEC_PORT7_RANGE0*/
 		ldr r1,=0x80000000
 		str r1,[r0,#0x84] /*DMC_SEC_CTRL*/
 		mov r1,#0xFF
 		ldr r0,=0xc8006400
 		str r1,[r0]
 		 		
    /* for debug */
    adr r1,reset_addr
    ldr r0,[r1]
 	  mov pc,r0
    mov r1, #0
dbg_loop:
		cmp r1,#0
		beq dbg_loop 
    wfi
    
pwr_undefined_instruction: 
	mov r0,#1
	b pwr_fail_sleep
pwr_software_interrupt:	
	mov r0,#2
	b pwr_fail_sleep
pwr_prefetch_abort:	
	mov r0,#3
	b pwr_fail_sleep
pwr_data_abort:
	mov r0,#4
	b pwr_fail_sleep		
pwr_not_used:		
	mov r0,#5
	b pwr_fail_sleep
pwr_irq:	
	mov r0,#6
	b pwr_fail_sleep		
pwr_fiq:
	mov r0,#7
	b pwr_fail_sleep	
pwr_fail_sleep:
	wfi
	