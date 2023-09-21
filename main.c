#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>

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
