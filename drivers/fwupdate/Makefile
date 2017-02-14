###########################

include $(TOPDIR)/config.mk

LIB	:= $(obj)libfwup.o

COBJS-y	+= fwupdate.o
COBJS-y	+= cmd_detect_gpio_key.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
