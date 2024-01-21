#include "../../commons/api.h"
#include <stdio.h>
#include <dlfcn.h>

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

int main(){
    void *handle;
    struct lbr_ioctl_request (*enable_lbr) ();
    void (*disable_lbr)(struct lbr_ioctl_request);
    void (*dump_lbr)(struct lbr_ioctl_request);
    void (*select_lbr)(struct lbr_ioctl_request);

    handle = dlopen("../../lkm/src/lbr_api.so", RTLD_NOW);

    enable_lbr = (struct lbr_ioctl_request (*)()) dlsym(handle, "enable_lbr");
    disable_lbr = (void (*)(struct lbr_ioctl_request)) dlsym(handle, "disable_lbr");
    dump_lbr = (void (*)(struct lbr_ioctl_request)) dlsym(handle, "dump_lbr");
    select_lbr = (void (*)(struct lbr_ioctl_request)) dlsym(handle, "select_lbr");

    struct lbr_ioctl_request query = enable_lbr();
    // printf("%d %llu\n",query.lbr_config.pid, query.lbr_config.lbr_select);
    // func1();
    // dump_lbr(query);
    // printf("%llu\n",query.buffer->lbr_tos);
    // for (int i = 0; i < query.buffer->lbr_tos; i++) {
    //     printf("0x%llx 0x%llx; ", query.buffer->entries[i].from, query.buffer->entries[i].to);
    // }
    // disable_lbr(query);

    return 0;
}
