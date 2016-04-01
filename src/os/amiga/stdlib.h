
#ifndef JAMIGA_STDLIB_H
#define JAMIGA_STDLIB_H

#include <proto/exec.h>
#include <proto/timer.h>
#include <sys/time.h>
#ifdef _SYS_CLIB2_STDC_H
#include "/SDK/clib2/include/stdlib.h"
#else
#include "/SDK/newlib/include/stdlib.h"
#endif

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_PRIVATE 3
#define MAP_ANON 4
#define MAP_FAILED NULL
#define MT_MALLOC 1
#define mmap(addr, length, prot, flags, fd, offset) mymmap(addr, length, prot, flags, fd, offset, __FILE__, __LINE__)
#define munmap(addr,size) mymunmap(addr, size, __FILE__, __LINE__)
//#define malloc(x) mymalloc(x, MT_MALLOC, __FILE__, __LINE__)
//#define realloc(oldptr, size) myrealloc(oldptr, size, __FILE__, __LINE__)
//#define free(x) myfree(x, MT_MALLOC, __FILE__, __LINE__)



#endif
