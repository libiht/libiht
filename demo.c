#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
        read(fd, buf, 1);
        func2();
    }
}

void func2()
{
    if (cnt != 0)
    {
        cnt--;
        read(fd, buf, 1);
        func1();
    }
}

/*
 * Simple demo to show the power of lbr
 */
int main(int argc, char const *argv[])
{

    fd = open("/proc/libiht-info", O_RDWR);
    func1();
    return 0;
}
