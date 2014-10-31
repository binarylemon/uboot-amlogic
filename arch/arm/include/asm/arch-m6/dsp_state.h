#ifndef _DSP_STATE_H_
#define _DSP_STATE_H_


#define DSP_CURRENT_RUN		0x44535052 	//"DSPR"
#define DSP_CURRENT_SLEEP	0x44535048	//"DSPH"
#define DSP_CURRENT_END		0x44535045	//"DSPE"
#define DSP_REQUST_START        0x44535053	//"DSPS"
#define DSP_REQUST_STOP 	0x44535054	//"DSPT"


#define write_reg(addr,val) (*((volatile unsigned int*)(addr)))=val
#define read_reg(addr) (*((volatile unsigned int*)(addr)))



#endif


