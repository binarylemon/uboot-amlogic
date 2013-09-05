#include <common.h>
//FILE: c_stimulus.c
//~ #include <stdlib.h>
//~ #include <stdio.h>
#include <stdarg.h>

#ifdef  INCLUDE_ONLY_FILENAME
    #include "register.h"
#else
        #ifdef  STB_LOCAL_COMPILE
            #include "../register.h"
        #else
           // #include "../../../../ucode/register.h"
            #include "register.h"
        #endif
#endif

#ifdef AUDIO_ARC
    #include "c_arc_pointer_reg.h"
#endif

#include "c_always_on_pointer.h"

#define PRINT_MAX_CHARS  80   // in c_stimulus.v

// 
void    stimulus_event( unsigned long event_num, unsigned long data );
int     am_printf(char *paddr, const char *fmt, ...);

#define stimulus_finish_pass()       stimulus_event( 99, 0 )
#define stimulus_finish_fail(val)    stimulus_event( 98, val )

void    stimulus_print( char *pStr )
{
}

// ----------------------------------------------
void    stimulus_print_without_timestamp( char *pStr )
{
}

// ----------------------------------------------
void    stimulus_print_num_hex( unsigned long data )
{
}

// ----------------------------------------------
void    stimulus_print_num_dec( unsigned long data )
{
}

// ----------------------------------------------
void    stimulus_display(char* fmt, unsigned long data )
{
}

void    stimulus_display2(char* fmt, unsigned long dat1, unsigned long dat2)
{
}


char* print_char(char *ptr, char ch);
char *myitoa(int num, char *str, int radix);


void st_printf(const char *fmt, ...)
{
}


int am_printf(char *paddr, const char *fmt, ...)
{
} // st_printf


char *myitoa(int num, char *str, int radix)
{
    char index[50]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum; 
    int i=0,j,k;
    
    if(radix==10 && num<0)
    {
        unum = (unsigned)-num;
        str[i++]='-';
    }
    else unum = (unsigned)num; 
    
    do
    {
        str[i++] = index[unum%(unsigned)radix];
        unum /= radix;
    } while(unum > 0);
    str[i]='\0';

    if(str[0] == '-') k = 1;
    else k = 0;
    
    char temp;
    for(j=k; j<=(i-k-1)/2; j++)
    {
        temp = str[j];
        str[j] = str[i-j-1];
        str[i-j-1] = temp;
    }
    return str;
}



// You can send the 'ch' to memory or other devices
char* print_char(char *ptr, char ch)
{
    *ptr++ = ch;
    return(ptr);
}

// ----------------------------------------------
void    stimulus_event( unsigned long event_num, unsigned long data )
{
    return;
    #ifdef AUDIO_ARC
        // load the value to print
        *P_ISA_DEBUG_REG0 = data;
        // trigger a print string action
        *P_ISA_DEBUG_REG1 = (1 << 31) | (event_num & 0xFF);
        *P_ISA_DEBUG_REG1 = 0;
    #else
        // load the value to print
        *P_AO_DEBUG_REG0 = data;
        // trigger a print string action
        *P_AO_DEBUG_REG1 = (1 << 31) | (event_num & 0xFF);
        *P_AO_DEBUG_REG1 = 0;
    #endif
}
// ----------------------------------------------
void    stimulus_wait_event_done( unsigned long event_num )
{
    return;
    // TODO use Always on?
    while( *P_AO_DEBUG_REG2 & (1 << event_num) ) {}
}

// ----------------------------------------------
// $top/common/c_stimulus.v will force a register to send a 32-bit value
unsigned long    stimulus_get_verilog_data( )
{
    return ;
    return( Rd(ISA_DEBUG_REG3) );
}

// don't delete this routine. It is for syscall. It is necessary for using stdlib.
int* _sbrk(int incr) 
{
    return (int*)0;
}

