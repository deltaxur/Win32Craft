#include "win32_compat.h"

#ifndef DXGI_PRESENT_DO_NOT_WAIT
#define DXGI_PRESENT_DO_NOT_WAIT 0x00000008U
#endif
#ifndef DXGI_ERROR_WAS_STILL_DRAWING
#define DXGI_ERROR_WAS_STILL_DRAWING ((HRESULT)0x887A000AL)
#endif

static BOOL host_is_0142;

/* Arguments are the independently matched 0.13.2 and 0.14.2 RVAs. */
#define EARLY_RVA(a, b) (host_is_0142 ? (b) : (a))
#define RVA_EARLY_WFOPEN_S_IAT EARLY_RVA(0x0060f7ec, 0x007868a8)
#define RVA_EARLY_FIOPEN_A_IAT EARLY_RVA(0x0060f0fc, 0x0078644c)
#define RVA_EARLY_FIOPEN_W_IAT EARLY_RVA(0x0060f370, 0x007863b8)
#define RVA_EARLY_THRD_SLEEP_IAT EARLY_RVA(0x0060f30c, 0x00786350)
#define RVA_EARLY_DEVELOPMENT_VERSION_GETTER EARLY_RVA(0x000c3fe0, 0x000e29d0)
#define RVA_EARLY_PLATFORM_OBJECT_CTOR_IAT EARLY_RVA(0x0060f8b0, 0x007869ac)
#define RVA_EARLY_PLATFORM_DELEGATE_CTOR_IAT EARLY_RVA(0x0060f8d8, 0x007869d4)
#define RVA_EARLY_PLATFORM_ALLOCATE2_IAT EARLY_RVA(0x0060f8c0, 0x007869bc)
#define RVA_EARLY_PLATFORM_ALLOCATE1_IAT EARLY_RVA(0x0060f8dc, 0x007869d8)
#define RVA_EARLY_GET_IBOX_ARRAY_VTABLE_IAT EARLY_RVA(0x0060f920, 0x007869e8)
#define RVA_EARLY_GET_IBOX_VTABLE_IAT EARLY_RVA(0x0060f940, 0x00786a3c)
#define RVA_EARLY_OBJECT_DISPOSED_IAT EARLY_RVA(0x0060f944, 0x0078698c)

#define OWNER_SIZE EARLY_RVA(0x1c, 0x8c)
#define OWNER_RENDERER_OFFSET EARLY_RVA(0x18, 0x78)
#define OWNER_DEVICE_OFFSET EARLY_RVA(0x10, 0x84)
#define RENDERER_CONTEXT_OFFSET EARLY_RVA(0x84, 0x100)
#define RENDERER_SWAP_CHAIN_OFFSET EARLY_RVA(0x74, 0xf4)
#define RENDERER_RTV_OFFSET EARLY_RVA(0x88, 0x108)
#define RENDERER_DSV_OFFSET EARLY_RVA(0x90, 0x110)
#define RENDERER_VIEWPORT_OFFSET EARLY_RVA(0x94, 0x118)
#define RENDERER_DISPLAY_OFFSET EARLY_RVA(0x60, 0xd8)
#define PLATFORM_HID_OFFSET EARLY_RVA(0x13c, 0x1bc)
#define PLATFORM_WIDTH_OFFSET EARLY_RVA(0x140, 0x1c0)
#define PLATFORM_HEIGHT_OFFSET EARLY_RVA(0x144, 0x1c4)
#define CLIENT_FRAME_READY_OFFSET EARLY_RVA(0x1d4, 0x1f4)

static FILE *log_file;
static IDXGISwapChain *game_swap_chain;
static ID3D11DeviceContext *game_context;
static ID3D11RenderTargetView *game_target;
static void *game_renderer;
static void *game_resource_owner;
static void *game_app_main;
static void *game_device_resources;
static HWND game_window;
static BOOL mouse_capture_active;
static BOOL host_relative_requested;
static BOOL initial_menu_cursor_guard;
static BOOL mouse_clip_dirty = TRUE;
static BYTE win32_key_down[256];
static WORD pending_high_surrogate;
static BOOL suppress_next_t_character;
static BOOL text_input_active;
static BOOL text_ctrl_down;
static WPARAM clipboard_handled_key;
static BOOL win32_fullscreen_active;
static LONG saved_window_style;
static LONG saved_window_exstyle;
static WINDOWPLACEMENT saved_window_placement;
static wchar_t package_path[MAX_PATH];
static wchar_t game_data_path[MAX_PATH];
static BOOL host_is_windows7;
static volatile LONG game_present_success_count;
static BOOL window_in_size_move;
static UINT pending_resize_width;
static UINT pending_resize_height;

typedef void *(__cdecl *GameMallocFn)(size_t);
typedef void *(__cdecl *GameOperatorNewFn)(size_t);
typedef void *(__attribute__((thiscall)) *GameOwnerCtorFn)(void *);
typedef void (__attribute__((thiscall)) *GameInitDeviceFn)(void *, void *);
typedef void (__attribute__((thiscall)) *GameInitTargetsFn)(void *, void *, void *);
typedef void (__attribute__((thiscall)) *GameReleaseTargetsFn)(void *);
typedef void (__attribute__((thiscall)) *GameSetWindowMetricsFn)(
    void *, const float *, const float *);
typedef void *(__attribute__((thiscall)) *GameCreateDeviceResourcesFn)(void *);
typedef void *(__attribute__((thiscall)) *GameCreateAppMainFn)(void **);
typedef void (__attribute__((thiscall)) *GameUpdateRenderFn)(void *);
typedef void (__attribute__((thiscall)) *GameTickFn)(void *, int, int);
typedef void (__attribute__((thiscall)) *GameApplyRenderStateFn)(
    void *, void *, void *, void *, void *);
typedef void (__attribute__((thiscall)) *GameMaterialDrawFn)(
    void *, void *, void *, void *);
typedef void (__attribute__((thiscall)) *GameResizeFn)(
    void *, int, int, int);
typedef void (__attribute__((thiscall)) *GameLifecycleFn)(void *);
typedef void (__attribute__((thiscall)) *GameEnqueueInputFn)(void *, void *);
typedef int (__attribute__((fastcall)) *GameMapVirtualKeyFn)(void *, int);
typedef int (__cdecl *Game0132WfopenSFn)(FILE **, const wchar_t *, const wchar_t *);
typedef FILE *(__cdecl *Game0132FiopenAFn)(const char *, int, int);
typedef FILE *(__cdecl *Game0132FiopenWFn)(const wchar_t *, int, int);
typedef void (__cdecl *Game0132ThrdSleepFn)(const void *);
typedef void (__cdecl *GamePlatformObjectCtorFn)(void *);
typedef void *(__cdecl *GamePlatformAllocate1Fn)(UINT);
typedef void *(__cdecl *GamePlatformAllocate2Fn)(UINT, UINT);

static GamePlatformAllocate1Fn original_platform_allocate1;
static GamePlatformAllocate2Fn original_platform_allocate2;

BOOL Win32CraftPrepareCopyrightLangW(const wchar_t *path, wchar_t *temporary_path);
BOOL Win32CraftPrepareStartScreenJsonW(const wchar_t *path, wchar_t *temporary_path);
BOOL Win32CraftPrepareStartScreenJsonA(const char *path, char *temporary_path, UINT temporary_capacity);
UINT Win32CraftPresentSyncInterval(void);
static void log_line(const char *text);

static void __cdecl win32_platform_object_ctor(void *object)
{
    (void)object;
}

static void __cdecl win32_ignore_object_disposed(void)
{
    log_line("Minecraft 0.13.2 ignored spurious vccorlib ObjectDisposedException");
}

static void *WINAPI win32_get_ibox_vtable(void *object)
{
    return object ? ((void **)object)[1] : NULL;
}

static void *__cdecl win32_platform_allocate1(UINT size)
{
    void *memory = original_platform_allocate1(size);
    if (memory) ZeroMemory(memory, size);
    return memory;
}

static void *__cdecl win32_platform_allocate2(
    UINT object_size, UINT allocation_size)
{
    void *memory = original_platform_allocate2(object_size, allocation_size);
    if (memory) ZeroMemory(memory, object_size);
    return memory;
}

static BOOL install_vccorlib_compat_0132(BYTE *image)
{
    GamePlatformObjectCtorFn *ctor = (GamePlatformObjectCtorFn *)(
        image + RVA_EARLY_PLATFORM_OBJECT_CTOR_IAT);
    GamePlatformObjectCtorFn *delegate_ctor = (GamePlatformObjectCtorFn *)(
        image + RVA_EARLY_PLATFORM_DELEGATE_CTOR_IAT);
    GamePlatformAllocate2Fn *allocate2 = (GamePlatformAllocate2Fn *)(
        image + RVA_EARLY_PLATFORM_ALLOCATE2_IAT);
    GamePlatformAllocate1Fn *allocate1 = (GamePlatformAllocate1Fn *)(
        image + RVA_EARLY_PLATFORM_ALLOCATE1_IAT);
    void **ibox_array = (void **)(
        image + RVA_EARLY_GET_IBOX_ARRAY_VTABLE_IAT);
    void **ibox = (void **)(image + RVA_EARLY_GET_IBOX_VTABLE_IAT);
    void **object_disposed = (void **)(image + RVA_EARLY_OBJECT_DISPOSED_IAT);
    BYTE *first = image + (host_is_0142 ? 0x0078698c
                                      : RVA_EARLY_PLATFORM_OBJECT_CTOR_IAT);
    SIZE_T length = host_is_0142 ? 0xb4 :
        RVA_EARLY_OBJECT_DISPOSED_IAT -
        RVA_EARLY_PLATFORM_OBJECT_CTOR_IAT + sizeof(void *);
    DWORD old_protection;
    DWORD ignored;

    if (!VirtualProtect(first, length, PAGE_READWRITE, &old_protection)) {
        log_line("Minecraft 0.13.2 vccorlib compatibility protection failed");
        return FALSE;
    }
    original_platform_allocate2 = *allocate2;
    original_platform_allocate1 = *allocate1;
    *ctor = win32_platform_object_ctor;
    *delegate_ctor = win32_platform_object_ctor;
    *allocate2 = win32_platform_allocate2;
    *allocate1 = win32_platform_allocate1;
    *ibox_array = win32_get_ibox_vtable;
    *ibox = win32_get_ibox_vtable;
    *object_disposed = win32_ignore_object_disposed;
    VirtualProtect(first, length, old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), first, length);
    log_line("Minecraft 0.13.2 native vccorlib compatibility installed");
    return TRUE;
}

/*
 * The game text-event factory at image+0x219820 uses a nonstandard x86 ABI:
 *   ECX = output unique_ptr slot
 *   EDX = UTF-8 C string
 *   [ESP+4] = pointer to the trailing flag byte
 * but it returns with plain RET instead of RET 4.  Clang's normal fastcall
 * assumes callee stack cleanup, which leaves ESP shifted and makes the caller
 * read a bogus event pointer.  This naked cdecl bridge performs the exact
 * register setup and explicitly removes the one stack argument afterwards.
 */
__attribute__((naked, noinline))
static void **call_game_make_text_event(
    void *function, void **output, const char *utf8, const BYTE *flag)
{
    __asm__ volatile(
        "pushl 16(%esp)\n\t"
        "movl 8(%esp), %eax\n\t"
        "movl 12(%esp), %ecx\n\t"
        "movl 16(%esp), %edx\n\t"
        "call *%eax\n\t"
        "addl $4, %esp\n\t"
        "retl\n\t");
}
typedef void (STDMETHODCALLTYPE *D3DSetScissorRectsFn)(
    ID3D11DeviceContext *, UINT, const D3D11_RECT *);
typedef HRESULT (WINAPI *GameActivationFn)(
    LPCWSTR, const GUID *, void **);
typedef void (WINAPI *GameCxxThrowFn)(void *, const void *);
typedef HRESULT (WINAPI *GameD3DCompileFn)(
    LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO *, ID3DInclude *,
    LPCSTR, LPCSTR, UINT, UINT, ID3DBlob **, ID3DBlob **);
typedef HRESULT (WINAPI *GameD3D11CreateDeviceFn)(
    void *, UINT, HMODULE, UINT, const UINT *, UINT, UINT,
    ID3D11Device **, UINT *, ID3D11DeviceContext **);

typedef enum FakeKind {
    FAKE_PACKAGE_FACTORY,
    FAKE_APPDATA_FACTORY,
    FAKE_MEMORY_FACTORY,
    FAKE_CURRENTAPP_FACTORY,
    FAKE_COREAPP_FACTORY,
    FAKE_PACKAGE,
    FAKE_APPDATA,
    FAKE_CURRENTAPP,
    FAKE_LICENSE,
    FAKE_PACKAGE_FOLDER,
    FAKE_DATA_FOLDER,
    FAKE_NETWORK_FACTORY
} FakeKind;

typedef struct FakeInspectable {
    const void **vtable;
    LONG references;
    FakeKind kind;
} FakeInspectable;

static GameActivationFn original_activation;
static GameCxxThrowFn original_cxx_throw;
static GameD3DCompileFn original_d3d_compile;
static GameD3D11CreateDeviceFn original_d3d11_create_device;
static Game0132WfopenSFn original_0132_wfopen_s;
static Game0132FiopenAFn original_0132_fiopen_a;
static Game0132FiopenWFn original_0132_fiopen_w;
static Game0132ThrdSleepFn original_0132_thrd_sleep;
static DWORD host_0132_main_thread_id;
/* Windows 7 exposes only the base ID3D11Device/Context interfaces.
 * The original UWP initializer immediately QueryInterfaces the successful
 * D3D11CreateDevice outputs to ID3D11Device2/ID3D11DeviceContext2. Those
 * interfaces begin with Windows 8.1, so both QI calls fail on Windows 7 and
 * the game leaves owner+0x10 and renderer+0x84 null even though creation
 * itself succeeded. Keep one reference to the base interfaces so
 * initialize_game_d3d can transfer them into the renderer as a compatible
 * fallback. All methods used by this 0.13.2 renderer are in the inherited
 * base vtables. */
static ID3D11Device *captured_base_device;
static ID3D11DeviceContext *captured_base_context;
static GameTickFn original_game_tick;
static GameApplyRenderStateFn original_apply_render_state;
static GameMaterialDrawFn original_material_draw;
static D3DSetScissorRectsFn original_set_scissor_rects;
typedef HRESULT (STDMETHODCALLTYPE *SwapChainPresentFn)(IDXGISwapChain *, UINT, UINT);
static SwapChainPresentFn original_swap_chain_present;
static FakeInspectable package_factory;
static FakeInspectable appdata_factory;
static FakeInspectable memory_factory;
static FakeInspectable currentapp_factory;
static FakeInspectable coreapp_factory;
static FakeInspectable package_object;
static FakeInspectable appdata_object;
static FakeInspectable currentapp_object;
static FakeInspectable license_object;
static FakeInspectable package_folder;
static FakeInspectable data_folder;
static FakeInspectable network_factory;
static BYTE *game_image;
static int fake_fmod_system;
static int fake_fmod_group;
static int fake_fmod_sound;
static int fake_fmod_channel;
static char shader_include_directory[MAX_PATH * 2];


static int __cdecl win32_0132_wfopen_s(FILE **output,
                                       const wchar_t *path,
                                       const wchar_t *mode)
{
    wchar_t temporary[MAX_PATH];
    int result;
    if (original_0132_wfopen_s && mode && mode[0] == L'r') {
        if (Win32CraftPrepareCopyrightLangW(path, temporary)) {
            result = original_0132_wfopen_s(output, temporary, mode);
            if (!result && output && *output) return result;
        }
        if (Win32CraftPrepareStartScreenJsonW(path, temporary)) {
            result = original_0132_wfopen_s(output, temporary, mode);
            if (!result && output && *output) return result;
        }
    }
    return original_0132_wfopen_s
        ? original_0132_wfopen_s(output, path, mode) : 22;
}

static FILE *__cdecl win32_0132_fiopen_a(const char *path,
                                           int open_mode, int protection)
{
    char temporary[MAX_PATH * 3];
    if (original_0132_fiopen_a &&
        (Win32CraftPrepareStartScreenJsonA(
             path, temporary, sizeof(temporary)))) {
        FILE *stream = original_0132_fiopen_a(
            temporary, open_mode, protection);
        if (stream) return stream;
    }
    return original_0132_fiopen_a
        ? original_0132_fiopen_a(path, open_mode, protection) : NULL;
}

static FILE *__cdecl win32_0132_fiopen_w(const wchar_t *path,
                                           int open_mode, int protection)
{
    wchar_t temporary[MAX_PATH];
    if (original_0132_fiopen_w &&
        (Win32CraftPrepareStartScreenJsonW(path, temporary))) {
        FILE *stream = original_0132_fiopen_w(
            temporary, open_mode, protection);
        if (stream) return stream;
    }
    return original_0132_fiopen_w
        ? original_0132_fiopen_w(path, open_mode, protection) : NULL;
}

static BOOL install_0132_stdio_hook(void **slot, void *replacement,
                                    void **original);

static void __cdecl win32_0132_thrd_sleep(const void *deadline)
{
    /* Preserve worker timing, but remove the legacy main/render-thread
     * frame pacer. This is the v19 behavior that produced 3000-5000 FPS.
     * It does not touch the external FLS API-set shim or FlsSetValue. */
    if (GetCurrentThreadId() == host_0132_main_thread_id) {
        Sleep(0);
        return;
    }
    if (original_0132_thrd_sleep)
        original_0132_thrd_sleep(deadline);
}

static BOOL install_0132_frame_pacing_hook(BYTE *image)
{
    BOOL installed = install_0132_stdio_hook(
        (void **)(image + RVA_EARLY_THRD_SLEEP_IAT),
        win32_0132_thrd_sleep, (void **)&original_0132_thrd_sleep);
    log_line(installed
        ? "Minecraft 0.13.2 main-thread MSVCP frame sleep disabled"
        : "Minecraft 0.13.2 frame-sleep hook failed");
    return installed;
}

static BOOL install_0132_stdio_hook(void **slot, void *replacement,
                                    void **original)
{
    DWORD old_protection;
    DWORD ignored;
    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection))
        return FALSE;
    *original = *slot;
    *slot = replacement;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    return TRUE;
}

static BOOL install_0132_copyright_override(BYTE *image)
{
    BOOL wide_stdio;
    BOOL narrow_fiopen;
    BOOL wide_fiopen;

    wide_stdio = install_0132_stdio_hook(
        (void **)(image + RVA_EARLY_WFOPEN_S_IAT),
        win32_0132_wfopen_s, (void **)&original_0132_wfopen_s);
    narrow_fiopen = install_0132_stdio_hook(
        (void **)(image + RVA_EARLY_FIOPEN_A_IAT),
        win32_0132_fiopen_a, (void **)&original_0132_fiopen_a);
    wide_fiopen = install_0132_stdio_hook(
        (void **)(image + RVA_EARLY_FIOPEN_W_IAT),
        win32_0132_fiopen_w, (void **)&original_0132_fiopen_w);
    log_line((wide_stdio && narrow_fiopen && wide_fiopen)
        ? "Minecraft 0.13.2 legacy title/copyright file overrides installed (correct _Fiopen ABI)"
        : "Minecraft 0.13.2 legacy title/copyright file overrides incomplete");
    return wide_stdio && narrow_fiopen && wide_fiopen;
}

static HRESULT STDMETHODCALLTYPE win32_include_open(
    ID3DInclude *self, D3D_INCLUDE_TYPE include_type, const char *filename,
    const void *parent_data, const void **data, UINT *bytes)
{
    char path[MAX_PATH * 2];
    FILE *file;
    long size;
    void *buffer;

    (void)self;
    (void)include_type;
    (void)parent_data;
    if (!filename || !data || !bytes) return E_INVALIDARG;
    if (shader_include_directory[0]) {
        lstrcpynA(path, shader_include_directory, sizeof(path));
        lstrcatA(path, filename);
    } else {
        lstrcpynA(path, filename, sizeof(path));
    }
    file = fopen(path, "rb");
    if (!file) return E_FAIL;
    if (fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return E_FAIL;
    }
    buffer = HeapAlloc(GetProcessHeap(), 0, (SIZE_T)size + 1);
    if (!buffer) {
        fclose(file);
        return E_OUTOFMEMORY;
    }
    if (size && fread(buffer, 1, (size_t)size, file) != (size_t)size) {
        HeapFree(GetProcessHeap(), 0, buffer);
        fclose(file);
        return E_FAIL;
    }
    fclose(file);
    ((BYTE *)buffer)[size] = 0;
    *data = buffer;
    *bytes = (UINT)size;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE win32_include_close(
    ID3DInclude *self, const void *data)
{
    (void)self;
    if (data) HeapFree(GetProcessHeap(), 0, (void *)data);
    return S_OK;
}

static ID3DIncludeVtbl win32_include_vtable = {
    win32_include_open,
    win32_include_close
};
static ID3DInclude win32_include = { &win32_include_vtable };


static void STDMETHODCALLTYPE win32_set_scissor_rects(
    ID3D11DeviceContext *context, UINT count, const D3D11_RECT *rects)
{
    D3D11_RECT adjusted[16];
    const D3D11_RECT *submitted = rects;
    static unsigned logged_corrections;
    BOOL corrected = FALSE;
    RECT client;
    UINT index;

    /*
     * The WinRT renderer's Y conversion retains one UWP CoreWindow-height
     * offset.  Its scroll panes consequently submit e.g. -599..-47 for a
     * 688-pixel HWND, clipping every Options row.  Translate that wholly
     * negative interval back into HWND client coordinates and clamp the
     * oversized UWP bounds to the real swap-chain extent.
     */
    if (rects && count && count <= ARRAYSIZE(adjusted) && game_window &&
        GetClientRect(game_window, &client)) {
        CopyMemory(adjusted, rects, count * sizeof(adjusted[0]));
        for (index = 0; index < count; ++index) {
            if (adjusted[index].top < 0 && adjusted[index].bottom <= 0) {
                adjusted[index].top += client.bottom;
                adjusted[index].bottom += client.bottom;
                corrected = TRUE;
            }
            if (adjusted[index].left < 0) {
                adjusted[index].left = 0;
                corrected = TRUE;
            }
            if (adjusted[index].top < 0) {
                adjusted[index].top = 0;
                corrected = TRUE;
            }
            if (adjusted[index].right > client.right) {
                adjusted[index].right = client.right;
                corrected = TRUE;
            }
            if (adjusted[index].bottom > client.bottom) {
                adjusted[index].bottom = client.bottom;
                corrected = TRUE;
            }
        }
        submitted = adjusted;
    }
    if (corrected && logged_corrections < 8) {
        log_line("corrected WinRT scissor rectangle for HWND");
        ++logged_corrections;
    }
    original_set_scissor_rects(context, count, submitted);
}

static BOOL install_scissor_fix(ID3D11DeviceContext *context)
{
    D3DSetScissorRectsFn *slot;
    DWORD old_protection;
    DWORD ignored;

    if (!context) return FALSE;
    slot = (D3DSetScissorRectsFn *)&context->lpVtbl[45];
    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE,
                        &old_protection)) {
        return FALSE;
    }
    original_set_scissor_rects = *slot;
    *slot = win32_set_scissor_rects;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("installed D3D11 HWND scissor fix");
    return TRUE;
}

static int WINAPI fake_fmod_system_create(void **result)
{
    log_line("FMOD shim: System_Create");
    if (result) *result = &fake_fmod_system;
    return 0;
}

static int WINAPI fake_fmod_get_version(
    void *self, unsigned *version)
{
    (void)self;
    log_line("FMOD shim: getVersion");
    if (version) *version = 0xffffffffU;
    return 0;
}

static int WINAPI fake_fmod_init(
    void *self, int channels, unsigned flags, void *extra)
{
    (void)self; (void)channels; (void)flags; (void)extra;
    log_line("FMOD shim: init");
    return 0;
}

static int WINAPI fake_fmod_three_floats(
    void *self, float a, float b, float c)
{
    (void)self; (void)a; (void)b; (void)c;
    log_line("FMOD shim: set3DSettings");
    return 0;
}

static int WINAPI fake_fmod_create_group(
    void *self, const char *name, void **result)
{
    (void)self; (void)name;
    log_line("FMOD shim: createChannelGroup");
    if (result) *result = &fake_fmod_group;
    return 0;
}

static int WINAPI fake_fmod_get_group(
    void *self, void **result)
{
    (void)self;
    log_line("FMOD shim: getMasterChannelGroup");
    if (result) *result = &fake_fmod_group;
    return 0;
}

static int WINAPI fake_fmod_add_group(
    void *self, void *group, int propagate, void **connection)
{
    (void)self; (void)group; (void)propagate;
    if (connection) *connection = NULL;
    return 0;
}

static int WINAPI fake_fmod_no_args(void *self)
{
    (void)self;
    return 0;
}

static int WINAPI fake_fmod_bool(
    void *self, int value)
{
    (void)self; (void)value;
    return 0;
}

static int WINAPI fake_fmod_float(
    void *self, float value)
{
    (void)self; (void)value;
    return 0;
}

static int WINAPI fake_fmod_create_sound(
    void *self, const char *name, unsigned mode, void *info, void **result)
{
    (void)self; (void)name; (void)mode; (void)info;
    if (result) *result = &fake_fmod_sound;
    return 0;
}

static int WINAPI fake_fmod_two_floats(
    void *self, float a, float b)
{
    (void)self; (void)a; (void)b;
    return 0;
}

static int WINAPI fake_fmod_get_count(
    void *self, int *result)
{
    (void)self;
    if (result) *result = 0;
    return 0;
}

static int WINAPI fake_fmod_get_subsound(
    void *self, int index, void **result)
{
    (void)self; (void)index;
    if (result) *result = &fake_fmod_sound;
    return 0;
}

static int WINAPI fake_fmod_play_sound(
    void *self, void *sound, void *group, int paused, void **result)
{
    (void)self; (void)sound; (void)group; (void)paused;
    if (result) *result = &fake_fmod_channel;
    return 0;
}

static int WINAPI fake_fmod_vectors(
    void *self, const void *a, const void *b, const void *c)
{
    (void)self; (void)a; (void)b; (void)c;
    return 0;
}

static int WINAPI fake_fmod_is_playing(
    void *self, BYTE *result)
{
    (void)self;
    if (result) *result = FALSE;
    return 0;
}

static int WINAPI fake_fmod_listener(
    void *self, int listener, const void *position, const void *velocity,
    const void *forward, const void *up)
{
    (void)self; (void)listener; (void)position; (void)velocity;
    (void)forward; (void)up;
    return 0;
}


/*
 * Native Win32 FMOD bridge.
 *
 * The game delay-imports the Store build name fmod_WSA82_X86.dll.  Patching
 * the 25 delay thunks to these wrappers bypasses the UWP loader and forwards
 * them to the supplied desktop x86 build renamed to fmod.dll.
 * The bridge logs all initialization and sound-loading failures.  It defaults
 * to the native desktop DLL.  v9 patches the delay-IAT slots themselves,
 * avoiding rel32 range/layout dependence.  WASAPI is the default because a
 * Wine WINMM device can initialize successfully yet remain inaudible.
 * fmod_output.txt accepts "auto", "wasapi", "dsound", or "winmm".
 */
typedef int (WINAPI *FmodSystemCreateFn)(void **);
typedef int (WINAPI *FmodGetVersionFn)(void *, unsigned *);
typedef int (WINAPI *FmodInitFn)(void *, int, unsigned, void *);
typedef int (WINAPI *FmodThreeFloatsFn)(void *, float, float, float);
typedef int (WINAPI *FmodCreateGroupFn)(void *, const char *, void **);
typedef int (WINAPI *FmodGetGroupFn)(void *, void **);
typedef int (WINAPI *FmodAddGroupFn)(void *, void *, int, void **);
typedef int (WINAPI *FmodNoArgsFn)(void *);
typedef int (WINAPI *FmodBoolFn)(void *, int);
typedef int (WINAPI *FmodFloatFn)(void *, float);
typedef int (WINAPI *FmodCreateSoundFn)(void *, const char *, unsigned, void *, void **);
typedef int (WINAPI *FmodTwoFloatsFn)(void *, float, float);
typedef int (WINAPI *FmodGetCountFn)(void *, int *);
typedef int (WINAPI *FmodGetSubSoundFn)(void *, int, void **);
typedef int (WINAPI *FmodPlaySoundFn)(void *, void *, void *, int, void **);
typedef int (WINAPI *FmodVectorsFn)(void *, const void *, const void *, const void *);
typedef int (WINAPI *FmodIsPlayingFn)(void *, BYTE *);
typedef int (WINAPI *FmodListenerFn)(void *, int, const void *, const void *, const void *, const void *);
typedef int (WINAPI *FmodSetOutputFn)(void *, int);
typedef int (WINAPI *FmodGetOutputFn)(void *, int *);
typedef int (WINAPI *FmodSetDriverFn)(void *, int);
typedef int (WINAPI *FmodGetDriverFn)(void *, int *);
typedef const char *(WINAPI *FmodErrorStringFn)(int);
typedef int (WINAPI *FmodGetNumDriversFn)(void *, int *);
typedef int (WINAPI *FmodGetDriverInfoFn)(void *, int, char *, int, void *, int *, int *, int *);

static HMODULE fmod_real_module;
static BOOL fmod_real_attempted;
static BOOL fmod_real_ready;
static int fmod_output_preference = 8; /* FMOD_OUTPUTTYPE_WASAPI */
static BOOL fmod_output_applied;
static unsigned fmod_create_log_count;
static unsigned fmod_play_log_count;
static unsigned fmod_update_counter;
static void *fmod_master_group;
static void *fmod_sound_group;
static void *fmod_music_group;

static FmodSystemCreateFn real_fmod_system_create;
static FmodGetVersionFn real_fmod_get_version;
static FmodInitFn real_fmod_init;
static FmodThreeFloatsFn real_fmod_set_3d_settings;
static FmodCreateGroupFn real_fmod_create_group;
static FmodGetGroupFn real_fmod_get_group;
static FmodAddGroupFn real_fmod_add_group;
static FmodNoArgsFn real_fmod_sound_release;
static FmodNoArgsFn real_fmod_system_close;
static FmodNoArgsFn real_fmod_system_release;
static FmodBoolFn real_fmod_set_mute;
static FmodFloatFn real_fmod_set_volume;
static FmodCreateSoundFn real_fmod_create_stream;
static FmodCreateSoundFn real_fmod_create_sound;
static FmodTwoFloatsFn real_fmod_set_min_max;
static FmodGetCountFn real_fmod_get_sub_count;
static FmodGetSubSoundFn real_fmod_get_sub_sound;
static FmodPlaySoundFn real_fmod_play_sound;
static FmodVectorsFn real_fmod_set_3d_attributes;
static FmodFloatFn real_fmod_set_pitch;
static FmodBoolFn real_fmod_set_paused;
static FmodIsPlayingFn real_fmod_is_playing;
static FmodNoArgsFn real_fmod_stop;
static FmodNoArgsFn real_fmod_update;
static FmodListenerFn real_fmod_set_listener;
static FmodSetOutputFn real_fmod_set_output;
static FmodGetOutputFn real_fmod_get_output;
static FmodSetDriverFn real_fmod_set_driver;
static FmodGetDriverFn real_fmod_get_driver;
static FmodErrorStringFn real_fmod_error_string;
static FmodGetNumDriversFn real_fmod_get_num_drivers;
static FmodGetDriverInfoFn real_fmod_get_driver_info;

static BOOL ascii_prefix(const char *text, const char *prefix)
{
    while (*prefix) {
        char a = *text++;
        char b = *prefix++;
        if (a >= 'A' && a <= 'Z') a = (char)(a + ('a' - 'A'));
        if (b >= 'A' && b <= 'Z') b = (char)(b + ('a' - 'A'));
        if (a != b) return FALSE;
    }
    return TRUE;
}

static int read_fmod_output_preference(void)
{
    FILE *file = fopen("fmod_output.txt", "rb");
    char value[32];
    size_t count;
    if (!file) return 8;
    count = fread(value, 1, sizeof(value) - 1, file);
    fclose(file);
    value[count] = 0;
    if (ascii_prefix(value, "auto")) return 0;
    if (ascii_prefix(value, "wasapi")) return 8;
    if (ascii_prefix(value, "dsound")) return 6;
    if (ascii_prefix(value, "winmm")) return 7;
    return 8;
}

static const char *fmod_error_text(int result)
{
    const char *text = NULL;
    if (real_fmod_error_string) text = real_fmod_error_string(result);
    return text ? text : "unknown";
}

static void log_fmod_call(const char *name, int result)
{
    char line[512];
    wsprintfA(line, "FMOD bridge: %s -> %d (%s)",
              name, result, fmod_error_text(result));
    log_line(line);
}

static void *resolve_fmod(const char *name)
{
    void *result = GetProcAddress(fmod_real_module, name);
    if (!result) {
        char line[512];
        wsprintfA(line, "FMOD bridge: missing export %s", name);
        log_line(line);
    }
    return result;
}

static BOOL load_real_fmod(void)
{
    if (fmod_real_attempted) return fmod_real_ready;
    fmod_real_attempted = TRUE;
    fmod_output_preference = read_fmod_output_preference();
    fmod_real_module = LoadLibraryW(L"fmod.dll");
    if (!fmod_real_module) {
        char line[256];
        wsprintfA(line, "FMOD bridge: LoadLibraryW(fmod.dll) failed, error=%u",
                  (unsigned)GetLastError());
        log_line(line);
        return FALSE;
    }

#define RESOLVE_REQUIRED(field, type, symbol) \
    field = (type)resolve_fmod(symbol); if (!field) return FALSE
    RESOLVE_REQUIRED(real_fmod_system_create, FmodSystemCreateFn, "FMOD_System_Create");
    RESOLVE_REQUIRED(real_fmod_get_version, FmodGetVersionFn, "?getVersion@System@FMOD@@QAG?AW4FMOD_RESULT@@PAI@Z");
    RESOLVE_REQUIRED(real_fmod_init, FmodInitFn, "?init@System@FMOD@@QAG?AW4FMOD_RESULT@@HIPAX@Z");
    RESOLVE_REQUIRED(real_fmod_set_3d_settings, FmodThreeFloatsFn, "?set3DSettings@System@FMOD@@QAG?AW4FMOD_RESULT@@MMM@Z");
    RESOLVE_REQUIRED(real_fmod_create_group, FmodCreateGroupFn, "?createChannelGroup@System@FMOD@@QAG?AW4FMOD_RESULT@@PBDPAPAVChannelGroup@2@@Z");
    RESOLVE_REQUIRED(real_fmod_get_group, FmodGetGroupFn, "?getMasterChannelGroup@System@FMOD@@QAG?AW4FMOD_RESULT@@PAPAVChannelGroup@2@@Z");
    RESOLVE_REQUIRED(real_fmod_add_group, FmodAddGroupFn, "?addGroup@ChannelGroup@FMOD@@QAG?AW4FMOD_RESULT@@PAV12@_NPAPAVDSPConnection@2@@Z");
    RESOLVE_REQUIRED(real_fmod_sound_release, FmodNoArgsFn, "?release@Sound@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    RESOLVE_REQUIRED(real_fmod_system_close, FmodNoArgsFn, "?close@System@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    RESOLVE_REQUIRED(real_fmod_system_release, FmodNoArgsFn, "?release@System@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    RESOLVE_REQUIRED(real_fmod_set_mute, FmodBoolFn, "?setMute@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@_N@Z");
    RESOLVE_REQUIRED(real_fmod_set_volume, FmodFloatFn, "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
    RESOLVE_REQUIRED(real_fmod_create_stream, FmodCreateSoundFn, "?createStream@System@FMOD@@QAG?AW4FMOD_RESULT@@PBDIPAUFMOD_CREATESOUNDEXINFO@@PAPAVSound@2@@Z");
    RESOLVE_REQUIRED(real_fmod_create_sound, FmodCreateSoundFn, "?createSound@System@FMOD@@QAG?AW4FMOD_RESULT@@PBDIPAUFMOD_CREATESOUNDEXINFO@@PAPAVSound@2@@Z");
    RESOLVE_REQUIRED(real_fmod_set_min_max, FmodTwoFloatsFn, "?set3DMinMaxDistance@Sound@FMOD@@QAG?AW4FMOD_RESULT@@MM@Z");
    RESOLVE_REQUIRED(real_fmod_get_sub_count, FmodGetCountFn, "?getNumSubSounds@Sound@FMOD@@QAG?AW4FMOD_RESULT@@PAH@Z");
    RESOLVE_REQUIRED(real_fmod_get_sub_sound, FmodGetSubSoundFn, "?getSubSound@Sound@FMOD@@QAG?AW4FMOD_RESULT@@HPAPAV12@@Z");
    RESOLVE_REQUIRED(real_fmod_play_sound, FmodPlaySoundFn, "?playSound@System@FMOD@@QAG?AW4FMOD_RESULT@@PAVSound@2@PAVChannelGroup@2@_NPAPAVChannel@2@@Z");
    RESOLVE_REQUIRED(real_fmod_set_3d_attributes, FmodVectorsFn, "?set3DAttributes@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@PBUFMOD_VECTOR@@00@Z");
    RESOLVE_REQUIRED(real_fmod_set_pitch, FmodFloatFn, "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
    RESOLVE_REQUIRED(real_fmod_set_paused, FmodBoolFn, "?setPaused@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@_N@Z");
    RESOLVE_REQUIRED(real_fmod_is_playing, FmodIsPlayingFn, "?isPlaying@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@PA_N@Z");
    RESOLVE_REQUIRED(real_fmod_stop, FmodNoArgsFn, "?stop@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    RESOLVE_REQUIRED(real_fmod_update, FmodNoArgsFn, "?update@System@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    RESOLVE_REQUIRED(real_fmod_set_listener, FmodListenerFn, "?set3DListenerAttributes@System@FMOD@@QAG?AW4FMOD_RESULT@@HPBUFMOD_VECTOR@@000@Z");
#undef RESOLVE_REQUIRED

    real_fmod_set_output = (FmodSetOutputFn)resolve_fmod("?setOutput@System@FMOD@@QAG?AW4FMOD_RESULT@@W4FMOD_OUTPUTTYPE@@@Z");
    real_fmod_get_output = (FmodGetOutputFn)resolve_fmod("?getOutput@System@FMOD@@QAG?AW4FMOD_RESULT@@PAW4FMOD_OUTPUTTYPE@@@Z");
    real_fmod_set_driver = (FmodSetDriverFn)resolve_fmod("?setDriver@System@FMOD@@QAG?AW4FMOD_RESULT@@H@Z");
    real_fmod_get_driver = (FmodGetDriverFn)resolve_fmod("?getDriver@System@FMOD@@QAG?AW4FMOD_RESULT@@PAH@Z");
    real_fmod_error_string = (FmodErrorStringFn)resolve_fmod("FMOD_ErrorString");
    real_fmod_get_num_drivers = (FmodGetNumDriversFn)resolve_fmod("?getNumDrivers@System@FMOD@@QAG?AW4FMOD_RESULT@@PAH@Z");
    real_fmod_get_driver_info = (FmodGetDriverInfoFn)resolve_fmod("?getDriverInfo@System@FMOD@@QAG?AW4FMOD_RESULT@@HPADHPAUFMOD_GUID@@PAHPAW4FMOD_SPEAKERMODE@@2@Z");
    fmod_real_ready = TRUE;
    {
        char line[128];
        wsprintfA(line, "FMOD bridge: desktop DLL loaded, requested output=%d", fmod_output_preference);
        log_line(line);
    }
    return TRUE;
}

static const char *normalize_fmod_path(const char *input, char *output, int output_size)
{
    const char *source = input;
    int index = 0;
    if (!input || !output || output_size < 2) return input;
    if (ascii_prefix(source, "ms-appx:///")) source += 11;
    else if (ascii_prefix(source, "file:///")) source += 8;
    while (*source && index < output_size - 1) {
        char value = *source++;
        if (value == '/') value = '\\';
        output[index++] = value;
    }
    output[index] = 0;
    return output;
}

static void log_fmod_sound_result(const char *kind, const char *path,
                                  unsigned mode, int result, void *sound)
{
    char line[768];
    char clipped[420];
    lstrcpynA(clipped, path ? path : "(null)", sizeof(clipped));
    wsprintfA(line, "FMOD bridge: %s result=%d mode=0x%x sound=%p path=%s",
              kind, result, mode, sound, clipped);
    log_line(line);
}

static void log_fmod_driver_state(void *system)
{
    int output = -1;
    int count = -1;
    int result;
    char line[512];
    if (real_fmod_get_output) {
        result = real_fmod_get_output(system, &output);
        wsprintfA(line, "FMOD bridge: getOutput result=%d output=%d", result, output);
        log_line(line);
    }
    if (real_fmod_get_num_drivers) {
        result = real_fmod_get_num_drivers(system, &count);
        wsprintfA(line, "FMOD bridge: getNumDrivers result=%d count=%d", result, count);
        log_line(line);
        if (!result && count > 0 && real_fmod_get_driver_info) {
            int index;
            if (count > 8) count = 8;
            for (index = 0; index < count; ++index) {
                char name[256];
                BYTE guid[16];
                int rate = 0, speaker = 0, channels = 0;
                ZeroMemory(name, sizeof(name));
                ZeroMemory(guid, sizeof(guid));
                result = real_fmod_get_driver_info(system, index, name,
                    sizeof(name), guid, &rate, &speaker, &channels);
                wsprintfA(line,
                    "FMOD bridge: driver[%d] result=%d name=%s rate=%d speaker=%d channels=%d",
                    index, result, name, rate, speaker, channels);
                log_line(line);
            }
        }
    }
}

static int WINAPI bridge_fmod_system_create(void **result)
{
    int code;
    if (!load_real_fmod()) return fake_fmod_system_create(result);
    code = real_fmod_system_create(result);
    {
        char line[192];
        wsprintfA(line, "FMOD bridge: System_Create result=%d system=%p",
                  code, result ? *result : NULL);
        log_line(line);
    }
    if (code || !result || !*result) {
        log_line("FMOD bridge: System_Create failed; using silent fallback");
        return fake_fmod_system_create(result);
    }
    return code;
}

static int WINAPI bridge_fmod_get_version(void *self, unsigned *version)
{
    int code;
    if (self == &fake_fmod_system || !load_real_fmod()) return fake_fmod_get_version(self, version);
    code = real_fmod_get_version(self, version);
    {
        char line[192];
        wsprintfA(line, "FMOD bridge: getVersion result=%d version=0x%x",
                  code, version ? *version : 0);
        log_line(line);
    }
    return code;
}

static const char *fmod_output_name(int output)
{
    if (output == 0) return "AUTODETECT";
    if (output == 6) return "DSOUND";
    if (output == 7) return "WINMM";
    if (output == 8) return "WASAPI";
    return "OTHER";
}

static int try_fmod_init_output(void *self, int output, int channels,
                                unsigned flags, void *extra)
{
    int code;
    int drivers = -1;
    char label[96];
    if (output && real_fmod_set_output) {
        code = real_fmod_set_output(self, output);
        wsprintfA(label, "setOutput(%s)", fmod_output_name(output));
        log_fmod_call(label, code);
        if (code) return code;
    }
    if (real_fmod_get_num_drivers) {
        code = real_fmod_get_num_drivers(self, &drivers);
        {
            char line[160];
            wsprintfA(line, "FMOD bridge: pre-init drivers output=%s result=%d count=%d",
                      fmod_output_name(output), code, drivers);
            log_line(line);
        }
        if (!code && drivers > 0 && real_fmod_set_driver) {
            code = real_fmod_set_driver(self, 0);
            log_fmod_call("setDriver(0)", code);
        }
    }
    code = real_fmod_init(self, channels, flags, extra);
    {
        char line[320];
        wsprintfA(line, "FMOD bridge: init output=%s channels=%d flags=0x%x result=%d (%s)",
                  fmod_output_name(output), channels, flags, code,
                  fmod_error_text(code));
        log_line(line);
    }
    return code;
}

static int WINAPI bridge_fmod_init(void *self, int channels, unsigned flags, void *extra)
{
    int code;
    int sequence[4];
    int count = 0;
    int index;
    if (self == &fake_fmod_system || !load_real_fmod()) return fake_fmod_init(self, channels, flags, extra);

    /* Prefer the requested backend, then try the other desktop backends. */
    sequence[count++] = fmod_output_preference;
    if (fmod_output_preference != 8) sequence[count++] = 8;
    if (fmod_output_preference != 6) sequence[count++] = 6;
    if (fmod_output_preference != 7 && count < 4) sequence[count++] = 7;

    code = 1;
    for (index = 0; index < count; ++index) {
        code = try_fmod_init_output(self, sequence[index], channels, flags, extra);
        if (!code) break;
    }
    fmod_output_applied = TRUE;
    if (!code) log_fmod_driver_state(self);
    return code;
}

static void force_fmod_group_audible(void *group, const char *reason);

static int WINAPI bridge_fmod_set_3d_settings(void *s,float a,float b,float c)
{ int r; if (s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_three_floats(s,a,b,c); r=real_fmod_set_3d_settings(s,a,b,c); if(r) log_fmod_call("set3DSettings",r); return r; }
static int WINAPI bridge_fmod_create_group(void *s,const char *n,void **o)
{
    int r;
    if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_create_group(s,n,o);
    r=real_fmod_create_group(s,n,o);
    if(r) log_fmod_call("createChannelGroup",r);
    if (!r && o && *o) {
        if (n && ascii_prefix(n, "Sound")) fmod_sound_group = *o;
        if (n && ascii_prefix(n, "Music")) fmod_music_group = *o;
        force_fmod_group_audible(*o, n ? n : "channel group");
    }
    return r;
}
static void force_fmod_group_audible(void *group, const char *reason)
{
    int mute_result = 0;
    int volume_result = 0;
    char line[256];
    if (!group || !load_real_fmod()) return;
    if (real_fmod_set_mute) mute_result = real_fmod_set_mute(group, FALSE);
    if (real_fmod_set_volume) volume_result = real_fmod_set_volume(group, 1.0f);
    wsprintfA(line, "FMOD bridge: force audible (%s) group=%p mute=%d volume=%d",
              reason, group, mute_result, volume_result);
    log_line(line);
}

static int WINAPI bridge_fmod_get_group(void *s,void **o)
{
    int r;
    if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_get_group(s,o);
    r=real_fmod_get_group(s,o);
    if(r) log_fmod_call("getMasterChannelGroup",r);
    if (!r && o && *o) {
        fmod_master_group = *o;
        force_fmod_group_audible(fmod_master_group, "master acquired");
    }
    return r;
}
static int WINAPI bridge_fmod_add_group(void *s,void *g,int p,void **c)
{ int r; if(s==&fake_fmod_group||!load_real_fmod()) return fake_fmod_add_group(s,g,p,c); r=real_fmod_add_group(s,g,p,c); if(r) log_fmod_call("addGroup",r); return r; }
static int WINAPI bridge_fmod_sound_release(void *s)
{ if(s==&fake_fmod_sound||!load_real_fmod()) return fake_fmod_no_args(s); return real_fmod_sound_release(s); }
static int WINAPI bridge_fmod_system_close(void *s)
{ if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_no_args(s); return real_fmod_system_close(s); }
static int WINAPI bridge_fmod_system_release(void *s)
{ if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_no_args(s); return real_fmod_system_release(s); }
static int WINAPI bridge_fmod_mixer_resume(void *s)
{
    FmodNoArgsFn function;
    if (s == &fake_fmod_system || !load_real_fmod()) return 0;
    function = (FmodNoArgsFn)GetProcAddress(fmod_real_module,
        "?mixerResume@System@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    return function ? function(s) : 0;
}
static int WINAPI bridge_fmod_mixer_suspend(void *s)
{
    FmodNoArgsFn function;
    if (s == &fake_fmod_system || !load_real_fmod()) return 0;
    function = (FmodNoArgsFn)GetProcAddress(fmod_real_module,
        "?mixerSuspend@System@FMOD@@QAG?AW4FMOD_RESULT@@XZ");
    return function ? function(s) : 0;
}
static int WINAPI bridge_fmod_set_mute(void *s,int v)
{
    int requested = v;
    int result;
    char line[192];
    if(s==&fake_fmod_group||s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_bool(s,v);
    /* UWP lifecycle can leave the master group muted in an HWND host. */
    if (v && game_window && GetForegroundWindow() == game_window) v = FALSE;
    result = real_fmod_set_mute(s,v);
    wsprintfA(line, "FMOD bridge: setMute group=%p requested=%d applied=%d result=%d",
              s, requested, v, result);
    log_line(line);
    return result;
}
static int WINAPI bridge_fmod_set_volume(void *s,float v)
{
    int r;
    char line[192];
    if(s==&fake_fmod_group||s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_float(s,v);
    r=real_fmod_set_volume(s,v);
    wsprintfA(line, "FMOD bridge: setVolume object=%p value_milli=%d result=%d",
              s, (int)(v * 1000.0f), r);
    log_line(line);
    return r;
}

static int bridge_fmod_create_common(FmodCreateSoundFn function, const char *kind,
                                     void *s,const char *name,unsigned mode,
                                     void *info,void **out)
{
    char normalized[MAX_PATH * 3];
    const char *used;
    int result;
    if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_create_sound(s,name,mode,info,out);
    used = normalize_fmod_path(name, normalized, sizeof(normalized));
    result = function(s, used, mode, info, out);
    if (result || fmod_create_log_count < 48) {
        log_fmod_sound_result(kind, used, mode, result, out ? *out : NULL);
        ++fmod_create_log_count;
    }
    return result;
}
static int WINAPI bridge_fmod_create_stream(void *s,const char *n,unsigned m,void *i,void **o)
{ return bridge_fmod_create_common(real_fmod_create_stream,"createStream",s,n,m,i,o); }
static int WINAPI bridge_fmod_create_sound(void *s,const char *n,unsigned m,void *i,void **o)
{ return bridge_fmod_create_common(real_fmod_create_sound,"createSound",s,n,m,i,o); }
static int WINAPI bridge_fmod_set_min_max(void *s,float a,float b)
{ if(s==&fake_fmod_sound||!load_real_fmod()) return fake_fmod_two_floats(s,a,b); return real_fmod_set_min_max(s,a,b); }
static int WINAPI bridge_fmod_get_sub_count(void *s,int *o)
{ if(s==&fake_fmod_sound||!load_real_fmod()) return fake_fmod_get_count(s,o); return real_fmod_get_sub_count(s,o); }
static int WINAPI bridge_fmod_get_sub_sound(void *s,int i,void **o)
{ if(s==&fake_fmod_sound||!load_real_fmod()) return fake_fmod_get_subsound(s,i,o); return real_fmod_get_sub_sound(s,i,o); }
static int WINAPI bridge_fmod_play_sound(void *s,void *sound,void *group,int paused,void **out)
{
    int r;
    if(s==&fake_fmod_system||sound==&fake_fmod_sound||!load_real_fmod()) return fake_fmod_play_sound(s,sound,group,paused,out);
    r=real_fmod_play_sound(s,sound,group,paused,out);
    if(r || fmod_play_log_count < 32) {
        char line[256];
        wsprintfA(line,"FMOD bridge: playSound result=%d paused=%d sound=%p group=%p channel=%p",
                  r,paused,sound,group,out?*out:NULL);
        log_line(line);
        ++fmod_play_log_count;
    }
    return r;
}
static int WINAPI bridge_fmod_set_attributes(void *s,const void *a,const void *b,const void *c)
{ if(s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_vectors(s,a,b,c); return real_fmod_set_3d_attributes(s,a,b,c); }
static int WINAPI bridge_fmod_set_pitch(void *s,float v)
{ if(s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_float(s,v); return real_fmod_set_pitch(s,v); }
static int WINAPI bridge_fmod_set_paused(void *s,int v)
{ if(s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_bool(s,v); return real_fmod_set_paused(s,v); }
static int WINAPI bridge_fmod_is_playing(void *s,BYTE *o)
{ if(s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_is_playing(s,o); return real_fmod_is_playing(s,o); }
static int WINAPI bridge_fmod_stop(void *s)
{ if(s==&fake_fmod_channel||!load_real_fmod()) return fake_fmod_no_args(s); return real_fmod_stop(s); }
static int WINAPI bridge_fmod_update(void *s)
{
    int r;
    if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_no_args(s);
    r=real_fmod_update(s);
    if(r) log_fmod_call("update",r);
    if (!r && (++fmod_update_counter % 120u) == 0u &&
        game_window && GetForegroundWindow() == game_window) {
        if (fmod_master_group && real_fmod_set_mute)
            real_fmod_set_mute(fmod_master_group, FALSE);
        if (fmod_sound_group && real_fmod_set_mute)
            real_fmod_set_mute(fmod_sound_group, FALSE);
        if (fmod_music_group && real_fmod_set_mute)
            real_fmod_set_mute(fmod_music_group, FALSE);
    }
    return r;
}
static int WINAPI bridge_fmod_set_listener(void *s,int l,const void *p,const void *v,const void *f,const void *u)
{ if(s==&fake_fmod_system||!load_real_fmod()) return fake_fmod_listener(s,l,p,v,f,u); return real_fmod_set_listener(s,l,p,v,f,u); }

static void log_line(const char *text)
{
    static LONG diagnostics = -1;

    if (!text) return;
    if (diagnostics < 0) {
        diagnostics =
            GetEnvironmentVariableA("WIN32CRAFT_DEBUG", NULL, 0) ? 1 : 0;
    }
    if (diagnostics) {
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
}

static ULONG WINAPI fake_add_ref(FakeInspectable *object)
{
    return (ULONG)InterlockedIncrement(&object->references);
}

static ULONG WINAPI fake_release(FakeInspectable *object)
{
    LONG value = InterlockedDecrement(&object->references);
    if (value < 1) {
        InterlockedExchange(&object->references, 1);
        value = 1;
    }
    return (ULONG)value;
}

static HRESULT WINAPI fake_query_interface(
    FakeInspectable *object, REFIID iid, void **result)
{
    (void)iid;
    if (!result) {
        return E_POINTER;
    }
    *result = object;
    fake_add_ref(object);
    return S_OK;
}

static HRESULT WINAPI fake_get_iids(
    FakeInspectable *object, ULONG *count, IID **iids)
{
    (void)object;
    if (count) *count = 0;
    if (iids) *iids = NULL;
    return S_OK;
}

static HRESULT WINAPI fake_get_runtime_class_name(
    FakeInspectable *object, HSTRING *name)
{
    (void)object;
    if (!name) return E_POINTER;
    return WindowsCreateString(L"", 0, name);
}

static HRESULT WINAPI fake_get_trust_level(
    FakeInspectable *object, INT *level)
{
    (void)object;
    if (!level) return E_POINTER;
    *level = 0;
    return S_OK;
}

static HRESULT WINAPI fake_factory_current(
    FakeInspectable *factory, void **result)
{
    FakeInspectable *object;
    if (!result) return E_POINTER;
    if (factory->kind == FAKE_PACKAGE_FACTORY) {
        object = &package_object;
    } else if (factory->kind == FAKE_APPDATA_FACTORY) {
        object = &appdata_object;
    } else {
        object = &currentapp_object;
    }
    *result = object;
    fake_add_ref(object);
    return S_OK;
}

static HRESULT WINAPI fake_package_installed_location(
    FakeInspectable *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &package_folder;
    fake_add_ref(&package_folder);
    return S_OK;
}

static HRESULT WINAPI fake_appdata_local_folder(
    FakeInspectable *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &data_folder;
    fake_add_ref(&data_folder);
    return S_OK;
}

static HRESULT WINAPI fake_folder_path(
    FakeInspectable *folder, HSTRING *result)
{
    LPCWSTR path;
    if (!result) return E_POINTER;
    path = folder->kind == FAKE_PACKAGE_FOLDER
        ? package_path : game_data_path;
    return WindowsCreateString(path, (UINT32)lstrlenW(path), result);
}

static HRESULT WINAPI fake_memory_limit(
    FakeInspectable *object, ULONGLONG *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = 0x100000000ULL;
    return S_OK;
}

static HRESULT WINAPI fake_memory_usage(
    FakeInspectable *object, ULONGLONG *result)
{
    typedef struct Win32CraftProcessMemoryCounters {
        DWORD cb;
        DWORD page_fault_count;
        SIZE_T peak_working_set_size;
        SIZE_T working_set_size;
        SIZE_T quota_peak_paged_pool_usage;
        SIZE_T quota_paged_pool_usage;
        SIZE_T quota_peak_non_paged_pool_usage;
        SIZE_T quota_non_paged_pool_usage;
        SIZE_T pagefile_usage;
        SIZE_T peak_pagefile_usage;
    } Win32CraftProcessMemoryCounters;
    typedef BOOL (WINAPI *GetProcessMemoryInfoFn)(
        HANDLE, Win32CraftProcessMemoryCounters *, DWORD);
    static GetProcessMemoryInfoFn get_process_memory_info;
    static BOOL resolved;
    Win32CraftProcessMemoryCounters counters;
    HMODULE psapi;

    (void)object;
    if (!result) return E_POINTER;
    if (!resolved) {
        psapi = LoadLibraryW(L"psapi.dll");
        if (psapi) get_process_memory_info =
            (GetProcessMemoryInfoFn)GetProcAddress(
                psapi, "GetProcessMemoryInfo");
        resolved = TRUE;
    }
    ZeroMemory(&counters, sizeof(counters));
    counters.cb = sizeof(counters);
    if (get_process_memory_info && get_process_memory_info(
            GetCurrentProcess(), &counters, sizeof(counters))) {
        *result = (ULONGLONG)counters.working_set_size;
    } else {
        *result = 0x10000000ULL;
    }
    return S_OK;
}

static HRESULT WINAPI fake_coreapp_id(
    FakeInspectable *object, HSTRING *result)
{
    static const wchar_t app_id[] = L"Minecraft.Win32";
    (void)object;
    if (!result) return E_POINTER;
    return WindowsCreateString(app_id, ARRAYSIZE(app_id) - 1, result);
}

static HRESULT WINAPI fake_currentapp_license(
    FakeInspectable *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &license_object;
    fake_add_ref(&license_object);
    return S_OK;
}

static HRESULT WINAPI fake_license_active(
    FakeInspectable *object, BYTE *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = TRUE;
    return S_OK;
}

static HRESULT WINAPI fake_license_trial(
    FakeInspectable *object, BYTE *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = FALSE;
    return S_OK;
}

static HRESULT WINAPI fake_license_add_changed(
    FakeInspectable *object, void *handler, LONGLONG *token)
{
    (void)object; (void)handler;
    if (!token) return E_POINTER;
    *token = 0;
    return S_OK;
}


/* Windows 7 has no Windows.Networking.Connectivity WinRT class.  The
 * Minecraft startup path only asks NetworkInformation for the current
 * InternetConnectionProfile.  A successful call with a null profile is the
 * documented "no active connection" result and is already handled by the
 * game.  Supplying the complete statics vtable also prevents later status
 * registration calls from throwing REGDB_E_CLASSNOTREG. */
static HRESULT WINAPI fake_network_get_collection(
    FakeInspectable *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = NULL;
    return S_OK;
}

static HRESULT WINAPI fake_network_get_internet_profile(
    FakeInspectable *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = NULL;
    log_line("Win7 NetworkInformation: no active InternetConnectionProfile");
    return S_OK;
}

static HRESULT WINAPI fake_network_get_proxy_async(
    FakeInspectable *object, void *uri, void **operation)
{
    (void)object;
    (void)uri;
    if (!operation) return E_POINTER;
    *operation = NULL;
    return E_NOTIMPL;
}

static HRESULT WINAPI fake_network_get_sorted_pairs(
    FakeInspectable *object, void *destinations, INT options, void **result)
{
    (void)object;
    (void)destinations;
    (void)options;
    if (!result) return E_POINTER;
    *result = NULL;
    return E_NOTIMPL;
}

static HRESULT WINAPI fake_network_add_changed(
    FakeInspectable *object, void *handler, LONGLONG *token)
{
    (void)object;
    (void)handler;
    if (!token) return E_POINTER;
    *token = 0;
    return S_OK;
}

static HRESULT WINAPI fake_network_remove_changed(
    FakeInspectable *object, LONGLONG token)
{
    (void)object;
    (void)token;
    return S_OK;
}

/* IInspectable (0..5), then INetworkInformationStatics methods:
 * 6 GetConnectionProfiles, 7 GetInternetConnectionProfile,
 * 8 GetLanIdentifiers, 9 GetHostNames, 10 GetProxyConfigurationAsync,
 * 11 GetSortedEndpointPairs, 12/13 NetworkStatusChanged add/remove. */
static const void *network_factory_vtable[14] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_network_get_collection,
    fake_network_get_internet_profile,
    fake_network_get_collection,
    fake_network_get_collection,
    fake_network_get_proxy_async,
    fake_network_get_sorted_pairs,
    fake_network_add_changed,
    fake_network_remove_changed
};

static const void *factory_vtable[7] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_factory_current
};

static const void *currentapp_factory_vtable[12] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_currentapp_license, NULL, NULL, NULL, NULL, NULL
};

static const void *package_vtable[8] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    NULL, fake_package_installed_location
};

static const void *appdata_vtable[15] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    NULL, NULL, NULL, NULL, NULL, NULL,
    fake_appdata_local_folder, fake_appdata_local_folder, fake_appdata_local_folder
};

static const void *currentapp_vtable[16] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    NULL, fake_license_active, fake_license_trial, NULL, fake_license_add_changed,
    NULL, NULL, NULL, NULL, NULL
};

static const void *license_vtable[12] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    NULL, fake_license_active, fake_license_trial, NULL,
    fake_license_add_changed, NULL
};

static const void *folder_vtable[13] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    NULL, NULL, NULL, NULL, NULL, NULL, fake_folder_path
};

static const void *memory_vtable[8] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_memory_usage, fake_memory_limit
};

static const void *coreapp_vtable[12] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_coreapp_id, NULL, NULL, NULL, NULL, NULL
};

static HRESULT WINAPI win32_get_activation_factory(
    LPCWSTR class_name, const GUID *iid, void **result)
{
    HRESULT activation_result;
    char class_utf8[256];
    char line[320];

    (void)iid;
    if (!result) return E_POINTER;
    *result = NULL;
    if (class_name &&
        lstrcmpW(class_name, L"Windows.ApplicationModel.Package") == 0) {
        log_line("redirected Windows.ApplicationModel.Package");
        *result = &package_factory;
        fake_add_ref(&package_factory);
        return S_OK;
    }
    if (class_name &&
        lstrcmpW(class_name, L"Windows.Storage.ApplicationData") == 0) {
        log_line("redirected Windows.Storage.ApplicationData");
        *result = &appdata_factory;
        fake_add_ref(&appdata_factory);
        return S_OK;
    }
    if (class_name &&
        lstrcmpW(class_name, L"Windows.System.MemoryManager") == 0) {
        log_line("redirected Windows.System.MemoryManager");
        *result = &memory_factory;
        fake_add_ref(&memory_factory);
        return S_OK;
    }
    if (class_name &&
        lstrcmpW(class_name,
                 L"Windows.ApplicationModel.Store.CurrentApp") == 0) {
        log_line("redirected Windows.ApplicationModel.Store.CurrentApp");
        *result = &currentapp_factory;
        fake_add_ref(&currentapp_factory);
        return S_OK;
    }
    if (class_name &&
        lstrcmpW(class_name,
                 L"Windows.ApplicationModel.Core.CoreApplication") == 0) {
        log_line("redirected Windows.ApplicationModel.Core.CoreApplication");
        *result = &coreapp_factory;
        fake_add_ref(&coreapp_factory);
        return S_OK;
    }
    if (class_name &&
        lstrcmpW(class_name,
                 L"Windows.Networking.Connectivity.NetworkInformation") == 0) {
        log_line("redirected Windows.Networking.Connectivity.NetworkInformation");
        *result = &network_factory;
        fake_add_ref(&network_factory);
        return S_OK;
    }
    if (original_activation) {
        activation_result = original_activation(class_name, iid, result);
        class_utf8[0] = '\0';
        if (class_name) {
            WideCharToMultiByte(CP_UTF8, 0, class_name, -1, class_utf8,
                                sizeof(class_utf8), NULL, NULL);
        }
        wsprintfA(line, "WinRT fallback %s: HRESULT 0x%08lx",
                  class_utf8[0] ? class_utf8 : "(null)",
                  (unsigned long)activation_result);
        log_line(line);
        return activation_result;
    }
    return REGDB_E_CLASSNOTREG;
}

static BOOL install_activation_redirect(BYTE *image)
{
    GameActivationFn *slot =
        (GameActivationFn *)(image + EARLY_RVA(0x0060f980, 0x00786a70));
    DWORD old_protection;
    DWORD ignored;

    package_factory.vtable = factory_vtable;
    package_factory.references = 1;
    package_factory.kind = FAKE_PACKAGE_FACTORY;
    appdata_factory.vtable = factory_vtable;
    appdata_factory.references = 1;
    appdata_factory.kind = FAKE_APPDATA_FACTORY;
    memory_factory.vtable = memory_vtable;
    memory_factory.references = 1;
    memory_factory.kind = FAKE_MEMORY_FACTORY;
    currentapp_factory.vtable = currentapp_factory_vtable;
    currentapp_factory.references = 1;
    currentapp_factory.kind = FAKE_CURRENTAPP_FACTORY;
    coreapp_factory.vtable = coreapp_vtable;
    coreapp_factory.references = 1;
    coreapp_factory.kind = FAKE_COREAPP_FACTORY;
    package_object.vtable = package_vtable;
    package_object.references = 1;
    package_object.kind = FAKE_PACKAGE;
    appdata_object.vtable = appdata_vtable;
    appdata_object.references = 1;
    appdata_object.kind = FAKE_APPDATA;
    currentapp_object.vtable = currentapp_vtable;
    currentapp_object.references = 1;
    currentapp_object.kind = FAKE_CURRENTAPP;
    license_object.vtable = license_vtable;
    license_object.references = 1;
    license_object.kind = FAKE_LICENSE;
    package_folder.vtable = folder_vtable;
    package_folder.references = 1;
    package_folder.kind = FAKE_PACKAGE_FOLDER;
    data_folder.vtable = folder_vtable;
    data_folder.references = 1;
    data_folder.kind = FAKE_DATA_FOLDER;
    network_factory.vtable = network_factory_vtable;
    network_factory.references = 1;
    network_factory.kind = FAKE_NETWORK_FACTORY;

    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection)) {
        log_line("VirtualProtect activation IAT failed");
        return FALSE;
    }
    original_activation = *slot;
    *slot = win32_get_activation_factory;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("WinRT Package/ApplicationData redirect installed");
    return TRUE;
}

static void WINAPI win32_cxx_throw(void *exception, const void *throw_info)
{
    void *frames[24];
    USHORT count;
    USHORT index;
    char line[96];

    log_line("game raised a C++ exception; stack follows");
    count = RtlCaptureStackBackTrace(0, 24, frames, NULL);
    for (index = 0; index < count; ++index) {
        wsprintfA(line, "  frame %u: %p", (unsigned)index, frames[index]);
        log_line(line);
    }
    original_cxx_throw(exception, throw_info);
}

static BOOL install_exception_trace(BYTE *image)
{
    GameCxxThrowFn *slot =
        (GameCxxThrowFn *)(image + EARLY_RVA(0x0060f3c4, 0x00786498));
    DWORD old_protection;
    DWORD ignored;

    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection)) {
        return FALSE;
    }
    original_cxx_throw = *slot;
    *slot = win32_cxx_throw;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("C++ exception trace installed");
    return TRUE;
}

static HRESULT WINAPI win32_d3d_compile(
    LPCVOID source, SIZE_T source_size, LPCSTR source_name,
    const D3D_SHADER_MACRO *defines, ID3DInclude *include_handler,
    LPCSTR entry_point, LPCSTR target, UINT flags1, UINT flags2,
    ID3DBlob **code, ID3DBlob **errors)
{
    ID3DInclude *actual_include = include_handler;
    HRESULT result;
    const char *slash;
    const char *backslash;
    SIZE_T directory_length;

    if (include_handler == D3D_COMPILE_STANDARD_FILE_INCLUDE) {
        shader_include_directory[0] = 0;
        slash = source_name ? strrchr(source_name, '/') : NULL;
        backslash = source_name ? strrchr(source_name, '\\') : NULL;
        if (!slash || (backslash && backslash > slash)) slash = backslash;
        if (slash) {
            directory_length = (SIZE_T)(slash - source_name) + 1;
            if (directory_length < sizeof(shader_include_directory)) {
                CopyMemory(shader_include_directory, source_name,
                           directory_length);
                shader_include_directory[directory_length] = 0;
            }
        }
        actual_include = &win32_include;
    }
    result = original_d3d_compile(
        source, source_size, source_name, defines, actual_include,
        entry_point, target, flags1, flags2, code, errors);
    char line[256];

    wsprintfA(line, "D3DCompile entry=%s target=%s bytes=%u: HRESULT 0x%08lX",
              entry_point ? entry_point : "<null>",
              target ? target : "<null>", (unsigned)source_size,
              (unsigned long)result);
    log_line(line);
    if (FAILED(result)) {
        FILE *shader_file = fopen("shader_failure.hlsl", "wb");
        if (shader_file) {
            fwrite(source, 1, source_size, shader_file);
            fclose(shader_file);
        }
        if (errors && *errors) {
            const char *message =
                (const char *)ID3D10Blob_GetBufferPointer(*errors);
            SIZE_T message_size = ID3D10Blob_GetBufferSize(*errors);
            if (log_file && message && message_size) {
                fprintf(log_file, "D3DCompile error: %.*s\n",
                        (int)message_size, message);
                fflush(log_file);
            }
        }
    }
    return result;
}

static BOOL install_d3d_compile_trace(BYTE *image)
{
    GameD3DCompileFn *slot =
        (GameD3DCompileFn *)(image + EARLY_RVA(0x0060f028, 0x007860b4));
    DWORD old_protection;
    DWORD ignored;

    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection)) {
        log_line("VirtualProtect D3DCompile IAT failed");
        return FALSE;
    }
    original_d3d_compile = *slot;
    *slot = win32_d3d_compile;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("D3DCompile trace installed");
    return TRUE;
}

static HRESULT WINAPI win32_d3d11_create_device(
    void *adapter, UINT driver_type, HMODULE software, UINT flags,
    const UINT *feature_levels, UINT feature_level_count, UINT sdk_version,
    ID3D11Device **device, UINT *selected_feature_level,
    ID3D11DeviceContext **immediate_context)
{
    UINT compatible_levels[16];
    UINT compatible_count = 0;
    UINT index;
    UINT adjusted_flags = flags | 0x20U; /* D3D11_CREATE_DEVICE_BGRA_SUPPORT */
    const UINT *levels = feature_levels;
    UINT level_count = feature_level_count;
    HRESULT result;
    char line[192];

    if (host_is_windows7 && feature_levels && feature_level_count) {
        for (index = 0; index < feature_level_count && compatible_count < 16; ++index) {
            /* The Windows 7 D3D11 runtime rejects a list containing 11_1
             * with E_INVALIDARG, even when lower levels are available. */
            if (feature_levels[index] != 0xb100U) {
                compatible_levels[compatible_count++] = feature_levels[index];
            }
        }
        if (compatible_count) {
            levels = compatible_levels;
            level_count = compatible_count;
        }
    }

    result = original_d3d11_create_device(
        adapter, driver_type, software, adjusted_flags,
        levels, level_count, sdk_version,
        device, selected_feature_level, immediate_context);
    wsprintfA(line,
        "D3D11CreateDevice compat: driver=%u flags=0x%X levels=%u result=0x%08lX selected=0x%X device=%p context=%p",
        driver_type, adjusted_flags, level_count, (unsigned long)result,
        selected_feature_level ? *selected_feature_level : 0,
        device ? *device : NULL,
        immediate_context ? *immediate_context : NULL);
    log_line(line);

    if (FAILED(result) && driver_type == 1U) {
        if (device) *device = NULL;
        if (immediate_context) *immediate_context = NULL;
        if (selected_feature_level) *selected_feature_level = 0;
        result = original_d3d11_create_device(
            NULL, 5U, NULL, adjusted_flags,
            levels, level_count, sdk_version,
            device, selected_feature_level, immediate_context);
        wsprintfA(line,
            "D3D11CreateDevice WARP fallback: result=0x%08lX selected=0x%X device=%p context=%p",
            (unsigned long)result,
            selected_feature_level ? *selected_feature_level : 0,
            device ? *device : NULL,
            immediate_context ? *immediate_context : NULL);
        log_line(line);
    }

    if (SUCCEEDED(result) && device && *device &&
        immediate_context && *immediate_context) {
        if (captured_base_device) {
            ID3D11Device_Release(captured_base_device);
        }
        if (captured_base_context) {
            ID3D11DeviceContext_Release(captured_base_context);
        }
        captured_base_device = *device;
        captured_base_context = *immediate_context;
        ID3D11Device_AddRef(captured_base_device);
        ID3D11DeviceContext_AddRef(captured_base_context);
        log_line("captured base D3D11 device/context before UWP Device2 QI");
    }
    return result;
}

static BOOL patch_win7_device2_queries_to_base(BYTE *image)
{
    /* The original UWP DeviceResources code queries these two unique
     * constants immediately after D3D11CreateDevice:
     *   image+0x67F56C = IID_ID3D11Device2
     *   image+0x67F55C = IID_ID3D11DeviceContext2
     * Both interfaces require Windows 8.1.  Replacing only the IIDs with
     * their inherited base interfaces lets the original assignment and
     * AddRef/Release code run unchanged on Windows 7. */
    GUID *device_iid = (GUID *)(image + EARLY_RVA(0x0067f56c, 0x007fc964));
    GUID *context_iid = (GUID *)(image + EARLY_RVA(0x0067f55c, 0x007fc584));
    DWORD old_device = 0, old_context = 0, ignored = 0;

    if (!host_is_windows7) {
        return TRUE;
    }
    if (!VirtualProtect(device_iid, sizeof(*device_iid), PAGE_READWRITE,
                        &old_device) ||
        !VirtualProtect(context_iid, sizeof(*context_iid), PAGE_READWRITE,
                        &old_context)) {
        log_line("Windows 7 D3D base-IID patch protection failed; retained-pointer fallback remains active");
        return FALSE;
    }
    memcpy(device_iid, host_is_0142 ? &IID_ID3D11DeviceContext : &IID_ID3D11Device,
           sizeof(*device_iid));
    memcpy(context_iid, &IID_ID3D11DeviceContext, sizeof(*context_iid));
    VirtualProtect(device_iid, sizeof(*device_iid), old_device, &ignored);
    VirtualProtect(context_iid, sizeof(*context_iid), old_context, &ignored);
    FlushInstructionCache(GetCurrentProcess(), device_iid, sizeof(*device_iid));
    FlushInstructionCache(GetCurrentProcess(), context_iid, sizeof(*context_iid));
    log_line("Windows 7 D3D: Device2/Context2 QueryInterface IIDs replaced with base interfaces");
    return TRUE;
}

static BOOL install_d3d11_device_compat(BYTE *image)
{
    GameD3D11CreateDeviceFn *slot =
        (GameD3D11CreateDeviceFn *)(image + EARLY_RVA(0x0060f880, 0x00786970));
    DWORD old_protection;
    DWORD ignored;

    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection)) {
        log_line("D3D11CreateDevice IAT hook protection failed");
        return FALSE;
    }
    original_d3d11_create_device = *slot;
    *slot = win32_d3d11_create_device;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("installed Windows 7 D3D11CreateDevice compatibility hook");
    return TRUE;
}

static HRESULT STDMETHODCALLTYPE win32_present(
    IDXGISwapChain *swap_chain, UINT sync_interval, UINT flags)
{
    static unsigned present_count;
    static unsigned busy_drop_count;
    UINT actual_sync = sync_interval;
    UINT actual_flags = flags;
    HRESULT result;

    ++present_count;
    actual_sync = Win32CraftPresentSyncInterval();
    actual_flags &= ~DXGI_PRESENT_DO_NOT_WAIT;
    result = original_swap_chain_present
        ? original_swap_chain_present(swap_chain, actual_sync, actual_flags)
        : E_FAIL;
    if (result == DXGI_ERROR_WAS_STILL_DRAWING) {
        ++busy_drop_count;
        InterlockedIncrement((LONG *)&game_present_success_count);
        if (busy_drop_count == 1)
            log_line("VSync Present unexpectedly reported compositor busy");
        return S_OK;
    }
    if (SUCCEEDED(result))
        InterlockedIncrement((LONG *)&game_present_success_count);
    return result;
}

static BOOL install_swap_chain_present_hook(IDXGISwapChain *swap_chain)
{
    SwapChainPresentFn *slot;
    DWORD old_protection;
    DWORD ignored;

    if (!swap_chain || !swap_chain->lpVtbl) return FALSE;
    slot = (SwapChainPresentFn *)&swap_chain->lpVtbl[8];
    if (*slot == win32_present) return TRUE;
    if (!VirtualProtect(slot, sizeof(*slot), PAGE_EXECUTE_READWRITE,
                        &old_protection)) {
        log_line("swap-chain Present vtable hook protection failed");
        return FALSE;
    }
    original_swap_chain_present = *slot;
    *slot = win32_present;
    VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
    log_line("swap-chain Present vtable hook installed");
    return TRUE;
}

static void __attribute__((thiscall)) win32_game_tick(
    void *minecraft_game, int tick_index, int last_tick_index)
{
    static unsigned tick_count;
    char line[128];

    ++tick_count;
    if (tick_count <= 10 || tick_count == 60) {
        wsprintfA(line, "MinecraftGame tick %u begin (%d/%d)",
                  tick_count, tick_index, last_tick_index);
        log_line(line);
    }
    original_game_tick(minecraft_game, tick_index, last_tick_index);
    if (tick_count <= 10 || tick_count == 60) {
        wsprintfA(line, "MinecraftGame tick %u end", tick_count);
        log_line(line);
    }
}

static BOOL install_game_tick_trace(BYTE *image)
{
    BYTE *site = image + 0x003b33a4;
    static const BYTE expected[5] = {0xe8, 0x97, 0x00, 0x00, 0x00};
    DWORD old_protection;
    DWORD ignored;
    INT_PTR displacement = (const BYTE *)win32_game_tick - (site + 5);

    original_game_tick = (GameTickFn)(image + 0x003b3440);
    if (memcmp(site, expected, sizeof(expected)) != 0 ||
        displacement < (INT_PTR)INT_MIN ||
        displacement > (INT_PTR)INT_MAX ||
        !VirtualProtect(site, sizeof(expected), PAGE_EXECUTE_READWRITE,
                        &old_protection)) {
        log_line("MinecraftGame tick trace patch failed");
        return FALSE;
    }
    site[0] = 0xe8;
    *(LONG *)(site + 1) = (LONG)displacement;
    VirtualProtect(site, sizeof(expected), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(expected));
    log_line("MinecraftGame tick trace installed");
    return TRUE;
}

static void __attribute__((thiscall)) win32_apply_render_state(
    void *state, void *draw_context, void *render_context,
    void *reserved_1, void *reserved_2)
{
    static unsigned null_state_count;
    static unsigned valid_state_count;

    if (!state) {
        ++null_state_count;
        if (null_state_count <= 10 || null_state_count == 60 ||
            null_state_count == 600) {
            char line[96];
            wsprintfA(line, "skipped null render state %u",
                      null_state_count);
            log_line(line);
        }
        return;
    }
    ++valid_state_count;
    if (valid_state_count <= 3 || valid_state_count == 60 ||
        valid_state_count == 600) {
        char line[96];
        wsprintfA(line, "applying valid render state %u state=%p",
                  valid_state_count, state);
        log_line(line);
    }
    original_apply_render_state(
        state, draw_context, render_context, reserved_1, reserved_2);
}

static const char *material_string(const BYTE *material, size_t offset)
{
    return *(const DWORD *)(material + offset + 0x14) < 0x10
        ? (const char *)(material + offset)
        : *(const char * const *)(material + offset);
}

static void __attribute__((thiscall)) win32_material_draw(
    void *material, void *draw_context, void *render_context,
    void *reserved)
{
    static unsigned null_material_count;
    const BYTE *value = (const BYTE *)material;

    if (value && !IsBadReadPtr(value, 0xc4) &&
        !*(void * const *)(value + 0x58)) {
        ++null_material_count;
        if (null_material_count <= 10 || null_material_count == 60 ||
            null_material_count == 600) {
            char line[384];
            wsprintfA(line,
                      "null material %u object=%p flags=%08lX "
                      "vs='%s' ps='%s' gs='%s'",
                      null_material_count, material,
                      (unsigned long)*(const DWORD *)(value + 8),
                      material_string(value, 0x0c),
                      material_string(value, 0x24),
                      material_string(value, 0x3c));
            log_line(line);
        }
    }
    original_material_draw(
        material, draw_context, render_context, reserved);
}

static BOOL install_material_trace(BYTE *image)
{
    BYTE *site = image + 0x00535256;
    static const BYTE expected[5] = {0xe8, 0x95, 0xe2, 0xff, 0xff};
    DWORD old_protection;
    DWORD ignored;
    INT_PTR displacement = (const BYTE *)win32_material_draw - (site + 5);

    original_material_draw = (GameMaterialDrawFn)(image + 0x005334f0);
    if (memcmp(site, expected, sizeof(expected)) != 0 ||
        displacement < (INT_PTR)INT_MIN ||
        displacement > (INT_PTR)INT_MAX ||
        !VirtualProtect(site, sizeof(expected), PAGE_EXECUTE_READWRITE,
                        &old_protection)) {
        log_line("material trace patch failed");
        return FALSE;
    }
    site[0] = 0xe8;
    *(LONG *)(site + 1) = (LONG)displacement;
    VirtualProtect(site, sizeof(expected), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(expected));
    log_line("material trace installed");
    return TRUE;
}

static BOOL install_render_state_guard(BYTE *image)
{
    BYTE *site = image + 0x005335f4;
    static const BYTE expected[5] = {0xe8, 0x97, 0xa2, 0x00, 0x00};
    DWORD old_protection;
    DWORD ignored;
    INT_PTR displacement =
        (const BYTE *)win32_apply_render_state - (site + 5);

    original_apply_render_state =
        (GameApplyRenderStateFn)(image + 0x0053d890);
    if (memcmp(site, expected, sizeof(expected)) != 0 ||
        displacement < (INT_PTR)INT_MIN ||
        displacement > (INT_PTR)INT_MAX ||
        !VirtualProtect(site, sizeof(expected), PAGE_EXECUTE_READWRITE,
                        &old_protection)) {
        log_line("render-state guard patch failed");
        return FALSE;
    }
    site[0] = 0xe8;
    *(LONG *)(site + 1) = (LONG)displacement;
    VirtualProtect(site, sizeof(expected), old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(expected));
    log_line("render-state guard installed");
    return TRUE;
}

typedef struct DevelopmentVersionString0132 {
    union {
        char small[16];
        char *heap;
    } storage;
    DWORD length;
    DWORD capacity;
} DevelopmentVersionString0132;

static BOOL write_relative_jump(BYTE *site, const void *target);

/* Override 0.13.2's empty #development_version value with the project URL. */
static void *__attribute__((thiscall, noinline))
win32_development_version_text_0132(
    void *self, DevelopmentVersionString0132 *output)
{
    static const char text[] = "github.com/deltaxur/Win32Craft";
    GameMallocFn allocate;
    char *buffer;
    (void)self;
    if (!output) return NULL;
    ZeroMemory(output, sizeof(*output));
    allocate = *(GameMallocFn *)(game_image + EARLY_RVA(0x0060f694, 0x00786758));
    buffer = allocate ? (char *)allocate(sizeof(text)) : NULL;
    if (!buffer) return output;
    CopyMemory(buffer, text, sizeof(text));
    output->storage.heap = buffer;
    output->length = (DWORD)(sizeof(text) - 1);
    output->capacity = (DWORD)(sizeof(text) - 1);
    return output;
}

static BOOL install_development_version_text_0132(BYTE *image)
{
    static const BYTE expected[10] = {
        0x55, 0x8b, 0xec, 0x51, 0x8b,
        0x4d, 0x08, 0xc7, 0x45, 0xfc
    };
    BYTE *site;
    static const BYTE expected_0142[10] = {
        0x55, 0x8b, 0xec, 0x51, 0xff, 0x75, 0x08, 0x8b, 0x4d, 0x08
    };

    if (!image) return FALSE;
    site = image + RVA_EARLY_DEVELOPMENT_VERSION_GETTER;
    if (memcmp(site, host_is_0142 ? expected_0142 : expected,
               sizeof(expected)) != 0) {
        log_line("Minecraft 0.13.2 development-version getter signature mismatch");
        return FALSE;
    }
    if (!write_relative_jump(site, win32_development_version_text_0132)) {
        log_line("Minecraft 0.13.2 development-version text hook failed");
        return FALSE;
    }
    log_line("Minecraft 0.13.2 development-version text overridden");
    return TRUE;
}

static BOOL write_relative_jump(BYTE *site, const void *target)
{
    DWORD old_protection;
    DWORD ignored;
    INT_PTR displacement = (const BYTE *)target - (site + 5);

    if (displacement < (INT_PTR)INT_MIN ||
        displacement > (INT_PTR)INT_MAX) {
        return FALSE;
    }
    if (!VirtualProtect(site, 10, PAGE_EXECUTE_READWRITE, &old_protection)) {
        return FALSE;
    }
    site[0] = 0xe9;
    *(LONG *)(site + 1) = (LONG)displacement;
    FillMemory(site + 5, 5, 0x90);
    VirtualProtect(site, 10, old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, 10);
    return TRUE;
}

static BOOL verify_patched_audio_exe(BYTE *image)
{
    static const BYTE sound_branch[] = {
        0x3b, 0xf0, 0x74, 0x6f,
        0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static const char desktop_name[] = "fmod.dll";
    const BYTE *site = image + 0x001e6a43;
    const char *delay_name = (const char *)(image + 0x006108f0);

    if (host_is_0142) return TRUE;

    if (memcmp(site, sound_branch, sizeof(sound_branch)) != 0) {
        log_line("audio EXE mismatch: sound-event table branch is not restored");
        return FALSE;
    }
    if (memcmp(delay_name, desktop_name, sizeof(desktop_name)) != 0) {
        log_line("audio EXE mismatch: FMOD delay-import name is not fmod.dll");
        return FALSE;
    }
    log_line("audio EXE verified: sound-event table enabled and FMOD name=fmod.dll");
    return TRUE;
}

static BOOL install_native_fmod_bridge(BYTE *image)
{
    static const void *targets[] = {
        bridge_fmod_system_create, bridge_fmod_get_version,
        bridge_fmod_init, bridge_fmod_set_3d_settings,
        bridge_fmod_create_group, bridge_fmod_get_group,
        bridge_fmod_add_group, bridge_fmod_sound_release,
        bridge_fmod_system_close, bridge_fmod_system_release,
        bridge_fmod_set_mute, bridge_fmod_set_volume,
        bridge_fmod_create_stream, bridge_fmod_create_sound,
        bridge_fmod_set_min_max, bridge_fmod_get_sub_count,
        bridge_fmod_get_sub_sound, bridge_fmod_play_sound,
        bridge_fmod_set_attributes, bridge_fmod_set_pitch,
        bridge_fmod_set_paused, bridge_fmod_is_playing,
        bridge_fmod_stop, bridge_fmod_update,
        bridge_fmod_set_listener
    };
    const void *targets_0142[] = {
        bridge_fmod_system_create, bridge_fmod_get_version,
        bridge_fmod_init, bridge_fmod_set_3d_settings,
        bridge_fmod_create_group, bridge_fmod_get_group,
        bridge_fmod_add_group, bridge_fmod_sound_release,
        bridge_fmod_system_close, bridge_fmod_system_release,
        bridge_fmod_mixer_resume, bridge_fmod_mixer_suspend,
        bridge_fmod_set_mute, bridge_fmod_set_volume,
        bridge_fmod_create_stream, bridge_fmod_create_sound,
        bridge_fmod_set_min_max, bridge_fmod_get_sub_count,
        bridge_fmod_get_sub_sound, bridge_fmod_play_sound,
        bridge_fmod_set_pitch, bridge_fmod_set_paused,
        bridge_fmod_set_attributes, bridge_fmod_is_playing,
        bridge_fmod_stop, bridge_fmod_update, bridge_fmod_set_listener
    };
    const void *const *selected = host_is_0142 ? targets_0142 : targets;
    SIZE_T target_size = host_is_0142 ? sizeof(targets_0142) : sizeof(targets);
    void **delay_iat = (void **)(image + EARLY_RVA(0x007e09cc, 0x009b298c));
    DWORD old_protection;
    DWORD ignored;
    unsigned index;

    if (!VirtualProtect(delay_iat, target_size, PAGE_READWRITE,
                        &old_protection)) {
        log_line("native FMOD delay-IAT protection change failed");
        return FALSE;
    }
    for (index = 0; index < target_size / sizeof(selected[0]); ++index)
        delay_iat[index] = (void *)selected[index];
    VirtualProtect(delay_iat, target_size, old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), delay_iat, target_size);
    log_line("FMOD delay-IAT replaced with native Win32 bridge");
    return TRUE;
}

static LONG CALLBACK win32_vectored_exception(EXCEPTION_POINTERS *details)
{
    DWORD code;
    DWORD *stack;
    unsigned index;
    char line[128];

    if (!details || !details->ExceptionRecord || !details->ContextRecord) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    code = details->ExceptionRecord->ExceptionCode;
    if (code != 0xe06d7363 && code != EXCEPTION_ACCESS_VIOLATION) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    wsprintfA(line, "VEH exception 0x%08lx at %p EAX=%p ECX=%p "
              "EDX=%p EDI=%p EBP=%p ESP=%p",
              (unsigned long)code,
              details->ExceptionRecord->ExceptionAddress,
              (void *)details->ContextRecord->Eax,
              (void *)details->ContextRecord->Ecx,
              (void *)details->ContextRecord->Edx,
              (void *)details->ContextRecord->Edi,
              (void *)details->ContextRecord->Ebp,
              (void *)details->ContextRecord->Esp);
    log_line(line);

    stack = (DWORD *)details->ContextRecord->Esp;
    if (!IsBadReadPtr(stack, 256 * sizeof(*stack))) {
        for (index = 0; index < 256; ++index) {
            BYTE *candidate = (BYTE *)(ULONG_PTR)stack[index];
            if (candidate >= game_image &&
                candidate < game_image + 0x00900000) {
                wsprintfA(line, "  game stack[%u] = %p",
                          index, candidate);
                log_line(line);
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static void create_user_data(void)
{
    wchar_t roaming[MAX_PATH];
    wchar_t game_dir[MAX_PATH];
    wchar_t options_path[MAX_PATH];
    HANDLE file;
    static const char defaults[] =
        "mp_username:Steve\r\n"
        "game_difficulty_new:1\r\n"
        "game_thirdperson:0\r\n"
        "gfx_dpadscale:0.5\r\n"
        "mp_server_visible:1\r\n"
        "mp_xboxlive_visible:0\r\n"
        "game_flatworldlayers:[7,3,3,2]\r\n"
        "game_limitworldsize:0\r\n"
        "game_language:en_US\r\n"
        "game_skintypefull:Standard_Steve\r\n"
        "game_lastcustomskinnew:\r\n"
        "ctrl_sensitivity:0.33\r\n"
        "ctrl_invertmouse:0\r\n"
        "ctrl_islefthanded:0\r\n"
        "ctrl_usetouchscreen:0\r\n"
        "ctrl_usetouchjoypad:0\r\n"
        "ctrl_swapjumpandsneak:0\r\n"
        "feedback_vibration:1\r\n"
        "ctrl_autojump:0\r\n"
        "ctrl_keyboardlayout:0\r\n"
        "ctrl_gamePadMap:[0,1,t:1,t:0,2,3,10,11,8,9,13,4,5,6,12,X,X,X,X]\r\n"
        "gfx_renderdistance_new:96\r\n"
        "gfx_viewbobbing:1\r\n"
        "gfx_fancygraphics:1\r\n"
        "gfx_fancyskies:1\r\n"
        "gfx_animatetextures:1\r\n"
        "gfx_hidegui:0\r\n"
        "gfx_field_of_view:70\r\n"
        "gfx_gamma:0\r\n"
        "gfx_fullscreen:0\r\n"
        "audio_sound:1\r\n"
        "audio_music:1\r\n"
        "dev_autoloadlevel:0\r\n"
        "dev_showchunkmap:0\r\n"
        "dev_disablefilesystem:0\r\n"
        "old_game_version_major:0\r\n"
        "old_game_version_minor:13\r\n"
        "old_game_version_patch:2\r\n"
        "old_game_version_beta:0\r\n";
    DWORD written;

    if (SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL,
                         SHGFP_TYPE_CURRENT, roaming) != S_OK) {
        log_line("SHGetFolderPathW(CSIDL_APPDATA) failed");
        return;
    }
    if (lstrlenW(roaming) + 64 >= MAX_PATH) {
        log_line("APPDATA path is too long");
        return;
    }
    lstrcpyW(game_dir, roaming);
    lstrcatW(game_dir, L"\\MinecraftPE");
    lstrcpyW(game_data_path, game_dir);
    CreateDirectoryW(game_dir, NULL);

    lstrcpyW(options_path, game_dir);
    lstrcatW(options_path, L"\\games");
    CreateDirectoryW(options_path, NULL);
    lstrcatW(options_path, L"\\com.mojang");
    CreateDirectoryW(options_path, NULL);
    lstrcatW(options_path, L"\\minecraftpe");
    CreateDirectoryW(options_path, NULL);
    lstrcatW(options_path, L"\\options.txt");
    file = CreateFileW(options_path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                       CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_EXISTS) {
            WIN32_FILE_ATTRIBUTE_DATA attributes;

            if (GetFileAttributesExW(
                    options_path, GetFileExInfoStandard, &attributes) &&
                (attributes.nFileSizeHigh || attributes.nFileSizeLow)) {
                log_line("roaming MinecraftPE options.txt already exists");
                return;
            }
            file = CreateFileW(
                options_path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file == INVALID_HANDLE_VALUE) {
                log_line("could not regenerate empty options.txt");
                return;
            }
            log_line("regenerating empty roaming MinecraftPE options.txt");
        } else {
            log_line("CreateFileW(options.txt) failed");
            return;
        }
    }
    WriteFile(file, defaults, (DWORD)(sizeof(defaults) - 1), &written, NULL);
    CloseHandle(file);
    log_line("created roaming MinecraftPE/games/com.mojang/minecraftpe/options.txt");
}

static void log_hresult(const char *operation, HRESULT result)
{
    char line[160];
    wsprintfA(line, "%s: HRESULT 0x%08lx", operation, (unsigned long)result);
    log_line(line);
}

static void log_pointer(const char *name, const void *value)
{
    char line[160];
    wsprintfA(line, "%s: %p", name, value);
    log_line(line);
}


static BOOL safe_read_pointer(const void *address, void **value)
{
    SIZE_T bytes_read = 0;
    *value = NULL;
    return ReadProcessMemory(
        GetCurrentProcess(), address, value, sizeof(*value), &bytes_read) &&
        bytes_read == sizeof(*value);
}

static void sync_0142_window_metrics(UINT width, UINT height)
{
    BYTE *metrics = *(BYTE **)(game_image + 0x0093f608);
    if (!metrics) return;
    *(float *)(metrics + 0x14) = (float)width;
    *(float *)(metrics + 0x18) = (float)height;
    *(float *)(metrics + 0x1c) = (float)width;
    *(float *)(metrics + 0x20) = (float)height;
    *(float *)(metrics + 0x24) = 1.0f;
    *(float *)(metrics + 0x28) = 1.0f;
}

static void __attribute__((thiscall)) win32_0142_refresh_targets(void *renderer)
{
    BYTE *value = renderer;
    D3D11_VIEWPORT viewport;
    ID3D11DeviceContext *context = *(ID3D11DeviceContext **)(value + 0x100);
    ID3D11RenderTargetView *target = *(ID3D11RenderTargetView **)(value + 0x108);
    ID3D11DepthStencilView *depth = *(ID3D11DepthStencilView **)(value + 0x110);
    UINT width = *(UINT *)(value + 0xd8);
    UINT height = *(UINT *)(value + 0xdc);
    if (!context || !target || !width || !height) return;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MaxDepth = 1.0f;
    CopyMemory(value + 0x118, &viewport, sizeof(viewport));
    sync_0142_window_metrics(width, height);
    ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
    ID3D11DeviceContext_OMSetRenderTargets(context, 1, &target, depth);
}


static BOOL install_0142_native_host_hooks(BYTE *image)
{
    static const BYTE poll_expected[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x0c};
    static const BYTE refresh_expected[] = {0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68};
    DWORD protection, ignored;
    BYTE *poll = image + 0x00295fd0;
    BYTE *refresh = image + 0x00591d00;
    BYTE *license_async = image + 0x004aa034;
    BYTE *xsts_online = image + 0x006a4938;
    BYTE *automation = image + 0x00038408;
    BYTE *automation_tick = image + 0x0026cb50;
    BYTE *xaml_dispatch = image + 0x00259d60;
    BYTE *login_screen = image + 0x0013b1f0;
    BYTE *xbox_lobby = image + 0x00243db4;
    static const BYTE license_expected[] = {0xc7, 0x45, 0xfc, 1, 0, 0, 0, 0xe8};
    if (memcmp(poll, poll_expected, sizeof(poll_expected)) ||
        memcmp(refresh, refresh_expected, sizeof(refresh_expected)) ||
        memcmp(license_async, license_expected, sizeof(license_expected)) ||
        xsts_online[0] != 0x74 || xsts_online[1] != 0x41 ||
        memcmp(automation, "\x8b\xd7\x8d\x4d\xec\xe8", 6) ||
        memcmp(automation_tick, "\x55\x8b\xec\x6a\xff\x68", 6) ||
        memcmp(xaml_dispatch, "\x55\x8b\xec\x6a\xff\x68", 6) ||
        memcmp(login_screen, "\x55\x8b\xec\x6a\xff\x68", 6) ||
        memcmp(xbox_lobby, "\x0f\x84\xe9\x01\x00\x00", 6)) return FALSE;
    if (!VirtualProtect(poll, 1, PAGE_EXECUTE_READWRITE, &protection)) return FALSE;
    *poll = 0xc3;
    VirtualProtect(poll, 1, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), poll, 1);
    if (!write_relative_jump(refresh, win32_0142_refresh_targets)) return FALSE;
    if (!write_relative_jump(license_async, image + 0x004aa10a)) return FALSE;
    /* Do not construct the UWP-only AutomationClient WebSocket service. */
    if (!write_relative_jump(automation, image + 0x0003846c)) return FALSE;
    /* Both native screen-factory methods take the same this pointer. */
    if (!write_relative_jump(login_screen, image + 0x0013aeb0)) return FALSE;
    /* Offline worlds use the existing no-Xbox-lobby branch; keep LAN setup. */
    if (!write_relative_jump(xbox_lobby, image + 0x00243fa3)) return FALSE;
    if (!VirtualProtect(automation_tick, 1, PAGE_EXECUTE_READWRITE, &protection)) return FALSE;
    *automation_tick = 0xc3;
    VirtualProtect(automation_tick, 1, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), automation_tick, 1);
    /* Offline UI does not need the XAML dispatcher (nine stack arguments). */
    if (!VirtualProtect(xaml_dispatch, 3, PAGE_EXECUTE_READWRITE, &protection)) return FALSE;
    memcpy(xaml_dispatch, "\xc2\x24\x00", 3);
    VirtualProtect(xaml_dispatch, 3, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), xaml_dispatch, 3);
    /* Use the game's cached/offline result instead of UWP XSTS authentication. */
    if (!VirtualProtect(xsts_online, 2, PAGE_EXECUTE_READWRITE, &protection)) return FALSE;
    xsts_online[0] = xsts_online[1] = 0x90;
    VirtualProtect(xsts_online, 2, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), xsts_online, 2);
    log_line("0.14.2 HWND input and render-target hooks installed");
    return TRUE;
}

static BOOL initialize_game_d3d(HWND window)
{
    BYTE *image = (BYTE *)GetModuleHandleW(NULL);
    GameMallocFn game_malloc =
        *(GameMallocFn *)(image + EARLY_RVA(0x0060f694, 0x00786758));
    GameOwnerCtorFn owner_ctor =
        (GameOwnerCtorFn)(image + EARLY_RVA(0x005366e0, 0x005980b0));
    GameInitDeviceFn init_device =
        (GameInitDeviceFn)(image + EARLY_RVA(0x005391d0, 0x005983a0));
    GameInitTargetsFn init_targets =
        (GameInitTargetsFn)(image + EARLY_RVA(0x00538f90, 0x00598140));
    void *owner;
    void *renderer;
    ID3D11Device *device;
    IDXGIDevice *dxgi_device = NULL;
    IDXGIAdapter *adapter = NULL;
    IDXGIFactory *factory = NULL;
    DXGI_SWAP_CHAIN_DESC description;
    RECT client;
    HRESULT result;
    struct { UINT width, height; BYTE reserved[24]; } display;

    log_line("calling game resource constructor");
    /*
     * FUN_00433240 wraps this construction in a UWP-era global holder that
     * blocks under Wine before returning.  Construct the identical 0x1c-byte
     * owner directly, then publish it to the original global.
     */
    log_pointer("game malloc import", (const void *)game_malloc);
    if (host_is_0142) {
        GameCreateDeviceResourcesFn create_metrics =
            (GameCreateDeviceResourcesFn)(image + 0x0003e0f0);
        create_metrics(NULL);
        GetClientRect(window, &client);
        sync_0142_window_metrics(client.right - client.left, client.bottom - client.top);
    }
    owner = game_malloc(OWNER_SIZE);
    log_pointer("game owner allocation", owner);
    if (owner) {
        ZeroMemory(owner, OWNER_SIZE);
        owner = owner_ctor(owner);
        *(void **)(image + EARLY_RVA(0x0078c6c4, 0x0093f5f8)) = owner;
    }
    log_line("game resource constructor returned");
    log_pointer("game resource owner", owner);
    if (!owner) {
        log_line("game resource owner creation returned null");
        return FALSE;
    }
    renderer = *(void **)((BYTE *)owner + OWNER_RENDERER_OFFSET);
    game_resource_owner = owner;
    game_renderer = renderer;
    log_pointer("game renderer", renderer);
    if (!renderer) {
        log_line("game renderer creation returned null");
        return FALSE;
    }
    log_line("calling game D3D11 device initializer");
    init_device(owner, renderer);
    log_line("game D3D11 device initializer returned");
    device = *(ID3D11Device **)((BYTE *)owner + OWNER_DEVICE_OFFSET);
    game_context = *(ID3D11DeviceContext **)((BYTE *)renderer + RENDERER_CONTEXT_OFFSET);

    if (host_is_windows7 && (!device || !game_context) &&
        captured_base_device && captured_base_context) {
        /* The UWP binary requested ID3D11Device2 and ID3D11DeviceContext2.
         * Windows 7 returns E_NOINTERFACE, but the base interfaces are fully
         * sufficient for the renderer paths used here. Transfer our retained
         * references into the exact fields that the failed QI left empty. */
        if (!device) {
            *(ID3D11Device **)((BYTE *)owner + OWNER_DEVICE_OFFSET) = captured_base_device;
            device = captured_base_device;
            captured_base_device = NULL;
            log_line("Windows 7 fallback: base ID3D11Device stored at owner+0x10");
        }
        if (!game_context) {
            *(ID3D11DeviceContext **)((BYTE *)renderer + RENDERER_CONTEXT_OFFSET) =
                captured_base_context;
            game_context = captured_base_context;
            captured_base_context = NULL;
            log_line("Windows 7 fallback: base ID3D11DeviceContext stored at renderer+0x84");
        }
    }

    /* A successful Device2 QI on newer Windows makes the retained base
     * references unnecessary. */
    if (captured_base_device) {
        ID3D11Device_Release(captured_base_device);
        captured_base_device = NULL;
    }
    if (captured_base_context) {
        ID3D11DeviceContext_Release(captured_base_context);
        captured_base_context = NULL;
    }

    log_pointer("game D3D11 device", device);
    log_pointer("game D3D11 context", game_context);
    if (!device || !game_context) {
        log_line("game D3D device/context is null after Windows 7 fallback");
        return FALSE;
    }
    install_scissor_fix(game_context);

    result = ID3D11Device_QueryInterface(
        device, &IID_IDXGIDevice, (void **)&dxgi_device);
    if (FAILED(result)) {
        log_hresult("ID3D11Device::QueryInterface(IDXGIDevice)", result);
        return FALSE;
    }
    result = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
    if (FAILED(result)) {
        log_hresult("IDXGIDevice::GetAdapter", result);
        goto fail;
    }
    result = IDXGIAdapter_GetParent(
        adapter, &IID_IDXGIFactory, (void **)&factory);
    if (FAILED(result)) {
        log_hresult("IDXGIAdapter::GetParent(IDXGIFactory)", result);
        goto fail;
    }

    GetClientRect(window, &client);
    ZeroMemory(&description, sizeof(description));
    description.BufferDesc.Width = (UINT)(client.right - client.left);
    description.BufferDesc.Height = (UINT)(client.bottom - client.top);
    description.BufferDesc.RefreshRate.Numerator = 0;
    description.BufferDesc.RefreshRate.Denominator = 0;
    description.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount = host_is_windows7 ? 1 : 2;
    description.OutputWindow = window;
    description.Windowed = TRUE;
    description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    log_line("creating HWND swap chain for game device");
    result = IDXGIFactory_CreateSwapChain(
        factory, (IUnknown *)device, &description, &game_swap_chain);
    log_hresult("IDXGIFactory::CreateSwapChain(HWND BGRA)", result);
    if (FAILED(result) && host_is_windows7) {
        description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        game_swap_chain = NULL;
        result = IDXGIFactory_CreateSwapChain(
            factory, (IUnknown *)device, &description, &game_swap_chain);
        log_hresult("IDXGIFactory::CreateSwapChain(HWND RGBA fallback)", result);
    }
    if (FAILED(result)) {
        goto fail;
    }

    ZeroMemory(&display, sizeof(display));
    display.width = description.BufferDesc.Width;
    display.height = description.BufferDesc.Height;
    *(IDXGISwapChain **)((BYTE *)&display + EARLY_RVA(0x14, 0x1c)) = game_swap_chain;
    if (host_is_0142) {
        CopyMemory((BYTE *)renderer + RENDERER_DISPLAY_OFFSET, &display, 0x1c);
    }
    log_line("calling game render-target initializer");
    init_targets(owner, &display, renderer);
    log_line("game render-target initializer returned");
    if (*(IDXGISwapChain **)((BYTE *)renderer + RENDERER_SWAP_CHAIN_OFFSET)) {
        IDXGISwapChain_Release(
            *(IDXGISwapChain **)((BYTE *)renderer + RENDERER_SWAP_CHAIN_OFFSET));
    }
    IDXGISwapChain_AddRef(game_swap_chain);
    *(IDXGISwapChain **)((BYTE *)renderer + RENDERER_SWAP_CHAIN_OFFSET) = game_swap_chain;
    log_line("HWND swap chain attached to game renderer");
    game_target =
        *(ID3D11RenderTargetView **)((BYTE *)renderer + RENDERER_RTV_OFFSET);
    if (!game_target) {
        ID3D11Texture2D *backbuffer = NULL;
        log_line("game render target creation returned null; trying direct RTV fallback");
        result = IDXGISwapChain_GetBuffer(
            game_swap_chain, 0, &IID_ID3D11Texture2D, (void **)&backbuffer);
        log_hresult("IDXGISwapChain::GetBuffer direct fallback", result);
        if (SUCCEEDED(result) && backbuffer) {
            result = ID3D11Device_CreateRenderTargetView(
                device, backbuffer, NULL, &game_target);
            ID3D11Texture2D_Release(backbuffer);
            log_hresult("ID3D11Device::CreateRenderTargetView direct fallback", result);
            if (SUCCEEDED(result) && game_target) {
                *(ID3D11RenderTargetView **)((BYTE *)renderer + RENDERER_RTV_OFFSET) = game_target;
            }
        }
        if (!game_target) {
            goto fail;
        }
    }
    ID3D11DeviceContext_AddRef(game_context);
    ID3D11RenderTargetView_AddRef(game_target);
    if (host_is_0142) win32_0142_refresh_targets(renderer);
    install_swap_chain_present_hook(game_swap_chain);
    {
        const float startup_color[4] = {0.055f, 0.085f, 0.12f, 1.0f};
        HRESULT startup_present;
        ID3D11DeviceContext_OMSetRenderTargets(
            game_context, 1, &game_target, NULL);
        ID3D11DeviceContext_ClearRenderTargetView(
            game_context, game_target, startup_color);
        startup_present = IDXGISwapChain_Present(game_swap_chain, 0, 0);
        log_hresult("initial HWND backbuffer clear/present", startup_present);
    }
    log_line("game D3D11 device and HWND swap chain initialized");

    if (factory) IDXGIFactory_Release(factory);
    if (adapter) IDXGIAdapter_Release(adapter);
    if (dxgi_device) IDXGIDevice_Release(dxgi_device);
    return TRUE;

fail:
    if (game_swap_chain) {
        IDXGISwapChain_Release(game_swap_chain);
        game_swap_chain = NULL;
    }
    if (factory) IDXGIFactory_Release(factory);
    if (adapter) IDXGIAdapter_Release(adapter);
    if (dxgi_device) IDXGIDevice_Release(dxgi_device);
    return FALSE;
}

static BOOL initialize_game_app(BYTE *image)
{
    GameCreateDeviceResourcesFn create_device_resources =
        (GameCreateDeviceResourcesFn)(image + EARLY_RVA(0x003ea440, 0x004e18f0));
    GameCreateAppMainFn create_app_main =
        (GameCreateAppMainFn)(image + EARLY_RVA(0x003ea3b0, 0x004e1860));
    void *device_resources;
    void *holder_result;

    log_line("calling DX::DeviceResources constructor");
    device_resources = create_device_resources(NULL);
    game_device_resources = device_resources;
    log_pointer("DX::DeviceResources", device_resources);
    if (!device_resources) {
        log_line("DX::DeviceResources construction returned null");
        return FALSE;
    }

    game_app_main = NULL;
    log_line("calling MCPE_Host::AppMain constructor");
    holder_result = create_app_main(&game_app_main);
    log_pointer("AppMain holder result", holder_result);
    log_pointer("MCPE_Host::AppMain", game_app_main);
    if (!game_app_main) {
        log_line("MCPE_Host::AppMain construction returned null");
        return FALSE;
    }
    log_pointer("AppMain platform", *(void **)((BYTE *)game_app_main + 4));
    log_pointer("AppMain game", *(void **)((BYTE *)game_app_main + 8));
    log_pointer("AppMain services", *(void **)((BYTE *)game_app_main + 12));
    log_pointer("AppMain frame enabled",
                (void *)(ULONG_PTR)*(BYTE *)((BYTE *)game_app_main + 0x10));
    log_line("MCPE_Host::AppMain constructed");
    return TRUE;
}

static BOOL activate_game_app(HWND window)
{
    void *platform;
    void *game;
    BYTE *app_platform;
    BYTE *platform_state;
    void *sentinel;
    void *node;
    RECT client;
    unsigned count = 0;
    char line[96];

    if (!game_app_main) return FALSE;
    platform = *(void **)((BYTE *)game_app_main + 4);
    game = *(void **)((BYTE *)game_app_main + 8);
    if (!platform || !game || IsBadReadPtr(game, sizeof(void *))) {
        log_line("cannot activate AppMain: platform/game unavailable");
        return FALSE;
    }

    app_platform = *(BYTE **)(game_image + EARLY_RVA(0x007e0ee0, 0x009b33f8));
    platform_state = app_platform
        ? *(BYTE **)(app_platform + PLATFORM_HID_OFFSET) : NULL;
    log_pointer("AppPlatform", app_platform);
    log_pointer("AppPlatform lifecycle state", platform_state);
    if (app_platform && platform_state &&
        !IsBadReadPtr(*(void ***)app_platform, 0x2c)) {
        GameLifecycleFn app_activate =
            (GameLifecycleFn)(*(void ***)app_platform)[0x28 / sizeof(void *)];
        char line[96];
        wsprintfA(line, "AppPlatform state before activation: %ld",
                  (long)*(int *)(platform_state + 0x48));
        log_line(line);
        app_activate(app_platform);
        wsprintfA(line, "AppPlatform state after activation: %ld",
                  (long)*(int *)(platform_state + 0x48));
        log_line(line);
    }

    GetClientRect(window, &client);
    if (!IsBadReadPtr(*(void ***)game, 0x50)) {
        GameResizeFn resize =
            (GameResizeFn)(*(void ***)game)[0x4c / sizeof(void *)];
        if (resize) {
            resize(game, client.right - client.left,
                   client.bottom - client.top, 0);
            log_line("MinecraftClient Win32 size initialized");
        }
    }
    *(int *)((BYTE *)platform + PLATFORM_WIDTH_OFFSET) = client.right - client.left;
    *(int *)((BYTE *)platform + PLATFORM_HEIGHT_OFFSET) = client.bottom - client.top;
    if (host_is_0142) {
        typedef void (__attribute__((thiscall)) *GamePixelResizeFn)(void *, int, int);
        GamePixelResizeFn resize_pixels = (GamePixelResizeFn)(game_image + 0x000331a0);
        resize_pixels(game, client.right - client.left, client.bottom - client.top);
        log_line("0.14.2 pixel metrics and compositor stages initialized");
    }

    sentinel = *(void **)((BYTE *)platform + 0x100);
    if (sentinel && !IsBadReadPtr(sentinel, sizeof(void *))) {
        node = *(void **)sentinel;
        while (node && node != sentinel && count < 64) {
            void *listener = *((void **)node + 5);
            void **vtable = listener ? *(void ***)listener : NULL;
            if (vtable && !IsBadReadPtr(vtable, 0x18) &&
                vtable[0x14 / sizeof(void *)]) {
                GameLifecycleFn start =
                    (GameLifecycleFn)vtable[0x14 / sizeof(void *)];
                log_pointer("AppMain listener", listener);
                log_pointer("AppMain listener vtable", vtable);
                log_pointer("AppMain listener start", (const void *)start);
                start(listener);
                ++count;
            }
            if (IsBadReadPtr(node, sizeof(void *))) break;
            node = *(void **)node;
        }
    }
    *(BYTE *)((BYTE *)game_app_main + 0x10) = TRUE;
    wsprintfA(line, "AppMain activated; %u listener(s) started", count);
    log_line(line);
    return TRUE;
}

static void render_game_target(void)
{
    static float phase;
    float color[4] = {0.055f, 0.085f, 0.12f, 1.0f};
    GameUpdateRenderFn update_render;

    if (!game_context || !game_target || !game_swap_chain) {
        return;
    }
    if (game_app_main && game_renderer &&
        *(BYTE *)((BYTE *)game_app_main + 0x10)) {
        BYTE *game = *(BYTE **)((BYTE *)game_app_main + 8);

        /*
         * UWP's CompositionTarget::Rendering event raises this flag before
         * MinecraftClient's frame method.  The HWND message loop replaces
         * that event source, so raise the same flag for each Win32 frame.
         */
        if (game) {
            *(BYTE *)(game + CLIENT_FRAME_READY_OFFSET) = TRUE;
        }
        if (host_is_0142) win32_0142_refresh_targets(game_renderer);
        LONG presents_before;
        LONG presents_after;
        HRESULT fallback_result;
        static unsigned fallback_present_count;

        update_render =
            (GameUpdateRenderFn)(game_image + EARLY_RVA(0x003bd630, 0x004b5050));
        /*
         * The UWP build has an independent CompositionTarget callback that
         * drains a XAML task queue.  Calling that callback from the HWND loop
         * corrupts the UI material commands.  AppMain::Update owns the real
         * game tick and D3D render path and is the only per-frame callback
         * required by this Win32 host.
         *
         * On Windows 7 the UWP frame path can render into the HWND backbuffer
         * without reaching its original CompositionTarget-driven Present.
         * Observe the real swap-chain vtable: only present manually when the
         * game did not successfully present during this update.
         */
        presents_before = game_present_success_count;
        update_render(game_app_main);
        presents_after = game_present_success_count;
        if (presents_after == presents_before) {
                fallback_result = IDXGISwapChain_Present(
                game_swap_chain, host_is_windows7 ? 0 : 1, 0);
            ++fallback_present_count;
            if (fallback_present_count <= 10 ||
                fallback_present_count == 60 || FAILED(fallback_result)) {
                char line[160];
                wsprintfA(line,
                    "host fallback Present %u: HRESULT 0x%08lX",
                    fallback_present_count, (unsigned long)fallback_result);
                log_line(line);
            }
        }
        return;
    }
    phase += 0.0025f;
    if (phase > 0.06f) {
        phase = 0.0f;
    }
    color[2] += phase;
    ID3D11DeviceContext_OMSetRenderTargets(
        game_context, 1, &game_target, NULL);
    ID3D11DeviceContext_ClearRenderTargetView(
        game_context, game_target, color);
    IDXGISwapChain_Present(game_swap_chain, 1, 0);
}


static BYTE *get_hid_controller(void *platform)
{
    if (!platform || IsBadReadPtr((BYTE *)platform + PLATFORM_HID_OFFSET, sizeof(void *))) {
        return NULL;
    }
    return *(BYTE **)((BYTE *)platform + PLATFORM_HID_OFFSET);
}

/*
 * These are the three AppPlatform_Winrt mouse-mode entry points used by
 * MinecraftClient.  The stock UWP implementation stores a one-frame command
 * at HIDController+0x48 and immediately consumes it while trying to talk to
 * CoreWindow.  An HWND host therefore cannot reliably observe the request by
 * polling after the frame.  Replace the tiny entry points and retain the
 * requested state in the DLL instead.
 */
static void __attribute__((thiscall)) win32_request_relative_mouse(
    void *platform)
{
    BYTE *controller = get_hid_controller(platform);
    if (initial_menu_cursor_guard) {
        /* App activation emits one relative-mode request before the first
         * main-menu screen owns the pointer.  Ignore only that startup phase;
         * the first real menu click or an explicit absolute request removes
         * the guard, so entering a world can capture normally. */
        if (controller) *(int *)(controller + 0x48) = 0;
        host_relative_requested = FALSE;
        mouse_clip_dirty = TRUE;
        return;
    }
    if (controller && controller[0x6c] == 0) {
        *(int *)(controller + 0x48) = 1;
    }
    host_relative_requested = TRUE;
    mouse_clip_dirty = TRUE;
}

static void __attribute__((thiscall)) win32_request_absolute_mouse(
    void *platform)
{
    BYTE *controller = get_hid_controller(platform);
    if (controller) {
        *(int *)(controller + 0x48) = 2;
    }
    initial_menu_cursor_guard = FALSE;
    host_relative_requested = FALSE;
    mouse_clip_dirty = TRUE;
}

static void __attribute__((thiscall)) win32_toggle_mouse_mode(
    void *platform)
{
    BYTE *controller = get_hid_controller(platform);
    if (!controller) return;
    if (initial_menu_cursor_guard) {
        *(int *)(controller + 0x48) = 0;
        host_relative_requested = FALSE;
        mouse_clip_dirty = TRUE;
        return;
    }
    controller[0x6c] = controller[0x6c] == 0;
    *(int *)(controller + 0x48) = controller[0x6c] ? 2 : 1;
    host_relative_requested = controller[0x6c] == 0;
    mouse_clip_dirty = TRUE;
}

static BOOL install_mouse_mode_hooks(BYTE *image)
{
    static const BYTE relative_expected[10] = {
        0x8b, 0x81, 0x3c, 0x01, 0x00, 0x00, 0x80, 0x78, 0x6c, 0x00
    };
    static const BYTE absolute_expected[10] = {
        0x8b, 0x81, 0x3c, 0x01, 0x00, 0x00, 0xc7, 0x40, 0x48, 0x02
    };
    static const BYTE toggle_expected[10] = {
        0x8b, 0x91, 0x3c, 0x01, 0x00, 0x00, 0x80, 0x7a, 0x6c, 0x00
    };
    BYTE *relative_site = image + EARLY_RVA(0x003c0c90, 0x004ba470);
    BYTE *absolute_site = image + EARLY_RVA(0x003c0cb0, 0x004ba490);
    BYTE *toggle_site = image + EARLY_RVA(0x003c0cc0, 0x004ba4a0);

    BYTE relative_signature[10], absolute_signature[10], toggle_signature[10];
    CopyMemory(relative_signature, relative_expected, sizeof(relative_signature));
    CopyMemory(absolute_signature, absolute_expected, sizeof(absolute_signature));
    CopyMemory(toggle_signature, toggle_expected, sizeof(toggle_signature));
    *(DWORD *)(relative_signature + 2) = PLATFORM_HID_OFFSET;
    *(DWORD *)(absolute_signature + 2) = PLATFORM_HID_OFFSET;
    *(DWORD *)(toggle_signature + 2) = PLATFORM_HID_OFFSET;
    if (memcmp(relative_site, relative_signature, sizeof(relative_signature)) ||
        memcmp(absolute_site, absolute_signature, sizeof(absolute_signature)) ||
        memcmp(toggle_site, toggle_signature, sizeof(toggle_signature))) {
        log_line("mouse-mode hook signature mismatch");
        return FALSE;
    }
    if (!write_relative_jump(relative_site, win32_request_relative_mouse) ||
        !write_relative_jump(absolute_site, win32_request_absolute_mouse) ||
        !write_relative_jump(toggle_site, win32_toggle_mouse_mode)) {
        log_line("mouse-mode hook installation failed");
        return FALSE;
    }
    log_line("installed persistent HWND relative-mouse hooks");
    return TRUE;
}

static BOOL game_requests_relative_mouse(void)
{
    static int previous_requested = -1;
    static int previous_actual = -1;
    static int previous_command = -1;
    void *platform;
    BYTE *controller;
    int actual = 0;
    int command = 0;
    char line[128];

    if (!game_app_main) return host_relative_requested;
    platform = *(void **)((BYTE *)game_app_main + 4);
    controller = get_hid_controller(platform);
    if (controller) {
        actual = controller[0x45] != 0;
        command = *(int *)(controller + 0x48);
        /*
         * Do not infer the requested mode from +0x48 here.  That field is a
         * transient command slot and is initialized/stale as 1 in menu paths.
         * The three direct hooks above are the authoritative state source.
         */
    }
    if ((int)host_relative_requested != previous_requested ||
        actual != previous_actual || command != previous_command) {
        wsprintfA(line,
                  "HID mouse mode: requested=%d actual=%d command=%d",
                  host_relative_requested, actual, command);
        log_line(line);
        previous_requested = host_relative_requested;
        previous_actual = actual;
        previous_command = command;
    }
    return host_relative_requested;
}

static void center_mouse_cursor(HWND window)
{
    RECT client;
    POINT center;

    if (!GetClientRect(window, &client)) return;
    center.x = (client.right - client.left) / 2;
    center.y = (client.bottom - client.top) / 2;
    if (ClientToScreen(window, &center)) {
        SetCursorPos(center.x, center.y);
    }
}

static void set_game_relative_actual(BOOL active)
{
    void *platform;
    BYTE *controller;

    if (!game_app_main) return;
    platform = *(void **)((BYTE *)game_app_main + 4);
    controller = get_hid_controller(platform);
    if (controller) {
        /* HIDController::update only consumes relative deltas while +0x45=1. */
        controller[0x45] = active ? 1 : 0;
        *(int *)(controller + 0x48) = 0;
    }
}

static BOOL apply_mouse_clip(HWND window, BOOL recenter)
{
    RECT clip;
    POINT top_left;
    POINT bottom_right;

    if (!GetClientRect(window, &clip)) return FALSE;
    top_left.x = clip.left;
    top_left.y = clip.top;
    bottom_right.x = clip.right;
    bottom_right.y = clip.bottom;
    if (!ClientToScreen(window, &top_left) ||
        !ClientToScreen(window, &bottom_right)) {
        return FALSE;
    }
    clip.left = top_left.x;
    clip.top = top_left.y;
    clip.right = bottom_right.x;
    clip.bottom = bottom_right.y;
    if (!ClipCursor(&clip)) return FALSE;
    if (recenter) center_mouse_cursor(window);
    mouse_clip_dirty = FALSE;
    return TRUE;
}

static void update_mouse_capture(HWND window)
{
    BOOL requested = game_requests_relative_mouse();
    BOOL focused = GetForegroundWindow() == window;
    BOOL enable = requested && focused && !IsIconic(window);

    if (enable) {
        if (!mouse_capture_active) {
            mouse_capture_active = TRUE;
            SetCapture(window);
            while (ShowCursor(FALSE) >= 0) {}
            set_game_relative_actual(TRUE);
            mouse_clip_dirty = TRUE;
            apply_mouse_clip(window, TRUE);
            log_line("relative mouse captured");
        } else {
            set_game_relative_actual(TRUE);
            if (mouse_clip_dirty || GetCapture() != window) {
                SetCapture(window);
                apply_mouse_clip(window, TRUE);
                log_line("relative mouse clip refreshed");
            }
        }
        return;
    }

    if (mouse_capture_active) {
        mouse_capture_active = FALSE;
        set_game_relative_actual(FALSE);
        ClipCursor(NULL);
        if (GetCapture() == window) ReleaseCapture();
        while (ShowCursor(TRUE) < 0) {}
        SetCursor(LoadCursorW(NULL, MAKEINTRESOURCEW(32512)));
        log_line("relative mouse released");
    }
}


static BOOL append_wide_path(wchar_t *buffer, LPCWSTR suffix)
{
    int current = lstrlenW(buffer);
    int extra = lstrlenW(suffix);
    if (current < 0 || extra < 0 || current + extra >= MAX_PATH) return FALSE;
    lstrcatW(buffer, suffix);
    return TRUE;
}

static BOOL copy_valid_png(LPCWSTR source, LPCWSTR destination)
{
    static const BYTE png_signature[8] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a
    };
    BYTE buffer[1024];
    HANDLE input;
    HANDLE output;
    DWORD read_count;
    DWORD written;
    BOOL ok = FALSE;

    input = CreateFileW(source, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (input == INVALID_HANDLE_VALUE) return FALSE;
    if (!ReadFile(input, buffer, sizeof(png_signature), &read_count, NULL) ||
        read_count != sizeof(png_signature) ||
        memcmp(buffer, png_signature, sizeof(png_signature)) != 0) {
        CloseHandle(input);
        log_line("custom skin selection is not a PNG file");
        return FALSE;
    }

    output = CreateFileW(destination, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (output == INVALID_HANDLE_VALUE) {
        CloseHandle(input);
        return FALSE;
    }
    if (!WriteFile(output, buffer, read_count, &written, NULL) ||
        written != read_count) {
        goto done;
    }
    for (;;) {
        if (!ReadFile(input, buffer, sizeof(buffer), &read_count, NULL)) {
            goto done;
        }
        if (!read_count) {
            ok = TRUE;
            break;
        }
        if (!WriteFile(output, buffer, read_count, &written, NULL) ||
            written != read_count) {
            goto done;
        }
    }

done:
    CloseHandle(output);
    CloseHandle(input);
    return ok;
}

static BOOL line_has_key(const BYTE *line, DWORD length, const char *key)
{
    DWORD index = 0;
    while (key[index]) {
        if (index >= length || line[index] != (BYTE)key[index]) return FALSE;
        ++index;
    }
    return index < length && line[index] == ':';
}

static void copy_bytes(BYTE *destination, DWORD *position,
                       const BYTE *source, DWORD count)
{
    if (count) CopyMemory(destination + *position, source, count);
    *position += count;
}

static BOOL update_custom_skin_options(void)
{
    static const char type_key[] = "game_skintypefull";
    static const char last_key[] = "game_lastcustomskinnew";
    static const BYTE type_line[] =
        "game_skintypefull:Standard_Custom\r\n";
    static const BYTE last_line[] =
        "game_lastcustomskinnew:custom.png\r\n";
    wchar_t path[MAX_PATH];
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    HANDLE file;
    BYTE *input = NULL;
    BYTE *output = NULL;
    DWORD size = 0;
    DWORD read_count = 0;
    DWORD output_position = 0;
    DWORD position = 0;
    DWORD written = 0;
    BOOL wrote_type = FALSE;
    BOOL wrote_last = FALSE;
    BOOL ok = FALSE;

    lstrcpyW(path, game_data_path);
    if (!append_wide_path(path,
            L"\\games\\com.mojang\\minecraftpe\\options.txt")) {
        return FALSE;
    }
    if (GetFileAttributesExW(path, GetFileExInfoStandard, &attributes)) {
        size = attributes.nFileSizeLow;
    }
    if (size > 1024 * 1024) return FALSE;

    input = (BYTE *)HeapAlloc(GetProcessHeap(), 0, size + 1);
    output = (BYTE *)HeapAlloc(GetProcessHeap(), 0, size + 512);
    if (!input || !output) goto done;
    if (size) {
        file = CreateFileW(path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE) goto done;
        if (!ReadFile(file, input, size, &read_count, NULL)) {
            CloseHandle(file);
            goto done;
        }
        CloseHandle(file);
        size = read_count;
    }

    while (position < size) {
        DWORD line_start = position;
        DWORD line_end;
        while (position < size && input[position] != '\n') ++position;
        if (position < size) ++position;
        line_end = position;
        if (line_has_key(input + line_start, line_end - line_start, type_key)) {
            if (!wrote_type) {
                copy_bytes(output, &output_position, type_line,
                           sizeof(type_line) - 1);
                wrote_type = TRUE;
            }
        } else if (line_has_key(input + line_start,
                                line_end - line_start, last_key)) {
            if (!wrote_last) {
                copy_bytes(output, &output_position, last_line,
                           sizeof(last_line) - 1);
                wrote_last = TRUE;
            }
        } else {
            copy_bytes(output, &output_position, input + line_start,
                       line_end - line_start);
        }
    }
    if (output_position && output[output_position - 1] != '\n') {
        static const BYTE newline[] = "\r\n";
        copy_bytes(output, &output_position, newline, 2);
    }
    if (!wrote_type) {
        copy_bytes(output, &output_position, type_line, sizeof(type_line) - 1);
    }
    if (!wrote_last) {
        copy_bytes(output, &output_position, last_line, sizeof(last_line) - 1);
    }

    file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) goto done;
    ok = WriteFile(file, output, output_position, &written, NULL) &&
         written == output_position;
    CloseHandle(file);

done:
    if (input) HeapFree(GetProcessHeap(), 0, input);
    if (output) HeapFree(GetProcessHeap(), 0, output);
    return ok;
}

static void invoke_custom_skin_callback(void *callback)
{
    void **vtable;
    void (__attribute__((thiscall)) *invoke)(void *);

    if (!callback) return;
    vtable = *(void ***)callback;
    if (!vtable || !vtable[2]) return;
    invoke = (void (__attribute__((thiscall)) *)(void *))vtable[2];
    invoke(callback);
}

/* Replace AppPlatform::pickFile itself rather than its WinRT UI lambda.
 * The original stores the completion object at platform+0x1B0 and invokes
 * callback->vtable[2]() after selection or cancellation. */
static void __attribute__((thiscall)) win32_pick_custom_skin(
    void *platform, void *callback)
{
    wchar_t selected[MAX_PATH];
    wchar_t primary[MAX_PATH];
    wchar_t fallback[MAX_PATH];
    OPENFILENAMEW dialog;
    BOOL copied_primary = FALSE;
    BOOL copied_fallback = FALSE;

    if (platform) {
        *(void **)((BYTE *)platform + 0x1b0) = callback;
    }
    ZeroMemory(selected, sizeof(selected));
    ZeroMemory(&dialog, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = game_window;
    dialog.lpstrFilter = L"PNG skin (*.png)\0*.png\0All files (*.*)\0*.*\0\0";
    dialog.lpstrFile = selected;
    dialog.nMaxFile = ARRAYSIZE(selected);
    dialog.lpstrTitle = L"Choose a Minecraft skin PNG";
    dialog.lpstrDefExt = L"png";
    dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST |
                   OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

    host_relative_requested = FALSE;
    mouse_clip_dirty = TRUE;
    update_mouse_capture(game_window);
    if (!GetOpenFileNameW(&dialog)) {
        log_line("Win32 custom-skin picker cancelled");
        invoke_custom_skin_callback(callback);
        return;
    }

    lstrcpyW(primary, game_data_path);
    lstrcpyW(fallback, game_data_path);
    if (!append_wide_path(primary,
            L"\\games\\com.mojang\\minecraftpe\\custom.png") ||
        !append_wide_path(fallback, L"\\custom.png")) {
        log_line("custom skin destination path too long");
        invoke_custom_skin_callback(callback);
        return;
    }
    copied_primary = copy_valid_png(selected, primary);
    copied_fallback = copy_valid_png(selected, fallback);
    if (!copied_primary && !copied_fallback) {
        log_line("failed to copy selected custom skin PNG");
        invoke_custom_skin_callback(callback);
        return;
    }
    if (!update_custom_skin_options()) {
        log_line("custom skin copied, but options update failed");
        invoke_custom_skin_callback(callback);
        return;
    }
    log_line("Win32 custom skin PNG installed as custom.png");
    invoke_custom_skin_callback(callback);
}

static void __attribute__((thiscall)) win32_set_fullscreen_mode(
    void *platform, int mode)
{
    MONITORINFO monitor;
    HMONITOR handle;
    LONG style;

    (void)platform;
    if (!game_window) return;
    if (mode == 1 && !win32_fullscreen_active) {
        saved_window_placement.length = sizeof(saved_window_placement);
        if (!GetWindowPlacement(game_window, &saved_window_placement)) return;
        saved_window_style = GetWindowLongW(game_window, GWL_STYLE);
        saved_window_exstyle = GetWindowLongW(game_window, GWL_EXSTYLE);
        handle = MonitorFromWindow(game_window, MONITOR_DEFAULTTONEAREST);
        ZeroMemory(&monitor, sizeof(monitor));
        monitor.cbSize = sizeof(monitor);
        if (!handle || !GetMonitorInfoW(handle, &monitor)) return;
        style = (saved_window_style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP;
        SetWindowLongW(game_window, GWL_STYLE, style);
        SetWindowPos(game_window, NULL,
                     monitor.rcMonitor.left, monitor.rcMonitor.top,
                     monitor.rcMonitor.right - monitor.rcMonitor.left,
                     monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED |
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        win32_fullscreen_active = TRUE;
        mouse_clip_dirty = TRUE;
        log_line("entered Win32 borderless fullscreen");
    } else if (mode == 0 && win32_fullscreen_active) {
        SetWindowLongW(game_window, GWL_STYLE, saved_window_style);
        SetWindowLongW(game_window, GWL_EXSTYLE, saved_window_exstyle);
        SetWindowPlacement(game_window, &saved_window_placement);
        SetWindowPos(game_window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        win32_fullscreen_active = FALSE;
        mouse_clip_dirty = TRUE;
        log_line("left Win32 borderless fullscreen");
    }
    update_mouse_capture(game_window);
}



typedef HINSTANCE (WINAPI *ShellExecuteWFn)(
    HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, int);

static void win32_open_url(LPCWSTR url)
{
    HMODULE shell32;
    ShellExecuteWFn shell_execute;
    HINSTANCE result;

    shell32 = LoadLibraryW(L"shell32.dll");
    if (!shell32) {
        log_line("Help URL failed: shell32.dll unavailable");
        return;
    }
    shell_execute = (ShellExecuteWFn)GetProcAddress(shell32, "ShellExecuteW");
    if (!shell_execute) {
        log_line("Help URL failed: ShellExecuteW unavailable");
        return;
    }
    result = shell_execute(game_window, L"open", url,
                           NULL, NULL, 1);
    if ((UINT_PTR)result <= 32)
        log_line("Help URL ShellExecuteW failed");
    else
        log_line("Help URL opened through Win32 ShellExecuteW");
}

static void __attribute__((thiscall)) win32_open_help_url(void *self)
{
    (void)self;
    win32_open_url(L"http://aka.ms/minecraftfb");
}

/* 0.14.2's platform slot accepts a const std::string&, unlike 0.13.2. */
static void __attribute__((thiscall)) win32_launch_url_0142(void *self, const BYTE *text)
{
    static wchar_t url[2048];
    const char *data;
    DWORD length, capacity;
    int count;
    (void)self;
    if (!text || IsBadReadPtr(text, 24)) return;
    length = *(const DWORD *)(text + 16);
    capacity = *(const DWORD *)(text + 20);
    data = capacity < 16 ? (const char *)text : *(const char *const *)text;
    if (!length || length > 8192 || IsBadReadPtr(data, length)) return;
    count = MultiByteToWideChar(CP_UTF8, 0, data, length, url, ARRAYSIZE(url) - 1);
    if (count <= 0) return;
    url[count] = 0;
    win32_open_url(url);
}

static HRESULT WINAPI win32_show_achievements_noop(
    void *user, void *callback, void *state)
{
    (void)user; (void)callback; (void)state;
    log_line("Achievements ignored by Win32 host");
    return E_NOTIMPL;
}

static HRESULT WINAPI win32_show_game_invite_noop(
    void *service_configuration, void *session_template,
    void *session_id, void *invitation_text,
    void *callback, void *state)
{
    (void)service_configuration; (void)session_template;
    (void)session_id; (void)invitation_text;
    (void)callback; (void)state;
    log_line("Invite Player ignored by Win32 host");
    return E_NOTIMPL;
}


/*
 * UWP edit boxes call AppPlatform_Winrt::showKeyboard/hideKeyboard before
 * receiving CharacterReceived events.  In an HWND process the original
 * implementation tries to obtain Windows.UI.ViewManagement.InputPane and
 * crashes because there is no CoreWindow view.  Keep Minecraft's own
 * "keyboard visible" byte in sync, but replace only that WinRT boundary.
 */
static void __attribute__((thiscall)) win32_show_text_keyboard(
    void *self, void *text, int max_length, int multiline,
    int secure, const void *position)
{
    (void)text;
    (void)max_length;
    (void)multiline;
    (void)secure;
    (void)position;
    if (self) *((BYTE *)self + 9) = TRUE;
    text_input_active = TRUE;
    pending_high_surrogate = 0;
    log_line("Win32 text input activated (InputPane show bypassed)");
}

static void __attribute__((thiscall)) win32_hide_text_keyboard(void *self)
{
    if (self) *((BYTE *)self + 9) = FALSE;
    text_input_active = FALSE;
    pending_high_surrogate = 0;
    suppress_next_t_character = FALSE;
    log_line("Win32 text input deactivated (InputPane hide bypassed)");
}

extern BOOL Win32WorldFileDialogEarly2016(HWND, void *, BOOL);

static void __attribute__((thiscall)) win32_world_file_picker_0142(
    void *platform, BYTE *settings)
{
    DWORD mode;
    (void)platform;
    if (!settings) return;
    mode = *(DWORD *)(settings + 0x88);
    if (mode != 1 && mode != 2) return;
    host_relative_requested = FALSE;
    mouse_clip_dirty = TRUE;
    update_mouse_capture(game_window);
    if (Win32WorldFileDialogEarly2016(game_window, settings, mode == 2))
        log_line("0.14.2 world file selection callback invoked");
}

static BOOL install_win32_platform_fixes(BYTE *image)
{
    static const BYTE picker_expected[10] = {
        0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0x88, 0xcf, 0x9c, 0x00
    };
    static const BYTE fullscreen_expected[10] = {
        0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0xdf, 0xd3, 0x9c, 0x00
    };
    static const BYTE show_keyboard_expected[10] = {
        0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0x25, 0xcd, 0x9c, 0x00
    };
    static const BYTE hide_keyboard_expected[10] = {
        0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0x71, 0xcd, 0x9c, 0x00
    };
    static const BYTE help_expected[10] = {
        0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0xf8, 0xb7, 0x98, 0x00
    };
    void **achievements_slot = (void **)(image + EARLY_RVA(0x0060f870, 0x00786960));
    void **invite_slot = (void **)(image + 0x0060f878);
    DWORD old_protection;
    DWORD ignored;
    SIZE_T entry_signature_size = host_is_0142 ? 6 : 10;
    if (host_is_0142) {
        static const BYTE world_picker_expected[] = {0x55, 0x8b, 0xec, 0x57, 0x8b, 0xf9};
        if (memcmp(image + 0x004bbbe0, world_picker_expected, sizeof(world_picker_expected)) ||
            !write_relative_jump(image + 0x004bbbe0, win32_world_file_picker_0142)) return FALSE;
    }

    if (memcmp(image + EARLY_RVA(0x003c14d0, 0x004baba0), picker_expected,
               entry_signature_size) != 0 ||
        !write_relative_jump(image + EARLY_RVA(0x003c14d0, 0x004baba0),
                             win32_pick_custom_skin)) {
        log_line("Win32 custom-skin hook installation failed");
        return FALSE;
    }
    if (memcmp(image + EARLY_RVA(0x003c26e0, 0x004bbcc0), fullscreen_expected,
               entry_signature_size) != 0 ||
        !write_relative_jump(image + EARLY_RVA(0x003c26e0, 0x004bbcc0),
                             win32_set_fullscreen_mode)) {
        log_line("Win32 fullscreen hook installation failed");
        return FALSE;
    }
    if (memcmp(image + EARLY_RVA(0x003c0990, 0x004ba170), show_keyboard_expected,
               entry_signature_size) != 0 ||
        !write_relative_jump(image + EARLY_RVA(0x003c0990, 0x004ba170),
                             win32_show_text_keyboard)) {
        log_line("Win32 text keyboard show hook installation failed");
        return FALSE;
    }
    if (memcmp(image + EARLY_RVA(0x003c0b00, 0x004ba2e0), hide_keyboard_expected,
               entry_signature_size) != 0 ||
        !write_relative_jump(image + EARLY_RVA(0x003c0b00, 0x004ba2e0),
                             win32_hide_text_keyboard)) {
        log_line("Win32 text keyboard hide hook installation failed");
        return FALSE;
    }
    if (memcmp(image + EARLY_RVA(0x000c3cf0, 0x004b9e80), help_expected,
               entry_signature_size) != 0 ||
        !write_relative_jump(image + EARLY_RVA(0x000c3cf0, 0x004b9e80),
                             host_is_0142 ? (void *)win32_launch_url_0142 : (void *)win32_open_help_url)) {
        log_line("Win32 Help URL hook installation failed");
        return FALSE;
    }
    if (!VirtualProtect(achievements_slot, sizeof(*achievements_slot),
                        PAGE_READWRITE, &old_protection)) {
        log_line("Achievements IAT hook protection failed");
        return FALSE;
    }
    *achievements_slot = (void *)win32_show_achievements_noop;
    VirtualProtect(achievements_slot, sizeof(*achievements_slot),
                   old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), achievements_slot,
                          sizeof(*achievements_slot));
    if (host_is_0142) return TRUE;
    if (!VirtualProtect(invite_slot, sizeof(*invite_slot),
                        PAGE_READWRITE, &old_protection)) {
        log_line("Invite Player IAT hook protection failed");
        return FALSE;
    }
    *invite_slot = (void *)win32_show_game_invite_noop;
    VirtualProtect(invite_slot, sizeof(*invite_slot),
                   old_protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), invite_slot,
                          sizeof(*invite_slot));
    log_line("installed Win32 picker/fullscreen/text-input/Help/Achievements/Invite fixes");
    log_line("unsafe DeviceResources UI-task drain disabled");
    return TRUE;
}

static void sync_hid_pointer_scale(float scale_x, float scale_y)
{
    void *platform;
    BYTE *controller;
    char line[128];

    if (!game_app_main) return;
    platform = *(void **)((BYTE *)game_app_main + 4);
    controller = get_hid_controller(platform);
    if (!controller) return;
    *(float *)(controller + 0x64) = scale_x;
    *(float *)(controller + 0x68) = scale_y;
    wsprintfA(line, "HID pointer scale synchronized: %d/%d",
              (int)(scale_x * 1000.0f), (int)(scale_y * 1000.0f));
    log_line(line);
}

static BOOL resize_game_window(HWND window, UINT width, UINT height)
{
    BYTE *image = game_image ? game_image : (BYTE *)GetModuleHandleW(NULL);
    GameSetWindowMetricsFn set_metrics = image
        ? (GameSetWindowMetricsFn)(image + 0x005394f0) : NULL;
    GameReleaseTargetsFn release_targets = image
        ? (GameReleaseTargetsFn)(image + EARLY_RVA(0x005398f0, 0x005923a0)) : NULL;
    GameInitTargetsFn init_targets = image
        ? (GameInitTargetsFn)(image + EARLY_RVA(0x00538f90, 0x00598140)) : NULL;
    void *platform;
    void *game;
    float pixel_size[2];
    float dpi_scale[2] = {1.0f, 1.0f};
    HRESULT result;
    char line[192];

    struct { UINT width, height; BYTE reserved[24]; } display;

    (void)window;
    if (!width || !height || !game_swap_chain || !game_renderer ||
        !game_context) {
        return FALSE;
    }

    wsprintfA(line, "resize requested: %ux%u", width, height);
    log_line(line);

    if (host_is_windows7 || host_is_0142) {
        ID3D11DepthStencilView *depth_view;
        D3D11_VIEWPORT viewport;

        /*
         * Renderer::setWindowMetrics follows the UWP DeviceResources resize
         * path.  On Windows 7 the renderer stores base ID3D11Device/Context
         * interfaces in fields that normally contain Device2/Context2.  The
         * frame itself works because all used draw methods are inherited, but
         * the UWP resize sequence also walks newer display/DXGI state and can
         * leave the HWND swap chain with released targets.  Resize the legacy
         * DXGI 1.0 swap chain directly instead, then reuse the game's own
         * target constructor.
         */
        if (!game_resource_owner || !release_targets || !init_targets) {
            log_line("Win7 resize: owner/target helpers unavailable");
            return FALSE;
        }

        ID3D11DeviceContext_OMSetRenderTargets(game_context, 0, NULL, NULL);
        ID3D11DeviceContext_Flush(game_context);

        /* Drop only the host's extra RTV reference.  release_targets clears
         * renderer+0x88/+0x8c/+0x90 and releases the renderer-owned refs. */
        if (game_target) {
            ID3D11RenderTargetView_Release(game_target);
            game_target = NULL;
        }
        release_targets(game_renderer);
        if (host_is_0142) {
            ID3D11Texture2D **backbuffer = (ID3D11Texture2D **)((BYTE *)game_renderer + 0x114);
            if (*backbuffer) {
                ID3D11Texture2D_Release(*backbuffer);
                *backbuffer = NULL;
            }
        }

        result = IDXGISwapChain_ResizeBuffers(
            game_swap_chain, 1, width, height, DXGI_FORMAT_UNKNOWN, 0);
        log_hresult("Win7 IDXGISwapChain::ResizeBuffers", result);
        if (FAILED(result)) {
            /* Leave a precise diagnostic instead of silently continuing with
             * the target fields cleared. */
            SetWindowTextW(game_window, L"Minecraft - Win7 ResizeBuffers failed");
            return FALSE;
        }

        ZeroMemory(&display, sizeof(display));
        display.width = width;
        display.height = height;
        *(IDXGISwapChain **)((BYTE *)&display + EARLY_RVA(0x14, 0x1c)) = game_swap_chain;
        init_targets(game_resource_owner, &display, game_renderer);

        /* Keep both renderer swap-chain locations synchronized with the HWND
         * chain.  +0x60 is the native swap-chain-size wrapper; +0x74 is the
         * direct COM pointer used by target creation and Present. */
        *(UINT *)((BYTE *)game_renderer + RENDERER_DISPLAY_OFFSET) = width;
        *(UINT *)((BYTE *)game_renderer + (RENDERER_DISPLAY_OFFSET + 4)) = height;
        if (*(IDXGISwapChain **)((BYTE *)game_renderer + RENDERER_SWAP_CHAIN_OFFSET) !=
            game_swap_chain) {
            IDXGISwapChain *old =
                *(IDXGISwapChain **)((BYTE *)game_renderer + RENDERER_SWAP_CHAIN_OFFSET);
            if (old) IDXGISwapChain_Release(old);
            IDXGISwapChain_AddRef(game_swap_chain);
            *(IDXGISwapChain **)((BYTE *)game_renderer + RENDERER_SWAP_CHAIN_OFFSET) =
                game_swap_chain;
        }

        game_target = *(ID3D11RenderTargetView **)(
            (BYTE *)game_renderer + RENDERER_RTV_OFFSET);
        depth_view = *(ID3D11DepthStencilView **)(
            (BYTE *)game_renderer + RENDERER_DSV_OFFSET);
        if (!game_target) {
            log_line("Win7 resize: target recreation returned null RTV");
            SetWindowTextW(game_window, L"Minecraft - Win7 target recreation failed");
            return FALSE;
        }
        ID3D11RenderTargetView_AddRef(game_target);

        if (host_is_0142) sync_0142_window_metrics(width, height);
        else {
            *(float *)((BYTE *)game_renderer + 0xb0) = (float)width;
            *(float *)((BYTE *)game_renderer + 0xb4) = (float)height;
            *(float *)((BYTE *)game_renderer + 0xb8) = 1.0f;
            *(float *)((BYTE *)game_renderer + 0xbc) = 1.0f;
        }

        ZeroMemory(&viewport, sizeof(viewport));
        viewport.Width = (float)width;
        viewport.Height = (float)height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        CopyMemory((BYTE *)game_renderer + RENDERER_VIEWPORT_OFFSET,
                   &viewport, sizeof(viewport));
        ID3D11DeviceContext_RSSetViewports(game_context, 1, &viewport);
        ID3D11DeviceContext_OMSetRenderTargets(
            game_context, 1, &game_target, depth_view);

        /* Prove that the resized backbuffer is presentable before returning
         * to the normal frame loop. */
        result = IDXGISwapChain_Present(game_swap_chain, 0, 0);
        log_hresult("Win7 resize validation Present", result);
        if (FAILED(result)) return FALSE;
        log_line("Win7 legacy swap chain targets resized and rebound");
    } else {
        if (!set_metrics) return FALSE;
        if (game_target) {
            ID3D11RenderTargetView_Release(game_target);
            game_target = NULL;
        }
        pixel_size[0] = (float)width;
        pixel_size[1] = (float)height;
        set_metrics(game_renderer, pixel_size, dpi_scale);
        game_target = *(ID3D11RenderTargetView **)(
            (BYTE *)game_renderer + RENDERER_RTV_OFFSET);
        if (!game_target) {
            log_line("resize: full renderer resize returned null render target");
            return FALSE;
        }
        ID3D11RenderTargetView_AddRef(game_target);
    }

    /* HWND mouse coordinates are already physical client pixels. */
    sync_hid_pointer_scale(1.0f, 1.0f);

    if (game_app_main) {
        platform = *(void **)((BYTE *)game_app_main + 4);
        game = *(void **)((BYTE *)game_app_main + 8);
        if (platform) {
            *(int *)((BYTE *)platform + PLATFORM_WIDTH_OFFSET) = (int)width;
            *(int *)((BYTE *)platform + PLATFORM_HEIGHT_OFFSET) = (int)height;
        }
        if (game && !IsBadReadPtr(*(void ***)game, 0x50)) {
            GameResizeFn resize =
                (GameResizeFn)(*(void ***)game)[0x4c / sizeof(void *)];
            if (resize) resize(game, (int)width, (int)height, 0);
            if (host_is_0142) {
                typedef void (__attribute__((thiscall)) *GamePixelResizeFn)(void *, int, int);
                ((GamePixelResizeFn)(game_image + 0x000331a0))(game, (int)width, (int)height);
            }
        }
    }
    mouse_clip_dirty = TRUE;
    log_line("HWND renderer targets, viewport, UI and HID size synchronized");
    return TRUE;
}

/*
 * AppPlatform_Winrt normally turns CoreWindow pointer callbacks into this
 * compact 16-byte event and moves it into AppPlatform's input queue.  The UWP
 * poller is disabled in the EXE because an HWND process has no CoreWindow, so
 * reproduce that final, platform-independent queue operation here.
 *
 * Mouse event layout used by Minecraft 0.13.2:
 *   +0  byte  device type (0 = mouse)
 *   +2  word  client x
 *   +4  word  client y
 *   +8  dword movement/wheel payload
 *   +12 byte  action (0 = move, 1 = left, 2 = right, 4 = wheel)
 *   +13 byte  pressed
 */
static BOOL queue_mouse_event(int x, int y, DWORD payload,
                              BYTE action, BYTE pressed)
{
    static unsigned logged_events;
    GameOperatorNewFn game_operator_new;
    GameEnqueueInputFn enqueue_input;
    BYTE *event;
    void *platform;
    char line[128];

    if (!game_image || !game_app_main) return FALSE;
    platform = *(void **)((BYTE *)game_app_main + 4);
    if (!platform) return FALSE;

    if (payload) {
        if (x < SHRT_MIN) x = SHRT_MIN;
        if (y < SHRT_MIN) y = SHRT_MIN;
        if (x > SHRT_MAX) x = SHRT_MAX;
        if (y > SHRT_MAX) y = SHRT_MAX;
    } else {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > 0xffff) x = 0xffff;
        if (y > 0xffff) y = 0xffff;
    }

    game_operator_new =
        (GameOperatorNewFn)(game_image + EARLY_RVA(0x005798ea, 0x006cee5b));
    enqueue_input =
        (GameEnqueueInputFn)(game_image + EARLY_RVA(0x003c10e0, 0x004ba8c0));
    event = (BYTE *)game_operator_new(0x10);
    if (!event) return FALSE;

    ZeroMemory(event, 0x10);
    *(WORD *)(event + 2) = (WORD)x;
    *(WORD *)(event + 4) = (WORD)y;
    *(DWORD *)(event + 8) = payload;
    event[12] = action;
    event[13] = pressed;
    enqueue_input(platform, event);

    if (logged_events < 12) {
        wsprintfA(line, "mouse event: x=%d y=%d action=%u pressed=%u",
                  x, y, (unsigned)action, (unsigned)pressed);
        log_line(line);
        ++logged_events;
    }
    return TRUE;
}

/* Translate Win32 virtual keys through the exact UWP VirtualKey table used
 * by the original HIDController.  The mapper takes its input in EDX. */
static BYTE map_win32_virtual_key(WPARAM virtual_key)
{
    GameMapVirtualKeyFn map_key;
    int mapped;

    if (!game_image) return 0;
    map_key = (GameMapVirtualKeyFn)(game_image + EARLY_RVA(0x00214cf0, 0x00296760));
    mapped = map_key(NULL, (int)(virtual_key & 0xffff));
    if (mapped <= 0 || mapped > 0xff) return 0;
    return (BYTE)mapped;
}

/* Keyboard event layout used by Minecraft 0.13.2:
 *   +0 byte  type (2 = keyboard)
 *   +1 byte  mapped game key
 *   +4 dword state (1 = down, 0 = up)
 */
static BOOL queue_keyboard_event(BYTE key, DWORD state)
{
    static unsigned logged_events;
    GameOperatorNewFn game_operator_new;
    GameEnqueueInputFn enqueue_input;
    BYTE *event;
    void *platform;
    char line[112];

    if (!key || !game_image || !game_app_main) return FALSE;
    platform = *(void **)((BYTE *)game_app_main + 4);
    if (!platform) return FALSE;

    game_operator_new =
        (GameOperatorNewFn)(game_image + EARLY_RVA(0x005798ea, 0x006cee5b));
    enqueue_input =
        (GameEnqueueInputFn)(game_image + EARLY_RVA(0x003c10e0, 0x004ba8c0));
    event = (BYTE *)game_operator_new(8);
    if (!event) return FALSE;

    ZeroMemory(event, 8);
    event[0] = 2;
    event[1] = key;
    *(DWORD *)(event + 4) = state;
    enqueue_input(platform, event);

    if (logged_events < 24) {
        wsprintfA(line, "keyboard event: key=%u state=%u",
                  (unsigned)key, (unsigned)state);
        log_line(line);
        ++logged_events;
    }
    return TRUE;
}

/* The real CharacterReceived path converts text to UTF-8 and calls the
 * constructor at image+0x219820. It allocates a 0x20-byte event of type 3
 * and constructs the embedded MSVC std::string used by the UI controls. */
static BOOL queue_character_event(DWORD codepoint)
{
    void *make_text_event;
    GameEnqueueInputFn enqueue_input;
    char utf8[5];
    unsigned length;
    BYTE flag = 0;
    void *event = NULL;
    void *platform;

    if ((codepoint < 0x20 && codepoint != 0x08) ||
        codepoint > 0x10ffff ||
        (codepoint >= 0xd800 && codepoint <= 0xdfff) ||
        !game_image || !game_app_main) {
        return FALSE;
    }
    platform = *(void **)((BYTE *)game_app_main + 4);
    if (!platform) return FALSE;

    if (codepoint <= 0x7f) {
        utf8[0] = (char)codepoint;
        length = 1;
    } else if (codepoint <= 0x7ff) {
        utf8[0] = (char)(0xc0 | (codepoint >> 6));
        utf8[1] = (char)(0x80 | (codepoint & 0x3f));
        length = 2;
    } else if (codepoint <= 0xffff) {
        utf8[0] = (char)(0xe0 | (codepoint >> 12));
        utf8[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        utf8[2] = (char)(0x80 | (codepoint & 0x3f));
        length = 3;
    } else {
        utf8[0] = (char)(0xf0 | (codepoint >> 18));
        utf8[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        utf8[2] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        utf8[3] = (char)(0x80 | (codepoint & 0x3f));
        length = 4;
    }
    utf8[length] = 0;

    make_text_event = game_image + EARLY_RVA(0x00219820, 0x0029b5d0);
    enqueue_input =
        (GameEnqueueInputFn)(game_image + EARLY_RVA(0x003c10e0, 0x004ba8c0));
    call_game_make_text_event(make_text_event, &event, utf8, &flag);
    if (!event || IsBadReadPtr(event, 0x20) || ((BYTE *)event)[0] != 3) {
        log_line("text event factory returned an invalid event");
        return FALSE;
    }
    enqueue_input(platform, event);
    return TRUE;
}

#define WIN32CRAFT_CF_UNICODETEXT 13u
static wchar_t clipboard_text_0132[2048];

static BOOL paste_clipboard_text_0132(void)
{
    HANDLE memory;
    const wchar_t *input;
    UINT length = 0, index = 0;
    BOOL success = FALSE;
    if (!text_input_active || !OpenClipboard(game_window)) return FALSE;
    memory = GetClipboardData(WIN32CRAFT_CF_UNICODETEXT);
    input = memory ? (const wchar_t *)GlobalLock(memory) : NULL;
    if (input) {
        while (length < ARRAYSIZE(clipboard_text_0132) - 1 && input[length]) {
            clipboard_text_0132[length] = input[length];
            ++length;
        }
        clipboard_text_0132[length] = 0;
        GlobalUnlock(memory);
        success = TRUE;
    }
    CloseClipboard();
    if (!success) return FALSE;
    while (index < length) {
        DWORD codepoint = clipboard_text_0132[index++];
        if (codepoint >= 0xd800 && codepoint <= 0xdbff && index < length) {
            DWORD low = clipboard_text_0132[index];
            if (low >= 0xdc00 && low <= 0xdfff) {
                ++index;
                codepoint = 0x10000u + ((codepoint - 0xd800u) << 10) +
                    (low - 0xdc00u);
            }
        }
        if (codepoint < 0x20) continue;
        if (!queue_character_event(codepoint)) return FALSE;
    }
    return TRUE;
}

static void handle_win32_key(WPARAM virtual_key, BOOL pressed)
{
    BYTE key = map_win32_virtual_key(virtual_key);
    if (!key) return;

    if (pressed) {
        /* Match the UWP path: suppress Windows autorepeat while held. */
        if (win32_key_down[key]) return;
        win32_key_down[key] = 1;
        queue_keyboard_event(key, 1);
    } else {
        win32_key_down[key] = 0;
        queue_keyboard_event(key, 0);
    }
}

static void release_all_win32_keys(void)
{
    unsigned key;
    for (key = 1; key < ARRAYSIZE(win32_key_down); ++key) {
        if (win32_key_down[key]) {
            win32_key_down[key] = 0;
            queue_keyboard_event((BYTE)key, 0);
        }
    }
    pending_high_surrogate = 0;
    text_ctrl_down = FALSE;
    clipboard_handled_key = 0;
    suppress_next_t_character = FALSE;
}

static void handle_win32_character(WPARAM value)
{
    WORD unit = (WORD)value;

    if (suppress_next_t_character) {
        /* TranslateMessage produces one WM_CHAR for the same physical T key.
         * The key event opens chat; forwarding that character would also type
         * the layout-dependent letter into the freshly opened text box. */
        suppress_next_t_character = FALSE;
        return;
    }
    if (!text_input_active) {
        pending_high_surrogate = 0;
        return;
    }

    if (unit >= 0xd800 && unit <= 0xdbff) {
        pending_high_surrogate = unit;
        return;
    }
    if (unit >= 0xdc00 && unit <= 0xdfff && pending_high_surrogate) {
        DWORD codepoint = 0x10000u +
            (((DWORD)pending_high_surrogate - 0xd800u) << 10) +
            ((DWORD)unit - 0xdc00u);
        pending_high_surrogate = 0;
        queue_character_event(codepoint);
        return;
    }
    pending_high_surrogate = 0;
    queue_character_event((DWORD)unit);
}

static void queue_pointer_position(LPARAM lparam)
{
    int x = (short)LOWORD(lparam);
    int y = (short)HIWORD(lparam);

    if (mouse_capture_active) {
        RECT client;
        int center_x;
        int center_y;

        GetClientRect(game_window, &client);
        center_x = (client.right - client.left) / 2;
        center_y = (client.bottom - client.top) / 2;
        x -= center_x;
        y -= center_y;
        if (x || y) {
            queue_mouse_event(x, y, 1, 0, 0);
            center_mouse_cursor(game_window);
        }
        return;
    }

    /*
     * payload=0 selects Mouse::feed's absolute-coordinate path.  The UWP
     * direct-pointer callback uses payload=1 because its x/y fields are
     * relative deltas paired with a separately maintained cursor position.
     */
    queue_mouse_event(x, y, 0, 0, 0);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                    WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        /* Preserve the normal Windows Alt+F4 close command. */
        if (message == WM_SYSKEYDOWN && wparam == VK_F4 &&
            ((DWORD)lparam & (1u << 29))) {
            return DefWindowProcW(window, message, wparam, lparam);
        }
        if (wparam == 0x11 || wparam == 0xa2 || wparam == 0xa3) {
            text_ctrl_down = TRUE;
            /* Text is injected directly; forwarding Ctrl makes the legacy
             * edit box discard pasted characters while the key is held. */
            if (host_is_0142 && text_input_active) return 0;
        }
        if (text_input_active && text_ctrl_down && wparam == 'V' &&
            !((DWORD)lparam & (1u << 30))) {
            if (host_is_0142) handle_win32_key(0x11, FALSE);
            BOOL pasted = paste_clipboard_text_0132();
            clipboard_handled_key = wparam;
            log_line(pasted ? "early-2016 clipboard paste queued as game text"
                            : "early-2016 clipboard paste unavailable");
            return 0;
        }
        if (wparam == VK_T && !text_input_active &&
            !((DWORD)lparam & (1u << 30))) {
            suppress_next_t_character = TRUE;
        }
        handle_win32_key(wparam, TRUE);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wparam == clipboard_handled_key) {
            clipboard_handled_key = 0;
            return 0;
        }
        handle_win32_key(wparam, FALSE);
        if (wparam == 0x11 || wparam == 0xa2 || wparam == 0xa3)
            text_ctrl_down = FALSE;
        return 0;
    case WM_CHAR:
        handle_win32_character(wparam);
        return 0;
    case WM_UNICHAR:
        if (wparam == UNICODE_NOCHAR) return TRUE;
        if (text_input_active) queue_character_event((DWORD)wparam);
        return 0;
    case WM_MOUSEWHEEL: {
        int delta = (SHORT)HIWORD(wparam);
        if (delta > 127) delta = 127;
        if (delta < -128) delta = -128;
        queue_mouse_event(0, 0, 0, 4, (BYTE)(CHAR)delta);
        return 0;
    }
    case WM_KILLFOCUS:
        release_all_win32_keys();
        update_mouse_capture(window);
        return 0;
    case WM_SETFOCUS:
        update_mouse_capture(window);
        return 0;
    case WM_ENTERSIZEMOVE:
        window_in_size_move = TRUE;
        pending_resize_width = 0;
        pending_resize_height = 0;
        log_line("interactive window resize entered; DXGI resize deferred");
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED) {
            UINT width = (UINT)LOWORD(lparam);
            UINT height = (UINT)HIWORD(lparam);
            mouse_clip_dirty = TRUE;
            if (window_in_size_move && host_is_windows7) {
                pending_resize_width = width;
                pending_resize_height = height;
            } else {
                resize_game_window(window, width, height);
            }
            update_mouse_capture(window);
        }
        return 0;
    case WM_EXITSIZEMOVE:
        window_in_size_move = FALSE;
        if (host_is_windows7) {
            RECT client;
            UINT width = pending_resize_width;
            UINT height = pending_resize_height;
            if ((!width || !height) && GetClientRect(window, &client)) {
                width = (UINT)(client.right - client.left);
                height = (UINT)(client.bottom - client.top);
            }
            pending_resize_width = 0;
            pending_resize_height = 0;
            if (width && height) resize_game_window(window, width, height);
            log_line("interactive window resize exited; deferred DXGI resize applied");
        }
        mouse_clip_dirty = TRUE;
        update_mouse_capture(window);
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_MOVE:
    case WM_WINDOWPOSCHANGED:
        mouse_clip_dirty = TRUE;
        update_mouse_capture(window);
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_ACTIVATEAPP:
        if (!wparam) release_all_win32_keys();
        update_mouse_capture(window);
        return 0;
    case WM_SETCURSOR:
        if (mouse_capture_active && LOWORD(lparam) == HTCLIENT) {
            SetCursor(NULL);
            return TRUE;
        }
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_MOUSEMOVE:
        queue_pointer_position(lparam);
        return 0;
    case WM_LBUTTONDOWN:
        if (initial_menu_cursor_guard) {
            /* A real click proves that the Win32 menu is active.  Subsequent
             * relative requests (for example after Play enters a world) are
             * now authoritative. */
            initial_menu_cursor_guard = FALSE;
            host_relative_requested = FALSE;
            mouse_clip_dirty = TRUE;
            log_line("initial main-menu cursor guard released by click");
        }
        SetFocus(window);
        SetCapture(window);
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 1, 1);
        return 0;
    case WM_LBUTTONUP:
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 1, 0);
        if (!mouse_capture_active && GetCapture() == window) ReleaseCapture();
        return 0;
    case WM_RBUTTONDOWN:
        SetFocus(window);
        SetCapture(window);
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 2, 1);
        return 0;
    case WM_RBUTTONUP:
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 2, 0);
        if (!mouse_capture_active && GetCapture() == window) ReleaseCapture();
        return 0;
    case WM_MBUTTONDOWN:
        SetFocus(window);
        SetCapture(window);
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 3, 1);
        return 0;
    case WM_MBUTTONUP:
        queue_pointer_position(lparam);
        queue_mouse_event((short)LOWORD(lparam), (short)HIWORD(lparam),
                          0, 3, 0);
        if (!mouse_capture_active && GetCapture() == window) ReleaseCapture();
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_ERASEBKGND:
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_PAINT: {
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        return 0;
    }
    default:
        return DefWindowProcW(window, message, wparam, lparam);
    }
}

int WINAPI Win32Bootstrap(void)
{
    WNDCLASSEXW klass;
    HWND window;
    MSG message;
    HINSTANCE instance = GetModuleHandleW(NULL);
    BYTE *image = (BYTE *)instance;
    HRESULT ro_result;

    {
        DWORD pe = *(DWORD *)(image + 0x3c);
        DWORD size = *(DWORD *)(image + pe + 0x50);
        if (size != 0x008cc000 && size != 0x00abf000) return 3;
        host_is_0142 = size == 0x00abf000;
    }
    game_image = image;
    game_window = NULL;
    host_0132_main_thread_id = GetCurrentThreadId();
    {
        typedef DWORD (WINAPI *GetVersionFn)(void);
        GetVersionFn get_version = (GetVersionFn)GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"), "GetVersion");
        DWORD version = get_version ? get_version() : 0;
        host_is_windows7 =
            (version & 0xffu) == 6 && ((version >> 8) & 0xffu) == 1;
    }
    log_line(host_is_0142 ? "Win32Craft 0.14.2 bootstrap entered"
                          : "Win32Craft 0.13.2 bootstrap entered");
    install_exception_trace(image);
    if (host_is_0142 && !install_0142_native_host_hooks(image)) {
        log_line("0.14.2 native host signature mismatch");
        return 4;
    }
    log_line(host_is_windows7
        ? "Windows 7 DXGI present compatibility enabled"
        : "standard DXGI present path enabled");
    ro_result = RoInitialize(RO_INIT_MULTITHREADED);
    log_hresult("RoInitialize(RO_INIT_MULTITHREADED)", ro_result);
    if (!GetCurrentDirectoryW(MAX_PATH, package_path)) {
        lstrcpyW(package_path, L".");
    }
    create_user_data();
    install_activation_redirect(image);
    install_vccorlib_compat_0132(image);
    install_0132_copyright_override(image);
    install_development_version_text_0132(image);
    install_d3d_compile_trace(image);
    patch_win7_device2_queries_to_base(image);
    install_d3d11_device_compat(image);
    install_0132_frame_pacing_hook(image);
    verify_patched_audio_exe(image);
    install_native_fmod_bridge(image);
    install_mouse_mode_hooks(image);
    install_win32_platform_fixes(image);

    ZeroMemory(&klass, sizeof(klass));
    klass.cbSize = sizeof(klass);
    klass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    klass.lpfnWndProc = window_proc;
    klass.hInstance = instance;
    klass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    klass.hbrBackground = (HBRUSH)(ULONG_PTR)(COLOR_WINDOWTEXT + 1);
    klass.lpszClassName = L"Win32CraftEarly2016";
    if (!RegisterClassExW(&klass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        log_line("RegisterClassExW failed");
        return 1;
    }

    window = CreateWindowExW(
        0, klass.lpszClassName, host_is_0142 ? L"Win32Craft 0.14.2" : L"Win32Craft 0.13.2",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, instance, NULL);
    if (!window) {
        log_line("CreateWindowExW failed");
        return 2;
    }
    game_window = window;
    initial_menu_cursor_guard = TRUE;
    log_line("HWND created");
    if (!initialize_game_d3d(window)) {
        SetWindowTextW(window, host_is_0142 ? L"Win32Craft 0.14.2 - D3D11 initialization failed"
                                          : L"Win32Craft 0.13.2 - D3D11 initialization failed");
        log_line("startup stopped: initialize_game_d3d failed");
    } else if (!initialize_game_app(image)) {
        SetWindowTextW(window, host_is_0142 ? L"Win32Craft 0.14.2 - AppMain initialization failed"
                                          : L"Win32Craft 0.13.2 - AppMain initialization failed");
        log_line("startup stopped: initialize_game_app failed");
    } else if (!activate_game_app(window)) {
        SetWindowTextW(window, host_is_0142 ? L"Win32Craft 0.14.2 - AppMain activation failed"
                                          : L"Win32Craft 0.13.2 - AppMain activation failed");
        log_line("startup stopped: activate_game_app failed");
    } else {
        SetWindowTextW(window, host_is_0142 ? L"Win32Craft 0.14.2" : L"Win32Craft 0.13.2");
        /* Startup listeners leave one stale relative request even
         * though the first visible screen is the main menu. */
        host_relative_requested = FALSE;
        mouse_clip_dirty = TRUE;
        set_game_relative_actual(FALSE);
        update_mouse_capture(window);
        log_line("initial main-menu cursor forced to absolute mode");
    }

    for (;;) {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                log_line("message loop ended");
                return (int)message.wParam;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        update_mouse_capture(window);
        render_game_target();
        Sleep(0);
    }
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
    (void)instance;
    (void)reason;
    (void)reserved;
    return TRUE;
}
