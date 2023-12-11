#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

// Device name
#define DEVICE_NAME "libiht-info"

// I/O control table
#define LIBIHT_LKM_IOC_MAGIC 'l'
#define LIBIHT_LKM_IOC_ENABLE_TRACE     _IO(LIBIHT_LKM_IOC_MAGIC, 1)
#define LIBIHT_LKM_IOC_DISABLE_TRACE    _IO(LIBIHT_LKM_IOC_MAGIC, 2)
#define LIBIHT_LKM_IOC_DUMP_LBR         _IO(LIBIHT_LKM_IOC_MAGIC, 3)
#define LIBIHT_LKM_IOC_SELECT_LBR       _IO(LIBIHT_LKM_IOC_MAGIC, 4)

struct ioctl_request{
    unsigned long long lbr_select;
    unsigned int pid;
};

int cnt = 10;

void func1(void);
void func2(void);

void func1()
{
    //printf("func1: %d\n", cnt);
    if (cnt != 0)
    {
        cnt--;
        func2();
    }
}

void func2()
{
    //printf("func2: %d\n", cnt);
    if (cnt != 0)
    {
        cnt--;
        func1();
    }
}

void print_usage()
{
	printf("Usage: lkm-demo [pid] [count]\n");
    printf("pid: the pid of the process want to trace, trace it self if it is 0\n");
	printf("count: the number of recursive function call\n");
	printf("Example: lkm-demo 0 10\n");
    fflush(stdout);
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        print_usage();

    int pid = atoi(argv[1]);
    if (pid == 0)
        pid = getpid();
    cnt = atoi(argv[2]);
    printf("pid: %d, count: %d\n", pid, cnt);
    printf("func1's ptr: 0x%p\nfunc2's ptr: 0x%p\n", &func1, &func2);
    fflush(stdout);
    sleep(1);

    int fd = open("/proc/" DEVICE_NAME, O_RDWR);

    if (fd < 0)
    {
        printf("Failed to open device!\n");
        return 0;
    }

    struct ioctl_request input;
    input.lbr_select = 0;
    input.pid = pid;

    // Simulate critical logic
    func1();

    ioctl(fd, LIBIHT_LKM_IOC_ENABLE_TRACE, &input);
    sleep(1);

    ioctl(fd, LIBIHT_LKM_IOC_DUMP_LBR, &input);
    sleep(1);

    printf("Finished!\n");
    close(fd);
    return 0;
}