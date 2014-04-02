
include $(TOPDIR)/config.mk
ARMCC_$(CONFIG_ENABLE_ARMCC_DEBUGROM)=$(shell echo __CC_ARM | armcc -E -  | tail -n 1)
ifneq ($(ARMCC_y),)
targetComplier=arm
else
targetComplier=gcc
endif


all:	  
ifdef CONFIG_M8
	./bin2code m8_smp_code $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/m8_smp.bin $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/m8_smp.dat
endif
	$(MAKE) -f Makefile.$(targetComplier)
	$(MAKE) -f Makefile.ddr_init
ifdef CONFIG_AML_SECU_BOOT_V2
	$(MAKE) -f Makefile.usb
endif
ifeq ($(CONFIG_CMD_RUNARC),y)
	$(MAKE) -f Makefile.rbt
endif

ifeq ($(CONFIG_AML_SUSPEND),y)
ifneq ($(CONFIG_MESON_TRUSTZONE),y)
	$(MAKE) -f Makefile.krbt
endif
endif
