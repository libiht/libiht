#include "../../commons/api.h"
#include "../../kmd/kmd/kmd.h"
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

int main() {
    /*struct lbr_ioctl_request lbr_query = enable_lbr(0);
    cnt = 10;
    printf("%u %llu\n", lbr_query.lbr_config.pid, lbr_query.lbr_config.lbr_select);
    func1();
    dump_lbr(lbr_query);
    printf("%llu\n", lbr_query.buffer->lbr_tos);
    for (int i = 0; i < lbr_query.buffer->lbr_tos; i++) {
        printf("0x%llx 0x%llx; ", lbr_query.buffer->entries[i].from, lbr_query.buffer->entries[i].to);
    }
    printf("\n");
    disable_lbr(lbr_query);*/

    struct bts_ioctl_request bts_query = enable_bts(0);
    cnt = 10;
    printf("%d %llu %llu\n", bts_query.bts_config.pid, bts_query.bts_config.bts_config, bts_query.bts_config.bts_buffer_size);
    func1();
    dump_bts(bts_query);
    int bts_tos = 32;
    printf("%d\n", bts_tos);
    for (int i = 0; i < bts_tos; i++) {
        printf("0x%llx 0x%llx %llu\n", bts_query.buffer->bts_buffer_base[i].from, bts_query.buffer->bts_buffer_base[i].to, bts_query.buffer->bts_buffer_base[i].misc);
    }
    printf("\n");
    disable_bts(bts_query);
    return 0;
}
