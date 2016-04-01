

include ../../../../Config.mk
#CC     = $(CROSS_COMPILE)gcc
#STRIP  = $(CROSS_COMPILE)strip
#CXX    = $(CROSS_COMPILE)gcc
#AS     = $(CROSS_COMPILE)as
#LD     = $(CROSS_COMPILE)ld
#RANLIB = $(CROSS_COMPILE)ranlib
#RM     = delete

#CFLAGS  = -mcrt=clib2 -W -Wall -D__USE_INLINE__  -Wwrite-strings $(DEBUG)

# Name of the "thing" to build
TARGET  = ppc.o

INCLUDES = -I../../../ -I./os/amiga -I./interp -I./interp/engine -I/JAmiga/include/ -I../../../classlib/$(JAVA_RUNTIME_LIBRARY)/

os_la_SOURCES = dll_md.c init.c  callNative.S

SRCS = $(os_la_SOURCES)

# -------------------------------------------------------------
# Nothing should need changing below this line

OBJS = $(SRCS:.c=.o)
# Rules for building
$(TARGET) : $(OBJS)
	$(CC) -nostartfiles -r -o $(TARGET) $(OBJS) $(LIBS) $(INCLUDES)

#$(STRIP) -R.comment $(TARGET).debug -o $(TARGET)
#-Wl,--cref,-M,-Map=$@.map

%.o:%.c
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@

callNative.o:callNative.S
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TARGET).debug $(TARGET).map #?.o
	@echo "Done."

.PHONY: revision
revision:
	bumprev -v $(VERSION) -r $(REVISION) -n $(TARGET) -b ADM8511


