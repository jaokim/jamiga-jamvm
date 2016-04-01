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

#cd os; make Makefile
#include interp/Makefile
#include classlib/Makefile

include ../Config.mk 

# Name of the "thing" to build
TARGET  = jamvm

#INTERPRETER = ./interp/engine/libengine.o
INTERPRETER = ./interp/engine/libengine.o ./interp/libinterp.o
#INTERPRETER = ./interp/engine/relocatability.o  ./interp/direct.o ./interp/inlining.o

JAMVM_LIBS = $(INTERPRETER) ./classlib/gnuclasspath/libclasslib.o ./os/amiga/osobj.o ./os/amiga/powerpc/ppc.o

all : arch.h libinterp.o libengine.o libclasslib.o osobj.o ppc.o $(TARGET)

arch.h : ./arch/powerpc.h
	makelink arch.h FILE=arch/powerpc.h

libengine.o :
	make -C ./interp/engine/

libinterp.o :
	make -C ./interp/

libclasslib.o :
	make -C classlib/gnuclasspath

osobj.o :
	make -C os/amiga

ppc.o:
	make -C os/amiga/powerpc


# Additional linker libraries
LIBS = -lauto -lm -lz
#-lpthread

CLASSPATH = gnuclasspath

INCLUDES = -I. -I./os/amiga -I./interp -I./interp/engine -I./classlib/$(JAVA_RUNTIME_LIBRARY)/
# -I/JAmiga/include/


libcore_la_SOURCES = alloc.c cast.c class.c dll.c excep.c execute.c \
					 hash.c jni.c lock.c jam.c\
					 natives.c reflect.c resolve.c sig.h string.c thread.c \
					 utf8.c zip.c properties.c\
					 dll_ffi.c access.c frame.c init.c hooks.c \
					 symbol.c shutdown.c time.c \
					 sig.c stubs.c \
					 jni-stubs.c
#                     interp/direct.c interp/inlining.c interp/inlining.h interp/shared.h
# frame.h       alloc.h          hash.h jam.h   lock.h  thread.h u       natives.h     reflect.h      zip.h
#symbol.h                     jni-internal.h properties.h    stubs.h         class.h excep.h
# Source code files used in this project
# Add any additional files to this line

#SRCS = wbrestorer.cpp WorkbenchProgramList.cpp WBRestoreGUI.cpp library_error.cpp
SRCS = $(libcore_la_SOURCES)

# os/amiga/os.c os/amiga/scandir.c\
#    os/amiga/powerpc/init.c os/amiga/powerpc/callNative.S os/amiga/powerpc/dll_md.c\#
#    interp/engine/interp.c interp/engine/interp2.c \
#    jam.c jni.c alloc.c access.c cast.c class.c dll.c excep.c execute.c frame.c\
#    hash.c hooks.c init.c lock.c natives.c properties.c reflect.c resolve.c\
#    shutdown.c string.c symbol.c thread.c time.c utf8.c zip.c
#    interp/direct.c interp/inlining.c\

cleanall:
	run make Makefile clean
	run make -C interp Makefile clean
	run make -C interp/engine Makefile clean
	run make -C classlib/gnuclasspath Makefile clean
	run make -C os/amiga Makefile clean




# -------------------------------------------------------------
# Nothing should need changing below this line

OBJS = $(SRCS:.c=.o)
# Rules for building
$(TARGET) : $(JAMVM_LIBS) $(OBJS)
	$(CC) $(LINK) -o $(TARGET) $(OBJS) $(JAMVM_LIBS) $(LIBS)  $(INCLUDES)

#$(STRIP) -R.comment $(TARGET).debug -o $(TARGET)
#-Wl,--cref,-M,-Map=$@.map

%.o:%.c
	$(CC) -c $<  $(CFLAGS) $(INCLUDES) -o $@


.PHONY: clean
clean:
	$(RM) $(TARGET) $(TARGET).debug $(TARGET).map #?.o
	@echo "Done!"


.PHONY: revision
revision:
	bumprev -v $(VERSION) -r $(REVISION) -n $(TARGET) -b ADM8511

