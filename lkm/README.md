# LKM (Linux Kernel Module)

The `lkm` directory contains the Linux Kernel Module (LKM) component of LIBIHT. It provides functionality for retrieving raw hardware trace information from the Intel processors within the Linux kernel.

## Building

If you wish to build the LKM component of LIBIHT yourself, you will need to install the [GNU Make](https://www.gnu.org/software/make/), build essentials and Linux header. On Ubuntu, you can install these dependencies using the following command:

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

Once installed, you can build the LKM component of LIBIHT using the provided Makefile. Navigate to the `lkm` directory and run the following command to build the LKM component of LIBIHT:

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

## Driver APIs

IOCTLs are used to communicate with the LKM component of LIBIHT. Request format as follows:

```c
struct ioctl_request
{
    u64 lbr_select; // LBR selection bit
    u32 pid;        // Process ID
};
```

The LKM component of LIBIHT provides the following IOCTLs for user space components to access the raw hardware trace information:

- `LIBIHT_LKM_IOC_ENABLE_TRACE` - Enable hardware trace capabilities for the assigned process id (PID) and all its child processes.
- `LIBIHT_LKM_IOC_DISABLE_TRACE` - Disable hardware trace capabilities for the assigned process id (PID) and all its child processes.
- `LIBIHT_LKM_IOC_DUMP_LBR` - Dump the Last Branch Record (LBR) stack information for the assigned process id (PID).
- `LIBIHT_LKM_IOC_SELECT_LBR` - Update the Last Branch Record (LBR) selection bits for the assigned process id (PID).
