#define PROC_FILE_MAX 16384
static const char *const g_proc_root_files[] = {"cpuinfo", "filesystems", "loadavg", "meminfo", "mounts", "stat", "uptime", "version"};
static const char *const g_proc_pid_files[] = {"cmdline", "comm", "stat", "statm", "status", "task"};

NTF(NtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);

static Proc *proc_lookup(const char *name, size_t len) {
    if ((len == 4 && !memcmp(name, "self", 4)) || (len == 11 && !memcmp(name, "thread-self", 11))) return me;
    int pid = 0;
    if (!len) return NULL;
    for (size_t i = 0; i < len; i++) {
        if (name[i] < '0' || name[i] > '9') return NULL;
        pid = pid * 10 + (name[i] - '0');
    }
    return proc_by_pid(pid);
}

static void proc_times(Proc *p, u32 *utime, u32 *stime, u32 *start, u32 *rss_pages, u32 *vsize) {
    HANDLE h = NULL;
    CLIENT_ID id;
    OBJECT_ATTRIBUTES oa;
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(oa);
    id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
    id.UniqueThread = NULL;
    *utime = *stime = *start = *rss_pages = *vsize = 0;
    if (!p->nt_pid || !NT_OK(NtOpenProcess(&h, PROCESS_QUERY_LIMITED_INFORMATION, &oa, &id))) return;
    i64 t[4];
    if (NT_OK(NtQueryInformationProcess(h, 4  , t, sizeof(t), NULL))) {

        u64 now = now_100ns(), mono = mono_ns() / 100;
        *stime = (u32)(t[2] / 100000);
        *utime = (u32)(t[3] / 100000);
        if ((u64)t[0] + mono > now) *start = (u32)(((u64)t[0] + mono - now) / 100000);
    }
    ULONG vm[16];
    if (NT_OK(NtQueryInformationProcess(h, 3  , vm, sizeof(vm), NULL))) {
        *vsize = vm[1];
        *rss_pages = vm[4] / PG;
    }
    NtClose(h);
}

static int proc_pid_file(Proc *p, const char *file, char *o, int max) {
    char st = p->state == 2 ? 'Z' : p->waiting ? 'S' : 'R';
    const char *comm = p->comm[0] ? p->comm : "tx";
    u32 ut, stm, start, rss, vs;
    if (!strcmp(file, "cmdline")) {
        int n = p->cmdlen < max ? p->cmdlen : max;
        memcpy(o, p->cmd, n);
        return n;
    }
    if (!strcmp(file, "comm")) return _snprintf(o, max, "%s\n", comm);
    proc_times(p, &ut, &stm, &start, &rss, &vs);
    if (!strcmp(file, "stat")) {
        return _snprintf(o, max, "%d (%s) %c %d %d %d 34816 %d 0 0 0 0 0 %u %u 0 0 20 0 1 0 %u %u %u 4294967295 0 0 0 0 0 0 0 0 0 0 0 0 17 0 0 0 0 0 0\n",
                         p->pid, comm, st, p->ppid, p->pgid, p->sid, g->ttys[0].fg_pgrp, ut, stm, start, vs, rss);
    }
    if (!strcmp(file, "statm")) return _snprintf(o, max, "%u %u 0 0 0 0 0\n", vs / PG, rss);
    if (!strcmp(file, "status")) {
        return _snprintf(o, max, "Name:\t%s\nState:\t%c\nTgid:\t%d\nPid:\t%d\nPPid:\t%d\nUid:\t0\t0\t0\t0\nGid:\t0\t0\t0\t0\nVmSize:\t%u kB\nVmRSS:\t%u kB\nThreads:\t1\n",
                         comm, st, p->pid, p->pid, p->ppid, vs / 1024, rss * 4);
    }
    return -ENOENT;
}

static int proc_root_file(const char *file, char *o, int max) {
    if (!strcmp(file, "uptime")) {
        u64 ms = mono_ns() / 10000000ULL;
        return _snprintf(o, max, "%u.%02u %u.%02u\n", (u32)(ms / 100), (u32)(ms % 100), (u32)(ms / 100), (u32)(ms % 100));
    }
    if (!strcmp(file, "meminfo")) {

        ULONG basic[16], perf[96];
        memset(basic, 0, sizeof(basic));
        memset(perf, 0, sizeof(perf));
        NtQuerySystemInformation(0, basic, 44, NULL);
        NtQuerySystemInformation(2, perf, sizeof(perf), NULL);
        u32 tot = basic[3] * 4, av = perf[11] * 4;
        u32 limit = perf[13] * 4, used = perf[12] * 4;
        u32 swt = limit > tot ? limit - tot : 0;
        u32 swf = limit > used ? limit - used : 0;
        if (swf > swt) swf = swt;
        return _snprintf(o, max, "MemTotal:       %8u kB\nMemFree:        %8u kB\nMemAvailable:   %8u kB\nBuffers:        %8u kB\nCached:         %8u kB\nSwapCached:     %8u kB\nSwapTotal:      %8u kB\nSwapFree:       %8u kB\nShmem:          %8u kB\nSReclaimable:   %8u kB\n",
                         tot, av, av, 0, 0, 0, swt, swf, 0, 0);
    }
    if (!strcmp(file, "stat")) {

        static i64 pc[6][6];
        memset(pc, 0, sizeof(pc));
        NtQuerySystemInformation(8, pc, sizeof(pc), NULL);
        u64 ti = 0, ts = 0, tu = 0;
        for (int i = 0; i < 6; i++) {
            ti += (u64)pc[i][0];
            ts += (u64)(pc[i][1] - pc[i][0]);
            tu += (u64)pc[i][2];
        }
        int n = _snprintf(o, max, "cpu  %llu 0 %llu %llu 0 0 0 0 0 0\n", tu / 100000, ts / 100000, ti / 100000);
        for (int i = 0; i < 6; i++)
            n += _snprintf(o + n, max - n, "cpu%d %llu 0 %llu %llu 0 0 0 0 0 0\n", i, (u64)pc[i][2] / 100000,
                           (u64)(pc[i][1] - pc[i][0]) / 100000, (u64)pc[i][0] / 100000);
        n += _snprintf(o + n, max - n, "intr 0\nctxt 0\nbtime %u\nprocesses 0\nprocs_running 1\nprocs_blocked 0\n",
                       (u32)((now_100ns() - 116444736000000000ULL) / 10000000) - (u32)(mono_ns() / 1000000000ULL));
        return n;
    }
    if (!strcmp(file, "loadavg")) return _snprintf(o, max, "0.00 0.00 0.00 1/1 %d\n", g->next_pid);
    if (!strcmp(file, "cpuinfo")) {
        int n = 0;
        for (int i = 0; i < 6; i++)
            n += _snprintf(o + n, max - n, "processor\t: %d\nmodel name\t: ARMv7 Processor rev 3 (v7l)\nFeatures\t: half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt lpae\nCPU architecture: 7\nHardware\t: Lumia 950 (Snapdragon 808)\n\n", i);
        return n;
    }
    if (!strcmp(file, "version")) return _snprintf(o, max, "Linux version 6.6.0-tx (tx on Windows 10 Mobile)\n");
    if (!strcmp(file, "mounts")) return _snprintf(o, max, "rootfs / ntfs rw 0 0\nproc /proc proc rw 0 0\n");
    if (!strcmp(file, "filesystems")) return _snprintf(o, max, "\tntfs\nnodev\tproc\n");
    return -ENOENT;
}

static int proc_net_file(const char *file, char *o, int max) {
    int n = 0;
    if (!strcmp(file, "dev")) {
        n = _snprintf(o, max, "Inter-|   Receive                                                |  Transmit\n face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n");
        for (int i = 0; i < g->nnetif; i++)
            n += _snprintf(o + n, max - n, "%6s: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n", g->netifs[i].name);
        return n;
    }
    if (!strcmp(file, "route")) {
        n = _snprintf(o, max, "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU\tWindow\tIRTT\n");
        for (int i = 0; i < g->nnetif; i++) {
            NetIf *f = &g->netifs[i];
            if (f->loopback) continue;
            if (f->gw) n += _snprintf(o + n, max - n, "%s\t00000000\t%08X\t0003\t0\t0\t0\t00000000\t0\t0\t0\n", f->name, f->gw);
            n += _snprintf(o + n, max - n, "%s\t%08X\t00000000\t0001\t0\t0\t0\t%08X\t0\t0\t0\n", f->name, f->ip & f->mask, f->mask);
        }
        return n;
    }
    return -ENOENT;
}

static int proc_classify(const char *abs, char *o, int max, int *len) {
    const char *r = abs + 5;
    if (!*r || !strcmp(r, "/")) return 1;
    r++;
    const char *slash = strchr(r, '/');
    size_t nl = slash ? (size_t)(slash - r) : strlen(r);
    if (nl == 3 && !memcmp(r, "net", 3)) {
        if (!slash || !slash[1]) return 1;
        int n = proc_net_file(slash + 1, o, max);
        if (n < 0) return n;
        *len = n;
        return 2;
    }
    Proc *p = proc_lookup(r, nl);
    if (p) {
        if (!slash || !slash[1]) return 1;
        const char *file = slash + 1;
        if (!strcmp(file, "fd")) return 1;

        if (!strncmp(file, "task", 4) && (file[4] == 0 || file[4] == '/')) {
            if (!file[4] || !file[5]) return 1;
            const char *tid = file + 5, *ts = strchr(tid, '/');
            size_t tl = ts ? (size_t)(ts - tid) : strlen(tid);
            if (proc_lookup(tid, tl) != p) return -ENOENT;
            if (!ts || !ts[1]) return 1;
            file = ts + 1;
        }
        int n = proc_pid_file(p, file, o, max);
        if (n < 0) return n;
        *len = n;
        return 2;
    }
    if (slash) return -ENOENT;
    int n = proc_root_file(r, o, max);
    if (n < 0) return n;
    *len = n;
    return 2;
}

static char g_proc_buf[PROC_FILE_MAX];

static int proc_open(const char *abs, int n, int flags) {
    int len = 0;
    int k = proc_classify(abs, g_proc_buf, PROC_FILE_MAX, &len);
    if (k < 0) return k;
    if ((flags & 3) != 0) return -EACCES;
    Fd *f = &g_fd[n];
    memset(f, 0, sizeof(*f));
    f->flags = flags;
    f->cloexec = !!(flags & O_CLOEXEC_);
    strncpy(g_fdpath[n], abs, 255);
    g_fdpath[n][255] = 0;
    if (k == 1) {
        if (strlen(abs) < 5 || flags & 0) return -EINVAL;
        f->type = F_DIR;
        return n;
    }
    PVOID p = NULL;
    SIZE_T s = len ? len : 1;
    if (!NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))) return -ENOMEM;
    memcpy(p, g_proc_buf, len);
    f->type = F_MEM;
    f->mem = p;
    f->memlen = len;
    return n;
}

static int proc_put_dent(u8 *buf, u32 size, u32 *used, const char *name, int type, u64 ino) {
    int l = (int)strlen(name);
    u32 rec = (19 + l + 1 + 7) & ~7u;
    if (*used + rec > size) return 0;
    u8 *d = buf + *used;
    memcpy(d, &ino, 8);
    i64 off = *used + rec;
    memcpy(d + 8, &off, 8);
    *(u16 *)(d + 16) = (u16)rec;
    d[18] = (u8)type;
    memcpy(d + 19, name, l + 1);
    *used += rec;
    return 1;
}

static int proc_getdents(Fd *f, const char *abs, u8 *buf, u32 size) {
    u32 used = 0;
    int pos = f->pos;
    char name[16];
    if (f->restart) { pos = 0; f->restart = 0; }
    if (!strcmp(abs, "/proc") || !strcmp(abs, "/proc/")) {
        int nroot = sizeof(g_proc_root_files) / sizeof(g_proc_root_files[0]);
        for (; pos < 2 + nroot + 1 + MAX_PROC; pos++) {
            int ok = 1;
            if (pos == 0) ok = proc_put_dent(buf, size, &used, ".", 4, 1);
            else if (pos == 1) ok = proc_put_dent(buf, size, &used, "..", 4, 2);
            else if (pos < 2 + nroot) ok = proc_put_dent(buf, size, &used, g_proc_root_files[pos - 2], 8, 3 + pos);
            else if (pos == 2 + nroot) ok = proc_put_dent(buf, size, &used, "self", 4, 50);
            else {
                Proc *p = &g->procs[pos - (3 + nroot)];
                if (p->state == 1 || p->state == 2) {
                    _snprintf(name, sizeof(name), "%d", p->pid);
                    ok = proc_put_dent(buf, size, &used, name, 4, 100 + p->pid);
                }
            }
            if (!ok) break;
        }
    } else if (!strcmp(abs, "/proc/net")) {
        static const char *const net_files[] = {".", "..", "dev", "route"};
        for (; pos < 4; pos++)
            if (!proc_put_dent(buf, size, &used, net_files[pos], pos < 2 ? 4 : 8, 60 + pos)) break;
    } else if (strlen(abs) > 3 && !strcmp(abs + strlen(abs) - 3, "/fd")) {

        for (; pos < 2 + MAX_FD; pos++) {
            int ok = 1;
            if (pos == 0) ok = proc_put_dent(buf, size, &used, ".", 4, 1);
            else if (pos == 1) ok = proc_put_dent(buf, size, &used, "..", 4, 2);
            else if (g_fd[pos - 2].type) {
                _snprintf(name, sizeof(name), "%d", pos - 2);
                ok = proc_put_dent(buf, size, &used, name, 10, 500 + pos);
            }
            if (!ok) break;
        }
    } else if (strlen(abs) > 5 && !strcmp(abs + strlen(abs) - 5, "/task")) {
        Proc *p = proc_lookup(abs + 6, strcspn(abs + 6, "/"));
        for (; pos < 3; pos++) {
            int ok = 1;
            if (pos == 0) ok = proc_put_dent(buf, size, &used, ".", 4, 1);
            else if (pos == 1) ok = proc_put_dent(buf, size, &used, "..", 4, 2);
            else if (p) {
                _snprintf(name, sizeof(name), "%d", p->pid);
                ok = proc_put_dent(buf, size, &used, name, 4, 100 + p->pid);
            }
            if (!ok) break;
        }
    } else {
        int nf = sizeof(g_proc_pid_files) / sizeof(g_proc_pid_files[0]);
        for (; pos < 2 + nf; pos++) {
            int ok;
            if (pos == 0) ok = proc_put_dent(buf, size, &used, ".", 4, 1);
            else if (pos == 1) ok = proc_put_dent(buf, size, &used, "..", 4, 2);
            else ok = proc_put_dent(buf, size, &used, g_proc_pid_files[pos - 2], 8, 3 + pos);
            if (!ok) break;
        }
    }
    f->pos = pos;
    return (int)used;
}
