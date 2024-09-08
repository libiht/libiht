# LibIHT (Intel Hardware Trace Library)

<p align="center">
   <img src="https://i.imgur.com/PbQXUEL.png" style="width: 50%;" alt="LibIHT Logo Desc"/>
</p>

LibIHT (Intel Hardware Trace Library) is a cross platform library framework for managing (i.e., retrieving, analyzing, and visualizing) hardware trace information from Intel processors and helping reverse engineers and security researchers to understand the control flow behavior of the target program. We assume users have a basic understanding of operating systems, computer architecture, and Intel hardware trace capabilities.

Modern Intel CPUs have introduced a suite of hardware features, such as Last Branch Record (LBR) and Branch Trace Store (BTS), which promise to deliver detailed program tracing information with minimal overhead. LibIHT bridge the gap between low-level hardware trace capabilities and high-level user space applications by offering both kernel and user interfaces that hide all the complexity of hardware-assisted tracing and a user-friendly approach to interacting with Intel CPU hardware trace features.

It is proudly brought to you by **[Tencent Security Xuanwu Lab](https://xlab.tencent.com/en/)** and its long-term talent cultivation program **Tencent Spark Talent Program**.

It has been selected by the following security conferences:

- **Black Hat USA 2024**: [LIBIHT: A Cross-Platform Library for Accessing Intel Hardware Trace Features](https://www.blackhat.com/us-24/arsenal/schedule/#libiht-a-cross-platform-library-for-accessing-intel-hardware-trace-features-38754), [Video](https://www.youtube.com/watch?v=4FodtsPNABc), [Slides](https://github.com/ThomasonZhao/Slides/blob/main/BHUS24-Arsenal-Zhao-LibIHT.pdf)

## LibIHT Components

The LibIHT library consists of two main components: kernel space components and user space components.

The kernel space components are responsible for retrieving raw hardware trace information from the Intel processors and process granular trace control operations, such as enabling/disabling hardware trace capabilities, configuring hardware trace parameters.

The user space components are responsible for providing a set of APIs for user space applications to access the raw hardware trace information, control the hardware trace capabilities of the Intel processors, and help with basic analyze of the raw hardware trace information.

### Kernel Space Components

The kernel space components are implemented as a kernel module on Linux and a kernel driver on Windows. By taking advantage of the ring 0 kernel space, the kernel space components can communicate with the Intel processors directly and collect the information as it is generated.

### User Space Components

The user space components are implemented as a shared library on Linux and a dynamic link library on Windows. It interact with the kernel space components to retrieve the raw hardware trace information through IOCTLs. Base on the raw hardware trace information, it provides interfaces for user space applications with more upper-level operations, such as control the hardware trace capabilities of the Intel processors, and help with basic analyze of the raw hardware trace information.

## Build & Usage

Depending on the operating systems and components, detailed instructions are provided in separate documents and directories. Please refer to the following links for more information:

- Build Instructions documents directory: [docs/build](./docs/build/)
  - [Build LibIHT Linux Kernel Components](./docs/build/lkm.md)
  - [Build LibIHT Windows Kernel Components](./docs/build/kmd.md)
  - [Build LibIHT User Space Library](./docs/build/lib.md)

- Usage documents directory: [docs/usage](./docs/usage/)
  - [Usage of LibIHT Kernel Space Components](./docs/usage/kernel.md)
  - [Usage of LibIHT User Space Library](./docs/usage/lib.md)
  - [Usage of LibIHT Debugger Plugin](./docs/usage/plugins.md)

## Contributing

We welcome contributions to the LibIHT project. Here's how you can help:

- **Reporting issues:** If you find any problems or have suggestions about the library, please create an issue on GitHub. Be sure to include as much detail as possible so we can understand and reproduce the problem.

- **Improving the code:** If you'd like to fix a bug or implement a new feature, feel free to fork the repository and submit a pull request. Please make sure your code follows the existing style for consistency.

- **Testing on different platforms:** The library supports Windows 10 and above, and Linux kernel version 5.0 and above. If you have access to other platforms or versions, your help in testing the library would be very valuable.

Thank you for considering contributing to LibIHT!

## FAQ

**Q: What versions of Windows and Linux are supported?**

A: The library is currently been tested on Windows 10 21H1 debug version, and Linux kernel version 5.15.* with Ubuntu 22.04.

**Q: What Intel processors are supported?**

A: The library supports any Intel processors with hardware trace (Intel Processor Trace, Last Branch Record, Brach Trace Store, etc.) capabilities. Please refer to [Intel Software Developer Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) for more information. Typically, morden Intel processors (e.g., Intel Core, Intel Xeon, etc.) are supported.

**Q: Is there any documentation available for the APIs?**

A: Yes, you can find the API documentation are specified in [usage](./docs/usage/) directory.
