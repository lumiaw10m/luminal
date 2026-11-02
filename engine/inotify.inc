#define MAX_INO 8
#define MAX_WATCH 64
#define IN_MODIFY_ 0x2
#define IN_ATTRIB_ 0x4
#define IN_CLOSE_WRITE_ 0x8
#define IN_MOVED_FROM_ 0x40
#define IN_MOVED_TO_ 0x80
#define IN_CREATE_ 0x100
#define IN_DELETE_ 0x200
#define IN_DELETE_SELF_ 0x400
#define IN_Q_OVERFLOW_ 0x4000
#define IN_ISDIR_ 0x40000000u

NTF(NtNotifyChangeDirectoryFile)(HANDLE, HANDLE, PVOID, PVOID, IO_STATUS_BLOCK *, PVOID, ULONG, ULONG, BOOLEAN);

typedef struct {
    int used, ino, wd;
    u32 mask;
    HANDLE dir, ev;
    IO_STATUS_BLOCK io;
    char path[260];
    char file[256];
    u8 buf[2048];
} Watch;

typedef struct {
    int used, owner, next_wd;
    u32 len, pos;
    u32 cookie;
    u8 out[4096];
} Ino;

static Ino g_ino[MAX_INO];
static Watch g_watch[MAX_WATCH];

static void watch_arm(Watch *w) {
    NtNotifyChangeDirectoryFile(w->dir, w->ev, NULL, NULL, &w->io, w->buf, sizeof(w->buf), 0x5F, FALSE);
}

static void ino_emit(Ino *q, int wd, u32 mask, u32 cookie, const char *name) {
    u32 nl = name && *name ? (u32)strlen(name) + 1 : 0, pad = (nl + 3) & ~3u, total = 16 + pad;
    if (q->pos == q->len) q->pos = q->len = 0;
    if (q->len + total > sizeof(q->out)) return;
    u32 *h = (u32 *)(q->out + q->len);
    h[0] = (u32)wd;
    h[1] = mask;
    h[2] = cookie;
    h[3] = pad;
    memset(q->out + q->len + 16, 0, pad);
    if (nl) memcpy(q->out + q->len + 16, name, nl);
    q->len += total;
}

static void watch_event(Ino *q, Watch *w, u32 action, const char *name) {
    static char full[PATH_MAX_];
    u32 m = 0;
    if (action == 1) m = IN_CREATE_;
    else if (action == 2) m = IN_DELETE_;
    else if (action == 3) m = IN_MODIFY_ | IN_ATTRIB_;
    else if (action == 4) m = IN_MOVED_FROM_;
    else if (action == 5) m = IN_MOVED_TO_;
    if (w->file[0]) {
        if (strcmp(name, w->file)) return;
        if (action == 2 || action == 4) m = IN_DELETE_SELF_;
        name = "";
    }
    if (action != 2 && action != 4) {
        _snprintf(full, sizeof(full), "%s/%s", w->path, w->file[0] ? w->file : name);
        full[sizeof(full) - 1] = 0;
        ULONG a = path_attr(full);
        if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) m |= IN_ISDIR_;
    }
    if (action == 4) q->cookie++;
    u32 cookie = action == 4 || action == 5 ? q->cookie : 0;
    if (w->mask & (m & 0xFFFF & ~(u32)IN_ATTRIB_)) ino_emit(q, w->wd, (m & ~(u32)IN_ATTRIB_) & (w->mask | IN_ISDIR_), cookie, name);
    if (action == 3 && !(m & IN_ISDIR_) && (w->mask & IN_ATTRIB_) && !(w->mask & IN_MODIFY_)) ino_emit(q, w->wd, IN_ATTRIB_, 0, name);
    if (action == 3 && !(m & IN_ISDIR_) && (w->mask & IN_CLOSE_WRITE_)) ino_emit(q, w->wd, IN_CLOSE_WRITE_, 0, name);
}

static void ino_collect(int i) {
    Ino *q = &g_ino[i];
    if (q->owner != me->pid) return;
    for (int k = 0; k < MAX_WATCH; k++) {
        Watch *w = &g_watch[k];
        LARGE_INTEGER zero;
        zero.QuadPart = 0;
        if (!w->used || w->ino != i || NtWaitForSingleObject(w->ev, FALSE, &zero) != 0) continue;
        if (w->io.Status == 0x0000010C || !w->io.Information) {
            ino_emit(q, -1, IN_Q_OVERFLOW_, 0, NULL);
        } else if (NT_OK(w->io.Status)) {
            u32 off = 0;
            for (;;) {
                u32 *n = (u32 *)(w->buf + off);
                char name[256];
                int len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)&n[3], n[2] / 2, name, sizeof(name) - 1, NULL, NULL);
                name[len] = 0;
                watch_event(q, w, n[1], name);
                if (!n[0]) break;
                off += n[0];
            }
        }
        watch_arm(w);
    }
}

static int ino_ready(int i) {
    ino_collect(i);
    return g_ino[i].pos < g_ino[i].len;
}

static int ino_init(int flags) {
    int n = fd_alloc(0);
    if (n < 0) return n;
    int i;
    for (i = 0; i < MAX_INO; i++)
        if (!g_ino[i].used || g_ino[i].owner != me->pid) break;
    if (i == MAX_INO) return -24;
    memset(&g_ino[i], 0, sizeof(g_ino[i]));
    g_ino[i].used = 1;
    g_ino[i].owner = me->pid;
    g_ino[i].next_wd = 1;
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = F_INOTIFY;
    g_fd[n].idx = i;
    g_fd[n].flags = flags & O_NONBLOCK_;
    g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
    g_fdpath[n][0] = 0;
    return n;
}

static int ino_add(int fd, const char *path, u32 mask) {
    static char abs[PATH_MAX_];
    if (fd < 0 || fd >= MAX_FD || g_fd[fd].type != F_INOTIFY) return -EINVAL;
    int i = g_fd[fd].idx;
    if (g_ino[i].owner != me->pid) return -EINVAL;
    int r = resolve(AT_FDCWD_, path, !(mask & 0x02000000), abs);
    if (r) return r;
    ULONG a = path_attr(abs);
    if (a == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    char dir[260], file[256];
    file[0] = 0;
    if (strlen(abs) >= sizeof(dir)) return -36;
    strcpy(dir, abs);
    if (!(a & FILE_ATTRIBUTE_DIRECTORY)) {
        char *sl = strrchr(dir, '/');
        strncpy(file, sl + 1, sizeof(file) - 1);
        file[sizeof(file) - 1] = 0;
        if (sl == dir) dir[1] = 0;
        else *sl = 0;
    }
    int freeslot = -1;
    for (int k = 0; k < MAX_WATCH; k++) {
        Watch *w = &g_watch[k];
        if (!w->used) { if (freeslot < 0) freeslot = k; continue; }
        if (w->ino == i && !strcmp(w->path, dir) && !strcmp(w->file, file)) {
            w->mask = mask;
            return w->wd;
        }
    }
    if (freeslot < 0) return -28;
    Watch *w = &g_watch[freeslot];
    memset(w, 0, sizeof(*w) - sizeof(w->buf));

    static wchar_t wp[PATH_MAX_];
    UNICODE_STRING us;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK oio;
    init_oa(&oa, &us, wp, nt_path(dir, wp), OBJ_INHERIT_);
    NTSTATUS st = NtCreateFile(&w->dir, FILE_LIST_DIRECTORY | SYNCHRONIZE, &oa, &oio, NULL, 0, SHARE_ALL, FILE_OPEN_, FO_DIR | FO_BACKUP, NULL, 0);
    if (!NT_OK(st)) return -errno_of(st);
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(oa);
    oa.Attributes = OBJ_INHERIT_;
    NtCreateEvent(&w->ev, EVENT_ALL_ACCESS, &oa, 0  , FALSE);
    strcpy(w->path, dir);
    strcpy(w->file, file);
    w->used = 1;
    w->ino = i;
    w->mask = mask;
    w->wd = g_ino[i].next_wd++;
    watch_arm(w);
    return w->wd;
}

static void watch_free(Watch *w) {
    if (g_ino[w->ino].owner == me->pid) {
        NtClose(w->dir);
        NtClose(w->ev);
    }
    w->used = 0;
}

static int ino_rm(int fd, int wd) {
    if (fd < 0 || fd >= MAX_FD || g_fd[fd].type != F_INOTIFY) return -EINVAL;
    for (int k = 0; k < MAX_WATCH; k++) {
        Watch *w = &g_watch[k];
        if (w->used && w->ino == g_fd[fd].idx && w->wd == wd) {
            ino_emit(&g_ino[w->ino], wd, 0x8000, 0, NULL);
            watch_free(w);
            return 0;
        }
    }
    return -EINVAL;
}

static void ino_close(int i) {
    for (int k = 0; k < MAX_WATCH; k++)
        if (g_watch[k].used && g_watch[k].ino == i) watch_free(&g_watch[k]);
    if (g_ino[i].owner == me->pid) g_ino[i].used = 0;
}

static int ino_read(int fd, u8 *buf, u32 len) {
    int i = g_fd[fd].idx;
    while (!ino_ready(i)) {
        if (g_fd[fd].flags & O_NONBLOCK_) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(50);
    }
    Ino *q = &g_ino[i];
    u32 done = 0;
    while (q->pos < q->len) {
        u32 total = 16 + *(u32 *)(q->out + q->pos + 12);
        if (done + total > len) break;
        memcpy(buf + done, q->out + q->pos, total);
        done += total;
        q->pos += total;
    }
    return done ? (int)done : -EINVAL;
}
