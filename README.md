# LIBIHT (Intel Hardware Trace Library)

LIBIHT (Intel Hardware Trace Library) is a library framworkd that provides user space and kernel space components for accessing and utilizing Intel hardware trace capabilities. It allows you to extract and analyze hardware trace information from Intel processors. It is brought to you by [Tencent Security Xuanwu Lab](https://xlab.tencent.com/en/) and its long-term talent cultivation program Spark Program.

## Components

### Kernel Space Components

Both Windows and Linux systems are supported. The kernel space components are responsible for retrieving raw hardware trace information from the Intel processors. The kernel space components are implemented as [KMD (Kernel Mode Driver)](https://en.wikipedia.org/wiki/Kernel-Mode_Driver_Framework) on Windows and [LKM (Linux Kernel module)](https://wiki.archlinux.org/title/Kernel_module) on Linux.

They mainly provide the following functionalities:

- Retrieve raw hardware trace information from the Intel processors.
- Provide a kernel maintained datastructure for storing the raw hardware trace information.
- Provide a set of APIs for user space components to access the raw hardware trace information.
- Provide a set of APIs for user space components to control the hardware trace capabilities of the Intel processors.

### User Space Components (TODO)

The user space components are responsible for providing a set of APIs for user space applications to access the raw hardware trace information, control the hardware trace capabilities of the Intel processors, and help with basic analyze of the raw hardware trace information. The user space components are implemented as a shared library on both Windows and Linux.

They mainly provide the following functionalities:

- Provide a set of APIs for user space applications to access the raw hardware trace information.
- Provide a set of APIs for user space applications to control the hardware trace capabilities of the Intel processors.
- Provide a set of APIs for user space applications to help with basic analyze of the raw hardware trace information.
- Provide a set of APIs for user space applications to help with basic visualization of the raw hardware trace information.
- Provide a set of APIs for user space applications to help with basic filtering of the raw hardware trace information.

## Building and Usage

Please refer to the README files in the [`kernel`](./kernel/READEME.md), and [`lib`](./lib/README.md) directories for instructions on building the components of LIBIHT.

**WARNING:** **DO NOT** use the `libiht` components in production environments (especially on you working PC). The `libiht` components are still under active development and have not been thoroughly tested. Use at your own risk.

## Contributing

We welcome contributions to the LIBIHT project. Here's how you can help:

- **Reporting issues:** If you find any problems or have suggestions about the library, please create an issue on GitHub. Be sure to include as much detail as possible so we can understand and reproduce the problem.

- **Improving the code:** If you'd like to fix a bug or implement a new feature, feel free to fork the repository and submit a pull request. Please make sure your code follows the existing style for consistency.

- **Improving the documentation:** The user space components are currently marked as `TODO`. If you have used these components and can provide more information, your contributions to the documentation would be greatly appreciated.

- **Testing on different platforms:** The library supports Windows 10 and above, and Linux kernel version 5.0 and above. If you have access to other platforms or versions, your help in testing the library would be very valuable.

Thank you for considering contributing to LIBIHT!

## FAQ

**Q: What versions of Windows and Linux are supported?**

A: The library supports Windows 10 and above, and Linux kernel version 5.0 and above.

**Q: What Intel processors are supported?**

A: The library supports Intel processors with hardware trace (Intel Processor Trace, Last Branch Record, Brach Trace Store, etc.) capabilities. Please refer to Intel's documentation for a list of supported processors.

**Q: How can I contribute to this project?**

A: Please refer to the [Contributing](#contributing) section of this README for guidelines on how to contribute.

**Q: I encountered an error while using the library. Where can I get help?**

A: You can raise an issue on the project's GitHub page. Please refer to the [Contributing](#contributing) section of this README for guidelines on how to raise an issue.

**Q: Is there any documentation available for the APIs?**

A: Yes, you can find the API documentation in [`kernel`](./lib/README.md), and [`lib`](./lib/README.md) directories.
