#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "strace.h"

extern char** environ;

#define MAX_PRINT_SIZE 1024
void print_line(const char* file, const char* func, int line, const char* fmt, ...) {
    char buf[MAX_PRINT_SIZE];
    va_list varlist;
    va_start(varlist, fmt);
    vsnprintf(buf, MAX_PRINT_SIZE, fmt, varlist);
    va_end(varlist);

    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm* tm = localtime(&t.tv_sec);

    fprintf(stderr, "%02d:%02d:%02d.%03d| %s:%d: %s(): [%s]\n", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(t.tv_usec / 1000), file, line, func, buf);
}

static bool is_exec(const char* name) {
    struct stat sb;
    if (stat(name, &sb) != 0) {
        return false;
    }
    if (sb.st_mode & S_IXUSR) {
        return true;
    }
    return false;
}

void get_exec_name(const char* filename, char* buf) {
    if (is_exec(filename)) {
        strncpy(buf, filename, NAMEBUF_SIZE);
        return;
    }
    
    char path_buf[NAMEBUF_SIZE];

    int str_it = 0;
    const char* path = getenv("PATH");

    while (1) {
        int buf_it = 0;
        if (path[str_it] == 0) {
            break;
        }
        for (; path[str_it] != ':' && path[str_it] && buf_it < NAMEBUF_SIZE; ++str_it, ++buf_it) {
            path_buf[buf_it] = path[str_it];
        }
        if (path[str_it] == ':') {
            ++str_it;
        }

        if (buf_it < NAMEBUF_SIZE) {
            path_buf[buf_it++] = '/';
        }
        for (int k = 0; filename[k] && buf_it < NAMEBUF_SIZE; ++k, ++buf_it) {
            path_buf[buf_it] = filename[k];
        }

        if (buf_it == NAMEBUF_SIZE) {
            // too long, not checking
            continue;
        }
        path_buf[buf_it++] = 0;
        if (is_exec(path_buf)) {
            strncpy(buf, path_buf, NAMEBUF_SIZE);
            return;
        }
    }
}
