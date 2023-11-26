# KMD (Kernel Mode Driver)

The `kmd` directory contains the Windows Kernel Mode Driver (KMD) component of LIBIHT. It provides functionality for retrieving raw hardware trace information from the Intel processors within the Windows kernel.

## Building

If you wish to build the KMD component of LIBIHT yourself, you will need to install the [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) and [Visual Studio](https://visualstudio.microsoft.com/downloads/). Once installed, you can build the KMD component of LIBIHT using the provided Visual Studio solution file (`kmd.sln`).

Navigate to the `kmd` directory and run the following command to build the KMD component of LIBIHT:

```powershell
msbuild .\kmd.vcxproj
```

Please refer to the Microsoft's documentation on [building a driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-a-driver) for more detailed instructions on building the KMD component of LIBIHT.

## Usage

To use the KMD component of LIBIHT, you will need to load the driver into the Windows kernel. You can do this using the `sc` command in the command prompt. Run the following command to load the driver:

```powershell
sc create libiht type= kernel binPath= <path-to-kmd.sys>
```

Once the driver is loaded, you can use the `libiht` user library to access the raw hardware trace information.

Please refer to the article on [loading a Windows kernel driver](https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/loading-a-windows-kernel-driver-osr-driver-loader-debugging-with-source-code) for more detailed instructions on loading the KMD component of LIBIHT.

## Driver APIs

IOCTLs are used to communicate with the KMD component of LIBIHT. Request format as follows:

```c
struct ioctl_request
{
    u64 lbr_select; // LBR selection bit
    u32 pid;        // Process ID
};
```

The KMD component of LIBIHT provides the following IOCTLs for user space components to access the raw hardware trace information:

- `LIBIHT_KMD_IOC_ENABLE_TRACE` - Enable hardware trace capabilities for the assigned process id (PID) and all its child processes.
- `LIBIHT_KMD_IOC_DISABLE_TRACE` - Disable hardware trace capabilities for the assigned process id (PID) and all its child processes.
- `LIBIHT_KMD_IOC_DUMP_LBR` - Dump the Last Branch Record (LBR) stack information for the assigned process id (PID).
- `LIBIHT_KMD_IOC_SELECT_LBR` - Update the Last Branch Record (LBR) selection bits for the assigned process id (PID).

## Useful Links

- [Microsoft's documentation on building a driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-a-driver)
- [Microsoft's documentation on getting started with WinDbg](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/getting-started-with-windbg--kernel-mode-)
- [Microsoft's documentation on debugging a universal driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-universal-drivers---step-by-step-lab--echo-kernel-mode-)
