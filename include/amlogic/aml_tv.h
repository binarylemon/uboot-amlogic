#ifndef TVOUT_H
#define TVOUT_H

/* same to tv encoder */
enum
{
    TVOUT_480I  = 0,
    TVOUT_480CVBS,
    TVOUT_480P  ,
    TVOUT_576I  ,
    TVOUT_576CVBS,
    TVOUT_576P  ,
    TVOUT_720P  ,
    TVOUT_1080I ,
    TVOUT_1080P ,
    TVOUT_MAX   
};

typedef enum {
    VMODE_480I  = 0,
    VMODE_480CVBS,
    VMODE_480P  ,
    VMODE_576I   ,
    VMODE_576CVBS   ,
    VMODE_576P  ,
    VMODE_720P  ,
    VMODE_1080I ,
    VMODE_1080P ,
    VMODE_720P_50HZ ,
    VMODE_1080I_50HZ ,
    VMODE_1080P_50HZ ,
    VMODE_LCD   ,
    VMODE_MAX,
    VMODE_INIT_NULL,
} vmode_t;

#define TVOUT_VALID(m) (m < TVOUT_MAX)

int tv_out_open(int mode);
int tv_out_close(void);
int tv_out_cur_mode(void);
int tv_out_get_info(int mode, unsigned *width, unsigned *height);

typedef struct tv_operations {
	void  (*enable)(void);
	void  (*disable)(void);
	void  (*power_on)(void);
	void  (*power_off)(void);
} tv_operations_t;

extern tv_operations_t tv_oper;


#endif