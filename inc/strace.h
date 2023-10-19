#ifndef STRACE_STRACE_H
#define STRACE_STRACE_H

#ifdef __x86_64__
    #if SIZE_MAX == 0xFFFFFFFF
        #define x32
    #else
        #define x86_64
    #endif
#endif

#endif //STRACE_STRACE_H
