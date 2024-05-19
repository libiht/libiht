////////////////////////////////////////////////////////////////////////////////
//
//  File           : demo/kmd-demo/kmd-demo/kmd-demo.cpp
//  Description    : This is the main program for the kmd-demo program. It will
//                   open the helper device and send the ioctl request to the
//                   helper device. The kernel driver will trace the process 
//                   and dump the LBR to show the call stack information of the
//                   cross recursive function call.
//
//   Author        : Thomason Zhao
//   Last Modified : May 25, 2023
//

// Include Files
#include <iostream>
#include <Windows.h>
#include <winioctl.h>

#define ENABLE_LBR
//#define ENABLE_BTS

/*
 * I/O Device name
 */
#define DEVICE_NAME			L"\\Device\\libiht-info"
#define SYM_DEVICE_NAME		L"\\DosDevices\\libiht-info"

 /*
  * I/O control table
  */
#define KMD_IOCTL_TYPE 0x8888
#define KMD_IOCTL_FUNC 0x888

#define LIBIHT_KMD_IOCTL_BASE       CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Library constants
enum IOCTL {
    LIBIHT_IOCTL_BASE,          // Placeholder

    // LBR
    LIBIHT_IOCTL_ENABLE_LBR,
    LIBIHT_IOCTL_DISABLE_LBR,
    LIBIHT_IOCTL_DUMP_LBR,
    LIBIHT_IOCTL_CONFIG_LBR,
    LIBIHT_IOCTL_LBR_END,       // End of LBR

    // BTS
    LIBIHT_IOCTL_ENABLE_BTS,
    LIBIHT_IOCTL_DISABLE_BTS,
    LIBIHT_IOCTL_DUMP_BTS,
    LIBIHT_IOCTL_CONFIG_BTS,
    LIBIHT_IOCTL_BTS_END,       // End of BTS
};

//
// LBR Type definitions

// Define LBR stack entry
struct lbr_stack_entry
{
    unsigned long long from;   // Retrieve from MSR_LBR_NHM_FROM + offset
    unsigned long long to;     // Retrieve from MSR_LBR_NHM_TO + offset
};

// Define LBR configuration
struct lbr_config
{
    unsigned int pid;                          // Process ID
    unsigned long long lbr_select;                   // MSR_LBR_SELECT
};

// Define LBR data
struct lbr_data
{
    unsigned long long lbr_tos;                      // MSR_LBR_TOS
    struct lbr_stack_entry* entries;  // LBR stack entries
};

// Define the lbr IOCTL structure
struct lbr_ioctl_request {
    struct lbr_config lbr_config;
    struct lbr_data* buffer;
};

//
// BTS Type definitions

// Define BTS record
struct bts_record
{
    unsigned long long from;   // branch from
    unsigned long long to;     // branch to
    unsigned long long misc;   // misc information
};

// Define BTS configuration
struct bts_config
{
    unsigned int pid;                        // Process ID
    unsigned long long bts_config;                 // MSR_IA32_DEBUGCTLMSR
    unsigned long long bts_buffer_size;            // BTS buffer size
};

// Define BTS data
// TODO: pay attention when using this struct in dump bts
struct bts_data
{
    struct bts_record* bts_buffer_base; // BTS buffer base
    unsigned long long bts_index;                      // BTS current index
    unsigned long long bts_absolute_maximum;           // BTS absolute maximum
    unsigned long long bts_interrupt_threshold;        // BTS interrupt threshold
};

// Define the bts IOCTL structure
struct bts_ioctl_request {
    struct bts_config bts_config;
    struct bts_data* buffer;
};

//
// xIOCTL Type definitions

// Define the xIOCTL structure
struct xioctl_request {
    enum IOCTL cmd;
    union {
        struct lbr_ioctl_request lbr;
        struct bts_ioctl_request bts;
    } body;
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
	printf("Usage: kmd-demo.exe [pid] [count]\n");
    printf("pid: the pid of the process want to trace, trace it self if it is 0\n");
	printf("count: the number of recursive function call\n");
	printf("Example: kmd-demo.exe 0 10\n");
    fflush(stdout);
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        print_usage();

    int pid = atoi(argv[1]);
    if (pid == 0)
        pid = GetCurrentProcessId();
    cnt = atoi(argv[2]);
    printf("func1's ptr: 0x%p\nfunc2's ptr: 0x%p\n", &func1, &func2);
    fflush(stdout);
    Sleep(1000);

    HANDLE hDevice = CreateFileA("\\\\.\\libiht-info", GENERIC_READ |
        GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open device!\n");
        CloseHandle(hDevice);
        return 0;
    }

    struct xioctl_request input;
    memset(&input, 0, sizeof(input));

#ifdef ENABLE_LBR
    // Setup LBR buffer
    input.body.lbr.buffer = (struct lbr_data*)malloc(sizeof(struct lbr_data));
    input.body.lbr.buffer->entries = (struct lbr_stack_entry*)malloc(32 * sizeof(struct lbr_stack_entry));

    // Enable LBR
    input.body.lbr.lbr_config.lbr_select = 0;
    input.body.lbr.lbr_config.pid = pid;

    input.cmd = LIBIHT_IOCTL_ENABLE_LBR;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    Sleep(1000);

    // Simulate critical logic
    func1();

    // Dump LBR
    input.cmd = LIBIHT_IOCTL_DUMP_LBR;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    printf("LBR TOS: %llx\n", input.body.lbr.buffer->lbr_tos);
    for (int i = 0; i < 32; i++)
    {
        printf("LBR[%d]: %llx -> %llx\n", i, input.body.lbr.buffer->entries[i].from, input.body.lbr.buffer->entries[i].to);
    }
    Sleep(1000);

    // Disable LBR
    input.cmd = LIBIHT_IOCTL_DISABLE_LBR;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    Sleep(1000);
#endif // ENABLE_LBR

#ifdef ENABLE_BTS
    // Enable BTS
    input.body.bts.bts_config.bts_buffer_size = 0;
    input.body.bts.bts_config.bts_config = 0;
    input.body.bts.bts_config.pid = pid;

    input.cmd = LIBIHT_IOCTL_ENABLE_BTS;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    Sleep(1000);

    // Simulate critical logic
    func1();

    // Dump BTS
    input.cmd = LIBIHT_IOCTL_DUMP_BTS;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    Sleep(1000);

    // Disable BTS
    input.cmd = LIBIHT_IOCTL_DISABLE_BTS;
    DeviceIoControl(hDevice, LIBIHT_KMD_IOCTL_BASE, &input, sizeof(input), NULL, 0, NULL, NULL);
    Sleep(1000);
#endif // ENABLE_BTS

    printf("Finished!\n");
    CloseHandle(hDevice);
    return 0;
}
