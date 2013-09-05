#include "register.h" 
#include "tv_enc.h"
#include "c_stimulus.h"

#define NO_ENCT

#ifdef NO_ENCT
#define VIU1_SEL_ENCT 0
#define VIU2_SEL_ENCT 0
#else
#define VIU1_SEL_ENCT 3
#define VIU2_SEL_ENCT 3
#endif

void venc_set_lcd(int w, int h, int w1, int h1)
{
        int havon_begin = 80;

    Wr(VPU_VIU_VENC_MUX_CTRL,
       (VIU1_SEL_ENCT<<0) |    // viu1 select enct
       (VIU2_SEL_ENCT<<2)      // viu2 select enct
       );

	Wr(	ENCT_VIDEO_MODE				,0       );
 	Wr(	ENCT_VIDEO_MODE_ADV			,0x0418		);	

	Wr(	ENCT_VIDEO_MAX_PXCNT			,w1-1       );
	Wr(	ENCT_VIDEO_MAX_LNCNT			,h1-1       );

	Wr(	ENCT_VIDEO_HAVON_BEGIN			,0       +havon_begin      );
	Wr(	ENCT_VIDEO_HAVON_END			,w -1        +havon_begin      );
	Wr(	ENCT_VIDEO_VAVON_BLINE			,0       +10      );
	Wr(	ENCT_VIDEO_VAVON_ELINE			,h -1        +10      );

	Wr(	ENCT_VIDEO_HSO_BEGIN			,5         );
	Wr(	ENCT_VIDEO_HSO_END				,10         );
	Wr(	ENCT_VIDEO_VSO_BEGIN			,5         );
	Wr(	ENCT_VIDEO_VSO_END				,10         );
	Wr(	ENCT_VIDEO_VSO_BLINE			,2         );
	Wr(	ENCT_VIDEO_VSO_ELINE			,4         );

 	// bypass filter
 	Wr(	ENCT_VIDEO_FILT_CTRL			,0x1000		);
}

void venc_set_lvds(int w, int h, int w1, int h1)
{
        int havon_begin = 80;

    Wr(VPU_VIU_VENC_MUX_CTRL,
       (0<<0) |    // viu1 select encl
       (0<<2)      // viu2 select encl
       );
	Wr(	ENCL_VIDEO_MODE				,0       );
 	Wr(	ENCL_VIDEO_MODE_ADV			,0x0418		);	

	Wr(	ENCL_VIDEO_MAX_PXCNT			,w1-1       );
	Wr(	ENCL_VIDEO_MAX_LNCNT			,h1-1       );

	Wr(	ENCL_VIDEO_HAVON_BEGIN			,0       +havon_begin      );
	Wr(	ENCL_VIDEO_HAVON_END			,w -1        +havon_begin      );
	Wr(	ENCL_VIDEO_VAVON_BLINE			,0       +10      );
	Wr(	ENCL_VIDEO_VAVON_ELINE			,h -1        +10      );

	Wr(	ENCL_VIDEO_HSO_BEGIN			,5         );
	Wr(	ENCL_VIDEO_HSO_END				,10         );
	Wr(	ENCL_VIDEO_VSO_BEGIN			,5         );
	Wr(	ENCL_VIDEO_VSO_END				,10         );
	Wr(	ENCL_VIDEO_VSO_BLINE			,2         );
	Wr(	ENCL_VIDEO_VSO_ELINE			,4         );

 	// bypass filter
 	Wr(	ENCL_VIDEO_FILT_CTRL			,0x1000		);
} /* venc_set_lvds */

void venc_set_lvds_and_sel_viu (int w, int h, int w1, int h1, int viu1_sel, int viu2_sel)
{
    int havon_begin = 80;

    if (viu1_sel) { // 1=Connect to ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 1=Connect to ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
    }

    Wr( ENCL_VIDEO_MODE             ,0       );
    Wr( ENCL_VIDEO_MODE_ADV         ,0x0418     );  

    Wr( ENCL_VIDEO_MAX_PXCNT            ,w1-1       );
    Wr( ENCL_VIDEO_MAX_LNCNT            ,h1-1       );

    Wr( ENCL_VIDEO_HAVON_BEGIN          ,0       +havon_begin      );
    Wr( ENCL_VIDEO_HAVON_END            ,w -1        +havon_begin      );
    Wr( ENCL_VIDEO_VAVON_BLINE          ,0       +10      );
    Wr( ENCL_VIDEO_VAVON_ELINE          ,h -1        +10      );

    Wr( ENCL_VIDEO_HSO_BEGIN            ,5         );
    Wr( ENCL_VIDEO_HSO_END              ,10         );
    Wr( ENCL_VIDEO_VSO_BEGIN            ,5         );
    Wr( ENCL_VIDEO_VSO_END              ,10         );
    Wr( ENCL_VIDEO_VSO_BLINE            ,2         );
    Wr( ENCL_VIDEO_VSO_ELINE            ,4         );

    // bypass filter
    Wr( ENCL_VIDEO_FILT_CTRL            ,0x1000     );
} /* venc_set_lvds_and_sel_viu */

void config_tv_enc_lcd( tv_enc_lcd_type_t output_type )
{
   
    switch(output_type) {

        case TV_ENC_LCD240x160:

	        Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
 	        Wr(	ENCL_VIDEO_MODE_ADV             ,0x0018		);	

	        Wr(	ENCL_VIDEO_MAX_PXCNT            ,367        );
	        Wr(	ENCL_VIDEO_MAX_LNCNT            ,187        );

	        Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   +50    );
	        Wr(	ENCL_VIDEO_HAVON_END            ,239 +50    );
	        Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   +20    );
	        Wr(	ENCL_VIDEO_VAVON_ELINE          ,159 +20    );

	        Wr(	ENCL_VIDEO_HSO_BEGIN            ,15         );
	        Wr(	ENCL_VIDEO_HSO_END              ,31         );
	        Wr(	ENCL_VIDEO_VSO_BEGIN            ,15         );
	        Wr(	ENCL_VIDEO_VSO_END              ,31         );
	        Wr(	ENCL_VIDEO_VSO_BLINE            ,10         );
	        Wr(	ENCL_VIDEO_VSO_ELINE            ,12         );

            // bypass filter
 	        Wr(	ENCL_VIDEO_FILT_CTRL            ,0x1000		);
            break;
        case TV_ENC_LCD240x160_dsi:
        case TV_ENC_LCD240x160_slow:

	        Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
 	        Wr(	ENCL_VIDEO_MODE_ADV             ,0x0018		);	

	        Wr(	ENCL_VIDEO_MAX_PXCNT            ,240+160-1  );  // H_ACTIVE+H_BLANK-1=399
	        Wr(	ENCL_VIDEO_MAX_LNCNT            ,160+35-1   );  // V_ACTIVE+V_BLANK-1=194

	        Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   +160-3 );  // 0+H_BLANK-1-2=157    // -2 is for extra 2-cycle delay for AVON and data to mipi_dsi input
	        Wr(	ENCL_VIDEO_HAVON_END            ,239 +160-3 );  // H_ACTIVE-1+H_BLANK-1-2=396
	        Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   +35    );  // 0+V_BLANK=35
	        Wr(	ENCL_VIDEO_VAVON_ELINE          ,159 +35    );  // V_ACTIVE-1+V_BLANK=194

	        Wr(	ENCL_VIDEO_HSO_BEGIN            ,48-1       );  // H_FRONT_PORCH-1=47
	        Wr(	ENCL_VIDEO_HSO_END              ,48-1+32    );  // H_FRONT_PORCH-1+H_SYNC=79
	        Wr(	ENCL_VIDEO_VSO_BEGIN            ,48-1       );  // H_FRONT_PORCH-1=47
	        Wr(	ENCL_VIDEO_VSO_END              ,48-1       );  // H_FRONT_PORCH-1=47
	        Wr(	ENCL_VIDEO_VSO_BLINE            ,3          );  // V_FRONT_PORCH=3
	        Wr(	ENCL_VIDEO_VSO_ELINE            ,9          );  // V_SYNC=9

            // bypass filter
 	        Wr(	ENCL_VIDEO_FILT_CTRL            ,0x1000		);
            break;
        case TV_ENC_LCD240x160_dsi36b:  // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
	        Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
 	        Wr(	ENCL_VIDEO_MODE_ADV             ,0x0019		);  // 0x0018 | 0x1

	        Wr(	ENCL_VIDEO_MAX_PXCNT            ,735        );  // (367+1)*2 - 1
	        Wr(	ENCL_VIDEO_MAX_LNCNT            ,187        );

	        Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   +101   );  // 0 + (50*2+1)
	        Wr(	ENCL_VIDEO_HAVON_END            ,479 +101   );  // (240*2-1) + (50*2+1)
	        Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   +20    );
	        Wr(	ENCL_VIDEO_VAVON_ELINE          ,159 +20    );

	        Wr(	ENCL_VIDEO_HSO_BEGIN            ,31         );  // 15*2+1
	        Wr(	ENCL_VIDEO_HSO_END              ,63         );  // 31*2+1
	        Wr(	ENCL_VIDEO_VSO_BEGIN            ,31         );  // 15*2+1
	        Wr(	ENCL_VIDEO_VSO_END              ,63         );  // 31*2+1
	        Wr(	ENCL_VIDEO_VSO_BLINE            ,10         );
	        Wr(	ENCL_VIDEO_VSO_ELINE            ,12         );

            // bypass filter
 	        Wr(	ENCL_VIDEO_FILT_CTRL            ,0x1000		);
            break;

        case TV_ENC_LCD480x234:
            Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
            Wr(	ENCL_VIDEO_MODE_ADV             ,0x0418		);	

            Wr(	ENCL_VIDEO_MAX_PXCNT            ,607        );
            Wr(	ENCL_VIDEO_MAX_LNCNT            ,261        );

            Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   +50    );
            Wr(	ENCL_VIDEO_HAVON_END            ,479 +50    );
            Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   +20    );
            Wr(	ENCL_VIDEO_VAVON_ELINE          ,233 +20    );

            Wr(	ENCL_VIDEO_HSO_BEGIN            ,15         );
            Wr(	ENCL_VIDEO_HSO_END              ,31         );
            Wr(	ENCL_VIDEO_VSO_BEGIN            ,15         );
            Wr(	ENCL_VIDEO_VSO_END              ,31         );
            Wr(	ENCL_VIDEO_VSO_BLINE            ,10         );
            Wr(	ENCL_VIDEO_VSO_ELINE            ,12         );

            // bypass filter
            Wr(	ENCL_VIDEO_FILT_CTRL            ,0x1000		);
            break;
        case TV_ENC_LCD480x234_dsi36b:  // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
            Wr(	ENCL_VIDEO_MODE_ADV             ,0x0419     );  // 0x0418 | 0x1

            Wr(	ENCL_VIDEO_MAX_PXCNT            ,1215       );  // (607+1)*2 - 1
            Wr(	ENCL_VIDEO_MAX_LNCNT            ,261        );

            Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   +101   );  // 0 + (50*2+1)
            Wr(	ENCL_VIDEO_HAVON_END            ,959 +101   );  // (480*2-1) + (50*2+1)
            Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   +20    );
            Wr(	ENCL_VIDEO_VAVON_ELINE          ,233 +20    );

            Wr(	ENCL_VIDEO_HSO_BEGIN            ,31         );  // 15*2+1
            Wr(	ENCL_VIDEO_HSO_END              ,63         );  // 31*2+1
            Wr(	ENCL_VIDEO_VSO_BEGIN            ,31         );  // 15*2+1
            Wr(	ENCL_VIDEO_VSO_END              ,63         );  // 31*2+1
            Wr(	ENCL_VIDEO_VSO_BLINE            ,10         );
            Wr(	ENCL_VIDEO_VSO_ELINE            ,12         );

            // bypass filter
            Wr(	ENCL_VIDEO_FILT_CTRL            ,0x1000		);
            break;
       case TV_ENC_LCD640x480:  //-------------------------------------add
	        Wr(	ENCL_VIDEO_MODE                 ,0x0040     );
            Wr(	ENCL_VIDEO_MODE_ADV             ,0x0018     );  

            Wr(	ENCL_VIDEO_MAX_PXCNT            , 800      );  
            Wr(	ENCL_VIDEO_MAX_LNCNT            , 525     );

            Wr(	ENCL_VIDEO_HAVON_BEGIN          ,0   + 50  );  
			Wr(	ENCL_VIDEO_HAVON_END            ,639 + 50  );  
            Wr(	ENCL_VIDEO_VAVON_BLINE          ,0   + 20   );
            Wr(	ENCL_VIDEO_VAVON_ELINE          ,479 + 20   );
           
		    Wr(	ENCL_VIDEO_HSO_BEGIN            ,40         );
            Wr(	ENCL_VIDEO_HSO_END              ,729        );
            Wr(	ENCL_VIDEO_VSO_BEGIN            ,488         );
            Wr(	ENCL_VIDEO_VSO_END              ,507         );
            Wr(	ENCL_VIDEO_VSO_BLINE            ,2         );
            Wr(	ENCL_VIDEO_VSO_ELINE            ,8         );
  
            break;
	   case TV_ENC_LCD720x480:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0008);
            
            Wr(ENCL_VIDEO_MAX_PXCNT,        857);
            Wr(ENCL_VIDEO_MAX_LNCNT,        524);
            
            Wr(ENCL_VIDEO_HAVON_BEGIN,      80);
            Wr(ENCL_VIDEO_HAVON_END,        799);
            Wr(ENCL_VIDEO_VAVON_BLINE,      11);
            Wr(ENCL_VIDEO_VAVON_ELINE,      490);
            
            Wr(ENCL_VIDEO_HSO_BEGIN,	    18);
            Wr(ENCL_VIDEO_HSO_END, 		    840);
            Wr(ENCL_VIDEO_VSO_BEGIN,	    830);
            Wr(ENCL_VIDEO_VSO_END, 		    848);
            
            Wr(ENCL_VIDEO_VSO_BLINE,        3);
            Wr(ENCL_VIDEO_VSO_ELINE,        5);
            break;
        case TV_ENC_LCD720x480_dsi36b:  // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD720x480_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD720x576:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0018);

            //Wr(ENCL_VIDEO_HAVON_BEGIN, 248);//264);//231);   //7);karlson edit
            //Wr(ENCL_VIDEO_HAVON_END, 1688);  //1672);
            Wr(ENCL_VIDEO_HAVON_BEGIN, 124);
            Wr(ENCL_VIDEO_HAVON_END, 843);
             
            Wr(ENCL_VIDEO_VAVON_BLINE, 44);
            Wr(ENCL_VIDEO_VAVON_ELINE, 624);   //22);   //19);karlson edit
            Wr(ENCL_VIDEO_VSO_BLINE, 11);     //19);karlson edit,haunghonglin 19-->15 for bug#
            Wr(ENCL_VIDEO_VSO_ELINE, 16);
            
            //Wr(ENCL_VIDEO_MAX_PXCNT, 1727);
            Wr(ENCL_VIDEO_MAX_PXCNT, 863);
            Wr(ENCL_VIDEO_MAX_LNCNT, 624);

            Wr(ENCL_VIDEO_HSO_BEGIN, 0x3);
            Wr(ENCL_VIDEO_HSO_END, 0x5);
            Wr(ENCL_VIDEO_VSO_BEGIN, 0x3);
            Wr(ENCL_VIDEO_VSO_END, 0x5);
          break;
        case TV_ENC_LCD720x576_dsi36b:  // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD720x576_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD1280x720:
            Wr(ENCL_VIDEO_MODE,                 0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,             0x0018);    
            
            Wr(ENCL_VIDEO_MAX_PXCNT,            1649);
            //Wr(ENCL_VIDEO_HAVON_BEGIN,          519);
            //Wr(ENCL_VIDEO_HAVON_END,            3078);
            Wr(ENCL_VIDEO_HAVON_BEGIN,          257);
            Wr(ENCL_VIDEO_HAVON_END,            1536);
            Wr(ENCL_VIDEO_VAVON_BLINE,          25);
            Wr(ENCL_VIDEO_VAVON_ELINE,          744);            
            //Wr(ENCL_VIDEO_HSO_BEGIN,            15);
            //Wr(ENCL_VIDEO_HSO_END,              31);
            Wr(ENCL_VIDEO_HSO_BEGIN,            7);
            Wr(ENCL_VIDEO_HSO_END,              15);

            Wr(ENCL_VIDEO_VSO_BEGIN,            15);
            Wr(ENCL_VIDEO_VSO_END,              31);
            Wr(ENCL_VIDEO_VSO_BLINE,            19);
            Wr(ENCL_VIDEO_VSO_ELINE,            21);
            Wr(ENCL_VIDEO_MAX_LNCNT,            749);
            break;
        case TV_ENC_LCD1280x720_dsi36b: // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD1280x720_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD1920x1080:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0018);
            
            Wr(ENCL_VIDEO_MAX_PXCNT,        2199);
            
            Wr(ENCL_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCL_VIDEO_HAVON_END,        2067);
            Wr(ENCL_VIDEO_VAVON_BLINE,      41);
            Wr(ENCL_VIDEO_VAVON_ELINE,      1120);
            
            Wr(ENCL_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCL_VIDEO_HSO_END, 		    2156);
            Wr(ENCL_VIDEO_VSO_BEGIN,	    2100);
            Wr(ENCL_VIDEO_VSO_END, 		    2164);
            
            Wr(ENCL_VIDEO_VSO_BLINE,        3);
            Wr(ENCL_VIDEO_VSO_ELINE,        5);
            Wr(ENCL_VIDEO_MAX_LNCNT,        1124);
            break;
        case TV_ENC_LCD1920x1080_dsi36b:    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD1920x1080_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD1920x2205:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0008);
        
            Wr(ENCL_VIDEO_MAX_PXCNT,        2749);
            
            Wr(ENCL_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCL_VIDEO_HAVON_END,        2067);
            Wr(ENCL_VIDEO_VAVON_BLINE,      41);
            Wr(ENCL_VIDEO_VAVON_ELINE,      2245);
            
            Wr(ENCL_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCL_VIDEO_HSO_END, 		    2156);
            Wr(ENCL_VIDEO_VSO_BEGIN,	    2100);
            Wr(ENCL_VIDEO_VSO_END, 		    2164);
            
            Wr(ENCL_VIDEO_VSO_BLINE,        3);
            Wr(ENCL_VIDEO_VSO_ELINE,        5);
            Wr(ENCL_VIDEO_MAX_LNCNT,        2249);
            break;
        case TV_ENC_LCD1920x2205_dsi36b:    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD1920x2205_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD2560x1600:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0008);
        
            Wr(ENCL_VIDEO_MAX_PXCNT,        2719);
            
            Wr(ENCL_VIDEO_HAVON_BEGIN,      0   +139);
            Wr(ENCL_VIDEO_HAVON_END,        2559+139);
            Wr(ENCL_VIDEO_VAVON_BLINE,      0   +43);
            Wr(ENCL_VIDEO_VAVON_ELINE,      1599+43);
            
            Wr(ENCL_VIDEO_HSO_BEGIN,	    27);
            Wr(ENCL_VIDEO_HSO_END, 		    59);
            Wr(ENCL_VIDEO_VSO_BEGIN,	    27);
            Wr(ENCL_VIDEO_VSO_END, 		    59);
            
            Wr(ENCL_VIDEO_VSO_BLINE,        1);
            Wr(ENCL_VIDEO_VSO_ELINE,        7);
            Wr(ENCL_VIDEO_MAX_LNCNT,        1645);
            break;

        case TV_ENC_LCD3840x2440:
            Wr(ENCL_VIDEO_MODE,             0x0040); 
            Wr(ENCL_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            
            Wr(ENCL_VIDEO_MAX_PXCNT,        3840+500-1);
            
            Wr(ENCL_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCL_VIDEO_HAVON_END,        2067+1920);
            Wr(ENCL_VIDEO_VAVON_BLINE,      41);
            Wr(ENCL_VIDEO_VAVON_ELINE,      1120+1360);
            
            Wr(ENCL_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCL_VIDEO_HSO_END, 		    2156+1920);
            Wr(ENCL_VIDEO_VSO_BEGIN,	    2100+1920);
            Wr(ENCL_VIDEO_VSO_END, 		    2164+1920);
            
            Wr(ENCL_VIDEO_VSO_BLINE,        3);
            Wr(ENCL_VIDEO_VSO_ELINE,        5);
            Wr(ENCL_VIDEO_MAX_LNCNT,        1124+1360);
            break;
        case TV_ENC_LCD3840x2440_dsi36b:    // For MIPI_DSI 36-bit color: sample rate=2, 1 pixel per 2 cycle
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD3840x2440_dsi36b not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD1920x1200p:
            Wr(ENCL_VIDEO_MODE,             0x0040);
            Wr(ENCL_VIDEO_MODE_ADV,         0x0018);
            
	        Wr(ENCL_VIDEO_MAX_PXCNT,        1920+160-1);    // H_ACTIVE+H_BLANK-1=2079
	        Wr(ENCL_VIDEO_MAX_LNCNT,        1200+35-1);     // V_ACTIVE+V_BLANK-1=1234
            
	        Wr(ENCL_VIDEO_HAVON_BEGIN,      0   +160-3);    // 0+H_BLANK-1-2=157    // -2 is for extra 2-cycle delay for AVON and data to mipi_dsi input
	        Wr(ENCL_VIDEO_HAVON_END,        1919+160-3);    // H_ACTIVE-1+H_BLANK-1-2=2076
	        Wr(ENCL_VIDEO_VAVON_BLINE,      0   +35);       // 0+V_BLANK=35
	        Wr(ENCL_VIDEO_VAVON_ELINE,      1199+35);       // V_ACTIVE-1+V_BLANK=1234
            
	        Wr(ENCL_VIDEO_HSO_BEGIN,        48-1);          // H_FRONT_PORCH-1=47
	        Wr(ENCL_VIDEO_HSO_END,          48-1+32);       // H_FRONT_PORCH-1+H_SYNC=79
	        Wr(ENCL_VIDEO_VSO_BEGIN,        48-1);          // H_FRONT_PORCH-1=47
	        Wr(ENCL_VIDEO_VSO_END,          48-1);          // H_FRONT_PORCH-1=47
            
	        Wr(ENCL_VIDEO_VSO_BLINE,        3);             // V_FRONT_PORCH=3
	        Wr(ENCL_VIDEO_VSO_ELINE,        9);             // V_SYNC=9
            break;

        case TV_ENC_LCD3840x2160p_vic03:
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD3840x2160p_vic03 not yet added!\n");
            stimulus_finish_fail(1);
            break;

        case TV_ENC_LCD4096x2160p_vic04:
            stimulus_print("[SET_TV_ENC.C] Error: TV_ENC_LCD4096x2160p_vic04 not yet added!\n");
            stimulus_finish_fail(1);
            break;

        default :
            break;
    }

} /* config_tv_enc_lcd */

int set_tv_enc_lcd( tv_enc_lcd_type_t output_type )
{
   
    Wr(VPU_VIU_VENC_MUX_CTRL,
       (0<<0) |    // viu1 select encl
       (0<<2)      // viu2 select encl
       );

    config_tv_enc_lcd(output_type);
    
    return 0;
}


void set_analog_lcd (void)
{
  int read_32;

#ifdef NO_ENCT
  read_32 = Rd(HHI_VIID_CLK_DIV);
  read_32 = (read_32 >> 12) & 0xf;  // encl_clk_sel
#else
  read_32 = Rd(HHI_VID_CLK_DIV);
  read_32 = (read_32 >> 20) & 0xf;  // enct_clk_sel
#endif

  // Select DAC clock
  Wr_reg_bits (HHI_VIID_CLK_DIV,
               (read_32 << 8) | (read_32 << 4) | read_32,
               20, 12); // [31:20] dac0_clk_sel, dac1_clk_sel, dac2_clk_sel

  Wr(VENC_VDAC_FIFO_CTRL, 
               (1 << 14) | // fifo_en_enct
               (0 << 6) |  // DAC_clock_2X
               (0 << 0)    // DAC_clock_4X
               );

    Wr(ENCT_DACSEL_0,           0x3210);  // R, G, B, DTH_R

     //---- dacsel
	Wr(VENC_VDAC_DACSEL0,             8);  // enct
	Wr(VENC_VDAC_DACSEL1,             8);  // enct
	Wr(VENC_VDAC_DACSEL2,             8);  // enct
	Wr(VENC_VDAC_DACSEL3,             8);  // enct
	Wr(VENC_VDAC_DACSEL4,             8);  // enct
	Wr(VENC_VDAC_DACSEL5,             8);  // enct
    
}


void config_tv_enc ( tv_enc_type_t output_type )
{
    int read_32;
    switch(output_type) {
        case TV_ENC_480i:
            Wr(VENC_SYNC_ROUTE, 0);                // Set sync route on vpins
            Wr(VENC_VIDEO_PROG_MODE, 0xf0);        // Set hsync/vsync source from interlace vencoder
            
            Wr(ENCI_YC_DELAY, 0x22);  // both Y and C delay 2 clock
            
            Wr(ENCI_VFIFO2VD_PIXEL_START, 233 );
            Wr(ENCI_VFIFO2VD_PIXEL_END, 233+720*2 );
            Wr(ENCI_VFIFO2VD_LINE_TOP_START, 17);
            Wr(ENCI_VFIFO2VD_LINE_TOP_END, 17+240);
            Wr(ENCI_VFIFO2VD_LINE_BOT_START, 18);
            Wr(ENCI_VFIFO2VD_LINE_BOT_END, 18+240);
            Wr(ENCI_VFIFO2VD_CTL, (0x4e << 8) | 1);     // enable vfifo2vd
            
            // Force line/field change (field =7, line = 261 )
            Wr(ENCI_DBG_FLDLN_RST, 0x0f05);
            Wr(ENCI_SYNC_VSO_EVNLN, 0x0508);
            Wr(ENCI_SYNC_VSO_ODDLN, 0x0508);
            Wr(ENCI_SYNC_HSO_BEGIN, 11-2);
            Wr(ENCI_SYNC_HSO_END, 31-2);
            Wr(ENCI_DBG_FLDLN_RST, 0xcf05);
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Wr(ENCI_DBG_FLDLN_RST, 0x0f05);
            break;
        
        case TV_ENC_480p:
       // USE_DELAYED_VSYNC -- delay 3 line
//       read_32 = Rd(ENCP_VIDEO_SYNC_MODE);
//       read_32 = (read_32 & 0x00ff) | (3<<8);
//       Wr(ENCP_VIDEO_SYNC_MODE, read_32);
//       Wr(VENC_DVI_SETTING, 0x2011); 

            Wr(ENCP_DVI_VSO_BLINE_EVN,   40);
            Wr(ENCP_DVI_VSO_ELINE_EVN,   42);
            Wr(ENCP_DVI_VSO_BEGIN_EVN,   16);
            Wr(ENCP_DVI_VSO_END_EVN,     32);
            Wr(ENCP_DVI_HSO_BEGIN,       16);
            Wr(ENCP_DVI_HSO_END,         32);
            Wr(ENCP_DE_H_BEGIN,          217);
            Wr(ENCP_DE_H_END,            1657);
            Wr(ENCP_DE_V_BEGIN_EVEN,     42);
            Wr(ENCP_DE_V_END_EVEN,       522);
            Wr(ENCP_VIDEO_MODE,          1<<14); // cfg_de_v = 1
            Wr(VENC_DVI_SETTING, 0x8011); // sel_dvi = 1
            
            Wr(ENCP_VIDEO_MODE_ADV, 0x09);       // Use FIFO interface
            
            // SETVAVON
            Wr(ENCP_VIDEO_VAVON_BLINE, 42);
            Wr(ENCP_VIDEO_VAVON_ELINE, 522);
            break;


       case TV_ENCP_480p:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       40);
            Wr(ENCP_VIDEO_YFP2_HTIME,       830);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        857);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     840);
            Wr(ENCP_VIDEO_HSPULS_END,       18);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    18);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     40);
            Wr(ENCP_VIDEO_VSPULS_END,       524);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      80);
            Wr(ENCP_VIDEO_HAVON_END,        799);
            Wr(ENCP_VIDEO_VAVON_BLINE,      11);
            Wr(ENCP_VIDEO_VAVON_ELINE,      490);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    18);
            Wr(ENCP_VIDEO_HSO_END, 		    840);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    830);
            Wr(ENCP_VIDEO_VSO_END, 		    848);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        3);
            Wr(ENCP_VIDEO_VSO_ELINE,        5);
            Wr(ENCP_VIDEO_MAX_LNCNT,        524);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            Wr(VENC_VDAC_DACSEL0,           0x9);
            Wr(VENC_VDAC_DACSEL1,           0xa);
            Wr(VENC_VDAC_DACSEL2,           0xb);
            Wr(VENC_VDAC_DACSEL3,           0xc);
            Wr(VENC_VDAC_DACSEL4,           0xd);
            Wr(VENC_VDAC_DACSEL5,           0xe);
            break;


        case TV_ENC_576p:
            Wr(VENC_VIDEO_PROG_MODE, 0x100);        // select cfg_sel_prog
            
            Wr(ENCP_VFIFO2VD_PIXEL_START, 245 );
            Wr(ENCP_VFIFO2VD_PIXEL_END, 245+720*2 );
            Wr(ENCP_VFIFO2VD_LINE_TOP_START, 44);
            Wr(ENCP_VFIFO2VD_LINE_TOP_END, 44+576);
            Wr(ENCP_VFIFO2VD_LINE_BOT_START, 44);
            Wr(ENCP_VFIFO2VD_LINE_BOT_END, 44+576);
            Wr(ENCP_VFIFO2VD_CTL, (0x4e << 8) | 1);     // enable vfifo2vd
            
            Wr(ENCP_VIDEO_MODE, 0);
            Wr(ENCP_VIDEO_MODE_ADV, 0x01); // do not use VFIFO interface

            Wr(ENCP_VIDEO_HSPULS_BEGIN, 34); 
            Wr(ENCP_VIDEO_HSPULS_END, 148);  
            
            Wr(ENCP_VIDEO_VSPULS_BEGIN,0 );
            Wr(ENCP_VIDEO_VSPULS_END,  1599);
            //Wr(ENCP_VIDEO_VSPULS_BLINE, 624);
            Wr(ENCP_VIDEO_VSPULS_BLINE, 0); // Changed by Gordon.
            Wr(ENCP_VIDEO_VSPULS_ELINE, 5);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN, 248);//264);//231);   //7);karlson edit
            Wr(ENCP_VIDEO_HAVON_END, 1688);  //1672);
             
            Wr(ENCP_VIDEO_VAVON_BLINE, 44);
            Wr(ENCP_VIDEO_VAVON_ELINE, 624);   //22);   //19);karlson edit
            Wr(ENCP_VIDEO_VSO_BLINE, 11);     //19);karlson edit,haunghonglin 19-->15 for bug#
            Wr(ENCP_VIDEO_VSO_ELINE, 16);
            
            Wr(ENCP_MACV_TS_CNT_MAX_L, 0x7dcd);
            Wr(ENCP_MACV_TS_CNT_MAX_H, 0x3);
            Wr(ENCP_MACV_TIME_DOWN, 3068);
            Wr(ENCP_MACV_TIME_LO, 3658);
            Wr(ENCP_MACV_TIME_UP, 4366);
            Wr(ENCP_MACV_TIME_RST, 4956);
            Wr(ENCP_MACV_MAXY_VAL, 590);
            Wr(ENCP_MACV_1ST_PSSYNC_STRT, 237);
            Wr(ENCI_SYNC_HOFFST, 0x12); // adjust for interlace output
            
            Wr(ENCP_VIDEO_YFP1_HTIME, 263);
            Wr(ENCP_VIDEO_YFP2_HTIME, 1646);
            Wr(ENCP_VIDEO_YC_DLY, 0);
            Wr(ENCP_VIDEO_MAX_PXCNT, 1727);
            Wr(ENCP_VIDEO_MAX_LNCNT, 624);
            //Wr(ENCP_MACV_STRTLINE, 4);
            Wr(ENCP_MACV_STRTLINE, 5);
            //Wr(ENCP_MACV_ENDLINE, 12);
            Wr(ENCP_MACV_ENDLINE, 13);

            Wr(ENCP_VIDEO_SYNC_MODE, 0x07); /**** Set for master mode ****/
     
            Wr(ENCP_VIDEO_HSO_BEGIN, 0x3);
            Wr(ENCP_VIDEO_HSO_END, 0x5);
            Wr(ENCP_VIDEO_VSO_BEGIN, 0x3);
            Wr(ENCP_VIDEO_VSO_END, 0x5);

            // USE_DELAYED_VSYNC -- delay 3 line
            read_32 = Rd(ENCP_VIDEO_SYNC_MODE);
            read_32 = (read_32 & 0x00ff) | (3<<8);
            Wr(ENCP_VIDEO_SYNC_MODE, read_32);
            break;

        case TV_ENC_720p:
            Wr(ENCP_VIDEO_MODE,                 0x0040);
            Wr(ENCP_VIDEO_MODE_ADV,             0x0019);    
            Wr(ENCP_VIDEO_YFP1_HTIME,           520);
            Wr(ENCP_VIDEO_YFP2_HTIME,           3080);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,            3299);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,         3220);
            Wr(ENCP_VIDEO_HSPULS_END,           80);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,        80);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,         520);
            Wr(ENCP_VIDEO_VSPULS_END,           3079);
            Wr(ENCP_VIDEO_VSPULS_BLINE,         0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,         5);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,          519);
            Wr(ENCP_VIDEO_HAVON_END,            3078);
            Wr(ENCP_VIDEO_VAVON_BLINE,          25);
            Wr(ENCP_VIDEO_VAVON_ELINE,          744);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,            15);
            Wr(ENCP_VIDEO_HSO_END,              31);
            Wr(ENCP_VIDEO_VSO_BEGIN,            15);
            Wr(ENCP_VIDEO_VSO_END,              31);
            Wr(ENCP_VIDEO_VSO_BLINE,            19);
            Wr(ENCP_VIDEO_VSO_ELINE,            21);
            Wr(ENCP_VIDEO_MAX_LNCNT,            749);
            
            Wr(VENC_VDAC_DACSEL0,               0x9);
            Wr(VENC_VDAC_DACSEL1,               0xa);
            Wr(VENC_VDAC_DACSEL2,               0xb);
            Wr(VENC_VDAC_DACSEL3,               0xc);
            Wr(VENC_VDAC_DACSEL4,               0xd);
            Wr(VENC_VDAC_DACSEL5,               0xe);
            break;

        case TV_ENC_1080p:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       2060);
            Wr(ENCP_VIDEO_MAX_PXCNT,        2199);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     2156);
            Wr(ENCP_VIDEO_HSPULS_END,       44);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    44);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     140);
            Wr(ENCP_VIDEO_VSPULS_END,       2059);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        2067);
            Wr(ENCP_VIDEO_VAVON_BLINE,      41);
            Wr(ENCP_VIDEO_VAVON_ELINE,      1120);
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100);
            Wr(ENCP_VIDEO_VSO_END, 		    2164);
            Wr(ENCP_VIDEO_VSO_BLINE,        3);
            Wr(ENCP_VIDEO_VSO_ELINE,        5);
            Wr(ENCP_VIDEO_MAX_LNCNT,        1124);
            break;

        case TV_ENC_1080i:
            Wr(VENC_VIDEO_PROG_MODE,        0x0100);

	        Wr(ENCP_VIDEO_MODE,             0x1ffc | (1<<14)); // bit[14] cfg_de_v = 1
 	        Wr(ENCP_VIDEO_MODE_ADV,         0x0019);	
	        Wr(ENCP_VIDEO_YFP1_HTIME,       384);
	        Wr(ENCP_VIDEO_YFP2_HTIME,       4223);

	        Wr(ENCP_VIDEO_MAX_PXCNT,        4399);
	        Wr(ENCP_VIDEO_HSPULS_BEGIN,     4312);
	        Wr(ENCP_VIDEO_HSPULS_END,       88);
	        Wr(ENCP_VIDEO_HSPULS_SWITCH,    88);
	        Wr(ENCP_VIDEO_VSPULS_BEGIN,     264);
	        Wr(ENCP_VIDEO_VSPULS_END,       2024);
	        Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
	        Wr(ENCP_VIDEO_VSPULS_ELINE,     4);

	        Wr(ENCP_VIDEO_HAVON_BEGIN,      383);
	        Wr(ENCP_VIDEO_HAVON_END,        4222);
	        Wr(ENCP_VIDEO_VAVON_BLINE,      20);
	        Wr(ENCP_VIDEO_VAVON_ELINE,      559);

            Wr(ENCP_VIDEO_HSO_BEGIN,        15);
            Wr(ENCP_VIDEO_HSO_END,          31);
            Wr(ENCP_VIDEO_VSO_BEGIN,        15);
            Wr(ENCP_VIDEO_VSO_END,          31);
            Wr(ENCP_VIDEO_VSO_BLINE,        15);
	        Wr(ENCP_VIDEO_VSO_ELINE,        17);
	        Wr(ENCP_VIDEO_MAX_LNCNT,        1124);
	
	        Wr(ENCP_VIDEO_OFLD_VPEQ_OFST,   0x0100);
	        Wr(ENCP_VIDEO_OFLD_VOAV_OFST,   0x0111);

            Wr(VENC_VDAC_DACSEL0,           0x9);
            Wr(VENC_VDAC_DACSEL1,           0xa);
            Wr(VENC_VDAC_DACSEL2,           0xb);
            Wr(VENC_VDAC_DACSEL3,           0xc);
            Wr(VENC_VDAC_DACSEL4,           0xd);
            Wr(VENC_VDAC_DACSEL5,           0xe);
            break;

        case TV_ENC_576i:
            Wr(ENCI_CFILT_CTRL,     0x0800);
            Wr(ENCI_CFILT_CTRL2,    0x0010);

            //adjust the hsync start point and end point
            Wr(ENCI_SYNC_HSO_BEGIN, 1);
            Wr(ENCI_SYNC_HSO_END,   127);

            //adjust the vsync start line and end line
            Wr(ENCI_SYNC_VSO_EVNLN, (0 << 8) | 3 );
            Wr(ENCI_SYNC_VSO_ODDLN, (0 << 8) | 3 );

            Wr(ENCI_SYNC_HOFFST,    0x16); // adjust for interlace output, Horizontal offset after HSI in slave mode,

            Wr(ENCI_MACV_MAX_AMP,   0x8107); // ENCI_MACV_MAX_AMP
            Wr(VENC_VIDEO_PROG_MODE,0xff); /**** Set for interlace mode ****/
            Wr(ENCI_VIDEO_MODE,     0x13); /**** Set for PAL mode ****/
            Wr(ENCI_VIDEO_MODE_ADV, 0x26); /**** Set for High Bandwidth for CBW&YBW ****/
            Wr(ENCI_VIDEO_SCH,      0x28); /**** Set SCH ****/
            Wr(ENCI_SYNC_MODE,      0x07); /**** Set for master mode ****/
            /**** Set hs/vs out timing  ****/
            Wr(ENCI_YC_DELAY,       0x341);   // Gordon 2003.3.28

            Wr(ENCI_VFIFO2VD_PIXEL_START,    267);
            Wr(ENCI_VFIFO2VD_PIXEL_END,      267+1440);
            Wr(ENCI_VFIFO2VD_LINE_TOP_START, 21);
            Wr(ENCI_VFIFO2VD_LINE_TOP_END,   21+288);
            Wr(ENCI_VFIFO2VD_LINE_BOT_START, 22);
            Wr(ENCI_VFIFO2VD_LINE_BOT_END,   22+288);
            Wr(ENCI_VFIFO2VD_CTL,            (0x4e << 8) | 1);     // enable vfifo2vd
            Wr(ENCI_DBG_PX_RST,         0x0); /* Enable TV encoder */
            break;

        case TV_ENC_2205p:  // 1920x(1080+45+1080), 3D frame packing for 1920x1080p
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       2060);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        2749);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     2156);
            Wr(ENCP_VIDEO_HSPULS_END,       44);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    44);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     140);
            Wr(ENCP_VIDEO_VSPULS_END,       2059);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        2067);
            Wr(ENCP_VIDEO_VAVON_BLINE,      41);
            Wr(ENCP_VIDEO_VAVON_ELINE,      2245);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100);
            Wr(ENCP_VIDEO_VSO_END, 		    2164);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        3);
            Wr(ENCP_VIDEO_VSO_ELINE,        5);
            Wr(ENCP_VIDEO_MAX_LNCNT,        2249);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            Wr(VENC_VDAC_DACSEL0,           0x9);
            Wr(VENC_VDAC_DACSEL1,           0xa);
            Wr(VENC_VDAC_DACSEL2,           0xb);
            Wr(VENC_VDAC_DACSEL3,           0xc);
            Wr(VENC_VDAC_DACSEL4,           0xd);
            Wr(VENC_VDAC_DACSEL5,           0xe);
            break;

        case TV_ENC_2440p:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       140+1920*2);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        3840+500-1);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     2156+1920);
            Wr(ENCP_VIDEO_HSPULS_END,       44);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    44);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     140);
            Wr(ENCP_VIDEO_VSPULS_END,       2059+1920);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        2067+1920);
            Wr(ENCP_VIDEO_VAVON_BLINE,      41);
            Wr(ENCP_VIDEO_VAVON_ELINE,      1120+1360);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156+1920);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100+1920);
            Wr(ENCP_VIDEO_VSO_END, 		    2164+1920);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        3);
            Wr(ENCP_VIDEO_VSO_ELINE,        5);
            Wr(ENCP_VIDEO_MAX_LNCNT,        1124+1360);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            break;

         case TV_ENC_3840x2160p_vic01:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       140+3840);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        4439);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        3987);
            Wr(ENCP_VIDEO_VAVON_BLINE,      89);
            Wr(ENCP_VIDEO_VAVON_ELINE,      2248);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156+1920);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100+1920);
            Wr(ENCP_VIDEO_VSO_END, 		    2164+1920);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        51);
            Wr(ENCP_VIDEO_VSO_ELINE,        53);
            Wr(ENCP_VIDEO_MAX_LNCNT,        2249);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            break;
     

        case TV_ENC_3840x2160p_vic03:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       140+3840);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        5499);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     2156+1920);
            Wr(ENCP_VIDEO_HSPULS_END,       44);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    44);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     140);
            Wr(ENCP_VIDEO_VSPULS_END,       2059+1920);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        3987);
            Wr(ENCP_VIDEO_VAVON_BLINE,      89);
            Wr(ENCP_VIDEO_VAVON_ELINE,      2248);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156+1920);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100+1920);
            Wr(ENCP_VIDEO_VSO_END, 		    2164+1920);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        51);
            Wr(ENCP_VIDEO_VSO_ELINE,        53);
            Wr(ENCP_VIDEO_MAX_LNCNT,        2249);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            break;
            
        case TV_ENC_4096x2160p_vic04:
            Wr(ENCP_VIDEO_MODE,             0x0040 | (1<<14)); // Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
            Wr(ENCP_VIDEO_MODE_ADV,         0x0008); // Sampling rate: 1
            Wr(ENCP_VIDEO_YFP1_HTIME,       140);
            Wr(ENCP_VIDEO_YFP2_HTIME,       140+4096);
            
            Wr(ENCP_VIDEO_MAX_PXCNT,        5499);
            Wr(ENCP_VIDEO_HSPULS_BEGIN,     2156+1920);
            Wr(ENCP_VIDEO_HSPULS_END,       44);
            Wr(ENCP_VIDEO_HSPULS_SWITCH,    44);
            Wr(ENCP_VIDEO_VSPULS_BEGIN,     140);
            Wr(ENCP_VIDEO_VSPULS_END,       2059+1920);
            Wr(ENCP_VIDEO_VSPULS_BLINE,     0);
            Wr(ENCP_VIDEO_VSPULS_ELINE,     4);
            
            Wr(ENCP_VIDEO_HAVON_BEGIN,      148);
            Wr(ENCP_VIDEO_HAVON_END,        4243);
            Wr(ENCP_VIDEO_VAVON_BLINE,      89);
            Wr(ENCP_VIDEO_VAVON_ELINE,      2248);
            
            Wr(ENCP_VIDEO_HSO_BEGIN,	    44);
            Wr(ENCP_VIDEO_HSO_END, 		    2156+1920);
            Wr(ENCP_VIDEO_VSO_BEGIN,	    2100+1920);
            Wr(ENCP_VIDEO_VSO_END, 		    2164+1920);
            
            Wr(ENCP_VIDEO_VSO_BLINE,        51);
            Wr(ENCP_VIDEO_VSO_ELINE,        53);
            Wr(ENCP_VIDEO_MAX_LNCNT,        2249);
            
            Wr(ENCP_VIDEO_FILT_CTRL,        0x1000); //bypass filter

            break;
            
        default :
            break;
    }
} /* config_tv_enc */


void config_tv_enci ( tv_enc_type_t output_type )
{
    int read_32;

    switch(output_type) {
        case TV_ENC_480i:
            Wr(VENC_SYNC_ROUTE, 0);                // Set sync route on vpins
            Wr(VENC_VIDEO_PROG_MODE, 0xf0);        // Set hsync/vsync source from interlace vencoder
            
            Wr(ENCI_YC_DELAY, 0x22);  // both Y and C delay 2 clock
            
            Wr(ENCI_VFIFO2VD_PIXEL_START, 233 );
            Wr(ENCI_VFIFO2VD_PIXEL_END, 233+720*2 );
            Wr(ENCI_VFIFO2VD_LINE_TOP_START, 17);
            Wr(ENCI_VFIFO2VD_LINE_TOP_END, 17+240);
            Wr(ENCI_VFIFO2VD_LINE_BOT_START, 18);
            Wr(ENCI_VFIFO2VD_LINE_BOT_END, 18+240);
            Wr(ENCI_VFIFO2VD_CTL, (0x4e << 8) | 1);     // enable vfifo2vd
            
            // Force line/field change (field =7, line = 261 )
            Wr(ENCI_DBG_FLDLN_RST, 0x0f05);
            Wr(ENCI_SYNC_VSO_EVNLN, 0x0508);
            Wr(ENCI_SYNC_VSO_ODDLN, 0x0508);
            Wr(ENCI_SYNC_HSO_BEGIN, 11-2);
            Wr(ENCI_SYNC_HSO_END, 31-2);
            Wr(ENCI_DBG_FLDLN_RST, 0xcf05);
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Wr(ENCI_DBG_FLDLN_RST, 0x0f05);
            break;
        
        case TV_ENC_576i:
            Wr(VENC_SYNC_ROUTE, 0);                // Set sync route on vpins
            Wr(ENCI_VIDEO_MODE,     0x13); /**** Set for PAL mode ****/
            Wr(VENC_VIDEO_PROG_MODE, 0xf0);        // Set hsync/vsync source from interlace vencoder
            
            Wr(ENCI_YC_DELAY, 0x22);  // both Y and C delay 2 clock
            
            Wr(ENCI_VFIFO2VD_PIXEL_START, 233 );
            Wr(ENCI_VFIFO2VD_PIXEL_END, 233+720*2 );
            Wr(ENCI_VFIFO2VD_LINE_TOP_START, 17);
            Wr(ENCI_VFIFO2VD_LINE_TOP_END, 17+240);
            Wr(ENCI_VFIFO2VD_LINE_BOT_START, 18);
            Wr(ENCI_VFIFO2VD_LINE_BOT_END, 18+240);
            Wr(ENCI_VFIFO2VD_CTL, (0x4e << 8) | 1);     // enable vfifo2vd
            
            // Force line/field change (field =7, line = 261 )
            Wr(ENCI_DBG_FLDLN_RST, 0x0f05);
            Wr(ENCI_SYNC_VSO_EVNLN, 0x0508);
            Wr(ENCI_SYNC_VSO_ODDLN, 0x0508);
            Wr(ENCI_SYNC_HSO_BEGIN, 11-2);
            Wr(ENCI_SYNC_HSO_END, 31-2);
            Wr(ENCI_DBG_FLDLN_RST, 0xcf37);
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Rd(ENCI_DBG_FLDLN_RST); // delay fldln rst to make sure it synced by clk27
            Wr(ENCI_DBG_FLDLN_RST, 0x0f37);
            break;

        default :
            break;
    }
} /* config_tv_enci */



#ifdef NO_ENCP
int set_tv_enc( tv_enc_type_t output_type )
{
   switch (output_type) {
        case TV_ENC_480i:
          stimulus_print ("Error: Do not support 480i in encl\n");
          break;
        case TV_ENC_480p:
          stimulus_print ("set 480p in encl\n");
          set_tv_enc_lcd (TV_ENC_LCD720x480);
          break;
        case TV_ENC_576p:
          stimulus_print ("set 576p in encl\n");
          set_tv_enc_lcd (TV_ENC_LCD720x576);
          break;
        case TV_ENC_720p:
          stimulus_print ("set 720p in encl\n");
          set_tv_enc_lcd (TV_ENC_LCD1280x720);
          break;

        case TV_ENC_1080p:
          stimulus_print ("set 1080p in encl\n");
          set_tv_enc_lcd (TV_ENC_LCD1920x1080);
          //Wr(ENCP_VIDEO_EN, 1); // Enable it in test.c
          break;  
        case TV_ENC_1080i:
          stimulus_print ("Error: Do not support 1080i in encl\n");
          break;
        case TV_ENC_576i:
          stimulus_print ("Error: Do not support 576i in encl\n");
          break;

        case TV_ENC_2440p:
          stimulus_print ("set 2440p in encl\n");
          set_tv_enc_lcd (TV_ENC_LCD3840x2440);
          break;
   
        case TV_ENC_3840x2160p_vic03:
            stimulus_print ("set 3840x2160p_vic03 in encl\n");
            set_tv_enc_lcd(TV_ENC_LCD3840x2160p_vic03);
            break;

        case TV_ENC_4096x2160p_vic04:
            stimulus_print ("set 4096x2160p_vic04 in encl\n");
            set_tv_enc_lcd(TV_ENC_LCD4096x2160p_vic04);
            break;
   }

}

#else
int set_tv_enc( tv_enc_type_t output_type )
{
    printf("%s %d\n", __func__, __LINE__);
   
    Wr(ENCI_VIDEO_EN, 0);                 // Enable Interlace video encoder
    Wr(ENCP_VIDEO_EN, 0);                 // Enable Interlace video encoder

    config_tv_enc(output_type);
    printf("%s %d\n", __func__, __LINE__);
    switch(output_type) {
        case TV_ENC_480i:
            Wr(VPU_VIU_VENC_MUX_CTRL,
               (1<<0) |    // viu1 select enci
               (1<<2)      // viu2 select enci
               );
            Wr(VENC_VDAC_FIFO_CTRL, 
               (1 << 13) | // fifo_en_enci
               (0x3f << 6) | // DAC_clock_2X
               (0 << 0)      // DAC_clock_4X
               );
            
            Wr(ENCI_VIDEO_EN, 1);                 // Enable Interlace video encoder
            break;
        
        case TV_ENC_480p:
            Wr(VPU_VIU_VENC_MUX_CTRL,
               (2<<0) |    // viu1 select encp
               (2<<2)      // viu2 select encp
               );
            Wr(VENC_VDAC_FIFO_CTRL, 
               (0 << 13) | // fifo_en_enci
               (1 << 12) | // fifo_en_encp
               (0x3f << 6) | // DAC_clock_2X
               (0 << 0)      // DAC_clock_4X
               );
            
            Wr(ENCP_VIDEO_EN, 1);                 // Enable Interlace video encoder
            break;
        
        case TV_ENC_576p:
            Wr(VPU_VIU_VENC_MUX_CTRL,
               (2<<0) |    // viu1 select encp
               (2<<2)      // viu2 select encp
               );
            Wr(VENC_VDAC_FIFO_CTRL, 
               (0 << 13) | // fifo_en_enci
               (1 << 12) | // fifo_en_encp
               (0 << 6) |  // DAC_clock_2X
               (0 << 0)    // DAC_clock_4X
               );
            
            Wr(ENCP_VIDEO_EN, 1);                 // Enable Interlace video encoder
            break;
       
        case TV_ENC_720p:
            Wr(ENCP_VIDEO_EN, 1); 
            break;

        case TV_ENC_1080p:
        case TV_ENC_3840x2160p_vic03:
        case TV_ENC_4096x2160p_vic04:
    printf("%s %d\n", __func__, __LINE__);
            //Wr(ENCP_VIDEO_EN, 1); // Enable it in test.c
            break;
        
        case TV_ENC_1080i:
            //Wr(ENCP_VIDEO_EN, 1); // Enable it in test.c
            break;
            
        case TV_ENC_576i:
            //Wr(ENCI_VIDEO_EN, 1); // Enable it in test.c
            break;
            
        case TV_ENC_2440p:
            Wr(VPU_VIU_VENC_MUX_CTRL,
               (2<<0) |    // viu1 select encp
               (2<<2)      // viu2 select encp
               );
            Wr(VENC_VDAC_FIFO_CTRL, 
               (0 << 13) | // fifo_en_enci
               (1 << 12) | // fifo_en_encp
               (0 << 6) | // DAC_clock_2X
               (0 << 0)      // DAC_clock_4X
               );
            
            Wr(ENCP_VIDEO_EN, 1);                 // Enable Interlace video encoder
            break;

        default :
            break;
    }
    printf("%s %d\n", __func__, __LINE__);
   return 0;

}

int set_tv_enci( tv_enc_type_t output_type )
{
   
    Wr(ENCI_VIDEO_EN, 0);                 // Enable Interlace video encoder

    config_tv_enci(output_type);

            Wr(VPU_VIU_VENC_MUX_CTRL,
               (1<<0) |    // viu1 select enci
               (1<<2)      // viu2 select enci
               );
            Wr(VENC_VDAC_FIFO_CTRL, 
               (1 << 13) | // fifo_en_enci
               (0x3f << 6) | // DAC_clock_2X
               (0 << 0)      // DAC_clock_4X
               );
            
            Wr(ENCI_VIDEO_EN, 1);                 // Enable Interlace video encoder

   return 0;

}
#endif  //NO_ENCP



void init_lcd_ctrl_analog(int gamma_enable)
{
    unsigned i;

    Wr(RGB_BASE_ADDR, 240);
    Wr(RGB_COEFF_ADDR, 1871);
    Wr(POL_CNTL_ADDR,  (0x1 << HS_POL) |
                       (0x1 << VS_POL)
                       );

    Wr(DUAL_PORT_CNTL_ADDR, (0x1 << LCD_TTL_SEL |
                             0x1 << LCD_ANALOG_SEL_CPH3 |
                             0x1 << LCD_ANALOG_3PHI_CLK_SEL));
   
  if(gamma_enable){
//GAMMA PROG
//Auto Write & Simuteously to RGB
    while (!(Rd(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x0 << HADR)    );
                                                                                                                                
    for (i=0x0;i<256;i++) {
        while (!( Rd(GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        Wr(GAMMA_DATA_PORT, (i << 2));
    }
                                                                                                                                
    while (!(Rd(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x23 << HADR)    );

    Wr(GAMMA_CNTL_PORT, (1 << GAMMA_EN) | (1 << GAMMA_RVS_OUT));
  } // gamma_enable
}

void set_tcon_240x160(void)
{
   Wr(STH1_HS_ADDR, 67+2);
   Wr(STH1_HE_ADDR, 68+2);
   Wr(STH1_VS_ADDR, 20);
   Wr(STH1_VE_ADDR, 20+160);

   Wr(STH2_HS_ADDR, 67+2);
   Wr(STH2_HE_ADDR, 68+2);
   Wr(STH2_VS_ADDR, 20);
   Wr(STH2_VE_ADDR, 20+160);

   
   Wr(OEH_HS_ADDR, 68+240);
   Wr(OEH_HE_ADDR, 72+240);
   Wr(OEH_VS_ADDR, 20);
   Wr(OEH_VE_ADDR, 20+160);

   Wr(VCOM_HSWITCH_ADDR, 8);
   Wr(VCOM_VS_ADDR, 20);
   Wr(VCOM_VE_ADDR, 20+160+6);

   Wr(CPV1_HS_ADDR, 4);
   Wr(CPV1_HE_ADDR, (120+240)/2);
   Wr(CPV1_VS_ADDR, 4);
   Wr(CPV1_VE_ADDR, 3);

   Wr(CPV2_HS_ADDR, 4);
   Wr(CPV2_HE_ADDR, (120+240)/2);
   Wr(CPV2_VS_ADDR, 4);
   Wr(CPV2_VE_ADDR, 3);

   Wr(STV1_HS_ADDR, 2);
   Wr(STV1_HE_ADDR, 1);
   Wr(STV1_VS_ADDR, 20);
   Wr(STV1_VE_ADDR, 21);

   Wr(STV2_HS_ADDR, 2);
   Wr(STV2_HE_ADDR, 1);
   Wr(STV2_VS_ADDR, 20);
   Wr(STV2_VE_ADDR, 21);

   Wr(OEV1_HS_ADDR, 120+240);
   Wr(OEV1_HE_ADDR, 1);
   Wr(OEV1_VS_ADDR, 20);
   Wr(OEV1_VE_ADDR, 21+160);

   Wr(OEV2_HS_ADDR, 120+240);
   Wr(OEV2_HE_ADDR, 1);
   Wr(OEV2_VS_ADDR, 20);
   Wr(OEV2_VE_ADDR, 21+160);

   Wr(OEV3_HS_ADDR, 120+240);
   Wr(OEV3_HE_ADDR, 1);
   Wr(OEV3_VS_ADDR, 20);
   Wr(OEV3_VE_ADDR, 21+160);

   Wr(INV_CNT_ADDR, (1 << INV_EN) |
                     (12 << INV_CNT));


}

    // set new TCOM 
    // TCON_6 - vcom
    // TCON_4 - oev1
    // TCON_3 - cpv1
    // TCON_2 - oeh 
    // TCON_1 - stv1
    // TCON_0 - sth1
void set_new_tcon_1280x1024(void)
{
   unsigned int data32;

   // disable tcon_double on all channels
   Wr(DE_HS_ADDR, 0);
   Wr(DE_HE_ADDR, 0);

   Wr(TCON_MISC_SEL_ADDR, 
      (1<<3) // oev_unite
    | (1<<5) // stv2_sel
    | (1<<6) // cpv1_sel ( stv1 in new tcon - TCON6)
    );

   // STH1 - TCON_0 
   Wr(STH1_HS_ADDR, 95+2);
   Wr(STH1_HE_ADDR, 96+2);
   Wr(STH1_VS_ADDR, 10);
   Wr(STH1_VE_ADDR, 10+1024);

   // STH2 - old tcon
   Wr(STH2_HS_ADDR, 95+2);
   Wr(STH2_HE_ADDR, 96+2);
   Wr(STH2_VS_ADDR, 10);
   Wr(STH2_VE_ADDR, 10+1024);

   // OEH - TCON_2  -- STV1 ADDR
   Wr(STV1_HS_ADDR, 1382);
   Wr(STV1_HE_ADDR, 1386);
   Wr(STV1_VS_ADDR, 10);
   Wr(STV1_VE_ADDR, 10+1024);

   // VCOM - TCON_6 -- OEH ADDR

   Wr(OEH_HS_ADDR, 8);
   Wr(OEH_HE_ADDR, 0); // no use
   Wr(OEH_VS_ADDR, 5);
   Wr(OEH_VE_ADDR, 1205);

    data32 = (0x5555 << tcon_pattern_loop_data) |
             (0 << tcon_pattern_loop_start) |
             (1 << tcon_pattern_loop_end) |
             (1 << (6+tcon_pattern_enable)); // POL in channel 1 use pattern generate 

    Wr(TCON_PATTERN_HI,  (data32 >> 16));
    Wr(TCON_PATTERN_LO, (data32 & 0xffff));

   // CPV1 - TCON_3  -- OEV1 ADDR 
   Wr(OEV1_HS_ADDR, 4);
   Wr(OEV1_HE_ADDR, 700);
   Wr(OEV1_VS_ADDR, 2);
   Wr(OEV1_VE_ADDR, 1);

   // CPV2 - old tcon 
   Wr(CPV2_HS_ADDR, 4);
   Wr(CPV2_HE_ADDR, 700);
   Wr(CPV2_VS_ADDR, 2);
   Wr(CPV2_VE_ADDR, 1);

   // STV1 - TCON_1  -- CPV1 ADDR
   Wr(CPV1_HS_ADDR, 2);
   Wr(CPV1_HE_ADDR, 1);
   Wr(CPV1_VS_ADDR, 10);
   Wr(CPV1_VE_ADDR, 11);

   // STV2 - old tcon
   Wr(STV2_HS_ADDR, 2);
   Wr(STV2_HE_ADDR, 1);
   Wr(STV2_VS_ADDR, 10);
   Wr(STV2_VE_ADDR, 11);

   // OEV1 - TCON_4  -- HSYNC ADDR
   Wr(HSYNC_HS_ADDR, 1400);
   Wr(HSYNC_HE_ADDR, 1);
   Wr(HSYNC_VS_ADDR, 10);
   Wr(HSYNC_VE_ADDR, 11+1024);

   // OEV2 - old tcon
   Wr(OEV2_HS_ADDR, 1400);
   Wr(OEV2_HE_ADDR, 1);
   Wr(OEV2_VS_ADDR, 10);
   Wr(OEV2_VE_ADDR, 11+1024);

   // OEV3 - old tcon
   Wr(OEV3_HS_ADDR, 1400);
   Wr(OEV3_HE_ADDR, 1);
   Wr(OEV3_VS_ADDR, 10);
   Wr(OEV3_VE_ADDR, 11+1024);
}




void set_tcon_480x234(void)
{
   Wr(STH1_HS_ADDR, 67+2);
   Wr(STH1_HE_ADDR, 68+2);
   Wr(STH1_VS_ADDR, 20);
   Wr(STH1_VE_ADDR, 20+234);

   Wr(STH2_HS_ADDR, 67+2);
   Wr(STH2_HE_ADDR, 68+2);
   Wr(STH2_VS_ADDR, 20);
   Wr(STH2_VE_ADDR, 20+234);

   
   Wr(OEH_HS_ADDR, 548);
   Wr(OEH_HE_ADDR, 552);
   Wr(OEH_VS_ADDR, 20);
   Wr(OEH_VE_ADDR, 20+234);

   Wr(VCOM_HSWITCH_ADDR, 8);
   Wr(VCOM_VS_ADDR, 20);
   Wr(VCOM_VE_ADDR, 20+240);

   Wr(CPV1_HS_ADDR, 4);
   Wr(CPV1_HE_ADDR, 300);
   Wr(CPV1_VS_ADDR, 4);
   Wr(CPV1_VE_ADDR, 3);

   Wr(CPV2_HS_ADDR, 4);
   Wr(CPV2_HE_ADDR, 300);
   Wr(CPV2_VS_ADDR, 4);
   Wr(CPV2_VE_ADDR, 3);

   Wr(STV1_HS_ADDR, 2);
   Wr(STV1_HE_ADDR, 1);
   Wr(STV1_VS_ADDR, 20);
   Wr(STV1_VE_ADDR, 21);

   Wr(STV2_HS_ADDR, 2);
   Wr(STV2_HE_ADDR, 1);
   Wr(STV2_VS_ADDR, 20);
   Wr(STV2_VE_ADDR, 21);

   Wr(OEV1_HS_ADDR, 600);
   Wr(OEV1_HE_ADDR, 1);
   Wr(OEV1_VS_ADDR, 20);
   Wr(OEV1_VE_ADDR, 21+234);

   Wr(OEV2_HS_ADDR, 600);
   Wr(OEV2_HE_ADDR, 1);
   Wr(OEV2_VS_ADDR, 20);
   Wr(OEV2_VE_ADDR, 21+234);

   Wr(OEV3_HS_ADDR, 600);
   Wr(OEV3_HE_ADDR, 1);
   Wr(OEV3_VS_ADDR, 20);
   Wr(OEV3_VE_ADDR, 21+234);

   Wr(INV_CNT_ADDR, (1 << INV_EN) |
                     (12 << INV_CNT));

   

}


void init_lcd_ctrl_digital(int gamma_enable)
{
    unsigned i;

    Wr(RGB_BASE_ADDR, 240);
    Wr(RGB_COEFF_ADDR, 1871);
    Wr(POL_CNTL_ADDR,  (0x1 << HS_POL) |
                       (0x1 << VS_POL));

    Wr(DUAL_PORT_CNTL_ADDR, (0x1 << LCD_TTL_SEL));
   
  if(gamma_enable){
//GAMMA PROG
//Auto Write & Simuteously to RGB
    while (!(Rd(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x0 << HADR)    );
                                                                                                                                
    for (i=0x0;i<256;i++) {
        while (!( Rd(GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        Wr(GAMMA_DATA_PORT, (i << 2));
    }
                                                                                                                                
    while (!(Rd(GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x23 << HADR)    );

    Wr(GAMMA_CNTL_PORT, (1 << GAMMA_EN) | (1 << GAMMA_RVS_OUT));
  } // gamma_enable
}

void init_lvds_ctrl_digital(int gamma_enable)
{
    unsigned i;

    Wr(L_RGB_BASE_ADDR, 240);
    Wr(L_RGB_COEFF_ADDR, 1871);
    Wr(L_POL_CNTL_ADDR,  (0x1 << HS_POL) |
                       (0x1 << VS_POL));

    Wr(L_DUAL_PORT_CNTL_ADDR, (0x1 << LCD_TTL_SEL));
   
  if(gamma_enable){
//GAMMA PROG
//Auto Write & Simuteously to RGB
    while (!(Rd(L_GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(L_GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x0 << HADR)    );
                                                                                                                                
    for (i=0x0;i<256;i++) {
        while (!( Rd(L_GAMMA_CNTL_PORT) & (0x1 << WR_RDY) )) ;
        Wr(L_GAMMA_DATA_PORT, (i << 2));
    }
                                                                                                                                
    while (!(Rd(L_GAMMA_CNTL_PORT) & (0x1 << ADR_RDY)));
       Wr(L_GAMMA_ADDR_PORT, (0x1 << H_AUTO_INC) |
                           (0x1 << H_SEL_R)   |
                           (0x1 << H_SEL_G)   |
                           (0x1 << H_SEL_B)   |
                           (0x23 << HADR)    );

    Wr(L_GAMMA_CNTL_PORT, (1 << GAMMA_EN) | (1 << GAMMA_RVS_OUT));
  } // gamma_enable
}

void set_tcon_720x480(void)
{
   Wr(STH1_HS_ADDR, 97);
   Wr(STH1_HE_ADDR, 98);
   Wr(STH1_VS_ADDR, 10);
   Wr(STH1_VE_ADDR, 10+480);

   Wr(STH2_HS_ADDR, 97);
   Wr(STH2_HE_ADDR, 98);
   Wr(STH2_VS_ADDR, 10);
   Wr(STH2_VE_ADDR, 10+480);

   
   Wr(OEH_HS_ADDR, 28);
   Wr(OEH_HE_ADDR, 32);
   Wr(OEH_VS_ADDR, 11);
   Wr(OEH_VE_ADDR, 11+480);

   Wr(VCOM_HSWITCH_ADDR, 8);
   Wr(VCOM_VS_ADDR, 8);
   Wr(VCOM_VE_ADDR, 8+510);

   Wr(CPV1_HS_ADDR, 4);
   Wr(CPV1_HE_ADDR, 500);
   Wr(CPV1_VS_ADDR, 2);
   Wr(CPV1_VE_ADDR, 1);

   Wr(CPV2_HS_ADDR, 4);
   Wr(CPV2_HE_ADDR, 500);
   Wr(CPV2_VS_ADDR, 2);
   Wr(CPV2_VE_ADDR, 1);

   Wr(STV1_HS_ADDR, 2);
   Wr(STV1_HE_ADDR, 1);
   Wr(STV1_VS_ADDR, 10);
   Wr(STV1_VE_ADDR, 11);

   Wr(STV2_HS_ADDR, 2);
   Wr(STV2_HE_ADDR, 1);
   Wr(STV2_VS_ADDR, 10);
   Wr(STV2_VE_ADDR, 11);

   Wr(OEV1_HS_ADDR, 600);
   Wr(OEV1_HE_ADDR, 1);
   Wr(OEV1_VS_ADDR, 9);
   Wr(OEV1_VE_ADDR, 10+480);

   Wr(OEV2_HS_ADDR, 600);
   Wr(OEV2_HE_ADDR, 1);
   Wr(OEV2_VS_ADDR, 9);
   Wr(OEV2_VE_ADDR, 10+480);

   Wr(OEV3_HS_ADDR, 600);
   Wr(OEV3_HE_ADDR, 1);
   Wr(OEV3_VS_ADDR, 9);
   Wr(OEV3_VE_ADDR, 10+480);

   Wr(INV_CNT_ADDR, (1 << INV_EN) | (12 << INV_CNT));

}


void set_tcon_1024x768(void)
{
   Wr(STH1_HS_ADDR,  95+2);
   Wr(STH1_HE_ADDR,  96+2);
   Wr(STH1_VS_ADDR,  10);
   Wr(STH1_VE_ADDR,  10+768);

   Wr(STH2_HS_ADDR,  95+2);
   Wr(STH2_HE_ADDR,  96+2);
   Wr(STH2_VS_ADDR,  10);
   Wr(STH2_VE_ADDR,  10+768);

   
   Wr(OEH_HS_ADDR,  1124);
   Wr(OEH_HE_ADDR,  1128);
   Wr(OEH_VS_ADDR,  10);
   Wr(OEH_VE_ADDR,  10+768);

   Wr(VCOM_HSWITCH_ADDR,  8);
   Wr(VCOM_VS_ADDR,  5);
   Wr(VCOM_VE_ADDR,  805);

   Wr(CPV1_HS_ADDR,  4);
   Wr(CPV1_HE_ADDR,  600);
   Wr(CPV1_VS_ADDR,  2);
   Wr(CPV1_VE_ADDR,  1);

   Wr(CPV2_HS_ADDR,  4);
   Wr(CPV2_HE_ADDR,  600);
   Wr(CPV2_VS_ADDR,  2);
   Wr(CPV2_VE_ADDR,  1);

   Wr(STV1_HS_ADDR,  2);
   Wr(STV1_HE_ADDR,  1);
   Wr(STV1_VS_ADDR,  10);
   Wr(STV1_VE_ADDR,  11);

   Wr(STV2_HS_ADDR,  2);
   Wr(STV2_HE_ADDR,  1);
   Wr(STV2_VS_ADDR,  10);
   Wr(STV2_VE_ADDR,  11);

   Wr(OEV1_HS_ADDR,  1100);
   Wr(OEV1_HE_ADDR,  1);
   Wr(OEV1_VS_ADDR,  10);
   Wr(OEV1_VE_ADDR,  11+768);

   Wr(OEV2_HS_ADDR,  1100);
   Wr(OEV2_HE_ADDR,  1);
   Wr(OEV2_VS_ADDR,  10);
   Wr(OEV2_VE_ADDR,  11+768);

   Wr(OEV3_HS_ADDR,  1100);
   Wr(OEV3_HE_ADDR,  1);
   Wr(OEV3_VS_ADDR,  10);
   Wr(OEV3_VE_ADDR,  11+768);

   Wr(INV_CNT_ADDR,  (1 << INV_EN) |
                     (12 << INV_CNT));

}

void set_tcon_1280x1024(void)
{
   Wr(STH1_HS_ADDR, 95+2);
   Wr(STH1_HE_ADDR, 96+2);
   Wr(STH1_VS_ADDR, 10);
   Wr(STH1_VE_ADDR, 10+1024);

   Wr(STH2_HS_ADDR, 95+2);
   Wr(STH2_HE_ADDR, 96+2);
   Wr(STH2_VS_ADDR, 10);
   Wr(STH2_VE_ADDR, 10+1024);

   
   Wr(OEH_HS_ADDR, 1382);
   Wr(OEH_HE_ADDR, 1386);
   Wr(OEH_VS_ADDR, 10);
   Wr(OEH_VE_ADDR, 10+1024);

   Wr(VCOM_HSWITCH_ADDR, 8);
   Wr(VCOM_VS_ADDR, 5);
   Wr(VCOM_VE_ADDR, 1205);

   Wr(CPV1_HS_ADDR, 4);
   Wr(CPV1_HE_ADDR, 700);
   Wr(CPV1_VS_ADDR, 2);
   Wr(CPV1_VE_ADDR, 1);

   Wr(CPV2_HS_ADDR, 4);
   Wr(CPV2_HE_ADDR, 700);
   Wr(CPV2_VS_ADDR, 2);
   Wr(CPV2_VE_ADDR, 1);

   Wr(STV1_HS_ADDR, 2);
   Wr(STV1_HE_ADDR, 1);
   Wr(STV1_VS_ADDR, 10);
   Wr(STV1_VE_ADDR, 11);

   Wr(STV2_HS_ADDR, 2);
   Wr(STV2_HE_ADDR, 1);
   Wr(STV2_VS_ADDR, 10);
   Wr(STV2_VE_ADDR, 11);

   Wr(OEV1_HS_ADDR, 1400);
   Wr(OEV1_HE_ADDR, 1);
   Wr(OEV1_VS_ADDR, 10);
   Wr(OEV1_VE_ADDR, 11+1024);

   Wr(OEV2_HS_ADDR, 1400);
   Wr(OEV2_HE_ADDR, 1);
   Wr(OEV2_VS_ADDR, 10);
   Wr(OEV2_VE_ADDR, 11+1024);

   Wr(OEV3_HS_ADDR, 1400);
   Wr(OEV3_HE_ADDR, 1);
   Wr(OEV3_VS_ADDR, 10);
   Wr(OEV3_VE_ADDR, 11+1024);

   Wr(INV_CNT_ADDR, (1 << INV_EN) |
                     (12 << INV_CNT));

}

void set_lvds_tcon_1280x1024(void)
{
   Wr(L_STH1_HS_ADDR, 95+2);
   Wr(L_STH1_HE_ADDR, 96+2);
   Wr(L_STH1_VS_ADDR, 10);
   Wr(L_STH1_VE_ADDR, 10+1024);

   Wr(L_STH2_HS_ADDR, 95+2);
   Wr(L_STH2_HE_ADDR, 96+2);
   Wr(L_STH2_VS_ADDR, 10);
   Wr(L_STH2_VE_ADDR, 10+1024);

   
   Wr(L_OEH_HS_ADDR, 1382);
   Wr(L_OEH_HE_ADDR, 1386);
   Wr(L_OEH_VS_ADDR, 10);
   Wr(L_OEH_VE_ADDR, 10+1024);

   Wr(L_VCOM_HSWITCH_ADDR, 8);
   Wr(L_VCOM_VS_ADDR, 5);
   Wr(L_VCOM_VE_ADDR, 1205);

   Wr(L_CPV1_HS_ADDR, 4);
   Wr(L_CPV1_HE_ADDR, 700);
   Wr(L_CPV1_VS_ADDR, 2);
   Wr(L_CPV1_VE_ADDR, 1);

   Wr(L_CPV2_HS_ADDR, 4);
   Wr(L_CPV2_HE_ADDR, 700);
   Wr(L_CPV2_VS_ADDR, 2);
   Wr(L_CPV2_VE_ADDR, 1);

   Wr(L_STV1_HS_ADDR, 2);
   Wr(L_STV1_HE_ADDR, 1);
   Wr(L_STV1_VS_ADDR, 10);
   Wr(L_STV1_VE_ADDR, 11);

   Wr(L_STV2_HS_ADDR, 2);
   Wr(L_STV2_HE_ADDR, 1);
   Wr(L_STV2_VS_ADDR, 10);
   Wr(L_STV2_VE_ADDR, 11);

   Wr(L_OEV1_HS_ADDR, 1400);
   Wr(L_OEV1_HE_ADDR, 1);
   Wr(L_OEV1_VS_ADDR, 10);
   Wr(L_OEV1_VE_ADDR, 11+1024);

   Wr(L_OEV2_HS_ADDR, 1400);
   Wr(L_OEV2_HE_ADDR, 1);
   Wr(L_OEV2_VS_ADDR, 10);
   Wr(L_OEV2_VE_ADDR, 11+1024);

   Wr(L_OEV3_HS_ADDR, 1400);
   Wr(L_OEV3_HE_ADDR, 1);
   Wr(L_OEV3_VS_ADDR, 10);
   Wr(L_OEV3_VE_ADDR, 11+1024);

   Wr(L_INV_CNT_ADDR, (1 << INV_EN) |
                     (12 << INV_CNT));

}

void set_tv_enc_720x480i (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    stimulus_print ("Error: Don's support 480i in encl\n");
#else
    config_tv_enc(TV_ENC_480i);
#endif
    if (viu1_sel) { // 1=Connect to ENCI, 0=Not connect to ENCI
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 1, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 1=Connect to ENCI, 0=Not connect to ENCI
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 1, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCI_VIDEO_EN, 1); // Enable Interlace video encoder
    }
}

void set_tv_enc_720x576i (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    stimulus_print ("Error: Don's support 576i in encl\n");
#else
    config_tv_enc(TV_ENC_576i);
#endif
    if (viu1_sel) { // 1=Connect to ENCI, 0=Not connect to ENCI
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 1, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 1=Connect to ENCI, 0=Not connect to ENCI
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 1, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCI_VIDEO_EN, 1); // Enable Interlace video encoder
    }
}

void set_tv_enc_1920x1080p (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
   set_tv_enc_lcd (TV_ENC_LCD1920x1080);
#else
    config_tv_enc(TV_ENC_1080p);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}

void set_tv_enc_1920x2205p (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
   set_tv_enc_lcd (TV_ENC_LCD1920x2205);
#else
    config_tv_enc(TV_ENC_2205p);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif        
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}

void set_tv_enc_1280x720p (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
   set_tv_enc_lcd (TV_ENC_LCD1280x720);
#else
    config_tv_enc(TV_ENC_720p);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}

void set_tv_enc_1920x1080i (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    stimulus_print ("Error: Don's support 1080i in encl\n");
#else
    config_tv_enc(TV_ENC_1080i);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 1=Connect to ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
    }
}

void set_tv_encl (tv_enc_lcd_type_t output_type, int viu1_sel, int viu2_sel, int enable)
{
    config_tv_enc_lcd(output_type);

    if (viu1_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu1 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu2 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCL_VIDEO_EN, 1); // Enable VENCL
    }
}

void set_tv_encl_480x234 (int viu1_sel, int viu2_sel, int enable)
{
    config_tv_enc_lcd(TV_ENC_LCD480x234);

    if (viu1_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu1 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu2 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCL_VIDEO_EN, 1); // Enable VENCL
    }
}

void set_tv_encl_240x160 (int viu1_sel, int viu2_sel, int enable)
{
    config_tv_enc_lcd(TV_ENC_LCD240x160);

    if (viu1_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu1 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu2 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCL_VIDEO_EN, 1); // Enable VENCL
    }
}

void set_tv_encl_720x480 (int viu1_sel, int viu2_sel, int enable)
{
    config_tv_enc_lcd(TV_ENC_LCD720x480);

    if (viu1_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu1 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (viu2_sel) { // 0=Connect to ENCL
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    } else {        // Disable viu2 by select non-exist enct
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 3, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=No connection, 1=ENCI, 2=ENCP, 3=ENCT.
    }
    if (enable) {
        Wr(ENCL_VIDEO_EN, 1); // Enable VENCL
    }
}

void set_tv_enc_3840x2160p_vic01 (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    set_tv_enc_lcd(TV_ENC_LCD3840x2160p_vic01);
#else
    config_tv_enc(TV_ENC_3840x2160p_vic01);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}


void set_tv_enc_3840x2160p_vic03 (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    set_tv_enc_lcd(TV_ENC_LCD3840x2160p_vic03);
#else
    config_tv_enc(TV_ENC_3840x2160p_vic03);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}

void set_tv_enc_4096x2160p_vic04 (int viu1_sel, int viu2_sel, int enable)
{
#ifdef NO_ENCP
    set_tv_enc_lcd(TV_ENC_LCD4096x2160p_vic04);
#else
    config_tv_enc(TV_ENC_4096x2160p_vic04);
#endif
    if (viu1_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 0, 2); // [1:0] cntl_viu1_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (viu2_sel) { // 1=Connect to ENCP
#ifdef NO_ENCP
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 0, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#else
        Wr_reg_bits(VPU_VIU_VENC_MUX_CTRL, 2, 2, 2); // [3:2] cntl_viu2_sel_venc: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
#endif
    }
    if (enable) {
#ifdef NO_ENCP
        Wr(ENCL_VIDEO_EN, 1);
#else
        Wr(ENCP_VIDEO_EN, 1); // Enable Interlace video encoder
#endif
    }
}

