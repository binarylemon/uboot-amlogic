include $(TOPDIR)/config.mk

LIB-$(CONFIG_MESON_SECUREBOOT) = $(obj)libcrypto.a
LIB = $(LIB-y)

ARM_CPU = cortex-a9
SVN_REV = $(lastword $(subst M, ,$(subst :, ,$(shell (cd ..; (svnversion -c | grep -qv 'exported\|ÒÑµ¼³ö' && svnversion -c ) \
			|| git svn info | grep '^Revision: '| sed -e 's/^Revision: //' ) 2>/dev/null ))))
SVN_URL=$(shell ((svn info  || git svn info) | grep '^URL: '| sed -e 's/^URL: //') 2>/dev/null  ) r$(SVN_REV)
SVN_URL_SIZE= $(shell echo $(SVN_URL) | wc -c )
LD_LIB_PATH_ARM=$(dir $(shell $(CC) -print-libgcc-file-name)) \
									$(dir $(shell $(CC) -print-file-name=libc.a))
TRUST_CFLAGS=-DCONFIG_TRUSTZONE=1
CFLAGS = -I.  -I./include -I$(OBJTREE)/include -I$(OBJTREE)/include2 -I$(SRCTREE)/include -gdwarf-2  -mcpu=$(ARM_CPU)  $(TRUST_CFLAGS) -DSVN_URL="\"$(SVN_URL)\""  -Os -mno-long-calls -Wall  -MD 
LD_FLAGS = -r  $(LD_LIB_PATH_ARM:%=-L%) 

OBJ= decrypt.o 

OBJ := $(OBJ:%=$(obj)%)

$(LIB): $(OBJ) Makefile 
	$(AR) r $@ $(filter %.o,$^)
#	$(LD) -r -o $@ $(filter %.o,$^)
	

sinclude $(wildcard $(obj)*.d)