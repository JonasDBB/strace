#include <unistd.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/uio.h>
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include "strace.h"

extern char** environ;
extern const char* const syscall_names[];

void fatal_error(char* err_str, int exit_status) {
    if (errno != 0) {
        perror("perror");
    }
    PR("%s", err_str);
    exit(exit_status);
}

struct user_regs_struct get_syscall_data(pid_t child) {
    struct iovec iov;
    struct user_regs_struct regs;
    iov.iov_base = &regs;
    iov.iov_len = sizeof(regs);

    if (ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov) == -1) {
        fatal_error("ptrace getregset failed", 1);
    }

    return regs;
}

int wait_for_syscall(pid_t child) {
    int status;
    while (1) {
        if (ptrace(PTRACE_SYSCALL, child, 0, 0) == -1) {
            fatal_error("ptrace syscall failed", 1);
        }
        if (waitpid(child, &status, 0) == -1) {
            fatal_error("waitpid failed", 1);
        }

        if (WIFSTOPPED(status)) {
            if (WSTOPSIG(status) & 0x80) {
                return 0;
            }
            PR("stopped by something else");
        }

        if (WIFEXITED(status)) {
            PR("+++ exited with %d +++", status);
            return 1;
        }
    }
}

int do_trace(pid_t child) {
    if (ptrace(PTRACE_SEIZE, child, 0, PTRACE_O_TRACESYSGOOD) == -1) {
        fatal_error("ptrace seize failed", 1);
    }

    if (ptrace(PTRACE_INTERRUPT, child, 0, 0) == -1) {
        fatal_error("ptrace interrupt failed", 1);
    }

    int status;
    if (waitpid(child, &status, 0) == -1) {
        fatal_error("waitpid failed", 1);
    }

    while (1) {
        if (wait_for_syscall(child) == 1) {
            break;
        }

        struct user_regs_struct regs = get_syscall_data(child);
        unsigned long long syscall_nr;
        unsigned long long ret_val;

#ifdef __i386__
        syscall_nr = regs.orig_eax;
        ret_val = regs.eax;
#else
        syscall_nr = regs.orig_rax;
        ret_val = regs.rax;
#endif

        // todo: gets printed twice for some reason, first with huge weird ret val, then with correct ret val
        PR("syscall nr [%llu] %s with ret %llu", syscall_nr, syscall_names[syscall_nr], ret_val);
    }

    return 0;
}

int execute_child(char* execfile, char** av) {
//    kill(getpid(), SIGSTOP);
    execve(execfile, av, environ);
    fatal_error("execve failed", 1);
    return 1;
}

int main(int ac, char** av) {
    if (ac < 2) {
        fatal_error("ft_strace: must have PROG [ARGS] or -p PID\nTry \'strace-h\' for more information.", 1);
    }
    // TODO: maybe implement -h for help
    char exec_filename[NAMEBUF_SIZE];
    exec_filename[0] = 0;
    get_exec_name(av[1], exec_filename);
    PR("%s", exec_filename);
    if (exec_filename[0] == 0) {
        // TODO: insert execfile
        fatal_error("strace: Can't stat 'EXECFILE': No such file or directory", 1);
    }
    pid_t child = fork();
    if (child == -1) {
        fatal_error("fork failed", 1);
    }
    if (child == 0) {
        return execute_child(exec_filename, av + 1);
    } else {
        return do_trace(child);
    }
}