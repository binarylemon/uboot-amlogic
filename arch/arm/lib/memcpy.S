/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//#include <machine/cpu-features.h>
//#include <machine/asm.h>

        .code 32
        .align 5
        .globl memcpy
        .func
/* a prefetch distance of 4 cache-lines works best experimentally */
#define CACHE_LINE_SIZE     32
memcpy:
        .fnstart
        .save       {r0, lr}
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
