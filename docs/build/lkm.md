# Build Instruction for LKM (Linux Kernel Module)

The LKM (Linux Kernel Module) component of LIBIHT provides functionality for retrieving raw hardware trace information from the Intel processors within the Linux kernel.

## Disclaimer

Following this procedure will run some scripts, code, and command as system-level privileged user, and load a kernel module/driver. This is **inherently dangerous** and **should not** be done on a production system or any system that contains sensitive data. We **highly recommend** you to prepare a seperate machine for LIBIHT.

By proceeding, you acknowledge that you are doing so at your own risk. The authors of this document and the LIBIHT project are not responsible for any damage or loss of data that may occur as a result of following these instructions.

## Build

If you wish to build the LKM component of LIBIHT yourself, you will need to install the [GNU Make](https://www.gnu.org/software/make/), build essentials and Linux header. On Ubuntu, you can install these dependencies using the following command:

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

Once installed, you can build the LKM component of LIBIHT using the provided Makefile. Navigate to the `kernel/lkm` directory and run the following command to build the LKM component of LIBIHT:

```bash
make
```

## Usage

To use the LKM component of LIBIHT, you will need to load the driver into the Linux kernel. You can do this using the `insmod` command in the terminal. Run the following command to load the driver:

```bash
sudo insmod libiht.ko
```

Once the driver is loaded, you can use IOCTL to interact with `/proc/libiht-info` to access the raw hardware trace information.

Please refer to the article on [loading a Linux kernel module](https://www.cyberciti.biz/faq/linux-how-to-load-a-kernel-module-automatically-at-boot-time/) for more detailed instructions on loading the LKM component of LIBIHT.

## Kernel Debugging

Kernel debugging on Linux is a complex topic and is beyond the scope of this document. Please refer to the [Linux Kernel Debugging](https://www.kernel.org/doc/html/latest/dev-tools/kgdb.html) documentation for more detailed instructions on setting up kernel debugging on Linux.
