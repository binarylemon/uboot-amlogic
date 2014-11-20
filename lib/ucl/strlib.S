

        .code 32
        .align 5
        .globl init_neon
        .func
init_neon:
        .fnstart
		MRC		p15, 0, r12, c1, c0, 2
		ORR		r12, r12, #(3<<20)
		ORR		r12, r12, #(3<<22)
		BIC		r12, r12, #(3<<30)
		MCR		p15, 0, r12, c1, c0, 2
		ISB		
		MOV		r12, #(1<<30)
		MCR		p10, #7, r12, c8, c0, 0
		MRC		p15, 0, r12, c1, c0, 0
		BIC		r12, r12, #(1 << 1)
		MCR		p15, 0, r12, c1, c0, 0
		bx		lr
		.endfunc
			
        .code 32
        .align 5
        .globl memcpy_ucl
        .func
/* a prefetch distance of 4 cache-lines works best experimentally */
#define CACHE_LINE_SIZE     32
memcpy_ucl:
        cmp         r2, #0
        bxle        lr
        pld         [r1]
        mov         r12, r0                 // back up r0 
        eor         r3, r1, r0
        tst         r3, #0x03               // temp data
        bne         use_neon                // src and dest cannot make align
        tst         r1, #0x03
        beq         aligned_cpy

make_align:
        ldrb        r3, [r1], #1
        subs        r2,  r2,  #1
        strb        r3, [r0], #1
        beq         out
        tst         r1, #0x03
        bne         make_align

aligned_cpy:
        cmp         r2, #68
        blt         less_64_nocheck
        tst         r1, #0x04
        beq         aligned_64
        ldr         r3, [r1], #4
        subs        r2, #4
        str         r3, [r0], #4               // align to 8 bytes 
        beq         out
aligned_64:
        subs        r2, #64
        pld         [r1, #32]
        blt         less_64
        pld         [r1, #64]
        pld         [r1, #96]
        .align  3
loop_main:
        vldmia      r1!, {q0 - q3}              // 8
        pld         [r1, #128]                  // 1
        pld         [r1, #160]                  // 1
        subs        r2,  #64                    // 1
        vstmia      r0!, {q0 - q3}              // 8
        bge         loop_main                   // 1  64 / 20 = 3.2 bytes/cycle

less_64:
        adds        r2,  #64
        beq         out
less_64_nocheck:
        cmp         r2,  #8 
        blt         less_8
loop_arm_8:
        vldmia      r1!, {d0}
        sub         r2,  #8
        cmp         r2,  #8
        vstmia      r0!, {d0}
        bge         loop_arm_8
less_8: 
        tst         r2,  #4
        ldrne       r3, [r1], #4
        strne       r3, [r0], #4
        tst         r2,  #2
        ldrneh      r3, [r1], #2
        strneh      r3, [r0], #2
        tst         r2,  #1
        ldrneb      r3, [r1], #1
        strneb      r3, [r0], #1
out:
        mov         r0, r12 
        bx          lr

use_neon:
        cmp         r2, #64
        blt         use_neon_less64
use_neon_loop:
        vld1.8      {q0, q1}, [r1]!
        vld1.8      {q2, q3}, [r1]!
        pld         [r1, #64]
        pld         [r1, #96]
        sub         r2, #64
        cmp         r2, #64
        vst1.8      {q0, q1}, [r0]!
        vst1.8      {q2, q3}, [r0]!
        bge         use_neon_loop
use_neon_less64:
        cmp         r2, #32
        blt         use_neon_less32
        vld1.8      {q0, q1}, [r1]!
        subs        r2, #32
        vst1.8      {q0, q1}, [r0]!
        beq         out
use_neon_less32:
        cmp         r2, #16
        blt         use_neon_less16
        vld1.8      {q0}, [r1]!
        subs        r2, #16
        vst1.8      {q0}, [r0]!
        beq         out 
use_neon_less16:
        cmp         r2, #8
        blt         less_8
        vld1.8      {d0}, [r1]!
        subs        r2, #8
        vst1.8      {d0}, [r0]!
        beq         out 
        b           less_8
		.endfunc

#if 0
        .code 32
        .align 5
        .globl bzero 
        .func

bzero:
    mov     r2, r1
    mov     r1, #0
	.endfunc

        .code 32
        .align 5
        .globl memset 
        .func
memset:
    and     r1, r1, #0xff
    cmp     r2, #0
    bxeq    lr
    orr     r1, r1, r1, lsl #8
    tst     r0, #7
    mov     r3, r0
    orr     r1, r1, r1, lsl #16
    beq     .Lmemset_align8
.Lmemset_make_align:
    strb    r1, [r3], #1
    subs    r2, r2, #1
    bxeq    lr
    tst     r3, #7
    bne     .Lmemset_make_align

.Lmemset_align8:
    cmp     r2, #16
    mov     r12, r1
    blt     .Lmemset_less16
    push    {r4, lr}
    mov     r4, r1
    mov     lr, r1
.Lmemset_loop32:
    subs    r2, r2, #32
    stmhsia r3!, {r1, r4, r12, lr}
    stmhsia r3!, {r1, r4, r12, lr}
    bhs     .Lmemset_loop32
    adds    r2, r2, #32
    popeq   {r4, pc}
    tst     r2, #16
    stmneia r3!, {r1, r4, r12, lr}
    pop     {r4, lr}
    subs    r2, #16
    bxeq    lr
.Lmemset_less16:
    movs    r2, r2, lsl #29
    stmcsia r3!, {r1, r12}
    strmi   r1, [r3], #4
    movs    r2, r2, lsl #2
    strcsh  r1, [r3], #2
    strmib  r1, [r3], #1
    bx      lr
	.endfunc
#endif
