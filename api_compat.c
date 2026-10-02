#include "win32_compat.h"

#define WIN32CRAFT_E_BOUNDS ((HRESULT)0x8000000BL)
#define WIN32CRAFT_RO_E_METADATA_NAME_NOT_FOUND ((HRESULT)0x8000000FL)
#define WIN32CRAFT_S_FALSE ((HRESULT)1L)
#define WIN32CRAFT_ERROR_DEVICE_NOT_CONNECTED 1167UL

DWORD WINAPI Win32CraftJsStartDebugging(
    void *runtime, void *callback, void *callback_state)
{
    (void)runtime;
    (void)callback;
    (void)callback_state;
    return 0;
}

HMODULE WINAPI Win32CraftLoadPackagedLibrary(LPCWSTR path, DWORD reserved)
{
    (void)reserved;
    return LoadLibraryW(path);
}

int _fltused;

typedef struct D3CraftMultiQi {
    const GUID *iid;
    void *interface_pointer;
    HRESULT result;
} D3CraftMultiQi;

static void *resolve_ole32(const char *name)
{
    static HMODULE module;
    if (!module) module = LoadLibraryW(L"ole32.dll");
    return module ? GetProcAddress(module, name) : NULL;
}

HRESULT WINAPI Win32CraftCoCreateFreeThreadedMarshaler(void *outer, void **value)
{
    typedef HRESULT (WINAPI *Fn)(void *, void **);
    Fn function = (Fn)resolve_ole32("CoCreateFreeThreadedMarshaler");
    if (!value) return E_POINTER;
    *value = NULL;
    return function ? function(outer, value) : E_NOTIMPL;
}

HRESULT WINAPI Win32CraftCoCreateGuid(GUID *guid)
{
    typedef HRESULT (WINAPI *Fn)(GUID *);
    Fn function = (Fn)resolve_ole32("CoCreateGuid");
    return !guid ? E_POINTER : function ? function(guid) : E_NOTIMPL;
}

void *WINAPI Win32CraftCoTaskMemAlloc(SIZE_T size)
{
    typedef void *(WINAPI *Fn)(SIZE_T);
    Fn function = (Fn)resolve_ole32("CoTaskMemAlloc");
    return function ? function(size) : NULL;
}

void WINAPI Win32CraftCoTaskMemFree(void *memory)
{
    typedef void (WINAPI *Fn)(void *);
    Fn function = (Fn)resolve_ole32("CoTaskMemFree");
    if (function) function(memory);
}

HRESULT WINAPI Win32CraftIIDFromString(const WCHAR *text, GUID *iid)
{
    typedef HRESULT (WINAPI *Fn)(const WCHAR *, GUID *);
    Fn function = (Fn)resolve_ole32("IIDFromString");
    return !iid ? E_POINTER : function ? function(text, iid) : E_NOTIMPL;
}

INT WINAPI Win32CraftStringFromGUID2(const GUID *guid, WCHAR *text, INT count)
{
    typedef INT (WINAPI *Fn)(const GUID *, WCHAR *, INT);
    Fn function = (Fn)resolve_ole32("StringFromGUID2");
    return !guid || !text || count <= 0 ? 0
        : function ? function(guid, text, count) : 0;
}

HRESULT WINAPI Win32CraftCoCreateInstanceFromApp(
    const GUID *class_id, void *outer, DWORD context,
    ULONG query_count, D3CraftMultiQi *queries)
{
    typedef HRESULT (WINAPI *Fn)(const GUID *, void *, DWORD,
                                 const GUID *, void **);
    Fn function = (Fn)resolve_ole32("CoCreateInstance");
    HRESULT result = S_OK;
    ULONG index;
    if (!queries || !query_count) return E_INVALIDARG;
    for (index = 0; index < query_count; ++index) {
        queries[index].interface_pointer = NULL;
        queries[index].result = function && queries[index].iid
            ? function(class_id, outer, context, queries[index].iid,
                       &queries[index].interface_pointer)
            : REGDB_E_CLASSNOTREG;
        if (FAILED(queries[index].result)) result = queries[index].result;
    }
    return result;
}

HRESULT WINAPI Win32CraftCoGetContextToken(ULONG_PTR *token)
{
    if (!token) return E_POINTER;
    *token = 1;
    return S_OK;
}

ULONG WINAPI Win32CraftEventRegister(
    const GUID *provider, void *callback, void *context, ULONGLONG *handle)
{
    (void)provider; (void)callback; (void)context;
    if (handle) *handle = 1;
    return 0;
}

ULONG WINAPI Win32CraftEventSetInformation(
    ULONGLONG handle, INT type, const void *data, ULONG size)
{ (void)handle; (void)type; (void)data; (void)size; return 0; }
ULONG WINAPI Win32CraftEventUnregister(ULONGLONG handle)
{ (void)handle; return 0; }
ULONG WINAPI Win32CraftEventWriteTransfer(
    ULONGLONG handle, const void *descriptor, const GUID *activity,
    const GUID *related, ULONG count, const void *data)
{ (void)handle; (void)descriptor; (void)activity; (void)related;
  (void)count; (void)data; return 0; }

typedef struct Win32CraftHString {
    DWORD magic;
    DWORD flags;
    UINT32 length;
    const WCHAR *text;
} Win32CraftHString;

typedef struct Win32CraftHStringHeader {
    Win32CraftHString value;
} Win32CraftHStringHeader;

typedef struct Win32CraftHStringBuffer {
    Win32CraftHString value;
    DWORD reserved;
    WCHAR text[1];
} Win32CraftHStringBuffer;

static const WCHAR empty_string[] = L"";
#define WIN32CRAFT_HSTRING_MAGIC 0x48535452UL
#define WIN32CRAFT_HSTRING_OWNED 1UL
static void *native_hstring_proc(const char *name)
{
    static HMODULE combase;
    BYTE *image = (BYTE *)GetModuleHandleW(NULL);
    DWORD pe_offset;
    if (!image || IsBadReadPtr(image, 0x40)) return NULL;
    pe_offset = *(DWORD *)(image + 0x3c);
    if (pe_offset >= 0x1000 || IsBadReadPtr(image + pe_offset, 0x54) ||
        *(DWORD *)(image + pe_offset) != 0x00004550 ||
        *(DWORD *)(image + pe_offset + 0x50) != 0x02db1000)
        return NULL;
    if (!combase) combase = LoadLibraryW(L"combase.dll");
    return combase ? GetProcAddress(combase, name) : NULL;
}

static Win32CraftHString *hstring_value(HSTRING string)
{
    return (Win32CraftHString *)string;
}

HRESULT WINAPI WindowsCreateString(
    LPCWSTR source, UINT32 length, HSTRING *result)
{
    typedef HRESULT (WINAPI *NativeFn)(LPCWSTR, UINT32, HSTRING *);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsCreateString");
    Win32CraftHString *string;
    WCHAR *text;
    SIZE_T bytes;
    HANDLE heap;

    if (native) return native(source, length, result);
    if (!result) return E_POINTER;
    *result = NULL;
    if (!source && length) return E_INVALIDARG;
    bytes = sizeof(*string) + ((SIZE_T)length + 1) * sizeof(WCHAR);
    heap = GetProcessHeap();
    string = (Win32CraftHString *)HeapAlloc(heap, 0, bytes);
    if (!string) return E_OUTOFMEMORY;
    text = (WCHAR *)(string + 1);
    if (length) CopyMemory(text, source, (SIZE_T)length * sizeof(WCHAR));
    text[length] = 0;
    string->magic = WIN32CRAFT_HSTRING_MAGIC;
    string->flags = WIN32CRAFT_HSTRING_OWNED;
    string->length = length;
    string->text = text;
    *result = (HSTRING)string;
    return S_OK;
}

HRESULT WINAPI WindowsCreateStringReference(
    LPCWSTR source, UINT32 length, void *header, HSTRING *result)
{
    typedef HRESULT (WINAPI *NativeFn)(LPCWSTR, UINT32, void *, HSTRING *);
    NativeFn native = (NativeFn)native_hstring_proc(
        "WindowsCreateStringReference");
    Win32CraftHStringHeader *storage = (Win32CraftHStringHeader *)header;
    if (native) return native(source, length, header, result);
    if (!header || !result) return E_POINTER;
    *result = NULL;
    if (!source && length) return E_INVALIDARG;
    storage->value.magic = WIN32CRAFT_HSTRING_MAGIC;
    storage->value.flags = 0;
    storage->value.length = length;
    storage->value.text = source;
    *result = (HSTRING)&storage->value;
    return S_OK;
}

HRESULT WINAPI WindowsDeleteString(HSTRING string)
{
    typedef HRESULT (WINAPI *NativeFn)(HSTRING);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsDeleteString");
    Win32CraftHString *value = hstring_value(string);
    if (native) return native(string);
    if (value && value->magic == WIN32CRAFT_HSTRING_MAGIC &&
        (value->flags & WIN32CRAFT_HSTRING_OWNED))
        HeapFree(GetProcessHeap(), 0, value);
    return S_OK;
}

HRESULT WINAPI WindowsDuplicateString(HSTRING string, HSTRING *result)
{
    typedef HRESULT (WINAPI *NativeFn)(HSTRING, HSTRING *);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsDuplicateString");
    Win32CraftHString *value = hstring_value(string);
    if (native) return native(string, result);
    if (!result) return E_POINTER;
    *result = NULL;
    if (!value || value->magic != WIN32CRAFT_HSTRING_MAGIC)
        return WindowsCreateString(NULL, 0, result);
    return WindowsCreateString(value->text, value->length, result);
}

UINT32 WINAPI WindowsGetStringLen(HSTRING string)
{
    typedef UINT32 (WINAPI *NativeFn)(HSTRING);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsGetStringLen");
    Win32CraftHString *value = hstring_value(string);
    if (native) return native(string);
    return value && value->magic == WIN32CRAFT_HSTRING_MAGIC
        ? value->length : 0;
}

LPCWSTR WINAPI WindowsGetStringRawBuffer(HSTRING string, UINT32 *length)
{
    typedef LPCWSTR (WINAPI *NativeFn)(HSTRING, UINT32 *);
    NativeFn native = (NativeFn)native_hstring_proc(
        "WindowsGetStringRawBuffer");
    Win32CraftHString *value = hstring_value(string);
    if (native) return native(string, length);
    if (length) *length = value && value->magic == WIN32CRAFT_HSTRING_MAGIC
        ? value->length : 0;
    return value && value->magic == WIN32CRAFT_HSTRING_MAGIC
        ? value->text : empty_string;
}

BOOL WINAPI WindowsIsStringEmpty(HSTRING string)
{
    typedef BOOL (WINAPI *NativeFn)(HSTRING);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsIsStringEmpty");
    if (native) return native(string);
    return WindowsGetStringLen(string) == 0;
}

INT WINAPI WindowsCompareStringOrdinal(HSTRING left, HSTRING right, INT *result)
{
    typedef HRESULT (WINAPI *NativeFn)(HSTRING, HSTRING, INT *);
    NativeFn native = (NativeFn)native_hstring_proc(
        "WindowsCompareStringOrdinal");
    LPCWSTR a = WindowsGetStringRawBuffer(left, NULL);
    LPCWSTR b = WindowsGetStringRawBuffer(right, NULL);
    UINT32 alen = WindowsGetStringLen(left), blen = WindowsGetStringLen(right);
    UINT32 index, count = alen < blen ? alen : blen;
    if (native) return native(left, right, result);
    if (!result) return E_POINTER;
    for (index = 0; index < count && a[index] == b[index]; ++index) {}
    *result = index < count ? (a[index] < b[index] ? -1 : 1)
                            : (alen < blen ? -1 : alen > blen ? 1 : 0);
    return S_OK;
}

HRESULT WINAPI WindowsConcatString(HSTRING left, HSTRING right, HSTRING *result)
{
    typedef HRESULT (WINAPI *NativeFn)(HSTRING, HSTRING, HSTRING *);
    NativeFn native = (NativeFn)native_hstring_proc("WindowsConcatString");
    UINT32 alen = WindowsGetStringLen(left), blen = WindowsGetStringLen(right);
    LPCWSTR a = WindowsGetStringRawBuffer(left, NULL);
    LPCWSTR b = WindowsGetStringRawBuffer(right, NULL);
    WCHAR *text;
    HANDLE heap;
    HRESULT status;
    if (native) return native(left, right, result);
    if (!result) return E_POINTER;
    heap = GetProcessHeap();
    text = (WCHAR *)HeapAlloc(heap, 0, ((SIZE_T)alen + blen + 1) * sizeof(WCHAR));
    if (!text) return E_OUTOFMEMORY;
    CopyMemory(text, a, (SIZE_T)alen * sizeof(WCHAR));
    CopyMemory(text + alen, b, (SIZE_T)blen * sizeof(WCHAR));
    text[alen + blen] = 0;
    status = WindowsCreateString(text, alen + blen, result);
    HeapFree(heap, 0, text);
    return status;
}

HRESULT WINAPI WindowsSubstringWithSpecifiedLength(
    HSTRING string, UINT32 start, UINT32 length, HSTRING *result)
{
    UINT32 total = WindowsGetStringLen(string);
    LPCWSTR text = WindowsGetStringRawBuffer(string, NULL);
    if (start > total || length > total - start) return WIN32CRAFT_E_BOUNDS;
    return WindowsCreateString(text + start, length, result);
}

HRESULT WINAPI WindowsSubstring(HSTRING string, UINT32 start, HSTRING *result)
{
    UINT32 total = WindowsGetStringLen(string);
    if (start > total) return WIN32CRAFT_E_BOUNDS;
    return WindowsSubstringWithSpecifiedLength(string, start, total - start, result);
}

HRESULT WINAPI WindowsStringHasEmbeddedNull(HSTRING string, BOOL *result)
{
    LPCWSTR text = WindowsGetStringRawBuffer(string, NULL);
    UINT32 index, length = WindowsGetStringLen(string);
    if (!result) return E_POINTER;
    *result = FALSE;
    for (index = 0; index < length; ++index)
        if (!text[index]) { *result = TRUE; break; }
    return S_OK;
}

HRESULT WINAPI WindowsReplaceString(
    HSTRING string, HSTRING search, HSTRING replacement, HSTRING *result)
{
    (void)search;
    (void)replacement;
    return WindowsDuplicateString(string, result);
}

HRESULT WINAPI WindowsTrimStringStart(
    HSTRING string, HSTRING trim, HSTRING *result)
{
    (void)trim;
    return WindowsDuplicateString(string, result);
}

HRESULT WINAPI WindowsTrimStringEnd(
    HSTRING string, HSTRING trim, HSTRING *result)
{
    (void)trim;
    return WindowsDuplicateString(string, result);
}

HRESULT WINAPI WindowsPreallocateStringBuffer(
    UINT32 length, WCHAR **buffer, void **handle)
{
    Win32CraftHStringBuffer *memory;
    SIZE_T bytes;
    if (!buffer || !handle) return E_POINTER;
    *buffer = NULL;
    *handle = NULL;
    bytes = sizeof(Win32CraftHString) + sizeof(DWORD) +
            ((SIZE_T)length + 1) * sizeof(WCHAR);
    memory = (Win32CraftHStringBuffer *)HeapAlloc(
        GetProcessHeap(), 0, bytes);
    if (!memory) return E_OUTOFMEMORY;
    memory->value.magic = WIN32CRAFT_HSTRING_MAGIC;
    memory->value.flags = WIN32CRAFT_HSTRING_OWNED;
    memory->value.length = length;
    memory->value.text = memory->text;
    memory->reserved = 0;
    memory->text[length] = 0;
    *buffer = memory->text;
    *handle = memory;
    return S_OK;
}

HRESULT WINAPI WindowsPromoteStringBuffer(void *handle, HSTRING *result)
{
    if (!handle || !result) return E_POINTER;
    *result = (HSTRING)handle;
    return S_OK;
}

HRESULT WINAPI WindowsDeleteStringBuffer(void *handle)
{
    if (handle) HeapFree(GetProcessHeap(), 0, handle);
    return S_OK;
}

HRESULT WINAPI RoInitialize(UINT type)
{
    typedef HRESULT (WINAPI *Fn)(UINT);
    static Fn function;
    static BOOL resolved;
    if (!resolved) {
        HMODULE module;
        resolved = TRUE;
        module = LoadLibraryW(L"combase.dll");
        if (module) function = (Fn)GetProcAddress(module, "RoInitialize");
    }
    return function ? function(type) : S_OK;
}

void WINAPI RoUninitialize(void)
{
    typedef void (WINAPI *Fn)(void);
    static Fn function;
    static BOOL resolved;
    if (!resolved) {
        HMODULE module;
        resolved = TRUE;
        module = LoadLibraryW(L"combase.dll");
        if (module) function = (Fn)GetProcAddress(module, "RoUninitialize");
    }
    if (function) function();
}
HRESULT WINAPI RoActivateInstance(HSTRING name, void **instance)
{ (void)name; if (instance) *instance = NULL; return REGDB_E_CLASSNOTREG; }
HRESULT WINAPI RoGetActivationFactory(HSTRING name, const GUID *iid, void **factory)
{ (void)name; (void)iid; if (factory) *factory = NULL; return REGDB_E_CLASSNOTREG; }
HRESULT WINAPI RoGetApartmentIdentifier(ULONGLONG *id)
{ if (!id) return E_POINTER; *id = 1; return S_OK; }
HRESULT WINAPI RoGetBufferMarshaler(void **value)
{ if (value) *value = NULL; return E_NOTIMPL; }
HRESULT WINAPI RoGetParameterizedTypeInstanceIID(
    UINT32 count, const WCHAR **names, const void *locator, GUID *iid, void **extra)
{ (void)count; (void)names; (void)locator; (void)iid; if (extra) *extra = NULL;
  return WIN32CRAFT_RO_E_METADATA_NAME_NOT_FOUND; }
void WINAPI RoFreeParameterizedTypeExtra(void *extra) { (void)extra; }
HRESULT WINAPI RoRegisterActivationFactories(
    HSTRING *classes, void **callbacks, UINT32 count, void **cookie)
{ (void)classes; (void)callbacks; (void)count; if (cookie) *cookie = NULL;
  return E_NOTIMPL; }
void WINAPI RoRevokeActivationFactories(void *cookie) { (void)cookie; }
HRESULT WINAPI RoRegisterForApartmentShutdown(void *callback, ULONGLONG *id, void **cookie)
{ (void)callback; if (id) *id = 1; if (cookie) *cookie = NULL; return E_NOTIMPL; }
HRESULT WINAPI RoUnregisterForApartmentShutdown(void *cookie)
{ (void)cookie; return S_OK; }

HRESULT WINAPI GetRestrictedErrorInfo(void **info)
{ if (!info) return E_POINTER; *info = NULL; return WIN32CRAFT_S_FALSE; }
HRESULT WINAPI SetRestrictedErrorInfo(void *info) { (void)info; return S_OK; }
BOOL WINAPI RoOriginateError(HRESULT error, HSTRING message)
{ (void)error; (void)message; return FALSE; }
BOOL WINAPI RoOriginateErrorW(HRESULT error, UINT32 length, LPCWSTR message)
{ (void)error; (void)length; (void)message; return FALSE; }
BOOL WINAPI RoTransformError(HRESULT old_error, HRESULT new_error, HSTRING message)
{ (void)old_error; (void)new_error; (void)message; return FALSE; }
HRESULT WINAPI RoCaptureErrorContext(HRESULT error) { (void)error; return S_OK; }
void WINAPI RoFailFastWithErrorContext(HRESULT error) { (void)error; }
HRESULT WINAPI RoGetErrorReportingFlags(UINT32 *flags)
{ if (!flags) return E_POINTER; *flags = 0; return S_OK; }
HRESULT WINAPI RoSetErrorReportingFlags(UINT32 flags) { (void)flags; return S_OK; }
HRESULT WINAPI RoReportFailedDelegate(void *delegate, void *error)
{ (void)delegate; (void)error; return S_OK; }
HRESULT WINAPI RoReportUnhandledError(void *error) { (void)error; return S_OK; }

void WINAPI ProcessPendingGameUI(void) {}
HRESULT WINAPI ShowGameInviteUI(void *a, void *b, void *c, void *d, void *e, void *f)
{ (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; return E_NOTIMPL; }
HRESULT WINAPI ShowProfileCardUI(void *a, void *b, void *c, void *d)
{ (void)a; (void)b; (void)c; (void)d; return E_NOTIMPL; }
HRESULT WINAPI ShowTitleAchievementsUI(void *a, void *b, void *c, void *d)
{ (void)a; (void)b; (void)c; (void)d; return E_NOTIMPL; }

DWORD WINAPI Win32CraftXInputGetState(DWORD user_index, void *state)
{
    typedef DWORD (WINAPI *XInputGetStateFn)(DWORD, void *);
    static XInputGetStateFn function;
    static BOOL resolved;
    static const WCHAR *const names[] = {
        L"xinput1_4.dll", L"xinput1_3.dll", L"xinput9_1_0.dll"
    };
    UINT index;
    if (!resolved) {
        resolved = TRUE;
        for (index = 0; index < ARRAYSIZE(names) && !function; ++index) {
            HMODULE module = LoadLibraryW(names[index]);
            if (module)
                function = (XInputGetStateFn)GetProcAddress(
                    module, "XInputGetState");
        }
    }
    return function ? function(user_index, state)
                    : WIN32CRAFT_ERROR_DEVICE_NOT_CONNECTED;
}
