#ifndef C_STIMULUS_H
#define C_STIMULUS_H

extern void             stimulus_print(char *pStr);
extern void             stimulus_print_without_timestamp(char *pStr);
extern void             stimulus_print_num_hex(unsigned long data);
extern void             stimulus_print_num_dec(unsigned long data);
extern void             stimulus_event(unsigned long event_num, unsigned long data);
extern void             stimulus_wait_event_done(unsigned long event_num);
extern unsigned long    stimulus_get_verilog_data( void );
extern void             stimulus_display(char* fmt, unsigned long data );
extern void             stimulus_display2(char* fmt, unsigned long dat1, unsigned long dat2);
extern void             st_printf(const char* fmt, ...);
extern int              am_printf(char *paddr, const char* fmt, ...);

#define stimulus_finish_pass()    stimulus_event( 99, 0 )
#define stimulus_finish_fail(val) stimulus_event( 98, val )

#endif
