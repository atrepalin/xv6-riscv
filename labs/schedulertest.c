#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
    int n = 32;

    for (int i = 0; i < n; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("fork failed at %d\n", i);
            exit(1);
        }

        if (pid == 0) {
            volatile unsigned long x = 0;
            for (unsigned long j = 0; j < 100000000; j++) {
                x += j;
            }

            printf("child %d done\n", getpid());
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        wait(0);
    }

    printf("All %d children finished\n", n);
    exit(0);
}
