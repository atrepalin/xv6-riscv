#include "kernel/types.h"
#include "user/user.h"

int sum(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

void main() {
    printf("Hello from foo\n");
}