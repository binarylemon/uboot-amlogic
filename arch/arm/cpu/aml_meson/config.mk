CROSS_COMPILE=arm-none-eabi-
ARM_CPU=cortex-a9
PLATFORM_CPPFLAGS += $(call cc-option,-mcpu=cortex-a9  -ffixed-r8 -mno-long-calls  -Wall -fPIC )
#USE_PRIVATE_LIBGCC=yes
TEXT_BASE=0x8F800000
UCL_TEXT_BASE=0x8F000000
#$(warning $(PLATFORM_CPPFLAGS))


