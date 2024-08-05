# Debugger Plugin Usage

The debugger plugin is a cross-platform plugin framework for debugging and analyzing the control flow behavior of the target program. It acts as another layer of abstraction on top of the LibIHT user space library, providing a user-friendly interface for debugging and analyzing the control flow behavior of the target program. For more information about the LibIHT user space library, please refer to the [LibIHT User Library Usage](./lib.md).

Currently it supports GDB and WinDbg as the backend debugger. Both LBR and BTS tracing are supported in both GDB and WinDbg. The GDB plugin is more stable and has more features than the WinDbg plugin. We recommend you to use the GDB plugin for better experience.

## GDB Plugin

To start with, you need to source the GDB plugin script in GDB. The GDB plugin script is located at `lib/lkm/src/libiht-gdb.py`. You can source the script by running the following command in GDB, or add it to your `.gdbinit` file:

```
source /path/to/libiht-gdb.py
```

After sourcing the script, you can use the following commands to interact with the GDB plugin:

- LBR trace commands:

  - `enable_lbr`: Enable Last Branch Record (LBR) tracing.
  - `disable_lbr`: Disable Last Branch Record (LBR) tracing.
  - `dump_lbr`: Dump Last Branch Record (LBR) trace information. It will print the LBR trace information to the console. If valid symbols are available, it will also print the function names.
  - `config_lbr`: Config the Last Branch Record (LBR) trace information.

- BTS trace commands:

  - `enable_bts`: Enable Branch Trace Store (BTS) tracing.
  - `disable_bts`: Disable Branch Trace Store (BTS) tracing.
  - `dump_bts`: Dump Branch Trace Store (BTS) trace information. It will print the BTS trace information to the console. If valid symbols are available, it will also print the function names.
  - `config_bts`: Config the Branch Trace Store (BTS) trace information.

We recommend you to use the GDB plugin in the following way:

1. Start GDB with the target program.
2. Source the GDB plugin script.
3. Enable LBR tracing at the entry point.
4. Break at the ending point of the critical logic you want to analyze or interested in. (Because the LBR trace information is only available for limited amount of branch jumps, we recommend you to use LBR tracing for small functions or basic blocks.)
5. Dump the LBR trace information at the ending point.
6. Analyze the LBR trace information.
7. Continue the execution of the target program.
8.  Repeat the above steps for different functions or basic blocks.
9.  Disable LBR tracing when you finish the analysis.
10. Exit GDB.

## WinDbg Plugin

TODO: Not finish yet.

To start with, you need to load the WinDbg plugin DLL in WinDbg. You will first need to compile the KMD user library DLL under the `lib/kmd/kmd` and WinDbg plugin DLL under the `lib/kmd/kmd-ext` directory. For more information about building the KMD user library DLL, please refer to the [Build Instruction for User Space Library](../build/lib.md). Basically, you can just run `msbuild` under both directories to compile the DLLs.

After compiling the DLL, you can load the DLL in WinDbg by running the following command:

```windbg
.load C:\path\to\kmd-ext.dll
```

To unload the DLL, you can run the following command:

```windbg
.unload C:\path\to\kmd-ext.dll
```

We do recommend you to copy the DLL to one of the `PATH` directories (will show up if you use command `.chain`) that WinDbg searches, so that you can just use the DLL name to load it. (Can be something like `C:\Users\<username>\AppData\Local\EngineExtensions\` depending on how your WinDbg is configured.)

For more information about the WinDbg plugin, please refer to official WinDbg documentation about [Loading Debugger Extension DLLs](https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/loading-debugger-extension-dlls).

After loading the WinDbg plugin DLL, you can use the following commands to interact with the WinDbg plugin:

- LBR trace commands:
  - `!EnableLBR`: Enable Last Branch Record (LBR) tracing.
  - `!DisableLBR`: Disable Last Branch Record (LBR) tracing.
  - `!DumpLBR`: Dump Last Branch Record (LBR) trace information. It will print the LBR trace information to the console. If valid symbols are available, it will also print the function names.
  - `!ConfigLBR`: Config the Last Branch Record (LBR) trace information.

- BTS trace commands:
  - `!EnableBTS`: Enable Branch Trace Store (BTS) tracing.
  - `!DisableBTS`: Disable Branch Trace Store (BTS) tracing.
  - `!DumpBTS`: Dump Branch Trace Store (BTS) trace information. It will print the BTS trace information to the console. If valid symbols are available, it will also print the function names.
  - `!ConfigBTS`: Config the Branch Trace Store (BTS) trace information.


Similar to the GDB plugin, we recommend you to use the WinDbg plugin in the following way:

1. Start WinDbg with the target program.
2. Load the WinDbg plugin DLL.
3. Enable LBR tracing at the entry point.
4. Break at the ending point of the critical logic you want to analyze or interested in. (Because the LBR trace information is only available for limited amount of branch jumps, we recommend you to use LBR tracing for small functions or basic blocks.)
5. Dump the LBR trace information at the ending point.
6. Analyze the LBR trace information.
7. Continue the execution of the target program.
8. Repeat the above steps for different functions or basic blocks.
9. Disable LBR tracing when you finish the analysis.
10. Exit WinDbg.
