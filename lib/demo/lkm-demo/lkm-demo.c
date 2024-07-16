////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/demo/lkm-demo/lkm-demo.c
//  Description    : This is a simple demo for how to use LIBIHT LKM APIs to
//                   enable LBR and dump LBR for a user-space application.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 16, 2024
//

#include "../../commons/api.h"
#include "../../lkm/include/lkm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ENABLE_BLR
// #define ENABLE_BTS

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

int main(int argc, char* argv[]){
    if (argc != 3)
        print_usage();

    int pid = atoi(argv[1]);
    if (pid == 0)
        pid = getpid();
    cnt = atoi(argv[2]);
    printf("pid: %d, count: %d\n", pid, cnt);
    printf("func1's ptr: %p\nfunc2's ptr: %p\n", &func1, &func2);
    fflush(stdout);
    sleep(1);

#ifdef ENABLE_LBR
    // Enable LBR
    struct lbr_ioctl_request query = enable_lbr(0);

    // Simulate critical logic
    func1();

    // Dump LBR
    dump_lbr(query);

    // Disable LBR
    disable_lbr(query);

    // Print LBR buffer
    printf("LBR TOS: %lld\n", query.buffer->lbr_tos);
    for (int i = 0; i < 32; i++)
    {
        printf("LBR[%d]: 0x%llx -> 0x%llx\n", i, query.buffer->entries[i].from, query.buffer->entries[i].to);
    }
#endif ENABLE_LBR
#ifdef ENABLE_BTS
    // Enable BTS
    struct bts_ioctl_request query = enable_bts(0);

    // Simulate critical logic
    func1();

    // Dump BTS
    dump_bts(query);

    // Print BTS buffer
    int bts_tos = 32;
    printf("%d\n", bts_tos);
    for (int i = 0; i < bts_tos; i++) {
        printf("0x%llx 0x%llx %llu\n", bts_query.buffer->bts_buffer_base[i].from, bts_query.buffer->bts_buffer_base[i].to, bts_query.buffer->bts_buffer_base[i].misc);
    }
    printf("\n");
    disable_bts(bts_query);
    return 0;
}
