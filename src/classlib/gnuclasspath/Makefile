#CC = SDK:gcc/bin/gcc
#SRCS = os/amiga/os.c os/amiga/scandir.c

#OS_AMIGA_POWERPC = os/amiga/powerpc/callNative.c os/amiga/powerpc/init.c os/amiga/powerpc/dll_md.c

#INTERP_OBJ = interp/direct.c interp/inlining.c

#INTERP_ENGINE_OBJ = interp/engine/interp.c interp/engine/interp2.c interp/engine/relocatability.c

#OBJ = jni.c alloc.c access.c cast.c class.c dll.c excep.c execute.c frame.c\
#    hash.c hooks.c init.c lock.c natives.c properties.c reflect.c resolve.c\
#    shutdown.c string.c symbol.c thread.c time.c utf8.c zip.c

#-ldl -lauto -lm

#gcc -g -O2 -o jamvm jam.o

include ../../../Config.mk

#CC     = $(CROSS_COMPILE)gcc
#STRIP  = $(CROSS_COMPILE)strip
#CXX    = $(CROSS_COMPILE)gcc
#AS     = $(CROSS_COMPILE)as
#LD     = $(CROSS_COMPILE)ld
#RANLIB = $(CROSS_COMPILE)ranlib
#RM     = delete

#VERSION = 52
#REVISION= 1

# Change these as required
#OPTIMIZE= -O0
#DEBUG   = -ggdb
#CFLAGS  = -mcrt=newlib -W -Wall -Wwrite-strings $(OPTIMIZE)
#CFLAGS  = -W -Wall -mcrt=clib2 -Wwrite-strings $(DEBUG)  -D__USE_INLINE__

# Flags passed to gcc during linking  -D__USE_INLINE__
LINK    = $(DEBUG) -nostartfiles

# Name of the "thing" to build
TARGET  = libclasslib.o

# Additional linker libraries
LIBS =

CLASSPATH = gnuclasspath
AM_CPPFLAGS = 
#INCLUDES = -I$(top_builddir)/src -I$(top_srcdir)/src
INCLUDES = -I. -I../../ -I../../os/amiga/
# -I./os/amiga -I./interp -I./interp/engine -I/JAmiga/include/

libclasslib_la_SOURCES = thread.c class.c natives.c excep.c reflect.c \
						 dll.c jni.c properties.c annotations.c frame.c alloc.c
libclasslib_la_INC = classlib.h classlib-symbol.h classlib-defs.h \
						 gnuclasspath.h ../../config.h

# Source code files used in this project
# Add any additional files to this line

#SRCS = wbrestorer.cpp WorkbenchProgramList.cpp WBRestoreGUI.cpp library_error.cpp
SRCS = $(libclasslib_la_SOURCES)

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
# $(libclasslib_la_INC:.h=.o)
# Rules for building
$(TARGET) : $(OBJS) $(libclasslib_la_INC)
	$(LD) -r -o $(TARGET) $(OBJS) $(LIBS)
#$(STRIP) -R.comment $(TARGET).debug -o $(TARGET)
#-Wl,--cref,-M,-Map=$@.map

%.o:%.c
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@

%.o:%.h
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TARGET).debug $(TARGET).map #?.o
	@echo "Done!!"

.PHONY: revision
revision:
	bumprev -v $(VERSION) -r $(REVISION) -n $(TARGET) -b ADM8511

