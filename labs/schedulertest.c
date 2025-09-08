#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// CPU-bound: чистые вычисления
void run_cpu(int n) {
    printf("\n=== CPU test (%d processes) ===\n", n);

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
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        wait(0);
    }

    printf("CPU test done\n");
}

static char* append_str(char *p, const char *s) {
    while (*s) *p++ = *s++;
    *p = 0;
    return p;
}

static char* utoa_dec(char *p, unsigned int x) {
    if (x == 0) { 
        *p++ = '0'; 
        *p = 0; 
        return p; 
    }

    char tmp[16];
    int i = 0;

    while (x) {
        tmp[i++] = '0' + (x % 10);
        x /= 10;
    }
    for (int j = i - 1; j >= 0; j--) 
        *p++ = tmp[j];

    *p = 0;
    return p;
}

// I/O-bound: пишем в консоль
void run_io(int n) {
    printf("\n=== IO test (%d processes) ===\n", n);

    for (int i = 0; i < n; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("fork failed at %d\n", i);
            exit(1);
        }
        if (pid == 0) {
            int mypid = getpid();

            char fname[32];
            char *p = fname;
            p = append_str(p, "io_");
            p = utoa_dec(p, mypid);
            p = append_str(p, ".out");

            int fd = open(fname, O_CREATE | O_WRONLY);

            for (unsigned long j = 0; j < 100; j++) {
                write(fd, "x", 1);
            }

            close(fd);

            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        wait(0);
    }

    printf("IO test done\n");
}

int main()
{
    log_time(); // включаем сбор статистики в ядре

    run_cpu(16);
    run_io(16);

    printf("\nAll tests finished\n");
    exit(0);
}
