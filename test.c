#include <unistd.h>

int main() {
    write(1, "test\n", 5);
    return 0;
//    int x = 1;
//    --x;
//    return x;
}
