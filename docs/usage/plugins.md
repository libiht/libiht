# Debugger Plugin Usage

The debugger plugin is a cross-platform plugin framework for debugging and analyzing the control flow behavior of the target program. It acts as another layer of abstraction on top of the LibIHT user space library, providing a user-friendly interface for debugging and analyzing the control flow behavior of the target program. For more information about the LibIHT user space library, please refer to the [LibIHT User Library Usage](./lib.md).

Currently it supports GDB and WinDbg as the backend debugger. 

## GDB Plugin

To start with, you need to source the GDB plugin script in GDB. The GDB plugin script is located at `lib/lkm/src/libiht-gdb.py`. You can source the script by running the following command in GDB, or add it to your `.gdbinit` file:

```
source /path/to/libiht-gdb.py
```

After sourcing the script, you can use the following commands to interact with the GDB plugin:

- `enable_lbr`: Enable Last Branch Record (LBR) tracing.
- `disable_lbr`: Disable Last Branch Record (LBR) tracing.
- `dump_lbr`: Dump Last Branch Record (LBR) trace information. It will print the LBR trace information to the console. If valid symbols are available, it will also print the function names.
- `config_lbr`: Config the Last Branch Record (LBR) trace information.

We recommend you to use the GDB plugin in the following way:

1. Start GDB with the target program.
2. Source the GDB plugin script.
3. Enable LBR tracing at the entry point.
4. Break at the ending point of the critical logic you want to analyze or interested in. (Because the LBR trace information is only available for limited amount of branch jumps, we recommend you to use LBR tracing for small functions or basic blocks.)
5. Dump the LBR trace information at the ending point.
8. Analyze the LBR trace information.
9. Continue the execution of the target program.
10. Repeat the above steps for different functions or basic blocks.
11. Disable LBR tracing when you finish the analysis.
12. Exit GDB.

## WinDbg Plugin

TODO