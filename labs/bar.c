#include "kernel/types.h"
#include "user/user.h"

int mul(int a, int b) {
    return a * b;
}

int div(int a, int b) {
    return a / b;
}

void main() {
    printf("Hello from bar\n");
}