
//FILE: canvas.c
#include "register.h"
#include "canvas.h"
#include "m6tv_mmc.h"

int config_cav_lut( int lut_number, cav_con cav_lut) { 
   unsigned long datal_temp;
   unsigned long datah_temp;
   datal_temp = (cav_lut.start_addr & 0x1fffffff) | ((cav_lut.cav_width & 0x7 ) << 29 );
   datah_temp = ((cav_lut.cav_width  >> 3) & 0x1ff)   | (( cav_lut.cav_hight & 0x1fff) <<9 )  | ((cav_lut.x_wrap_en & 1) << 22 ) | 
                (( cav_lut.y_wrap_en & 1) << 23) | (( cav_lut.blk_mode & 0x3) << 24); 
   MMC_Wr(P_DC_CAV_LUT_DATAL, datal_temp);
   MMC_Wr(P_DC_CAV_LUT_DATAH, datah_temp);
   MMC_Wr(P_DC_CAV_LUT_ADDR,  lut_number | 0x200);
   // check the write is in.
   //     APB_Wr(DC_CAV_LUT_DATAL, 0); 
   //     APB_Wr(DC_CAV_LUT_DATAH, 0); 

//   APB_Wr(DC_CAV_LUT_ADDR,  lut_number | 0x100);
//   while ( APB_Rd(DC_CAV_LUT_ADDR) & 0x100 ) {} ;
//   if ( (datal_temp !=  APB_Rd(DC_CAV_LUT_DATAL)) || 
//        (datah_temp !=  APB_Rd(DC_CAV_LUT_DATAH)) ) {
//       return 1;
//   } else {
       return 0;
//   }
};

