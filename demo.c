#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>

char buf[10];
int cnt = 10;
int fd;

void func1(void);
void func2(void);

void func1()
{
    if (cnt != 0)
    {
        cnt--;
        // read(fd, buf, 1);
        func2();
    }
}

void func2()
{
    if (cnt != 0)
    {
        cnt--;
        func1();
    }
}

/*
 * Simple demo to show the power of lbr
 */
int main(int argc, char const *argv[])
{
    cpu_set_t set;

    // Bind process to cpu 0
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(set), &set);

    fd = open("/proc/libiht-info", O_RDWR);
    printf("func1's ptr: 0x%p\nfunc2's ptr: 0x%p\n", &func1, &func2);
    func1();

    if (fork() == 0) {
    read(fd, buf, 1);
    close(fd);
    }

    read(fd, buf, 1);

    close(fd);
    return 0;
}
