#include <iostream>
#include <Windows.h>
#include <winioctl.h>

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

#define LIBIHT_KMD_IOC_ENABLE_TRACE		CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DISABLE_TRACE    CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_DUMP_LBR			CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define LIBIHT_KMD_IOC_SELECT_LBR		CTL_CODE(KMD_IOCTL_TYPE, KMD_IOCTL_FUNC + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
 * The struct used for I/O control communication
 */
struct ioctl_request
{
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

    ioctl_request input, output;
    input.lbr_select = 0;
    input.pid = pid;
    memset(&output, 0, sizeof(output));

    DWORD ref_len = 0;

    // Simulate critical logic
    func1();

    DeviceIoControl(hDevice, LIBIHT_KMD_IOC_ENABLE_TRACE, &input, sizeof(input), &output,
        sizeof(output), &ref_len, 0);
    Sleep(1000);

    DeviceIoControl(hDevice, LIBIHT_KMD_IOC_DUMP_LBR, &input, sizeof(input), &output,
        sizeof(output), &ref_len, 0);
    Sleep(1000);

    printf("Finished!\n");
    CloseHandle(hDevice);
    return 0;
}
