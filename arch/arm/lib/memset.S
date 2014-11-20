/* Copyright (c) 2010-2011, Linaro Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

      * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

      * Neither the name of Linaro Limited nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Written by Dave Gilbert <david.gilbert@linaro.org>
   Adapted to Bionic by Bernhard Rosenkraenzer <Bernhard.Rosenkranzer@linaro.org>

   This memset routine is optimised on a Cortex-A9 and should work on
   all ARMv7 processors. */

	.syntax unified
	.arch armv7-a


@ 2011-08-30 david.gilbert@linaro.org
@    Extracted from local git 2f11b436

@ this lets us check a flag in a 00/ff byte easily in either endianness
@ ---------------------------------------------------------------------------
    .code   32
	.func
	.global	bzero
	.p2align 4,,15

bzero:
    mov     r2, r1
    mov     r1, #0
	.endfunc

    .code   32
	.func
	.global memset	
	.p2align 4,,15
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
    strhcs  r1, [r3], #2
    strbmi  r1, [r3], #1
    bx      lr
	.endfunc
