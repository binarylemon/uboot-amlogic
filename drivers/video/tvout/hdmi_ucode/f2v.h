/*******************************************************************
 * 
 *  Copyright C 2007 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: 3/8/2007 11:18AM
 *
 *******************************************************************/
#ifndef F2V_H
#define F2V_H

typedef enum {
    F2V_IT2IT = 0,
    F2V_IB2IB,
    F2V_IT2IB,
    F2V_IB2IT,
    F2V_P2IT, 
    F2V_P2IB, 
    F2V_IT2P, 
    F2V_IB2P, 
    F2V_P2P,
    F2V_TYPE_MAX
} f2v_vphase_type_t;   /* frame to video conversion type */

typedef struct {
    unsigned char   rcv_num; //0~15
    unsigned char   rpt_num; // 0~3 
    unsigned short  phase;
//    signed   char   repeat_skip_chroma;
//    unsigned char   phase_chroma;
} f2v_vphase_t;


extern void f2v_get_vertical_phase(unsigned zoom_ratio, f2v_vphase_type_t type, unsigned char bank_length,
                            f2v_vphase_t *vphase);

#endif /* F2V_H */
