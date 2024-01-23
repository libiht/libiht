#include "../../commons/api.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define DEVICE_NAME "libiht-info"

#define LIBIHT_LKM_IOCTL_MAGIC 'l'
#define LIBIHT_LKM_IOCTL_BASE       _IO(LIBIHT_LKM_IOCTL_MAGIC, 0)

int fd;

struct xioctl_request send_request;

struct lbr_ioctl_request enable_lbr(){
    fprintf(stderr,"LIBIHT-API: starting enable LBR\n");
    struct lbr_ioctl_request usr_request;
    usr_request.lbr_config.pid = getpid();
    usr_request.lbr_config.lbr_select = 0;

    usr_request.buffer = NULL;

    usr_request.buffer = malloc(sizeof (struct lbr_data));
    usr_request.buffer->lbr_tos = 0;
    usr_request.buffer->entries = 0;

    fprintf(stderr, "LIBIHT-API: enable LBR for pid %d\n", usr_request.lbr_config.pid);

    fd = open("/proc/" DEVICE_NAME, O_RDWR);

    send_request.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    send_request.body.lbr = usr_request;
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &send_request);
    return usr_request;
}

void disable_lbr(struct lbr_ioctl_request usr_request){
    send_request.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: disable LBR for pid %d\n", usr_request.lbr_config.pid);
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &send_request);
    fd = 0;
}

void dump_lbr(struct lbr_ioctl_request usr_request){
    send_request.cmd = LIBIHT_IOCTL_DUMP_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr ,"LIBIHT-API: dump LBR for pid %d\n", usr_request.lbr_config.pid);
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &send_request);
}

void select_lbr(struct lbr_ioctl_request usr_request){
    send_request.cmd = LIBIHT_IOCTL_SELECT_LBR;
    send_request.body.lbr = usr_request;
    fprintf(stderr, "LIBIHT-API: select LBR for pid %d\n", usr_request.lbr_config.pid);
    ioctl(fd, LIBIHT_LKM_IOCTL_BASE, &send_request);
}
