#include "api.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>

unsigned long fd, result;

struct lbr_request enable_lbr()
{
    struct lbr_request request;
    request.pid = getpid();
    request.lbr_select = 0;
    request.content = NULL;
    fd = open("/proc/" DEVICE_NAME, O_RDWR);
    ioctl(fd, LIBIHT_LKM_IOC_ENABLE_TRACE, &request);
    return request;
}

void disable_lbr(struct lbr_request *user_req)
{
    ioctl(fd, LIBIHT_LKM_IOC_DISABLE_TRACE, user_req);
}

void dump_lbr(struct lbr_request *user_req)
{
    ioctl(fd, LIBIHT_LKM_IOC_DUMP_LBR, user_req);
}

void copy_lbr(struct lbr_request *user_req)
{
    // ioctl(fd, LIBIHT_LKM_IOC_COPY_LBR, user_req);
    __asm__ volatile (
        "movq $1, %%rdi\n"          // 将文件描述符传递给 rdi 寄存器
        "movq $2, %%rsi\n"          // 将请求值传递给 rsi 寄存器
        "movq $3, %%rdx\n"          // 将 user_req 地址传递给 rdx 寄存器
        "movq $4, %%rax\n"         // 设置系统调用号为对应的 IOCTL 系统调用号
        "syscall\n"                 // 执行系统调用
        "movq %%rax, %0"            // 将返回值保存到 result 变量中
        : "=r" (result)
        : "r" ((unsigned long)fd), "r" ((unsigned long)LIBIHT_LKM_IOC_COPY_LBR), "r" (user_req), "r" (__NR_ioctl)
        : "rdi", "rsi", "rdx", "rax"
    );

}

void select_lbr(struct lbr_request *user_req)
{
    ioctl(fd, LIBIHT_LKM_IOC_SELECT_LBR, user_req);
}