/* 1.16.20.03-specific implementation, included from win32_host.c. */
static FakeInspectable pnp_async_116, pnp_info_116, pnp_factory_116;
static FakeInspectable navigation_factory_116, navigation_manager_116;
static FakeInspectable coreapp_view_116, activation_args_116;
static FakeInspectable mouse_capabilities_factory_116, mouse_capabilities_116;
static FakeInspectable pointer_event_args_116, pointer_point_116;
static FakeInspectable pointer_properties_116, pointer_device_116;
static FakeInspectable mouse_event_args_116;
static FakeInspectable key_event_args_116, character_event_args_116;
static FakeInspectable size_event_args_116;
static void *activated_handler_116;
static void *original_report_unobserved_exception_116;
static void *original_command_arguments_116;
static float pointer_x_116, pointer_y_116;
static BYTE pointer_left_116, pointer_right_116, pointer_middle_116;
static INT pointer_update_kind_116, pointer_wheel_delta_116;
static UINT pointer_frame_116;
typedef struct PointerPropertiesState116 {
    FakeInspectable inspect;
    BYTE left, right, middle;
    INT kind, wheel;
    float x, y;
} PointerPropertiesState116;
typedef struct PointerPointState116 {
    FakeInspectable inspect;
    PointerPropertiesState116 *properties;
    float x, y;
    UINT frame;
} PointerPointState116;
static PointerPointState116 pointer_points_116[32];
static PointerPropertiesState116 pointer_property_states_116[32];
static PointerPointState116 *pointer_current_point_116;
static INT mouse_delta_x_116, mouse_delta_y_116;
static INT key_virtual_116;
static DWORD key_lparam_116;
static UINT character_code_116;
static float window_width_116 = 1280.0f, window_height_116 = 720.0f;
static BOOL mouse_relative_requested_116;
static BOOL mouse_relative_active_116;
static BOOL mouse_recentering_116;
static BOOL text_ctrl_down_116;
static WPARAM text_ctrl_handled_key_116;
static void *pointer_cursor_116;
static FakeInspectable default_pointer_cursor_116;
static const void *default_pointer_cursor_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level
};
static BYTE *scene_image_116, *client_instance_116;
static BYTE *cursor_request_scene_116;
static BOOL cursor_request_pending_116;
static void update_relative_mouse_116(HWND window);


/* The verified 1.16 ClientInstance vtable has the local SceneStack getter
 * at +0x338. Capture the live client through this call, without memory
 * searches. getTopScene (+0x320) supplies the global-stack fallback.
 * UIScene +0xbc/+0xc0 project is_showing_menu/should_steal_mouse.
 * Destruction invalidates the pointer before the original destructor runs. */
static void *__attribute__((thiscall)) client_scene_stack_116(BYTE *client)
{
    client_instance_116 = client;
    return *(void **)(client + 0x340);
}

static void *__attribute__((thiscall)) client_destroy_116(void *client, UINT flags)
{
    typedef void *(__attribute__((thiscall)) *DestroyFn)(void *, UINT);
    if (client_instance_116 == client) {
        client_instance_116 = NULL;
        cursor_request_scene_116 = NULL;
        cursor_request_pending_116 = FALSE;
    }
    return ((DestroyFn)(scene_image_116 + 0x0010f440))(client, flags);
}

static void refresh_scene_mouse_116(void)
{
    typedef BYTE *(__attribute__((thiscall)) *TopSceneFn)(void *);
    typedef BYTE (__attribute__((thiscall)) *FlagFn)(void *);
    BYTE *scene;
    BOOL gameplay = FALSE;
    static int previous = -1;
    if (!client_instance_116 || IsBadReadPtr(client_instance_116, 0x344) ||
        *(void **)client_instance_116 != scene_image_116 + 0x024e7fac) return;
    scene = ((TopSceneFn)(scene_image_116 + 0x0011a740))(client_instance_116);
    /* A native cursor request during a closing animation takes effect
     * immediately. Keep it until that scene leaves the stack; otherwise
     * the menu flag would undo the game's hide request every frame. */
    if (cursor_request_pending_116 && scene == cursor_request_scene_116)
        return;
    cursor_request_pending_116 = FALSE;
    if (scene && !IsBadReadPtr(scene, 0x1c) &&
        *(void **)scene == scene_image_116 + 0x025226d4) {
        gameplay = !((FlagFn)(scene_image_116 + 0x006a70d0))(scene) &&
                     ((FlagFn)(scene_image_116 + 0x006a7110))(scene);
    }
    if (previous != (int)gameplay) {
        log_line(gameplay ? "1.16 top scene: gameplay capture"
                          : "1.16 top scene: UI cursor");
        previous = gameplay;
    }
    if (mouse_relative_requested_116 != gameplay) {
        mouse_relative_requested_116 = gameplay;
        update_relative_mouse_116(game_window);
    }
}

static BOOL mouse_client_center_116(HWND window, POINT *client_center,
                                    POINT *screen_center)
{
    RECT client;
    POINT center;
    if (!window || !GetClientRect(window, &client)) return FALSE;
    center.x = (client.right - client.left) / 2;
    center.y = (client.bottom - client.top) / 2;
    if (client_center) *client_center = center;
    if (screen_center) {
        *screen_center = center;
        if (!ClientToScreen(window, screen_center)) return FALSE;
    }
    return TRUE;
}

static void update_relative_mouse_116(HWND window)
{
    BOOL active = mouse_relative_requested_116 &&
        GetForegroundWindow() == window && !IsIconic(window);
    RECT clip;
    POINT top_left, bottom_right, client_center, screen_center;

    if (active) {
        if (!GetClientRect(window, &clip)) return;
        top_left.x = clip.left; top_left.y = clip.top;
        bottom_right.x = clip.right; bottom_right.y = clip.bottom;
        if (!ClientToScreen(window, &top_left) ||
            !ClientToScreen(window, &bottom_right) ||
            !mouse_client_center_116(window, &client_center, &screen_center))
            return;
        clip.left = top_left.x; clip.top = top_left.y;
        clip.right = bottom_right.x; clip.bottom = bottom_right.y;
        ClipCursor(&clip);
        SetCapture(window);
        if (!mouse_relative_active_116) {
            mouse_relative_active_116 = TRUE;
            while (ShowCursor(FALSE) >= 0) {}
            log_line("1.16 relative mouse captured");
            mouse_recentering_116 = TRUE;
            SetCursorPos(screen_center.x, screen_center.y);
        }
        return;
    }

    if (mouse_relative_active_116) {
        mouse_relative_active_116 = FALSE;
        mouse_recentering_116 = FALSE;
        ClipCursor(NULL);
        if (GetCapture() == window) ReleaseCapture();
        while (ShowCursor(TRUE) < 0) {}
        SetCursor(LoadCursorW(NULL, MAKEINTRESOURCEW(32512)));
        log_line("1.16 relative mouse released");
    }
}

static HRESULT WINAPI core_window_pointer_cursor_get_116(
    void *object, void **result)
{
    void **table;
    (void)object;
    if (!result) return E_POINTER;
    *result = pointer_cursor_116;
    table = pointer_cursor_116 ? *(void ***)pointer_cursor_116 : NULL;
    if (table && table[1])
        ((ULONG (WINAPI *)(void *))table[1])(pointer_cursor_116);
    return S_OK;
}

static HRESULT WINAPI core_window_pointer_cursor_put_116(
    FakeInspectable *object, void *value)
{
    BOOL requested;
    void *old_value = pointer_cursor_116;
    void **table = value ? *(void ***)value : NULL;
    (void)object;
    if (table && table[1]) ((ULONG (WINAPI *)(void *))table[1])(value);
    pointer_cursor_116 = value;
    table = old_value ? *(void ***)old_value : NULL;
    if (table && table[2]) ((ULONG (WINAPI *)(void *))table[2])(old_value);
    requested = value == NULL;
    if (client_instance_116 && !IsBadReadPtr(client_instance_116, 0x344)) {
        typedef BYTE *(__attribute__((thiscall)) *TopSceneFn)(void *);
        cursor_request_scene_116 = ((TopSceneFn)(scene_image_116 + 0x0011a740))(
            client_instance_116);
        cursor_request_pending_116 = TRUE;
    }
    if (mouse_relative_requested_116 != requested) {
        mouse_relative_requested_116 = requested;
        log_line(requested ? "1.16 relative mouse requested"
                           : "1.16 absolute mouse requested");
    }
    update_relative_mouse_116(game_window);
    return S_OK;
}

static HRESULT WINAPI coreapp_view_window_116(void *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &win32_core_window;
    fake_add_ref(&win32_core_window);
    return S_OK;
}
static HRESULT WINAPI coreapp_view_add_activated_116(
    void *object, void *handler, LONGLONG *token)
{
    void **vtable;
    (void)object;
    if (!token) return E_POINTER;
    *token = 1;
    activated_handler_116 = handler;
    vtable = handler ? *(void ***)handler : NULL;
    if (vtable && vtable[1]) ((ULONG (WINAPI *)(void *))vtable[1])(handler);
    log_pointer("1.16 CoreApplicationView.Activated handler", handler);
    return S_OK;
}
static HRESULT WINAPI coreapp_view_remove_activated_116(void *object, LONGLONG token)
{ (void)object; (void)token; return S_OK; }
static HRESULT WINAPI activation_get_int_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI activation_get_null_116(void *object, void **result)
{ (void)object; if (!result) return E_POINTER; *result = NULL; return S_OK; }
static HRESULT WINAPI activation_get_empty_116(void *object, HSTRING *result)
{ (void)object; return WindowsCreateString(L"", 0, result); }
static HRESULT WINAPI coreapp_view_get_bool_116(void *object, BYTE *result)
{ (void)object; if (!result) return E_POINTER; *result = TRUE; return S_OK; }
static HRESULT WINAPI core_event_add_116(
    void *object, void *handler, LONGLONG *token)
{ (void)object; (void)handler; if (!token) return E_POINTER; *token = 1; return S_OK; }
static HRESULT WINAPI core_event_remove_116(void *object, LONGLONG token)
{ (void)object; (void)token; return S_OK; }
static HRESULT WINAPI core_pointer_add_moved_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_pointer_moved_handler_116, handler, token, "1.16 CoreWindow.PointerMoved handler"); }
static HRESULT WINAPI core_pointer_add_entered_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_pointer_entered_handler_116, handler, token, "1.16 CoreWindow.PointerEntered handler"); }
static HRESULT WINAPI core_pointer_add_pressed_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_pointer_pressed_handler_116, handler, token, "1.16 CoreWindow.PointerPressed handler"); }
static HRESULT WINAPI core_pointer_add_released_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_pointer_released_handler_116, handler, token, "1.16 CoreWindow.PointerReleased handler"); }
static HRESULT WINAPI core_pointer_add_wheel_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_pointer_wheel_handler_116, handler, token, "1.16 CoreWindow.PointerWheelChanged handler"); }
static HRESULT WINAPI core_character_add_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_character_handler_116, handler, token, "1.16 CoreWindow.CharacterReceived handler"); }
static HRESULT WINAPI core_key_down_add_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_key_down_handler_116, handler, token, "1.16 CoreWindow.KeyDown handler"); }
static HRESULT WINAPI core_key_up_add_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_key_up_handler_116, handler, token, "1.16 CoreWindow.KeyUp handler"); }
static HRESULT WINAPI core_size_add_116(void *object, void *handler, LONGLONG *token)
{ (void)object; return fake_core_window_event_add_116(&core_window_size_handler_116, handler, token, "1.16 CoreWindow.SizeChanged handler"); }
static HRESULT WINAPI core_window5_dispatcher_queue_116(void *object, void **result)
{ (void)object; if (!result) return E_POINTER; *result = NULL; return S_OK; }
static HRESULT WINAPI core_window5_activation_mode_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI dispatcher_priority_get_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI dispatcher_priority_put_116(void *object, INT value)
{ (void)object; (void)value; return S_OK; }
static HRESULT WINAPI dispatcher_priority_should_yield_116(
    void *object, BYTE *result)
{ (void)object; if (!result) return E_POINTER; *result = FALSE; return S_OK; }
static HRESULT WINAPI mouse_capabilities_activate_116(void *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &mouse_capabilities_116;
    fake_add_ref(&mouse_capabilities_116);
    log_line("1.16 MouseCapabilities activated");
    return S_OK;
}
static HRESULT WINAPI mouse_capabilities_present_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 1; return S_OK; }
static HRESULT WINAPI mouse_capabilities_absent_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI mouse_capabilities_buttons_116(void *object, UINT *result)
{ (void)object; if (!result) return E_POINTER; *result = 3; return S_OK; }

static const void *coreapp_view_table_116[12] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    coreapp_view_window_116, coreapp_view_add_activated_116,
    coreapp_view_remove_activated_116, coreapp_view_get_bool_116,
    coreapp_view_get_bool_116, fake_core_window_dispatcher
};
static const void *activation_args_table_116[14] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    activation_get_int_116, activation_get_int_116, activation_get_null_116,
    activation_get_empty_116, activation_get_empty_116,
    activation_get_int_116, activation_get_null_116, activation_get_null_116
};
static const void *core_accelerator_keys_table_116[8] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    core_event_add_116, core_event_remove_116
};
static const void *core_window5_table_116[8] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    core_window5_dispatcher_queue_116, core_window5_activation_mode_116
};
static const void *core_dispatcher_priority_table_116[9] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    dispatcher_priority_get_116, dispatcher_priority_put_116,
    dispatcher_priority_should_yield_116
};
static const void *mouse_capabilities_factory_table_116[7] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    mouse_capabilities_activate_116
};
static const void *mouse_capabilities_table_116[11] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    mouse_capabilities_present_116, mouse_capabilities_present_116,
    mouse_capabilities_absent_116, mouse_capabilities_absent_116,
    mouse_capabilities_buttons_116
};

/* The 1.16 input layer subscribes directly to ICoreWindow pointer events.
 * Model the small WinRT object graph inspected by those callbacks instead of
 * trying to reuse the much older MouseItem ABI used by 1.2.8. */
static HRESULT WINAPI pointer_args_current_116(void *object, void **result)
{ (void)object; if (!result || !pointer_current_point_116) return E_POINTER; *result = pointer_current_point_116; fake_add_ref(&pointer_current_point_116->inspect); return S_OK; }
static HRESULT WINAPI pointer_args_modifiers_116(void *object, UINT *result)
{ (void)object; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI pointer_args_handled_get_116(void *object, BYTE *result)
{ (void)object; if (!result) return E_POINTER; *result = FALSE; return S_OK; }
static HRESULT WINAPI pointer_args_handled_put_116(void *object, BYTE value)
{ (void)object; (void)value; return S_OK; }
static HRESULT WINAPI pointer_point_device_116(void *object, void **result)
{ (void)object; if (!result) return E_POINTER; *result = &pointer_device_116; fake_add_ref(&pointer_device_116); return S_OK; }
static HRESULT WINAPI pointer_point_uint_116(void *object, UINT *result)
{ (void)object; if (!result) return E_POINTER; *result = 1; return S_OK; }
static HRESULT WINAPI pointer_point_frame_116(void *object, UINT *result)
{ if (!result) return E_POINTER; *result = ((PointerPointState116 *)object)->frame; return S_OK; }
static HRESULT WINAPI pointer_point_timestamp_116(void *object, ULONGLONG *result)
{ if (!result) return E_POINTER; *result = (ULONGLONG)((PointerPointState116 *)object)->frame * 16000u; return S_OK; }
static HRESULT WINAPI pointer_point_position_116(void *object, float *result)
{ PointerPointState116 *state = object; if (!result) return E_POINTER; result[0] = state->x; result[1] = state->y; return S_OK; }
static HRESULT WINAPI pointer_point_contact_116(void *object, BYTE *result)
{ PointerPointState116 *state = object; if (!result) return E_POINTER; *result = state->properties->left || state->properties->right || state->properties->middle; return S_OK; }
static HRESULT WINAPI pointer_point_properties_116(void *object, void **result)
{ PointerPointState116 *state = object; if (!result) return E_POINTER; *result = state->properties; fake_add_ref(&state->properties->inspect); return S_OK; }
static HRESULT WINAPI pointer_property_kind_116(void *object, INT *result)
{ if (!result) return E_POINTER; *result = ((PointerPropertiesState116 *)object)->kind; return S_OK; }
static HRESULT WINAPI pointer_property_zero_float_116(void *object, float *result)
{ (void)object; if (!result) return E_POINTER; *result = 0.0f; return S_OK; }
static HRESULT WINAPI pointer_property_true_116(void *object, BYTE *result)
{ (void)object; if (!result) return E_POINTER; *result = TRUE; return S_OK; }
static HRESULT WINAPI pointer_property_false_116(void *object, BYTE *result)
{ (void)object; if (!result) return E_POINTER; *result = FALSE; return S_OK; }
static HRESULT WINAPI pointer_property_left_116(void *object, BYTE *result)
{ if (!result) return E_POINTER; *result = ((PointerPropertiesState116 *)object)->left; return S_OK; }
static HRESULT WINAPI pointer_property_right_116(void *object, BYTE *result)
{ if (!result) return E_POINTER; *result = ((PointerPropertiesState116 *)object)->right; return S_OK; }
static HRESULT WINAPI pointer_property_middle_116(void *object, BYTE *result)
{ if (!result) return E_POINTER; *result = ((PointerPropertiesState116 *)object)->middle; return S_OK; }
static HRESULT WINAPI pointer_property_wheel_116(void *object, INT *result)
{ if (!result) return E_POINTER; *result = ((PointerPropertiesState116 *)object)->wheel; return S_OK; }
static HRESULT WINAPI pointer_property_rect_116(void *object, float *result)
{ PointerPropertiesState116 *state = object; if (!result) return E_POINTER; result[0] = state->x; result[1] = state->y; result[2] = result[3] = 1.0f; return S_OK; }
static HRESULT WINAPI pointer_property_has_usage_116(
    void *object, UINT usage_page, UINT usage_id, BYTE *result)
{ (void)object; (void)usage_page; (void)usage_id; if (!result) return E_POINTER; *result = FALSE; return S_OK; }
static HRESULT WINAPI pointer_property_usage_value_116(
    void *object, UINT usage_page, UINT usage_id, INT *result)
{ (void)object; (void)usage_page; (void)usage_id; if (!result) return E_POINTER; *result = 0; return S_OK; }
static HRESULT WINAPI pointer_device_type_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = 2; return S_OK; }
static HRESULT WINAPI pointer_device_contacts_116(void *object, UINT *result)
{ (void)object; if (!result) return E_POINTER; *result = 1; return S_OK; }

static const void *pointer_event_args_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    pointer_args_current_116, pointer_args_modifiers_116,
    pointer_args_handled_get_116, pointer_args_handled_put_116
};
static const void *pointer_point_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    pointer_point_device_116, pointer_point_position_116,
    pointer_point_position_116, pointer_point_uint_116,
    pointer_point_frame_116, pointer_point_timestamp_116,
    pointer_point_contact_116,
    pointer_point_properties_116
};
static const void *pointer_properties_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    pointer_property_zero_float_116,
    pointer_property_false_116, pointer_property_false_116,
    pointer_property_zero_float_116, pointer_property_zero_float_116,
    pointer_property_zero_float_116, pointer_property_zero_float_116,
    pointer_property_rect_116, pointer_property_rect_116,
    pointer_property_true_116, pointer_property_left_116,
    pointer_property_right_116, pointer_property_middle_116,
    pointer_property_wheel_116, pointer_property_false_116,
    pointer_property_true_116, pointer_property_true_116,
    pointer_property_false_116, pointer_property_false_116,
    pointer_property_false_116, pointer_property_false_116,
    pointer_property_kind_116, pointer_property_has_usage_116,
    pointer_property_usage_value_116
};
static const void *pointer_device_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level, pointer_device_type_116,
    pointer_property_false_116, pointer_device_contacts_116,
    pointer_property_rect_116, pointer_property_rect_116
};
static HRESULT WINAPI mouse_args_delta_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; result[0] = mouse_delta_x_116; result[1] = mouse_delta_y_116; return S_OK; }
static const void *mouse_event_args_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level, mouse_args_delta_116
};
static HRESULT WINAPI key_args_virtual_116(void *object, INT *result)
{ (void)object; if (!result) return E_POINTER; *result = key_virtual_116; return S_OK; }
static HRESULT WINAPI key_args_status_116(void *object, void *result)
{
    BYTE *status = result;
    (void)object;
    if (!status) return E_POINTER;
    ZeroMemory(status, 12);
    *(UINT *)(status + 0) = (key_lparam_116 & 0xffffu) ?
        (key_lparam_116 & 0xffffu) : 1u;
    *(UINT *)(status + 4) = (key_lparam_116 >> 16) & 0xffu;
    status[8] = (key_lparam_116 & (1u << 24)) != 0;
    status[9] = (key_lparam_116 & (1u << 29)) != 0;
    status[10] = (key_lparam_116 & (1u << 30)) != 0;
    status[11] = (key_lparam_116 & (1u << 31)) != 0;
    return S_OK;
}
static HRESULT WINAPI character_args_code_116(void *object, UINT *result)
{ (void)object; if (!result) return E_POINTER; *result = character_code_116; return S_OK; }
static HRESULT WINAPI size_args_size_116(void *object, float *result)
{ (void)object; if (!result) return E_POINTER; result[0] = window_width_116; result[1] = window_height_116; return S_OK; }
static HRESULT WINAPI size_args_handled_116(void *object, BYTE value)
{ (void)object; (void)value; return S_OK; }
static const void *key_event_args_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    key_args_virtual_116, key_args_status_116
};
static const void *character_event_args_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    character_args_code_116, key_args_status_116
};
static const void *size_event_args_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release, fake_get_iids,
    fake_get_runtime_class_name, fake_get_trust_level,
    size_args_size_116, size_args_handled_116
};
static FakeInspectable input_pane_factory_116, input_pane_116, input_pane2_116;
static FakeInspectable pointer_visual_factory_116, pointer_visual_116;

static HRESULT WINAPI navigation_current_116(void *object, void **result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = &navigation_manager_116;
    fake_add_ref(&navigation_manager_116);
    return S_OK;
}
static HRESULT WINAPI navigation_get_visibility_116(void *object, int *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = 0;
    return S_OK;
}
static HRESULT WINAPI navigation_put_visibility_116(void *object, int value)
{ (void)object; (void)value; return S_OK; }
static HRESULT WINAPI navigation_add_back_116(void *object, void *handler,
                                              LONGLONG *token)
{ (void)object; (void)handler; if (token) *token = 1; return S_OK; }
static HRESULT WINAPI navigation_remove_back_116(void *object, LONGLONG token)
{ (void)object; (void)token; return S_OK; }
static const void *navigation_factory_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    navigation_current_116
};
static const void *navigation_manager_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    navigation_add_back_116, navigation_remove_back_116,
    navigation_get_visibility_116, navigation_put_visibility_116
};
static HRESULT WINAPI input_pane_current_116(void *object, void **result)
{ (void)object; if (!result) return E_POINTER; *result = &input_pane_116; fake_add_ref(&input_pane_116); return S_OK; }
static HRESULT WINAPI input_pane_add_116(void *object, void *handler, LONGLONG *token)
{ (void)object; (void)handler; if (token) *token = 1; return S_OK; }
static HRESULT WINAPI input_pane_remove_116(void *object, LONGLONG token)
{ (void)object; (void)token; return S_OK; }
static HRESULT WINAPI input_pane_rect_116(void *object, void *rect)
{ (void)object; if (!rect) return E_POINTER; ZeroMemory(rect, 16); return S_OK; }
static HRESULT WINAPI input_pane_try_116(void *object, BYTE *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = TRUE;
    text_input_active = TRUE;
    pending_high_surrogate = 0;
    return S_OK;
}
static HRESULT WINAPI input_pane_query_116(
    FakeInspectable *object, REFIID iid, void **result)
{
    static const GUID input_pane2_iid = {
        0x8a6b3f26, 0x7090, 0x4793,
        {0x94, 0x4c, 0xc3, 0xf2, 0xcd, 0xe2, 0x62, 0x76}
    };
    if (!result) return E_POINTER;
    *result = (iid && !memcmp(iid, &input_pane2_iid, sizeof(GUID)))
        ? (void *)&input_pane2_116 : (void *)object;
    fake_add_ref((FakeInspectable *)*result);
    return S_OK;
}
static const void *input_pane_factory_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    input_pane_current_116
};
static const void *input_pane_table_116[] = {
    input_pane_query_116, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    input_pane_add_116, input_pane_remove_116,
    input_pane_add_116, input_pane_remove_116,
    input_pane_rect_116
};
static const void *input_pane2_table_116[] = {
    input_pane_query_116, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    input_pane_try_116, input_pane_try_116
};
static HRESULT WINAPI pointer_visual_current_116(void *object, void **result)
{ (void)object; if (!result) return E_POINTER; *result = &pointer_visual_116; fake_add_ref(&pointer_visual_116); return S_OK; }
static HRESULT WINAPI pointer_visual_put_116(void *object, BYTE value)
{ (void)object; (void)value; return S_OK; }
static const void *pointer_visual_factory_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    pointer_visual_current_116
};
static const void *pointer_visual_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    pointer_visual_put_116, pointer_visual_put_116
};
static HRESULT WINAPI pnp_query_116(FakeInspectable *object, REFIID iid, void **result)
{
    if (!result) return E_POINTER;
    *result = iid && iid->Data1 == 0x36 ? &pnp_info_116 : object;
    return S_OK;
}
static HRESULT WINAPI pnp_completed_116(void *object, void *handler)
{
    (void)object;
    (void)handler;
    return S_OK;
}
static HRESULT WINAPI pnp_results_116(void *object, void **result)
{
    (void)object;
    if (result) *result = NULL;
    return result ? S_OK : E_POINTER;
}
static HRESULT WINAPI pnp_status_116(void *object, DWORD *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = 1;
    return S_OK;
}
static HRESULT WINAPI pnp_error_116(void *object, HRESULT *result)
{
    (void)object;
    if (!result) return E_POINTER;
    *result = S_OK;
    return S_OK;
}
static HRESULT WINAPI pnp_from_id_116(void *object, int type, void *id,
                                    void *properties, void **result)
{
    (void)object; (void)type; (void)id; (void)properties;
    if (!result) return E_POINTER;
    *result = &pnp_async_116;
    log_line("1.16 Pnp metadata query: unavailable on desktop");
    return S_OK;
}
static const void *pnp_async_table_116[] = {
    pnp_query_116, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    pnp_completed_116, fake_async_get_completed, pnp_results_116
};
static const void *pnp_info_table_116[] = {
    pnp_query_116, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_core_window_get_int, pnp_status_116, pnp_error_116,
    fake_core_window_noop, fake_core_window_noop
};
static const void *pnp_factory_table_116[] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    pnp_from_id_116
};
static BOOL activation_116(LPCWSTR name, const GUID *iid, void **result)
{
    (void)iid;
    if (name && result &&
        lstrcmpW(name, L"Windows.Devices.Input.MouseCapabilities") == 0) {
        mouse_capabilities_factory_116.vtable =
            mouse_capabilities_factory_table_116;
        mouse_capabilities_116.vtable = mouse_capabilities_table_116;
        mouse_capabilities_factory_116.references =
            mouse_capabilities_116.references = 1;
        *result = &mouse_capabilities_factory_116;
        log_line("redirected 1.16 MouseCapabilities");
        return TRUE;
    }
    if (name && result && lstrcmpW(name, L"Windows.UI.Input.PointerVisualizationSettings") == 0) {
        pointer_visual_factory_116.vtable = pointer_visual_factory_table_116;
        pointer_visual_116.vtable = pointer_visual_table_116;
        pointer_visual_factory_116.references = pointer_visual_116.references = 1;
        *result = &pointer_visual_factory_116;
        log_line("redirected 1.16 PointerVisualizationSettings");
        return TRUE;
    }
    if (name && result && lstrcmpW(name, L"Windows.UI.ViewManagement.InputPane") == 0) {
        input_pane_factory_116.vtable = input_pane_factory_table_116;
        input_pane_116.vtable = input_pane_table_116;
        input_pane2_116.vtable = input_pane2_table_116;
        input_pane_factory_116.references = input_pane_116.references =
            input_pane2_116.references = 1;
        *result = &input_pane_factory_116;
        log_line("redirected 1.16 InputPane");
        return TRUE;
    }
    if (name && result && lstrcmpW(name, L"Windows.UI.Core.SystemNavigationManager") == 0) {
        navigation_factory_116.vtable = navigation_factory_table_116;
        navigation_manager_116.vtable = navigation_manager_table_116;
        navigation_factory_116.references = navigation_manager_116.references = 1;
        *result = &navigation_factory_116;
        log_line("redirected 1.16 SystemNavigationManager");
        return TRUE;
    }
    if (name && result && lstrcmpW(name, L"Windows.Devices.Enumeration.Pnp.PnpObject") == 0) {
        pnp_factory_116.vtable = pnp_factory_table_116;
        pnp_async_116.vtable = pnp_async_table_116;
        pnp_info_116.vtable = pnp_info_table_116;
        pnp_factory_116.references = pnp_async_116.references = pnp_info_116.references = 1;
        *result = &pnp_factory_116;
        return TRUE;
    }
    return FALSE;
}
static HRESULT WINAPI coreapp_run_116(void *object, void *source)
{
    typedef HRESULT (WINAPI *CreateViewFn)(void *, void **);
    typedef HRESULT (WINAPI *ViewArgFn)(void *, void *);
    typedef HRESULT (WINAPI *ViewFn)(void *);
    void *view = NULL;
    void **table;
    HRESULT hr;
    (void)object;
    log_line("1.16 CoreApplication.Run: CreateView");
    table = *(void ***)source;
    hr = ((CreateViewFn)table[6])(source, &view);
    log_hresult("1.16 CreateView", hr);
    if (FAILED(hr) || !view) return FAILED(hr) ? hr : E_FAIL;
    table = *(void ***)view;
    log_pointer("1.16 FrameworkView object", view);
    log_pointer("1.16 FrameworkView.Initialize entry", table[6]);
    log_pointer("1.16 FrameworkView.SetWindow entry", table[7]);
    log_pointer("1.16 FrameworkView.Load entry", table[8]);
    log_pointer("1.16 FrameworkView.Run entry", table[9]);
    log_line("1.16 FrameworkView.Initialize");
    coreapp_view_116.vtable = coreapp_view_table_116;
    coreapp_view_116.references = 1;
    activation_args_116.vtable = activation_args_table_116;
    activation_args_116.references = 1;
    core_accelerator_keys_116.vtable = core_accelerator_keys_table_116;
    core_accelerator_keys_116.references = 1;
    core_window5_116.vtable = core_window5_table_116;
    core_window5_116.references = 1;
    core_dispatcher_priority_116.vtable = core_dispatcher_priority_table_116;
    core_dispatcher_priority_116.references = 1;
    core_window_activated_args_116.vtable =
        core_window_activated_args_table_116;
    core_window_activated_args_116.references = 1;
    core_window_visibility_args_116.vtable =
        core_window_visibility_args_table_116;
    core_window_visibility_args_116.references = 1;
    core_window_activated_handler_116 = NULL;
    core_window_visibility_handler_116 = NULL;
    activated_handler_116 = NULL;
    hr = ((ViewArgFn)table[6])(view, &coreapp_view_116);
    if (FAILED(hr)) return hr;
    log_line("1.16 FrameworkView.SetWindow");
    hr = ((ViewArgFn)table[7])(view, &win32_core_window);
    if (FAILED(hr)) return hr;
    log_line("1.16 FrameworkView.Load");
    hr = ((ViewArgFn)table[8])(view, NULL);
    if (FAILED(hr)) return hr;
    if (activated_handler_116) {
        typedef HRESULT (WINAPI *ActivatedInvokeFn)(void *, void *, void *);
        void **handler_table = *(void ***)activated_handler_116;
        log_line("1.16 delivering CoreApplicationView.Activated");
        hr = ((ActivatedInvokeFn)handler_table[3])(
            activated_handler_116, &coreapp_view_116, &activation_args_116);
        log_hresult("1.16 Activated handler", hr);
        if (FAILED(hr)) return hr;
    }
    log_line("1.16 FrameworkView.Run");
    return ((ViewFn)table[9])(view);
}

static const void *coreapp_vtable_116[15] = {
    fake_query_interface, fake_add_ref, fake_release,
    fake_get_iids, fake_get_runtime_class_name, fake_get_trust_level,
    fake_coreapp_id, fake_mouse_device_add_moved, fake_mouse_device_remove_moved,
    fake_mouse_device_add_moved, fake_mouse_device_remove_moved,
    fake_core_window_null_object, fake_core_window_current,
    coreapp_run_116, NULL
};

/* MSVCP's desktop PPL implementation terminates the process when an
 * exception holder is released without somebody observing its exception.
 * The UWP process policy used by this build does not map cleanly to Wine:
 * Wine's _ReportUnobservedException enters the unavailable UCRT
 * _invoke_watson path after optional startup tasks (resource/audio probes)
 * fail.  Those tasks are deliberately allowed to fall back on desktop, so
 * preserve that behaviour instead of aborting the whole game. */
static void __cdecl report_unobserved_exception_116(void)
{
    static LONG reports;
    if (InterlockedIncrement(&reports) <= 8)
        log_line("1.16 ignored unobserved optional PPL exception");
}

static void invoke_pointer_handler_116(void *handler, LPARAM lparam,
                                       INT update_kind, INT wheel_delta)
{
    typedef HRESULT (WINAPI *InvokeFn)(void *, void *, void *);
    void **table;
    static unsigned logged;
    HRESULT result;
    if (!handler) return;
    pointer_x_116 = (float)(short)LOWORD(lparam);
    pointer_y_116 = (float)(short)HIWORD(lparam);
    pointer_update_kind_116 = update_kind;
    pointer_wheel_delta_116 = wheel_delta;
    ++pointer_frame_116;
    {
        unsigned slot = pointer_frame_116 % 32;
        PointerPointState116 *point = &pointer_points_116[slot];
        PointerPropertiesState116 *properties = &pointer_property_states_116[slot];
        point->inspect.vtable = pointer_point_table_116;
        point->inspect.references = 1;
        point->properties = properties;
        point->x = pointer_x_116; point->y = pointer_y_116;
        point->frame = pointer_frame_116;
        properties->inspect.vtable = pointer_properties_table_116;
        properties->inspect.references = 1;
        properties->left = pointer_left_116;
        properties->right = pointer_right_116;
        properties->middle = pointer_middle_116;
        properties->kind = update_kind; properties->wheel = wheel_delta;
        properties->x = pointer_x_116; properties->y = pointer_y_116;
        pointer_current_point_116 = point;
    }
    table = *(void ***)handler;
    if (!table || !table[3]) return;
    result = ((InvokeFn)table[3])(handler, &win32_core_window,
                                  &pointer_event_args_116);
    if (logged++ < 16) {
        char line[128];
        wsprintfA(line, "1.16 pointer event: x=%d y=%d kind=%d wheel=%d hr=%08lX",
                  (int)pointer_x_116, (int)pointer_y_116, update_kind,
                  wheel_delta, (unsigned long)result);
        log_line(line);
    }
}

static void invoke_mouse_moved_116(INT delta_x, INT delta_y)
{
    typedef HRESULT (WINAPI *InvokeFn)(void *, void *, void *);
    unsigned i;
    if (!mouse_moved_handler_count_116 || (!delta_x && !delta_y)) return;
    mouse_delta_x_116 = delta_x;
    mouse_delta_y_116 = delta_y;
    for (i = 0; i < mouse_moved_handler_count_116; ++i) {
        void *handler = mouse_moved_handlers_116[i];
        void **table = handler ? *(void ***)handler : NULL;
        if (table && table[3])
            ((InvokeFn)table[3])(handler, &mouse_device,
                                &mouse_event_args_116);
    }
}

static void invoke_key_handler_116(void *handler, WPARAM key, LPARAM lparam)
{
    typedef HRESULT (WINAPI *InvokeFn)(void *, void *, void *);
    void **table = handler ? *(void ***)handler : NULL;
    key_virtual_116 = (INT)key;
    key_lparam_116 = (DWORD)lparam;
    if (table && table[3])
        ((InvokeFn)table[3])(handler, &win32_core_window,
                            &key_event_args_116);
}

static void invoke_character_handler_116(WPARAM character, LPARAM lparam)
{
    typedef HRESULT (WINAPI *InvokeFn)(void *, void *, void *);
    static unsigned logged_characters;
    void *handler = core_window_character_handler_116;
    void **table = handler ? *(void ***)handler : NULL;
    if (logged_characters < 32) {
        char line[128];
        wsprintfA(line,
            "1.16 CharacterReceived: code=U+%04lX handler=%p invoke=%p",
            (unsigned long)character, handler,
            table ? table[3] : NULL);
        log_line(line);
        ++logged_characters;
    }
    character_code_116 = (UINT)character;
    key_lparam_116 = (DWORD)lparam;
    if (table && table[3])
        ((InvokeFn)table[3])(handler, &win32_core_window,
                            &character_event_args_116);
}

static void invoke_size_handler_116(UINT width, UINT height)
{
    typedef HRESULT (WINAPI *InvokeFn)(void *, void *, void *);
    void *handler = core_window_size_handler_116;
    void **table = handler ? *(void ***)handler : NULL;
    window_width_116 = (float)width;
    window_height_116 = (float)height;
    if (table && table[3])
        ((InvokeFn)table[3])(handler, &win32_core_window,
                            &size_event_args_116);
}

static LRESULT CALLBACK window_proc_116(HWND window, UINT message,
                                        WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        game_window = NULL;
        PostQuitMessage(0);
        ExitProcess(0);
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (message == WM_SYSKEYDOWN && wparam == VK_F4 &&
            ((DWORD)lparam & (1u << 29)))
            return DefWindowProcW(window, message, wparam, lparam);
        if (wparam == 0x11 || wparam == 0xa2 || wparam == 0xa3) {
            text_ctrl_down_116 = TRUE;
        } else if (text_input_active && text_ctrl_down_116 &&
                   wparam == 'V') {
            log_line(paste_core_text_selection()
                ? "1.16 clipboard paste completed"
                : "1.16 clipboard paste unavailable");
            text_ctrl_handled_key_116 = wparam;
            return 0;
        }
        invoke_key_handler_116(core_window_key_down_handler_116,
                               wparam, lparam);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wparam == text_ctrl_handled_key_116) {
            text_ctrl_handled_key_116 = 0;
            return 0;
        }
        if (wparam == 0x11 || wparam == 0xa2 || wparam == 0xa3)
            text_ctrl_down_116 = FALSE;
        invoke_key_handler_116(core_window_key_up_handler_116,
                               wparam, lparam);
        return 0;
    case WM_CHAR:
        {
        INT selection_start = core_text_selection_start;
        INT selection_end = core_text_selection_end;
        invoke_character_handler_116(wparam, lparam);
        if (text_input_active && wparam >= 0x20 && wparam <= 0xffff) {
            HRESULT text_hr = dispatch_core_text_character(
                (UINT)wparam, selection_start, selection_end);
            if (text_hr != (HRESULT)1)
                log_hresult("1.16 CoreText TextUpdating", text_hr);
        }
        return 0;
        }
    case WM_UNICHAR:
        if (wparam == UNICODE_NOCHAR) return TRUE;
        {
        INT selection_start = core_text_selection_start;
        INT selection_end = core_text_selection_end;
        invoke_character_handler_116(wparam, lparam);
        if (text_input_active && wparam >= 0x20 && wparam <= 0xffff) {
            HRESULT text_hr = dispatch_core_text_character(
                (UINT)wparam, selection_start, selection_end);
            if (text_hr != (HRESULT)1)
                log_hresult("1.16 CoreText TextUpdating", text_hr);
        }
        return 0;
        }
    case WM_MOUSEMOVE:
        {
            static BOOL pointer_inside;
            INT x = (short)LOWORD(lparam), y = (short)HIWORD(lparam);
            POINT center;
            if (mouse_relative_active_116 &&
                mouse_client_center_116(window, &center, NULL)) {
                INT delta_x = x - center.x;
                INT delta_y = y - center.y;
                if (mouse_recentering_116 && !delta_x && !delta_y) {
                    mouse_recentering_116 = FALSE;
                    return 0;
                }
                mouse_recentering_116 = FALSE;
                invoke_mouse_moved_116(delta_x, delta_y);
                if (delta_x || delta_y) {
                    POINT screen_center;
                    if (mouse_client_center_116(
                            window, NULL, &screen_center)) {
                        mouse_recentering_116 = TRUE;
                        SetCursorPos(screen_center.x, screen_center.y);
                    }
                }
                return 0;
            }
            if (!pointer_inside) {
                pointer_inside = TRUE;
                invoke_pointer_handler_116(
                    core_window_pointer_entered_handler_116, lparam, 0, 0);
            }
        }
        invoke_pointer_handler_116(core_window_pointer_moved_handler_116, lparam, 0, 0); return 0;
    case WM_KILLFOCUS:
        update_relative_mouse_116(window); return 0;
    case WM_SETFOCUS:
        update_relative_mouse_116(window); return 0;
    case WM_MOVE:
        update_relative_mouse_116(window);
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED)
            invoke_size_handler_116(LOWORD(lparam), HIWORD(lparam));
        update_relative_mouse_116(window);
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_ACTIVATEAPP:
        update_relative_mouse_116(window); return 0;
    case WM_SETCURSOR:
        if (mouse_relative_active_116 && LOWORD(lparam) == HTCLIENT) {
            SetCursor(NULL);
            return TRUE;
        }
        return DefWindowProcW(window, message, wparam, lparam);
    case WM_LBUTTONDOWN:
        SetFocus(window); SetCapture(window); pointer_left_116 = TRUE;
        invoke_pointer_handler_116(core_window_pointer_pressed_handler_116, lparam, 1, 0); return 0;
    case WM_LBUTTONUP:
        pointer_left_116 = FALSE;
        invoke_pointer_handler_116(core_window_pointer_released_handler_116, lparam, 2, 0);
        if (!mouse_relative_active_116 && GetCapture() == window)
            ReleaseCapture(); return 0;
    case WM_RBUTTONDOWN:
        SetFocus(window); SetCapture(window); pointer_right_116 = TRUE;
        invoke_pointer_handler_116(core_window_pointer_pressed_handler_116, lparam, 3, 0); return 0;
    case WM_RBUTTONUP:
        pointer_right_116 = FALSE;
        invoke_pointer_handler_116(core_window_pointer_released_handler_116, lparam, 4, 0);
        if (!mouse_relative_active_116 && GetCapture() == window)
            ReleaseCapture(); return 0;
    case WM_MBUTTONDOWN:
        SetFocus(window); SetCapture(window); pointer_middle_116 = TRUE;
        invoke_pointer_handler_116(core_window_pointer_pressed_handler_116, lparam, 5, 0); return 0;
    case WM_MBUTTONUP:
        pointer_middle_116 = FALSE;
        invoke_pointer_handler_116(core_window_pointer_released_handler_116, lparam, 6, 0);
        if (!mouse_relative_active_116 && GetCapture() == window)
            ReleaseCapture(); return 0;
    case WM_MOUSEWHEEL: {
        invoke_pointer_handler_116(core_window_pointer_wheel_handler_116,
            (LPARAM)(DWORD)((WORD)(int)pointer_x_116 |
                ((DWORD)(WORD)(int)pointer_y_116 << 16)),
            0, (SHORT)HIWORD(wparam));
        return 0;
    }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

static wchar_t **__cdecl command_arguments_116(int *count)
{
    typedef wchar_t **(WINAPI *ParseFn)(LPCWSTR, int *);
    HMODULE shell = LoadLibraryW(L"shell32.dll");
    ParseFn parse = shell ? (ParseFn)GetProcAddress(shell, "CommandLineToArgvW") : NULL;
    static wchar_t *empty_arguments[1];
    wchar_t **arguments;
    if (!count) return NULL;
    *count = 0;
    arguments = parse ? parse(GetCommandLineW(), count) : NULL;
    return arguments ? arguments : empty_arguments;
}

/* FrameworkView::Run calls AppMain slot +0x68 with render=true. Preserve
 * its thiscall ABI and result, then synchronize the completed scene and
 * publish its backbuffer. This avoids presenting the previous frame before
 * input and scene updates have taken effect. */
static BYTE __attribute__((thiscall)) frame_116(void *app, BOOL render)
{
    typedef BYTE (__attribute__((thiscall)) *FrameFn)(void *, BOOL);
    static unsigned frames;
    BYTE result = ((FrameFn)(*(void ***)app)[0x68 / 4])(app, render);
    refresh_scene_mouse_116();
    if (game_swap_chain) {
        HRESULT hr = IDXGISwapChain_Present(
            game_swap_chain, Win32CraftPresentSyncInterval(), 0);
        if (++frames == 1 || FAILED(hr))
            log_hresult("1.16 completed frame Present", hr);
    }
    return result;
}

static BOOL install_frame_116(BYTE *image)
{
    static const BYTE expected[] = {0x8b,0x01,0x8b,0x40,0x68,0xff,0xd0};
    BYTE *site = image + 0x016e469e;
    DWORD protection, ignored;
    if (memcmp(site, expected, sizeof(expected))) {
        log_line("1.16 frame callsite signature mismatch");
        return FALSE;
    }
    if (!VirtualProtect(site, sizeof(expected), PAGE_EXECUTE_READWRITE,
                        &protection)) return FALSE;
    site[0] = 0xe8;
    *(INT *)(site + 1) = (INT)((BYTE *)frame_116 - (site + 5));
    site[5] = site[6] = 0x90;
    VirtualProtect(site, sizeof(expected), protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(expected));
    return TRUE;
}

static BOOL install_development_label_116(BYTE *image)
{
    static const BYTE expected[] = {0xc6,0x45,0xa4,0x01};
    BYTE *site = image + 0x0183c94a;
    DWORD protection, ignored;

    if (memcmp(site, expected, sizeof(expected))) {
        log_line("1.16 $is_publish signature mismatch");
        return FALSE;
    }
    if (!VirtualProtect(site, sizeof(expected), PAGE_EXECUTE_READWRITE,
                        &protection))
        return FALSE;
    site[3] = 0;
    VirtualProtect(site, sizeof(expected), protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), site, sizeof(expected));
    log_line("Minecraft 1.16 development label enabled");
    return TRUE;
}

typedef BOOL (__attribute__((thiscall)) *DirectoryGetContentsFn116)(
    void *, const GameStringX86 *, GameStringX86 *, BOOL);
typedef void (__attribute__((thiscall)) *GameStringDestroyFn116)(
    GameStringX86 *);
static DirectoryGetContentsFn116 original_directory_get_contents_116;
static DirectoryGetContentsFn116 original_zip_get_contents_116;
static volatile LONG language_override_log_116;
static volatile LONG start_screen_override_log_116;

static void override_language_contents_116(
    const GameStringX86 *path, GameStringX86 *output)
{
    static const BYTE copyright_key[] = "menu.copyright=";
    static const BYTE copyright_replacement[] = COPYRIGHT_OVERRIDE_LINE_A;
    Game128StringAssignFn construct =
        (Game128StringAssignFn)(scene_image_116 + 0x000dffc0);
    GameStringDestroyFn116 destroy =
        (GameStringDestroyFn116)(scene_image_116 + 0x000e0280);
    const char *contents;
    BYTE *replacement = NULL;
    DWORD replacement_size = 0;
    if (resource_path_has_suffix(path, "start_screen.json")) {
        contents = game_string_data(output);
        if (!contents || IsBadReadPtr(contents, output->length)) return;
        if (start_screen_force_development_control(
                (const BYTE *)contents, output->length,
                &replacement, &replacement_size)) {
            destroy(output);
            construct(output, (const char *)replacement, replacement_size);
            HeapFree(GetProcessHeap(), 0, replacement);
            if (InterlockedIncrement(&start_screen_override_log_116) == 1)
                log_line("Minecraft 1.16 development text overridden");
        }
        return;
    }
    if (!resource_path_has_suffix(path, ".lang")) return;
    contents = game_string_data(output);
    if (!contents || IsBadReadPtr(contents, output->length)) return;
    if (lang_apply_line_override(
            (const BYTE *)contents, output->length,
            copyright_key, sizeof(copyright_key) - 1,
            copyright_replacement, sizeof(copyright_replacement) - 1,
            &replacement, &replacement_size)) {
        destroy(output);
        construct(output, (const char *)replacement, replacement_size);
        HeapFree(GetProcessHeap(), 0, replacement);
        if (InterlockedIncrement(&language_override_log_116) == 1)
            log_line("Minecraft 1.16 copyright text overridden");
    }
}

static BOOL __attribute__((thiscall)) directory_get_contents_116(
    void *self, const GameStringX86 *path, GameStringX86 *output,
    BOOL require_valid_pack)
{
    BOOL result = original_directory_get_contents_116(
        self, path, output, require_valid_pack);
    if (result) override_language_contents_116(path, output);
    return result;
}

static BOOL __attribute__((thiscall)) zip_get_contents_116(
    void *self, const GameStringX86 *path, GameStringX86 *output,
    BOOL require_valid_pack)
{
    BOOL result = original_zip_get_contents_116(
        self, path, output, require_valid_pack);
    if (result) override_language_contents_116(path, output);
    return result;
}

static BOOL install_language_override_116(BYTE *image)
{
    void **slot = (void **)(image + 0x02551b30);
    void *expected = image + 0x00f23070;
    void *original;

    if (*slot != expected) {
        log_line("1.16 DirectoryPack getContents signature mismatch");
        return FALSE;
    }
    if (!patch_115_iat_slot(slot, directory_get_contents_116, &original,
                            "1.16 DirectoryPack language override"))
        return FALSE;
    original_directory_get_contents_116 =
        (DirectoryGetContentsFn116)original;
    slot = (void **)(image + 0x02552348);
    expected = image + 0x00f64550;
    if (*slot != expected) {
        log_line("1.16 ZipPack getContents signature mismatch");
        return FALSE;
    }
    if (!patch_115_iat_slot(slot, zip_get_contents_116, &original,
                            "1.16 ZipPack language override"))
        return FALSE;
    original_zip_get_contents_116 = (DirectoryGetContentsFn116)original;
    return TRUE;
}

typedef void (STDMETHODCALLTYPE *ClearViewFn116)(
    ID3D11DeviceContext *, void *, const float *, const RECT *, UINT);
static ClearViewFn116 original_clear_view_116;
static const GUID iid_context1_116 = {0xbb2c6faa,0xb5fb,0x4082,
    {0x8e,0x6b,0x38,0x8b,0x8c,0xfa,0x90,0xe1}};
static const GUID iid_rtv_116 = {0xdfdba067,0x0b8d,0x4865,
    {0x87,0x5b,0xd7,0xb4,0x51,0x6c,0xc1,0x64}};

/* Wine's Context1 ClearView is a stub. A full-surface floating/UNORM
 * render-target clear has the same semantics as the inherited D3D11
 * ClearRenderTargetView, including multisampled views. Partial rectangles
 * and other view types remain on the original path. Never expand a partial
 * clear into a full one. */
static void STDMETHODCALLTYPE clear_view_116(ID3D11DeviceContext *context,
    void *view, const float *color, const RECT *rects, UINT count)
{
    IUnknownLike *rtv = NULL, *resource = NULL;
    UINT texture[11], description[5], type, width, height, i;
    BOOL full = rects == NULL;
    static unsigned traced;
    if (view && SUCCEEDED(ID3D11Device_QueryInterface(
            (IUnknownLike *)view, &iid_rtv_116, (void **)&rtv))) {
        COMCALL(rtv,7,void,STDMETHODCALLTYPE,void **)(rtv, (void **)&resource);
        if (resource) {
            COMCALL(resource,7,void,STDMETHODCALLTYPE,UINT *)(resource, &type);
            if (type == 3) {
                COMCALL(resource,10,void,STDMETHODCALLTYPE,UINT *)(resource, texture);
                COMCALL(rtv,8,void,STDMETHODCALLTYPE,UINT *)(rtv, description);
                width = texture[0]; height = texture[1];
                if (description[1] == 4 || description[1] == 5) {
                    width >>= description[2]; height >>= description[2];
                    if (!width) width = 1;
                    if (!height) height = 1;
                }
                for (i = 0; rects && i < count; ++i)
                    if (rects[i].left <= 0 && rects[i].top <= 0 &&
                        rects[i].right >= (LONG)width &&
                        rects[i].bottom >= (LONG)height) full = TRUE;
                if (traced++ < 8) {
                    char line[160];
                    wsprintfA(line, "1.16 ClearView: %ux%u samples=%u format=%u rect=%d,%d,%d,%d full=%u",
                        width, height, texture[5], description[0],
                        rects && count ? rects[0].left : 0,
                        rects && count ? rects[0].top : 0,
                        rects && count ? rects[0].right : 0,
                        rects && count ? rects[0].bottom : 0, full);
                    log_line(line);
                }
                if (full && (description[0] == 87 || description[0] == 28 ||
                             description[0] == 10 || description[0] == 2)) {
                    ID3D11DeviceContext_ClearRenderTargetView(context, rtv, color);
                    ID3D11Texture2D_Release(resource);
                    ID3D11RenderTargetView_Release(rtv);
                    return;
                }
            }
            ID3D11Texture2D_Release(resource);
        }
        ID3D11RenderTargetView_Release(rtv);
    }
    original_clear_view_116(context, view, color, rects, count);
}

static void install_clear_view_116(ID3D11DeviceContext *context)
{
    ID3D11DeviceContext *extended = NULL;
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    void *original;
    if (!ntdll || !GetProcAddress(ntdll, "wine_get_version")) return;
    if (SUCCEEDED(ID3D11Device_QueryInterface(context, &iid_context1_116,
                                            (void **)&extended))) {
        void **slot = &extended->lpVtbl[132];
        if (*slot != clear_view_116 && patch_115_iat_slot(
                slot, clear_view_116, &original, "1.16 Wine ClearView"))
            original_clear_view_116 = (ClearViewFn116)original;
        ID3D11DeviceContext_Release(extended);
    }
}

static int bootstrap_116(BYTE *image, HINSTANCE instance)
{
    typedef int (__cdecl *OriginalMainFn)(void);
    WNDCLASSEXW klass;
    int result;
    host_is_116 = TRUE;
    {
        typedef DWORD (WINAPI *GetVersionFn)(void);
        GetVersionFn get_version = (GetVersionFn)GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"), "GetVersion");
        DWORD version = get_version ? get_version() : 0;
        host_is_windows7 =
            (version & 0xffu) == 6 && ((version >> 8) & 0xffu) == 1;
    }
    /* CoreWindow starts with an arrow, not NULL. The game saves this
     * object before hiding the cursor, then restores it when opening UI.
     * Starting with NULL makes that restore request relative mode again. */
    default_pointer_cursor_116.vtable = default_pointer_cursor_table_116;
    default_pointer_cursor_116.references = 1;
    pointer_cursor_116 = &default_pointer_cursor_116;
    scene_image_116 = image;
    if (!install_frame_116(image)) return 3;
    if (!install_development_label_116(image)) return 3;
    if (!install_language_override_116(image)) return 3;
    {
        void *original;
        void **slot = (void **)(image + 0x024e7fac + 0x338);
        if (*slot != image + 0x0011a7f0 ||
            !patch_115_iat_slot(slot, client_scene_stack_116, &original,
                                "1.16 ClientInstance scene stack")) return 3;
        slot = (void **)(image + 0x024e7fac);
        if (*slot != image + 0x0010f440 ||
            !patch_115_iat_slot(slot, client_destroy_116, &original,
                                "1.16 ClientInstance destruction")) return 3;
    }
    log_line("Win32Craft 1.16.20.03 bootstrap entered");
    install_crash_trace();
    install_exception_trace(image, 0x01ea6654);
    log_hresult("RoInitialize 1.16", RoInitialize(RO_INIT_MULTITHREADED));
    if (!GetCurrentDirectoryW(MAX_PATH, package_path)) lstrcpyW(package_path, L".");
    create_user_data();
    install_activation_redirect(image, 0x01ea6bcc);
    coreapp_factory.vtable = coreapp_vtable_116;
    install_vccorlib_compat(image, 0x01ea6bc4, 0x01ea6c40,
        0x01ea6c6c, 0x01ea6c8c, 0x01ea6bf4);
    patch_115_iat_slot((void **)(image + 0x01ea6c60),
        command_arguments_116, &original_command_arguments_116,
        "1.16 GetCmdArguments");
    install_d3d_compile_trace(image, 0x01ea601c);
    if (host_is_windows7) {
        GUID *factory_iid = (GUID *)(image + 0x0204244c);
        DWORD old_protection;
        DWORD ignored;
        if (VirtualProtect(factory_iid, sizeof(*factory_iid), PAGE_READWRITE,
                           &old_protection)) {
            memcpy(factory_iid, &IID_IDXGIFactory, sizeof(*factory_iid));
            VirtualProtect(factory_iid, sizeof(*factory_iid), old_protection,
                           &ignored);
            log_line("Windows 7 DXGI: IDXGIFactory4 probe downgraded to IDXGIFactory");
        }
    }
    install_d3d11_device_compat(image, 0x01ea6b90);
    if (!resolve_fmod_delay_imports_116(image)) {
        log_line("Minecraft 1.16 sound disabled: incompatible or missing fmod.dll");
    }
    patch_115_iat_slot((void **)(image + 0x01ea6940),
        win32_115_fopen, (void **)&original_115_fopen, "1.16 fopen");
    patch_115_iat_slot((void **)(image + 0x01ea6948),
        win32_115_wfopen, (void **)&original_115_wfopen, "1.16 _wfopen");
    patch_115_iat_slot((void **)(image + 0x01ea699c),
        win32_115_wfopen_s, (void **)&original_115_wfopen_s,
        "1.16 _wfopen_s");
    patch_115_iat_slot((void **)(image + 0x01ea6978),
        win32_115_vsscanf, (void **)&original_115_vsscanf,
        "1.16 __stdio_common_vsscanf");
    patch_115_iat_slot((void **)(image + 0x01ea68d0),
        win32_115_invalid_parameter,
        (void **)&original_115_invalid_parameter,
        "1.16 _invalid_parameter_noinfo");
    patch_115_iat_slot((void **)(image + 0x01ea68c8),
        win32_115_invalid_parameter_noreturn,
        (void **)&original_115_invalid_parameter_noreturn,
        "1.16 _invalid_parameter_noinfo_noreturn");
    patch_115_iat_slot((void **)(image + 0x01ea68c0),
        win32_115_terminate, (void **)&original_115_terminate,
        "1.16 terminate");
    patch_115_iat_slot((void **)(image + 0x01ea663c),
        win32_115_std_terminate, (void **)&original_115_std_terminate,
        "1.16 __std_terminate");
    patch_115_iat_slot((void **)(image + 0x01ea6584),
        report_unobserved_exception_116,
        &original_report_unobserved_exception_116,
        "1.16 _ReportUnobservedException");
    pointer_event_args_116.vtable = pointer_event_args_table_116;
    pointer_point_116.vtable = pointer_point_table_116;
    pointer_properties_116.vtable = pointer_properties_table_116;
    pointer_device_116.vtable = pointer_device_table_116;
    mouse_event_args_116.vtable = mouse_event_args_table_116;
    key_event_args_116.vtable = key_event_args_table_116;
    character_event_args_116.vtable = character_event_args_table_116;
    size_event_args_116.vtable = size_event_args_table_116;
    pointer_event_args_116.references = pointer_point_116.references = 1;
    pointer_properties_116.references = pointer_device_116.references = 1;
    mouse_event_args_116.references = 1;
    key_event_args_116.references = character_event_args_116.references = 1;
    size_event_args_116.references = 1;
    ZeroMemory(&klass, sizeof(klass));
    klass.cbSize = sizeof(klass);
    klass.lpfnWndProc = window_proc_116;
    klass.hInstance = instance;
    klass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    klass.lpszClassName = L"MCPE116Win32";
    if (!RegisterClassExW(&klass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 1;
    game_window = CreateWindowExW(0, klass.lpszClassName,
        L"Win32Craft 1.16.20.03", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, instance, NULL);
    if (!game_window) return 2;
    log_line("1.16 HWND ready; entering original application startup");
    game115_main_thread_id = get_current_thread_id_dynamic();
    InterlockedExchange(&game115_appmain_factory_active, 1);
    /* Enter through the executable's real _main.  It obtains the command
     * line and constructs the Platform::Array<String^> consumed by
     * FUN_01AE2EC0; calling that inner routine with NULL corrupts the first
     * PPL task constructed during startup. */
    result = ((OriginalMainFn)(image + 0x01c48deb))();
    log_pointer("1.16 original startup returned", (void *)(INT_PTR)result);
    return result;
}
