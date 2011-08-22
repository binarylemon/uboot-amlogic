#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk
include $(OBJTREE)/include/autoconf.mk

LIB				= libsystest.o
GPLIB-$(CONFIG_SYS_TEST)	+= libgentest.o
COBJS-$(CONFIG_SYS_TEST)	+= systest.o
COBJS-$(CONFIG_SYS_TEST)	+= tests.o
COBJS-$(CONFIG_SYS_TEST)	+= log.o

SPLIB-$(CONFIG_SYS_TEST) = drivers/libtestdrivers.o
SPLIB-$(CONFIG_SYS_TEST) += $(shell if [ -d board/$(BOARD) ]; then echo \
			    "board/$(BOARD)/libtest$(BOARD).o"; fi)
SPLIB-$(CONFIG_SYS_TEST) += $(shell if [ -d lib_$(ARCH) ]; then echo \
			    "lib_$(ARCH)/libtest$(ARCH).o"; fi)
SPLIB-$(CONFIG_SYS_TEST) += $(shell if [ -d cpu/$(SOC) ]; then echo \
			    "cpu/$(SOC)/libtest$(SOC).o"; fi)

GPLIB	:= $(addprefix $(obj),$(GPLIB-y))
SPLIB	:= $(addprefix $(obj),$(SPLIB-y))
COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
LIB	:= $(obj)$(LIB)

all:	$(LIB)
	echo lib_$(ARCH)

systestdeps:
	@for lib in $(SPLIB-y) ; do \
		$(MAKE) -C `dirname $$lib` all ; \
	done

# generic systest library
$(GPLIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# specific systest libraries
$(SPLIB): $(obj).depend systestdeps
	$(MAKE) -C $(dir $(subst $(obj),,$@))

# the systest lib archive
$(LIB): $(GPLIB) $(SPLIB)
	$(call cmd_link_o_target, $^)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
