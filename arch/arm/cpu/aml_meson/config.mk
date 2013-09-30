CROSS_COMPILE=arm-none-eabi-
ARM_CPU=cortex-a9
PLATFORM_CPPFLAGS += $(call cc-option,-mcpu=cortex-a9  -ffixed-r8 -mno-long-calls  -Wall -fPIC )
#USE_PRIVATE_LIBGCC=yes
ifeq ($(CONFIG_M8),y)
TEXT_BASE=0x10000000
UCL_TEXT_BASE=0x10000000
else #not NOT M8
TEXT_BASE=0x8F800000
UCL_TEXT_BASE=0x8F000000
endif #ifeq ($(CONFIG_M8),y)
#$(warning $(PLATFORM_CPPFLAGS))


