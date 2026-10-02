@echo off
setlocal EnableExtensions

rem Native Windows build using the 32-bit MinGW-w64 GCC toolchain.
rem Run from cmd.exe after adding MinGW-w64 and LLVM's lld-link to PATH.

cd /d "%~dp0"
if not defined CC set "CC=i686-w64-mingw32-gcc"
if not defined LINK set "LINK=lld-link"
if not defined BUILD set "BUILD=build-mingw-windows"
if not defined DIST set "DIST=dist-mingw-windows"

where "%CC%" >nul 2>nul || (
  echo ERROR: %CC% was not found in PATH.
  echo Install a 32-bit MinGW-w64 GCC toolchain or set CC explicitly.
  exit /b 1
)
where "%LINK%" >nul 2>nul || (
  echo ERROR: %LINK% was not found in PATH.
  echo Install LLVM lld and add its bin directory to PATH.
  exit /b 1
)

for /f "delims=" %%I in ('"%CC%" -print-file-name=libkernel32.a') do set "KERNEL32=%%I"
for /f "delims=" %%I in ('"%CC%" -print-file-name=libuser32.a') do set "USER32=%%I"
for /f "delims=" %%I in ('"%CC%" -print-file-name=libshell32.a') do set "SHELL32=%%I"
for /f "delims=" %%I in ('"%CC%" -print-file-name=libcomdlg32.a') do set "COMDLG32=%%I"
for /f "delims=" %%I in ('"%CC%" -print-file-name=libmsvcrt.a') do set "MSVCRT=%%I"

if exist "%BUILD%" rmdir /s /q "%BUILD%"
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%BUILD%" || exit /b 1
mkdir "%DIST%" || exit /b 1

set "CFLAGS=-c -O2 -fno-stack-protector -fno-builtin -Wno-incompatible-pointer-types -Wno-attributes -I%BUILD%"

"%CC%" %CFLAGS% win32_host.c -o "%BUILD%\win32_host.obj" || exit /b 1
"%CC%" %CFLAGS% -DWin32Bootstrap=Win32BootstrapEarly2016 -DDllMain=DllMainEarly2016 win32_host_early2016.c -o "%BUILD%\win32_host_early2016.obj" || exit /b 1
"%CC%" %CFLAGS% api_compat.c -o "%BUILD%\api_compat.obj" || exit /b 1

"%LINK%" /dll /machine:x86 /entry:DllMain@12 /def:win32craft.def ^
  /out:"%DIST%\win32craft.dll" /implib:"%BUILD%\win32craft.lib" ^
  "%BUILD%\win32_host.obj" "%BUILD%\win32_host_early2016.obj" ^
  "%BUILD%\api_compat.obj" "%KERNEL32%" "%USER32%" "%SHELL32%" ^
  "%COMDLG32%" "%MSVCRT%" /nodefaultlib /subsystem:windows,6.00 ^
  /nxcompat /dynamicbase /safeseh:no /base:0x62940000 ^
  /stack:0x200000,0x1000 /opt:ref /opt:icf || exit /b 1

for %%D in (
  api-ms-win-core-winrt-error-l1-1-0
  api-ms-win-core-winrt-l1-1-0
  api-ms-win-core-winrt-string-l1-1-0
) do (
  "%LINK%" /dll /machine:x86 /noentry /def:%%D.def ^
    "%BUILD%\win32craft.lib" /out:"%DIST%\%%D.dll" ^
    /nodefaultlib /subsystem:windows,6.00 /nxcompat /dynamicbase ^
    /opt:ref || exit /b 1
)

echo.
echo Build completed: %CD%\%DIST%
dir /b "%DIST%\*.dll"
exit /b 0
