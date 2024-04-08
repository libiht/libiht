# LIB (Userspace Library Components)

The `lib` directory contains the userspace library component of LIBIHT. It provides a convenient interface for accessing and utilizing Intel hardware trace capabilities from userspace applications.

# Building and Usage

To build the userspace library, use the provided Makefile located in the `lib` directory. Run `make` to compile the library. Once compiled, you can link your applications with the LIBIHT library and utilize the provided APIs for accessing Intel hardware trace capabilities. Refer to the documentation for detailed instructions on building and using the userspace library.

Overview

This lib is divided into two parts: lbr and bts, which provide functionalities for handling lbr and bts data. Before usage, you need to call make to generate the lib_api.so file.
Installation

    First, make sure that the make tool is installed on your system.

    Run the following command in the lib/lkm/src (Linux) or lib/kmd/src (Windows) directory to generate the lib_api.so file:

    ```
    make all (Linux)
    nmake all (Windows)
    ```

    This command will compile the source code and generate the required dynamic linking library file.

LBR
Using LBR

    Call the enable_lbr function, which will return a value of type lbr_ioctl_request.
    Use the returned value to call the dump_lbr or config_lbr functions to retrieve or modify the LBR content.
    When you no longer need to use LBR, call the disable_lbr function and pass the previously obtained value to close LBR and free the memory.

BTS
Using BTS

    Call the enable_bts function, which will return a value of type bts_ioctl_request.
    Use the returned value to call the dump_bts or config_bts functions to retrieve or modify the BTS content.
    When you no longer need to use BTS, call the disable_bts function and pass the previously obtained value to close BTS and free the memory.

Using in GDB

    If you want to use this lib in GDB for simplified usage, follow these steps:

    Run the following command in your GDB session to load the libiht-gdb.py script:

    ```
    source libiht-gdb.py
    ```

    Call enable_lbr() or enable_bts() functions to enable LBR or BTS functionality.

    Use dump_lbr() or dump_bts() functions to retrieve the respective content.

    When you no longer need to use it, call disable_lbr() or disable_bts() functions to close LBR or BTS.
