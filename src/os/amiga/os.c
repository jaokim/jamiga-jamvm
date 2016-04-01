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

#include "jamvm_rev.h"
#undef VERSION

#include <string.h>
#define USE_MALLOC 1
#define JA_TARGETOS JA_AMIGAOS4

#include "../../jam.h"

//#include "MemGuardian.h"
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/nativelib.h>
#define VERSIONTAG VERSTAG" AmigaOS 4 build of "PACKAGE" "VERSION" "
STRPTR USED ver = (STRPTR)VERSIONTAG;

/**
 * Struct from JAmiga JNI library definition.
 */
struct JNINativeMethod {
    char *className;
    char *methodName;
    char *methodDescriptor;
    void *fnPtr;
    int flags;
};

struct JNINativeLibrary {
    char *name;
    int version;
    int numberOfMethods;
    struct JNINativeMethod *methods;
};


/**
 * Internal struct for native methods
 * with mangled symbol name.
 */
struct JamNativeMethodSymbol {
    char *symbolName;
    void *fnPtr;
};


/**
 * Internal struct for list of native methods
 * with mangled symbol names.
 */
struct JamLibraryInfo {
    char *name;
    int numberOfMethods;
    //
    struct JamNativeMethodSymbol * nativeMethods;
    struct Library *osLib;
#if (JA_TARGETOS==JA_AMIGAOS4)
        struct Interface *osIFace;
#endif
} JamLibraryInfo ;

void *nativeStackBase() {
    DebugPrintF("Native stack base\n");
    return NULL;
}

int nativeAvailableProcessors() {
    DebugPrintF("Native avail processorn");
    return 1;
}

char *nativeLibPath() {
    return getenv("LD_LIBRARY_PATH");
}


char* nativeLibError() {
    return "Misc error";
}

extern char *mangleString(char *utf8);
/**
 *
 */
void *nativeLibOpen(char *path) {
    char * tempName;
    char * tempType;
    char * osName = NULL;
    struct JNINativeLibrary * nativeInfo;
    struct JamLibraryInfo* libraryInfo;
    struct Library *myLib;
    int j, len, i;
#if (JA_TARGETOS==JA_AMIGAOS4)
    struct NativelibIFace *INativelib;
#endif
    
    myLib = OpenLibrary(path, 0);
    if(!myLib) {

        JA_TRACE("Cannot open Library %s\n", path);
        return NULL;
    }
    #if (JA_TARGETOS==JA_AMIGAOS4)
    // Get Interface
    INativelib = (struct NativelibIFace*)GetInterface(myLib, "main", 1, NULL);
    if(!INativelib) {
        JA_TRACE("Cannot get interface for library %s\n", osName);
        CloseLibrary(myLib);
        return NULL;
    }
    #endif


    // Obtain pointer array to native methods
    nativeInfo = (APTR)GetLibraryContent();

    if(!nativeInfo) {
        #if (JA_TARGETOS==JA_AMIGAOS4)
        DropInterface(INativelib);
        INativelib = NULL;
        #endif
        CloseLibrary(myLib);
        sysFree(libraryInfo);
        return NULL;
    }
    libraryInfo = sysMalloc(sizeof(struct JamLibraryInfo));

    #if (JA_TARGETOS==JA_AMIGAOS4)
    libraryInfo->osIFace = (struct Interface*)INativelib;
    #endif
    JA_TRACE("Openend \"%s\", version %ld.%ld\n", myLib->lib_Node.ln_Name, myLib->lib_Version, myLib->lib_Revision);
    libraryInfo->osLib = myLib;
    libraryInfo->name = myLib->lib_Node.ln_Name;


    libraryInfo->numberOfMethods = nativeInfo->numberOfMethods;

    len=(sizeof(struct JamNativeMethodSymbol)) * libraryInfo->numberOfMethods;
    libraryInfo->nativeMethods = sysMalloc(len);
    for(j=0; j < libraryInfo->numberOfMethods; j++) {
        // calculate length of the full method descriptor. Add 3 to cater fot separators
        len = strlen(nativeInfo->methods[j].className)+
                                strlen(nativeInfo->methods[j].methodDescriptor)+
                                strlen(nativeInfo->methods[j].methodName)+3;
        tempName = sysMalloc(len);
        tempName[0] = '\0';
        strcat(tempName, nativeInfo->methods[j].className);
        strcat(tempName, "/");
        strcat(tempName, nativeInfo->methods[j].methodName);

        strcat(tempName, "/");
        strcat(tempName, "/");
        // find the last (
        strcat(tempName, nativeInfo->methods[j].methodDescriptor+1);

        for(i = strlen(tempName)-1; tempName[i] != ')'; i--);
        tempName[i] = '\0';
        if(tempName[i - 1] == '(') {
            // we have sig (), which measn no sig, and no need
            // for the two extra // added above.
            tempName[i - 3] = '\0';
        }

        
        libraryInfo->nativeMethods[j].fnPtr = nativeInfo->methods[j].fnPtr;
        libraryInfo->nativeMethods[j].symbolName = mangleString(tempName);
        sysFree(tempName);
    }

    return libraryInfo;
}

void nativeLibClose(void *handle) {
    struct JamLibraryInfo* libraryInfo = handle;
    int j = 0;
    
#if (JA_TARGETOS==JA_AMIGAOS4)
    DropInterface(libraryInfo->osIFace);
#endif
    CloseLibrary(libraryInfo->osLib);
    for(j=0; j < libraryInfo->numberOfMethods; j++) {
        sysFree(libraryInfo->nativeMethods[j].symbolName);
    }
    sysFree(libraryInfo);
}

void *nativeLibSym(void *handle, char *symbol) {
    struct JamLibraryInfo * libInfo = (struct JamLibraryInfo*)handle;
    //   Class *class;
   //char *name;
   //char *type;
   //char *signature;
    #if __amigaos4__
    struct NativelibIFace *INativelib;
    #endif

    int i,j, len, srcptr;
    int checkHex;
    char * copy = NULL;
    char * className = NULL;
    char * methodName = NULL;
    char * sig = NULL;
    char hex[5];
    char lastChar = '\0';
    BOOL isJni = FALSE;
    #ifdef __amigaos4__
    INativelib = libInfo->osIFace;
    #endif

    if(strstr(symbol, "Java_") == symbol) {
        i = 5;
        isJni = FALSE;

    } else if(strstr(symbol, "JNI_") == symbol) {
        // These are the two JNI_OnLoad and JNI_Onload functions
        isJni = TRUE;
        i = 0;
    } else {
        // Don't know what this could be, shouldn't happen
        i = 0;
    }
    for(j=0; j < libInfo->numberOfMethods; j++) {
        if(isJni && (strstr(libInfo->nativeMethods[j].symbolName, symbol) == libInfo->nativeMethods[j].symbolName)) {
            return libInfo->nativeMethods[j].fnPtr;
        } else if(strcmp(symbol + i, libInfo->nativeMethods[j].symbolName) == 0) {
            return libInfo->nativeMethods[j].fnPtr;
        } else {
            //JA_TRACE("NO match %s == %s\n", symbol+i, libInfo->nativeMethods[j].symbolName);
        }
    }
    return NULL;
}

char *nativeLibMapName(char *name) {
    char *buff = sysMalloc(strlen(name) + sizeof(".library") + 1);
    sprintf(buff,"%s.library", name);
    return buff;
}
