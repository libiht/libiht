#include "../../commons/api.h"
#include <stdio.h>

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
    struct lbr_ioctl_request query = enable_lbr(0);
    printf("%d %llu\n",query.lbr_config.pid, query.lbr_config.lbr_select);
    func1();
    dump_lbr(query);
    printf("%llu\n",query.buffer->lbr_tos);
    for (int i = 0; i < query.buffer->lbr_tos; i++) {
        printf("0x%llx 0x%llx; ", query.buffer->entries[i].from, query.buffer->entries[i].to);
    }
    printf("\n");
    disable_lbr(query);

    return 0;
}
