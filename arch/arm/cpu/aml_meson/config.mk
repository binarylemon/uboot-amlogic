CROSS_COMPILE ?= arm-none-eabi-
ARM_CPU=cortex-a5
PLATFORM_CPPFLAGS += $(call cc-option,-mcpu=cortex-a5  -ffixed-r8 -mno-long-calls  -Wall -fPIC) -Werror
#USE_PRIVATE_LIBGCC=yes
TEXT_BASE=$(CONFIG_SYS_TEXT_BASE)
UCL_TEXT_BASE=$(CONFIG_SYS_UCL_TEXT_BASE)
#$(warning $(PLATFORM_CPPFLAGS))
