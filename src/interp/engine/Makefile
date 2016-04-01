
include ../../../Config.mk

#CC     = $(CROSS_COMPILE)gcc
#STRIP  = $(CROSS_COMPILE)strip
#CXX    = $(CROSS_COMPILE)gcc
#AS     = $(CROSS_COMPILE)as
#LD     = $(CROSS_COMPILE)ld
#RANLIB = $(CROSS_COMPILE)ranlib
#RM     = delete

#CFLAGS  = -mcrt=clib2 -W -Wall -D__USE_INLINE__  -Wwrite-strings $(DEBUG)

top_srcdir = ../../..
top_builddir = ../../..
TARGET    = libengine.o
#DEBUG   = -ggdb

# For "normal" interpreter
#libengine_la_SOURCES  = interp.c interp2.c relocatability.c
# For DIRECT INLINING THREADED intepreter
#libengine_la_SOURCES  = interp2.c
libengine_la_SOURCES = interp.c interp2.c relocatability.c

SRCS = $(libengine_la_SOURCES)

#AM_CFLAGS   = @interp_cflags@
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/interp -I$(top_builddir)/src -I$(top_builddir)/src/classlib/$(JAVA_RUNTIME_LIBRARY)

# -------------------------------------------------------------
# Nothing should need changing below this line

OBJS = $(SRCS:.c=.o)
# Rules for building
$(TARGET) : $(OBJS)
	$(LD) -nostartfiles -r -o $(TARGET) $(OBJS) $(LIBS)  $(INCLUDES)
#$(STRIP) -R.comment $(TARGET).debug -o $(TARGET)
#-Wl,--cref,-M,-Map=$@.map

compute_reloc.exe : compute_relocatability.o $(SRCS:.c=.o)
	$(CC) $(LINK) -o compute_reloc.exe  relocatability.o compute_relocatability.o interp.o interp2.o


%.o:%.c
	$(CC) -r -c $<  $(CFLAGS) $(INCLUDES) -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TARGET).debug $(TARGET).map #?.o
	@echo "Done!!"

.PHONY: revision
revision:
	bumprev -v $(VERSION) -r $(REVISION) -n $(TARGET) -b ADM8511


