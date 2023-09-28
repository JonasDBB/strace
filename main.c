#include <unistd.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/uio.h>
#include <elf.h>
#include <stdlib.h>

extern char** environ;

void fatal_error(char* err_str, int exit_status) {
    if (errno != 0) {
        perror("error: ");
    }
    fprintf(stderr, "%s\n", err_str);
    exit(exit_status);
}

long get_syscall_nr(pid_t child) {
    int status;
    struct iovec iov;
    iov.iov_base = &status;
    iov.iov_len = sizeof(status);

    if (ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov) == -1) {
        fatal_error("ptrace getregset failed", 1);
    }

    return ((long*)iov.iov_base)[ORIG_RAX / sizeof(long)];
}

int do_trace(pid_t child) {
    fprintf(stderr, "child pid is %d\n", child);

    int status;
    if (waitpid(child, &status, 0) == -1) { // this hangs for some reason :(
        fatal_error("waitpid failed", 1);
    }

    if (ptrace(PTRACE_SEIZE, child, 0, PTRACE_O_TRACESYSGOOD) == -1) {
        fatal_error("ptrace seize failed", 1);
    }

    if (ptrace(PTRACE_INTERRUPT, child, 0, 0) == -1) {
        fatal_error("ptrace interrupt failed", 1);
    }

    while (!WIFEXITED(status)) {
        if (ptrace(PTRACE_LISTEN, child, 0, 0) == -1) {
            fatal_error("ptrace listen failed", 1);
        }

        if (waitpid(child, &status, 0) == -1) {
            fatal_error("waitpid failed", 1);
        }

        if (WIFSTOPPED(status)) {
            long syscall_nr = get_syscall_nr(child);
            fprintf(stderr, "syscall nr %ld\n", syscall_nr);
        }
    }

    return 0;
}

int execute_child(char** av) {
    kill(getpid(), SIGSTOP);
    execve(*av, av, environ);
    fatal_error("execve failed", 1);
    return 1;
}

int main(int ac, char** av) {
    if (ac < 2) {
        fatal_error("ft_strace: must have PROG [ARGS] or -p PID\nTry \' strace-h\' for more information.", 1);
    }
    // TODO: check if av[1] exists and is executable
    // TODO: maybe implement -h for help
    pid_t child = fork();
    if (child == -1) {
        fatal_error("fork failed", 1);
    }
    if (child == 0) {
        return execute_child(av + 1);
    } else {
        return do_trace(child);
    }
}