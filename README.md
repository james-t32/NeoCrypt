# NeoCrypt

NeoCrypt (Revived) is a GUI-based file encryption utility for Windows.

## About

This project is a revival of the open-source application NeoCrypt, developed by Arijit De (available at https://neocrypt.sourceforge.net/). NeoCrypt was developed and released between 2002 and 2005, but it has not been updated since then. This revived version includes minor enhancements aimed at improving the user experience and modernizing the application for better security and compatibility with newer systems.

## Release

To use the application, simply download the zip file from the [releases section](../../releases). Version 1.4 has been tested on Windows 7 and later, including Windows 11 (24H2 release).

## Compiling

The solution files are compatible with Visual Studio 2022. To compile the source code, first download the cryptlib library from [cryptlib's official website](https://www.cs.auckland.ac.nz/~pgut001/cryptlib/). The solution is configured to look for cryptlib in `C:\projects\cryptlib`, but you can modify this path if necessary. After compiling, ensure the cryptlib `cl32.dll` or `cl64.dll` files are placed in the same directory as the NeoCrypt `NeoCrypt.exe` or `NeoCrypt_x64.exe` files.

## Changes since version 1.1 SourceForge release

- Upgraded solution and project files to Visual Studio 2022.
- Updated codebase to C++ 17 and the latest Windows 10 SDK.
  - Provides updated file browser.
- Added the ability to drag and drop files and folders.
- Added the ability to upload password lists from `.csv` or `.txt` files.
  - Useful for decrypting large numbers of files.
- Allowed passwords up to 256 characters.
- Compiled executables for 64-bit systems.
- Improved handling of large files (2+ GB).
- Upgraded the cryptlib library to version 3.4.7.
- Enabled AES algorithm by default.
- Disabled outdated and insecure algorithms.
  - DES is available only for decryption.

## Notes

Version 1.4 of NeoCrypt can open and decrypt files encrypted by earlier versions. However, files encrypted with version 1.4 cannot be decrypted by older versions due to enhanced security features in cryptlib (i.e., a higher default password iteration count).

While the front-end allows passwords up to 256 characters, cryptlib limits this to 64 characters by default. To extend the limit, you can modify `CRYPT_MAX_TEXTSIZE` in the `cryptlib.h` file by changing the line:
```c
#define CRYPT_MAX_TEXTSIZE 256
```
