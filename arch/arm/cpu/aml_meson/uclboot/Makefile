

include $(TOPDIR)/config.mk

LIB	= $(obj)ucl_lib_$(CPU).o

START	= start.o
OBJS	= misc.o piggy.o interrupts.o

OBJS	:= $(addprefix $(obj),$(OBJS))
START	:= $(addprefix $(obj),$(START))
AFLAGS_$(BCURDIR)+=-DPIGGY_UCL=\"$(OBJTREE)/piggy.ucl\"
all:	$(obj).depend $(START) $(LIB)
$(obj)/piggy.o:$(obj)/piggy.s $(OBJTREE)/piggy.ucl
$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
