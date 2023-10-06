#include <unistd.h>
#include <signal.h>

int main() {
    kill(getpid(), SIGUSR1);
    return 0;
}
