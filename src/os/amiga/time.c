


#include "time.h"

#ifdef _SYS_CLIB2_STDC_H
void amiga_gettimeofday(struct timeval *ts, struct timezone *__z) {
    GetSysTime(ts);
}
#endif
