# Win32Craft - A UWP wrapper designed to run "Minecraft: Windows 10 Edition" on Windows 7/8/8.1 as well as Wine

## Supported Versions (UWP x86)
- 0.13.2
- 0.14.2
- 0.15.10
- 1.1.5
- 1.2.8
- 1.16.20.03 (Experimental)

## Build
Platform|Script
---|---
Windows|build_windows_mingw.bat
Linux (cross-compilation to run in Wine)|./build_mingw.sh

### Patch Example
```
cd tools
python 0.13.2-patcher.py Minecraft.Win10.DX11.exe
```
## Requirements

Requirement|Link
---|---
The original data folder and .exe file from "Minecraft: Windows 10 Edition"|https://www.mcappx.com/bedrock
Win32 FMOD archive|https://www.mediafire.com/file/36ylad9xcz8s7xg/fmod-archive.7z/file

## FMOD
FMOD|Versions
---|---
FMOD 1.06.07|0.13.2-1.1.5 support
FMOD 1.09.04|1.2.8+ support

## Donate
BTC: `bc1qkkckr4d8f9l4me5efnd3e5sf4q6akfhe26lpjy`
