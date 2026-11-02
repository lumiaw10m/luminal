NTF(NtCreateThreadEx)(HANDLE *, ACCESS_MASK, PVOID, HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);

static DWORD WINAPI thread_boot(LPVOID param) {
    T *t = param;
    NT_TIB *tib = (NT_TIB *)NtCurrentTeb();
    tib->FiberData = t;
    tib->ArbitraryUserPointer = (PVOID)(ULONG_PTR)t->fork_tls;
    tib->StackBase = (PVOID)0x7FFE0000;
    tib->StackLimit = (PVOID)0x00010000;

    t->host_top = (u8 *)(((ULONG_PTR)&t - 4096) & ~7u);
    NtContinue(&t->fork_ctx, FALSE);
    return 0;
}

static int do_thread_create(CONTEXT *c, u32 flags, u32 stack, u32 ptid, u32 tls, u32 ctid) {
    T *t = NULL;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (!g_threads[i].used) {
            t = &g_threads[i];
            break;
        }
    }
    if (!t) return -EAGAIN;
    T *self = CUR;
    HANDLE ev = t->fut_ev;
    memset(t, 0, sizeof(*t));
    t->fut_ev = ev;
    t->used = 1;
    lock_();
    t->tid = g->next_pid++;
    unlock_();
    t->blocked = self->blocked;
    t->fork_ctx = *c;
    t->fork_ctx.R0 = 0;
    t->fork_ctx.Sp = stack;
    t->fork_ctx.Pc = (c->Cpsr & 0x20) ? (c->Pc + 2) | 1 : c->Pc + 4;
    t->fork_tls = (flags & 0x80000) ? tls : (u32)(ULONG_PTR)((NT_TIB *)NtCurrentTeb())->ArbitraryUserPointer;
    if (flags & 0x200000) t->clear_tid = ctid;
    if (flags & 0x100000) *(u32 *)(ULONG_PTR)ptid = t->tid;
    if (flags & 0x1000000) *(u32 *)(ULONG_PTR)ctid = t->tid;
    g_nthreads++;
    HANDLE h = NULL;

    NTSTATUS st = NtCreateThreadEx(&h, THREAD_ALL_ACCESS, NULL, SELF, thread_boot, t, 2, 0, 0x20000, 0x20000, NULL);
    if (!NT_OK(st)) {
        trace("thread create failed %08x\n", st);
        t->used = 0;
        g_nthreads--;
        return -EAGAIN;
    }
    NtClose(h);
    lock_();
    proc_blocked_update();
    unlock_();
    return t->tid;
}

static int futex_wake(u32 addr, int n) {
    int woke = 0;
    for (int i = 0; i < MAX_THREADS && woke < n; i++) {
        T *t = &g_threads[i];
        if (t->used && t->fut_addr == addr) {
            t->fut_addr = 0;
            NtSetEvent(t->fut_ev, NULL);
            woke++;
        }
    }
    return woke;
}

static int futex_wait(u32 addr, u32 val, i64 timeout_100ns) {
    T *t = CUR;
    if (*(volatile u32 *)(ULONG_PTR)addr != val) return -EAGAIN;
    if (!t->fut_ev) {
        OBJECT_ATTRIBUTES oa;
        memset(&oa, 0, sizeof(oa));
        oa.Length = sizeof(oa);
        oa.Attributes = OBJ_INHERIT_;
        NtCreateEvent(&t->fut_ev, EVENT_ALL_ACCESS, &oa, 1, FALSE);
    }
    u64 start = mono_ns();
    t->fut_addr = addr;
    for (;;) {
        LARGE_INTEGER w;
        i64 slice = 500000;
        if (timeout_100ns >= 0) {
            i64 left = timeout_100ns - (i64)(mono_ns() - start) / 100;
            if (left <= 0) {
                t->fut_addr = 0;
                return -110;
            }
            if (left < slice) slice = left;
        }
        w.QuadPart = -slice;
        bkl_unlock();
        NtWaitForSingleObject(t->fut_ev, FALSE, &w);
        bkl_lock();
        if (!t->fut_addr) return 0;
        timers_poll();
        if (signal_ready()) {
            t->fut_addr = 0;
            return -EINTR;
        }
    }
}

static int sys_futex(u32 addr, u32 op, u32 val, u32 ts, u32 addr2, u32 val3, int ts64) {
    u32 cmd = op & 0x7F;
    if (cmd == 0 || cmd == 9) {
        i64 to = -1;
        if (ts) {
            i64 sec, nsec;
            if (ts64) { sec = ((i64 *)(ULONG_PTR)ts)[0]; nsec = ((i64 *)(ULONG_PTR)ts)[1]; }
            else { sec = ((long *)(ULONG_PTR)ts)[0]; nsec = ((long *)(ULONG_PTR)ts)[1]; }
            to = sec * 10000000 + nsec / 100;
            if (cmd == 9) {

                i64 s2, n2;
                fill_time((op & 256) ? 0 : 1, &s2, &n2);
                to -= s2 * 10000000 + n2 / 100;
                if (to < 0) to = 0;
            }
        }
        return futex_wait(addr, val, to);
    }
    if (cmd == 1 || cmd == 10) return futex_wake(addr, (int)val);
    if (cmd == 3 || cmd == 4) {
        if (cmd == 4 && *(volatile u32 *)(ULONG_PTR)addr != val3) return -EAGAIN;
        int woke = futex_wake(addr, (int)val), moved = 0, limit = (int)ts;
        for (int i = 0; i < MAX_THREADS && moved < limit; i++) {
            T *t = &g_threads[i];
            if (t->used && t->fut_addr == addr) {
                t->fut_addr = addr2;
                moved++;
            }
        }
        return woke + moved;
    }
    return -ENOSYS;
}

static int sys_munmap(u32 addr, u32 len);

static void __cdecl thread_end_stub(void) {
    T *t = CUR;
    bkl_lock();
    if (t->unmap_len) sys_munmap(t->unmap_addr, t->unmap_len);
    t->used = 0;
    t->fut_addr = 0;
    g_nthreads--;
    lock_();
    proc_blocked_update();
    unlock_();
    bkl_unlock();
    NtTerminateThread((HANDLE)(LONG_PTR)-2, 0);
}

static void thread_exit(CONTEXT *c, int code) {
    T *t = CUR;
    if (t->clear_tid) {
        *(u32 *)(ULONG_PTR)t->clear_tid = 0;
        futex_wake(t->clear_tid, 1);
    }
    if (g_nthreads <= 1) do_exit((code & 0xFF) << 8);
    c->Sp = (DWORD)(ULONG_PTR)t->host_top & ~7u;
    c->Pc = (DWORD)(ULONG_PTR)thread_end_stub | 1u;
    c->Cpsr = (c->Cpsr & ~0x0600FC00u) | 0x20;
    bkl_unlock();
    NtContinue(c, FALSE);
}

static int sys_timer_create(u32 clock, const u32 *sev, u32 *out) {
    (void)clock;
    for (int i = 0; i < MAX_TIMERS - 1; i++) {
        Timer *tm = &g_timers[i];
        if (tm->used) continue;
        memset(tm, 0, sizeof(*tm));
        tm->used = 1;
        tm->signo = SIGALRM_;
        tm->tid = 0;
        if (sev) {

            tm->value = sev[0];
            tm->signo = (int)sev[1];
            if ((int)sev[2] == 1) tm->signo = 0;
            if ((int)sev[2] == 4) tm->tid = (int)sev[3];
        }
        *out = (u32)i;
        return 0;
    }
    return -EAGAIN;
}

static int sys_timer_settime(u32 id, u32 flags, i64 ival_ns, i64 val_ns) {
    if (id >= MAX_TIMERS || !g_timers[id].used) return -EINVAL;
    Timer *tm = &g_timers[id];
    tm->interval_ns = (u64)ival_ns;
    if (!val_ns) {
        tm->armed = 0;
        return 0;
    }
    if (flags & 1) {
        tm->deadline_ns = (u64)val_ns;
    } else {
        tm->deadline_ns = mono_ns() + (u64)val_ns;
    }
    tm->armed = 1;
    return 0;
}

static int sys_sigtimedwait(u64 set, u32 *info, i64 timeout_100ns) {
    T *t = CUR;
    u64 start = mono_ns();
    for (;;) {
        timers_poll();
        lock_();
        u64 hit = (me->pending | t->pending) & set;
        int sig = 0;
        if (hit) {
            sig = 1;
            while (!(hit & SIGBIT(sig))) sig++;
            t->pending &= ~SIGBIT(sig);
            me->pending &= ~SIGBIT(sig);
        }
        unlock_();
        if (sig) {
            if (info) {
                memset(info, 0, 128);
                info[0] = sig;

                info[2] = t->timer_fired & SIGBIT(sig) ? (u32)-2 : (u32)-6;
                t->timer_fired &= ~SIGBIT(sig);
            }
            return sig;
        }
        i64 wait_ms = 100;
        if (timeout_100ns >= 0) {
            i64 left = timeout_100ns - (i64)(mono_ns() - start) / 100;
            if (left <= 0) return -EAGAIN;
            if (left / 10000 + 1 < wait_ms) wait_ms = left / 10000 + 1;
        }
        u64 next = timers_next_ns();
        if (next) {
            u64 now = mono_ns();
            i64 ms = next > now ? (i64)((next - now) / 1000000) + 1 : 1;
            if (ms < wait_ms) wait_ms = ms;
        }
        if (signal_ready()) return -EINTR;
        wait_event((int)wait_ms);
    }
}
