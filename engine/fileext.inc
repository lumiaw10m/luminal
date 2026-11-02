#define MAX_SHARED_MAPS 32
typedef struct { u32 addr, len; i64 off; HANDLE h; } SharedMap;
static SharedMap g_smaps[MAX_SHARED_MAPS];

static void smap_add(u32 addr, u32 len, i64 off, HANDLE file) {
    for (int i = 0; i < MAX_SHARED_MAPS; i++) {
        if (g_smaps[i].len) continue;
        if (!NT_OK(NtDuplicateObject(SELF, file, SELF, &g_smaps[i].h, 0, OBJ_INHERIT_, DUPLICATE_SAME_ACCESS))) return;
        g_smaps[i].addr = addr;
        g_smaps[i].len = len;
        g_smaps[i].off = off;
        return;
    }
}

static void smap_write(SharedMap *m, u32 addr, u32 len) {
    IO_STATUS_BLOCK io;
    FILE_STD std;
    LARGE_INTEGER off;
    i64 at = m->off + (addr - m->addr);
    if (!NT_OK(NtQueryInformationFile(m->h, &io, &std, sizeof(std), 5))) return;
    if (at >= std.eof.QuadPart) return;
    if (at + len > std.eof.QuadPart) len = (u32)(std.eof.QuadPart - at);
    off.QuadPart = at;
    NtWriteFile(m->h, NULL, NULL, NULL, &io, (void *)(ULONG_PTR)addr, len, &off, NULL);
}

static void smap_sync(u32 addr, u32 len, int drop) {
    for (int i = 0; i < MAX_SHARED_MAPS; i++) {
        SharedMap *m = &g_smaps[i];
        if (!m->len || addr >= m->addr + m->len || addr + len <= m->addr) continue;
        u32 lo = addr > m->addr ? addr : m->addr;
        u32 hi = addr + len < m->addr + m->len ? addr + len : m->addr + m->len;
        smap_write(m, lo, hi - lo);
        if (drop && lo == m->addr && hi == m->addr + m->len) {
            NtClose(m->h);
            memset(m, 0, sizeof(*m));
        }
    }
}

static void smap_sync_all(void) { smap_sync(0, 0xFFFFFFFF, 1); }

static int sys_mknod(int dirfd, const char *path, u32 mode) {
    static char abs[PATH_MAX_];
    int r = resolve(dirfd, path, 0, abs);
    if (r) return r;
    if (path_attr(abs) != INVALID_FILE_ATTRIBUTES) return -EEXIST;
    u32 type = mode & S_IFMT_;
    if (type && type != S_IFREG_ && type != S_IFIFO_) return -EPERM;
    HANDLE h;
    NTSTATUS st = nt_open(abs, GENERIC_WRITE | FILE_WRITE_EA, FILE_CREATE_, FO_NONDIR, type == S_IFIFO_ ? FILE_ATTRIBUTE_HIDDEN : FILE_ATTRIBUTE_NORMAL, &h);
    if (!NT_OK(st)) return -errno_of(st);
    TxMeta m = {(type ? type : S_IFREG_) | (mode & ~g_umask & 07777), 0, 0};
    meta_set(h, &m);
    NtClose(h);
    return 0;
}

static int fifo_open(const char *abs, int n, int flags) {
    int write_side = (flags & 3) == O_WRONLY_, i, pipe_i;
    if (strlen(abs) >= sizeof(g->fifos[0].path)) return -36;
    lock_();
    int slot = -1;
    for (i = 0; i < MAX_FIFO; i++) {
        if (g->fifos[i].used && !strcmp(g->fifos[i].path, abs)) slot = i;
    }
    if (slot < 0) {
        for (i = 0; i < MAX_FIFO && g->fifos[i].used; i++) {}
        if (i == MAX_FIFO) { unlock_(); return -ENFILE; }
        for (pipe_i = 0; pipe_i < MAX_PIPE; pipe_i++) {
            Pipe *p = &g->pipes[pipe_i];
            if (p->readers <= 0 && p->writers <= 0 && !p->reserved) break;
        }
        if (pipe_i == MAX_PIPE) { unlock_(); return -ENFILE; }
        slot = i;
        g->fifos[slot].used = 1;
        g->fifos[slot].pipe = pipe_i;
        strcpy(g->fifos[slot].path, abs);
        memset(&g->pipes[pipe_i], 0, sizeof(Pipe));
        g->pipes[pipe_i].reserved = 1;
    }
    pipe_i = g->fifos[slot].pipe;
    Pipe *p = &g->pipes[pipe_i];
    if (write_side && !p->readers && (flags & O_NONBLOCK_)) {
        unlock_();
        return -ENXIO_;
    }
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = write_side ? F_PIPE_W : F_PIPE_R;
    g_fd[n].idx = pipe_i;
    g_fd[n].flags = flags;
    g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
    strncpy(g_fdpath[n], abs, 255);
    pipe_ref_add(&g_fd[n]);
    unlock_();
    wake_all();
    if (!(flags & O_NONBLOCK_)) {
        while (write_side ? !p->readers : !p->writers) {
            if (signal_ready()) {
                fd_close(n);
                return -EINTR;
            }
            wait_event(50);
        }
    }
    return n;
}
