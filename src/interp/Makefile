##
## Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
## Robert Lougher <rob@jamvm.org.uk>.
##
## This file is part of JamVM.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2,
## or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##
include ../../Config.mk
SUBDIRS = engine
TARGET = libinterp.o
noinst_LTLIBRARIES   = libinterp.o
libinterp_la_SOURCES = direct.c inlining.c
# inlining.h shared.h

libinterp_la_LIBADD  = engine/libengine.o

INCLUDES = -I. -I../ -I../os/amiga -I./engine -I../classlib/gnuclasspath
#if COMPILE_TIME_RELOC_CHECKS
#inlining.lo: relocatability.inc
#endif

#relocatability.inc: engine/compute_relocatability
#    engine/compute_relocatability

AM_CPPFLAGS = -I../ -I$(top_srcdir)/src -I$(top_srcdir)/src/interp/engine -I$(top_builddir)/src -I$(top_builddir)/classlib/gnuclasspath

CLEANFILES = relocatability.inc
SRCS = $(libinterp_la_SOURCES)

# os/amiga/os.c os/amiga/scandir.c\
#    os/amiga/powerpc/init.c os/amiga/powerpc/callNative.S os/amiga/powerpc/dll_md.c\#
#    interp/engine/interp.c interp/engine/interp2.c \
#    jam.c jni.c alloc.c access.c cast.c class.c dll.c excep.c execute.c frame.c\
#    hash.c hooks.c init.c lock.c natives.c properties.c reflect.c resolve.c\
#    shutdown.c string.c symbol.c thread.c time.c utf8.c zip.c
#    interp/direct.c interp/inlining.c\


# -------------------------------------------------------------
# Nothing should need changing below this line

OBJS = $(SRCS:.c=.o)

# Rules for building
$(TARGET) : $(OBJS)
	$(LD) -nostartfiles -r -o $(TARGET) $(OBJS) $(LIBS)  $(INCLUDES)

#$(CC) -nostartfiles $(CFLAGS) -r -o $(TARGET) $(OBJS) $(LIBS)
#$(STRIP) -R.comment $(TARGET).debug -o $(TARGET)
#-Wl,--cref,-M,-Map=$@.map

%.o:%.c
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TARGET).debug $(TARGET).map #?.o
	@echo "Done!!"

#.PHONY: revision
#revision:
#    bumprev -v $(VERSION) -r $(REVISION) -n $(TARGET) -b ADM8511

