/* win32_vast.c - v0.1 - public domain data structures - nickscha 2026

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "vast_types.h"
#include "vast.h"
#include "vast_string_builder.h"
#include "vast_profiler.h"
#include "vast_opengl.h"
#include "vast_camera.h"
#include "vast_ui.h"
#include "vast_screen_recording.h"
#include "vast_sky_renderer.h"
#include "vast_tile_renderer.h"
#include "win32_vast_opengl.h"
#include "win32_vast_api.h"
#include "win32_vast_xinput.h"

/* Unfortunaly "modern" compilers sometimes inject memset intrinsics in the generated code
 * even if the application does not call memset and even with -fno-builtin, ... set.
 * Therefore we have to provide our own memset function.
 */
#ifdef _MSC_VER
#pragma function(memset)
#endif
void *memset(void *dest, i32 c, u32 count)
{
  s8 *bytes = (s8 *)dest;
  while (count--)
  {
    *bytes++ = (s8)c;
  }
  return dest;
}

VAST_API f64 vast_profiler_time_ms(void)
{
  static i64 freq;

  i64 time;

  if (!freq)
  {
    QueryPerformanceFrequency(&freq);
  }

  QueryPerformanceCounter(&time);

  return ((f64)time * 1000.0) / (f64)freq;
}

/* #############################################################################
 * # [SECTION] Force Discrete GPU
 * #############################################################################
 */
__declspec(dllexport) u32 NvOptimusEnablement = 0x00000001;         /* NVIDIA Force discrete GPU */
__declspec(dllexport) i32 AmdPowerXpressRequestHighPerformance = 1; /* AMD Force discrete GPU    */

/* #############################################################################
 * # [SECTION] WIN32 specifiy functions
 * #############################################################################
 */
VAST_API u8 win32_print(s8 *str)
{
  static u32 written;
  static void *log_file;

  if (!log_file)
  {
    log_file = CreateFileA("vast.log", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  }

  {
    s8 *p = str;
    u32 len = 0;

    while (*p++)
    {
      len++;
    }

    WriteFile(log_file, str, len, &written, 0);
  }

  return 1;
}

VAST_API u8 win32_file_size(s8 *filename, u32 *file_size)
{
  WIN32_FILE_ATTRIBUTE_DATA fad;

  if (!GetFileAttributesExA(filename, 0, &fad))
  {
    return 0;
  }

  /* Only support up to 4GB files */
  if (fad.nFileSizeHigh != 0)
  {
    return 0;
  }

  *file_size = fad.nFileSizeLow;

  return 1;
}

VAST_API u8 win32_file_mod_signature(s8 *filename, u32 *file_mod_signature)
{
  WIN32_FILE_ATTRIBUTE_DATA fad;
  ULARGE_INTEGER uli;

  if (!GetFileAttributesExA(filename, 0, &fad))
  {
    return 0;
  }

  uli.u.LowPart = fad.ftLastWriteTime.dwLowDateTime;
  uli.u.HighPart = fad.ftLastWriteTime.dwHighDateTime;

  /* fold quad part (64 bit) to 32 bit */
  *file_mod_signature = (u32)(uli.QuadPart ^ (uli.QuadPart >> 32));

  return 1;
}

VAST_API u8 win32_file_read(s8 *filename, u8 *buffer, u32 buffer_size)
{
  void *hFile = INVALID_HANDLE;
  u32 bytesRead = 0;
  i32 attempt;

  /* Retry loop for hot-reload: file might be locked or partially written */
  for (attempt = 0; attempt < 4; ++attempt)
  {
    hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (hFile != INVALID_HANDLE)
    {
      break;
    }

    Sleep(5); /* Small delay */
  }

  if (hFile == INVALID_HANDLE)
  {
    return 0;
  }

  if (!ReadFile(hFile, buffer, buffer_size, &bytesRead, 0) || bytesRead != buffer_size)
  {
    CloseHandle(hFile);
    return 0;
  }

  buffer[buffer_size] = '\0';

  CloseHandle(hFile);
  return 1;
}

VAST_API VAST_INLINE FILETIME win32_file_mod_time(s8 *file)
{
  static FILETIME empty = {0, 0};
  WIN32_FILE_ATTRIBUTE_DATA fad;
  return GetFileAttributesExA(file, 0, &fad) ? fad.ftLastWriteTime : empty;
}

VAST_API VAST_INLINE u8 win32_enable_high_priority(void)
{
  /* TODO(nickscha): Check integrated plus discrete GPU
   *
   * It was noticed that on a device with a integrated GPU (intel)
   * and a discrete GPU (NVIDIA) the fans were spinning high even
   * though there was only 1.5% total CPU and 2.4% GPU usage.
   * Check carefully when these priority functions can be set safely.
   *
  if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
  {
    return 1;
  }

  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
  {
    return 1;
  }

  if (!SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED))
  {
    return 1;
  }
  */
  (void)HIGH_PRIORITY_CLASS;
  (void)THREAD_PRIORITY_HIGHEST;
  (void)ES_CONTINUOUS;
  (void)ES_DISPLAY_REQUIRED;
  (void)ES_SYSTEM_REQUIRED;

  return 1;
}

VAST_API VAST_INLINE u8 win32_enable_dpi_awareness(void)
{
  /* Try Windows 10 / 11 (Per-Monitor V2) */
  void *user32 = GetModuleHandleA("user32.dll");

  void *shcore;

  if (user32)
  {
    typedef i32(__stdcall * SetProcessDpiAwarenessContextProc)(void *);
    SetProcessDpiAwarenessContextProc setProcessDpiAwarenessContext;

    *(void **)(&setProcessDpiAwarenessContext) = GetProcAddress(user32, "SetProcessDpiAwarenessContext");

    if (setProcessDpiAwarenessContext && setProcessDpiAwarenessContext((void *)-4)) /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  */
    {
      return 1;
    }
  }

  shcore = LoadLibraryA("Shcore.dll");

  if (shcore)
  {
    typedef i32(__stdcall * SetProcessDpiAwarenessProc)(i32);
    SetProcessDpiAwarenessProc setDpiAwareness;

    *(void **)(&setDpiAwareness) = GetProcAddress(shcore, "SetProcessDpiAwareness");

    if (setDpiAwareness)
    {
      setDpiAwareness(2); /* PROCESS_PER_MONITOR_DPI_AWARE */
    }

    FreeLibrary(shcore);
  }
  else
  {
    SetProcessDPIAware();
  }

  return 1;
}

VAST_API VAST_INLINE u8 win32_enable_high_resolution_timer(void)
{
  void *winmm = LoadLibraryA("Winmm.dll");

  if (winmm)
  {
    typedef u32(__stdcall * timeBeginPeriodProc)(u32);
    timeBeginPeriodProc timeBeginPeriod;

    u32 res = 0;

    *(void **)(&timeBeginPeriod) = GetProcAddress(winmm, "timeBeginPeriod");

    if (!timeBeginPeriod)
    {
      return 0;
    }

    res = timeBeginPeriod(1);

    FreeLibrary(winmm);

    /* TIMERR_NOCANDO */
    if (res == 97)
    {
      return 0;
    }
  }

  return 1;
}

VAST_API i32 win32_process_thread_count(void)
{
  u32 pid = GetCurrentProcessId();
  void *snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  THREADENTRY32 te;
  i32 count = 0;

  if (snapshot == INVALID_HANDLE)
  {
    return 0;
  }

  te.dwSize = sizeof(te);

  if (Thread32First(snapshot, &te))
  {
    do
    {
      if (te.th32OwnerProcessID == pid)
      {
        count++;
      }
    } while (Thread32Next(snapshot, &te));
  }

  CloseHandle(snapshot);

  return count;
}

typedef struct process_memory_info
{
  u64 private_bytes; /* Commit charge (what you asked for) */
  u64 working_set;   /* RAM currently used */
  u64 peak_working_set;
} process_memory_info;

u8 win32_process_memory(process_memory_info *out)
{
  typedef i32(__stdcall * GetProcessMemoryInfo_Fn)(void *, PROCESS_MEMORY_COUNTERS_EX *, u32);
  static GetProcessMemoryInfo_Fn pGetMemInfo = 0;
  static i32 initialized = 0;

  PROCESS_MEMORY_COUNTERS_EX pmc;

  if (!initialized)
  {
    /* Win7+ */
    void *kernel32 = LoadLibraryA("kernel32.dll");

    if (kernel32)
    {
      *(void **)(&pGetMemInfo) = GetProcAddress(kernel32, "K32GetProcessMemoryInfo");
    }

    /* Vista and older */
    if (!pGetMemInfo)
    {
      void *psapi = LoadLibraryA("psapi.dll");

      if (psapi)
      {
        *(void **)(&pGetMemInfo) = GetProcAddress(psapi, "GetProcessMemoryInfo");
      }
    }

    initialized = 1;
  }

  if (!pGetMemInfo)
  {
    return 0;
  }

  pmc.cb = sizeof(pmc);

  if (!pGetMemInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
  {
    return 0;
  }

  out->private_bytes = (u64)pmc.PrivateUsage;
  out->working_set = (u64)pmc.WorkingSetSize;
  out->peak_working_set = (u64)pmc.PeakWorkingSetSize;

  return 1;
}

typedef struct win32_vast_application
{
  void *hDLL;
  FILETIME lastWriteTime;
  char *dllName;
} win32_vast_application;

typedef struct win32_vast_state
{
  win32_vast_application application;

  vast_platform_state platform_state;

  u32 window_width_pending;
  u32 window_height_pending;

  u8 controller_id;           /* The XInput id associated with this controller */
  u8 controller_check_needed; /* If a device is plugged in or disconnected we should check XInput controller state again */

  u8 shader_paused;
  u8 ui_enabled;
  u8 fullscreen_enabled;
  u8 borderless_enabled;
  u8 screen_recording_enabled;
  u8 screen_recording_initialized;

  void *window_handle;
  void *device_context;
  WINDOWPLACEMENT g_wpPrev;

  s8 *gl_version;
  s8 *gl_renderer;
  s8 *gl_vendor;
  i32 gl_max_3d_texture_size;

} win32_vast_state;

VAST_API VAST_INLINE i32 win32_vk_to_vast_key(i32 vk)
{
  if (vk >= '0' && vk <= '9')
  {
    return VAST_KEYBOARD_KEY_0 + (vk - '0');
  }

  if (vk >= 'A' && vk <= 'Z')
  {
    return VAST_KEYBOARD_KEY_A + (vk - 'A');
  }

  /* function keys (f1 - f12) */
  if (vk >= 0x70 && vk <= 0x7B)
  {
    return VAST_KEYBOARD_KEY_F1 + (vk - 0x70);
  }

  switch (vk)
  {
  case 0x11:
    return VAST_KEYBOARD_KEY_CONTROL;

  case 0x0D:
    return VAST_KEYBOARD_KEY_RETURN;

  case 0x20:
    return VAST_KEYBOARD_KEY_SPACE;

  case 0x10:
    return VAST_KEYBOARD_KEY_SHIFT;

  case 0x09:
    return VAST_KEYBOARD_KEY_TAB;

  case 0x12:
    return VAST_KEYBOARD_KEY_ALT;

  case 0x1B:
    return VAST_KEYBOARD_KEY_ESCAPE;

  case 0x25:
    return VAST_KEYBOARD_KEY_LEFT;

  case 0x26:
    return VAST_KEYBOARD_KEY_UP;

  case 0x27:
    return VAST_KEYBOARD_KEY_RIGHT;

  case 0x28:
    return VAST_KEYBOARD_KEY_DOWN;

  case 0xBB:
    return VAST_KEYBOARD_KEY_PLUS;

  case 0xBD:
    return VAST_KEYBOARD_KEY_MINUS;

  case 0xBC:
    return VAST_KEYBOARD_KEY_COMMA;

  case 0xBE:
    return VAST_KEYBOARD_KEY_POINT;
  }

  return -1;
}

VAST_API VAST_INLINE i64 win32_window_callback(void *window, u32 message, u64 wParam, i64 lParam)
{
  win32_vast_state *state = (win32_vast_state *)GetWindowLongPtrA(window, GWLP_USERDATA);

  i64 result = 0;

  switch (message)
  {
  case WM_ERASEBKGND:
    return 1;
  case WM_CREATE:
  {
    CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
    state = (win32_vast_state *)cs->lpCreateParams;
    SetWindowLongPtrA(window, GWLP_USERDATA, (i64)state);

    /* Setup raw input for mouse and keyboard */
    {
      RAWINPUTDEVICE rid[2] = {0};

      (void)RIDEV_INPUTSINK; /* Receive input even when not focused */

      rid[0].usUsagePage = 0x01;
      rid[0].usUsage = 0x06; /* Keyboard */
      rid[0].dwFlags = 0;    /* Receive input only when focused */
      rid[0].hwndTarget = window;

      rid[1].usUsagePage = 0x01;
      rid[1].usUsage = 0x02; /* Mouse */
      rid[1].dwFlags = 0;    /* Receive input only when focused */
      rid[1].hwndTarget = window;

      if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0])))
      {
        win32_print("[win32] Failed to register RAWINPUT device\n");
      }
    }
  }
  break;
  case WM_CLOSE:
  case WM_QUIT:
  {
    if (!state)
    {
      break;
    }

    state->platform_state.running = 0;
  }
  break;
  case WM_SIZE:
  {
    if (!state)
    {
      break;
    }

    if (wParam == SIZE_MINIMIZED)
    {
      state->platform_state.window.minimized = 1;
    }
    else
    {
      state->platform_state.window.minimized = 0;
      state->platform_state.window.size_changed = 1;
      state->window_width_pending = (u16)(((u64)(lParam)) & 0xffff);          /* Low Word  */
      state->window_height_pending = (u16)((((u64)(lParam)) >> 16) & 0xffff); /* High Word */
    }
  }
  break;
  case WM_INPUT:
  {
    static u8 rawBuffer[128];
    RAWINPUT *raw = (RAWINPUT *)rawBuffer;

    u32 dwSize = 0;

    if (!state)
    {
      break;
    }

    GetRawInputData((RAWINPUT *)lParam, RID_INPUT, VAST_NULL, &dwSize, sizeof(RAWINPUTHEADER));

    if (dwSize > sizeof(rawBuffer) ||
        GetRawInputData((RAWINPUT *)lParam, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
    {
      return result;
    }

    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
      RAWKEYBOARD *keyboard = &raw->data.keyboard;

      u16 vKey = keyboard->VKey;

      if (vKey < 256)
      {
        /*key->was_down = key->is_down;*/
        i32 vast_key = win32_vk_to_vast_key(vKey);

        state->platform_state.input.keyboard.keys_is_down[vast_key] = !(keyboard->Flags & RI_KEY_BREAK); /* 1 if pressed, 0 if released */
      }
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE)
    {
      RAWMOUSE *mouse = &raw->data.mouse;

      i32 dx = mouse->lLastX;
      i32 dy = mouse->lLastY;

      state->platform_state.input.mouse.dx += dx;
      state->platform_state.input.mouse.dy -= dy;

      /* Scroll wheel */
      if (mouse->usButtonFlags & RI_MOUSE_WHEEL)
      {
        i16 wheelDelta = (i16)mouse->usButtonData;
        state->platform_state.input.mouse.scroll += (f32)wheelDelta / (f32)WHEEL_DELTA;
      }
    }
  }
  break;
  case WM_DEVICECHANGE:
  {
    /* Check if a device notification arrived in order to find out when to requery XInput controller states */
    /* DBT_DEVNODES_CHANGED is the most reliable for USB plugging/unplugging */
    if (wParam == DBT_DEVNODES_CHANGED || wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
    {
      state->controller_check_needed = 1;
    }
  }
  break;
  case WM_LBUTTONDOWN:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_LEFT] = 1;
    break;
  case WM_LBUTTONUP:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_LEFT] = 0;
    break;
  case WM_MBUTTONDOWN:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_MIDDLE] = 1;
    break;
  case WM_MBUTTONUP:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_MIDDLE] = 0;
    break;
  case WM_RBUTTONDOWN:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_RIGHT] = 1;
    break;
  case WM_RBUTTONUP:
    state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_RIGHT] = 0;
    break;
  default:
  {
    result = DefWindowProcA(window, message, wParam, lParam);
  }
  break;
  }

  return (result);
}

VAST_API void win32_window_enter_fullscreen(win32_vast_state *state)
{
  i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

  if (dwStyle & WS_OVERLAPPEDWINDOW)
  {
    if (GetWindowPlacement(state->window_handle, &state->g_wpPrev))
    {
      MONITORINFO mi;
      mi.cbSize = sizeof(mi);

      GetMonitorInfoA(MonitorFromWindow(state->window_handle, MONITOR_DEFAULTTONEAREST), &mi);
      SetWindowLongA(state->window_handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

      SetWindowPos(
          state->window_handle, HWND_TOP,
          mi.rcMonitor.left, mi.rcMonitor.top,
          mi.rcMonitor.right - mi.rcMonitor.left,
          mi.rcMonitor.bottom - mi.rcMonitor.top,
          SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

      state->window_width_pending = (u32)(mi.rcMonitor.right - mi.rcMonitor.left);
      state->window_height_pending = (u32)(mi.rcMonitor.bottom - mi.rcMonitor.top);
      state->platform_state.window.size_changed = 1;
    }
  }
}

VAST_API void win32_window_enter_borderless(win32_vast_state *state)
{
  if (GetWindowPlacement(state->window_handle, &state->g_wpPrev))
  {
    i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    GetMonitorInfoA(MonitorFromWindow(state->window_handle, MONITOR_DEFAULTTONEAREST), &mi);
    SetWindowLongA(state->window_handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

    SetWindowPos(
        state->window_handle, HWND_TOP,
        mi.rcMonitor.left, mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    state->window_width_pending = (u32)(mi.rcMonitor.right - mi.rcMonitor.left);
    state->window_height_pending = (u32)(mi.rcMonitor.bottom - mi.rcMonitor.top);
    state->platform_state.window.size_changed = 1;
  }
}

VAST_API void win32_window_enter_windowed(win32_vast_state *state)
{
  i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

  if (!(dwStyle & WS_OVERLAPPEDWINDOW))
  {
    RECT rect;

    SetWindowLongA(state->window_handle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(state->window_handle, &state->g_wpPrev);
    SetWindowPos(state->window_handle, VAST_NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    GetClientRect(state->window_handle, &rect);

    state->window_width_pending = (u32)(rect.right - rect.left);
    state->window_height_pending = (u32)(rect.bottom - rect.top);
    state->platform_state.window.size_changed = 1;
  }
  else
  {
    /* resize windowed mode */
    RECT rect = {0};
    rect.right = (i32)state->platform_state.window.width;
    rect.bottom = (i32)state->platform_state.window.height;

    AdjustWindowRect(&rect, (u32)GetWindowLongA(state->window_handle, GWL_STYLE), 0);
    SetWindowPos(state->window_handle, VAST_NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

VAST_API u8 win32_load_application(win32_vast_state *state)
{
  s8 *dllName = "vast.dll";
  s8 *dllTempName = "vast_temp.dll";

  win32_print("[win32] load application\n");

  if (state->application.hDLL != VAST_NULL)
  {
    if (!FreeLibrary(state->application.hDLL))
    {
      win32_print("[win32] cannot free library: ");
      win32_print(dllTempName);
      win32_print("\n");
      return 0;
    }
    state->application.hDLL = VAST_NULL;
  }

  if (!CopyFileA(dllName, dllTempName, 0))
  {
    win32_print("[win32] cannot copy file: ");
    win32_print(dllName);
    win32_print("\n");
    return 0;
  }

  state->application.hDLL = LoadLibraryA(dllTempName);
  state->application.dllName = dllName;
  state->application.lastWriteTime = win32_file_mod_time(dllName);

  if (!state->application.hDLL)
  {
    return 0;
  }

  /* FIX for ERROR: ISO C forbids conversion of object pointer to function pointer type*/
  /* https://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html */
  *(void **)(&vast_update) = GetProcAddress(state->application.hDLL, "vast_update");

  return 1;
}

/* #############################################################################
 * # [SECTION] OpenGL Context Creation and Shader Management
 * #############################################################################
 */
static u32 opengl_failed_function_load_count = 0;

VAST_API PROC win32_opengl_load_function(s8 *name)
{
  static void *win32_opengl_lib = 0;

  PROC gl_function;

  if (!win32_opengl_lib)
  {
    win32_opengl_lib = LoadLibraryA("opengl32.dll");

    if (!win32_opengl_lib)
    {
      return VAST_NULL;
    }
  }

  if (!wglGetProcAddress)
  {
    *(void **)(&wglGetProcAddress) = GetProcAddress(win32_opengl_lib, "wglGetProcAddress");

    if (!wglGetProcAddress)
    {
      win32_print("[opengl] FATAL: cannot load wglGetProcAddress!\n");
      FreeLibrary(win32_opengl_lib);
      win32_opengl_lib = VAST_NULL;
      return VAST_NULL;
    }
  }

  gl_function = wglGetProcAddress(name);

  /* Some GPU drivers do not return a valid null pointer if requested OpenGL function is not available */
  if (gl_function == (PROC)0 || gl_function == (PROC)0x1 || gl_function == (PROC)0x2 || gl_function == (PROC)0x3 || gl_function == (PROC)-1)
  {
    void *object_ptr = (void *)GetProcAddress(win32_opengl_lib, name);
    gl_function = *(PROC *)&object_ptr;
  }

  if (gl_function == (PROC)0)
  {
    win32_print("[opengl] \"");
    win32_print(name);
    win32_print("\" not found!\n");

    opengl_failed_function_load_count++;
  }

  return gl_function;
}

VAST_API VAST_INLINE i32 opengl_create_context(win32_vast_state *state)
{
  void *window_instance = GetModuleHandleA(0);
  WNDCLASSA window_class = {0};
  u32 window_style = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
  RECT rect = {0};

  void *fake_window;
  void *fake_device_context;
  void *fake_rc;
  i32 fake_pixel_format;
  PIXELFORMATDESCRIPTOR fake_pfd = {0};

  VAST_PROFILER_BEGIN(win32_opengl_init);

  window_class.style = CS_OWNDC;
  window_class.lpfnWndProc = win32_window_callback;
  window_class.hInstance = window_instance;
  window_class.hCursor = LoadCursorA(0, IDC_ARROW);
  window_class.hIcon = LoadIconA(window_instance, MAKEINTRESOURCEA(1));
  window_class.hbrBackground = 0;
  window_class.lpszClassName = state->platform_state.window.title;

  if (!RegisterClassA(&window_class))
  {
    return 0;
  }

  fake_window = CreateWindowExA(
      0,
      window_class.lpszClassName,
      window_class.lpszClassName,
      WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      0, 0,
      1, 1,
      0, 0,
      window_instance, 0);

  if (!fake_window)
  {
    return 0;
  }

  fake_device_context = GetDC(fake_window);

  fake_pfd.nSize = sizeof(fake_pfd);
  fake_pfd.nVersion = 1;
  fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  fake_pfd.iPixelType = PFD_TYPE_RGBA;
  fake_pfd.cColorBits = 32;
  fake_pfd.cAlphaBits = 8;
  fake_pfd.cDepthBits = 24;

  fake_pixel_format = ChoosePixelFormat(fake_device_context, &fake_pfd);

  if (!fake_pixel_format || !SetPixelFormat(fake_device_context, fake_pixel_format, &fake_pfd))
  {
    return 0;
  }

  win32_vast_opengl_load_functions(win32_opengl_load_function, 1);

  fake_rc = wglCreateContext(fake_device_context);

  if (!fake_rc || !wglMakeCurrent(fake_device_context, fake_rc))
  {
    return 0;
  }

  rect.right = (i32)state->platform_state.window.width;
  rect.bottom = (i32)state->platform_state.window.height;
  AdjustWindowRect(&rect, window_style, 0);

  state->window_handle = CreateWindowExA(
      0,
      window_class.lpszClassName,
      window_class.lpszClassName,
      window_style,
      0, 0,
      rect.right - rect.left,
      rect.bottom - rect.top,
      0, 0,
      window_instance,
      state /* Pass pointer to user data to the window callback */
  );

  /* Modal window */
  SetWindowPos(state->window_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  state->device_context = GetDC(state->window_handle);

  /* Load wgl function */
  win32_vast_opengl_load_functions(win32_opengl_load_function, 0);

  /* Load common opengl functions (platform independant) */
  vast_opengl_load_functions(win32_opengl_load_function);

  if (opengl_failed_function_load_count > 0)
  {
    win32_print("[opengl] Some of the required OpenGL functions are not available on your machine (see log file above)!\n");
    return 0;
  }

  /* Set Pixel Format */
  {

    i32 pixel_attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0};

    i32 pixel_format_id;
    u32 num_formats;

    if (!wglChoosePixelFormatARB(state->device_context, pixel_attributes, 0, 1, &pixel_format_id, &num_formats) || !num_formats)
    {
      return 0;
    }

    SetPixelFormat(state->device_context, pixel_format_id, 0);
  }

  /* Create the OpenGL context */
  {
    i32 context_attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0};

    void *rc = wglCreateContextAttribsARB(state->device_context, 0, context_attributes);

    wglMakeCurrent(0, 0);
    wglDeleteContext(fake_rc);
    ReleaseDC(fake_window, fake_device_context);
    DestroyWindow(fake_window);

    if (!rc || !wglMakeCurrent(state->device_context, rc))
    {
      return 0;
    }
  }

  /* Disable VSync */
  if (wglSwapIntervalEXT)
  {
    wglSwapIntervalEXT(0);
  }

  /* Print opengl information */
  state->gl_vendor = (s8 *)glGetString(GL_VENDOR);
  state->gl_renderer = (s8 *)glGetString(GL_RENDERER);
  state->gl_version = (s8 *)glGetString(GL_VERSION);

  win32_print("[opengl] vendor  : ");
  win32_print(state->gl_vendor);
  win32_print("\n");

  win32_print("[opengl] renderer: ");
  win32_print(state->gl_renderer);
  win32_print("\n");

  win32_print("[opengl] version : ");
  win32_print(state->gl_version);
  win32_print("\n");

  VAST_PROFILER_END(win32_opengl_init);

  return 1;
}

/* #############################################################################
 * # [SECTION] simple Camera Movement
 * #############################################################################
 */
VAST_API VAST_INLINE void vast_update_camera_movement(win32_vast_state *state, vast_camera *camera, f32 camera_speed)
{
  static u8 mouse_attached = 0;

  f32 cam_speed = camera_speed * (f32)state->platform_state.timing.time_delta;

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_W])
  {
    camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->forward, cam_speed));
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_S])
  {
    camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->forward, cam_speed));
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_A])
  {
    camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->right, cam_speed));
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_D])
  {
    camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->right, cam_speed));
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_SPACE])
  {
    camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->worldUp, cam_speed));
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_CONTROL])
  {
    camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->worldUp, cam_speed));
  }

  /* Roll handling */
  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_Q])
  {
    camera->roll -= 1.0f;
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_E])
  {
    camera->roll += 1.0f;
  }

  /* FOV handling */
  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_F] && !state->platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_F])
  {
    camera->fov = 90.0f;
    camera->roll = 0.0f;
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_PLUS])
  {
    camera->fov -= 1.0f;

    if (camera->fov <= 0.0f)
    {
      camera->fov = 1.0f;
    }
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_MINUS])
  {
    camera->fov += 1.0f;

    if (camera->fov >= 180.0f)
    {
      camera->fov = 179.0f;
    }
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_LEFT])
  {
    camera->yaw -= vast_minf(cam_speed * 0.1f, 89.0f);
  }

  if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_RIGHT])
  {
    camera->yaw += vast_minf(cam_speed * 0.1f, 89.0f);
  }

  /* Mouse handling */
  if (state->platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_RIGHT] && !state->platform_state.input.mouse.keys_was_down[VAST_MOUSE_KEY_RIGHT])
  {
    mouse_attached = !mouse_attached;
  }

  if (mouse_attached)
  {
    f32 mouseSensitivity = 0.1f;
    camera->yaw += vast_minf((f32)state->platform_state.input.mouse.dx * mouseSensitivity, 89.0f);
    camera->pitch += vast_maxf((f32)state->platform_state.input.mouse.dy * mouseSensitivity, -89.0f);
    camera->pitch = vast_clampf(camera->pitch, -89.0f, 89.0f);

    if (state->platform_state.input.mouse.scroll != 0.0f)
    {
      camera->fov = vast_clampf(camera->fov - (state->platform_state.input.mouse.scroll * 2), 1.0f, 179.0f);
    }
  }

  vast_camera_update(camera);
}

VAST_API void vast_render_scene(win32_vast_state *state)
{
  static u8 scene_initialized = 0;

  static vast_camera camera = {0};
  static f32 camera_speed = 1000.0f;

  vast_vec3 sun_dir;

  if (!scene_initialized)
  {
    /* Tiles */
    camera = vast_camera_init();
    camera.position.y = 10.0f;
    camera_speed = 50.0f;

    scene_initialized = 1;
  }

  /* Update camera */
  vast_update_camera_movement(state, &camera, camera_speed);

  /* Sun/Moon movement logic */
  {

    static u8 move_sun = 0;
    static u8 move_sun_loop = 0;
    static f32 move_val = 0.0f;

    if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_M] && !state->platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_M])
    {
      move_sun = !move_sun;

      if (move_sun)
      {
        move_val = 0.0f;
        move_sun_loop = 0;
      }
    }

    sun_dir = vast_vec3_init(0.0f, 0.0f, -1.0f);

    if (move_sun)
    {

      f32 t = (f32)state->platform_state.timing.time_elapsed * 0.5f;

      if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_L] && !state->platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_L])
      {
        move_sun_loop = !move_sun_loop;
      }

      if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_UP])
      {
        move_val += (f32)state->platform_state.timing.time_delta;
      }

      if (state->platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_DOWN])
      {
        move_val -= (f32)state->platform_state.timing.time_delta;

        if (move_val < 0.0f)
        {
          move_val = 0.0f;
        }
      }

      sun_dir = vast_vec3_init(0.0f, vast_sinf(move_val), -vast_cosf(move_val));

      if (move_sun_loop)
      {
        sun_dir = vast_vec3_init(0.0f, vast_sinf(t * 0.025f), -vast_cosf(t * 0.025f));
      }

      sun_dir = vast_vec3_normalize(sun_dir);
    }
  }

  /* Render */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  vast_sky_render(&state->platform_state, &camera, sun_dir);
  vast_tile_render(&state->platform_state, &camera, sun_dir);
}

/* #############################################################################
 * # [SECTION] Main Entry Point
 * #############################################################################
 */
VAST_API i32 start(i32 argc, u8 **argv)
{
  win32_vast_state state = {0};

  state.platform_state.running = 1;
  state.platform_state.window.title = "vast v0.1 (F1=Debug UI, F2=Screen Recording, R=Reset, P=Pause, F9=Borderless, F11=Fullscreen)";
  state.platform_state.timing.frame_rate_target = 30; /* 30 FPS, 0 = unlimited */
  state.controller_check_needed = 1;                  /* By default we have to query first XInput state */

  /* Platform API setup */
  state.platform_state.api.io_print = win32_print;
  state.platform_state.api.io_file_size = win32_file_size;
  state.platform_state.api.io_file_mod_signature = win32_file_mod_signature;
  state.platform_state.api.io_file_read = win32_file_read;

  /* Platform Window setup */
  state.platform_state.window.width = 800;
  state.platform_state.window.height = 600;
  state.platform_state.window.clear_color_r = 0.1f;
  state.platform_state.window.clear_color_g = 0.1f;
  state.platform_state.window.clear_color_b = 0.1f;

  /* Platform Memory Setup */
  state.platform_state.memory.size = 1024 * 1024 * 8; /* 8 MB */
  state.platform_state.memory.data = VirtualAlloc(0, state.platform_state.memory.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  (void)argc;
  (void)argv;

  /******************************/
  /* Load application once      */
  /******************************/
  if (!win32_load_application(&state))
  {
    win32_print("[ERROR] Could not load application!\n");
    return 1;
  }

  /******************************/
  /* Set Process Priorities     */
  /******************************/
  if (!win32_enable_high_priority())
  {
    win32_print("[WARNING] Failed to set high priority process\n");
  }

  /******************************/
  /* Set DPI aware mode         */
  /******************************/
  if (!win32_enable_dpi_awareness())
  {
    win32_print("[WARNING] Cannot set DPI awareness\n");
  }

  /******************************/
  /* HighRes timer for Sleep(1) */
  /******************************/
  if (!win32_enable_high_resolution_timer())
  {
    win32_print("[WARNING] Cannot set win32 high resolution timer using Winmm.dll (timeBeginPeriod)\n");
  }

  /******************************/
  /* Load XInput Controller     */
  /******************************/
  if (!xinput_load())
  {
    win32_print("[WARNING] Could not load XInput (xinput1_4.dll, xinput1_3.dll or xinput9_1_0.dll)! XInput gamepads/controllers (e.g. XBOX) are not functional.\n");
  }

  /******************************/
  /* Window and OpenGL context  */
  /******************************/
  if (!opengl_create_context(&state))
  {
    win32_print("[ERROR] Could not create opengl context!\n");
    return 1;
  }

  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &state.gl_max_3d_texture_size);

  /* Avoid clear color flickering */
  glViewport(0, 0, (i32)state.platform_state.window.width, (i32)state.platform_state.window.height);
  glClearColor(state.platform_state.window.clear_color_r, state.platform_state.window.clear_color_g, state.platform_state.window.clear_color_b, state.platform_state.window.clear_color_a);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_MULTISAMPLE);
  glClear(GL_COLOR_BUFFER_BIT);

  SwapBuffers(state.device_context);

  /* Make the window visible */
  ShowWindow(state.window_handle, SW_SHOW);

  {
    i64 perf_freq;
    i64 time_start;
    i64 time_start_fps_cap;
    i64 time_last;

    i32 thread_count = win32_process_thread_count();

    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&time_start);
    QueryPerformanceCounter(&time_start_fps_cap);

    time_last = time_start;

    while (state.platform_state.running)
    {
      i64 time_now;

      /******************************/
      /* Timing                     */
      /******************************/
      {
        QueryPerformanceCounter(&time_now);

        state.platform_state.timing.time_delta = (f64)(time_now - time_last) / (f64)perf_freq;
        state.platform_state.timing.time_elapsed = (f64)(time_now - time_start) / (f64)perf_freq;

        time_last = time_now;

        if (state.platform_state.timing.time_delta > 0.0)
        {
          state.platform_state.timing.frame_rate = 1.0 / state.platform_state.timing.time_delta;
        }
      }

      /******************************/
      /* Idle when window minimized */
      /******************************/
      if (state.platform_state.window.minimized)
      {
        MSG msg;
        GetMessageA(&msg, 0, 0, 0);
        DispatchMessageA(&msg);
        continue;
      }

      /******************************/
      /* Hot Reload Application     */
      /******************************/
      if (state.platform_state.timing.frame_count % 30 == 0)
      {
        FILETIME ddlFtCurrent = win32_file_mod_time(state.application.dllName);

        if (CompareFileTime(&ddlFtCurrent, &state.application.lastWriteTime) != 0)
        {
          win32_load_application(&state);
          state.platform_state.application_initialized = 0;
        }
      }

      /******************************/
      /* Input Processing           */
      /******************************/
      {
        MSG message = {0};

        u64 *src = (u64 *)state.platform_state.input.keyboard.keys_is_down;
        u64 *dst = (u64 *)state.platform_state.input.keyboard.keys_was_down;

        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;

        state.platform_state.input.mouse.keys_was_down[VAST_MOUSE_KEY_LEFT] = state.platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_LEFT];
        state.platform_state.input.mouse.keys_was_down[VAST_MOUSE_KEY_MIDDLE] = state.platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_MIDDLE];
        state.platform_state.input.mouse.keys_was_down[VAST_MOUSE_KEY_RIGHT] = state.platform_state.input.mouse.keys_is_down[VAST_MOUSE_KEY_RIGHT];
        state.platform_state.input.mouse.dx = 0;
        state.platform_state.input.mouse.dy = 0;

        while (PeekMessageA(&message, state.window_handle, 0, 0, PM_REMOVE))
        {
          DispatchMessageA(&message);
        }
      }

      /* Get current frames mouse position */
      {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(state.window_handle, &p);

        state.platform_state.input.mouse.x = p.x;
        state.platform_state.input.mouse.y = (i32)state.platform_state.window.height - 1 - p.y;
      }

      /******************************/
      /* XInput Controller          */
      /******************************/
      if (XInputGetState)
      {

        /* If we recieve a WM_DEVICECHANGE message (hardware connected or disconnected to machine)
         * we need to check again the XInput Controller state.
         */
        if (state.controller_check_needed)
        {
          u8 i = 0;

          XINPUT_STATE xinput_state = {0};

          state.platform_state.input.controller.connected = 0;

          for (i = 0; i < XINPUT_USER_MAX_COUNT; ++i)
          {
            u32 result = XInputGetState(i, &xinput_state);

            if (result == 0)
            {
              state.controller_id = i;
              state.platform_state.input.controller.connected = 1;
              break;
            }
          }

          state.controller_check_needed = 0;
        }

        if (state.platform_state.input.controller.connected)
        {
          XINPUT_STATE xinput_state = {0};
          u32 result = XInputGetState(state.controller_id, &xinput_state);

          if (result == 0)
          {
            XINPUT_GAMEPAD *gp = &xinput_state.Gamepad;
            state.platform_state.input.controller.button_a = (gp->wButtons & XINPUT_GAMEPAD_A) ? 1 : 0;
            state.platform_state.input.controller.button_b = (gp->wButtons & XINPUT_GAMEPAD_B) ? 1 : 0;
            state.platform_state.input.controller.button_x = (gp->wButtons & XINPUT_GAMEPAD_X) ? 1 : 0;
            state.platform_state.input.controller.button_y = (gp->wButtons & XINPUT_GAMEPAD_Y) ? 1 : 0;
            state.platform_state.input.controller.shoulder_left = (gp->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
            state.platform_state.input.controller.shoulder_right = (gp->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1 : 0;
            state.platform_state.input.controller.dpad_up = (gp->wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1 : 0;
            state.platform_state.input.controller.dpad_down = (gp->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0;
            state.platform_state.input.controller.dpad_left = (gp->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1 : 0;
            state.platform_state.input.controller.dpad_right = (gp->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0;
            state.platform_state.input.controller.start = (gp->wButtons & XINPUT_GAMEPAD_START) ? 1 : 0;
            state.platform_state.input.controller.back = (gp->wButtons & XINPUT_GAMEPAD_BACK) ? 1 : 0;
            state.platform_state.input.controller.stick_left = (gp->wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0;
            state.platform_state.input.controller.stick_right = (gp->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0;
            state.platform_state.input.controller.trigger_left_value = xinput_process_trigger(gp->bLeftTrigger);
            state.platform_state.input.controller.trigger_right_value = xinput_process_trigger(gp->bRightTrigger);
            state.platform_state.input.controller.trigger_left = state.platform_state.input.controller.trigger_left_value > 0.0f ? 1 : 0;
            state.platform_state.input.controller.trigger_right = state.platform_state.input.controller.trigger_right_value > 0.0f ? 1 : 0;
            state.platform_state.input.controller.stick_left_x = xinput_process_thumbstick(gp->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            state.platform_state.input.controller.stick_left_y = xinput_process_thumbstick(gp->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            state.platform_state.input.controller.stick_right_x = xinput_process_thumbstick(gp->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            state.platform_state.input.controller.stick_right_y = xinput_process_thumbstick(gp->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
          }
          else
          {
            state.controller_check_needed = 1;
            state.platform_state.input.controller.connected = 0;
          }
        }
      }

      /******************************/
      /* Full or Borderless (F9,F11)*/
      /******************************/
      if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_F9] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_F9]) /* F9 */
      {
        state.borderless_enabled = !state.borderless_enabled;

        if (state.borderless_enabled && !state.fullscreen_enabled)
        {
          win32_window_enter_borderless(&state);
        }
        else
        {
          state.fullscreen_enabled = 0;
          state.borderless_enabled = 0;
          win32_window_enter_windowed(&state);
        }
      }

      if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_F11] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_F11])
      {
        state.fullscreen_enabled = !state.fullscreen_enabled;

        if (state.fullscreen_enabled && !state.borderless_enabled)
        {
          win32_window_enter_fullscreen(&state);
        }
        else
        {
          state.fullscreen_enabled = 0;
          state.borderless_enabled = 0;
          win32_window_enter_windowed(&state);
        }
      }

      /******************************/
      /* Handle Window Size changes */
      /******************************/
      if (state.platform_state.window.size_changed && !state.screen_recording_enabled)
      {
        state.platform_state.window.width = state.window_width_pending;
        state.platform_state.window.height = state.window_height_pending;

        glViewport(0, 0, (i32)state.platform_state.window.width, (i32)state.platform_state.window.height);

        state.platform_state.window.size_changed = 0;
      }

      /******************************/
      /* Pause Shader (P)           */
      /******************************/
      if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_P] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_P])
      {
        state.shader_paused = !state.shader_paused;
      }

      /******************************/
      /* Reset Timer (R)            */
      /******************************/
      if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_R] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_R])
      {
        /* Reset iTime elapsed seconds on hot reload */
        QueryPerformanceCounter(&time_start);
        state.platform_state.timing.frame_count = 0;
      }

      /******************************/
      /* Main Application Logic     */
      /******************************/
      vast_update(&state.platform_state);
      vast_render_scene(&state);

      /* Application updated the window size */
      if (state.platform_state.window.size_changed)
      {
        state.window_width_pending = state.platform_state.window.width;
        state.window_height_pending = state.platform_state.window.height;
      }

      /* Application updated the window clear color */
      if (state.platform_state.window.clear_color_changed)
      {
        glClearColor(state.platform_state.window.clear_color_r, state.platform_state.window.clear_color_g, state.platform_state.window.clear_color_b, state.platform_state.window.clear_color_a);
        state.platform_state.window.clear_color_changed = 0;
      }

      /* Application updated the window title */
      if (state.platform_state.window.title_changed)
      {
        SetWindowTextA(state.window_handle, state.platform_state.window.title);
      }

      /******************************/
      /* UI Rendering (F1 pressed)  */
      /******************************/
      if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_F1] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_F1])
      {
        state.ui_enabled = !state.ui_enabled;
      }

      if (state.ui_enabled)
      {
        static u32 handle_count = 0;
        static process_memory_info mem = {0};

        GetProcessHandleCount(GetCurrentProcess(), &handle_count);
        win32_process_memory(&mem);

        vast_ui_render(
            &state.platform_state,
            thread_count, (i32)handle_count,
            (i32)(mem.working_set / 1024), (i32)(mem.peak_working_set / 1024), (i32)(mem.private_bytes / 1024),
            state.gl_version, state.gl_renderer, state.gl_vendor,
            state.borderless_enabled, state.fullscreen_enabled, state.shader_paused, state.screen_recording_enabled);
      }

      /******************************/
      /* Screen Recording (F2)      */
      /******************************/
      {
        static u8 *framebuffer;
        static void *video_file_handle;

        if (state.platform_state.input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_F2] && !state.platform_state.input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_F2])
        {
          state.screen_recording_enabled = !state.screen_recording_enabled;
        }

        if (state.screen_recording_enabled)
        {

          u32 written = 0;

          if (!state.screen_recording_initialized)
          {
            s8 buffer[128];
            vast_sb t = {0};

            t.size = sizeof(buffer);
            t.buffer = buffer;

            /* Create recording output file name.
             * Format:  vast_capture_<window_width>x<window_height>_<platform_state.timing.frame_rate_target>.raw
             * Example: vast_capture_800x600_60.raw
             */
            vast_sb_s8(&t, "vast_capture_");
            vast_sb_i32(&t, (i32)(state.platform_state.window.width));
            vast_sb_s8(&t, "x");
            vast_sb_i32(&t, (i32)(state.platform_state.window.height));
            vast_sb_s8(&t, "_");
            vast_sb_i32(&t, (i32)(state.platform_state.timing.frame_rate_target));
            vast_sb_s8(&t, ".raw");

            framebuffer = VirtualAlloc(0, state.platform_state.window.width * state.platform_state.window.height * 3, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            video_file_handle = CreateFileA(t.buffer, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

            state.screen_recording_initialized = 1;
          }

          /* Render Image to framebuffer */
          vast_screen_recording_render(&state.platform_state, framebuffer);

          WriteFile(video_file_handle, framebuffer, state.platform_state.window.width * state.platform_state.window.height * 3, &written, 0);
        }
        else if (state.screen_recording_initialized)
        {
          VirtualFree(framebuffer, 0, MEM_RELEASE);
          CloseHandle(video_file_handle);
          state.screen_recording_initialized = 0;
        }
      }

      SwapBuffers(state.device_context);

      /* Measure RAW FPS (rendering without cap)*/
      {
        i64 time_render_now;
        QueryPerformanceCounter(&time_render_now);
        state.platform_state.timing.frame_rate_raw = 1.0 / ((f64)(time_render_now - time_last) / (f64)perf_freq);
      }

      /******************************/
      /* Frame Rate Limiting        */
      /******************************/
      if (state.platform_state.timing.frame_rate_target > 0)
      {
        i64 time_end;

        f64 frame_time;
        f64 remaining;
        f64 target_frame_time = 1.0 / (f64)state.platform_state.timing.frame_rate_target;

        QueryPerformanceCounter(&time_end);

        frame_time = (f64)(time_end - time_start_fps_cap) / (f64)perf_freq;
        remaining = target_frame_time - frame_time;

        if (remaining > 0.0)
        {
          /* Sleep most of it (milliseconds) */
          if (remaining > 0.0005)
          {
            u32 sleep_ms = (u32)((remaining - 0.00025) * 1000.0);

            if (sleep_ms > 0)
            {
              Sleep(sleep_ms);
            }
          }

          /* Spin for the rest */
          for (;;)
          {
            QueryPerformanceCounter(&time_end);

            frame_time = (f64)(time_end - time_start_fps_cap) / (f64)perf_freq;

            if (frame_time >= target_frame_time)
            {
              break;
            }
          }
        }

        /* Start timing next frame */
        time_start_fps_cap = time_end;
      }

      state.platform_state.timing.frame_count++;
    }
  }

  return 0;
}

/* #############################################################################
 * # [SECTION] nostdlib entry point
 * #############################################################################
 */
#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
i32 WinMainCRTStartup(void)
{
  u8 *cmdline = (u8 *)GetCommandLineA();
  u8 *argv[8];
  i32 argc = 0;

  i32 return_code;

  /* Parse command line arguments into argv */
  while (*cmdline)
  {
    /* skip whitespace */
    while (*cmdline == ' ' || *cmdline == '\t')
    {
      cmdline++;
    }

    if (!*cmdline)
    {
      break;
    }

    if (argc < 9)
    {
      argv[argc++] = cmdline;
    }

    /* parse token (basic, no quote handling) */
    while (*cmdline && *cmdline != ' ' && *cmdline != '\t')
    {
      cmdline++;
    }

    if (*cmdline)
    {
      *cmdline++ = '\0';
    }
  }

  argv[argc] = (u8 *)0;

  /* Run the program and exit with return code */
  return_code = start(argc, argv);
  ExitProcess((u32)return_code);
  return return_code;
}

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2026 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/