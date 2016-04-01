CC     = $(CROSS_COMPILE)gcc
STRIP  = $(CROSS_COMPILE)strip
CXX    = $(CROSS_COMPILE)gcc
AS     = $(CROSS_COMPILE)as
LD     = $(CROSS_COMPILE)ld
RANLIB = $(CROSS_COMPILE)ranlib
RM     = delete

VERSION = 52
REVISION= 1

# Change these as required
OPTIMIZE= -O0
DEBUG   = -ggdb -gstabs
CFLAGS  = -W -Wall -D__USE_INLINE__ -mcrt=newlib  -Wwrite-strings $(DEBUG)


# This can be openjdk|gnuclasspath
JAVA_RUNTIME_LIBRARY = gnuclasspath


# -mcrt=newlib
# Flags passed to gcc during linking
LINK    = -mcrt=newlib $(DEBUG)

