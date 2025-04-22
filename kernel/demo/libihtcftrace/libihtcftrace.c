/* LD_PRELOAD hook main credit: https://gist.github.com/apsun/1e144bf7639b22ff0097171fa0f8c6b1 */
#define _GNU_SOURCE
#include "libihtcftrace.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct xioctl_request input;

int num_bts_records = 0x400 << 10;
// int num_bts_records = 1024;
int fd = -1;

void setup_bts_buf() {
    memset(&input, 0, sizeof(input));
    input.body.bts.buffer = (struct bts_data *)malloc(sizeof(struct bts_data));
    input.body.bts.buffer->bts_buffer_base = (struct bts_record *)malloc(sizeof(struct bts_record) * num_bts_records);
    memset(input.body.bts.buffer->bts_buffer_base, 0, sizeof(struct bts_record) * num_bts_records);
    printf("BTS buffer: %p\n", input.body.bts.buffer);
    printf("BTS buffer base: %p\n", input.body.bts.buffer->bts_buffer_base);
}

void free_bts_buf() {
    free(input.body.bts.buffer->bts_buffer_base);
    free(input.body.bts.buffer);
}

void flush_bts_buf() {
    FILE *fp = fopen("./libihtcftrace.log", "w");
    for (int i = 0; i < num_bts_records; i++) {
        fprintf(fp, "0x%llx,0x%llx\n", input.body.bts.buffer->bts_buffer_base[i].from, input.body.bts.buffer->bts_buffer_base[i].to);
    }
    fclose(fp);
}

void enable_bts(int pid) {
    input.body.bts.bts_config.bts_buffer_size = sizeof(struct bts_record) * num_bts_records;
    input.body.bts.bts_config.bts_config = 0;
    input.body.bts.bts_config.pid = pid;

    input.cmd = LIBIHT_IOCTL_ENABLE_BTS;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
}

void dump_bts(int pid) {
    input.body.bts.bts_config.pid = pid;
    input.cmd = LIBIHT_IOCTL_DUMP_BTS;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
}

void disable_bts(int pid) {
    input.body.bts.bts_config.pid = pid;
    input.cmd = LIBIHT_IOCTL_DISABLE_BTS;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &input);
}

// int main(int argc, char *argv[]) {
//     fd = open("/proc/" DEVICE_NAME, O_RDWR);
//     memset(&input, 0, sizeof(input));

//     setup_bts_buf();

//     int pid = fork();
//     if (pid == 0) {
//         // Child process
//         printf("Child process: %d\n", getpid());
//         enable_bts(getpid());
//         char *args[] = {"/bin/true", NULL};
//         execv(args[0], args);
//     }
//     // Parent process
//     pid = wait(NULL);

//     // int pid = getpid();
//     // enable_bts(pid);
//     // for (int i = 0; i < 2000; i++) {
//     //     asm volatile("nop");
//     // }

//     dump_bts(pid);
//     disable_bts(pid);
//     flush_bts_buf();
//     free_bts_buf();

//     close(fd);
//     return 0;
// }
/* Trampoline for the real main() */
static int (*main_orig)(int, char **, char **);

/* Our fake main() that gets called by __libc_start_main() */
int main_hook(int argc, char **argv, char **envp)
{
    fd = open("/proc/" DEVICE_NAME, O_RDWR);
    memset(&input, 0, sizeof(input));
    setup_bts_buf();
    int pid = getpid();
    enable_bts(pid);

    int ret = main_orig(argc, argv, envp);

    dump_bts(pid);
    disable_bts(pid);
    flush_bts_buf();
    free_bts_buf();

    close(fd);
    return ret;
}

/*
 * Wrapper for __libc_start_main() that replaces the real main
 * function with our hooked version.
 */
int __libc_start_main(
    int (*main)(int, char **, char **),
    int argc,
    char **argv,
    int (*init)(int, char **, char **),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
    /* Save the real main function address */
    main_orig = main;

    /* Find the real __libc_start_main()... */
    typeof(&__libc_start_main) orig = dlsym(RTLD_NEXT, "__libc_start_main");

    /* ... and call it with our custom main function */
    return orig(main_hook, argc, argv, init, fini, rtld_fini, stack_end);
}