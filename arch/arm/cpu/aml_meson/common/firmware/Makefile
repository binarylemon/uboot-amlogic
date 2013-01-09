
include $(TOPDIR)/config.mk
ARMCC_$(CONFIG_ENABLE_ARMCC_DEBUGROM)=$(shell echo __CC_ARM | armcc -E -  | tail -n 1)
ifneq ($(ARMCC_y),)
targetComplier=arm
else
targetComplier=gcc
endif

all:	  
	$(MAKE) -f Makefile.$(targetComplier)
	$(MAKE) -f Makefile.ddr_init
ifeq ($(CONFIG_CMD_RUNARC),y)
	$(MAKE) -f Makefile.rbt
endif

ifeq ($(CONFIG_AML_SUSPEND),y)
	$(MAKE) -f Makefile.krbt
endif
