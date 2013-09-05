
//FILE: f2v.c
/*******************************************************************
 *
 *  Copyright C 2007 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description:
 *
 *  Author: Amlogic Software
 *  Created: 3/8/2007 11:23AM
 *
 *******************************************************************/
#include "f2v.h"

#define Wr(addr, data) *(volatile unsigned long *)(0x01100000|(addr<<2))=data

#define ZOOM_BITS       20
#define PHASE_BITS      16

static const unsigned char f2v_420_in_pos_luma[F2V_TYPE_MAX]   = {0,2,0,2,0,0,0,2,0};
//static const unsigned char f2v_420_in_pos_chroma[F2V_TYPE_MAX] = {1,5,1,5,2,2,1,5,2};
static const unsigned char f2v_420_out_pos[F2V_TYPE_MAX]       = {0,2,2,0,0,2,0,0,0};


void f2v_get_vertical_phase(unsigned zoom_ratio, f2v_vphase_type_t type, unsigned char bank_length,
                            f2v_vphase_t *vphase)
{
    int offset_in, offset_out;
    
    /* luma */
    offset_in = f2v_420_in_pos_luma[type] << PHASE_BITS;
    offset_out = (f2v_420_out_pos[type] * zoom_ratio) >> (ZOOM_BITS - PHASE_BITS);
#if 0
    Wr(0xfe0, offset_in);
    Wr(0xfe1, offset_out);
#endif
    
    vphase->rcv_num = bank_length;
    if (bank_length == 4 || bank_length == 3)
       vphase->rpt_num = 1;     
    else
       vphase->rpt_num = 0;     

    if (offset_in > offset_out) {
        vphase->rpt_num = vphase->rpt_num + 1;     
        vphase->phase = 
            ((4 << PHASE_BITS) + offset_out - offset_in) >> 2;
    }
    else {
        while ((offset_in + (4 << PHASE_BITS)) <= offset_out) {
            if (vphase->rpt_num == 1)
               vphase->rpt_num = 0;
            else
               vphase->rcv_num++;            
            offset_in += 4 << PHASE_BITS;
#if 0
            Wr(0xfe1, offset_in);
#endif
        }
        vphase->phase = (offset_out - offset_in) >> 2;
#if 0
        Wr(0xfe2, vphase->phase);
#endif
    }
}

