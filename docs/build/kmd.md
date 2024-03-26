# Build Instruction for KMD (Kernel Mode Driver)

The KMD(Kernel Mode Driver) component of LIBIHT provides functionality for retrieving raw hardware trace information from the Intel processors within the Windows kernel.

## Disclaimer

Following this procedure will run some scripts, code, and command as system-level privileged user, and load a kernel module/driver. This is **inherently dangerous** and **should not** be done on a production system or any system that contains sensitive data. We **highly recommend** you to prepare a seperate machine for LIBIHT.

By proceeding, you acknowledge that you are doing so at your own risk. The authors of this document and the LIBIHT project are not responsible for any damage or loss of data that may occur as a result of following these instructions.

## Build

If you wish to build the KMD component of LIBIHT yourself, you will need to install the [Software Development Kit (SDK)](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) and [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) and [Visual Studio](https://visualstudio.microsoft.com/downloads/). (Typically SDK will come with the Visual Studio, but it is recommended to install the latest SDK and WDK). Once installed, you can build the KMD component of LIBIHT using the provided Visual Studio solution file (`kmd.sln`).

Navigate to the `kernel/kmd` directory and run the following command to build the KMD component of LIBIHT:

```powershell
msbuild /p:Configuration=Debug /p:Platform=x64 .\kmd.sln
```

Please refer to the Microsoft's documentation on [building a driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-a-driver) for more detailed instructions on building the KMD component of LIBIHT.

## Usage

To use the KMD component of LIBIHT, you will need to load the driver into the Windows kernel. You can do this using the `sc` command in the command prompt. Run the following command to load the driver:

```powershell
sc create libiht type= kernel binPath= <path-to-kmd.sys>
```

Once the driver is loaded, you can use the IOCTL to interact with Windows file device `\\\\.\\libiht-info` to access the raw hardware trace information.

Please refer to the article on [loading a Windows kernel driver](https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/loading-a-windows-kernel-driver-osr-driver-loader-debugging-with-source-code) for more detailed instructions on loading the KMD component of LIBIHT.

## Kernel Debugging

Luckily, Microsoft provides a powerful feature to do remote kernel debug on Windows. Please refer to the Microsoft's documentation on [set up KDNET network kernel debugging manually](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-network-debugging-connection) for more detailed instructions on setting up KDNET network kernel debugging.

## Useful Links

- [Microsoft's documentation on building a driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-a-driver)
- [Microsoft's documentation on getting started with WinDbg](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/getting-started-with-windbg--kernel-mode-)
- [Microsoft's documentation on debugging a universal driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-universal-drivers---step-by-step-lab--echo-kernel-mode-)
