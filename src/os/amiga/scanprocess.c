/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
 * Robert Lougher <rob@lougher.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "signal.h"



//extern struct hash_table hash_table;
//extern void unloadDll(void *dll, int unloader);
extern int VM_initing;
extern unloadBootClassLoaderDlls();
extern struct PthreadInitData * initPthreadInitData(void * args);
extern void freeProcessInitData(struct PthreadInitData * initData);
extern int main(int argc, char *argv[]);
extern void MG_call_init(void);
extern void MG_call_exit(void);
//extern void unloadClassLoaderDlls(struct object *loader);

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

//struct UtilityBase * UtilityBase = NULL;
//struct UtilityIFace * IUtility = NULL;

#include "pthread.h"
#include "signal.h"

#define MAGIC_SECRET_PROCESSDATA 999
#define MAGIC_SECRET_JAVATHREADDATA 998
static struct Task * mainAmigaTask = NULL;

#undef exit
void (*therealmccoy)(int) = &exit;
#define exit amigaexit

/**
 * Hook function for the ProcessScan() call below.
 */
STATIC uint32 ASM find_java_child_sigint( REG(a0, struct Hook *hook),
                                  REG(a2, struct Process *parentproc ),
                                  REG(a1, struct Process *proc) )
{
    if((proc->pr_ParentID == parentproc->pr_ProcessID)
            && // make sure this isn't like a CON child or something
            (proc->pr_FinalData == MAGIC_SECRET_PROCESSDATA)
    ) /* my child ? */
    {
        if(proc->pr_Task.tc_State == TS_SUSPENDED) {
            JA_TRACE("It suspended: %ld\n", proc->pr_ProcessID);

        }
        uint32 sigSet =  (1 << SIGINT);
        JA_TRACE("Signalling %ld %lx, %lx sig: %ld state: %ld\n", proc->pr_ProcessID, proc, proc->pr_FinalData, sigSet, proc->pr_Task.tc_State);
        Signal(proc, sigSet);
        (*(uint32*)hook->h_Data)++;
        // continue to scan
        return 0;
    } else {
        //JA_TRACE("NOT Signalling %ld %ld, %lx\n", proc->pr_ProcessID,  proc, proc->pr_FinalData);
        //JA_TRACE("Not Signalling %ld\n", proc);
        return 0;  /* FALSE = not found - continue to scan list */
    }
}
#define BYTETOBINARYPATTERN "%d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d"
#define BYTETOBINARY(byte)  \
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

/**
 * Hook function for the ProcessScan() call below.
 */
STATIC uint32 ASM find_java_child_kill( REG(a0, struct Hook *hook),
                                  REG(a2, struct Process *parentproc ),
                                  REG(a1, struct Process *proc) )
{
    uint32 sigSet = 0;
    if((proc->pr_ParentID == parentproc->pr_ProcessID)
            && // make sure this isn't like a CON child or something
            (proc->pr_FinalData == MAGIC_SECRET_PROCESSDATA)) {
        JA_TRACE(    "Checking           %ld (%20s) %lx, %lx sig: %ld state:"BYTETOBINARYPATTERN"\n", proc->pr_ProcessID, ((struct Task*)proc)->tc_Node.ln_Name, proc, proc->pr_FinalData, sigSet, BYTETOBINARY(((struct Task*)proc)->tc_State));
        if(proc->pr_Task.tc_State == TS_SUSPENDED) {
            JA_TRACE("Removing suspended %ld (%20s) %lx, %lx sig: %ld state:"BYTETOBINARYPATTERN"\n", proc->pr_ProcessID, ((struct Task*)proc)->tc_Node.ln_Name, proc, proc->pr_FinalData, sigSet, BYTETOBINARY(((struct Task*)proc)->tc_State));
            RemTask(proc);
            (*(uint32*)hook->h_Data)++;
            return 0;
        } else if((proc->pr_Task.tc_State != TS_READY) && (proc->pr_Task.tc_State != TS_REMOVED) ) {
            sigSet =  (1 << SIGKILL);
            JA_TRACE("Signalling         %ld (%20s) %lx, %lx sig: %ld state:"BYTETOBINARYPATTERN"\n", proc->pr_ProcessID, ((struct Task*)proc)->tc_Node.ln_Name, proc, proc->pr_FinalData, sigSet, BYTETOBINARY(((struct Task*)proc)->tc_State));
            Signal(proc, sigSet);
            (*(uint32*)hook->h_Data)++;
            // continue to scan
            return 0;
        } else {
            JA_TRACE("NOT Signalling %ld %ld, %lx\n", proc->pr_ProcessID,  proc, proc->pr_FinalData);
            //JA_TRACE("Not Signalling %ld\n", proc);
            return 0;  /* FALSE = not found - continue to scan list */
        }
    }
    return 0;
}


/**
 * This function overwrites the C exit() function. This function
 * first tries to end all Java threads, and then signals the main
 * Amiga task, which will in turn call realAmigaExit().
 */
void amigaexit(int error) {
    JA_TRACE("in amigaexit signal main_thread with %ld %ld\n", error, VM_initing);
    if(VM_initing == 1) {
        JA_TRACE("vm init, calling _exit\n");
        _exit(error);
    }
    if(mainAmigaTask == NULL) {
        //#undef exit
        //exit(error);
        //#define exit amigaexit
        return;
    } else
    if(mainAmigaTask == FindTask(NULL)) {
        JA_TRACE("We are main task\n");
        realAmigaExit(error);
        JA_TRACE("Back from amigaexit\n");
    } else {
        struct Task * me = FindTask(NULL);

        NotifyProcListChange(me, SIGBREAKB_CTRL_F, 0);
        // Wait for any Amiga child processes

        uint32 sigSet;
        struct Hook H;
        uint32 proc_count; /* we pass a pointer to this as 'userdata' */
        H.h_Entry = (APTR) &find_java_child_sigint;     /* initialise the hook struct */
        H.h_Data = &proc_count;

        do {
            proc_count = 0;                    /* set initial count to zero  */

            ProcessScan(&H, (APTR)me, 0);
            JA_TRACE("Waiting for %ld java children to end\n", proc_count);
            if(proc_count > 0) {
                Wait(SIGBREAKF_CTRL_F);
            }
        } while(proc_count > 0);
                                
        NotifyProcListChange(NULL, NPLC_END, 0);
        if(mainAmigaTask != NULL) {
            JA_TRACE("Ended all Java threads, NOT main task, singalling it\n");
            Signal(mainAmigaTask, 1);
        }
    }
}


struct FindJvmChildData {
    int fj_Count;
    const char * fj_ChildName;
};

/**
 * Hook function for the ProcessScan() call below.
 */
STATIC uint32 ASM find_jvm_child( REG(a0, struct Hook *hook),
                                  REG(a2, struct Process *parentproc ),
                                  REG(a1, struct Process *proc) )
{
    struct FindJvmChildData *data = ((struct FindJvmChildData *)hook->h_Data);
    //JA_TRACE("Looking for %s (is it %s?)\n", data->fj_ChildName, proc->pr_Task.tc_Node.ln_Name);

    if(proc->pr_ParentID == parentproc->pr_ProcessID) {
        JA_TRACE("Got child: %ld %ld, %lx, state: %ld, ane: \"%s\"\n", proc->pr_ProcessID,  proc, proc->pr_FinalData, proc->pr_Task.tc_State, proc->pr_Task.tc_Node.ln_Name);

        if( // make sure this isn't like a CON child or something
            ((proc->pr_Flags & (1<<PRB_HANDLERPROCESS)) != (1<<PRB_HANDLERPROCESS))
            &&
            (strcmp(proc->pr_Task.tc_Node.ln_Name, data->fj_ChildName) == 0)
            &&
            ((proc->pr_Task.tc_State & (1<<TS_REMOVED)) != (1<<TS_REMOVED))
        ) /* my child ? */
        {
            uint32 sigSet =  (1 << SIGKILL);
            JA_TRACE("Signalling %s %lx\n", proc->pr_Task.tc_Node.ln_Name, proc);
            JA_TRACE("Signalling %ld %lx\n", proc->pr_ProcessID, proc);
            JA_TRACE("Signalling %ld %lx, %lx\n", proc->pr_ProcessID, proc, proc->pr_FinalData);
            Signal(proc, sigSet);

            data->fj_Count++;
            // don't continue to scan
            return 1;
        }
    }
    //JA_TRACE("Not Signalling %ld\n", proc);
    return 0;  /* FALSE = not found - continue to scan list */

}


/**
 * Real amigas exit where we really exit.
 */
void realAmigaExit(int error) {
    JA_TRACE("in realAmigaExit signal main_thread with %ld\n", error);
    //_SETDEBUGLEVEL(2);
    SetExcept(0,0);
    struct Task * me = FindTask(NULL);
    if(mainAmigaTask != me) {
        abort();
    }

    // Kill of any still running Java Threads
    NotifyProcListChange(me, SIGBREAKB_CTRL_F, 0);
    uint32 sigSet;
    struct Hook H;
    uint32 proc_count; /* we pass a pointer to this as 'userdata' */
    H.h_Entry = (APTR) &find_java_child_kill;     /* initialise the hook struct */
    H.h_Data = &proc_count;

    JA_TRACE("Searching for java children to end\n");
    do {
        proc_count = 0;                    /* set initial count to zero  */

        ProcessScan(&H, (APTR)me, 0);

        JA_TRACE("Waiting for %ld java children to end\n", proc_count);
        if(proc_count > 0) {
            Wait(SIGBREAKF_CTRL_F);
        }
    } while(proc_count > 0);
    JA_TRACE("Search for java children done\n");

    // Unload DLLs to close any opened libraries therein,
    // BEFORE killing th Java helper threads
    JA_TRACE("Unloading bootclassapth laoder ddls\n");
    unloadBootClassLoaderDlls();

    // Now, kill of the JVM helper processes
    const char * jvmChildThreads[] = {"Async GC", "Reference Handler", "Finalizer", "Signal Handler"};

    H.h_Entry = (APTR) &find_jvm_child;     /* initialise the hook struct */

    struct FindJvmChildData data;
    int i;
    for(i = 0 ; i< 4 ; i++) {
        
        data.fj_ChildName = jvmChildThreads[i];
        JA_TRACE("Looking for %s\n", data.fj_ChildName);
        H.h_Data = &data;
        do {
            //proc_count = 0;                     /* set initial count to zero  */
            data.fj_Count = 0;
            ProcessScan(&H, (APTR)me, 0);

            JA_TRACE("Waiting for %ld jvm children to end\n", proc_count);
            if(data.fj_Count > 0) {
                Wait(SIGBREAKF_CTRL_F);
            }
        } while(data.fj_Count > 0);
    }
    NotifyProcListChange(NULL, NPLC_END, 0);
    //Forbid();
    
    sysFree(me->tc_ExceptData);
    me->tc_ExceptData = NULL;
    me->tc_ExceptCode = NULL;
    freeProcessInitData((struct PthreadInitData *)((struct Process *)me)->pr_ExitData);
    JA_TRACE("Wainting for Ctrl-F! Flags: %lx\n", ((struct Process *)me)->pr_Flags);
    JA_TRACE("Exit data: %lx\n", ((struct Process *)me)->pr_ExitData);
    //Wait(SIGBREAKF_CTRL_F);
    ((struct Process *)me)->pr_ExitData = NULL;
    JA_TRACE("exit EXIT EXIT setting exitcode=NULL Wainting for Ctrl-F! Flags: %lx\n", ((struct Process *)me)->pr_Flags);
    //((struct Process *)me)->pr_Flags &= ~(1 << 6);
    //((struct Process *)me)->pr_ExitCode = NULL;
    
    FreeSignal(SIGUSR1);
    FreeSignal(SIGUSR2);
    // doing this will hinder jamvm_exit from running java stuff again
    VM_initing = 1;
    JA_TRACE("Setting VM_Inikting:%ld %s, from funct: %ld\n", VM_initing, VM_initing ? "true":"false", VMInitialising());
    if(IUtility)DropInterface((struct Interface *)IUtility);
    IUtility = NULL;
    if(UtilityBase) CloseLibrary((struct Library *)UtilityBase);
    UtilityBase = NULL;
    //Permit();
    //Wait(SIGBREAKF_CTRL_F);
    // bit 6 is set on process -- trying to close error stream,
    // while something is writing to it,.
    // clib2 _exit should not call the exit_trap_trigger
    MG_call_exit();
    _Exit(error);
    //RemTask(FindTask(0L));
}


int isMainAmigaTask() {
    return FindTask(NULL) == mainAmigaTask;
}
void testFunc() {
    DebugPrintF("THIS IS THE EXIT CODE\n");
}

void amiga_exit() {
    DebugPrintF("in amiga_exit signal main_thread with 1\n");
    Signal(mainAmigaTask, 1);
}




/**
 *
 */
void initialisePlatform() {
    // we're doing this lazily in memory.c:mymalloc
    //MG_call_init();
    UtilityBase = OpenLibrary("utility.library", 0);
    if(UtilityBase) {
        IUtility = (struct UtilityIFace *)GetInterface((struct Library *)UtilityBase, "main", 1, NULL);
    }
    mainAmigaTask = FindTask(NULL);
    if(AllocSignal(SIGUSR1) == -1) {
        DebugPrintF("Couldn't allocate signal SIGUSR1 %d\n", SIGUSR1);
        return;
    }
    if(AllocSignal(SIGUSR2) == -1) {
        FreeSignal(SIGUSR1);
        DebugPrintF("Couldn't allocate signal SIGUSR2 %d\n", SIGUSR2);
        return;
    }
    DebugPrintF("Allocated SIGUSR1 and SIGUSR2\n");
    DebugPrintF("Settign exitdata to null\n");
    //sysMalloc(sizeof(struct PthreadInitData));
    mainAmigaTask->tc_ExceptData = sysMalloc(sizeof(struct sigaction));
    //((struct Process *)FindTask(NULL))->pr_ExitData;
    //((struct Process *)mainAmigaTask)->pr_ExitCode = &testFunc;
    /* Nothing to do for powerpc */
    ((struct Process *)mainAmigaTask)->pr_ExitData = initPthreadInitData(NULL);
    

}
    

int32 PRUTT_start(STRPTR args, int32 arglen, struct ExecBase *sysbase)
{
   IExec = (APTR)sysbase->MainInterface;
   struct WBStartup *wbmsg = NULL;
   struct Process *process;
   int32  rc = RETURN_FAIL;
   int i;

   IExec->Obtain();
   process = (struct Process *) FindTask(NULL);

   if( ! process->pr_CLI )
   {
       WaitPort(&process->pr_MsgPort);
       wbmsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
   }

   if(( DOSBase = OpenLibrary("dos.library", 51) ))
   {
       if(( IDOS = (APTR)GetInterface(DOSBase, "main", 1, NULL)))
       {
           rc = RETURN_OK;

           // ...your own code goes here... //
           Printf("Hello World...%ld\n", arglen);
           for(i = 0; i < arglen; i++) {
            Printf("%ld: %s\n", i, args[i]);
           }
           rc = 0;//main(arglen, args);
           DropInterface((APTR)IDOS);
       }
       CloseLibrary(DOSBase);
   }

   if( wbmsg )
   {
       Forbid();  // <-- With workbench.library V52, this is no longer needed.
       ReplyMsg((APTR)wbmsg );
   }

   IExec->Release();
   return(rc);
  }



