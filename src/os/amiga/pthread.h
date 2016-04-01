
#ifndef AMIGA_PTHREAD_H
#define AMIGA_PTHREAD_H

#include "signal.h"

//
#include <time.h>
#include <sys/time.h>
//#include <sys/errno.h>
// Blargh: Hardcodin this here, sincve I get timeval issues.. *sigh*
#define ESRCH 3     /* No such process */
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/timezone.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <exec/semaphores.h>

/* Define our trace here aswell */
#define JA_TRACE(x, y...)
//Forbid(); DebugPrintF("TRACE %-5ld %15s:%-5d", ((struct Process *)FindTask(NULL))->pr_ProcessID,__FILE__,__LINE__); DebugPrintF(x, ## y); Permit();

#define ETIMEDOUT 60

/*struct timespec
{
    unsigned int tv_sec;
    unsigned int tv_nsec;
};*/

//typedef struct {
//    APTR lock;
//    struct CondVariable cv;
//    struct Semaphore *semaphore;
//    enum CondType cv;
//} VMWaitLock;

//typedef APTR VMLock;


enum CondType {
    NONE,
    SIGNAL,
    BROADCAST
};
struct ProcessNode {
    struct MinNode node;
    struct Process * process;
};
struct CondVariable {
    struct MinList waitingProcessList;
    enum CondType condType;
    struct SignalSemaphore semaphore;
} CondVariable;

struct PthreadInitData {
//    void * (*pd_Func)(void*);
    void *pd_Args;
    APTR pd_ExceptCode;
    APTR pd_ExceptData;
    char pd_Dummy;
    uint32 pd_SigMask;
    void * pd_Specific;
    //struct TimeRequest *pd_timerIO;
    //struct MsgPort *pd_timerMP;

} PthreadInitData;
//void SignalThreads(enum CondType type, struct CondVariable cv, struct thread *self);
int CondWait(struct CondVariable * cv, struct SignalSemaphore * lock, struct timespec * ts);




/* Defines to override pthread stuff */
#define pthread_t APTR
#define pthread_mutex_t struct SignalSemaphore
#define pthread_cond_t struct CondVariable
#define pthread_key_t APTR
#define pthread_attr_t APTR
#define PTHREAD_CREATE_DETACHED 1

void pthread_sigmask(int blocktype, uint32 * mask_ptr, void *unused1);
void pthread_mutex_init(pthread_mutex_t *lock, void*used);
int pthread_mutex_lock(struct SignalSemaphore *lock);
int pthread_mutex_unlock(struct SignalSemaphore * lock);
int pthread_mutex_trylock(struct SignalSemaphore * lock);
void pthread_cond_init(struct CondVariable *cv, void * unused);

int pthread_cond_signal(struct CondVariable * cv);
int pthread_cond_broadcast(struct CondVariable * cv);
#define pthread_attr_init(attr) {}
#define pthread_attr_getstacksize(unused, unused2) 1*MB
#define pthread_attr_setstacksize(unused, unused2) {}
#define pthread_attr_setdetachstate {}
#define pthread_key_create(pt_key_t, unused) { *pt_key_t = FindTask(NULL); }
void* pthread_getspecific(void * self);
void pthread_setspecific(void * self, void *value);// { JA_TRACE("set spec\n"); FindTask(NULL)->tc_UserData = value;}
#define pthread_self() FindTask(NULL)


#define MAGIC_SECRET_PROCESSDATA 999
#define MAGIC_SECRET_JAVATHREADDATA 998
int pthread_create2(pthread_t * tid, void * attributes, void * (*func)(void*), void ** thread, char *name);
int pthread_create(pthread_t * tid, void * attributes, void * (*func)(void*), void * thread);
/*
#define pthread_create(tid, attributes, func, thread) \
    (*(tid) = CreateNewProcTags(\
            NP_Entry, &(func),        \
            NP_Name, "Java thread",  \
            NP_Child, TRUE,           \
            NP_ExitData, thread,       \
            NP_FinalData, MAGIC_SECRET_PROCESSDATA,\
            TAG_END))!=NULL?0:1
#define pthread_create(tid, attributes, func, thread, name) \
    (*(tid) = CreateNewProcTags(\
            NP_Entry, &(func),        \
            NP_Name, name,  \
            NP_Child, TRUE,           \
            NP_ExitData, thread,       \
            NP_FinalData, MAGIC_SECRET_PROCESSDATA,\
            TAG_END))!=NULL?0:1
*/

 

int pthread_cond_wait(struct CondVariable *cv, struct SignalSemaphore * lock);
int pthread_cond_timedwait(struct CondVariable * cv, struct SignalSemaphore * lock, struct timespec * absoluteTime);

#endif
