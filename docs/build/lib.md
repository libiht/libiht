# Build Instruction for User Space Library

The User Space Library component of LibIHT provides functionality for retrieving raw hardware trace information from the Intel processors within the user space.

## Linux Build

To build the userspace library, use the provided Makefile located in the `lib` directory. Run `make` to compile the library. Once compiled, you can link your applications with the LibIHT library and utilize the provided APIs for accessing Intel hardware trace capabilities. Refer to the documentation for detailed instructions on building and using the userspace library.

## Windows Build

To build the userspace library on Windows, you will need to install the [Software Development Kit (SDK)](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) and [Visual Studio](https://visualstudio.microsoft.com/downloads/). Once installed, you can build the userspace library using the provided Visual Studio solution file.

Navigate to the `lib/kmd` directory and run the following command to build the KMD component of LibIHT:

```powershell
msbuild /p:Configuration=Debug /p:Platform=x64 .\kmd.sln
```
