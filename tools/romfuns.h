#include<stdio.h>
struct romboot_s {
    const char * cpu;
    int (* write)(FILE * spl,FILE * in ,FILE * out);
};
/* tools/m1_romboot.c */
int m1_write(FILE *spl, FILE *in, FILE *out);
int a3_write(FILE *spl, FILE *in, FILE *out);