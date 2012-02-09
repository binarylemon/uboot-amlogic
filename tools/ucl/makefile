
UTILOBJS = $(patsubst %.c,objs/%.o,$(wildcard src/*.c))

CC=gcc
AR=ar
LD=gcc
RM=rm -rf 
MKDIR=@mkdir

CFLAGS = -g -O2 -Wall -Iinclude -I.
LDFLAGS = -Iinclude -I.
SYSLIB = -lwinmm 

all: $(UTILOBJS)
	$(AR) r libucl.a $^


objs/%.o : %.cpp
	$(MKDIR) -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@
	
objs/%.o : %.c
	$(MKDIR) -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) objs
	rm *.exe




