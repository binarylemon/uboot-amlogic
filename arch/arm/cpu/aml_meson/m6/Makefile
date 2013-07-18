#
# (C) Copyright 2000-2011
# Jerry Yu , Amlogic  Engineering, jerry.yu@amlogic.com.
#
# (C) Copyright 2011 Amlogic Semiconductor, Inc.
#
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-$(CONFIG_AML_SDIO)+=sdio.o
COBJS-$(CONFIG_CMD_RUNARC)+=cmd_arc.o
COBJS	= clock.o gpio.o pinmux.o $(COBJS-y)
SOBJS := mmutable.o

COBJS-$(CONFIG_AML_ETHERNET) += aml_eth_clk_set.o aml_eth_pinmux.o

COBJS-$(CONFIG_AML_RTC)+=aml_rtc_setting.o

COBJS-$(CONFIG_USB_DWC_OTG_HCD) += usb.o

COBJS-$(CONFIG_VIDEO_AMLLCD) += lcdoutc.o

ifdef CONFIG_AML_SUSPEND
COBJS += suspend.o
ifndef CONFIG_MESON_TRUSTZONE
COBJS += firmwareld.o
endif
endif

ifdef CONFIG_MESON_TRUSTZONE
COBJS += trustzone.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

all:	power_firmware $(obj).depend $(LIB)

$(LIB):	$(OBJS)	
	$(call cmd_link_o_target, $(OBJS))	

ifndef CONFIG_MESON_TRUSTZONE

ifdef CONFIG_AML_SUSPEND
FIRMWARE_AUTO_UPDATE=y
export FIRMWARE_AUTO_UPDATE
power_firmware:
		make -C $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/arc_power
		make -C $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/appf
ifdef CONFIG_CMD_RUNARC
#		make -C $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/arc_testpd
endif
else
power_firmware:
ifdef CONFIG_CMD_RUNARC
#		make -C $(SRCTREE)/$(CPUDIR)/$(SOC)/firmware/arc_testpd
endif
endif

else
power_firmware:
endif


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
