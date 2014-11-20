
	.code 32
	.align 5
	.globl ucl_adler32_neon 
	.func

	/*
	 * unsigned char *ucl_adler32_neon(unsigned char *buf, unsigned int *s1, unsigned int *s2, int k)
	 *
	 * #define UCL_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
	 * #define UCL_DO2(buf,i)  UCL_DO1(buf,i); UCL_DO1(buf,i+1);
	 * #define UCL_DO4(buf,i)  UCL_DO2(buf,i); UCL_DO2(buf,i+2);
	 * #define UCL_DO8(buf,i)  UCL_DO4(buf,i); UCL_DO4(buf,i+4);
	 * #define UCL_DO16(buf,i) UCL_DO8(buf,i); UCL_DO8(buf,i+8);
     * do {
     *     UCL_DO16(buf,0);
     *     buf += 16;
     *     k -= 16;
     * } while (k >= 16);
	 */
ucl_adler32_neon:
	/*
	 * Algorithm:
	 * buf: |x0|x1|x2|x3|x4|x5|x6|....|x15|
	 * init s1 s2: s10 s20
	 * Steps	s1 result			s2 result
	 * 1		s10+x0				s20+s10+x0
	 * 2		s10+x0+x1			s20+s10+x0 + s10+x0+x1 = s20+2*s10+2*x0+x1
	 * 3		s10+x0+x1+x2		s20+2*s10+2*x0+x1 + s10+x0+x1+x2 = s20+3*s10+3*x0+2*x1+x2
	 * ...
	 * 16		s10+x0+x1+...x15	s20+16*s10+16*x0+15*x1+14*x2+...+2*x14+x15
	 * ...
	 * 32		s10+sum(x0...x31)	s20+31*s10+32*x0+31*x1+...+x31 = s20
	 */
	ldr			r12, =.Lmul_table
	vldm		r12, {q15}
	vmov.i8		d28, #0
	vmov.i8		q0,  #0
	push		{lr}
	ldr			lr, [r1]								// lr  = s1
	ldr			r12, [r2]								// r12 = s2
	bic			r3, r3, #0xf							// clear 
	mla			r12, lr, r3, r12						// r12 = s2 + s1*(k&~0xf)
.Lloop:
	vld1.8		{d6, d7}, [r0]!
	vshl.i32	d12, d0, #4								// sum(f..0) * 16
	sub			r3, r3, #16								// k -= 16
	vmull.u8	q2, d6, d30						
	vmull.u8	q4, d7, d31						
	vadd.i32	d1, d1, d12								// s2 += 16*sum(f...0)
	vpaddl.u8	q1, q3									// q1 = |x15+x14|x13+x12|...|x3+x2|x1+x0|
	cmp			r3, #16									//
	vpadd.i16	d4, d4, d5								// d4 = |f+2*e|...|13*3+14*2+15*1+16*0|
	vpadd.i16	d8, d8, d9								//
	vpadd.i16	d2, d2, d3								// d2 = |f+e+d+c|b+a+9+8|7+6+5+4|3+2+1+0|
	vpadd.i16	d4, d4, d8								// d4 = |f+2*e|...|13*3+14*2+15*1+16*0|
	vpadd.i16	d2, d2, d28								// d2 = |0|0|f+e+d+..+8|7+6+..+0|.16
	vpadd.i16	d4, d4, d28								//
	vpadal.u16	d0, d2									// d0 = |0|f+e+d+..+1+0|.32
	vpadal.u16	d1, d4									// d1 = |0|f+2*e+3*d+..+15*1+16*0|.32 
	bge			.Lloop

	vmov.32		r0,  d0[0]							//
	vmov.32		r3,  d1[0]							//
	add			lr,  r0, lr
	add			r12, r3, r12
	str			lr,  [r1]
	str			r12, [r2]
	pop			{pc}	


.Lmul_table:									
	.word		0x0d0e0f10						
	.word		0x090a0b0c						
	.word		0x05060708						
	.word		0x01020304						
