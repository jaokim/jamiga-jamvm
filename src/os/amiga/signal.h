
#ifndef AMIGA_SIGNAL_H
#define AMIGA_SIGNAL_H

//#include "sys/signal.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/semaphores.h>
#include "pthread.h"

#define BYTETOBINARYPATTERN "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d.%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n"
#define BYTETOBINARY(byte)  \
  (byte & 0x80000000 ? 1 : 0), \
  (byte & 0x40000000 ? 1 : 0), \
  (byte & 0x20000000 ? 1 : 0), \
  (byte & 0x10000000 ? 1 : 0), \
  (byte & 0x8000000 ? 1 : 0), \
  (byte & 0x4000000 ? 1 : 0), \
  (byte & 0x2000000 ? 1 : 0), \
  (byte & 0x1000000 ? 1 : 0), \
  (byte & 0x800000 ? 1 : 0), \
  (byte & 0x400000 ? 1 : 0), \
  (byte & 0x200000 ? 1 : 0), \
  (byte & 0x100000 ? 1 : 0), \
  (byte & 0x80000 ? 1 : 0), \
  (byte & 0x40000 ? 1 : 0), \
  (byte & 0x20000 ? 1 : 0), \
  (byte & 0x10000 ? 1 : 0), \
  (byte & 0x8000 ? 1 : 0), \
  (byte & 0x4000 ? 1 : 0), \
  (byte & 0x2000 ? 1 : 0), \
  (byte & 0x1000 ? 1 : 0), \
  (byte & 0x0800 ? 1 : 0), \
  (byte & 0x0400 ? 1 : 0), \
  (byte & 0x0200 ? 1 : 0), \
  (byte & 0x0100 ? 1 : 0), \
  (byte & 0x0080 ? 1 : 0), \
  (byte & 0x0040 ? 1 : 0), \
  (byte & 0x0020 ? 1 : 0), \
  (byte & 0x0010 ? 1 : 0), \
  (byte & 0x0008 ? 1 : 0), \
  (byte & 0x0004 ? 1 : 0), \
  (byte & 0x0002 ? 1 : 0), \
  (byte & 0x0001 ? 1 : 0)

typedef unsigned long sigset_t;

typedef void (*_sig_func_ptr)(int);

/* These depend upon the type of sigset_t, which right now
   is always a long.. They're in the POSIX namespace, but
   are not ANSI. */
#define sigaddset(what,sig) (*(what) |= (1<<(sig)), 0)
#define sigdelset(what,sig) (*(what) &= ~(1<<(sig)), 0)
#define sigemptyset(what)   (*(what) = 0, 0)
#define sigfillset(what)    (*(what) = ~(0), 0)
#define sigismember(what,sig) (((*(what)) & (1<<(sig))) != 0)

#define SIGKILL 2
#define SIGINT  SIGBREAKB_CTRL_C
#define SIGUSR1 16
#define SIGQUIT 6
#define SIGTERM 9
#define SIGPIPE 10
#define SIGUSR2 17      // used for JAmiga specific CondWait

/* Define our trace here aswell */
//#define JA_TRACE(x, y...) Forbid(); DebugPrintF("TRACE %-5ld %15s:%-5d", ((struct Process *)FindTask(NULL))->pr_ProcessID,__FILE__,__LINE__); DebugPrintF(x, ## y); Permit();
#define SIG_UNBLOCK 1001
#define SIG_BLOCK 1002
#define SIG_SETMASK 1003        /* unused by jamvm, here for completeness */
#define SA_RESTART 3000
/* A normal pointer */
#define sigjmp_buf long
//#define alloca(size) AllocMem(size, MEMF_PUBLIC)
struct sigaction {
    void (*sa_handler)(int);
    uint32 sa_flags;
    uint32 sa_mask;
};

void sigprocmask(int blocktype, uint32 * mask_ptr, void *unused1);
void sigsuspend(uint32 * maskptr);
void sigaction(uint32 sig, struct sigaction * act, void *unused);
void sigwait(uint32 * maskptr, uint32 * sigptr);
int sigsetjmp(sigjmp_buf env, int savemask);
#endif

