#define TX_API __declspec(dllexport)

static int g_prepared;
static HANDLE g_master_thread;
static volatile int g_req_tty;
static volatile int g_req_done;
static char g_req_cmd[4096];
static int g_req_has_cmd;

static HANDLE g_host, g_host_req;

static void host_start(OBJECT_ATTRIBUTES *oa) {
    static wchar_t exe[400], cmd[500];
    HMODULE m = NULL;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)host_start, &m)) return;
    DWORD n = GetModuleFileNameW(m, exe, 380);
    wchar_t *sl = wcsrchr(exe, L'\\');
    if (!n || !sl) return;
    wcscpy(sl + 1, L"LuminalHost.exe");
    if (!NT_OK(NtCreateEvent(&g_host_req, EVENT_ALL_ACCESS, oa, 1, FALSE))) return;
    g->host_ui = (u32)(ULONG_PTR)g_ui_event;
    g->host_req = (u32)(ULONG_PTR)g_host_req;
    g->host_owner = GetCurrentProcessId();
    wcsncpy(g->host_root, g_root_dos, 299);
    wcsncpy(g->host_musl, g_musl_dos, 299);
    swprintf(cmd, 500, L"\"%s\" -H %x", exe, (u32)(ULONG_PTR)g_shm_sec);
    HANDLE hp = NULL;
    if (NT_OK(nt_spawn_clear(exe, cmd, NULL, &hp))) g_host = hp;
}

static int host_spawn(int tty) {
    if (!g_host) return -1;
    LARGE_INTEGER z;
    z.QuadPart = 0;
    if (NtWaitForSingleObject(g_host, FALSE, &z) == 0) {
        NtClose(g_host);
        g_host = NULL;
        return -1;
    }
    g->req_has_cmd = g_req_has_cmd;
    if (g_req_has_cmd) memcpy(g->req_cmd, g_req_cmd, sizeof(g->req_cmd));
    g->req_done = 0;
    g->req_tty = tty;
    NtSetEvent(g_host_req, NULL);
    for (int i = 0; i < 2000 && !g->req_done; i++) Sleep(5);
    int r = g->req_done;
    g->req_tty = 0;
    return r == 1 ? 0 : 1;
}

static void master_spawn(void) {
    static char *argv[4];
    int tty = g_req_tty;
    int hr = host_spawn(tty);
    if (hr >= 0) {
        if (hr) {
            Tty *t = &g->ttys[tty];
            t->sess_status = 126 << 8;
            t->sess_done = 1;
            NtSetEvent(g_ui_event, NULL);
        }
        g_req_done = 1;
        g_req_tty = 0;
        return;
    }
    argv[0] = "/bin/sh";
    if (g_req_has_cmd) {
        argv[1] = "-c";
        argv[2] = g_req_cmd;
        argv[3] = NULL;
    } else {
        argv[1] = "-l";
        argv[2] = NULL;
    }
    if (spawn_session(argv, tty)) {
        Tty *t = &g->ttys[tty];
        t->sess_status = 126 << 8;
        t->sess_done = 1;
        NtSetEvent(g_ui_event, NULL);
    }
    g_req_done = 1;
    g_req_tty = 0;
}

static DWORD WINAPI master_thread(LPVOID unused) {
    static WSADATA wd;
    int tick = 0;
    (void)unused;
    WSAStartup(MAKEWORD(2, 2), &wd);
    write_resolv_conf();
    for (;;) {
        if (g_req_tty) master_spawn();
        if (!net_pump(2)) {
            LARGE_INTEGER t;
            t.QuadPart = -200000;
            NtWaitForSingleObject(g_events[MAX_PROC - 1], FALSE, &t);
        }
        if (++tick % 5 == 0) reap_dead();
        if (tick % 400 == 0) net_refresh_ifs();
    }
}

static void fix_home_names(const wchar_t *dir, int depth) {
    wchar_t pat[700], from[700], to[700];
    WIN32_FIND_DATAW e;
    if (depth > 24 || swprintf(pat, 700, L"%s\\*", dir) < 0) return;
    HANDLE h = FindFirstFileW(pat, &e);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!wcscmp(e.cFileName, L".") || !wcscmp(e.cFileName, L"..")) continue;
        wchar_t name[MAX_PATH];
        wcscpy(name, e.cFileName);
        int changed = 0;
        for (wchar_t *c = name; *c; c++)
            if (*c >= 0xF041 && *c <= 0xF05A) {
                *c -= 0xF000;
                changed = 1;
            }
        swprintf(from, 700, L"%s\\%s", dir, e.cFileName);
        swprintf(to, 700, L"%s\\%s", dir, name);
        if (changed && !MoveFileExW(from, to, 0)) wcscpy(to, from);
        if ((e.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(e.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) fix_home_names(to, depth + 1);
    } while (FindNextFileW(h, &e));
    FindClose(h);
}

static void fix_home_names_once(void) {
    wchar_t mark[400];
    swprintf(mark, 400, L"%s\\.home-names", g_root_dos);
    if (GetFileAttributesW(mark) != INVALID_FILE_ATTRIBUTES) return;
    fix_home_names(HOME_DIR, 0);
    HANDLE h = CreateFileW(mark, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
}

TX_API int __stdcall tx_init(const wchar_t *root, const wchar_t *musl) {
    if (g_prepared) return 0;
    wcsncpy(g_root_dos, root, 299);
    wcsncpy(g_musl_dos, musl, 299);
    low_reserve();
    if (musl_prepare()) return -1;
    if (shm_init()) return -2;
    arena_init();
    if (!g_arena) return -3;
    OBJECT_ATTRIBUTES oa;
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(oa);
    oa.Attributes = OBJ_INHERIT_;
    NtCreateEvent(&g_ui_event, EVENT_ALL_ACCESS, &oa, 1, FALSE);
    RtlAddVectoredExceptionHandler(1, guest_veh);
    host_start(&oa);
    fix_home_names_once();
    g_prepared = 1;
    return 0;
}

TX_API int __stdcall tx_import(const wchar_t *tar) {
    int r = import_tar(tar);
    make_sys_dirs();
    return r;
}

TX_API void __stdcall tx_trace(int on) {
    if (!on || g_trace_h) return;
    static wchar_t tp[400];
    int n = swprintf(tp, 400, L"\\??\\%s\\trace.log", g_root_dos);
    UNICODE_STRING us;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK io;
    init_oa(&oa, &us, tp, n * 2, OBJ_INHERIT_);
    NtCreateFile(&g_trace_h, FILE_APPEND_DATA | SYNCHRONIZE, &oa, &io, NULL, 0, SHARE_ALL, FILE_OVERWRITE_IF_, FO_SYNC | FO_NONDIR, NULL, 0);
    g_trace = 1;
}

TX_API int __stdcall tx_session_open(const char *cmd, int rows, int cols) {
    if (!g_prepared) return -1;
    make_sys_dirs();
    int id;
    lock_();
    for (id = 1; id < MAX_TTY; id++)
        if (!g->ttys[id].used) break;
    if (id == MAX_TTY) {
        unlock_();
        return -2;
    }
    Tty *t = &g->ttys[id];
    memset(t, 0, sizeof(*t));
    tty_setup(t);
    t->used = 1;
    t->is_session = 1;
    t->master_refs = 1;
    t->rows = (u16)rows;
    t->cols = (u16)cols;
    unlock_();
    g_req_has_cmd = cmd != NULL;
    if (cmd) {
        strncpy(g_req_cmd, cmd, sizeof(g_req_cmd) - 1);
        g_req_cmd[sizeof(g_req_cmd) - 1] = 0;
    }
    g_req_done = 0;
    g_req_tty = id;
    if (!g_master_thread) g_master_thread = CreateThread(NULL, 1 << 20, master_thread, NULL, 0, NULL);
    NtSetEvent(g_events[MAX_PROC - 1], NULL);
    for (int i = 0; i < 2000 && !g_req_done; i++) Sleep(5);
    return g_req_done ? id : -3;
}

TX_API void __stdcall tx_wait(int timeout_ms) {
    LARGE_INTEGER t;
    if (!g_prepared) return;
    t.QuadPart = -(i64)timeout_ms * 10000;
    NtWaitForSingleObject(g_ui_event, FALSE, &t);
}

TX_API int __stdcall tx_session_read(int id, u8 *buf, int max) {
    if (!g_prepared || id < 1 || id >= MAX_TTY) return -1;
    Tty *t = &g->ttys[id];
    u32 n = tty_drain_t(t, buf, max);
    if (n) return (int)n;
    return t->sess_done ? -1 : 0;
}

TX_API void __stdcall tx_session_write(int id, const u8 *buf, int n) {
    if (id < 1 || id >= MAX_TTY) return;
    for (int i = 0; i < n; i++) tty_input_t(&g->ttys[id], buf[i]);
    NtSetEvent(g_ui_event, NULL);
}

TX_API void __stdcall tx_session_resize(int id, int rows, int cols) {
    if (id < 1 || id >= MAX_TTY) return;
    Tty *t = &g->ttys[id];
    t->rows = (u16)rows;
    t->cols = (u16)cols;
    lock_();
    if (t->fg_pgrp) signal_pgrp_locked(t->fg_pgrp, SIGWINCH_);
    unlock_();
    wake_all();
}

TX_API int __stdcall tx_session_status(int id) { return id >= 1 && id < MAX_TTY ? g->ttys[id].sess_status : 0; }

TX_API void __stdcall tx_session_close(int id) {
    if (id < 1 || id >= MAX_TTY) return;
    Tty *t = &g->ttys[id];
    lock_();
    int leader = t->leader_pid;
    for (int i = 0; i < MAX_PROC && leader; i++) {
        Proc *p = &g->procs[i];
        if (p->state == 1 && p->sid == leader) send_signal_locked(p, SIGKILL_);
    }
    t->master_refs = 0;
    t->hup = 1;
    t->leader_pid = 0;
    t->is_session = 0;
    if (t->slave_refs <= 0) t->used = 0;
    unlock_();
    wake_all();
}
