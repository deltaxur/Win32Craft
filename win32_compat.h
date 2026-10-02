#ifndef WIN32_COMPAT_H
#define WIN32_COMPAT_H

/* Minimal 32-bit Windows / DXGI / D3D11 declarations for building the host
 * without the Windows SDK. ABI-compatible with i686-pc-windows-msvc. */

typedef unsigned char BYTE;
typedef signed char CHAR;
typedef unsigned short WORD;
typedef WORD ATOM;
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef unsigned short WCHAR;
typedef unsigned short wchar_t;
typedef unsigned int UINT;
typedef unsigned int UINT32;
typedef signed int INT;
typedef signed int BOOL;
typedef signed int LONG;
typedef unsigned int ULONG;
typedef unsigned int DWORD;
typedef unsigned long long ULONGLONG;
typedef signed long long LONGLONG;
typedef unsigned int SIZE_T;
typedef unsigned int ULONG_PTR;
typedef signed int LONG_PTR;
typedef signed int INT_PTR;
typedef unsigned int UINT_PTR;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef LONG HRESULT;
typedef void *HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HICON;
typedef HANDLE HCURSOR;
typedef HANDLE HBRUSH;
typedef HANDLE HMENU;
typedef HANDLE HDC;
typedef HANDLE HMONITOR;
typedef const void *LPCVOID;
typedef void *LPVOID;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef const WCHAR *LPCWSTR;
typedef WCHAR *LPWSTR;
typedef void *HSTRING;
typedef void FILE;
typedef unsigned int size_t;
typedef int ptrdiff_t;

#ifndef __cplusplus
#define nullptr ((void*)0)
#endif
#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define WINAPI __stdcall
#define CALLBACK __stdcall
#define STDMETHODCALLTYPE __stdcall
#define STDAPICALLTYPE __stdcall
#define WINAPIV __cdecl
#define STDMETHODCALLTYPE __stdcall
#define __forceinline __inline
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#define LOWORD(l) ((WORD)((ULONG_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((ULONG_PTR)(l) >> 16) & 0xffff))
#define MAKEINTRESOURCEW(i) ((LPCWSTR)(ULONG_PTR)((WORD)(i)))
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define ZeroMemory(p,n) memset((p),0,(n))
#define FillMemory(p,n,v) memset((p),(v),(n))
#define CopyMemory(d,s,n) memcpy((d),(s),(n))

#define S_OK ((HRESULT)0)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define REGDB_E_CLASSNOTREG ((HRESULT)0x80040154L)

#define PAGE_READWRITE 0x04
#define PAGE_NOACCESS 0x01
#define PAGE_GUARD 0x100
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_READ 0x20
#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define MEM_PRIVATE 0x00020000
#define GENERIC_READ 0x80000000U
#define GENERIC_WRITE 0x40000000U
#define FILE_SHARE_READ 0x00000001U
#define FILE_SHARE_WRITE 0x00000002U
#define FILE_SHARE_DELETE 0x00000004U
#define CREATE_NEW 1U
#define CREATE_ALWAYS 2U
#define OPEN_EXISTING 3U
#define FILE_ATTRIBUTE_NORMAL 0x00000080U
#define INVALID_HANDLE_VALUE ((HANDLE)(INT_PTR)-1)
#define ERROR_FILE_EXISTS 80U
#define ERROR_CLASS_ALREADY_EXISTS 1410U
#define GetFileExInfoStandard 0
#define CP_ACP 0U
#define CP_UTF8 65001U
#define INVALID_FILE_SIZE 0xffffffffU
#define CSIDL_APPDATA 0x001a
#define CSIDL_FLAG_CREATE 0x8000
#define SHGFP_TYPE_CURRENT 0
#define RO_INIT_MULTITHREADED 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_ACCESS_VIOLATION 0xc0000005U
#define CS_VREDRAW 0x0001U
#define CS_HREDRAW 0x0002U
#define CS_OWNDC 0x0020U
#define WS_VISIBLE 0x10000000U
#define WS_POPUP 0x80000000U
#define WS_OVERLAPPED 0x00000000U
#define WS_CAPTION 0x00c00000U
#define WS_SYSMENU 0x00080000U
#define WS_THICKFRAME 0x00040000U
#define WS_MINIMIZEBOX 0x00020000U
#define WS_MAXIMIZEBOX 0x00010000U
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX)
#define CW_USEDEFAULT ((int)0x80000000)
#define PM_REMOVE 0x0001U
#define SIZE_MINIMIZED 1U
#define HTCLIENT 1U
#define WM_SIZE 0x0005U
#define WM_MOVE 0x0003U
#define WM_CLOSE 0x0010U
#define WM_QUIT 0x0012U
#define WM_DESTROY 0x0002U
#define WM_PAINT 0x000fU
#define WM_ERASEBKGND 0x0014U
#define COLOR_WINDOWTEXT 8U
#define WM_SETCURSOR 0x0020U
#define WM_ACTIVATEAPP 0x001cU
#define WM_SETFOCUS 0x0007U
#define WM_KILLFOCUS 0x0008U
#define WM_KEYDOWN 0x0100U
#define WM_KEYUP 0x0101U
#define WM_CHAR 0x0102U
#define WM_SYSKEYDOWN 0x0104U
#define WM_SYSKEYUP 0x0105U
#define WM_UNICHAR 0x0109U
#define WM_MOUSEWHEEL 0x020aU
#define UNICODE_NOCHAR 0xffffU
#define VK_RETURN 0x0DU
#define VK_F4 0x73U
#define VK_T 0x54U
#define WM_WINDOWPOSCHANGED 0x0047U
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define MONITOR_DEFAULTTONEAREST 2U
#define SWP_NOSIZE 0x0001U
#define SWP_NOMOVE 0x0002U
#define SWP_NOZORDER 0x0004U
#define SWP_NOACTIVATE 0x0010U
#define SWP_FRAMECHANGED 0x0020U
#define SWP_SHOWWINDOW 0x0040U
#define SWP_NOOWNERZORDER 0x0200U
#define OFN_OVERWRITEPROMPT 0x00000002U
#define OFN_HIDEREADONLY 0x00000004U
#define OFN_NOCHANGEDIR 0x00000008U
#define OFN_PATHMUSTEXIST 0x00000800U
#define OFN_FILEMUSTEXIST 0x00001000U
#define OFN_EXPLORER 0x00080000U
#define WM_ENTERSIZEMOVE 0x0231U
#define WM_EXITSIZEMOVE 0x0232U
#define WM_MOUSEMOVE 0x0200U
#define WM_LBUTTONDOWN 0x0201U
#define WM_LBUTTONUP 0x0202U
#define WM_RBUTTONDOWN 0x0204U
#define WM_RBUTTONUP 0x0205U
#define WM_MBUTTONDOWN 0x0207U
#define WM_MBUTTONUP 0x0208U
#define SEEK_SET 0
#define SEEK_END 2
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647

#define DXGI_FORMAT_UNKNOWN 0
#define DXGI_FORMAT_R8G8B8A8_UNORM 28
#define DXGI_FORMAT_B8G8R8A8_UNORM 87
#define DXGI_USAGE_RENDER_TARGET_OUTPUT 0x20U
#define DXGI_SWAP_EFFECT_DISCARD 0
#define D3D11_SDK_VERSION 7
#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude *)(ULONG_PTR)1)

typedef struct _GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;
typedef GUID IID;
typedef const GUID *REFIID;

static const GUID IID_IDXGIDevice = {0x54ec77faU,0x1377,0x44e6,{0x8c,0x32,0x88,0xfd,0x5f,0x44,0xc8,0x4c}};
static const GUID IID_IDXGIFactory = {0x7b7166ecU,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};
static const GUID IID_ID3D11Device = {0xdb6f6ddbU,0xac77,0x4e88,{0x82,0x53,0x81,0x9d,0xf9,0xbb,0xf1,0x40}};
static const GUID IID_ID3D11DeviceContext = {0xc0bfa96cU,0xe089,0x44fb,{0x8e,0xaf,0x26,0xf8,0x79,0x61,0x90,0xda}};
static const GUID IID_ID3D11Texture2D = {0x6f15aaf2U,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};

typedef struct tagPOINT { LONG x; LONG y; } POINT;
typedef struct tagRECT { LONG left; LONG top; LONG right; LONG bottom; } RECT;
typedef struct tagWINDOWPLACEMENT {
    UINT length; UINT flags; UINT showCmd; POINT ptMinPosition;
    POINT ptMaxPosition; RECT rcNormalPosition;
} WINDOWPLACEMENT;
typedef struct tagMONITORINFO {
    DWORD cbSize; RECT rcMonitor; RECT rcWork; DWORD dwFlags;
} MONITORINFO;
typedef struct tagOFNW {
    DWORD lStructSize; HWND hwndOwner; HINSTANCE hInstance; LPCWSTR lpstrFilter;
    LPWSTR lpstrCustomFilter; DWORD nMaxCustFilter; DWORD nFilterIndex;
    LPWSTR lpstrFile; DWORD nMaxFile; LPWSTR lpstrFileTitle; DWORD nMaxFileTitle;
    LPCWSTR lpstrInitialDir; LPCWSTR lpstrTitle; DWORD Flags; WORD nFileOffset;
    WORD nFileExtension; LPCWSTR lpstrDefExt; LPARAM lCustData; LPVOID lpfnHook;
    LPCWSTR lpTemplateName; LPVOID pvReserved; DWORD dwReserved; DWORD FlagsEx;
} OPENFILENAMEW;
typedef struct tagPAINTSTRUCT {
    HDC hdc; BOOL fErase; RECT rcPaint; BOOL fRestore; BOOL fIncUpdate; BYTE rgbReserved[32];
} PAINTSTRUCT;
typedef struct tagMSG {
    HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt;
} MSG;
typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef struct tagWNDCLASSEXW {
    UINT cbSize; UINT style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra;
    HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground;
    LPCWSTR lpszMenuName; LPCWSTR lpszClassName; HICON hIconSm;
} WNDCLASSEXW;
typedef struct _SYSTEMTIME {
    WORD wYear,wMonth,wDayOfWeek,wDay,wHour,wMinute,wSecond,wMilliseconds;
} SYSTEMTIME;
typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes; DWORD ftCreationTimeLow,ftCreationTimeHigh;
    DWORD ftLastAccessTimeLow,ftLastAccessTimeHigh;
    DWORD ftLastWriteTimeLow,ftLastWriteTimeHigh;
    DWORD nFileSizeHigh,nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode; DWORD ExceptionFlags; struct _EXCEPTION_RECORD *ExceptionRecord;
    void *ExceptionAddress; DWORD NumberParameters; ULONG_PTR ExceptionInformation[15];
} EXCEPTION_RECORD;
typedef struct _CONTEXT {
    DWORD ContextFlags; DWORD Dr0,Dr1,Dr2,Dr3,Dr6,Dr7;
    BYTE FloatSave[112]; DWORD SegGs,SegFs,SegEs,SegDs;
    DWORD Edi,Esi,Ebx,Edx,Ecx,Eax,Ebp,Eip,SegCs,EFlags,Esp,SegSs;
    BYTE ExtendedRegisters[512];
} CONTEXT;
typedef struct _EXCEPTION_POINTERS { EXCEPTION_RECORD *ExceptionRecord; CONTEXT *ContextRecord; } EXCEPTION_POINTERS;

typedef struct D3D11_RECT { LONG left, top, right, bottom; } D3D11_RECT;
typedef struct D3D_SHADER_MACRO { LPCSTR Name; LPCSTR Definition; } D3D_SHADER_MACRO;
typedef enum D3D_INCLUDE_TYPE { D3D_INCLUDE_LOCAL=0, D3D_INCLUDE_SYSTEM=1, D3D10_INCLUDE_LOCAL=0, D3D10_INCLUDE_SYSTEM=1 } D3D_INCLUDE_TYPE;

typedef struct ID3DInclude ID3DInclude;
typedef struct ID3DIncludeVtbl {
    HRESULT (STDMETHODCALLTYPE *Open)(ID3DInclude*,D3D_INCLUDE_TYPE,LPCSTR,LPCVOID,LPCVOID*,UINT*);
    HRESULT (STDMETHODCALLTYPE *Close)(ID3DInclude*,LPCVOID);
} ID3DIncludeVtbl;
struct ID3DInclude { ID3DIncludeVtbl *lpVtbl; };

typedef struct IUnknownLike { void **lpVtbl; } IUnknownLike;
typedef IUnknownLike IUnknown;
typedef IUnknownLike ID3D11Device;
typedef IUnknownLike ID3D11DeviceContext;
typedef IUnknownLike ID3D11RenderTargetView;
typedef IUnknownLike ID3D11DepthStencilView;
typedef IUnknownLike ID3D11Texture2D;
typedef IUnknownLike IDXGIDevice;
typedef IUnknownLike IDXGIAdapter;
typedef IUnknownLike IDXGIFactory;
typedef IUnknownLike IDXGISwapChain;
typedef IUnknownLike ID3DBlob;
typedef ID3DBlob ID3D10Blob;

typedef struct DXGI_RATIONAL { UINT Numerator; UINT Denominator; } DXGI_RATIONAL;
typedef struct DXGI_MODE_DESC {
    UINT Width; UINT Height; DXGI_RATIONAL RefreshRate; UINT Format; UINT ScanlineOrdering; UINT Scaling;
} DXGI_MODE_DESC;
typedef struct DXGI_SAMPLE_DESC { UINT Count; UINT Quality; } DXGI_SAMPLE_DESC;
typedef struct D3D11_VIEWPORT {
    float TopLeftX;
    float TopLeftY;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;
} D3D11_VIEWPORT;

typedef struct DXGI_SWAP_CHAIN_DESC {
    DXGI_MODE_DESC BufferDesc; DXGI_SAMPLE_DESC SampleDesc; UINT BufferUsage; UINT BufferCount;
    HWND OutputWindow; BOOL Windowed; UINT SwapEffect; UINT Flags;
} DXGI_SWAP_CHAIN_DESC;

#define COMCALL(obj,index,rettype,callconv,...) ((rettype (callconv *)(void*, ##__VA_ARGS__))((obj)->lpVtbl[(index)]))
#define ID3D11Device_QueryInterface(o,i,p) COMCALL((o),0,HRESULT,STDMETHODCALLTYPE,const GUID*,void**)((o),(i),(void**)(p))
#define ID3D11Device_AddRef(o) COMCALL((o),1,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11Device_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11DeviceContext_AddRef(o) COMCALL((o),1,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11DeviceContext_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11RenderTargetView_AddRef(o) COMCALL((o),1,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11RenderTargetView_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGIDevice_GetAdapter(o,p) COMCALL((o),7,HRESULT,STDMETHODCALLTYPE,IDXGIAdapter**)((o),(p))
#define IDXGIDevice_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGIAdapter_GetParent(o,i,p) COMCALL((o),6,HRESULT,STDMETHODCALLTYPE,const GUID*,void**)((o),(i),(void**)(p))
#define IDXGIAdapter_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGIFactory_CreateSwapChain(o,d,desc,p) COMCALL((o),10,HRESULT,STDMETHODCALLTYPE,ID3D11Device*,DXGI_SWAP_CHAIN_DESC*,IDXGISwapChain**)((o),(d),(desc),(p))
#define IDXGIFactory_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGISwapChain_AddRef(o) COMCALL((o),1,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGISwapChain_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define IDXGISwapChain_Present(o,s,f) COMCALL((o),8,HRESULT,STDMETHODCALLTYPE,UINT,UINT)((o),(s),(f))
#define IDXGISwapChain_GetBuffer(o,i,g,p) COMCALL((o),9,HRESULT,STDMETHODCALLTYPE,UINT,const GUID*,void**)((o),(i),(g),(p))
#define IDXGISwapChain_ResizeBuffers(o,c,w,h,fmt,f) COMCALL((o),13,HRESULT,STDMETHODCALLTYPE,UINT,UINT,UINT,UINT,UINT)((o),(c),(w),(h),(fmt),(f))
#define ID3D11Device_CreateRenderTargetView(o,r,d,p) COMCALL((o),9,HRESULT,STDMETHODCALLTYPE,void*,const void*,ID3D11RenderTargetView**)((o),(r),(d),(p))
#define ID3D11Texture2D_Release(o) COMCALL((o),2,ULONG,STDMETHODCALLTYPE)((o))
#define ID3D11DeviceContext_OMSetRenderTargets(o,c,t,d) COMCALL((o),33,void,STDMETHODCALLTYPE,UINT,ID3D11RenderTargetView* const*,void*)((o),(c),(t),(d))
#define ID3D11DeviceContext_RSSetViewports(o,c,v) COMCALL((o),44,void,STDMETHODCALLTYPE,UINT,const D3D11_VIEWPORT*)((o),(c),(v))
#define ID3D11DeviceContext_ClearRenderTargetView(o,t,col) COMCALL((o),50,void,STDMETHODCALLTYPE,ID3D11RenderTargetView*,const float*)((o),(t),(col))
#define ID3D11DeviceContext_Flush(o) COMCALL((o),111,void,STDMETHODCALLTYPE)((o))
#define ID3D10Blob_GetBufferPointer(o) COMCALL((o),3,void*,STDMETHODCALLTYPE)((o))
#define ID3D10Blob_GetBufferSize(o) COMCALL((o),4,SIZE_T,STDMETHODCALLTYPE)((o))

/* C runtime */
extern int memcmp(const void*,const void*,size_t);
extern void *memcpy(void*,const void*,size_t);
extern void *memset(void*,int,size_t);
extern char *strrchr(const char*,int);
extern size_t strlen(const char*);
extern FILE *fopen(const char*,const char*);
extern int fclose(FILE*);
extern int fflush(FILE*);
extern size_t fread(void*,size_t,size_t,FILE*);
extern size_t fwrite(const void*,size_t,size_t,FILE*);
extern int fseek(FILE*,long,int);
extern long ftell(FILE*);
extern int fprintf(FILE*,const char*,...);

/* Kernel32 */
extern HANDLE WINAPI GetCurrentProcess(void);
extern DWORD WINAPI GetCurrentThreadId(void);
extern HANDLE WINAPI GetProcessHeap(void);
extern LPVOID WINAPI HeapAlloc(HANDLE,DWORD,SIZE_T);
extern BOOL WINAPI HeapFree(HANDLE,DWORD,LPVOID);
extern BOOL WINAPI VirtualProtect(LPVOID,SIZE_T,DWORD,DWORD*);
extern LPVOID WINAPI VirtualAlloc(LPVOID,SIZE_T,DWORD,DWORD);
extern BOOL WINAPI VirtualFree(LPVOID,SIZE_T,DWORD);
extern BOOL WINAPI FlushInstructionCache(HANDLE,LPCVOID,SIZE_T);
extern BOOL WINAPI ReadProcessMemory(HANDLE,LPCVOID,LPVOID,SIZE_T,SIZE_T*);
extern HMODULE WINAPI GetModuleHandleW(LPCWSTR);
extern DWORD WINAPI GetEnvironmentVariableA(LPCSTR,LPSTR,DWORD);
extern void WINAPI OutputDebugStringA(LPCSTR);
extern HMODULE WINAPI LoadLibraryW(LPCWSTR);
extern void *WINAPI GetProcAddress(HMODULE,LPCSTR);
extern DWORD WINAPI GetCurrentDirectoryW(DWORD,LPWSTR);
extern BOOL WINAPI CreateDirectoryW(LPCWSTR,LPVOID);
extern HANDLE WINAPI CreateFileW(LPCWSTR,DWORD,DWORD,LPVOID,DWORD,DWORD,HANDLE);
extern BOOL WINAPI ReadFile(HANDLE,LPVOID,DWORD,DWORD*,LPVOID);
extern BOOL WINAPI WriteFile(HANDLE,LPCVOID,DWORD,DWORD*,LPVOID);
extern BOOL WINAPI CloseHandle(HANDLE);
extern DWORD WINAPI GetLastError(void);
extern void WINAPI GetLocalTime(SYSTEMTIME*);
extern BOOL WINAPI GetFileAttributesExW(LPCWSTR,int,WIN32_FILE_ATTRIBUTE_DATA*);
extern DWORD WINAPI GetFileSize(HANDLE,DWORD*);
extern DWORD WINAPI GetTempPathW(DWORD,LPWSTR);
extern UINT WINAPI GetTempFileNameW(LPCWSTR,LPCWSTR,UINT,LPWSTR);
extern BOOL WINAPI DeleteFileW(LPCWSTR);
extern int WINAPI MultiByteToWideChar(UINT,DWORD,LPCSTR,int,LPWSTR,int);
extern int WINAPI WideCharToMultiByte(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,BOOL*);
extern void WINAPI Sleep(DWORD);
typedef struct _LARGE_INTEGER {
    LONGLONG QuadPart;
} LARGE_INTEGER;
typedef struct _MEMORY_BASIC_INFORMATION {
    LPVOID BaseAddress;
    LPVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION;
extern LPCWSTR WINAPI GetCommandLineW(void);
extern SIZE_T WINAPI VirtualQuery(LPCVOID,MEMORY_BASIC_INFORMATION*,SIZE_T);
extern BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *);
extern BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER *);
extern LONG WINAPI InterlockedIncrement(volatile LONG*);
extern LONG WINAPI InterlockedDecrement(volatile LONG*);
extern LONG WINAPI InterlockedExchange(volatile LONG*,LONG);
extern BOOL WINAPI IsBadReadPtr(LPCVOID,UINT_PTR);
extern USHORT WINAPI RtlCaptureStackBackTrace(ULONG,ULONG,void**,ULONG*);
extern int WINAPI lstrlenW(LPCWSTR);
extern LPWSTR WINAPI lstrcpyW(LPWSTR,LPCWSTR);
extern LPWSTR WINAPI lstrcatW(LPWSTR,LPCWSTR);
extern LPWSTR WINAPI lstrcpynW(LPWSTR,LPCWSTR,int);
extern int WINAPI lstrcmpW(LPCWSTR,LPCWSTR);
extern LPSTR WINAPI lstrcpynA(LPSTR,LPCSTR,int);
extern LPSTR WINAPI lstrcatA(LPSTR,LPCSTR);

/* User32 */
extern BOOL WINAPI GetClientRect(HWND,RECT*);
extern LONG WINAPI GetWindowLongW(HWND,int);
extern LONG WINAPI SetWindowLongW(HWND,int,LONG);
extern BOOL WINAPI GetWindowPlacement(HWND,WINDOWPLACEMENT*);
extern BOOL WINAPI SetWindowPlacement(HWND,const WINDOWPLACEMENT*);
extern HMONITOR WINAPI MonitorFromWindow(HWND,DWORD);
extern BOOL WINAPI GetMonitorInfoW(HMONITOR,MONITORINFO*);
extern BOOL WINAPI SetWindowPos(HWND,HWND,int,int,int,int,UINT);
extern BOOL WINAPI SetWindowTextW(HWND,LPCWSTR);
extern BOOL WINAPI ClientToScreen(HWND,POINT*);
extern BOOL WINAPI ClipCursor(const RECT*);
extern BOOL WINAPI SetCursorPos(int,int);
extern HWND WINAPI SetCapture(HWND);
extern BOOL WINAPI ReleaseCapture(void);
extern HWND WINAPI GetCapture(void);
extern int WINAPI ShowCursor(BOOL);
extern HCURSOR WINAPI SetCursor(HCURSOR);
extern HCURSOR WINAPI LoadCursorW(HINSTANCE,LPCWSTR);
extern HWND WINAPI GetForegroundWindow(void);
extern BOOL WINAPI IsIconic(HWND);
extern HWND WINAPI SetFocus(HWND);
extern ATOM WINAPI RegisterClassExW(const WNDCLASSEXW*);
extern HWND WINAPI CreateWindowExW(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID);
extern LRESULT WINAPI DefWindowProcW(HWND,UINT,WPARAM,LPARAM);
extern BOOL WINAPI DestroyWindow(HWND);
extern BOOL WINAPI PostMessageW(HWND,UINT,WPARAM,LPARAM);
extern void WINAPI PostQuitMessage(int);
extern void WINAPI ExitProcess(UINT);
extern BOOL WINAPI OpenClipboard(HWND);
extern BOOL WINAPI EmptyClipboard(void);
extern HANDLE WINAPI GetClipboardData(UINT);
extern HANDLE WINAPI SetClipboardData(UINT,HANDLE);
extern BOOL WINAPI CloseClipboard(void);
extern HANDLE WINAPI GlobalAlloc(UINT,SIZE_T);
extern LPVOID WINAPI GlobalLock(HANDLE);
extern BOOL WINAPI GlobalUnlock(HANDLE);
extern HANDLE WINAPI GlobalFree(HANDLE);
extern BOOL WINAPI PeekMessageW(MSG*,HWND,UINT,UINT,UINT);
extern BOOL WINAPI TranslateMessage(const MSG*);
extern LRESULT WINAPI DispatchMessageW(const MSG*);
extern HDC WINAPI BeginPaint(HWND,PAINTSTRUCT*);
extern BOOL WINAPI EndPaint(HWND,const PAINTSTRUCT*);
extern int WINAPIV wsprintfA(LPSTR,LPCSTR,...);

/* Shell / WinRT */
extern HRESULT WINAPI SHGetFolderPathW(HWND,int,HANDLE,DWORD,LPWSTR);
extern BOOL WINAPI GetOpenFileNameW(OPENFILENAMEW*);
extern BOOL WINAPI GetSaveFileNameW(OPENFILENAMEW*);
extern HRESULT WINAPI RoInitialize(UINT);
extern HRESULT WINAPI WindowsCreateString(LPCWSTR,UINT32,HSTRING*);
extern LPCWSTR WINAPI WindowsGetStringRawBuffer(HSTRING,UINT32*);

#endif
