#include <unistd.h>
#include <printf.h>
#include <sys/types.h>
#include <sys/ptrace.h>

int main(int ac, char** av, char** ev) {
    if (ac != 2) {
        fprintf(stderr, "incorrect nr of arguments\n");
    }
    // todo check if av[1] exists and is executable
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execve(av[1], av, ev);
        fprintf(stderr, "execve failed!\n");
        return 0;
    }

    return 0;
}
