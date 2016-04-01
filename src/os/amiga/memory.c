
#include "memory.h"

// Define this to track the allocations and frees made
#undef USE_MEMGUARDIAN
//#define USE_MEMGUARDIAN 1
#define COUNT_MEM_STATS 1
//#define USE_AMIGA_POOL 1
// COME BACK!
// There's a bug when not using pools. The size
// should be stored at position 0, i norder for
// realloc to work. Sorry for the inconvenience.
// #undef USE_AMIGA_POOL
#define USE_AMIGA_POOL

#ifdef USE_MEMGUARDIAN
enum {
    /* Memory was recently allocated */
    MS_ALLOCATED,

    /* Memory was recently free'd */
    MS_FREED,

    /* Not used currently */
    MS_PROTECTED
};// MemStatus;

enum {
    /* C */
    AT_MALLOC,

    /* C++ */
    AT_NEW,
    AT_NEW_ARRAY,

    /* Exec library */
    AT_ALLOCABS,
    AT_ALLOCATE,
    AT_ALLOCENTRY,
    AT_ALLOCMEM,
    AT_ALLOCPOOLED,
    AT_ALLOCSIGNAL,
    AT_ALLOCSYSOBJECT,
    AT_ALLOCTRAP,
    AT_ALLOCVEC,
    AT_ALLOCVECPOOLED
};// AllocationType;

#define MG_NAME_BUFFER_SIZE 80

typedef struct
{
    /* For list navigation */
    struct Node _node;

    /* Current status of allocation */
    uint32 _status;

    /* Way of allocation */
    uint32 _type;

    /* Address of allocation */
    void* _address;

    /* File + Line number where allocated */
    char _alloc_file[ MG_NAME_BUFFER_SIZE ] ;

    /* File + Line number where deallocated */
    uint32 _alloc_line;

    char _dealloc_file[ MG_NAME_BUFFER_SIZE ];
    uint32 _dealloc_line;
    uint32 _size;

} MG_allocation;


static struct List MG_list;
static char MG_file[ MG_NAME_BUFFER_SIZE ];
static int MG_line;
#endif // USE_MEMGUARDIAN

#define MT_MALLOC 1
#define MT_MMAP 2


struct MyMemEntry {
    uint32 me_Type;
    uint32 me_Length;
    APTR * me_Ptr;
} MyMemEntry;

#define MyMemEntryExtraSize (sizeof(uint32)+sizeof(uint32))
#define MyMemEntryOffset MyMemEntryExtraSize/8
//

#ifdef USE_AMIGA_POOL
static APTR mem_pool;
static APTR map_pool;
#endif
static long mmapped;

#ifdef COUNT_MEM_STATS
static uint32 allocs;
static uint32 frees;
static uint32 mmaps;
static uint32 munmaps;
#endif


/* Avoid starting / finishing MemGuardian twice */
static BOOL bRunning = FALSE;
static BOOL bExiting = FALSE;


/**
 * Called on startup
 */
void MG_call_init(void)
{
    if ( bRunning )
    {
        DEBUGOUT( "Already Initialized!\n" );
        return;
    }
    mmapped = 0L;
    bRunning = TRUE;
    #ifdef USE_AMIGA_POOL
    mem_pool = AllocSysObjectTags(ASOT_MEMPOOL,
        //ASOPOOL_Puddle, 1024,
        //ASOPOOL_Threshold, 512,
        //#ifdef INLINING
        ASOPOOL_MFlags, MEMF_EXECUTABLE,
        //#else
        //ASOPOOL_MFlags, MEMF_SHARED,
        //#endif
        //ASOPOOL_Protected, TRUE,
        ASOPOOL_Name, "Memory pool",
        TAG_END);
    map_pool = AllocSysObjectTags(ASOT_MEMPOOL,
        //ASOPOOL_Puddle, 1024,
        //ASOPOOL_Threshold, 512,
        //#ifdef INLINING
        ASOPOOL_MFlags, MEMF_EXECUTABLE|MEMF_DELAYED,
        //#else
        //ASOPOOL_MFlags, MEMF_SHARED|MEMF_DELAYED,
        //#endif
        //ASOPOOL_Protected, TRUE,
        ASOPOOL_Name, "Memory map pool",
        TAG_END);
    #ifdef INLINING
    DEBUGOUT( "[Memory] Init pools, executable mem, mem_pool: %lx, map_pool: %lx, mmapped:%ld\n", mem_pool, map_pool, mmapped);
    #else
    DEBUGOUT( "[Memory] Init pools, shared mem, mem_pool: %lx, map_pool: %lx, mmapped:%ld\n", mem_pool, map_pool, mmapped);
    #endif
    #endif
    #ifdef COUNT_MEM_STATS
    DEBUGOUT( "[Memory] Counting memory statistics\n" );
    allocs = 0;
    frees = 0;
    mmaps = 0;
    munmaps = 0;
    #endif
    #ifdef USE_MEMGUARDIAN
    DEBUGOUT( "[Memory] Using mem guardian\n" );
    NewList( &MG_list );
    #endif
}


/**
 * Called on exit. Frees any unfreed memory.
 */
void MG_call_exit(void)
{
    bExiting = TRUE;
    if ( !bRunning )
    {
        DEBUGOUT( "Already Finished.\n" );
        return;
    }

    bRunning = FALSE;
    uint32 allsigs = 0;


    sigaddset(&allsigs, SIGKILL);
    sigaddset(&allsigs, SIGINT);
    SetExcept(allsigs,0);

    DEBUGOUT( "[Memory] Exit\n" );

    #ifdef COUNT_MEM_STATS
    DEBUGOUT( "[Memory] Exit allocs: %ld, frees: %ld\n" , allocs, frees);
    DEBUGOUT( "[Memory] Exit mmaps: %ld, munmaps: %ld\n" , mmaps, munmaps);
    #endif

    
    #ifdef USE_MEMGUARDIAN
    uint32 unfrees = 0;
    uint32 frees = 0;
    uint32 sizes = 0;
    uint32 biggest = 0;
    uint32 smallest = -1;
    MG_allocation * itr;
    while ( ( itr = (MG_allocation*)     RemTail(&MG_list) ) )
    {
        frees++;
        sizes += itr->_size;
        biggest = biggest > itr->_size ? biggest : itr->_size;
        smallest = smallest < itr->_size ? smallest : itr->_size;
        if ( MS_ALLOCATED == itr->_status )
        {
            unfrees++;
            switch ( itr->_type )
            {
            case AT_MALLOC:
                free( itr->_address );
                break;

            case AT_NEW:
                /* operator new was implemented by using malloc() */
                free( itr->_address );
                break;

            case AT_NEW_ARRAY:
                /* operator new [] was implemented by using malloc() */
                free( itr->_address );
                break;

            case AT_ALLOCVEC:
                FreeVec( itr->_address );
                break;

            default:
                break;
            }

            itr->_address = NULL;
            itr->_status = MS_FREED;
        }

        /* Remove allocation node */
        FreeVec( itr );
    }
    DEBUGOUT( "Number of unfrees: %ld/%ld.\n", unfrees, frees);
    DEBUGOUT( "Total memory: %ld %ld\n", sizes, (uint32)(sizes/frees) );
    DEBUGOUT( "Alloced sizes: small:%ld biggest:%ld\n", smallest, biggest);
    NewList( &MG_list );
    #endif //#ifdef USE_MEMGUARDIAN

    #ifdef USE_AMIGA_POOL
    DEBUGOUT( "Freeing mem pool\n");
    FreeSysObject(ASOT_MEMPOOL, mem_pool);
    DEBUGOUT( "Freeing map pool\n");
    FreeSysObject(ASOT_MEMPOOL, map_pool);
    DEBUGOUT( "Freed pools\n");
    mem_pool = 0xCAFECAFE;
    map_pool = 0xCAFECAFE;
    DEBUGOUT( "set pointers to boo\n");
    #endif

}


inline struct MyMemEntry * getMemEntry(APTR ptr) {
    struct MyMemEntry * entry;
    entry = ((uint32*)ptr)-MyMemEntryOffset;
    //JA_TRACE("getMemEntry: from: %lx(%lx), to: %lx(%lx): len:%ld type:%ld\n", ptr, (*(uint32*)ptr), entry, *entry, entry->me_Length, entry->me_Type);
    return entry;
}

/**
 * Allocates memory
 */
void* mymalloc(size_t size, uint32 memType, const char* file, int line) {
    #ifdef COUNT_MEM_STATS
    allocs++;
    #endif
    #ifdef USE_AMIGA_POOL
    struct MyMemEntry * mem;


    if(memType == MT_MALLOC) {
        //JA_TRACE("MT_MALLOC: %ld+%ld=%ld pool:%lx\n", size, MyMemEntryExtraSize, size + MyMemEntryExtraSize, mem_pool);
        mem = AllocVecPooled(mem_pool, size + MyMemEntryExtraSize);
        //JA_TRACE("MT_MALLOC: %ld+%ld=%ld pool:%lx\n", size, MyMemEntryExtraSize, size + MyMemEntryExtraSize, mem_pool);
    } else {
        //JA_TRACE("MT_MMAP: %ld+%ld=%ld pool:%lx\n", size, MyMemEntryExtraSize, size + MyMemEntryExtraSize, map_pool);
        mem = AllocVecPooled(map_pool, size + MyMemEntryExtraSize);
        //JA_TRACE("MT_MMAP: %ld+%ld=%ld pool:%lx\n", size, MyMemEntryExtraSize, size + MyMemEntryExtraSize, map_pool);
    }
    if(mem == NULL) {
        JA_TRACE("Couldn't allocate memory!\n");

    } else {
        mem->me_Type = memType;
        mem->me_Length = size;
        mem->me_Ptr = ((uint32*)mem)+MyMemEntryOffset;// getMemEntry(ptr);// ((uint32)(*mem)) + MyMemEntryOffset;
        //JA_TRACE("AllocedPooled myalloc  free:%lx %lx with size: %ld MemType:%ld mem-ptr: %lx addre: %lx, cts:%lx (%s:%ld)\n", *mem, mem, mem->me_Length, mem->me_Type, mem->me_Ptr, &mem->me_Ptr, *mem->me_Ptr,  file, line);
    }

    //if(memType!=MT_MALLOC) {
    //    JA_TRACE("AllocedPooled %lx with size: %ld MemType:%ld (%s:%ld)\n", ptr, sizeof(size_t)+size, memType, file, line);
    //}
    //JA_TRACE("AllocedPooled %lx with size: %ld MemType:%ld (%s:%ld)\n", ptr, size, memType, file, line);
    #else
    void * ptr = AllocVecTags(size,
                            AVT_Type, MEMF_EXECUTABLE,//SHARED,
                            AVT_Lock, FALSE,
                            AVT_ClearWithValue, 0xB00BB00B,
                            TAG_DONE);
    #endif
    #ifdef USE_MEMGUARDIAN
    if ( ptr )
    {
        //JA_TRACE("Malloc() called in file %s (line %d)\n", file, line);

        MG_allocation * _ptr = (MG_allocation*)    AllocVec( sizeof( MG_allocation ), MEMF_PRIVATE|MEMF_CLEAR );

        if ( _ptr )
        {
            _ptr->_status = MS_ALLOCATED;
            _ptr->_type = AT_ALLOCVEC;
            _ptr->_address = ptr;
            _ptr->_size = size;
            strncpy( _ptr->_alloc_file, file, 80 );
            _ptr->_alloc_line = line;

                AddTail( &MG_list, (struct Node*) _ptr );
        }
    }
    #endif

    if(!mem) {
        // Check that we aren't closing down
        // COME BACK!
        // Possible concurrency issue here, if memory allocation
        // fails for other reason than non-initialized pools.
        // If it fails (due to out of mem, f.i.) exiting might
        // be called from other thread. Need better locking.
        if(!bExiting && !bRunning) {
            JA_TRACE("Initializing memory 1st time\n");
            MG_call_init();
            // try again (this should only happen the first time)
            #ifdef USE_AMIGA_POOL
            if(memType == MT_MALLOC) {
                mem = AllocVecPooled(mem_pool, size+MyMemEntryExtraSize);
            } else {
                mem = AllocVecPooled(map_pool, size+MyMemEntryExtraSize);
            }
            mem->me_Type = memType;
            mem->me_Length = size;
            mem->me_Ptr = ((uint32*)mem)+MyMemEntryOffset;

            #else
                ptr = AllocVecTags(size,
                            AVT_Type, MEMF_EXECUTABLE,//MEMF_SHARED,
                            AVT_Lock, FALSE,
                            AVT_ClearWithValue, 0xB00BB00B,
                            TAG_DONE);
            #endif
        }
        if(!mem) {
            JA_TRACE("Couldn't allocate sz: %ld\n",  sizeof(size_t)+size);
            return NULL;
        }
    }
    return mem->me_Ptr;
}


/**
 * Frees ptr
 */
void myfree(void* ptr, uint32 memType, const char* file, int line)
{
    #ifdef COUNT_MEM_STATS
    frees++;
    #endif
    if(ptr) {
    } else {
        JA_TRACE("Invalid ptr\n");
        return;
    }

    
    #ifdef USE_MEMGUARDIAN
    BOOL bFound = FALSE;

    MG_allocation * itr = (MG_allocation*) MG_list.lh_Head;
    while ( itr->_node.ln_Succ )
    {
        if ( itr->_address == ptr )
        {
            if ( MS_FREED == itr->_status )
            {
                DEBUGOUT( "Warning: possible attempt to free(%lx) already free()'d object in file %s (line %d). Memory was earlier free()'d in file %s (line %ld)\n", ptr, file, line, itr->_dealloc_file, itr->_dealloc_line );
            }
            else if ( MS_ALLOCATED == itr->_status )
            {
                bFound = TRUE;
                if ( AT_ALLOCVEC == itr->_type )
                {
                    #ifdef USE_AMIGA_POOL
                    FreePooled(mem_pool, ptr, size);
                    #else
                    FreeVec( ptr );
                    #endif
                    itr->_address = NULL;
                    itr->_status = MS_FREED;
                    strncpy( itr->_dealloc_file, file, 80 );
                    itr->_dealloc_line = line;

                    //DEBUGOUT("free() called in file %s (line %d)\n", file, line);
                }
                else
                {
                    DEBUGOUT("Warning: memory wasn't allocated with malloc() but still trying to free() it.\n");
                    // NOTE: memory remains UNFREED! could be fixed here but unless programmer fixes his bug, freeing in debug version doens't help anyone..
                }
            }
            break;
        }

        itr = (MG_allocation*) itr->_node.ln_Succ;
    }

    if ( !bFound )
    {
        DEBUGOUT("Warning: couldn't find memory allocation to free() (%p) in file %s (line %d)\n", ptr, file, line);
    }
    #else

    #ifdef USE_AMIGA_POOL
    //JA_TRACE("About to free %lx with size: %ld MemType:%ld (%s:%ld)\n", ptr, size, memType, file, line);
    struct MyMemEntry * mem;
    mem = getMemEntry(ptr);
    //JA_TRACE("-1: %lx\n", *((uint32 *)ptr)-1);
    //JA_TRACE("-2: %lx\n", *((uint32 *)ptr)-2);
    //JA_TRACE("AllocedPooled myfree   free:%lx (%lx)-> %lx with size: %ld %lx MemType:%ld %lx mem-ptr: %lx (%s:%ld)\n", ptr, *mem, (void*)mem, mem->me_Length, mem->me_Length, mem->me_Type, mem->me_Type, mem->me_Ptr, file, line);
    
    if(mem->me_Type == MT_MALLOC) {
        FreeVecPooled(mem_pool, (void*)mem);//, size);
    } else {
        FreeVecPooled(map_pool, (void*)mem);
    }
    //JA_TRACE("Just did free %lx with size: %ld MemType:%ld (%s:%ld)\n", ptr, size, memType, file, line);
    #else
    FreeVec( ptr );
    #endif

    #endif
}



#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
void* myrealloc(void * oldptr, size_t size, const char* file, int line) {
    struct MyMemEntry * mem;
    if(oldptr != NULL) {
        mem = getMemEntry(oldptr);
        JA_TRACE("AllocedPooled myrealloc newsize:%ld before: %ld  %lx with %lx MemType:%lx mem-ptr: %lx (%s:%ld)\n", size, mem->me_Length, mem, mem->me_Length,  mem->me_Type, mem->me_Ptr, file, line);
        size_t oldsize = mem->me_Length;//oldptr ? *((size_t*)oldptr-1) : 0;

        void * newptr = mymalloc(size, mem->me_Type/*MT_MALLOC*/, file, line);

        if(newptr != NULL) {
            JA_TRACE("Copying %ld bytes from %lx to %lx\n", MIN(oldsize, size), oldptr, newptr);
            CopyMem(oldptr, newptr, MIN(oldsize, size));
            
            myfree(oldptr, mem->me_Type/*MT_MALLOC*/, file, line);


            return newptr;
        } else {
            return NULL;
        }
    } else {
        //JA_TRACE("AllocedPooled myrealloc: (%s:%ld)\n", file, line);
        return mymalloc(size, MT_MALLOC, file, line);
    }
}

/*
Calle3 dlike this (alloc.c):
uintptr_t *mem = mmap(0, size, PROT_READ|PROT_WRITE,
                                   MAP_PRIVATE|MAP_ANON, -1, 0);

*/
void *mymmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset, const char* file, int line) {
    #ifdef COUNT_MEM_STATS
    mmaps++;
    #endif
    mmapped = mmapped + length;
    void * ptr = mymalloc(length, MT_MMAP, file, line);
    if(ptr == NULL) {
        JA_TRACE("mmap: failed\n");
        return MAP_FAILED;
    }
    return ptr;

}
int mymunmap(void *addr, size_t len, const char* file, int line) {
    #ifdef COUNT_MEM_STATS
    munmaps++;
    #endif

    size_t oldsize = addr ? *((size_t*)addr-1) : 0;
    //if(len != oldsize) {
    //    JA_TRACE("munmap: %lx, %ld (of %ld) (not freeing this time)\n", addr, len, oldsize);
    //} else {
    myfree(addr, MT_MMAP, file, line);
    //}
    return 0;
}

int getpagesize() {
    return 1;
}
