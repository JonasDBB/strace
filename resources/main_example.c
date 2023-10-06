#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <string.h>
#include <signal.h>

// #include <unistd_64.h> potential syscall names

//https://blog.nelhage.com/2010/08/write-yourself-an-strace-in-70-lines-of-code/

int do_child(int ac, char** av) {
    char* args[ac +1];
    memcpy(args, av, ac * sizeof(char*));
    args[ac] = NULL;
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

int wait_for_syscall(pid_t child_pid) {
    int status;
    while (1) {
        ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
        waitpid(child_pid, &status, 0);

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
            return 0;
        }
        if (WIFEXITED(status)) {
            return 1;
        }
    }
}

int do_trace(pid_t child_pid) {
    int status;
    long syscall, ret;

    waitpid(child_pid, &status, 0);

    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD);

    while (1) {
        if (wait_for_syscall(child_pid) != 0) {
            break;
        }
        syscall = ptrace(PTRACE_PEEKUSER, child_pid, sizeof(long) * ORIG_RAX);
        fprintf(stderr, "syscall %ld = ", syscall);

        if (wait_for_syscall(child_pid) != 0) {
            break;
        }
        ret = ptrace(PTRACE_PEEKUSER, child_pid, sizeof(long) * RAX);
        fprintf(stderr, "%ld\n", ret);
    }
    return 0;
}

int main(int ac, char** av) {
    if (ac != 2) {
        fprintf(stderr, "incorrect nr of arguments\n");
    }
    // todo check if av[1] exists and is executable
    pid_t child_pid = fork();
    if (child_pid == 0) {
        return do_child(ac - 1, av + 1);
    } else {
        return do_trace(child_pid);
    }
}
