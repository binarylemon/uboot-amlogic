
        .code 32
        .align 5
        .globl bcopy_neon_align 
        .func
bcopy_neon_align:
    pld        [r1, #-64]                  
    tst        r1, #0x03                   
    beq        .Lbbcopy_aligned                

.Lbbcopy_make_align:                           
    ldrb       r12, [r1, #-1]!             
    subs       r2, r2, #1        
    strb       r12, [r0, #-1]!             
    beq        .Lbbcopy_out                    
    tst        r1, #0x03                   
    bne        .Lbbcopy_make_align             

.Lbbcopy_aligned:                              
    cmp        r2, #64                  
    blt        .Lbbcopy_align_less_64          
.Lbbcopy_align_loop64:                         
    vldmdb     r1!, {q0 - q3}              
    sub        r2, r2, #64       
    cmp        r2, #64                  
    pld        [r1, #-64]                  
    pld        [r1, #-96]                  
    vstmdb     r0!, {q0 - q3}              
    bge        .Lbbcopy_align_loop64           
    cmp        r2, #0                   
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_64:                        
    cmp        r2, #32                  
    blt        .Lbbcopy_align_less_32          
    vldmdb     r1!, {q0 - q1}              
    subs       r2, r2, #32       
    vstmdb     r0!, {q0 - q1}              
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_32:                        
    cmp        r2, #16                  
    blt        .Lbbcopy_align_less_16          
    vldmdb     r1!, {q0}                   
    subs       r2, r2, #16       
    vstmdb     r0!, {q0}                   
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_16:                        
    cmp        r2, #8                   
    blt        .Lbbcopy_align_less_8           
    vldmdb     r1!, {d0}                   
    subs       r2, r2, #8        
    vstmdb     r0!, {d0}                   
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_8:                         
    cmp        r2, #4                   
    blt        .Lbbcopy_align_less_4           
    ldr        r12, [r1, #-4]!             
    subs       r2, r2, #4        
    str        r12, [r0, #-4]!             
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_4:                         
    cmp        r2, #2                   
    blt        .Lbbcopy_align_less_2           
    ldrh       r12, [r1, #-2]!             
    subs       r2, r2, #2        
    strh       r12, [r0, #-2]!             
    beq        .Lbbcopy_out                    

.Lbbcopy_align_less_2:                         
    ldrb       r12, [r1, #-1]!             
    strb       r12, [r0, #-1]!             

.Lbbcopy_out:                                  
	bx			lr
	.endfunc

    .code 32
    .align 5
    .globl bcopy_neon_not_align 
    .func
bcopy_neon_not_align:
    cmp        r2, #64                  
    pld        [r1, #-32]                  
    blt        .Lbbcopy___less_64              
    mov        r12, #-32                       
    sub        r1, r1, #32             
    sub        r0, r0, #32             
.Lbbcopy___loop64:                             
    vld1.8     {q0 - q1}, [r1], r12        
    vld1.8     {q2 - q3}, [r1], r12        
    sub        r2, r2, #64       
    cmp        r2, #64                  
    pld        [r1, #-64]                  
    pld        [r1, #-96]                  
    vst1.8     {q0 - q1}, [r0], r12        
    vst1.8     {q2 - q3}, [r0], r12        
    bge        .Lbbcopy___loop64               
    cmp        r2, #0                   
    beq        .Lbcopy_out                     
    add        r1, r1, #32             
    add        r0, r0, #32             

.Lbbcopy___less_64:                            
    cmp        r2, #32                  
    blt        .Lbbcopy___less_32              
    sub        r1, r1, #32             
    sub        r0, r0, #32             
    vld1.8     {q0 - q1}, [r1]             
    subs       r2, r2, #32       
    vst1.8     {q0 - q1}, [r0]             
    beq        .Lbcopy_out                     

.Lbbcopy___less_32:                            
    cmp        r2, #16                  
    blt        .Lbbcopy___less_16              
    sub        r1, r1, #16             
    sub        r0, r0, #16             
    vld1.8     {q0}, [r1]                  
    subs       r2, r2, #16       
    vst1.8     {q0}, [r0]                  
    beq        .Lbcopy_out                     

.Lbbcopy___less_16:                            
    cmp        r2, #8                   
    blt        .Lbbcopy___less_8               
    sub        r1, r1, #8              
    sub        r0, r0, #8              
    vld1.8     {d0}, [r1]                  
    subs       r2, r2, #8        
    vst1.8     {d0}, [r0]                  
    beq        .Lbcopy_out                     

.Lbbcopy___less_8:                             
    cmp        r2, #4                   
    blt        .Lbbcopy___less_4               
    ldr        r12, [r1, #-4]!             
    subs       r2, r2, #4        
    str        r12, [r0, #-4]!             
    beq        .Lbcopy_out                     

.Lbbcopy___less_4:                             
    cmp        r2, #2                   
    blt        .Lbbcopy___less_2               
    ldrh       r12, [r1, #-2]!             
    subs       r2, r2, #2        
    strh       r12, [r0, #-2]!             
    beq        .Lbcopy_out                     

.Lbbcopy___less_2:                             
    ldrb       r12, [r1, #-1]!             
    strb       r12, [r0, #-1]!             

.Lbcopy_out:                                   
	bx			lr

	.endfunc
