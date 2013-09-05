#ifndef _CANVAS_H_
#define _CANVAS_H_
#include "m6tv_mmc.h"

typedef struct  CAV_type {
   // word 1
   unsigned long      start_addr; //: 29;       // canvas start address.
   unsigned long      x_wrap_en ; //: 1;        // wrap mode in x direction.
   unsigned long      y_wrap_en ; //: 1;        // wrap mode in y direction.
   unsigned long      blk_mode  ; //: 2;        // blk_mode : 2 : 64x32; 1: 32x32; 0 : linear mode.
   unsigned long      cav_width ; //: 12;       // canvas width,  unit :  8bytes.      
   unsigned long      cav_hight ; //: 13;       // canvas hight 
} cav_con; 

typedef struct {
  unsigned long       ptr0       : 8;
  unsigned long       ptr1       : 8;
  unsigned long       ptr2       : 8;
  unsigned long       ptr3       : 8;

  unsigned long       ptr4       : 8;
  unsigned long       ptr5       : 8;
  unsigned long       ptr6       : 8;
  unsigned long       ptr7       : 8;

  unsigned long       ptr8       : 8;
  unsigned long       ptr9       : 8;
  unsigned long       ptr10      : 8;
  unsigned long       ptr11      : 8;

  unsigned long       ptr12      : 8;
  unsigned long       ptr13      : 8;
  unsigned long       ptr14      : 8;
  unsigned long       ptr15      : 8;
} CAV_LVL3_ptrs_S;

typedef struct {
  unsigned long       ptr0       : 32;
  unsigned long       ptr1       : 32;
  unsigned long       ptr2       : 32;
  unsigned long       ptr3       : 32;
} CAV_LVL3_ptr_regs_S;

typedef union {
  CAV_LVL3_ptrs_S     ptrs;
  CAV_LVL3_ptr_regs_S regs;
} CAV_LVL3_ptr_reg_U;

typedef struct CAV_LVL3_typ0 {               // 2 long 28 bit mode.
   unsigned long     lut_num0;
   cav_con         *lut_con0;
   unsigned long     lut_num1;
   cav_con         *lut_con1;
   unsigned long     grant_num;
   unsigned long     lvl3_addr; 
} cav_lvl3_typ0;

  typedef struct {           // nonCPU
    unsigned long  x       : 15;
    unsigned long  y       : 13;
    unsigned long  ptrIdx  : 7;
    unsigned long  modeSel : 1;
  } nonCPUAddr_S;

  typedef struct {           // CPU linear
    unsigned long  addr : 32;
    unsigned long  rsvd : 4;
  } CPULinearAddr_S;

  typedef struct {           // lvl3 mode 0
    unsigned long  x       : 15;
    unsigned long  y       : 13;
    unsigned long  ptrIdx  : 1;
    unsigned long  map     : 3;
    unsigned long  rsvd    : 3;
    unsigned long  modeSel : 1;
  } mode0Addr_S;

  typedef struct {           // lvl3 mode 1
    unsigned long  x       : 13;
    unsigned long  y       : 13;
    unsigned long  ptrIdx  : 3;
    unsigned long  map     : 3;
    unsigned long  rsvd    : 3;
    unsigned long  modeSel : 1;
  } mode1Addr_S;

  typedef struct {           // lvl3 mode 2
    unsigned long  x       : 15;
    unsigned long  y       : 13;
    unsigned long  ptrIdx  : 1;
    unsigned long  map     : 3;
    unsigned long  rsvd    : 3;
    unsigned long  modeSel : 1;
  } mode2Sub0Addr_S;

  typedef struct { 
    unsigned long  x            : 13;
    unsigned long  y            : 13;
    unsigned long  ptrIdx       : 2;
    unsigned long  ptr0_modeSel : 1;
    unsigned long  map          : 3;
    unsigned long  rsvd         : 3;
    unsigned long  modeSel      : 1;
  } mode2Sub1Addr_S;

  typedef struct {           // lvl3 mode 3 
    unsigned long  x       : 15;
    unsigned long  y       : 13;
    unsigned long  ptrIdx  : 1;
    unsigned long  map     : 3;
    unsigned long  rsvd    : 3;
    unsigned long  modeSel : 1;
  } mode3Sub0Addr_S;

  typedef struct { 
    unsigned long  x            : 13;
    unsigned long  y            : 12;
    unsigned long  ptrIdx       : 3;
    unsigned long  ptr0_modeSel : 1;
    unsigned long  map          : 3;
    unsigned long  rsvd         : 3;
    unsigned long  modeSel      : 1;
  } mode3Sub1Addr_S;

  typedef struct { 
    unsigned long  x       : 14;
    unsigned long  y       : 11;
    unsigned long  ptrIdx  : 3;
    unsigned long  ptr0_modeSel : 1;
    unsigned long  map     : 3;
    unsigned long  rsvd    : 3;
    unsigned long  modeSel : 1;
  } mode3Sub2Addr_S;

  typedef struct {           // lvl3 mode 4 
    unsigned long  x            : 13;
    unsigned long  y            : 13;
    unsigned long  ptrIdx       : 2;
    unsigned long  ptr0_modeSel : 1;
    unsigned long  map          : 3;
    unsigned long  rsvd         : 3;
    unsigned long  modeSel      : 1;
  } mode4Sub0Addr_S;

  typedef struct { 
    unsigned long x       : 13;
    unsigned long y       : 12;
    unsigned long ptrIdx  : 3;
    unsigned long ptr0_modeSel : 1;
    unsigned long map     : 3;
    unsigned long rsvd    : 3;
    unsigned long modeSel : 1;
  } mode4Sub1Addr_S;

  typedef struct { 
    unsigned long  x            : 14;
    unsigned long  y            : 11;
    unsigned long  ptrIdx       : 3;
    unsigned long  ptr0_modeSel : 1;
    unsigned long  map          : 3;
    unsigned long  rsvd         : 3;
    unsigned long  modeSel      : 1;
  } mode4Sub2Addr_S;

  typedef struct {           // lvl3 mode 5 
    unsigned long x       : 13;
    unsigned long y       : 12;
    unsigned long ptrIdx  : 4;
    unsigned long map     : 3;
    unsigned long rsvd    : 3;
    unsigned long modeSel : 1;
  } mode5Sub0Addr_S;

  typedef struct { 
    unsigned long x       : 14;
    unsigned long y       : 11;
    unsigned long ptrIdx  : 4;
    unsigned long map     : 3;
    unsigned long rsvd    : 3;
    unsigned long modeSel : 1;
  } mode5Sub1Addr_S;

  typedef struct {
    unsigned long long  addr : 36;
  } ddrAddr_S;

  typedef union {
    ddrAddr_S           addr;
    nonCPUAddr_S        nonCPU;
    CPULinearAddr_S     cpuLinear;
    mode0Addr_S         lvl3Mode0;
    mode1Addr_S         lvl3Mode1;
    mode2Sub0Addr_S     lvl3Mode2Sub0;
    mode2Sub1Addr_S     lvl3Mode2Sub1;
    mode3Sub0Addr_S     lvl3Mode3Sub0;
    mode3Sub1Addr_S     lvl3Mode3Sub1;
    mode3Sub2Addr_S     lvl3Mode3Sub2;
    mode4Sub0Addr_S     lvl3Mode4Sub0;
    mode4Sub1Addr_S     lvl3Mode4Sub1;
    mode4Sub2Addr_S     lvl3Mode4Sub2;
    mode5Sub0Addr_S     lvl3Mode5Sub0;
    mode5Sub1Addr_S     lvl3Mode5Sub1;
  } ddrAddr_U;

extern int config_cav_lut( int lut_number, cav_con cav_lut);

#endif
