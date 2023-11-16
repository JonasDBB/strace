#ifndef STRACE_STRACE_H
#define STRACE_STRACE_H
#include <string.h>
#include <stdbool.h>

#define UNUSED (void)

#ifdef __x86_64__
    #if SIZE_MAX == 0xFFFFFFFF
        #define x32
    #else
        #define x86_64
    #endif
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
void print_line(const char* file, const char* func, int line, const char* fmt, ...);
#define PR(fmt, ...) print_line(__FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define NAMEBUF_SIZE 256
void get_exec_name(const char* filename, char* buf);

#endif //STRACE_STRACE_H
