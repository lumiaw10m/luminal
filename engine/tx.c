#define WIN32_LEAN_AND_MEAN
#define FD_SETSIZE 256
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "ws2_32.lib")

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef long long i64;
typedef LONG NTSTATUS;

typedef struct { USHORT Length, MaximumLength; PWSTR Buffer; } UNICODE_STRING;
typedef struct { ULONG Length; HANDLE RootDirectory; UNICODE_STRING *ObjectName; ULONG Attributes; PVOID Sd, Qos; } OBJECT_ATTRIBUTES;
typedef struct { union { NTSTATUS Status; PVOID Pointer; }; ULONG_PTR Information; } IO_STATUS_BLOCK;
typedef struct { HANDLE UniqueProcess, UniqueThread; } CLIENT_ID;

#define NTAPI_ __stdcall
#define NTF(name) __declspec(dllimport) NTSTATUS NTAPI_ name
NTF(NtCreateFile)(HANDLE *, ACCESS_MASK, OBJECT_ATTRIBUTES *, IO_STATUS_BLOCK *, LARGE_INTEGER *, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
NTF(NtClose)(HANDLE);
NTF(NtReadFile)(HANDLE, HANDLE, PVOID, PVOID, IO_STATUS_BLOCK *, PVOID, ULONG, LARGE_INTEGER *, ULONG *);
NTF(NtWriteFile)(HANDLE, HANDLE, PVOID, PVOID, IO_STATUS_BLOCK *, const void *, ULONG, LARGE_INTEGER *, ULONG *);
NTF(NtQueryInformationFile)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG, ULONG);
NTF(NtSetInformationFile)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG, ULONG);
NTF(NtQueryDirectoryFile)(HANDLE, HANDLE, PVOID, PVOID, IO_STATUS_BLOCK *, PVOID, ULONG, ULONG, BOOLEAN, UNICODE_STRING *, BOOLEAN);
NTF(NtQueryAttributesFile)(OBJECT_ATTRIBUTES *, PVOID);
NTF(NtSetEaFile)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG);
NTF(NtQueryEaFile)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG, BOOLEAN, PVOID, ULONG, PULONG, BOOLEAN);
NTF(NtFlushBuffersFile)(HANDLE, IO_STATUS_BLOCK *);
NTF(NtDuplicateObject)(HANDLE, HANDLE, HANDLE, HANDLE *, ACCESS_MASK, ULONG, ULONG);
NTF(NtAllocateVirtualMemory)(HANDLE, PVOID *, ULONG_PTR, SIZE_T *, ULONG, ULONG);
NTF(NtFreeVirtualMemory)(HANDLE, PVOID *, SIZE_T *, ULONG);
NTF(NtProtectVirtualMemory)(HANDLE, PVOID *, SIZE_T *, ULONG, PULONG);
NTF(NtFlushInstructionCache)(HANDLE, PVOID, SIZE_T);
NTF(NtWaitForSingleObject)(HANDLE, BOOLEAN, LARGE_INTEGER *);
NTF(NtSetEvent)(HANDLE, PLONG);
NTF(NtCreateEvent)(HANDLE *, ACCESS_MASK, OBJECT_ATTRIBUTES *, ULONG, BOOLEAN);
NTF(NtCreateSection)(HANDLE *, ACCESS_MASK, OBJECT_ATTRIBUTES *, LARGE_INTEGER *, ULONG, ULONG, HANDLE);
NTF(NtMapViewOfSection)(HANDLE, HANDLE, PVOID *, ULONG_PTR, SIZE_T, LARGE_INTEGER *, SIZE_T *, ULONG, ULONG, ULONG);
NTF(NtTerminateProcess)(HANDLE, NTSTATUS);
NTF(NtOpenProcess)(HANDLE *, ACCESS_MASK, OBJECT_ATTRIBUTES *, CLIENT_ID *);
NTF(NtResumeThread)(HANDLE, PULONG);
NTF(NtSuspendProcess)(HANDLE);
NTF(NtResumeProcess)(HANDLE);
NTF(NtTerminateThread)(HANDLE, NTSTATUS);
NTF(NtFlushProcessWriteBuffers)(void);
NTF(NtQueryVirtualMemory)(HANDLE, PVOID, ULONG, PVOID, SIZE_T, SIZE_T *);
NTF(NtQueryInformationThread)(HANDLE, ULONG, PVOID, ULONG, PULONG);
NTF(NtReadVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, SIZE_T *);
NTF(NtSuspendThread)(HANDLE, PULONG);
NTF(NtGetContextThread)(HANDLE, CONTEXT *);
NTF(NtSetContextThread)(HANDLE, CONTEXT *);
NTF(NtGetNextThread)(HANDLE, HANDLE, ACCESS_MASK, ULONG, ULONG, HANDLE *);
NTF(NtWriteVirtualMemory)(HANDLE, PVOID, const void *, SIZE_T, SIZE_T *);
NTF(NtContinue)(CONTEXT *, BOOLEAN);
NTF(NtQuerySystemTime)(LARGE_INTEGER *);
NTF(NtQueryPerformanceCounter)(LARGE_INTEGER *, LARGE_INTEGER *);
NTF(NtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
NTF(NtYieldExecution)(void);
NTF(NtDelayExecution)(BOOLEAN, LARGE_INTEGER *);
NTF(NtCreateUserProcess)(HANDLE *, HANDLE *, ACCESS_MASK, ACCESS_MASK, PVOID, PVOID, ULONG, ULONG, PVOID, PVOID, PVOID);
__declspec(dllimport) PVOID NTAPI_ RtlAddVectoredExceptionHandler(ULONG, PVECTORED_EXCEPTION_HANDLER);
__declspec(dllimport) void NTAPI_ RtlCaptureContext(CONTEXT *);
__declspec(dllimport) ULONG NTAPI_ RtlRandomEx(PULONG);

#define NT_OK(s) ((NTSTATUS)(s) >= 0)
#define SELF ((HANDLE)(LONG_PTR)-1)
#define OBJ_INHERIT_ 2
#define FILE_OPEN_ 1
#define FILE_CREATE_ 2
#define FILE_OPEN_IF_ 3
#define FILE_OVERWRITE_IF_ 5
#define FO_DIR 0x1
#define FO_SYNC 0x20
#define FO_NONDIR 0x40
#define FO_BACKUP 0x4000
#define SHARE_ALL 7
#define CLONE_CHILD 297

#define E2BIG 7
#define ENOEXEC_ 8
#undef EPERM
#undef ENOENT
#undef ESRCH
#undef EINTR
#undef EIO
#undef EBADF
#undef ECHILD
#undef EAGAIN
#undef ENOMEM
#undef EACCES
#undef EFAULT
#undef EBUSY
#undef EEXIST
#undef ENOTDIR
#undef EISDIR
#undef EINVAL
#undef ENFILE
#undef EMFILE
#undef ENOTTY
#undef ESPIPE
#undef EPIPE
#undef ERANGE
#undef ENOSYS
#undef ENOTEMPTY
#undef ELOOP
#undef ENOTSUP
#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ESPIPE 29
#define EPIPE 32
#define ERANGE 34
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define ERESTART_ 512
#define ENXIO_ 6

#define O_WRONLY_ 1
#define O_RDWR_ 2
#define O_CREAT_ 0x40
#define O_EXCL_ 0x80
#define O_TRUNC_ 0x200
#define O_APPEND_ 0x400
#define O_NONBLOCK_ 0x800
#define O_DIRECTORY_ 0x4000
#define O_NOFOLLOW_ 0x8000
#define O_CLOEXEC_ 0x80000

#define AT_FDCWD_ (-100)
#define AT_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_EMPTY_PATH 0x1000

#define S_IFMT_ 0170000
#define S_IFLNK_ 0120000
#define S_IFREG_ 0100000
#define S_IFDIR_ 0040000
#define S_IFCHR_ 0020000
#define S_IFIFO_ 0010000

#define SIGINT_ 2
#define SIGQUIT_ 3
#define SIGILL_ 4
#define SIGKILL_ 9
#define SIGSEGV_ 11
#define SIGPIPE_ 13
#define SIGALRM_ 14
#define SIGCHLD_ 17
#define SIGCONT_ 18
#define SIGSTOP_ 19
#define SIGTSTP_ 20
#define SIGTTIN_ 21
#define SIGTTOU_ 22
#define SIGURG_ 23
#define SIGWINCH_ 28
#define SIGBIT(s) (1ULL << ((s) - 1))
#define SIG_STOP_DEFAULT (SIGBIT(SIGTSTP_) | SIGBIT(SIGTTIN_) | SIGBIT(SIGTTOU_))
#define SIG_IGNORE_DEFAULT (SIGBIT(SIGCHLD_) | SIGBIT(SIGCONT_) | SIGBIT(SIGURG_) | SIGBIT(SIGWINCH_))
#define SA_SIGINFO_ 4
#define SA_RESTART_ 0x10000000u
#define SA_NODEFER_ 0x40000000u
#define SA_RESETHAND_ 0x80000000u

#define MAX_PROC 64
#define MAX_PIPE 64
#define PIPE_SZ 16384
#define MAX_FD 128
#define PATH_MAX_ 1024
#define TTY_IN_SZ 4096
#define TTY_OUT_SZ 16384
#define MAX_TTY 16
#define MAX_UNIX 16
#define MAX_NETIF 4
#define UDG_SZ 16384
#define MAX_FIFO 8
#define MAX_SOCK 128
#define SOCK_RX 65536
#define SOCK_TX 32768

typedef struct {
    u32 c_iflag, c_oflag, c_cflag, c_lflag;
    u8 c_line, c_cc[19];
} ktermios;
#define ICRNL_ 0x100
#define OPOST_ 1
#define ONLCR_ 4
#define ISIG_ 1
#define ICANON_ 2
#define ECHO_ 8
#define ECHOE_ 0x10
#define VINTR_ 0
#define VQUIT_ 1
#define VERASE_ 2
#define VKILL_ 3
#define VEOF_ 4
#define VSUSP_ 10

typedef struct {
    volatile LONG state;
    int pid, ppid, pgid, sid, status;
    volatile DWORD nt_pid;
    volatile u64 pending, blocked, handled, ignored;
    volatile LONG waiting;
    u8 rrefs[MAX_PIPE], wrefs[MAX_PIPE], srefs[MAX_SOCK], trefs[MAX_TTY], mrefs[MAX_TTY], urefs[MAX_UNIX];
    int ctty;
    volatile int stopped;
    int stop_reported;
    char comm[16];
    u32 uid[3], gid[3];
    char cmd[512];
    int cmdlen;
} Proc;

typedef struct {
    u8 type;
    int idx, idx2, flags;
    char path[128];
} FdMsg;

typedef struct {
    int readers, writers;
    volatile u32 head, tail;
    FdMsg fdq[4];
    int nfdq;
    int reserved;
} Pipe;

typedef struct {
    char name[16];
    u32 ip, mask, gw;
    u8 mac[6];
    u16 loopback;
} NetIf;

typedef struct {
    int used, pipe;
    char path[120];
} Fifo;

typedef struct {
    int used, bound, listening, refs;
    char path[120];
    int qrd[8], qwr[8];
    volatile int nq;
    int dgram;
    char dpeer[120];
    volatile u32 dhead, dtail;
    u8 dbuf[UDG_SZ];
} Unix;

typedef struct {
    volatile u32 ihead, itail, ohead, otail;
    ktermios t;
    int fg_pgrp, sid;
    u16 rows, cols;
    int eof_pending;
    int lines;
    int used, master_refs, slave_refs, slave_opened, hup;
    int is_session, leader_pid, sess_done, sess_status;
    u8 edit[1024];
    int edit_len;
    u8 in[TTY_IN_SZ], out[TTY_OUT_SZ];
} Tty;

enum { OP_NONE, OP_BIND, OP_LISTEN, OP_CONNECT, OP_NAME };

typedef struct {
    int state, type, refs;
    volatile int op, op_done, op_res, op_arg;
    u32 op_ip;
    u16 op_port;
    u8 icmp;
    volatile int conn;
    volatile int err, rx_eof, tx_shut, shut_sent;
    u32 laddr, raddr;
    u16 lport, rport;
    volatile u32 rhead, rtail, thead, ttail;
    int q[8];
    volatile int nq;
} Sock;

typedef struct {
    volatile LONG lock;
    volatile int next_pid;
    int done, exit_status;
    Proc procs[MAX_PROC];
    NetIf netifs[MAX_NETIF];
    volatile int nnetif;
    Pipe pipes[MAX_PIPE];
    Tty ttys[MAX_TTY];
    Unix unixs[MAX_UNIX];
    Fifo fifos[MAX_FIFO];
    u8 pipebuf[MAX_PIPE][PIPE_SZ];
    Sock socks[MAX_SOCK];
    u8 sock_rx[MAX_SOCK][SOCK_RX], sock_tx[MAX_SOCK][SOCK_TX];
    u32 host_events[MAX_PROC], host_ui, host_req, host_owner;
    wchar_t host_root[300], host_musl[300];
    volatile int req_tty, req_done, req_has_cmd;
    char req_cmd[4096];
} Shm;

static Shm *g;
static HANDLE g_events[MAX_PROC];
static HANDLE g_shm_sec;
static int g_self;
static Proc *me;

static void lock_(void) {
    int spins = 0;
    while (InterlockedCompareExchange(&g->lock, 1, 0)) {
        if (++spins > 200) NtYieldExecution();
    }
}
static void unlock_(void) { InterlockedExchange(&g->lock, 0); }

static wchar_t g_root_dos[300] = L"U:\\tx";
#define HOME_DIR L"U:\\Users\\Public\\Documents\\Luminal"
static wchar_t g_musl_dos[300];
static HANDLE g_ui_event;
static int g_trace;
static HANDLE g_trace_h;

static void trace(const char *fmt, ...) {
    char b[768];
    va_list a;
    IO_STATUS_BLOCK io;
    LARGE_INTEGER eof;
    if (!g_trace_h) return;
    va_start(a, fmt);
    int n = _vsnprintf(b, sizeof(b) - 1, fmt, a);
    va_end(a);
    if (n < 0) n = sizeof(b) - 1;
    eof.QuadPart = -1;
    eof.HighPart = -1;
    eof.LowPart = 0xFFFFFFFF;
    NtWriteFile(g_trace_h, NULL, NULL, NULL, &io, b, n, &eof, NULL);
}

typedef struct { ULONG_PTR attr; SIZE_T size; ULONG_PTR value; SIZE_T *ret; } PsAttr;
NTF(RtlCreateProcessParametersEx)(PVOID *, UNICODE_STRING *, UNICODE_STRING *, UNICODE_STRING *, UNICODE_STRING *, PVOID, UNICODE_STRING *, UNICODE_STRING *, UNICODE_STRING *, UNICODE_STRING *, ULONG);

static NTSTATUS nt_spawn_clear(const wchar_t *image, const wchar_t *cmdline, HANDLE out, HANDLE *hp) {
    static wchar_t nt[600];
    static struct { PVOID addr; SIZE_T size; } reserve = { (PVOID)0x10000, 0x10000000 - 0x10000 };
    UNICODE_STRING img, ntimg, cl;
    swprintf(nt, 600, L"\\??\\%s", image);
    img.Buffer = (PWSTR)image;
    img.Length = img.MaximumLength = (USHORT)(wcslen(image) * 2);
    ntimg.Buffer = nt;
    ntimg.Length = ntimg.MaximumLength = (USHORT)(wcslen(nt) * 2);
    cl.Buffer = (PWSTR)cmdline;
    cl.Length = cl.MaximumLength = (USHORT)(wcslen(cmdline) * 2);
    u8 *pp = NULL;
    NTSTATUS st = RtlCreateProcessParametersEx((PVOID *)&pp, &img, NULL, NULL, &cl, NULL, NULL, NULL, NULL, NULL, 1);
    if (!NT_OK(st)) return st;
    if (out) {
        *(HANDLE *)(pp + 0x1C) = out;
        *(HANDLE *)(pp + 0x20) = out;
        *(ULONG *)(pp + 0x68) |= STARTF_USESTDHANDLES;
    }
    struct { SIZE_T total; PsAttr a[2]; } al;
    memset(&al, 0, sizeof(al));
    al.total = sizeof(al);
    al.a[0].attr = 0x20005;
    al.a[0].size = ntimg.Length;
    al.a[0].value = (ULONG_PTR)ntimg.Buffer;
    al.a[1].attr = 0x20007;
    al.a[1].size = sizeof(reserve);
    al.a[1].value = (ULONG_PTR)&reserve;
    __declspec(align(8)) char create_info[72];
    memset(create_info, 0, sizeof(create_info));
    *(ULONG *)create_info = sizeof(create_info);
    HANDLE ht = NULL;
    st = NtCreateUserProcess(hp, &ht, MAXIMUM_ALLOWED, MAXIMUM_ALLOWED, NULL, NULL, 0x4, 0, pp, create_info, &al);
    if (NT_OK(st)) NtClose(ht);
    return st;
}

static NTSTATUS nt_fork(HANDLE *hp, HANDLE *ht) {
    __declspec(align(8)) char create_info[72];
    memset(create_info, 0, sizeof(create_info));
    *(ULONG *)create_info = sizeof(create_info);
    return NtCreateUserProcess(hp, ht, MAXIMUM_ALLOWED, MAXIMUM_ALLOWED, NULL, NULL, 0x4  , 0x1  , NULL, create_info, NULL);
}

#define MAX_THREADS 32
#define MAX_TIMERS 16

typedef struct {
    int used, tid;
    u64 blocked, pending, timer_fired;
    u32 clear_tid;
    volatile u32 fut_addr;
    HANDLE fut_ev;
    u8 *host_top;
    u32 guest_sp;
    u32 alt_sp, alt_size;
    u32 fault_addr;
    u64 suspend_old;
    int suspend_active;
    u32 fork_flags, fork_stack, fork_tls, fork_ctid;
    u32 unmap_addr, unmap_len;
    CONTEXT cont_ctx, fork_ctx;
} T;

typedef struct {
    int used, armed, signo, tid;
    u32 value;
    u64 deadline_ns, interval_ns;
} Timer;

static T g_threads[MAX_THREADS];
static Timer g_timers[MAX_TIMERS];
static int g_nthreads = 1;
static volatile LONG g_bkl;

#define CUR ((T *)((NT_TIB *)NtCurrentTeb())->FiberData)

static void *volatile g_bkl_owner;

static void bkl_lock(void) {
    while (InterlockedCompareExchange(&g_bkl, 1, 0)) NtYieldExecution();
    g_bkl_owner = NtCurrentTeb();
}
static void bkl_unlock(void) {
    g_bkl_owner = NULL;
    InterlockedExchange(&g_bkl, 0);
}

static void proc_blocked_update(void) {
    u64 all = ~0ULL;
    for (int i = 0; i < MAX_THREADS; i++)
        if (g_threads[i].used) all &= g_threads[i].blocked;
    me->blocked = all;
}

static void wake_slot(int i) {
    NtSetEvent(g_events[i], NULL);
}

static void send_signal_locked(Proc *p, int sig);

static void wake_all(void) {
    for (int i = 0; i < MAX_PROC; i++)
        if (g->procs[i].state == 1 && g->procs[i].waiting) wake_slot(i);
    NtSetEvent(g_events[MAX_PROC - 1], NULL);
}

static void timers_poll(void);

static void wait_event(int ms) {
    LARGE_INTEGER t;
    timers_poll();
    t.QuadPart = -(i64)ms * 10000;
    InterlockedIncrement(&me->waiting);
    bkl_unlock();
    NtWaitForSingleObject(g_events[g_self], FALSE, &t);
    bkl_lock();
    if (InterlockedDecrement(&me->waiting) > 0) NtSetEvent(g_events[g_self], NULL);
}

static u64 now_100ns(void) {
    LARGE_INTEGER t;
    NtQuerySystemTime(&t);
    return (u64)t.QuadPart;
}

static u64 mono_ns(void) {
    LARGE_INTEGER c, f;
    NtQueryPerformanceCounter(&c, &f);
    return (u64)c.QuadPart / (u64)f.QuadPart * 1000000000ULL + ((u64)c.QuadPart % (u64)f.QuadPart) * 1000000000ULL / (u64)f.QuadPart;
}

static int signal_ready(void) { T *t = CUR; return ((me->pending | t->pending) & ~t->blocked) != 0; }

static void timers_poll(void) {
    u64 now = 0;
    for (int i = 0; i < MAX_TIMERS; i++) {
        Timer *tm = &g_timers[i];
        if (!tm->used || !tm->armed) continue;
        if (!now) now = mono_ns();
        if (now < tm->deadline_ns) continue;
        if (tm->interval_ns) tm->deadline_ns = now + tm->interval_ns;
        else tm->armed = 0;
        if (!tm->signo) continue;
        T *target = NULL;
        for (int j = 0; j < MAX_THREADS && tm->tid; j++)
            if (g_threads[j].used && g_threads[j].tid == tm->tid) target = &g_threads[j];
        lock_();
        if (target) {
            target->pending |= 1ULL << (tm->signo - 1);
            target->timer_fired |= 1ULL << (tm->signo - 1);
        } else {
            me->pending |= 1ULL << (tm->signo - 1);
        }
        unlock_();
    }
}

static u64 timers_next_ns(void) {
    u64 best = 0;
    for (int i = 0; i < MAX_TIMERS; i++) {
        Timer *tm = &g_timers[i];
        if (tm->used && tm->armed && (!best || tm->deadline_ns < best)) best = tm->deadline_ns;
    }
    return best;
}

#define PG 4096u
static u8 *g_arena;
static u32 g_arena_pages;
static u8 *g_page_used;
static u32 g_alloc_hint;

static void arena_init(void) {
    SIZE_T sizes[] = {0x60000000, 0x50000000, 0x40000000, 0x30000000, 0x20000000};
    for (int i = 0; i < 5; i++) {
        PVOID p = NULL;
        SIZE_T s = sizes[i];
        if (NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE, PAGE_NOACCESS))) {
            g_arena = p;
            g_arena_pages = (u32)(s / PG);
            break;
        }
    }
    PVOID b = NULL;
    SIZE_T s = g_arena_pages;
    NtAllocateVirtualMemory(SELF, &b, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    g_page_used = b;
    g_alloc_hint = g_arena_pages;
}

static u32 g_fixed_lo, g_fixed_hi;

#define LOW_END 0x10000000u
static struct { PVOID base; SIZE_T size; } g_low[32];
static int g_low_n;

static int low_is_clear(void) {
    struct { PVOID base, alloc; ULONG aprot; SIZE_T size; ULONG state, prot, type; } mi;
    if (!NT_OK(NtQueryVirtualMemory(SELF, (PVOID)0x10000, 0, &mi, sizeof(mi), NULL))) return 0;
    return mi.state == MEM_RESERVE && (u32)(ULONG_PTR)mi.alloc == 0x10000 && (u32)(ULONG_PTR)mi.base + (u32)mi.size >= LOW_END;
}

static void low_reserve(void) {
    if (!g_low_n && low_is_clear()) {
        g_low[0].base = (PVOID)0x10000;
        g_low[0].size = LOW_END - 0x10000;
        g_low_n = 1;
        return;
    }
    for (u32 a = 0x10000; a < LOW_END && g_low_n < 32;) {
        struct { PVOID base, alloc; ULONG aprot; SIZE_T size; ULONG state, prot, type; } mi;
        if (!NT_OK(NtQueryVirtualMemory(SELF, (PVOID)(ULONG_PTR)a, 0, &mi, sizeof(mi), NULL))) break;
        u32 end = (u32)(ULONG_PTR)mi.base + (u32)mi.size;
        if (mi.state == MEM_FREE) {
            u32 lo = (a + 0xFFFF) & ~0xFFFFu, hi = (end < LOW_END ? end : LOW_END) & ~0xFFFFu;
            if (hi > lo) {
                PVOID p = (PVOID)(ULONG_PTR)lo;
                SIZE_T s = hi - lo;
                if (NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE, PAGE_NOACCESS))) {
                    g_low[g_low_n].base = p;
                    g_low[g_low_n++].size = s;
                }
            }
        }
        a = end;
    }
}

static void low_release(void) {
    for (int i = 0; i < g_low_n; i++) {
        SIZE_T s = 0;
        NtFreeVirtualMemory(SELF, &g_low[i].base, &s, MEM_RELEASE);
    }
    g_low_n = 0;
}

static int in_fixed(u32 addr, u32 len) {
    return g_fixed_hi && addr >= g_fixed_lo && addr + len <= g_fixed_hi;
}

static int in_arena(u32 addr, u32 len) {
    return addr >= (u32)(ULONG_PTR)g_arena && addr + len <= (u32)(ULONG_PTR)g_arena + g_arena_pages * PG && addr + len >= addr;
}

static ULONG nt_prot(int prot) {
    int r = prot & 1, w = prot & 2, x = prot & 4;
    if (x) return w ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    if (w) return PAGE_READWRITE;
    if (r) return PAGE_READONLY;
    return PAGE_NOACCESS;
}

static u32 arena_find(u32 pages) {
    for (int attempt = 0; attempt < 2; attempt++) {
        u32 top = attempt ? g_arena_pages : g_alloc_hint, run = 0;
        for (u32 i = top; i > 0; i--) {
            if (g_page_used[i - 1]) {
                run = 0;
                continue;
            }
            if (++run == pages) {
                g_alloc_hint = i - 1;
                return i - 1;
            }
        }
    }
    return 0xFFFFFFFF;
}

static void mem_decommit(u32 first, u32 pages) {
    u32 i = first, end = first + pages;
    while (i < end) {
        if (!g_page_used[i]) { i++; continue; }
        u32 j = i;
        while (j < end && g_page_used[j]) j++;
        PVOID p = g_arena + (SIZE_T)i * PG;
        SIZE_T s = (SIZE_T)(j - i) * PG;
        NtFreeVirtualMemory(SELF, &p, &s, MEM_DECOMMIT);
        memset(g_page_used + i, 0, j - i);
        i = j;
    }
}

static int mem_commit(u32 first, u32 pages) {
    mem_decommit(first, pages);
    PVOID p = g_arena + (SIZE_T)first * PG;
    SIZE_T s = (SIZE_T)pages * PG;
    NTSTATUS st = NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_COMMIT, PAGE_READWRITE);
    if (!NT_OK(st)) {
        trace("commit fail %08x pages=%u\n", st, pages);
        return -ENOMEM;
    }
    memset(g_page_used + first, 1, pages);
    return 0;
}

static void mem_reserve_only(u32 first, u32 pages) {
    mem_decommit(first, pages);
    memset(g_page_used + first, 2, pages);
}

static int mem_commit_lazy(u32 first, u32 pages) {
    for (u32 i = first, end = first + pages; i < end;) {
        if (g_page_used[i] != 2) { i++; continue; }
        u32 j = i;
        while (j < end && g_page_used[j] == 2) j++;
        PVOID p = g_arena + (SIZE_T)i * PG;
        SIZE_T s = (SIZE_T)(j - i) * PG;
        if (!NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_COMMIT, PAGE_READWRITE))) return -ENOMEM;
        memset(g_page_used + i, 1, j - i);
        i = j;
    }
    return 0;
}

static void mem_protect(void *addr, SIZE_T len, int prot) {
    PVOID p = addr;
    SIZE_T s = len;
    ULONG old;
    NtProtectVirtualMemory(SELF, &p, &s, nt_prot(prot), &old);
    if (prot & 4) NtFlushInstructionCache(SELF, addr, len);
}

static u8 *mem_alloc_any(u32 bytes, u32 *first_out) {
    u32 pages = (bytes + PG - 1) / PG;
    u32 f = arena_find(pages);
    if (f == 0xFFFFFFFF || mem_commit(f, pages)) return NULL;
    if (first_out) *first_out = f;
    return g_arena + (SIZE_T)f * PG;
}

static void esc_name(const char *in, wchar_t **pw, int keep_caps) {
    wchar_t *w = *pw;
    const u8 *s = (const u8 *)in;
    while (*s) {
        u32 c = *s++;
        if (c >= 0xC0 && c < 0xE0 && *s) c = ((c & 0x1F) << 6) | (*s++ & 0x3F);
        else if (c >= 0xE0 && c < 0xF0 && s[0] && s[1]) { c = ((c & 0xF) << 12) | ((s[0] & 0x3F) << 6) | (s[1] & 0x3F); s += 2; }
        else if (c >= 0x80) c = 0xF000 + c;
        if ((!keep_caps && c >= 'A' && c <= 'Z') || c < 0x20 || c == '"' || c == '*' || c == ':' || c == '<' || c == '>' || c == '?' || c == '\\' || c == '|')
            c += 0xF000;
        *w++ = (wchar_t)c;
    }
    *pw = w;
}

static int unesc_name(const wchar_t *w, int wlen, char *out) {
    int n = 0;
    for (int i = 0; i < wlen; i++) {
        u32 c = w[i];
        if (c >= 0xF080 && c < 0xF100) { out[n++] = (char)(c - 0xF000); continue; }
        if (c >= 0xF000 && c < 0xF100) c -= 0xF000;
        if (c < 0x80) out[n++] = (char)c;
        else if (c < 0x800) { out[n++] = (char)(0xC0 | (c >> 6)); out[n++] = (char)(0x80 | (c & 0x3F)); }
        else { out[n++] = (char)(0xE0 | (c >> 12)); out[n++] = (char)(0x80 | ((c >> 6) & 0x3F)); out[n++] = (char)(0x80 | (c & 0x3F)); }
    }
    out[n] = 0;
    return n;
}

static int nt_path(const char *abs, wchar_t *w) {
    wchar_t *start = w;
    const char *p = abs;
    int home = !strncmp(abs, "/root", 5) && (!abs[5] || abs[5] == '/');
    if (home) {
        w += swprintf(w, 340, L"\\??\\%s", HOME_DIR);
        p = abs + 5;
    } else {
        w += swprintf(w, 340, L"\\??\\%s\\root", g_root_dos);
    }
    char comp[256];
    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;
        int n = 0;
        while (*p && *p != '/' && n < 255) comp[n++] = *p++;
        comp[n] = 0;
        *w++ = L'\\';
        esc_name(comp, &w, home);
    }
    *w = 0;
    return (int)(w - start) * 2;
}

static void init_oa(OBJECT_ATTRIBUTES *oa, UNICODE_STRING *us, wchar_t *w, int bytes, ULONG attr) {
    us->Buffer = w;
    us->Length = us->MaximumLength = (USHORT)bytes;
    memset(oa, 0, sizeof(*oa));
    oa->Length = sizeof(*oa);
    oa->ObjectName = us;
    oa->Attributes = attr;
}

static int errno_of(NTSTATUS st) {
    switch ((u32)st) {
        case 0xC0000034: case 0xC000003A: case 0xC000000F: return ENOENT;
        case 0xC0000035: return EEXIST;
        case 0xC0000022: case 0xC0000121: return EACCES;
        case 0xC0000043: return EBUSY;
        case 0xC0000103: return ENOTDIR;
        case 0xC00000BA: return EISDIR;
        case 0xC0000101: return ENOTEMPTY;
        case 0xC0000017: case 0xC000012D: return ENOMEM;
        case 0xC000011F: return EMFILE;
        case 0xC000007F: return 28;
    }
    trace("unmapped status %08x\n", st);
    return EIO;
}

typedef struct { LARGE_INTEGER c, a, w, ch; ULONG attr; ULONG pad; } FILE_BASIC;
typedef struct { LARGE_INTEGER alloc, eof; ULONG links; BOOLEAN del, dir; } FILE_STD;
typedef struct { LARGE_INTEGER c, a, w, ch, alloc, eof; ULONG attr; } FILE_NET;

static ULONG g_attr_scratch;

static ULONG path_attr(const char *abs) {
    static wchar_t w[PATH_MAX_];
    UNICODE_STRING us;
    OBJECT_ATTRIBUTES oa;
    FILE_BASIC b;
    int n = nt_path(abs, w);
    init_oa(&oa, &us, w, n, 0);
    NTSTATUS st = NtQueryAttributesFile(&oa, &b);
    g_attr_scratch = st;
    return NT_OK(st) ? b.attr : INVALID_FILE_ATTRIBUTES;
}

static NTSTATUS nt_open(const char *abs, ACCESS_MASK access, ULONG disp, ULONG opts, ULONG attrs, HANDLE *h) {
    static wchar_t w[PATH_MAX_];
    UNICODE_STRING us;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK io;
    int n = nt_path(abs, w);
    init_oa(&oa, &us, w, n, OBJ_INHERIT_);
    return NtCreateFile(h, access | SYNCHRONIZE, &oa, &io, NULL, attrs, SHARE_ALL, disp, opts | FO_SYNC | FO_BACKUP, NULL, 0);
}

static int read_link_abs(const char *abs, char *out, int max) {
    HANDLE h;
    IO_STATUS_BLOCK io;
    NTSTATUS st = nt_open(abs, GENERIC_READ, FILE_OPEN_, FO_NONDIR, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    st = NtReadFile(h, NULL, NULL, NULL, &io, out, max - 1, NULL, NULL);
    NtClose(h);
    if (!NT_OK(st)) return -EIO;
    out[io.Information] = 0;
    return (int)io.Information;
}

enum { F_NONE, F_FILE, F_DIR, F_PIPE_R, F_PIPE_W, F_TTY, F_NULL, F_ZERO, F_RANDOM, F_SOCK, F_SPAIR, F_MEM, F_EPOLL, F_PTM, F_UNIX, F_NETLINK, F_INOTIFY };

typedef struct {
    u8 type, cloexec;
    u8 restart;
    int flags;
    int idx;
    int idx2;
    HANDLE h;
    u8 *mem;
    int memlen, pos;
} Fd;

static Fd g_fd[MAX_FD];
static char g_fdpath[MAX_FD][256];
static char g_cwd[PATH_MAX_] = "/";
static u32 g_umask = 022;
static char g_exe[PATH_MAX_];

static int resolve(int dirfd, const char *in, int follow, char *out) {
    static char pend[PATH_MAX_ * 2], link[PATH_MAX_], tmp[PATH_MAX_ * 2];
    int loops = 0;
    if (!in[0]) return -ENOENT;
    if (strlen(in) >= PATH_MAX_) return -36;
    strcpy(pend, in);
    if (in[0] == '/') out[0] = 0;
    else if (dirfd == AT_FDCWD_) strcpy(out, g_cwd[1] ? g_cwd : "");
    else {
        if (dirfd < 0 || dirfd >= MAX_FD || g_fd[dirfd].type != F_DIR) return dirfd >= 0 && dirfd < MAX_FD && g_fd[dirfd].type ? -ENOTDIR : -EBADF;
        strcpy(out, g_fdpath[dirfd][1] ? g_fdpath[dirfd] : "");
    }
    char *p = pend;
    if (!strncmp(out, "/proc", 5) && (out[5] == 0 || out[5] == '/')) {

        strcat(out, "/");
        strcat(out, pend);
        size_t ln = strlen(out);
        while (ln > 5 && out[ln - 1] == '/') out[--ln] = 0;
        return 0;
    }
    for (;;) {
        while (*p == '/') p++;
        if (!*p) break;
        char *e = p;
        while (*e && *e != '/') e++;
        int len = (int)(e - p);
        int last = 1;
        for (char *q = e; *q; q++) if (*q != '/') { last = 0; break; }
        if (len == 1 && p[0] == '.') { p = e; continue; }
        if (len == 2 && p[0] == '.' && p[1] == '.') {
            char *s = strrchr(out, '/');
            if (s) *s = 0;
            p = e;
            continue;
        }
        size_t ol = strlen(out);
        if (ol + len + 2 >= PATH_MAX_) return -36;
        out[ol] = '/';
        memcpy(out + ol + 1, p, len);
        out[ol + 1 + len] = 0;
        if (!strcmp(out, "/dev/pts")) {
            strcat(out, e);
            return 0;
        }
        if (!strcmp(out, "/proc")) {

            strcat(out, e);
            size_t ln = strlen(out);
            while (ln > 5 && out[ln - 1] == '/') out[--ln] = 0;
            return 0;
        }
        if (!last || follow) {
            ULONG a = path_attr(out);
            if (a == INVALID_FILE_ATTRIBUTES) {
                if (!last) return -ENOENT;
            } else if ((a & FILE_ATTRIBUTE_SYSTEM) && !(a & FILE_ATTRIBUTE_DIRECTORY)) {
                if (++loops > 32) return -ELOOP;
                int n = read_link_abs(out, link, sizeof(link));
                if (n < 0) return n;
                strcpy(tmp, link);
                strcat(tmp, "/");
                strcat(tmp, e);
                strcpy(pend, tmp);
                p = pend;
                if (link[0] == '/') out[0] = 0;
                else out[ol] = 0;
                continue;
            } else if (!last && !(a & FILE_ATTRIBUTE_DIRECTORY)) {
                return -ENOTDIR;
            }
        }
        p = e;
    }
    if (!out[0]) strcpy(out, "/");
    return 0;
}

static int fd_alloc(int from) {
    for (int i = from; i < MAX_FD; i++)
        if (g_fd[i].type == F_NONE) return i;
    return -EMFILE;
}

static void ref_count_add(Fd *f) {
    switch (f->type) {
        case F_PIPE_R: g->pipes[f->idx].readers++; break;
        case F_PIPE_W: g->pipes[f->idx].writers++; break;
        case F_SOCK: g->socks[f->idx].refs++; break;
        case F_SPAIR: g->pipes[f->idx].readers++; g->pipes[f->idx2].writers++; break;
        case F_TTY: if (f->idx) g->ttys[f->idx].slave_refs++; break;
        case F_PTM: g->ttys[f->idx].master_refs++; break;
        case F_UNIX: g->unixs[f->idx].refs++; break;
    }
}

static void ref_mirror_add(Fd *f) {
    switch (f->type) {
        case F_PIPE_R: me->rrefs[f->idx]++; break;
        case F_PIPE_W: me->wrefs[f->idx]++; break;
        case F_SOCK: me->srefs[f->idx]++; break;
        case F_SPAIR: me->rrefs[f->idx]++; me->wrefs[f->idx2]++; break;
        case F_TTY: if (f->idx) me->trefs[f->idx]++; break;
        case F_PTM: me->mrefs[f->idx]++; break;
        case F_UNIX: me->urefs[f->idx]++; break;
    }
}

static void pipe_ref_add(Fd *f) {
    ref_count_add(f);
    ref_mirror_add(f);
}

static void g_ep_release(int i);
static void tty_master_close_locked(int i, Proc *owner);
static void unix_free_locked(Unix *u);
static int udg_send(int fd, const u8 *buf, u32 len, const u8 *addr, u32 alen, int flags);
static int udg_recv(int fd, u8 *buf, u32 len, int flags, u8 *from, u32 *fromlen);
static int nl_send(const u8 *buf, u32 len);
static int nl_recv(int fd, u8 *buf, u32 len, int flags, u8 *from, u32 *fromlen);
static int nl_ready(void);
static int ino_read(int fd, u8 *buf, u32 len);
static int ino_ready(int i);
static void ino_close(int i);

static int fd_close(int n) {
    if (n < 0 || n >= MAX_FD || !g_fd[n].type) return -EBADF;
    Fd *f = &g_fd[n];
    if ((f->type == F_FILE || f->type == F_DIR) && f->h) NtClose(f->h);
    if (f->type == F_EPOLL) g_ep_release(f->idx);
    if (f->type == F_TTY && f->idx) {
        lock_();
        Tty *tt = &g->ttys[f->idx];
        tt->slave_refs--;
        me->trefs[f->idx]--;
        if (tt->slave_refs <= 0 && tt->master_refs <= 0) tt->used = 0;
        unlock_();
        wake_all();
    }
    if (f->type == F_PTM) {
        lock_();
        tty_master_close_locked(f->idx, me);
        unlock_();
        wake_all();
    }
    if (f->type == F_UNIX) {
        lock_();
        Unix *uu = &g->unixs[f->idx];
        uu->refs--;
        me->urefs[f->idx]--;
        if (uu->refs <= 0) unix_free_locked(uu);
        unlock_();
    }
    if (f->type == F_INOTIFY) ino_close(f->idx);
    if (f->type == F_MEM) {
        PVOID p = f->mem;
        SIZE_T s = 0;
        NtFreeVirtualMemory(SELF, &p, &s, MEM_RELEASE);
    }
    if (f->type == F_PIPE_R || f->type == F_PIPE_W) {
        lock_();
        if (f->type == F_PIPE_R) { g->pipes[f->idx].readers--; me->rrefs[f->idx]--; }
        else { g->pipes[f->idx].writers--; me->wrefs[f->idx]--; }
        unlock_();
        wake_all();
    }
    if (f->type == F_SPAIR) {
        lock_();
        g->pipes[f->idx].readers--; me->rrefs[f->idx]--;
        g->pipes[f->idx2].writers--; me->wrefs[f->idx2]--;
        unlock_();
        wake_all();
    }
    if (f->type == F_SOCK) {
        lock_();
        g->socks[f->idx].refs--;
        me->srefs[f->idx]--;
        unlock_();
        NtSetEvent(g_events[MAX_PROC - 1], NULL);
    }
    memset(f, 0, sizeof(*f));
    return 0;
}

static int fd_dup_to(int from, int to, int cloexec) {
    Fd nf = g_fd[from];
    if (nf.type == F_MEM) {
        PVOID p = NULL;
        SIZE_T s = nf.memlen ? nf.memlen : 1;
        if (!NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))) return -ENOMEM;
        memcpy(p, nf.mem, nf.memlen);
        nf.mem = p;
    }
    if ((nf.type == F_FILE || nf.type == F_DIR) && nf.h) {
        NTSTATUS st = NtDuplicateObject(SELF, g_fd[from].h, SELF, &nf.h, 0, OBJ_INHERIT_, DUPLICATE_SAME_ACCESS);
        if (!NT_OK(st)) return -errno_of(st);
    }
    if (g_fd[to].type) fd_close(to);
    nf.cloexec = (u8)cloexec;
    g_fd[to] = nf;
    strcpy(g_fdpath[to], g_fdpath[from]);
    lock_();
    pipe_ref_add(&g_fd[to]);
    unlock_();
    return to;
}

typedef struct { u32 mode, uid, gid; } TxMeta;

static void meta_set(HANDLE h, const TxMeta *m) {
    char buf[40];
    IO_STATUS_BLOCK io;
    ULONG *next = (ULONG *)buf;
    memset(buf, 0, sizeof(buf));
    *next = 0;
    buf[5] = 6;
    *(u16 *)(buf + 6) = sizeof(*m);
    memcpy(buf + 8, "TXMETA", 7);
    memcpy(buf + 15, m, sizeof(*m));
    NtSetEaFile(h, &io, buf, 8 + 7 + sizeof(*m));
}

static int meta_get(HANDLE h, TxMeta *m) {
    char buf[64];
    IO_STATUS_BLOCK io;
    if (!NT_OK(NtQueryEaFile(h, &io, buf, sizeof(buf), TRUE, NULL, 0, NULL, TRUE))) return 0;
    if (buf[5] != 6 || *(u16 *)(buf + 6) != sizeof(*m)) return 0;
    memcpy(m, buf + 15, sizeof(*m));
    return 1;
}

#pragma pack(push, 4)
typedef struct {
    u64 st_dev; int pad0; long ino32; u32 st_mode, st_nlink, st_uid, st_gid;
    u64 st_rdev; int pad1; int pad1b;
    i64 st_size; long st_blksize; int pad2; i64 st_blocks;
    long at, atn, mt, mtn, ct, ctn;
    u64 st_ino;
} kstat64;
#pragma pack(pop)

static void nt_time(i64 t, long *sec, long *nsec) {
    i64 u = t - 116444736000000000LL;
    *sec = (long)(u / 10000000);
    *nsec = (long)((u % 10000000) * 100);
}

static void stat_chr(kstat64 *s, u32 maj, u32 min, u64 ino) {
    memset(s, 0, sizeof(*s));
    s->st_mode = S_IFCHR_ | 0666;
    s->st_nlink = 1;
    s->st_rdev = ((u64)maj << 8) | min;
    s->st_ino = ino;
    s->ino32 = (long)ino;
    s->st_blksize = 4096;
    s->st_dev = 5;
}

static int stat_handle(HANDLE h, kstat64 *s) {
    IO_STATUS_BLOCK io;
    FILE_BASIC b;
    FILE_STD std;
    LARGE_INTEGER id;
    NTSTATUS st = NtQueryInformationFile(h, &io, &b, sizeof(b), 4);
    if (!NT_OK(st)) return -errno_of(st);
    st = NtQueryInformationFile(h, &io, &std, sizeof(std), 5);
    if (!NT_OK(st)) return -errno_of(st);
    NtQueryInformationFile(h, &io, &id, sizeof(id), 6);
    memset(s, 0, sizeof(*s));
    TxMeta m;
    if (meta_get(h, &m)) {
        s->st_mode = m.mode;
        s->st_uid = m.uid;
        s->st_gid = m.gid;
    } else {
        s->st_mode = (b.attr & FILE_ATTRIBUTE_DIRECTORY) ? (S_IFDIR_ | 0755) : (b.attr & FILE_ATTRIBUTE_SYSTEM) ? (S_IFLNK_ | 0777) : (S_IFREG_ | 0644);
    }
    s->st_dev = 1;
    s->st_ino = (u64)id.QuadPart;
    s->ino32 = (long)id.LowPart;
    s->st_nlink = std.links;
    s->st_size = std.eof.QuadPart;
    s->st_blksize = 4096;
    s->st_blocks = (std.alloc.QuadPart + 511) / 512;
    nt_time(b.a.QuadPart, &s->at, &s->atn);
    nt_time(b.w.QuadPart, &s->mt, &s->mtn);
    nt_time(b.ch.QuadPart, &s->ct, &s->ctn);
    return 0;
}

#define PROC_FILE_MAX_FWD 16384
static int proc_classify(const char *abs, char *o, int max, int *len);
static int proc_open(const char *abs, int n, int flags);
static int proc_getdents(Fd *f, const char *abs, u8 *buf, u32 size);

static int tty_index(const char *abs) {
    if (!strcmp(abs, "/dev/tty")) return me->ctty >= 0 ? me->ctty : -6;
    if (!strcmp(abs, "/dev/console")) return 0;
    int n = atoi(abs + 9);
    if (n < 0 || n >= MAX_TTY || (n && !g->ttys[n].used)) return -ENOENT;
    return n;
}

static int proc_self_fd(const char *abs) {
    if (!strncmp(abs, "/proc/self/fd/", 14)) {
        int n = atoi(abs + 14);
        if (n >= 0 && n < MAX_FD && g_fd[n].type) return n;
        return -EBADF;
    }
    return -1;
}

static int dev_kind(const char *abs) {
    if (!strcmp(abs, "/dev/null")) return F_NULL;
    if (!strcmp(abs, "/dev/zero")) return F_ZERO;
    if (!strcmp(abs, "/dev/urandom") || !strcmp(abs, "/dev/random")) return F_RANDOM;
    if (!strcmp(abs, "/dev/tty") || !strcmp(abs, "/dev/console") || !strncmp(abs, "/dev/pts/", 9)) return abs[8] == '/' && !abs[9] ? 0 : F_TTY;
    if (!strcmp(abs, "/dev/ptmx")) return F_PTM;
    return 0;
}

static int stat_fd(int n, kstat64 *s) {
    Fd *f = &g_fd[n];
    switch (f->type) {
        case F_FILE: case F_DIR:
            if (!f->h) {
                memset(s, 0, sizeof(*s));
                s->st_mode = S_IFDIR_ | 0555;
                s->st_nlink = 2;
                s->st_blksize = 4096;
                s->st_ino = 90000 + n;
                s->ino32 = 90000 + n;
                s->st_dev = 8;
                return 0;
            }
            return stat_handle(f->h, s);
        case F_MEM:
            memset(s, 0, sizeof(*s));
            s->st_mode = S_IFREG_ | 0444;
            s->st_nlink = 1;
            s->st_blksize = 4096;
            s->st_size = 0;
            s->st_dev = 8;
            return 0;
        case F_TTY: stat_chr(s, 136, f->idx, 200 + f->idx); return 0;
        case F_PTM: stat_chr(s, 5, 2, 110); return 0;
        case F_UNIX: case F_NETLINK: case F_INOTIFY:
            memset(s, 0, sizeof(*s));
            s->st_mode = 0140000 | 0600;
            s->st_nlink = 1;
            s->st_ino = 4000 + f->idx;
            s->ino32 = 4000 + f->idx;
            s->st_blksize = 4096;
            s->st_dev = 7;
            return 0;
        case F_NULL: stat_chr(s, 1, 3, 101); return 0;
        case F_ZERO: stat_chr(s, 1, 5, 102); return 0;
        case F_RANDOM: stat_chr(s, 1, 9, 103); return 0;
        case F_SOCK:
            memset(s, 0, sizeof(*s));
            s->st_mode = 0140000 | 0600;
            s->st_nlink = 1;
            s->st_ino = 2000 + f->idx;
            s->ino32 = 2000 + f->idx;
            s->st_blksize = 4096;
            s->st_dev = 7;
            return 0;
        case F_SPAIR:
            memset(s, 0, sizeof(*s));
            s->st_mode = 0140000 | 0600;
            s->st_nlink = 1;
            s->st_ino = 3000 + f->idx;
            s->ino32 = 3000 + f->idx;
            s->st_blksize = 4096;
            s->st_dev = 7;
            return 0;
        case F_PIPE_R: case F_PIPE_W:
            memset(s, 0, sizeof(*s));
            s->st_mode = S_IFIFO_ | 0600;
            s->st_nlink = 1;
            s->st_ino = 1000 + f->idx;
            s->ino32 = 1000 + f->idx;
            s->st_blksize = 4096;
            s->st_dev = 6;
            return 0;
    }
    return -EBADF;
}

static int sys_fstatat(int dirfd, const char *path, kstat64 *out, int flags) {
    static char abs[PATH_MAX_];
    kstat64 s;
    if (!path[0] && (flags & AT_EMPTY_PATH)) {
        if (dirfd < 0 || dirfd >= MAX_FD || !g_fd[dirfd].type) return -EBADF;
        int r = stat_fd(dirfd, &s);
        if (!r) *out = s;
        return r;
    }
    int r = resolve(dirfd, path, !(flags & AT_NOFOLLOW), abs);
    if (r) return r;
    int d = proc_self_fd(abs);
    if (d >= 0) {
        r = stat_fd(d, &s);
        if (!r) *out = s;
        return r;
    }
    if (d == -EBADF) return -ENOENT;
    int k = dev_kind(abs);
    if (k) {
        if (k == F_TTY) {
            int ti = tty_index(abs);
            if (ti < 0) return ti == -6 ? -ENOENT : ti;
            stat_chr(&s, 136, ti, 200 + ti);
        } else if (k == F_PTM) {
            stat_chr(&s, 5, 2, 110);
        } else {
            stat_chr(&s, 1, k == F_NULL ? 3 : k == F_ZERO ? 5 : 9, 100 + k);
        }
        *out = s;
        return 0;
    }
    if (!strcmp(abs, "/proc/self/exe")) {
        memset(&s, 0, sizeof(s));
        s.st_mode = S_IFLNK_ | 0777;
        s.st_nlink = 1;
        *out = s;
        return 0;
    }
    if (!strncmp(abs, "/proc", 5) && (abs[5] == 0 || abs[5] == '/')) {
        static char tmp[PROC_FILE_MAX_FWD];
        int len = 0;
        int k = proc_classify(abs, tmp, sizeof(tmp), &len);
        if (k < 0) return k;
        memset(&s, 0, sizeof(s));
        s.st_mode = k == 1 ? (S_IFDIR_ | 0555) : (S_IFREG_ | 0444);
        s.st_nlink = k == 1 ? 2 : 1;
        s.st_blksize = 4096;
        s.st_dev = 8;
        *out = s;
        return 0;
    }
    HANDLE h;
    NTSTATUS st = nt_open(abs, FILE_READ_ATTRIBUTES | FILE_READ_EA, FILE_OPEN_, 0, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    r = stat_handle(h, &s);
    NtClose(h);
    if (!r) *out = s;
    return r;
}

static void tty_setup(Tty *t);
static int fifo_open(const char *abs, int n, int flags);

static void make_tty_fd(Fd *f, int flags) {
    memset(f, 0, sizeof(*f));
    f->type = F_TTY;
    f->flags = flags;
}

static int sys_openat(int dirfd, const char *path, int flags, u32 mode) {
    static char abs[PATH_MAX_];
    int r = resolve(dirfd, path, !(flags & O_NOFOLLOW_), abs);
    if (r) return r;
    int d = proc_self_fd(abs);
    if (d >= 0) {
        int n = fd_alloc(0);
        if (n < 0) return n;
        r = fd_dup_to(d, n, !!(flags & O_CLOEXEC_));
        return r;
    }
    if (d == -EBADF) return -ENOENT;
    int k = dev_kind(abs);
    int n = fd_alloc(0);
    if (n < 0) return n;
    if (k) {
        int ti = 0;
        if (k == F_TTY) {
            ti = tty_index(abs);
            if (ti < 0) return ti;
        } else if (k == F_PTM) {
            lock_();
            for (ti = 1; ti < MAX_TTY; ti++)
                if (!g->ttys[ti].used) break;
            if (ti == MAX_TTY) {
                unlock_();
                return -28;
            }
            memset(&g->ttys[ti], 0, sizeof(Tty));
            tty_setup(&g->ttys[ti]);
            g->ttys[ti].used = 1;
            unlock_();
        }
        make_tty_fd(&g_fd[n], flags);
        g_fd[n].type = (u8)k;
        g_fd[n].idx = ti;
        g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
        if (k == F_TTY) {
            _snprintf(g_fdpath[n], 255, "/dev/pts/%d", ti);
            lock_();
            if (ti) g->ttys[ti].slave_opened = 1;
            if (me->sid == me->pid && me->ctty < 0 && !(flags & 0x100)) {

                me->ctty = ti;
                g->ttys[ti].sid = me->sid;
                g->ttys[ti].fg_pgrp = me->pgid;
            }
            pipe_ref_add(&g_fd[n]);
            unlock_();
        } else {
            strcpy(g_fdpath[n], abs);
            lock_();
            pipe_ref_add(&g_fd[n]);
            unlock_();
        }
        return n;
    }
    if (!strncmp(abs, "/proc", 5) && (abs[5] == 0 || abs[5] == '/')) {
        r = proc_open(abs, n, flags);
        if (g_trace) trace("[%d] proc_open %s -> %d\n", me->pid, abs, r);
        return r;
    }

    int acc = flags & 3;
    ACCESS_MASK am = FILE_READ_ATTRIBUTES | FILE_READ_EA;
    if (acc != O_WRONLY_) am |= FILE_READ_DATA | FILE_LIST_DIRECTORY;
    if (acc != 0) am |= FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES;
    ULONG disp = FILE_OPEN_, opts = 0;
    ULONG a = path_attr(abs);
    int existed = a != INVALID_FILE_ATTRIBUTES;
    if ((flags & O_CREAT_) && (flags & O_EXCL_) && existed) return -EEXIST;
    if (existed && (a & FILE_ATTRIBUTE_HIDDEN) && !(a & FILE_ATTRIBUTE_DIRECTORY)) return fifo_open(abs, n, flags);
    if (!existed && !(flags & O_CREAT_)) return -ENOENT;
    if (existed && (a & FILE_ATTRIBUTE_DIRECTORY)) {
        if (acc != 0) return -EISDIR;
        opts = FO_DIR;
    } else {
        if (flags & O_DIRECTORY_) return existed ? -ENOTDIR : -ENOENT;
        if (existed && (a & FILE_ATTRIBUTE_SYSTEM) && (flags & O_NOFOLLOW_)) return -ELOOP;
        opts = FO_NONDIR;
        if (flags & O_CREAT_) disp = existed ? FILE_OPEN_ : FILE_CREATE_;
        if ((flags & O_CREAT_)) am |= FILE_WRITE_EA;
    }
    HANDLE h;
    NTSTATUS st = nt_open(abs, am, disp, opts, FILE_ATTRIBUTE_NORMAL, &h);
    if (!NT_OK(st)) return -errno_of(st);
    if (!existed) {
        TxMeta m = {S_IFREG_ | (mode & ~g_umask & 07777), 0, 0};
        meta_set(h, &m);
    }
    if ((flags & O_TRUNC_) && existed && acc != 0) {
        IO_STATUS_BLOCK io;
        LARGE_INTEGER z;
        z.QuadPart = 0;
        NtSetInformationFile(h, &io, &z, sizeof(z), 20);
    }
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = (opts & FO_DIR) ? F_DIR : F_FILE;
    g_fd[n].flags = flags;
    g_fd[n].h = h;
    g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
    strncpy(g_fdpath[n], abs, 255);
    g_fdpath[n][255] = 0;
    return n;
}

static LONG g_trash_seq;

static NTSTATUS rename_handle(HANDLE h, const wchar_t *target, int bytes, ULONG cls, ULONG flags) {
    static char buf[16 + PATH_MAX_ * 2];
    struct R { ULONG Flags; HANDLE Root; ULONG Len; WCHAR Name[1]; } *r = (struct R *)buf;
    IO_STATUS_BLOCK io;
    memset(buf, 0, 16);
    r->Flags = flags;
    r->Len = bytes;
    memcpy(r->Name, target, bytes);
    return NtSetInformationFile(h, &io, r, (ULONG)(16 + bytes), cls);
}

static int unlink_abs(const char *abs, int rmdir_) {
    HANDLE h;
    IO_STATUS_BLOCK io;
    ULONG a = path_attr(abs);
    if (a == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    int isdir = !!(a & FILE_ATTRIBUTE_DIRECTORY);
    if (rmdir_ && !isdir) return -ENOTDIR;
    if (!rmdir_ && isdir) return -EISDIR;
    NTSTATUS st = nt_open(abs, DELETE, FILE_OPEN_, isdir ? FO_DIR : FO_NONDIR, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    if (!isdir) {

        static wchar_t t[400];
        int n = swprintf(t, 400, L"\\??\\%s\\trash\\%d-%ld", g_root_dos, me ? me->pid : 0, InterlockedIncrement(&g_trash_seq));
        rename_handle(h, t, n * 2, 65, 3);
    }
    BOOLEAN del = TRUE;
    st = NtSetInformationFile(h, &io, &del, sizeof(del), 13);
    NtClose(h);
    return NT_OK(st) ? 0 : -errno_of(st);
}

static int sys_rename(int od, const char *o, int nd, const char *n) {
    static char a[PATH_MAX_], b[PATH_MAX_];
    static wchar_t w[PATH_MAX_];
    int r = resolve(od, o, 0, a);
    if (r) return r;
    r = resolve(nd, n, 0, b);
    if (r) return r;
    ULONG attr = path_attr(a);
    if (attr == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    HANDLE h;
    NTSTATUS st = nt_open(a, DELETE, FILE_OPEN_, (attr & FILE_ATTRIBUTE_DIRECTORY) ? FO_DIR : FO_NONDIR, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    int bytes = nt_path(b, w);
    st = rename_handle(h, w, bytes, 65, 3);
    NtClose(h);
    return NT_OK(st) ? 0 : -errno_of(st);
}

static int mkdir_abs(const char *abs, u32 mode, u32 uid, u32 gid) {
    HANDLE h;
    NTSTATUS st = nt_open(abs, FILE_LIST_DIRECTORY | FILE_WRITE_EA, FILE_CREATE_, FO_DIR, FILE_ATTRIBUTE_NORMAL, &h);
    if (!NT_OK(st)) return -errno_of(st);
    TxMeta m = {S_IFDIR_ | (mode & 07777), uid, gid};
    meta_set(h, &m);
    NtClose(h);
    return 0;
}

static int symlink_abs(const char *target, const char *abs, u32 uid, u32 gid) {
    HANDLE h;
    IO_STATUS_BLOCK io;
    NTSTATUS st = nt_open(abs, GENERIC_WRITE | FILE_WRITE_EA, FILE_CREATE_, FO_NONDIR, FILE_ATTRIBUTE_SYSTEM, &h);
    if (!NT_OK(st)) return -errno_of(st);
    NtWriteFile(h, NULL, NULL, NULL, &io, target, (ULONG)strlen(target), NULL, NULL);
    TxMeta m = {S_IFLNK_ | 0777, uid, gid};
    meta_set(h, &m);
    NtClose(h);
    return 0;
}

static int sys_chmod(int dirfd, const char *path, u32 mode) {
    static char abs[PATH_MAX_];
    int r = resolve(dirfd, path, 1, abs);
    if (r) return r;
    if (proc_self_fd(abs) >= 0) strcpy(abs, g_fdpath[proc_self_fd(abs)]);
    if (!strncmp(abs, "/dev/pts/", 9)) return 0;
    HANDLE h;
    NTSTATUS st = nt_open(abs, FILE_READ_ATTRIBUTES | FILE_READ_EA | FILE_WRITE_EA, FILE_OPEN_, 0, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    kstat64 s;
    stat_handle(h, &s);
    TxMeta m = {(s.st_mode & S_IFMT_) | (mode & 07777), s.st_uid, s.st_gid};
    meta_set(h, &m);
    NtClose(h);
    return 0;
}

static int sys_chown(int dirfd, const char *path, u32 uid, u32 gid, int nofollow) {
    static char abs[PATH_MAX_];
    int r = resolve(dirfd, path, !nofollow, abs);
    if (r) return r;
    if (proc_self_fd(abs) >= 0) strcpy(abs, g_fdpath[proc_self_fd(abs)]);
    if (!strncmp(abs, "/dev/pts/", 9)) return 0;
    HANDLE h;
    NTSTATUS st = nt_open(abs, FILE_READ_ATTRIBUTES | FILE_READ_EA | FILE_WRITE_EA, FILE_OPEN_, 0, 0, &h);
    if (!NT_OK(st)) return -errno_of(st);
    kstat64 s;
    stat_handle(h, &s);
    TxMeta m = {s.st_mode, uid == 0xFFFFFFFF ? s.st_uid : uid, gid == 0xFFFFFFFF ? s.st_gid : gid};
    meta_set(h, &m);
    NtClose(h);
    return 0;
}

static int sys_getdents64(int n, u8 *buf, u32 size) {
    static u8 info[1024];
    static char name[768];
    if (n < 0 || n >= MAX_FD || !g_fd[n].type) return -EBADF;
    Fd *f = &g_fd[n];
    if (f->type != F_DIR) return -ENOTDIR;
    if (!f->h) return proc_getdents(f, g_fdpath[n], buf, size);
    u32 used = 0;
    for (;;) {
        if (size - used < 300) {
            if (!used) return -EINVAL;
            break;
        }
        IO_STATUS_BLOCK io;
        NTSTATUS st = NtQueryDirectoryFile(f->h, NULL, NULL, NULL, &io, info, sizeof(info), 37, TRUE, NULL, f->restart);
        f->restart = 0;
        if (!NT_OK(st)) break;
        ULONG attr = *(ULONG *)(info + 56), nlen = *(ULONG *)(info + 60);
        i64 id = *(i64 *)(info + 96);
        int l = unesc_name((wchar_t *)(info + 104), nlen / 2, name);
        u32 rec = (19 + l + 1 + 7) & ~7u;
        u8 *d = buf + used;
        memcpy(d, &id, 8);
        i64 off = used + rec;
        memcpy(d + 8, &off, 8);
        *(u16 *)(d + 16) = (u16)rec;
        d[18] = (attr & FILE_ATTRIBUTE_DIRECTORY) ? 4 : (attr & FILE_ATTRIBUTE_SYSTEM) ? 10 : 8;
        memcpy(d + 19, name, l + 1);
        used += rec;
    }
    return (int)used;
}

static void tty_out_byte(Tty *t, u8 c) {
    u32 next = (t->ohead + 1) % TTY_OUT_SZ;
    if (next == t->otail) return;
    t->out[t->ohead] = c;
    t->ohead = next;
}

static void tty_in_byte(Tty *t, u8 c) {
    u32 next = (t->ihead + 1) % TTY_IN_SZ;
    if (next == t->itail) return;
    t->in[t->ihead] = c;
    t->ihead = next;
}

static void signal_pgrp_locked(int pgrp, int sig);

static void tty_input_t(Tty *t, u8 c) {
    lock_();
    if ((t->t.c_iflag & ICRNL_) && c == '\r') c = '\n';
    if (t->t.c_lflag & ISIG_) {
        if (c == t->t.c_cc[VINTR_] || c == t->t.c_cc[VQUIT_] || c == t->t.c_cc[VSUSP_]) {
            int sig = c == t->t.c_cc[VINTR_] ? SIGINT_ : c == t->t.c_cc[VSUSP_] ? SIGTSTP_ : SIGQUIT_;
            if (t->t.c_lflag & ECHO_) { tty_out_byte(t, '^'); tty_out_byte(t, (u8)(c + 64)); }
            t->edit_len = 0;
            t->ihead = t->itail = 0;
            t->lines = 0;
            signal_pgrp_locked(t->fg_pgrp, sig);
            unlock_();
            wake_all();
            return;
        }
    }
    if (t->t.c_lflag & ICANON_) {
        if (c == t->t.c_cc[VERASE_] || c == 8) {
            if (t->edit_len > 0) {
                t->edit_len--;
                if (t->t.c_lflag & ECHOE_) { tty_out_byte(t, 8); tty_out_byte(t, ' '); tty_out_byte(t, 8); }
            }
        } else if (c == t->t.c_cc[VKILL_]) {
            while (t->edit_len > 0) {
                t->edit_len--;
                if (t->t.c_lflag & ECHOE_) { tty_out_byte(t, 8); tty_out_byte(t, ' '); tty_out_byte(t, 8); }
            }
        } else if (c == t->t.c_cc[VEOF_]) {
            for (int i = 0; i < t->edit_len; i++) tty_in_byte(t, t->edit[i]);
            if (!t->edit_len) t->eof_pending = 1;
            t->edit_len = 0;
            t->lines++;
        } else if (c == '\n') {
            for (int i = 0; i < t->edit_len; i++) tty_in_byte(t, t->edit[i]);
            tty_in_byte(t, '\n');
            t->edit_len = 0;
            t->lines++;
            if (t->t.c_lflag & ECHO_) { tty_out_byte(t, '\r'); tty_out_byte(t, '\n'); }
        } else if (t->edit_len < (int)sizeof(t->edit)) {
            t->edit[t->edit_len++] = c;
            if (t->t.c_lflag & ECHO_) tty_out_byte(t, c);
        }
    } else {
        tty_in_byte(t, c);
        if (t->t.c_lflag & ECHO_) tty_out_byte(t, c);
    }
    unlock_();
    wake_all();
}

static void tty_input(u8 c) { tty_input_t(&g->ttys[0], c); }

static int tty_readable(Tty *t) {
    if (t->hup) return 1;
    if (t->t.c_lflag & ICANON_) return t->lines > 0 || t->eof_pending;
    return t->ihead != t->itail;
}

static int tty_read(Tty *t, u8 *buf, u32 n, int nonblock) {
    for (;;) {
        lock_();
        if (tty_readable(t)) {
            u32 got = 0;
            if (t->hup && t->itail == t->ihead) {
                unlock_();
                return 0;
            }
            int canon = t->t.c_lflag & ICANON_;
            if (canon && t->eof_pending && t->ihead == t->itail) {
                t->eof_pending = 0;
                t->lines = t->lines > 0 ? t->lines - 1 : 0;
                unlock_();
                return 0;
            }
            while (got < n && t->itail != t->ihead) {
                u8 c = t->in[t->itail];
                t->itail = (t->itail + 1) % TTY_IN_SZ;
                buf[got++] = c;
                if (canon && c == '\n') {
                    t->lines--;
                    break;
                }
            }
            if (canon && t->itail == t->ihead && got && buf[got - 1] != '\n' && t->lines > 0) t->lines--;
            unlock_();
            return (int)got;
        }
        unlock_();
        if (nonblock) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static int tty_write(Tty *t, const u8 *buf, u32 n) {
    u32 done = 0;
    if (t->hup) return -EIO;
    while (done < n) {
        lock_();
        int onlcr = (t->t.c_oflag & (OPOST_ | ONLCR_)) == (OPOST_ | ONLCR_);
        while (done < n) {
            u32 space = (t->otail + TTY_OUT_SZ - t->ohead - 1) % TTY_OUT_SZ;
            if (space < 2) break;
            if (onlcr && buf[done] == '\n') tty_out_byte(t, '\r');
            tty_out_byte(t, buf[done++]);
        }
        unlock_();
        if (t == &g->ttys[0] || t->is_session) {
            NtSetEvent(g_events[MAX_PROC - 1], NULL);
            if (g_ui_event) NtSetEvent(g_ui_event, NULL);
        } else {
            wake_all();
        }
        if (done < n) {
            if (signal_ready()) return done ? (int)done : -ERESTART_;
            wait_event(20);
        }
    }
    return (int)n;
}

static int ptm_read(Tty *t, u8 *buf, u32 n, int nonblock) {
    for (;;) {
        lock_();
        if (t->otail != t->ohead) {
            u32 got = 0;
            while (got < n && t->otail != t->ohead) {
                buf[got++] = t->out[t->otail];
                t->otail = (t->otail + 1) % TTY_OUT_SZ;
            }
            unlock_();
            wake_all();
            return (int)got;
        }
        int gone = t->slave_opened && t->slave_refs <= 0;
        unlock_();
        if (gone) return -EIO;
        if (nonblock) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static int pipe_read(Fd *f, u8 *buf, u32 n) {
    Pipe *p = &g->pipes[f->idx];
    u8 *ring = g->pipebuf[f->idx];
    for (;;) {
        lock_();
        u32 avail = (p->head + PIPE_SZ - p->tail) % PIPE_SZ;
        if (avail) {
            u32 got = avail < n ? avail : n;
            for (u32 i = 0; i < got; i++) buf[i] = ring[(p->tail + i) % PIPE_SZ];
            p->tail = (p->tail + got) % PIPE_SZ;
            unlock_();
            wake_all();
            return (int)got;
        }
        int writers = p->writers;
        unlock_();
        if (!writers) return 0;
        if (f->flags & O_NONBLOCK_) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static void raise_self(int sig);

static int pipe_write(Fd *f, const u8 *buf, u32 n) {
    Pipe *p = &g->pipes[f->idx];
    u8 *ring = g->pipebuf[f->idx];
    u32 done = 0;
    while (done < n) {
        lock_();
        if (!p->readers) {
            unlock_();
            raise_self(SIGPIPE_);
            return done ? (int)done : -EPIPE;
        }
        u32 space = (p->tail + PIPE_SZ - p->head - 1) % PIPE_SZ;
        if (space) {
            u32 put = space < n - done ? space : n - done;
            for (u32 i = 0; i < put; i++) ring[(p->head + i) % PIPE_SZ] = buf[done + i];
            p->head = (p->head + put) % PIPE_SZ;
            done += put;
        }
        unlock_();
        wake_all();
        if (done < n) {
            if (f->flags & O_NONBLOCK_) return done ? (int)done : -EAGAIN;
            if (signal_ready()) return done ? (int)done : -ERESTART_;
            wait_event(100);
        }
    }
    return (int)n;
}

#include "net_guest.inc"

static void random_fill(u8 *b, u32 n) {
    static ULONG seed;
    if (!seed) seed = (ULONG)now_100ns() ^ (ULONG)mono_ns() ^ (ULONG)(me ? me->pid * 2654435761u : 0);
    for (u32 i = 0; i < n; i++) b[i] = (u8)(RtlRandomEx(&seed) >> 7);
}

static int sys_read(int n, u8 *buf, u32 len) {
    if (n < 0 || n >= MAX_FD || !g_fd[n].type) return -EBADF;
    Fd *f = &g_fd[n];
    IO_STATUS_BLOCK io;
    switch (f->type) {
        case F_FILE: {
            NTSTATUS st = NtReadFile(f->h, NULL, NULL, NULL, &io, buf, len, NULL, NULL);
            if ((u32)st == 0xC0000011) return 0;
            return NT_OK(st) ? (int)io.Information : -errno_of(st);
        }
        case F_DIR: return -EISDIR;
        case F_MEM: {
            int left = f->memlen - f->pos;
            if (left <= 0) return 0;
            int take = (u32)left < len ? left : (int)len;
            memcpy(buf, f->mem + f->pos, take);
            f->pos += take;
            return take;
        }
        case F_PIPE_R: return pipe_read(f, buf, len);
        case F_PIPE_W: return -EBADF;
        case F_TTY: return tty_read(&g->ttys[f->idx], buf, len, f->flags & O_NONBLOCK_);
        case F_PTM: return ptm_read(&g->ttys[f->idx], buf, len, f->flags & O_NONBLOCK_);
        case F_UNIX: return g->unixs[f->idx].dgram ? udg_recv(n, buf, len, 0, NULL, NULL) : -ENOTCONN_;
        case F_NETLINK: return nl_recv(n, buf, len, 0, NULL, NULL);
        case F_INOTIFY: return ino_read(n, buf, len);
        case F_NULL: return 0;
        case F_ZERO: memset(buf, 0, len); return (int)len;
        case F_RANDOM: random_fill(buf, len); return (int)len;
        case F_SOCK: return sock_recv(n, buf, len, 0, NULL, NULL);
        case F_SPAIR: return pipe_read(f, buf, len);
    }
    return -EBADF;
}

static int sys_write(int n, const u8 *buf, u32 len) {
    if (n < 0 || n >= MAX_FD || !g_fd[n].type) return -EBADF;
    Fd *f = &g_fd[n];
    IO_STATUS_BLOCK io;
    switch (f->type) {
        case F_FILE: {
            LARGE_INTEGER end, *off = NULL;
            if (f->flags & O_APPEND_) {
                end.LowPart = 0xFFFFFFFF;
                end.HighPart = -1;
                off = &end;
            }
            NTSTATUS st = NtWriteFile(f->h, NULL, NULL, NULL, &io, buf, len, off, NULL);
            return NT_OK(st) ? (int)io.Information : -errno_of(st);
        }
        case F_DIR: return -EBADF;
        case F_PIPE_W: return pipe_write(f, buf, len);
        case F_PIPE_R: return -EBADF;
        case F_TTY: return tty_write(&g->ttys[f->idx], buf, len);
        case F_PTM:
            for (u32 i = 0; i < len; i++) tty_input_t(&g->ttys[f->idx], buf[i]);
            return (int)len;
        case F_UNIX: return g->unixs[f->idx].dgram ? udg_send(n, buf, len, NULL, 0, 0) : -EPIPE;
        case F_NETLINK: return nl_send(buf, len);
        case F_NULL: case F_ZERO: case F_RANDOM: return (int)len;
        case F_SOCK: return sock_send(n, buf, len, 0, NULL);
        case F_SPAIR: {
            Fd w = *f;
            w.idx = f->idx2;
            return pipe_write(&w, buf, len);
        }
    }
    return -EBADF;
}

static i64 file_seek(Fd *f, i64 off, int whence, int *err) {
    IO_STATUS_BLOCK io;
    LARGE_INTEGER pos;
    *err = 0;
    if (f->type == F_DIR) {

        if (off == 0 && whence == 0) { f->restart = 1; return 0; }
        return 0;
    }
    if (f->type == F_MEM) {
        i64 np = whence == 0 ? off : whence == 1 ? f->pos + off : f->memlen + off;
        if (np < 0) { *err = EINVAL; return -1; }
        f->pos = (int)np;
        return np;
    }
    if (f->type == F_NULL || f->type == F_ZERO) return 0;
    if (f->type != F_FILE) { *err = ESPIPE; return -1; }
    if (whence == 1) {
        NtQueryInformationFile(f->h, &io, &pos, sizeof(pos), 14);
        off += pos.QuadPart;
    } else if (whence == 2) {
        FILE_STD std;
        NtQueryInformationFile(f->h, &io, &std, sizeof(std), 5);
        off += std.eof.QuadPart;
    } else if (whence != 0) {
        *err = EINVAL;
        return -1;
    }
    if (off < 0) { *err = EINVAL; return -1; }
    pos.QuadPart = off;
    NTSTATUS st = NtSetInformationFile(f->h, &io, &pos, sizeof(pos), 14);
    if (!NT_OK(st)) { *err = errno_of(st); return -1; }
    return off;
}

#define POLLIN_ 1
#define POLLOUT_ 4
#define POLLERR_ 8
#define POLLHUP_ 0x10
#define POLLNVAL_ 0x20

static int fd_revents(int n, int ev) {
    if (n < 0 || n >= MAX_FD || !g_fd[n].type) return POLLNVAL_;
    Fd *f = &g_fd[n];
    int r = 0;
    if (f->type == F_PIPE_R) {
        Pipe *p = &g->pipes[f->idx];
        if (p->head != p->tail) r |= POLLIN_;
        else if (!p->writers) r |= POLLHUP_;
    } else if (f->type == F_PIPE_W) {
        Pipe *p = &g->pipes[f->idx];
        if (!p->readers) r |= POLLERR_;
        else if ((p->tail + PIPE_SZ - p->head - 1) % PIPE_SZ) r |= POLLOUT_;
    } else if (f->type == F_TTY) {
        if (tty_readable(&g->ttys[f->idx])) r |= POLLIN_;
        r |= POLLOUT_;
    } else if (f->type == F_PTM) {
        Tty *t = &g->ttys[f->idx];
        if (t->otail != t->ohead) r |= POLLIN_;
        else if (t->slave_opened && t->slave_refs <= 0) r |= POLLIN_ | POLLHUP_;
        r |= POLLOUT_;
    } else if (f->type == F_UNIX) {
        Unix *u = &g->unixs[f->idx];
        if (u->nq || u->dhead != u->dtail) r |= POLLIN_;
        if (u->dgram) r |= POLLOUT_;
    } else if (f->type == F_INOTIFY) {
        if (ino_ready(f->idx)) r |= POLLIN_;
    } else if (f->type == F_NETLINK) {
        r |= POLLOUT_;
        if (nl_ready()) r |= POLLIN_;
    } else if (f->type == F_EPOLL) {
        r = 0;
    } else if (f->type == F_SPAIR) {
        Pipe *in = &g->pipes[f->idx], *out = &g->pipes[f->idx2];
        if (in->head != in->tail) r |= POLLIN_;
        else if (!in->writers) r |= POLLIN_ | POLLHUP_;
        if (!out->readers) r |= POLLERR_;
        else if ((out->tail + PIPE_SZ - out->head - 1) % PIPE_SZ) r |= POLLOUT_;
    } else if (f->type == F_SOCK) {
        Sock *s = &g->socks[f->idx];
        if (s->conn == 3) {
            if (s->nq) r |= POLLIN_;
        } else {
            if (ring_used(s->rhead, s->rtail, SOCK_RX) || s->rx_eof || s->err) r |= POLLIN_;
            if (s->type == 2 || s->err || (s->conn == 2 && ring_free(s->thead, s->ttail, SOCK_TX))) r |= POLLOUT_;
            if (s->err) r |= POLLERR_;
        }
    } else {
        r |= POLLIN_ | POLLOUT_;
    }
    return r & (ev | POLLERR_ | POLLHUP_ | POLLNVAL_);
}

typedef struct { int fd; short events, revents; } kpollfd;

static int do_poll(kpollfd *fds, u32 nfds, i64 timeout_100ns) {
    u64 start = mono_ns();
    for (;;) {
        int count = 0;
        lock_();
        for (u32 i = 0; i < nfds; i++) {
            fds[i].revents = fds[i].fd < 0 ? 0 : (short)fd_revents(fds[i].fd, fds[i].events);
            if (fds[i].revents) count++;
        }
        unlock_();
        if (count) return count;
        if (timeout_100ns == 0) return 0;
        i64 left = 50;
        if (timeout_100ns > 0) {
            i64 el = (i64)(mono_ns() - start) / 100;
            if (el >= timeout_100ns) return 0;
            left = (timeout_100ns - el) / 10000 + 1;
            if (left > 50) left = 50;
        }
        if (signal_ready()) return -EINTR;
        wait_event((int)left);
    }
}

static int do_select(int nfds, u32 *r, u32 *w, u32 *e, i64 timeout_100ns) {
    kpollfd pf[MAX_FD];
    int back[MAX_FD];
    u32 cnt = 0;
    if (nfds > MAX_FD) nfds = MAX_FD;
    for (int i = 0; i < nfds; i++) {
        int ev = 0;
        if (r && (r[i / 32] >> (i % 32) & 1)) ev |= POLLIN_;
        if (w && (w[i / 32] >> (i % 32) & 1)) ev |= POLLOUT_;
        if (!ev) continue;
        pf[cnt].fd = i;
        pf[cnt].events = (short)ev;
        back[cnt++] = i;
    }
    int res = do_poll(pf, cnt, timeout_100ns);
    if (res < 0) return res;
    int words = (nfds + 31) / 32;
    if (r) memset(r, 0, words * 4);
    if (w) memset(w, 0, words * 4);
    if (e) memset(e, 0, words * 4);
    int total = 0;
    for (u32 i = 0; i < cnt; i++) {
        int rv = pf[i].revents, f = back[i];
        if ((rv & (POLLIN_ | POLLHUP_ | POLLERR_)) && (pf[i].events & POLLIN_)) { r[f / 32] |= 1u << (f % 32); total++; }
        if ((rv & (POLLOUT_ | POLLERR_)) && (pf[i].events & POLLOUT_)) { w[f / 32] |= 1u << (f % 32); total++; }
    }
    return total;
}

#pragma pack(push, 4)
typedef struct { u32 handler, flags, restorer; u64 mask; } ksigaction;
#pragma pack(pop)
static ksigaction g_sa[65];

static void __cdecl continue_stub(void);
static u8 g_host_stack[131072];

static void interrupt_guest_locked(Proc *p, u64 bit) {
    HANDLE ph = NULL, th = NULL, prev = NULL;
    CLIENT_ID id;
    OBJECT_ATTRIBUTES oa;
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(oa);
    id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
    id.UniqueThread = NULL;
    if (!NT_OK(NtOpenProcess(&ph, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, &oa, &id))) return;

    while (NT_OK(NtGetNextThread(ph, prev, THREAD_ALL_ACCESS, 0, 0, &th))) {
        if (prev) NtClose(prev);
        prev = th;
        struct { LONG status; PVOID teb; CLIENT_ID cid; ULONG_PTR rest[3]; } bi;
        T *rt = NULL;
        u64 rblocked = 0;
        u8 *rtop = NULL;
        if (!NT_OK(NtQueryInformationThread(th, 0, &bi, sizeof(bi), NULL))) continue;
        if (!NT_OK(NtReadVirtualMemory(ph, (u8 *)bi.teb + 0x10, &rt, sizeof(rt), NULL)) || !rt) continue;
        if (!NT_OK(NtReadVirtualMemory(ph, (u8 *)rt + offsetof(T, blocked), &rblocked, sizeof(rblocked), NULL))) continue;
        if (rblocked & bit) continue;
        if (!NT_OK(NtReadVirtualMemory(ph, (u8 *)rt + offsetof(T, host_top), &rtop, sizeof(rtop), NULL))) continue;
        int done = 0;
        if (NT_OK(NtSuspendThread(th, NULL))) {
            static CONTEXT c;
            c.ContextFlags = CONTEXT_FULL;
            u32 ahead[4] = {0, 0, 0, 0};
            int in_veneer = 0;
            if (NT_OK(NtGetContextThread(th, &c)) && !(c.Cpsr & 0x20) && NT_OK(NtReadVirtualMemory(ph, (PVOID)(ULONG_PTR)c.Pc, ahead, sizeof(ahead), NULL)))
                for (int w = 0; w < 4; w++)
                    if (ahead[w] == 0xE7F5D0F0u) in_veneer = 1;
            if (!in_veneer && NT_OK(NtGetContextThread(th, &c)) && in_arena(c.Pc & ~1u, 2)) {
                NtWriteVirtualMemory(ph, (u8 *)rt + offsetof(T, cont_ctx), &c, sizeof(c), NULL);
                c.Sp = (DWORD)(ULONG_PTR)rtop & ~7u;
                c.Pc = (DWORD)(ULONG_PTR)continue_stub | 1u;
                c.Cpsr = (c.Cpsr & ~0x0600FC00u) | 0x20;
                NtSetContextThread(th, &c);
                done = 1;
            }
            NtResumeThread(th, NULL);
        }
        if (done) break;
    }
    if (prev) NtClose(prev);
    NtClose(ph);
}

static void release_refs_locked(Proc *p);

static void proc_finished_locked(Proc *p, int status) {
    if (p->pid == 1) {
        g->exit_status = status;
        g->done = 1;
    }
    for (int i = 1; i < MAX_TTY; i++) {
        Tty *t = &g->ttys[i];
        if (t->is_session && t->leader_pid == p->pid) {
            t->sess_status = status;
            t->sess_done = 1;
            if (g_ui_event) NtSetEvent(g_ui_event, NULL);
        }
    }
    int has_parent = 0;
    for (int i = 0; i < MAX_PROC; i++)
        if (g->procs[i].state == 1 && g->procs[i].pid == p->ppid) has_parent = 1;
    if (!has_parent) p->state = 0;
}

static void reparent_locked(Proc *c) {
    int init = 0;
    c->ppid = 1;
    for (int i = 0; i < MAX_PROC; i++)
        if (g->procs[i].state == 1 && g->procs[i].pid == 1) init = 1;
    if (!init && c->state == 2) c->state = 0;
}

static void set_stopped_locked(Proc *p, int sig) {
    if (p->state != 1 || p->stopped == sig) return;
    int was = p->stopped;
    p->stopped = sig;
    p->stop_reported = 0;
    if (p != me && p->nt_pid && (!was || !sig)) {
        HANDLE h = NULL;
        CLIENT_ID id;
        OBJECT_ATTRIBUTES oa;
        memset(&oa, 0, sizeof(oa));
        oa.Length = sizeof(oa);
        id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
        id.UniqueThread = NULL;
        if (NT_OK(NtOpenProcess(&h, PROCESS_SUSPEND_RESUME, &oa, &id))) {
            if (sig) NtSuspendProcess(h);
            else NtResumeProcess(h);
            NtClose(h);
        }
    }
    for (int i = 0; i < MAX_PROC; i++) {
        Proc *par = &g->procs[i];
        if (sig && par->state == 1 && par->pid == p->ppid) send_signal_locked(par, SIGCHLD_);
    }
    wake_all();
}

static void send_signal_locked(Proc *p, int sig) {
    if (p->state != 1) return;
    u64 b = SIGBIT(sig);
    if (sig == SIGCONT_ && p->stopped) set_stopped_locked(p, 0);
    if (sig == SIGSTOP_ || ((SIG_STOP_DEFAULT & b) && !(p->handled & b) && !(p->ignored & b) && !(p->blocked & b))) {
        set_stopped_locked(p, sig);
        return;
    }
    if (sig != SIGKILL_) {
        if (p->ignored & b) return;
        if (!(p->handled & b) && (SIG_IGNORE_DEFAULT & b)) {
            if (sig != SIGCHLD_ || !(p->handled & b)) return;
        }
    }
    p->pending |= b;
    if (sig == SIGKILL_ || (!(p->handled & b) && !(p->blocked & b))) {
        if (sig == SIGKILL_) p->stopped = 0;

        if (p != me && p->nt_pid) {
            HANDLE h = NULL;
            CLIENT_ID id;
            OBJECT_ATTRIBUTES oa;
            memset(&oa, 0, sizeof(oa));
            oa.Length = sizeof(oa);
            id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
            id.UniqueThread = NULL;
            if (NT_OK(NtOpenProcess(&h, PROCESS_TERMINATE, &oa, &id))) {
                NtTerminateProcess(h, sig);
                NtClose(h);

                release_refs_locked(p);
                p->status = sig;
                p->state = 2;
                for (int i = 0; i < MAX_PROC; i++) {
                    if (g->procs[i].state && g->procs[i].ppid == p->pid) reparent_locked(&g->procs[i]);
                }
                proc_finished_locked(p, sig);

                for (int i = 0; i < MAX_PROC; i++) {
                    if (g->procs[i].state == 1 && g->procs[i].pid == p->ppid) send_signal_locked(&g->procs[i], SIGCHLD_);
                }
                return;
            }
        }
    }
    if (p != me && p->nt_pid && (p->handled & b) && !(p->blocked & b)) interrupt_guest_locked(p, b);
    int slot = (int)(p - g->procs);
    wake_slot(slot);
}

static void signal_pgrp_locked(int pgrp, int sig) {
    for (int i = 0; i < MAX_PROC; i++) {
        Proc *p = &g->procs[i];
        if (p->state == 1 && p->pgid == pgrp) send_signal_locked(p, sig);
    }
}

static void raise_self(int sig) {
    lock_();
    me->pending |= SIGBIT(sig);
    unlock_();
}

static void do_exit(int status);

typedef struct {
    u8 siginfo[128];
    u8 uc[740];
    u64 saved_mask;
    u32 magic;
    u32 pad;
    CONTEXT ctx;
} SigFrame;

static u32 sig_trampoline(void) {
    static u32 tramp;
    if (!tramp) {
        PVOID p = NULL;
        SIZE_T s = PG;
        if (!NT_OK(NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))) return 0;
        ((u32 *)p)[0] = 0xE3A070ADu;
        ((u32 *)p)[1] = 0xE7F5C0F0u;
        NtFlushInstructionCache(SELF, p, 8);
        tramp = (u32)(ULONG_PTR)p;
    }
    return tramp;
}

#define UC_REGS 8
#define UC_MASK 26

static void deliver_signals(CONTEXT *c) {
    T *t = CUR;
    while (me->stopped) wait_event(100);
    for (;;) {
        lock_();
        u64 ready = (me->pending | t->pending) & ~t->blocked;
        if (!ready) {
            unlock_();
            return;
        }
        int sig = 1;
        while (!(ready & SIGBIT(sig))) sig++;

        me->pending &= ~SIGBIT(sig);
        t->pending &= ~SIGBIT(sig);
        unlock_();
        ksigaction *sa = &g_sa[sig];
        if (g_trace) trace("[%d] deliver sig %d handler=%x flags=%x restorer=%x\n", me->pid, sig, sa->handler, sa->flags, sa->restorer);
        if (sa->handler == 1) continue;
        if (sa->handler == 0) {
            if (SIG_IGNORE_DEFAULT & SIGBIT(sig)) continue;
            if ((SIG_STOP_DEFAULT | SIGBIT(SIGSTOP_)) & SIGBIT(sig)) {
                lock_();
                set_stopped_locked(me, sig);
                unlock_();
                while (me->stopped) wait_event(100);
                continue;
            }
            do_exit(sig);
        }
        u32 top = c->Sp;
        if ((sa->flags & 0x08000000u) && t->alt_size && c->Sp - t->alt_sp >= t->alt_size) top = t->alt_sp + t->alt_size;
        u32 sp = (top - sizeof(SigFrame)) & ~7u;
        SigFrame *fr = (SigFrame *)(ULONG_PTR)sp;
        memset(fr->siginfo, 0, sizeof(fr->siginfo) + sizeof(fr->uc));
        *(int *)fr->siginfo = sig;
        fr->saved_mask = t->suspend_active ? t->suspend_old : t->blocked;
        t->suspend_active = 0;
        fr->magic = 0x54585347;
        fr->ctx = *c;
        {
            u32 *si = (u32 *)fr->siginfo, *uc = (u32 *)fr->uc;
            si[2] = t->fault_addr ? 1 : (u32)-6;
            si[3] = t->fault_addr;
            uc[2] = t->alt_sp;
            uc[3] = t->alt_size ? 0 : 2;
            uc[4] = t->alt_size;
            memcpy(uc + UC_REGS, &c->R0, 13 * 4);
            uc[UC_REGS + 13] = c->Sp;
            uc[UC_REGS + 14] = c->Lr;
            uc[UC_REGS + 15] = (c->Cpsr & 0x20) ? c->Pc & ~1u : c->Pc;
            uc[UC_REGS + 16] = c->Cpsr;
            uc[UC_REGS + 17] = t->fault_addr;
            memcpy(uc + UC_MASK, &fr->saved_mask, 8);
            t->fault_addr = 0;
        }
        c->R0 = sig;
        c->R1 = sp;
        c->R2 = sp + 128;
        c->Sp = sp;
        c->Lr = sa->restorer ? sa->restorer : sig_trampoline();
        c->Pc = sa->handler;
        if (sa->handler & 1) c->Cpsr |= 0x20;
        else c->Cpsr &= ~0x20u;
        c->Cpsr &= ~0x0600FC00u;
        lock_();
        t->blocked = fr->saved_mask | sa->mask | ((sa->flags & SA_NODEFER_) ? 0 : SIGBIT(sig));
        t->blocked &= ~(SIGBIT(SIGKILL_) | SIGBIT(SIGSTOP_));
        proc_blocked_update();
        if (sa->flags & SA_RESETHAND_) {
            sa->handler = 0;
            me->handled &= ~SIGBIT(sig);
        }
        unlock_();
        return;
    }
}

static int sys_rt_sigaction(int sig, const ksigaction *act, ksigaction *old) {
    if (sig < 1 || sig > 64 || sig == SIGKILL_ || sig == SIGSTOP_) return -EINVAL;
    if (old) *old = g_sa[sig];
    if (act) {
        g_sa[sig] = *act;
        lock_();
        u64 b = SIGBIT(sig);
        me->handled &= ~b;
        me->ignored &= ~b;
        if (act->handler == 1) { me->ignored |= b; me->pending &= ~b; }
        else if (act->handler != 0) me->handled |= b;
        unlock_();
    }
    return 0;
}

static Proc *proc_by_pid(int pid) {
    for (int i = 0; i < MAX_PROC; i++)
        if (g->procs[i].state && g->procs[i].pid == pid) return &g->procs[i];
    return NULL;
}

#include "procfs.inc"

static void release_refs_locked(Proc *p) {
    for (int i = 0; i < MAX_PIPE; i++) {
        g->pipes[i].readers -= p->rrefs[i];
        g->pipes[i].writers -= p->wrefs[i];
        p->rrefs[i] = p->wrefs[i] = 0;
    }
    for (int i = 0; i < MAX_SOCK; i++) {
        g->socks[i].refs -= p->srefs[i];
        p->srefs[i] = 0;
    }
    for (int i = 1; i < MAX_TTY; i++) {
        if (p->trefs[i]) {
            g->ttys[i].slave_refs -= p->trefs[i];
            p->trefs[i] = 0;
            if (g->ttys[i].slave_refs <= 0 && g->ttys[i].master_refs <= 0) g->ttys[i].used = 0;
        }
        while (p->mrefs[i]) tty_master_close_locked(i, p);
    }
    for (int i = 0; i < MAX_UNIX; i++) {
        if (p->urefs[i]) {
            g->unixs[i].refs -= p->urefs[i];
            p->urefs[i] = 0;
            if (g->unixs[i].refs <= 0) unix_free_locked(&g->unixs[i]);
        }
    }
}

static void tty_master_close_locked(int i, Proc *owner) {
    Tty *t = &g->ttys[i];
    t->master_refs--;
    owner->mrefs[i]--;
    if (t->master_refs > 0) return;
    t->hup = 1;
    if (t->fg_pgrp) signal_pgrp_locked(t->fg_pgrp, 1);
    if (t->sid && t->sid != t->fg_pgrp) signal_pgrp_locked(t->sid, 1);
    if (t->slave_refs <= 0) t->used = 0;
}

static void unix_free_locked(Unix *u) {
    for (int i = 0; i < u->nq; i++) {
        g->pipes[u->qrd[i]].readers--;
        g->pipes[u->qwr[i]].writers--;
    }
    memset(u, 0, sizeof(*u));
}

static void kill_other_threads(void) {
    if (g_nthreads <= 1) return;
    HANDLE th = NULL, prev = NULL;
    lock_();
    while (NT_OK(NtGetNextThread(SELF, prev, THREAD_ALL_ACCESS, 0, 0, &th))) {
        if (prev) NtClose(prev);
        prev = th;
        struct { LONG status; PVOID teb; CLIENT_ID cid; ULONG_PTR rest[3]; } bi;
        if (NT_OK(NtQueryInformationThread(th, 0, &bi, sizeof(bi), NULL)) && bi.teb != NtCurrentTeb()) NtTerminateThread(th, 0);
    }
    unlock_();
    if (prev) NtClose(prev);
    T *self = CUR;
    for (int i = 0; i < MAX_THREADS; i++)
        if (&g_threads[i] != self) g_threads[i].used = 0;
    g_nthreads = 1;
    self->tid = me->pid;
}

static void smap_sync_all(void);

static void do_exit(int status) {
    kill_other_threads();
    smap_sync_all();
    for (int i = 0; i < MAX_FD; i++)
        if (g_fd[i].type) fd_close(i);
    lock_();
    me->status = status;
    int ppid = me->ppid;
    for (int i = 0; i < MAX_PROC; i++)
        if (g->procs[i].state && g->procs[i].ppid == me->pid) reparent_locked(&g->procs[i]);
    release_refs_locked(me);
    me->state = 2;
    Proc *par = proc_by_pid(ppid);
    if (par && par != me) send_signal_locked(par, SIGCHLD_);
    proc_finished_locked(me, status);
    unlock_();
    wake_all();
    NtTerminateProcess(SELF, status);
}

static int slot_alloc_locked(void) {
    for (int i = 0; i < MAX_PROC - 1; i++) {
        if (g->procs[i].state == 0) return i;
    }
    return -1;
}

static HANDLE g_kid_h[64];
static int g_kid_pid[64], g_kids;

static void exit_log_add(HANDLE h, int pid) {
    if (g_kids < 64) { g_kid_h[g_kids] = h; g_kid_pid[g_kids++] = pid; }
    else NtClose(h);
}

static void exit_log_poll(void) {
    for (int i = 0; i < g_kids; i++) {
        DWORD code;
        if (g_kid_h[i] && GetExitCodeProcess(g_kid_h[i], &code) && code != STILL_ACTIVE) {
            trace("[%d] exit code of child %d: %08x\n", me ? me->pid : 0, g_kid_pid[i], code);
            NtClose(g_kid_h[i]);
            g_kid_h[i] = NULL;
        }
    }
}

static int do_fork(CONTEXT *c, u32 flags, u32 stack, u32 *ctid) {
    HANDLE hp = NULL, ht = NULL;
    lock_();
    int s = slot_alloc_locked();
    if (s < 0) {
        unlock_();
        return -EAGAIN;
    }
    Proc *n = &g->procs[s];
    int pid = g->next_pid++;
    memset(n, 0, sizeof(*n));
    n->pid = pid;
    n->ppid = me->pid;
    n->pgid = me->pgid;
    n->sid = me->sid;
    memcpy(n->uid, me->uid, sizeof(n->uid));
    memcpy(n->gid, me->gid, sizeof(n->gid));
    n->blocked = CUR->blocked;
    n->handled = me->handled;
    n->ignored = me->ignored;
    memcpy(n->comm, me->comm, sizeof(n->comm));
    memcpy(n->cmd, me->cmd, sizeof(n->cmd));
    n->cmdlen = me->cmdlen;
    n->state = 1;
    for (int i = 0; i < MAX_PIPE; i++) {
        n->rrefs[i] = me->rrefs[i];
        n->wrefs[i] = me->wrefs[i];
        g->pipes[i].readers += me->rrefs[i];
        g->pipes[i].writers += me->wrefs[i];
    }
    for (int i = 0; i < MAX_SOCK; i++) {
        n->srefs[i] = me->srefs[i];
        g->socks[i].refs += me->srefs[i];
    }
    for (int i = 0; i < MAX_TTY; i++) {
        n->trefs[i] = me->trefs[i];
        n->mrefs[i] = me->mrefs[i];
        g->ttys[i].slave_refs += me->trefs[i];
        g->ttys[i].master_refs += me->mrefs[i];
    }
    for (int i = 0; i < MAX_UNIX; i++) {
        n->urefs[i] = me->urefs[i];
        g->unixs[i].refs += me->urefs[i];
    }
    n->ctty = me->ctty;
    unlock_();

    trace("[%d] do_fork calling kernel, sp=%p slot=%d\n", me->pid, &hp, s);
    NTSTATUS st = nt_fork(&hp, &ht);
    if (st != CLONE_CHILD) trace("[%d] fork returned %08x\n", me->pid, st);
    if (st == CLONE_CHILD) {
        trace("[%d] fork child alive\n", pid);
        g_self = s;
        me = n;
        {

            T *self = CUR;
            for (int i = 0; i < MAX_THREADS; i++)
                if (&g_threads[i] != self) g_threads[i].used = 0;
            g_nthreads = 1;
            self->tid = pid;
            memset(g_timers, 0, sizeof(g_timers));
        }
        if (stack) c->Sp = stack;
        if (ctid) *ctid = pid;
        return 0;
    }
    if (!NT_OK(st)) {
        trace("clone failed %08x\n", st);
        lock_();
        release_refs_locked(n);
        n->state = 0;
        unlock_();
        return -EAGAIN;
    }
    n->nt_pid = GetProcessId(hp);
    NtResumeThread(ht, NULL);
    NtClose(ht);
    if (g_trace) exit_log_add(hp, pid); else NtClose(hp);
    return pid;
}

static int sys_wait4(int pid, int *status, int options) {
    for (;;) {
        if (g_trace) exit_log_poll();
        int any = 0;
        lock_();
        for (int i = 0; i < MAX_PROC; i++) {
            Proc *p = &g->procs[i];
            if (!p->state || p->ppid != me->pid || p == me) continue;
            if (pid > 0 && p->pid != pid) continue;
            if (pid == 0 && p->pgid != me->pgid) continue;
            if (pid < -1 && p->pgid != -pid) continue;
            any = 1;
            if (p->state == 1 && p->stopped && !p->stop_reported && (options & 2)) {
                int rp = p->pid, st = (p->stopped << 8) | 0x7F;
                p->stop_reported = 1;
                unlock_();
                if (status) *status = st;
                return rp;
            }
            if (p->state == 2) {
                int rp = p->pid, st = p->status;
                p->state = 0;
                unlock_();
                if (status) *status = st;
                return rp;
            }
        }
        unlock_();
        if (!any) return -ECHILD;
        if (options & 1) return 0;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

typedef struct { u8 ident[16]; u16 type, machine; u32 version, entry, phoff, shoff, flags; u16 ehsize, phentsize, phnum, shentsize, shnum, shstrndx; } Ehdr;
typedef struct { u32 type, offset, vaddr, paddr, filesz, memsz, flags, align; } Phdr;
typedef struct { u32 name, type, flags, addr, offset, size, link, info, addralign, entsize; } Shdr;

#define TRAP_INSN 0xDE55
#define SYS_TRAP_ARM 0xE7F5C0F0u
#define SYS_TRAP_ARM_SW 0xE7F5D0F0u
#define TRAP_LEN(c) (((c)->Cpsr & 0x20) ? 2u : 4u)

static u8 *g_musl_img;
static u32 g_musl_span, g_musl_stub;
static Phdr g_musl_ph[12];
static int g_musl_phnum;
static u32 g_musl_entry;

static void write_bw(u8 *at, u8 *target) {
    int off = (int)(target - (at + 4));
    u32 S = off < 0, imm10 = (off >> 12) & 0x3FF, imm11 = (off >> 1) & 0x7FF;
    u32 I1 = (off >> 23) & 1, I2 = (off >> 22) & 1;
    u32 J1 = (~(I1 ^ S)) & 1, J2 = (~(I2 ^ S)) & 1;
    *(u16 *)at = (u16)(0xF000 | (S << 10) | imm10);
    *(u16 *)(at + 2) = (u16)(0x9000 | (J1 << 13) | (J2 << 11) | imm11);
}

#define MUSL_SITES 604

static int musl_prepare(void) {
    static wchar_t w[400];
    UNICODE_STRING us;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK io;
    HANDLE h;
    int n = g_musl_dos[0] ? swprintf(w, 400, L"\\??\\%s", g_musl_dos) : swprintf(w, 400, L"\\??\\%s\\musl.so", g_root_dos);
    init_oa(&oa, &us, w, n * 2, 0);
    NTSTATUS st = NtCreateFile(&h, GENERIC_READ | SYNCHRONIZE, &oa, &io, NULL, 0, SHARE_ALL, FILE_OPEN_, FO_SYNC | FO_NONDIR, NULL, 0);
    if (!NT_OK(st)) { printf("tx: can't open musl.so: %08x\n", st); return -1; }
    FILE_STD std;
    NtQueryInformationFile(h, &io, &std, sizeof(std), 5);
    u32 size = std.eof.LowPart;
    PVOID p = NULL;
    SIZE_T s = size;
    NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    u8 *file = p;
    NtReadFile(h, NULL, NULL, NULL, &io, file, size, NULL, NULL);
    NtClose(h);

    Ehdr *eh = (Ehdr *)file;
    Phdr *ph = (Phdr *)(file + eh->phoff);
    u32 span = 0;
    g_musl_phnum = 0;
    for (int i = 0; i < eh->phnum && i < 12; i++) {
        if (ph[i].type == 1) {
            if (ph[i].vaddr + ph[i].memsz > span) span = ph[i].vaddr + ph[i].memsz;
            g_musl_ph[g_musl_phnum++] = ph[i];
        }
    }
    span = (span + PG - 1) & ~(PG - 1);
    g_musl_stub = span;
    g_musl_span = span + 0x1000;
    g_musl_entry = eh->entry;
    p = NULL;
    s = g_musl_span;
    NtAllocateVirtualMemory(SELF, &p, 0, &s, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    g_musl_img = p;
    for (int i = 0; i < g_musl_phnum; i++) memcpy(g_musl_img + g_musl_ph[i].vaddr, file + g_musl_ph[i].offset, g_musl_ph[i].filesz);

    Shdr *sh = (Shdr *)(file + eh->shoff);
    char *names = (char *)file + sh[eh->shstrndx].offset;
    u32 ta = 0, ts = 0;
    for (int i = 0; i < eh->shnum; i++)
        if (!strcmp(names + sh[i].name, ".text")) { ta = sh[i].addr; ts = sh[i].size; }
    u8 *t = g_musl_img + ta;
    u8 *stub = g_musl_img + g_musl_stub;
    int svcs = 0, mrcs = 0;
    u32 o = 0;
    while (o + 4 <= ts) {
        u16 hw = *(u16 *)(t + o);
        if ((hw >> 11) == 0x1D || (hw >> 11) == 0x1E || (hw >> 11) == 0x1F) {
            u16 h2 = *(u16 *)(t + o + 2);
            if (hw == 0xEE1D && (h2 & 0x0FFF) == 0x0F70) {
                u32 rx = h2 >> 12;

                *(u16 *)stub = 0xEE1D;
                *(u16 *)(stub + 2) = (u16)((rx << 12) | 0x0F50);
                *(u16 *)(stub + 4) = (u16)(0xF8D0 | rx);
                *(u16 *)(stub + 6) = (u16)((rx << 12) | 0x14);
                write_bw(stub + 8, t + o + 4);
                write_bw(t + o, stub);
                stub += 12;
                mrcs++;
            }
            o += 4;
        } else {
            if (hw == 0xDF00) {
                *(u16 *)(t + o) = TRAP_INSN;
                svcs++;
            }
            o += 2;
        }
    }
    p = file;
    s = 0;
    NtFreeVirtualMemory(SELF, &p, &s, MEM_RELEASE);
    if (svcs + mrcs != MUSL_SITES) {
        printf("tx: unexpected musl build: %d svc + %d mrc\n", svcs, mrcs);
        return -1;
    }
    return 0;
}

static u8 *musl_map(void) {
    u32 first;
    u8 *base = mem_alloc_any(g_musl_span, &first);
    if (!base) return NULL;
    memcpy(base, g_musl_img, g_musl_span);
    for (int i = 0; i < g_musl_phnum; i++) {
        Phdr *p = &g_musl_ph[i];
        u32 start = p->vaddr & ~(PG - 1), end = (p->vaddr + p->memsz + PG - 1) & ~(PG - 1);
        int prot = (p->flags & 4 ? 1 : 0) | (p->flags & 2 ? 2 : 0) | (p->flags & 1 ? 4 : 0);
        mem_protect(base + start, end - start, prot);
    }
    mem_protect(base + g_musl_stub, 0x1000, 1 | 4);
    return base;
}

#define EXEC_BUF 262144
static char g_exec_buf[EXEC_BUF];
static char *g_exec_argv[4096], *g_exec_envp[4096];
static int g_exec_argc, g_exec_envc;
static char g_exec_path[PATH_MAX_];
static HANDLE g_exec_file;
static CONTEXT g_exec_ctx;

static int copy_strings(char **dst, char *const *src, char **cur, char *end, int max) {
    int n = 0;
    if (!src) { dst[0] = NULL; return 0; }
    for (; src[n]; n++) {
        if (n >= max - 1) return -E2BIG;
        size_t l = strlen(src[n]) + 1;
        if (*cur + l > end) return -E2BIG;
        memcpy(*cur, src[n], l);
        dst[n] = *cur;
        *cur += l;
    }
    dst[n] = NULL;
    return n;
}

static void reset_for_exec(void);
static void __cdecl exec_finish(void);

static int sys_execve(const char *path, char *const *argv, char *const *envp) {
    static char abs[PATH_MAX_], interp_buf[PATH_MAX_];
    char *cur = g_exec_buf, *end = g_exec_buf + EXEC_BUF;
    int r = resolve(AT_FDCWD_, path, 1, abs);
    if (r) return r;
    int ac = copy_strings(g_exec_argv, argv, &cur, end, 4096);
    if (ac < 0) return ac;
    int ec = copy_strings(g_exec_envp, envp, &cur, end, 4096);
    if (ec < 0) return ec;

    for (int depth = 0; depth < 4; depth++) {
        HANDLE h;
        IO_STATUS_BLOCK io;
        u8 hdr[256];
        ULONG a = path_attr(abs);
        if (a == INVALID_FILE_ATTRIBUTES) return -ENOENT;
        if (a & FILE_ATTRIBUTE_DIRECTORY) return -EACCES;
        NTSTATUS st = nt_open(abs, GENERIC_READ, FILE_OPEN_, FO_NONDIR, 0, &h);
        if (!NT_OK(st)) return -errno_of(st);
        kstat64 s;
        stat_handle(h, &s);
        if (!(s.st_mode & 0111)) { NtClose(h); return -EACCES; }
        memset(hdr, 0, sizeof(hdr));
        LARGE_INTEGER zero;
        zero.QuadPart = 0;
        st = NtReadFile(h, NULL, NULL, NULL, &io, hdr, sizeof(hdr) - 1, &zero, NULL);
        if (!NT_OK(st)) { NtClose(h); return -EACCES; }
        if (hdr[0] == '#' && hdr[1] == '!') {
            NtClose(h);
            char *p = (char *)hdr + 2;
            while (*p == ' ' || *p == '\t') p++;
            char *e = p;
            while (*e && *e != '\n' && *e != ' ' && *e != '\t') e++;
            char saved = *e;
            *e = 0;
            strcpy(interp_buf, p);
            char *arg = NULL;
            if (saved == ' ' || saved == '\t') {
                arg = e + 1;
                while (*arg == ' ' || *arg == '\t') arg++;
                char *ae = arg;
                while (*ae && *ae != '\n') ae++;
                while (ae > arg && (ae[-1] == ' ' || ae[-1] == '\t' || ae[-1] == '\r')) ae--;
                *ae = 0;
                if (!*arg) arg = NULL;
            }

            int extra = arg ? 3 : 2;
            if (ac < 1) { ac = 1; }
            if (ac + extra >= 4095) return -E2BIG;
            memmove(g_exec_argv + extra, g_exec_argv + 1, sizeof(char *) * ac);
            size_t l = strlen(interp_buf) + 1;
            if (cur + l + strlen(abs) + 1 + (arg ? strlen(arg) + 1 : 0) > end) return -E2BIG;
            g_exec_argv[0] = cur;
            memcpy(cur, interp_buf, l);
            cur += l;
            int k = 1;
            if (arg) {
                l = strlen(arg) + 1;
                memcpy(cur, arg, l);
                g_exec_argv[k++] = cur;
                cur += l;
            }
            l = strlen(abs) + 1;
            memcpy(cur, abs, l);
            g_exec_argv[k] = cur;
            cur += l;
            ac += extra - 1;
            r = resolve(AT_FDCWD_, interp_buf, 1, abs);
            if (r) return r;
            continue;
        }
        Ehdr *eh = (Ehdr *)hdr;
        if (memcmp(hdr, "\x7f" "ELF", 4) || hdr[4] != 1 || eh->machine != 40 || (eh->type != 3 && eh->type != 2)) {
            NtClose(h);
            return -ENOEXEC_;
        }
        if (!strcmp(abs, "/usr/bin/nmap") && ac >= 1 && ac + 4 < 4095) {

            static char *const timing[] = {"--min-rtt-timeout", "1200ms", "--initial-rtt-timeout", "1500ms"};
            memmove(g_exec_argv + 5, g_exec_argv + 1, sizeof(char *) * ac);
            memcpy(g_exec_argv + 1, timing, sizeof(timing));
            ac += 4;
        }
        g_exec_file = h;
        strcpy(g_exec_path, abs);
        g_exec_argc = ac;
        g_exec_envc = ec;
        RtlCaptureContext(&g_exec_ctx);
        g_exec_ctx.ContextFlags = CONTEXT_FULL;
        g_exec_ctx.Pc = (DWORD)(ULONG_PTR)exec_finish | 1u;
        g_exec_ctx.Cpsr = (g_exec_ctx.Cpsr & ~0x0600FC00u) | 0x20;
        kill_other_threads();
        memset(g_timers, 0, sizeof(g_timers));
        g_exec_ctx.Sp = (DWORD)(ULONG_PTR)CUR->host_top & ~7u;
        NtContinue(&g_exec_ctx, FALSE);
    }
    return -ELOOP;
}

static void smap_sync_all(void);

static void reset_for_exec(void) {
    smap_sync_all();
    for (int i = 0; i < MAX_FD; i++)
        if (g_fd[i].type && g_fd[i].cloexec) fd_close(i);
    lock_();
    me->handled = 0;
    me->pending &= me->ignored ? ~me->ignored : ~0ULL;
    unlock_();
    for (int i = 1; i <= 64; i++)
        if (g_sa[i].handler > 1) g_sa[i].handler = 0;
    CUR->suspend_active = 0;
    CUR->alt_sp = CUR->alt_size = 0;
    PVOID p = g_arena;
    SIZE_T s = (SIZE_T)g_arena_pages * PG;
    NtFreeVirtualMemory(SELF, &p, &s, MEM_DECOMMIT);
    memset(g_page_used, 0, g_arena_pages);
    g_alloc_hint = g_arena_pages;
    if (g_fixed_hi) {
        p = (PVOID)(ULONG_PTR)g_fixed_lo;
        s = 0;
        NtFreeVirtualMemory(SELF, &p, &s, MEM_RELEASE);
        g_fixed_lo = g_fixed_hi = 0;
        low_release();
        low_reserve();
    }
}

static int sets_r7_nearby(const u8 *t, u32 o) {
    for (u32 back = 2; back <= 6 && back <= o; back += 2) {
        u32 at = o - back;
        u16 p1 = *(const u16 *)(t + at);
        if ((p1 >> 8) == 0x27 && back == 2) return 1;
        if ((p1 & 0xFF87) == 0x4687 || ((p1 & 0xFF87) == 0x4607 && back == 2)) return 1;
        if ((p1 >> 8) == 0x4F) {
            u32 lit = ((at + 4) & ~3u) + (p1 & 0xFF) * 4;
            return *(const u32 *)(t + lit) >= 0xF0000 && *(const u32 *)(t + lit) <= 0xF0010;
        }
        if (at >= 2) {
            u16 p2 = *(const u16 *)(t + at - 2);
            if (p2 == 0xF8DF && (p1 >> 12) == 7) {
                u32 lit = ((at - 2 + 4) & ~3u) + (p1 & 0xFFF);
                return *(const u32 *)(t + lit) >= 0xF0000 && *(const u32 *)(t + lit) <= 0xF0010;
            }
            if (((p2 & 0xFBF0) == 0xF2C0 || (p2 & 0xFBF0) == 0xF240 || (p2 & 0xFBEF) == 0xF04F) && (p1 & 0x8F00) == 0x0700) return 1;
        }
        if ((p1 >> 11) != 4) return 0;
    }
    return 0;
}

static int is_arm_flush(const u32 *w, u32 k, u32 count) {
    if (w[k] == 0xEF9F0002u) return 1;
    if (w[k] != 0xEF000000u || !k || (w[k - 1] & 0xFFFFF000u) != 0xE59F7000u) return 0;
    u32 lit = k - 1 + 2 + (w[k - 1] & 0xFFF) / 4;
    return lit < count && w[lit] == 0xF0002;
}

static int is_arm_svc(const u32 *w, u32 k) {
    if (w[k] != 0xEF000000u) return 0;
    for (u32 back = 1; back <= 10 && back <= k; back++) {
        u32 i = w[k - back];
        if ((i & 0x0FFFF000u) == 0x03A07000u || (i & 0x0FF0F000u) == 0x03007000u) return 1;
        if ((i & 0x0FFFFFF0u) == 0x01A07000u || (i & 0x0FF0F000u) == 0x05907000u) return 1;
        if ((i & 0x0E000000u) == 0x0A000000u || (i & 0x0FFFFFF0u) == 0x012FFF10u) return 0;
    }
    return 0;
}

static u32 cp15_barrier_arm(u32 w) {
    if ((w & 0xF0000000u) != 0xE0000000u) return 0;
    switch (w & 0x0FFF0FFFu) {
        case 0x0E070FBAu: return 0xF57FF05Bu;
        case 0x0E070F9Au: return 0xF57FF04Bu;
        case 0x0E070F95u: return 0xF57FF06Fu;
    }
    return 0;
}

#define TLS_TRAP 0xDE54
#define TLS_TRAP_ARM 0xE7F5A0F0u
#define OABI_FLUSH 0xEF9F0002u
#define FLUSH_TRAP_ARM 0xE7F5B0F0u

static void patch_image_tls(HANDLE h, const Ehdr *eh, u8 *base, u8 *stub, u32 stub_room) {
    static Shdr sh[128];
    static char names[4096];
    IO_STATUS_BLOCK io;
    LARGE_INTEGER off;
    if (!eh->shoff || eh->shnum == 0 || eh->shnum > 128 || eh->shstrndx >= eh->shnum) return;
    off.QuadPart = eh->shoff;
    if (!NT_OK(NtReadFile(h, NULL, NULL, NULL, &io, sh, eh->shnum * sizeof(Shdr), &off, NULL))) return;
    off.QuadPart = sh[eh->shstrndx].offset;
    u32 nl = sh[eh->shstrndx].size < sizeof(names) - 1 ? sh[eh->shstrndx].size : sizeof(names) - 1;
    if (!NT_OK(NtReadFile(h, NULL, NULL, NULL, &io, names, nl, &off, NULL))) return;
    names[nl] = 0;
    u8 *stub_end = stub ? stub + stub_room : NULL;
    int veneers = 0, traps = 0, svcs = 0;
    for (int i = 0; i < eh->shnum; i++) {
        if (sh[i].name >= nl || strcmp(names + sh[i].name, ".text")) continue;
        u8 *t = base + sh[i].addr;
        u32 ts = sh[i].size, o = 0;
        while (o + 4 <= ts) {
            u16 hw = *(u16 *)(t + o);
            if ((hw >> 11) == 0x1D || (hw >> 11) == 0x1E || (hw >> 11) == 0x1F) {
                u16 h2 = *(u16 *)(t + o + 2);
                if (hw == 0xEE1D && (h2 & 0x0FFF) == 0x0F70) {
                    u32 rx = h2 >> 12;
                    i64 dist = stub ? (i64)(ULONG_PTR)stub - (i64)(ULONG_PTR)(t + o) : 0x7FFFFFFF;
                    if (stub && stub + 12 <= stub_end && dist < 0xF00000 && dist > -0xF00000) {
                        *(u16 *)stub = 0xEE1D;
                        *(u16 *)(stub + 2) = (u16)((rx << 12) | 0x0F50);
                        *(u16 *)(stub + 4) = (u16)(0xF8D0 | rx);
                        *(u16 *)(stub + 6) = (u16)((rx << 12) | 0x14);
                        write_bw(stub + 8, t + o + 4);
                        write_bw(t + o, stub);
                        stub += 12;
                        veneers++;
                    } else {
                        *(u16 *)(t + o) = TLS_TRAP;
                        traps++;
                    }
                }
                o += 4;
            } else {

                if (hw == 0xDF00 && sets_r7_nearby(t, o)) {
                    *(u16 *)(t + o) = TRAP_INSN;
                    svcs++;
                }
                o += 2;
            }
        }
    }

    int arm = 0, armtraps = 0;
    for (int i = 0; i < eh->shnum; i++) {
        if (sh[i].name >= nl || strcmp(names + sh[i].name, ".text")) continue;
        u32 *w = (u32 *)(base + sh[i].addr);
        for (u32 k = 0; k < sh[i].size / 4; k++) {
            if (is_arm_flush(w, k, sh[i].size / 4)) {
                w[k] = FLUSH_TRAP_ARM;
                svcs++;
                continue;
            }
            if (is_arm_svc(w, k)) {

                i64 dist = stub ? (i64)(ULONG_PTR)stub - (i64)(ULONG_PTR)&w[k] : 0x7FFFFFFF;
                if (stub && stub + 24 <= stub_end && dist < 0x1F00000 && dist > -0x1F00000) {
                    u32 *s = (u32 *)stub;
                    s[0] = 0xEE1DCF50u;
                    s[1] = 0xE59CC010u;
                    s[2] = 0xE58CD000u | (u32)offsetof(T, guest_sp);
                    s[3] = 0xE59CD000u | (u32)offsetof(T, host_top);
                    s[4] = SYS_TRAP_ARM_SW;
                    s[5] = (u32)(ULONG_PTR)&w[k];
                    w[k] = 0xEA000000u | ((u32)(((i64)(ULONG_PTR)s - (i64)(ULONG_PTR)&w[k] - 8) >> 2) & 0xFFFFFF);
                    stub += 24;
                } else {
                    w[k] = SYS_TRAP_ARM;
                }
                svcs++;
                continue;
            }
            if (cp15_barrier_arm(w[k])) {
                w[k] = cp15_barrier_arm(w[k]);
                continue;
            }

            if ((w[k] & 0x0FFF0FFF) == 0x0E1D0F70 && (w[k] >> 28) != 15) {
                u32 rx = (w[k] >> 12) & 15, cond = w[k] & 0xF0000000u;
                i64 dist = stub ? (i64)(ULONG_PTR)stub - (i64)(ULONG_PTR)&w[k] : 0x7FFFFFFF;
                if (stub && stub + 12 <= stub_end && dist < 0x1F00000 && dist > -0x1F00000) {

                    u32 *s = (u32 *)stub;
                    s[0] = 0xEE1D0F50 | (rx << 12);
                    s[1] = 0xE5900014 | (rx << 16) | (rx << 12);
                    s[2] = 0xEA000000 | ((u32)(((i64)(ULONG_PTR)&w[k + 1] - (i64)(ULONG_PTR)&s[2] - 8) >> 2) & 0xFFFFFF);
                    w[k] = cond | 0x0A000000 | ((u32)(((i64)(ULONG_PTR)s - (i64)(ULONG_PTR)&w[k] - 8) >> 2) & 0xFFFFFF);
                    stub += 12;
                    arm++;
                } else {
                    if (cond == 0xE0000000u) w[k] = TLS_TRAP_ARM | rx;
                    armtraps++;
                }
            }
        }
    }
    if (veneers || traps || arm || armtraps || svcs) trace("[%d] TLS patched: Thumb %d veneers %d traps, ARM %d veneers %d traps; raw svc %d\n", me->pid, veneers, traps, arm, armtraps, svcs);
}

static void fatal_exec(const char *why) {
    trace("exec failed: %s\n", why);

    if (g_fd[2].type) {
        char msg[PATH_MAX_ + 128];
        int n = _snprintf(msg, sizeof(msg), "tx: can't run %s: %s\n", g_exec_path, why);
        if (n > 0) sys_write(2, (const u8 *)msg, n);
    }
    do_exit(126 << 8);
}

static void __cdecl exec_finish(void) {
    static Phdr ph[16];
    static u8 hbuf[52];
    IO_STATUS_BLOCK io;
    LARGE_INTEGER off;
    trace("[%d] exec_finish start %s\n", me->pid, g_exec_path);
    reset_for_exec();
    trace("[%d] exec reset done\n", me->pid);
    HANDLE h = g_exec_file;
    off.QuadPart = 0;
    NtReadFile(h, NULL, NULL, NULL, &io, hbuf, sizeof(hbuf), &off, NULL);
    Ehdr *eh = (Ehdr *)hbuf;
    int nph = eh->phnum < 16 ? eh->phnum : 16;
    off.QuadPart = eh->phoff;
    NtReadFile(h, NULL, NULL, NULL, &io, ph, nph * sizeof(Phdr), &off, NULL);

    u32 span = 0;
    int has_interp = 0;
    u32 phdr_vaddr = 0;
    for (int i = 0; i < nph; i++) {
        if (ph[i].type == 1) {
            if (ph[i].vaddr + ph[i].memsz > span) span = ph[i].vaddr + ph[i].memsz;
            if (eh->phoff >= ph[i].offset && eh->phoff < ph[i].offset + ph[i].filesz)
                phdr_vaddr = ph[i].vaddr + (eh->phoff - ph[i].offset);
        }
        if (ph[i].type == 3) has_interp = 1;
        if (ph[i].type == 6) phdr_vaddr = ph[i].vaddr;
    }
    span = (span + PG - 1) & ~(PG - 1);
    u32 first;
    u8 *base, *stub_area = NULL;
    u32 stub_room = 0;
    if (eh->type == 2) {
        u32 lo = 0xFFFFFFFF;
        for (int i = 0; i < nph; i++)
            if (ph[i].type == 1 && ph[i].vaddr < lo) lo = ph[i].vaddr;
        lo &= ~(PG - 1);
        if (lo < 0x10000 || span > LOW_END || span <= lo) fatal_exec("its fixed load address is outside the range Luminal keeps free");
        u32 gap = 0, gap_end = 0;
        for (int i = 0; i < nph; i++) {
            if (ph[i].type != 1) continue;
            u32 e = (ph[i].vaddr + ph[i].memsz + PG - 1) & ~(PG - 1), next = span;
            for (int j = 0; j < nph; j++)
                if (ph[j].type == 1 && (ph[j].vaddr & ~(PG - 1)) >= e && (ph[j].vaddr & ~(PG - 1)) < next) next = ph[j].vaddr & ~(PG - 1);
            if (next > e && next - e > gap_end - gap && e < span) { gap = e; gap_end = next; }
        }
        if (gap_end - gap < 0x2000) {
            gap = span;
            gap_end = span + 0x10000 <= LOW_END ? span + 0x10000 : span;
            span = gap_end;
        }
        if (gap_end - gap > 0x10000) gap_end = gap + 0x10000;
        low_release();
        PVOID want = (PVOID)(ULONG_PTR)lo;
        SIZE_T sz = span - lo;
        NTSTATUS ast = NtAllocateVirtualMemory(SELF, &want, 0, &sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!NT_OK(ast)) {
            trace("fixed image at %08x+%x: %08x\n", lo, span - lo, ast);
            for (u32 a = 0x10000; a < span + 0x10000;) {
                struct { PVOID base, alloc; ULONG aprot; SIZE_T size; ULONG state, prot, type; } mi;
                if (!NT_OK(NtQueryVirtualMemory(SELF, (PVOID)(ULONG_PTR)a, 0, &mi, sizeof(mi), NULL))) break;
                trace("  %08x size %08x state %x prot %x type %x\n", (u32)(ULONG_PTR)mi.base, (u32)mi.size, mi.state, mi.prot, mi.type);
                a = (u32)(ULONG_PTR)mi.base + (u32)mi.size;
            }
            fatal_exec("it needs a fixed low address that is in use here (non-PIE program)");
        }
        g_fixed_lo = lo;
        g_fixed_hi = span;
        low_reserve();
        base = NULL;
        if (gap_end > gap) {
            stub_area = (u8 *)(ULONG_PTR)gap;
            stub_room = gap_end - gap;
        }
    } else {

        stub_room = 0x10000;
        stub_area = mem_alloc_any(span + stub_room, &first);
        if (!stub_area) fatal_exec("no memory for image");
        base = stub_area + stub_room;
    }
    for (int i = 0; i < nph; i++) {
        if (ph[i].type != 1) continue;
        off.QuadPart = ph[i].offset;
        if (ph[i].filesz) {
            NTSTATUS st = NtReadFile(h, NULL, NULL, NULL, &io, base + ph[i].vaddr, ph[i].filesz, &off, NULL);
            if (!NT_OK(st)) fatal_exec("read image");
        }
    }
    patch_image_tls(h, eh, base, stub_area, stub_room);
    for (int i = 0; i < nph; i++) {
        if (ph[i].type != 1) continue;
        u32 start = ph[i].vaddr & ~(PG - 1), end = (ph[i].vaddr + ph[i].memsz + PG - 1) & ~(PG - 1);
        int prot = (ph[i].flags & 4 ? 1 : 0) | (ph[i].flags & 2 ? 2 : 0) | (ph[i].flags & 1 ? 4 : 0);
        mem_protect(base + start, end - start, prot);
    }
    if (stub_area) mem_protect(stub_area, stub_room, 1 | 4);
    NtClose(h);
    u8 *ibase = NULL;
    if (has_interp && !(ibase = musl_map())) fatal_exec("no memory for musl");

    const u32 stack_sz = 8 << 20;
    u8 *stack = mem_alloc_any(stack_sz, NULL);
    if (!stack) fatal_exec("no memory for stack");
    u8 *top = stack + stack_sz;
    u32 envc = g_exec_envc, argc = g_exec_argc;
    static u32 argp[4096], envpp[4096];
    u8 *sp = top - 16;
    size_t pl = strlen(g_exec_path) + 1;
    sp -= pl;
    memcpy(sp, g_exec_path, pl);
    u32 execfn = (u32)(ULONG_PTR)sp;

    size_t env_total = 0, arg_total = 0;
    for (u32 i = 0; i < envc; i++) env_total += strlen(g_exec_envp[i]) + 1;
    for (u32 i = 0; i < argc; i++) arg_total += strlen(g_exec_argv[i]) + 1;
    sp -= env_total;
    u8 *sw = sp;
    for (u32 i = 0; i < envc; i++) {
        size_t l = strlen(g_exec_envp[i]) + 1;
        memcpy(sw, g_exec_envp[i], l);
        envpp[i] = (u32)(ULONG_PTR)sw;
        sw += l;
    }
    sp -= arg_total;
    sw = sp;
    for (u32 i = 0; i < argc; i++) {
        size_t l = strlen(g_exec_argv[i]) + 1;
        memcpy(sw, g_exec_argv[i], l);
        argp[i] = (u32)(ULONG_PTR)sw;
        sw += l;
    }
    sp -= 16;
    random_fill(sp, 16);
    u32 rnd = (u32)(ULONG_PTR)sp;
    sp -= 8;
    memcpy(sp, "v7l", 4);
    u32 plat = (u32)(ULONG_PTR)sp;

    u32 hwcap = 2 | 4 | 16 | 64 | 128 | 4096 | 8192 | 32768 | (1 << 16) | (1 << 17) | (1 << 18) | (1 << 20);
    u32 aux[][2] = {
        {3, (u32)(ULONG_PTR)base + phdr_vaddr}, {4, 32}, {5, eh->phnum}, {6, PG}, {7, (u32)(ULONG_PTR)ibase},
        {8, 0}, {9, (u32)(ULONG_PTR)base + eh->entry}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {16, hwcap}, {17, 100},
        {23, 0}, {25, rnd}, {26, 0}, {31, execfn}, {15, plat}, {0, 0},
    };
    u32 naux = sizeof(aux) / sizeof(aux[0]);
    u32 words = 1 + argc + 1 + envc + 1 + naux * 2;
    u32 *s = (u32 *)(((ULONG_PTR)sp - words * 4) & ~7u);
    u32 *w = s;
    *w++ = argc;
    for (u32 i = 0; i < argc; i++) *w++ = argp[i];
    *w++ = 0;
    for (u32 i = 0; i < envc; i++) *w++ = envpp[i];
    *w++ = 0;
    memcpy(w, aux, naux * 8);

    strcpy(g_exe, g_exec_path);
    {

        const char *base = argc ? g_exec_argv[0] : g_exec_path;
        const char *sl = strrchr(base, '/');
        strncpy(me->comm, sl ? sl + 1 : base, 15);
        me->comm[15] = 0;
        int len = 0;
        for (u32 i = 0; i < argc; i++) {
            int l = (int)strlen(g_exec_argv[i]) + 1;
            if (len + l > (int)sizeof(me->cmd)) break;
            memcpy(me->cmd + len, g_exec_argv[i], l);
            len += l;
        }
        me->cmdlen = len;
    }
    CONTEXT *c = &g_exec_ctx;
    RtlCaptureContext(c);
    c->ContextFlags = CONTEXT_FULL;
    c->R0 = c->R1 = c->R2 = c->R3 = c->R4 = c->R5 = c->R6 = c->R7 = c->R8 = c->R9 = c->R10 = c->R11 = c->R12 = 0;
    c->Lr = 0;
    c->Sp = (u32)(ULONG_PTR)s;
    u32 entry = has_interp ? (u32)(ULONG_PTR)ibase + g_musl_entry : (u32)(ULONG_PTR)base + eh->entry;
    c->Pc = entry;
    c->Cpsr = (c->Cpsr & ~0x0600FC00u) | ((entry & 1) ? 0x20 : 0);
    trace("[%d] exec %s base=%p musl=%p sp=%08x pc=%08x\n", me->pid, g_exec_path, base, ibase, c->Sp, c->Pc);
    bkl_unlock();
    NtContinue(c, FALSE);
}

#include "fileext.inc"

static void patch_mapped_code(u8 *t, u32 ts) {
    int svcs = 0, tls = 0;
    u32 o = 0;
    while (o + 4 <= ts) {
        u16 hw = *(u16 *)(t + o);
        if ((hw >> 11) == 0x1D || (hw >> 11) == 0x1E || (hw >> 11) == 0x1F) {
            if (hw == 0xEE1D && (*(u16 *)(t + o + 2) & 0x0FFF) == 0x0F70) {
                *(u16 *)(t + o) = TLS_TRAP;
                tls++;
            }
            o += 4;
        } else {
            if (hw == 0xDF00 && sets_r7_nearby(t, o)) {
                *(u16 *)(t + o) = TRAP_INSN;
                svcs++;
            }
            o += 2;
        }
    }
    for (u32 k = 0; k < ts / 4; k++) {
        u32 *w = (u32 *)t + k;
        if (is_arm_flush((u32 *)t, k, ts / 4)) {
            *w = FLUSH_TRAP_ARM;
            svcs++;
        } else if (is_arm_svc((u32 *)t, k)) {
            *w = SYS_TRAP_ARM;
            svcs++;
        } else if (cp15_barrier_arm(*w)) {
            *w = cp15_barrier_arm(*w);
        } else if ((*w & 0xFFFF0FFF) == 0xEE1D0F70) {
            *w = TLS_TRAP_ARM | ((*w >> 12) & 15);
            tls++;
        }
    }
    if (svcs || tls) trace("[%d] library code %p+%x: raw svc %d, TLS %d\n", me->pid, t, ts, svcs, tls);
}

static int sys_mmap2(u32 addr, u32 len, int prot, int flags, int fd, u32 pgoff, u32 *out) {
    if (!len) return -EINVAL;
    u32 pages = (len + PG - 1) / PG;
    u32 first;
    if (flags & 0x10) {
        if (addr & (PG - 1)) return -EINVAL;
        if (!in_arena(addr, pages * PG)) return -ENOMEM;
        first = (addr - (u32)(ULONG_PTR)g_arena) / PG;
    } else {
        first = arena_find(pages);
        if (first == 0xFFFFFFFF) return -ENOMEM;
    }
    u8 *p = g_arena + (SIZE_T)first * PG;
    if ((flags & 0x20) && !prot) {
        mem_reserve_only(first, pages);
        *out = (u32)(ULONG_PTR)p;
        return 0;
    }
    int r = mem_commit(first, pages);
    if (r) return r;
    if (g_trace && !(flags & 0x20) && fd >= 0 && fd < MAX_FD) trace("[%d] map %s off=%x at %p len=%x prot=%x\n", me->pid, g_fdpath[fd], pgoff * PG, p, len, prot);
    if (!(flags & 0x20)) {
        if (fd < 0 || fd >= MAX_FD || g_fd[fd].type != F_FILE) {
            mem_decommit(first, pages);
            return fd >= 0 && fd < MAX_FD && g_fd[fd].type ? -19 : -EBADF;
        }
        LARGE_INTEGER off;
        IO_STATUS_BLOCK io;
        off.QuadPart = (i64)pgoff * PG;
        NTSTATUS st = NtReadFile(g_fd[fd].h, NULL, NULL, NULL, &io, p, len, &off, NULL);
        if (!NT_OK(st) && (u32)st != 0xC0000011) {
            mem_decommit(first, pages);
            return -errno_of(st);
        }
    }
    if (!(flags & 0x20) && (prot & 4)) patch_mapped_code(p, len);
    if (!(flags & 0x20) && (flags & 1) && (prot & 2)) smap_add((u32)(ULONG_PTR)p, len, (i64)pgoff * PG, g_fd[fd].h);
    if (prot != 3) mem_protect(p, (SIZE_T)pages * PG, prot);
    *out = (u32)(ULONG_PTR)p;
    return 0;
}

static int sys_mremap(u32 addr, u32 oldlen, u32 newlen, u32 flags) {
    u32 oldp = (oldlen + PG - 1) / PG, newp = (newlen + PG - 1) / PG;
    if ((addr & (PG - 1)) || !newp || !oldp) return -EINVAL;
    if (flags & 2) return -EINVAL;
    if (!in_arena(addr, oldp * PG)) return -ENOMEM;
    u32 first = (addr - (u32)(ULONG_PTR)g_arena) / PG;
    if (newp == oldp) return (int)addr;
    if (newp < oldp) {
        mem_decommit(first + newp, oldp - newp);
        return (int)addr;
    }
    u32 extra = newp - oldp, i;
    if (first + newp <= g_arena_pages) {
        for (i = 0; i < extra; i++)
            if (g_page_used[first + oldp + i]) break;
        if (i == extra && !mem_commit(first + oldp, extra)) return (int)addr;
    }
    if (!(flags & 1)) return -ENOMEM;
    u32 nf = arena_find(newp);
    if (nf == 0xFFFFFFFF || mem_commit(nf, newp)) return -ENOMEM;
    memcpy(g_arena + (SIZE_T)nf * PG, (void *)(ULONG_PTR)addr, (SIZE_T)oldp * PG);
    mem_decommit(first, oldp);
    return (int)(ULONG_PTR)(g_arena + (SIZE_T)nf * PG);
}

static int sys_munmap(u32 addr, u32 len) {
    if (addr & (PG - 1) || !len) return -EINVAL;
    smap_sync(addr, len, 1);
    u32 pages = (len + PG - 1) / PG;
    if (!in_arena(addr, pages * PG)) return 0;
    mem_decommit((addr - (u32)(ULONG_PTR)g_arena) / PG, pages);
    return 0;
}

static int sys_mprotect(u32 addr, u32 len, int prot) {
    if (addr & (PG - 1)) return -EINVAL;
    u32 pages = (len + PG - 1) / PG;
    if (!pages) return 0;
    if (!in_arena(addr, pages * PG) && !in_fixed(addr, pages * PG)) return -ENOMEM;
    if (prot && in_arena(addr, pages * PG) && mem_commit_lazy((addr - (u32)(ULONG_PTR)g_arena) / PG, pages)) return -ENOMEM;
    mem_protect((void *)(ULONG_PTR)addr, (SIZE_T)pages * PG, prot);
    return 0;
}

static u64 ts_to_100ns(const long *ts) { return (u64)ts[0] * 10000000ULL + (u64)ts[1] / 100; }
static u64 ts64_to_100ns(const i64 *ts) { return (u64)ts[0] * 10000000ULL + (u64)ts[1] / 100; }

static int do_sleep(u64 t100) {
    u64 start = mono_ns();
    for (;;) {
        u64 el = (mono_ns() - start) / 100;
        if (el >= t100) return 0;
        if (signal_ready()) return -EINTR;
        u64 left = (t100 - el) / 10000 + 1;
        wait_event(left > 100 ? 100 : (int)left);
    }
}

static void fill_time(u32 clk, i64 *sec, i64 *nsec) {
    if (clk == 1 || clk == 4 || clk == 6 || clk == 7) {
        u64 n = mono_ns();
        *sec = (i64)(n / 1000000000ULL);
        *nsec = (i64)(n % 1000000000ULL);
    } else {
        u64 t = now_100ns() - 116444736000000000ULL;
        *sec = (i64)(t / 10000000);
        *nsec = (i64)(t % 10000000) * 100;
    }
}

static int net_ioctl(u32 req, u32 arg) {
    if (req == 0x8912) {
        u32 *ifc = (u32 *)(ULONG_PTR)arg;
        u8 *out = (u8 *)(ULONG_PTR)ifc[1];
        u32 room = ifc[0], used = 0;
        for (int i = 0; i < g->nnetif; i++) {
            if (out) {
                if (used + 32 > room) break;
                memset(out + used, 0, 32);
                strcpy((char *)out + used, g->netifs[i].name);
                *(u16 *)(out + used + 16) = 2;
                memcpy(out + used + 20, &g->netifs[i].ip, 4);
            }
            used += 32;
        }
        ifc[0] = used;
        return 0;
    }
    u8 *r = (u8 *)(ULONG_PTR)arg;
    NetIf *n = NULL;
    int index = 0;
    for (int i = 0; i < g->nnetif; i++)
        if (!strncmp((char *)r, g->netifs[i].name, 16)) { n = &g->netifs[i]; index = i + 1; }
    if (!n) return -19;
    memset(r + 16, 0, 16);
    switch (req) {
        case 0x8913: *(u16 *)(r + 16) = n->loopback ? 0x49 : 0x1043; return 0;
        case 0x8915: *(u16 *)(r + 16) = 2; memcpy(r + 20, &n->ip, 4); return 0;
        case 0x8917: *(u16 *)(r + 16) = 2; memcpy(r + 20, &n->ip, 4); return 0;
        case 0x8919: {
            u32 b = n->ip | ~n->mask;
            *(u16 *)(r + 16) = 2;
            memcpy(r + 20, &b, 4);
            return 0;
        }
        case 0x891B: *(u16 *)(r + 16) = 2; memcpy(r + 20, &n->mask, 4); return 0;
        case 0x891D: return 0;
        case 0x8921: *(u32 *)(r + 16) = n->loopback ? 65536 : 1500; return 0;
        case 0x8927: *(u16 *)(r + 16) = n->loopback ? 772 : 1; memcpy(r + 18, n->mac, 6); return 0;
        case 0x8933: *(u32 *)(r + 16) = index; return 0;
        case 0x8942: *(u32 *)(r + 16) = n->loopback ? 0 : 1000; return 0;
    }
    return -ENOTTY;
}

static int do_ioctl(int fd, u32 req, u32 arg) {
    if (fd < 0 || fd >= MAX_FD || !g_fd[fd].type) return -EBADF;
    if ((req & 0xFF00) == 0x8900 && (g_fd[fd].type == F_SOCK || g_fd[fd].type == F_UNIX || g_fd[fd].type == F_NETLINK)) return net_ioctl(req, arg);
    Tty *t = &g->ttys[g_fd[fd].type == F_TTY || g_fd[fd].type == F_PTM ? g_fd[fd].idx : 0];
    if (req == 0x5452) return 0;
    if (req == 0x5451 || req == 0x5450) {
        g_fd[fd].cloexec = req == 0x5451;
        return 0;
    }
    if (req == 0x5421) {
        if (*(int *)(ULONG_PTR)arg) g_fd[fd].flags |= O_NONBLOCK_;
        else g_fd[fd].flags &= ~O_NONBLOCK_;
        return 0;
    }
    if (req == 0x541B && g_fd[fd].type == F_PIPE_R) {
        Pipe *p = &g->pipes[g_fd[fd].idx];
        *(int *)(ULONG_PTR)arg = (int)((p->head + PIPE_SZ - p->tail) % PIPE_SZ);
        return 0;
    }
    if (req == 0x541B && g_fd[fd].type == F_SOCK) {
        Sock *s = &g->socks[g_fd[fd].idx];
        *(int *)(ULONG_PTR)arg = (int)ring_used(s->rhead, s->rtail, SOCK_RX);
        return 0;
    }
    if (g_fd[fd].type != F_TTY && g_fd[fd].type != F_PTM) return -ENOTTY;
    if (g_fd[fd].type == F_PTM) {
        if (req == 0x80045430) { *(u32 *)(ULONG_PTR)arg = (u32)g_fd[fd].idx; return 0; }
        if (req == 0x40045431) return 0;
    }
    switch (req) {
        case 0x5401: memcpy((void *)(ULONG_PTR)arg, &t->t, sizeof(t->t)); return 0;
        case 0x5402: case 0x5403: case 0x5404: memcpy(&t->t, (void *)(ULONG_PTR)arg, sizeof(t->t)); return 0;
        case 0x540F: *(int *)(ULONG_PTR)arg = t->fg_pgrp; return 0;
        case 0x5410: t->fg_pgrp = *(int *)(ULONG_PTR)arg; return 0;
        case 0x5413: { u16 *w = (u16 *)(ULONG_PTR)arg; w[0] = t->rows; w[1] = t->cols; w[2] = w[3] = 0; return 0; }
        case 0x5414: {
            u16 *w = (u16 *)(ULONG_PTR)arg;
            t->rows = w[0];
            t->cols = w[1];
            lock_();
            if (t->fg_pgrp) signal_pgrp_locked(t->fg_pgrp, SIGWINCH_);
            unlock_();
            wake_all();
            return 0;
        }
        case 0x541B: *(int *)(ULONG_PTR)arg = (int)((t->ihead + TTY_IN_SZ - t->itail) % TTY_IN_SZ); return 0;
        case 0x540E:
            me->ctty = g_fd[fd].idx;
            t->sid = me->sid;
            t->fg_pgrp = me->pgid;
            return 0;
        case 0x5422: me->ctty = -1; return 0;
        case 0x5429: *(int *)(ULONG_PTR)arg = t->sid; return 0;
    }
    return -EINVAL;
}

static int do_pipe(int *fds, int flags) {
    lock_();
    int i;
    for (i = 0; i < MAX_PIPE; i++)
        if (g->pipes[i].readers <= 0 && g->pipes[i].writers <= 0 && !g->pipes[i].reserved) break;
    if (i == MAX_PIPE) {
        unlock_();
        return -ENFILE;
    }
    Pipe *p = &g->pipes[i];
    p->readers = p->writers = 0;
    p->head = p->tail = 0;
    p->nfdq = 0;
    int a = fd_alloc(0);
    if (a < 0) { unlock_(); return a; }
    g_fd[a].type = F_PIPE_R;
    int b = fd_alloc(0);
    if (b < 0) { g_fd[a].type = 0; unlock_(); return b; }
    g_fd[a].idx = i;
    g_fd[a].flags = flags & O_NONBLOCK_;
    g_fd[a].cloexec = !!(flags & O_CLOEXEC_);
    g_fd[b].type = F_PIPE_W;
    g_fd[b].idx = i;
    g_fd[b].flags = (flags & O_NONBLOCK_) | O_WRONLY_;
    g_fd[b].cloexec = !!(flags & O_CLOEXEC_);
    pipe_ref_add(&g_fd[a]);
    pipe_ref_add(&g_fd[b]);
    unlock_();
    fds[0] = a;
    fds[1] = b;
    return 0;
}

static int do_socketpair(int type, int *fds) {
    int a[2], b[2];
    int flags = type & (O_NONBLOCK_ | O_CLOEXEC_);
    int r = do_pipe(a, flags);
    if (r) return r;
    r = do_pipe(b, flags);
    if (r) { fd_close(a[0]); fd_close(a[1]); return r; }

    Fd *f0 = &g_fd[a[0]], *f1 = &g_fd[b[0]];
    f0->type = F_SPAIR;
    f0->idx2 = g_fd[b[1]].idx;
    f0->flags = O_RDWR_ | (flags & O_NONBLOCK_);
    f1->type = F_SPAIR;
    f1->idx2 = g_fd[a[1]].idx;
    f1->flags = O_RDWR_ | (flags & O_NONBLOCK_);
    memset(&g_fd[a[1]], 0, sizeof(Fd));
    memset(&g_fd[b[1]], 0, sizeof(Fd));
    fds[0] = a[0];
    fds[1] = b[0];
    return 0;
}

static int do_fcntl(int fd, int cmd, u32 arg) {
    if (fd < 0 || fd >= MAX_FD || !g_fd[fd].type) return -EBADF;
    Fd *f = &g_fd[fd];
    switch (cmd) {
        case 0: case 1030: {
            int n = fd_alloc((int)arg);
            if (n < 0) return n;
            return fd_dup_to(fd, n, cmd == 1030);
        }
        case 1: return f->cloexec;
        case 2: f->cloexec = arg & 1; return 0;
        case 3: {
            int acc = f->flags & 3;
            if (f->type == F_PIPE_W) acc = O_WRONLY_;
            return (f->flags & ~3) | acc;
        }
        case 4: f->flags = (f->flags & ~(O_NONBLOCK_ | O_APPEND_)) | (arg & (O_NONBLOCK_ | O_APPEND_)); return 0;
        case 5: case 12: case 6: case 7: case 13: case 14: return 0;
        case 8: return 0;
        case 9: return 0;
    }
    return -EINVAL;
}

static int sys_getcwd(char *buf, u32 size) {
    u32 l = (u32)strlen(g_cwd) + 1;
    if (l > size) return -ERANGE;
    memcpy(buf, g_cwd, l);
    return (int)l;
}

static int sys_chdir(const char *path) {
    static char abs[PATH_MAX_];
    int r = resolve(AT_FDCWD_, path, 1, abs);
    if (r) return r;
    ULONG a = path_attr(abs);
    if (a == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    if (!(a & FILE_ATTRIBUTE_DIRECTORY)) return -ENOTDIR;
    strcpy(g_cwd, abs);
    return 0;
}

static int sys_readlinkat(int dirfd, const char *path, char *buf, u32 size) {
    static char abs[PATH_MAX_], tmp[PATH_MAX_];
    int r = resolve(dirfd, path, 0, abs);
    if (r) return r;
    if (!strcmp(abs, "/proc/self/exe") || !strcmp(abs, "/proc/self/cwd")) {
        strcpy(tmp, abs[11] == 'e' ? g_exe : g_cwd);
        r = (int)strlen(tmp);
    } else if (proc_self_fd(abs) >= 0) {
        int n = proc_self_fd(abs);
        strcpy(tmp, g_fdpath[n][0] ? g_fdpath[n] : "/dev/pts/0");
        r = (int)strlen(tmp);
    } else {
        ULONG a = path_attr(abs);
        if (a == INVALID_FILE_ATTRIBUTES) return -ENOENT;
        if (!(a & FILE_ATTRIBUTE_SYSTEM) || (a & FILE_ATTRIBUTE_DIRECTORY)) return -EINVAL;
        r = read_link_abs(abs, tmp, sizeof(tmp));
        if (r < 0) return r;
    }
    if ((u32)r > size) r = (int)size;
    memcpy(buf, tmp, r);
    return r;
}

static int sys_utimens(int dirfd, const char *path, const i64 *ts, int flags, int is64) {
    static char abs[PATH_MAX_];
    HANDLE h = NULL;
    int own = 0;
    if (!path) {
        if (dirfd < 0 || dirfd >= MAX_FD || (g_fd[dirfd].type != F_FILE && g_fd[dirfd].type != F_DIR)) return -EBADF;
        h = g_fd[dirfd].h;
    } else {
        int r = resolve(dirfd, path, !(flags & AT_NOFOLLOW), abs);
        if (r) return r;
        NTSTATUS st = nt_open(abs, FILE_WRITE_ATTRIBUTES, FILE_OPEN_, 0, 0, &h);
        if (!NT_OK(st)) return -errno_of(st);
        own = 1;
    }
    FILE_BASIC b;
    IO_STATUS_BLOCK io;
    memset(&b, 0, sizeof(b));
    i64 now = (i64)now_100ns();
    for (int i = 0; i < 2; i++) {
        i64 v = now;
        if (ts) {
            i64 sec, nsec;
            if (is64) { sec = ts[i * 2]; nsec = ts[i * 2 + 1]; }
            else { sec = ((const long *)ts)[i * 2]; nsec = ((const long *)ts)[i * 2 + 1]; }
            if (nsec == 0x3FFFFFFE) continue;
            if (nsec != 0x3FFFFFFF) v = sec * 10000000 + nsec / 100 + 116444736000000000LL;
        }
        if (i == 0) b.a.QuadPart = v;
        else b.w.QuadPart = v;
    }
    NTSTATUS st = NtSetInformationFile(h, &io, &b, sizeof(FILE_BASIC), 4);
    if (own) NtClose(h);
    return NT_OK(st) ? 0 : -errno_of(st);
}

static void __cdecl fork_stub(void) {
    T *t = CUR;
    bkl_lock();
    u32 fl = t->fork_flags;
    int r = do_fork(&t->fork_ctx, fl, t->fork_stack, (fl & 0x01000000) ? (u32 *)(ULONG_PTR)t->fork_ctid : NULL);
    if (r == 0 && (fl & 0x80000)) ((NT_TIB *)NtCurrentTeb())->ArbitraryUserPointer = (PVOID)(ULONG_PTR)t->fork_tls;
    t->fork_ctx.R0 = (u32)r;
    deliver_signals(&t->fork_ctx);
    if (t->fork_ctx.Cpsr & 0x20) t->fork_ctx.Pc |= 1;
    bkl_unlock();
    NtContinue(&t->fork_ctx, FALSE);
}

static void begin_fork(CONTEXT *c, u32 flags, u32 stack, u32 tls, u32 ctid) {
    T *t = CUR;
    t->fork_ctx = *c;
    t->fork_ctx.Pc += TRAP_LEN(c);
    t->fork_flags = flags;
    t->fork_stack = stack;
    t->fork_tls = tls;
    t->fork_ctid = ctid;
    c->Sp = (DWORD)(ULONG_PTR)t->host_top & ~7u;
    c->Pc = (DWORD)(ULONG_PTR)fork_stub | 1u;
    c->Cpsr = (c->Cpsr & ~0x0600FC00u) | 0x20;
}

#include "threads.inc"
#include "epoll.inc"
#include "unix.inc"
#include "netlink.inc"
#include "inotify.inc"
#define IS_UNIX(fd) ((fd) < MAX_FD && g_fd[fd].type == F_UNIX)
#define IS_UDG(fd) (IS_UNIX(fd) && g->unixs[g_fd[fd].idx].dgram)
#define IS_NL(fd) ((fd) < MAX_FD && g_fd[fd].type == F_NETLINK)

static int set_ids(u32 *ids, u32 r, u32 e, u32 s) {
    u32 v[3] = {r, e, s};
    if (me->uid[1])
        for (int i = 0; i < 3; i++)
            if (v[i] != ~0u && v[i] != ids[0] && v[i] != ids[1] && v[i] != ids[2]) return -EPERM;
    for (int i = 0; i < 3; i++)
        if (v[i] != ~0u) ids[i] = v[i];
    return 0;
}

static int do_syscall(CONTEXT *c, int *advance) {
    u32 nr = c->R7, a0 = c->R0, a1 = c->R1, a2 = c->R2, a3 = c->R3, a4 = c->R4, a5 = c->R5;
#define P(x) ((void *)(ULONG_PTR)(x))
    if (nr >= 282 && nr <= 297 && IS_NL(a0)) {
        int nlr;
        if (nl_syscall(nr, a0, a1, a2, a3, a4, a5, &nlr)) return nlr;
    }
    switch (nr) {
        case 1: thread_exit(c, a0); return 0;
        case 248: do_exit((a0 & 0xFF) << 8); return 0;
        case 2: case 190: begin_fork(c, 0, 0, 0, 0); *advance = 0; return 0;
        case 120:
            if (a0 & 0x10000) return do_thread_create(c, a0, a1, a2, a3, a4);
            begin_fork(c, a0, a1, a3, a4);
            *advance = 0;
            return 0;
        case 3: return sys_read(a0, P(a1), a2);
        case 4: return sys_write(a0, P(a1), a2);
        case 145: case 146: {
            u32 *iov = P(a1);
            int total = 0;
            for (u32 i = 0; i < a2; i++) {
                u32 base = iov[i * 2], len = iov[i * 2 + 1];
                if (!len) continue;
                int r = nr == 145 ? sys_read(a0, P(base), len) : sys_write(a0, P(base), len);
                if (r < 0) return total ? total : r;
                total += r;
                if ((u32)r < len) break;
            }
            return total;
        }
        case 180: case 181: {
            if (a0 >= MAX_FD || g_fd[a0].type != F_FILE) return -EBADF;
            IO_STATUS_BLOCK io;
            LARGE_INTEGER off;
            off.LowPart = a4;
            off.HighPart = a5;
            NTSTATUS st = nr == 180 ? NtReadFile(g_fd[a0].h, NULL, NULL, NULL, &io, P(a1), a2, &off, NULL)
                                    : NtWriteFile(g_fd[a0].h, NULL, NULL, NULL, &io, P(a1), a2, &off, NULL);
            if ((u32)st == 0xC0000011) return 0;
            return NT_OK(st) ? (int)io.Information : -errno_of(st);
        }
        case 5: return sys_openat(AT_FDCWD_, P(a0), a1, a2);
        case 322: return sys_openat(a0, P(a1), a2, a3);
        case 8: return sys_openat(AT_FDCWD_, P(a0), O_CREAT_ | O_WRONLY_ | O_TRUNC_, a1);
        case 6: return fd_close(a0);
        case 19: {
            if (a0 >= MAX_FD || !g_fd[a0].type) return -EBADF;
            int e;
            i64 r = file_seek(&g_fd[a0], (long)a1, a2, &e);
            return e ? -e : (int)r;
        }
        case 140: {
            if (a0 >= MAX_FD || !g_fd[a0].type) return -EBADF;
            int e;
            i64 r = file_seek(&g_fd[a0], ((i64)a1 << 32) | a2, a4, &e);
            if (e) return -e;
            *(i64 *)P(a3) = r;
            return 0;
        }
        case 195: return sys_fstatat(AT_FDCWD_, P(a0), P(a1), 0);
        case 196: return sys_fstatat(AT_FDCWD_, P(a0), P(a1), AT_NOFOLLOW);
        case 197: return sys_fstatat(a0, "", P(a1), AT_EMPTY_PATH);
        case 327: return sys_fstatat(a0, P(a1), P(a2), a3);
        case 397: return -ENOSYS;
        case 217: return sys_getdents64(a0, P(a1), a2);
        case 33: case 334: case 439: {
            static char abs[PATH_MAX_];
            kstat64 s;
            const char *p = nr == 33 ? P(a0) : P(a1);
            int dfd = nr == 33 ? AT_FDCWD_ : (int)a0;
            int mode = nr == 33 ? a1 : a2;
            int r = sys_fstatat(dfd, p, &s, 0);
            (void)abs;
            if (r) return r;
            if ((mode & 1) && !(s.st_mode & 0111) && (s.st_mode & S_IFMT_) != S_IFDIR_) return -EACCES;
            return 0;
        }
        case 10: {
            static char abs[PATH_MAX_];
            int r = resolve(AT_FDCWD_, P(a0), 0, abs);
            return r ? r : unlink_abs(abs, 0);
        }
        case 328: {
            static char abs[PATH_MAX_];
            int r = resolve(a0, P(a1), 0, abs);
            return r ? r : unlink_abs(abs, a2 & AT_REMOVEDIR);
        }
        case 40: {
            static char abs[PATH_MAX_];
            int r = resolve(AT_FDCWD_, P(a0), 0, abs);
            return r ? r : unlink_abs(abs, 1);
        }
        case 39: case 323: {
            static char abs[PATH_MAX_];
            int dfd = nr == 39 ? AT_FDCWD_ : (int)a0;
            const char *p = nr == 39 ? P(a0) : P(a1);
            u32 mode = nr == 39 ? a1 : a2;
            int r = resolve(dfd, p, 0, abs);
            return r ? r : mkdir_abs(abs, mode & ~g_umask, 0, 0);
        }
        case 83: case 331: {
            static char abs[PATH_MAX_];
            int dfd = nr == 83 ? AT_FDCWD_ : (int)a1;
            const char *p = nr == 83 ? P(a1) : P(a2);
            int r = resolve(dfd, p, 0, abs);
            if (r) return r;
            if (path_attr(abs) != INVALID_FILE_ATTRIBUTES) return -EEXIST;
            return symlink_abs(P(a0), abs, 0, 0);
        }
        case 85: return sys_readlinkat(AT_FDCWD_, P(a0), P(a1), a2);
        case 332: return sys_readlinkat(a0, P(a1), P(a2), a3);
        case 38: return sys_rename(AT_FDCWD_, P(a0), AT_FDCWD_, P(a1));
        case 329: case 382: return sys_rename(a0, P(a1), a2, P(a3));
        case 9: case 330: {
            static char abs[PATH_MAX_], abs2[PATH_MAX_];
            static wchar_t w[PATH_MAX_];
            int r = nr == 9 ? resolve(AT_FDCWD_, P(a0), 1, abs) : resolve(a0, P(a1), !!(a4 & 0x400), abs);
            if (r) return r;
            r = nr == 9 ? resolve(AT_FDCWD_, P(a1), 0, abs2) : resolve(a2, P(a3), 0, abs2);
            if (r) return r;
            HANDLE h;
            NTSTATUS st = nt_open(abs, FILE_READ_ATTRIBUTES, FILE_OPEN_, FO_NONDIR, 0, &h);
            if (!NT_OK(st)) return -errno_of(st);
            int bytes = nt_path(abs2, w);
            st = rename_handle(h, w, bytes, 11, 0);
            NtClose(h);
            return NT_OK(st) ? 0 : -errno_of(st);
        }
        case 15: return sys_chmod(AT_FDCWD_, P(a0), a1);
        case 333: return sys_chmod(a0, P(a1), a2);
        case 94: {
            if (a0 >= MAX_FD) return -EBADF;
            return sys_chmod(AT_FDCWD_, g_fdpath[a0], a1);
        }
        case 212: case 182: return sys_chown(AT_FDCWD_, P(a0), a1, a2, 0);
        case 198: return sys_chown(AT_FDCWD_, P(a0), a1, a2, 1);
        case 325: return sys_chown(a0, P(a1), a2, a3, a4 & AT_NOFOLLOW);
        case 207: case 95: {
            if (a0 >= MAX_FD) return -EBADF;
            return sys_chown(AT_FDCWD_, g_fdpath[a0], a1, a2, 0);
        }
        case 348: return sys_utimens(a0, P(a1), P(a2), a3, 0);
        case 412: return sys_utimens(a0, P(a1), P(a2), a3, 1);
        case 93: case 194: {
            if (a0 >= MAX_FD || g_fd[a0].type != F_FILE) return -EBADF;
            IO_STATUS_BLOCK io;
            LARGE_INTEGER z;
            z.LowPart = nr == 194 ? a2 : a1;
            z.HighPart = nr == 194 ? (LONG)a3 : 0;
            NTSTATUS st = NtSetInformationFile(g_fd[a0].h, &io, &z, sizeof(z), 20);
            return NT_OK(st) ? 0 : -errno_of(st);
        }
        case 352: {
            if (a0 >= MAX_FD || !g_fd[a0].type) return -EBADF;
            if (g_fd[a0].type != F_FILE) return -19;
            if (a1 & ~1u) return -EOPNOTSUPP_;
            IO_STATUS_BLOCK io;
            FILE_STD std;
            if (!NT_OK(NtQueryInformationFile(g_fd[a0].h, &io, &std, sizeof(std), 5))) return -EBADF;
            u64 want = ((u64)a3 << 32 | a2) + ((u64)a5 << 32 | a4);
            if ((a1 & 1) || want <= (u64)std.eof.QuadPart) return 0;
            LARGE_INTEGER z;
            z.QuadPart = (i64)want;
            NTSTATUS st = NtSetInformationFile(g_fd[a0].h, &io, &z, sizeof(z), 20);
            return NT_OK(st) ? 0 : -errno_of(st);
        }
        case 118: case 148: {
            if (a0 >= MAX_FD || !g_fd[a0].type) return -EBADF;
            if (g_fd[a0].type == F_FILE) {
                IO_STATUS_BLOCK io;
                NtFlushBuffersFile(g_fd[a0].h, &io);
            }
            return 0;
        }
        case 220: {

            if (a2 != 4 || !in_arena(a0, a1) || (a0 & (PG - 1))) return 0;
            u32 first = (a0 - (u32)(ULONG_PTR)g_arena) / PG, pages = (a1 + PG - 1) / PG;
            for (u32 i = 0; i < pages;) {
                if (g_page_used[first + i] != 1) { i++; continue; }
                struct { PVOID base, alloc; ULONG aprot; SIZE_T size; ULONG state, prot, type; } mi;
                u8 *at = g_arena + (SIZE_T)(first + i) * PG;
                if (!NT_OK(NtQueryVirtualMemory(SELF, at, 0, &mi, sizeof(mi), NULL))) break;
                u32 run = (u32)(((u8 *)mi.base + mi.size - at) / PG);
                if (run > pages - i) run = pages - i;
                for (u32 k = 1; k < run; k++)
                    if (g_page_used[first + i + k] != 1) { run = k; break; }
                PVOID q = at;
                SIZE_T s = (SIZE_T)run * PG;
                NtFreeVirtualMemory(SELF, &q, &s, MEM_DECOMMIT);
                q = at;
                s = (SIZE_T)run * PG;
                NtAllocateVirtualMemory(SELF, &q, 0, &s, MEM_COMMIT, mi.prot ? mi.prot : PAGE_READWRITE);
                i += run;
            }
            return 0;
        }
        case 144: smap_sync(a0, a1, 0); return 0;
        case 36: case 143: return 0;
        case 12: return sys_chdir(P(a0));
        case 133: {
            if (a0 >= MAX_FD || g_fd[a0].type != F_DIR) return -ENOTDIR;
            strcpy(g_cwd, g_fdpath[a0]);
            return 0;
        }
        case 183: return sys_getcwd(P(a0), a1);
        case 60: { u32 o = g_umask; g_umask = a0 & 0777; return (int)o; }
        case 41: {
            if (a0 >= MAX_FD || !g_fd[a0].type) return -EBADF;
            int n = fd_alloc(0);
            return n < 0 ? n : fd_dup_to(a0, n, 0);
        }
        case 63: case 358: {
            if (a0 >= MAX_FD || !g_fd[a0].type || a1 >= MAX_FD) return -EBADF;
            if (a0 == a1) return nr == 63 ? (int)a1 : -EINVAL;
            return fd_dup_to(a0, a1, nr == 358 && (a2 & O_CLOEXEC_));
        }
        case 42: return do_pipe(P(a0), 0);
        case 359: return do_pipe(P(a0), a1);
        case 55: case 221: return do_fcntl(a0, a1, a2);
        case 54: return do_ioctl(a0, a1, a2);
        case 168: return do_poll(P(a0), a1, (int)a2 < 0 ? -1 : (i64)a2 * 10000);
        case 336: return do_poll(P(a0), a1, a2 ? (i64)ts_to_100ns(P(a2)) : -1);
        case 414: return do_poll(P(a0), a1, a2 ? (i64)ts64_to_100ns(P(a2)) : -1);
        case 142: {
            long *tv = P(a4);
            return do_select(a0, P(a1), P(a2), P(a3), tv ? (i64)tv[0] * 10000000 + (i64)tv[1] * 10 : -1);
        }
        case 335: return do_select(a0, P(a1), P(a2), P(a3), a4 ? (i64)ts_to_100ns(P(a4)) : -1);
        case 413: return do_select(a0, P(a1), P(a2), P(a3), a4 ? (i64)ts64_to_100ns(P(a4)) : -1);
        case 45: return (int)a0 ? -ENOMEM : 0;
        case 192: { u32 out; int r = sys_mmap2(a0, a1, a2, a3, a4, a5, &out); return r ? r : (int)out; }
        case 91:
            if (c->Sp >= a0 && c->Sp - a0 < a1) {

                CUR->unmap_addr = a0;
                CUR->unmap_len = a1;
                return 0;
            }
            return sys_munmap(a0, a1);
        case 125: return sys_mprotect(a0, a1, a2);
        case 163: return sys_mremap(a0, a1, a2, a3);
        case 11: return sys_execve(P(a0), P(a1), P(a2));
        case 114: return sys_wait4(a0, P(a1), a2);
        case 20: return me->pid;
        case 224: return CUR->tid;
        case 64: return me->ppid;
        case 24: case 199: return me->uid[0];
        case 49: case 201: return me->uid[1];
        case 47: case 200: return me->gid[0];
        case 50: case 202: return me->gid[1];
        case 57: {
            int pid = a0 ? (int)a0 : me->pid, pg = a1 ? (int)a1 : pid;
            lock_();
            Proc *p = proc_by_pid(pid);
            if (p) p->pgid = pg;
            unlock_();
            return p ? 0 : -ESRCH;
        }
        case 132: {
            int pid = a0 ? (int)a0 : me->pid;
            Proc *p = proc_by_pid(pid);
            return p ? p->pgid : -ESRCH;
        }
        case 65: return me->pgid;
        case 66: me->sid = me->pid; me->pgid = me->pid; me->ctty = -1; return me->pid;
        case 147: {
            int pid = a0 ? (int)a0 : me->pid;
            Proc *p = proc_by_pid(pid);
            return p ? p->sid : -ESRCH;
        }
        case 37: case 238: case 268: {
            int pid = nr == 268 ? (int)a1 : (int)a0, sig = nr == 268 ? (int)a2 : (int)a1;
            if (sig < 0 || sig > 64) return -EINVAL;
            int found = 0;
            int own_thread = 0;
            if (nr != 37)
                for (int j = 0; j < MAX_THREADS; j++)
                    if (g_threads[j].used && g_threads[j].tid == pid) own_thread = 1;
            lock_();
            for (int i = 0; i < MAX_PROC; i++) {
                Proc *p = &g->procs[i];
                if (p->state != 1) continue;
                int match = own_thread ? p == me : pid > 0 ? p->pid == pid : pid == 0 ? p->pgid == me->pgid : pid == -1 ? p->pid != 1 : p->pgid == -pid;
                if (!match) continue;
                found = 1;
                if (sig && nr != 37 && p == me) {

                    int tid = nr == 268 ? (int)a1 : (int)a0;
                    for (int j = 0; j < MAX_THREADS; j++)
                        if (g_threads[j].used && g_threads[j].tid == tid) g_threads[j].pending |= SIGBIT(sig);
                } else if (sig) {
                    if (p == me && sig != SIGCONT_) { if (!(me->ignored & SIGBIT(sig))) me->pending |= SIGBIT(sig); }
                    else send_signal_locked(p, sig);
                }
            }
            unlock_();
            wake_all();
            return found ? 0 : -ESRCH;
        }
        case 174: return sys_rt_sigaction(a0, P(a1), P(a2));
        case 175: {
            T *t = CUR;
            lock_();
            u64 old = t->blocked;
            if (a1) {
                u64 v = *(u64 *)P(a1);
                if (a0 == 0) t->blocked |= v;
                else if (a0 == 1) t->blocked &= ~v;
                else t->blocked = v;
                t->blocked &= ~(SIGBIT(SIGKILL_) | SIGBIT(SIGSTOP_));
                proc_blocked_update();
            }
            unlock_();
            if (a2) *(u64 *)P(a2) = old;
            return 0;
        }
        case 176: *(u64 *)P(a0) = (me->pending | CUR->pending) & CUR->blocked; return 0;
        case 177: case 421: {
            i64 to = -1;
            if (a2) to = nr == 421 ? (i64)ts64_to_100ns(P(a2)) : (i64)ts_to_100ns(P(a2));
            return sys_sigtimedwait(*(u64 *)P(a0), P(a1), to);
        }
        case 179: {
            T *t = CUR;
            u64 old = t->blocked;
            lock_();
            t->blocked = *(u64 *)P(a0) & ~(SIGBIT(SIGKILL_) | SIGBIT(SIGSTOP_));
            proc_blocked_update();
            unlock_();
            for (;;) {
                timers_poll();
                if (signal_ready()) break;
                wait_event(100);
            }
            t->suspend_old = old;
            t->suspend_active = 1;
            return -EINTR;
        }
        case 27: {
            Timer *tm = &g_timers[MAX_TIMERS - 1];
            u64 now = mono_ns();
            int left = tm->armed && tm->deadline_ns > now ? (int)((tm->deadline_ns - now) / 1000000000ULL) + 1 : 0;
            memset(tm, 0, sizeof(*tm));
            tm->used = 1;
            tm->signo = SIGALRM_;
            if (a0) {
                tm->armed = 1;
                tm->deadline_ns = now + (u64)a0 * 1000000000ULL;
            }
            return left;
        }
        case 104: case 105: {
            if (a0 != 0) return nr == 104 ? 0 : -EINVAL;
            Timer *tm = &g_timers[MAX_TIMERS - 1];
            long *oldv = nr == 104 ? P(a2) : P(a1);
            if (oldv) {
                u64 now = mono_ns(), left = tm->armed && tm->deadline_ns > now ? tm->deadline_ns - now : 0;
                oldv[0] = (long)(tm->interval_ns / 1000000000ULL);
                oldv[1] = (long)((tm->interval_ns % 1000000000ULL) / 1000);
                oldv[2] = (long)(left / 1000000000ULL);
                oldv[3] = (long)((left % 1000000000ULL) / 1000);
            }
            if (nr == 104 && a1) {
                const long *nv = P(a1);
                u64 val = (u64)nv[2] * 1000000000ULL + (u64)nv[3] * 1000;
                memset(tm, 0, sizeof(*tm));
                tm->used = 1;
                tm->signo = SIGALRM_;
                tm->interval_ns = (u64)nv[0] * 1000000000ULL + (u64)nv[1] * 1000;
                if (val) {
                    tm->armed = 1;
                    tm->deadline_ns = mono_ns() + val;
                }
            }
            return 0;
        }
        case 257: return sys_timer_create(a0, P(a1), P(a2));
        case 258: {
            const long *its = P(a2);
            if (a3) memset(P(a3), 0, 16);
            return sys_timer_settime(a0, a1, (i64)its[0] * 1000000000 + its[1], (i64)its[2] * 1000000000 + its[3]);
        }
        case 409: {
            const i64 *its = P(a2);
            if (a3) memset(P(a3), 0, 32);
            return sys_timer_settime(a0, a1, its[0] * 1000000000 + its[1], its[2] * 1000000000 + its[3]);
        }
        case 259: case 408: {
            if (a0 >= MAX_TIMERS || !g_timers[a0].used) return -EINVAL;
            Timer *tm = &g_timers[a0];
            u64 now = mono_ns(), left = tm->armed && tm->deadline_ns > now ? tm->deadline_ns - now : 0;
            if (nr == 408) {
                i64 *o = P(a1);
                o[0] = (i64)(tm->interval_ns / 1000000000); o[1] = (i64)(tm->interval_ns % 1000000000);
                o[2] = (i64)(left / 1000000000); o[3] = (i64)(left % 1000000000);
            } else {
                long *o = P(a1);
                o[0] = (long)(tm->interval_ns / 1000000000); o[1] = (long)(tm->interval_ns % 1000000000);
                o[2] = (long)(left / 1000000000); o[3] = (long)(left % 1000000000);
            }
            return 0;
        }
        case 260: return 0;
        case 261:
            if (a0 >= MAX_TIMERS || !g_timers[a0].used) return -EINVAL;
            g_timers[a0].used = 0;
            return 0;
        case 173: case 119: {
            SigFrame *fr = (SigFrame *)(ULONG_PTR)c->Sp;
            if (fr->magic != 0x54585347) { do_exit(SIGSEGV_); }
            u32 *uc = (u32 *)fr->uc;
            u64 mask;
            memcpy(&mask, uc + UC_MASK, 8);
            lock_();
            CUR->blocked = mask & ~(SIGBIT(SIGKILL_) | SIGBIT(SIGSTOP_));
            proc_blocked_update();
            unlock_();
            *c = fr->ctx;

            memcpy(&c->R0, uc + UC_REGS, 13 * 4);
            c->Sp = uc[UC_REGS + 13];
            c->Lr = uc[UC_REGS + 14];
            c->Cpsr = (c->Cpsr & ~0xF0000020u) | (uc[UC_REGS + 16] & 0xF0000020u);
            c->Pc = (c->Cpsr & 0x20) ? uc[UC_REGS + 15] | 1u : uc[UC_REGS + 15] & ~3u;
            *advance = 0;
            return (int)c->R0;
        }
        case 187: case 239: {
            static u8 sf[32768];
            u32 total = 0, count = a3;
            LARGE_INTEGER off;
            if (a2) {
                if (a1 >= MAX_FD || g_fd[a1].type != F_FILE) return -EINVAL;
                off.QuadPart = nr == 239 ? *(i64 *)P(a2) : (i64)*(long *)P(a2);
            }
            while (total < count) {
                u32 want = count - total < sizeof(sf) ? count - total : sizeof(sf);
                int r;
                if (a2) {
                    IO_STATUS_BLOCK io;
                    NTSTATUS st = NtReadFile(g_fd[a1].h, NULL, NULL, NULL, &io, sf, want, &off, NULL);
                    r = (u32)st == 0xC0000011 ? 0 : NT_OK(st) ? (int)io.Information : -errno_of(st);
                    if (r > 0) off.QuadPart += r;
                } else {
                    r = sys_read(a1, sf, want);
                }
                if (r <= 0) { if (r < 0 && !total) return r; break; }
                int w = sys_write(a0, sf, r);
                if (w < 0) return total ? (int)total : w;
                total += w;
                if (w < r) break;
            }
            if (a2) {
                if (nr == 239) *(i64 *)P(a2) = off.QuadPart;
                else *(long *)P(a2) = (long)off.QuadPart;
            }
            return (int)total;
        }
        case 351: case 356: {

            int fds[2];
            int fl = nr == 356 ? a1 : 0;
            int r = do_pipe(fds, fl & (O_NONBLOCK_ | O_CLOEXEC_));
            if (r) return r;
            Fd *f = &g_fd[fds[0]];
            f->type = F_SPAIR;
            f->idx2 = g_fd[fds[1]].idx;
            f->flags = O_RDWR_ | (fl & O_NONBLOCK_);
            memset(&g_fd[fds[1]], 0, sizeof(Fd));
            return fds[0];
        }
        case 250: return sys_epoll_create(0);
        case 357: return sys_epoll_create(a0);
        case 251: return sys_epoll_ctl(a0, a1, a2, P(a3));
        case 252: case 346: return sys_epoll_wait(a0, P(a1), a2, (int)a3 < 0 ? -1 : (i64)(int)a3 * 10000);
        case 441: return sys_epoll_wait(a0, P(a1), a2, a3 ? (i64)ts64_to_100ns(P(a3)) : -1);
        case 288: return a0 == 1 ? do_socketpair(a1, P(a3)) : -EAFNOSUPPORT_;
        case 316: return ino_init(0);
        case 360: return ino_init(a0);
        case 317: return ino_add(a0, P(a1), a2);
        case 318: return ino_rm(a0, a1);
        case 281:
            if (a0 == 16) return nl_socket(a1, a2);
            return a0 == 1 ? unix_socket(a1) : sys_socket(a0, a1, a2);
        case 282: return IS_UNIX(a0) ? unix_bind(a0, P(a1), a2) : sys_bind(a0, P(a1));
        case 283: return IS_UNIX(a0) ? unix_connect(a0, P(a1), a2) : sys_connect(a0, P(a1));
        case 284: return IS_UNIX(a0) ? unix_listen(a0) : sys_listen(a0, a1);
        case 285: return IS_UNIX(a0) ? unix_accept(a0, P(a1), P(a2), 0) : sys_accept(a0, P(a1), P(a2), 0);
        case 366: return IS_UNIX(a0) ? unix_accept(a0, P(a1), P(a2), a3) : sys_accept(a0, P(a1), P(a2), a3);
        case 286: case 287:
            if (IS_UNIX(a0) || (a0 < MAX_FD && g_fd[a0].type == F_SPAIR)) {
                if (a1 && *(u32 *)P(a2) >= 2) *(u16 *)P(a1) = 1;
                *(u32 *)P(a2) = 2;
                return 0;
            }
            return sys_sockname(a0, P(a1), P(a2), nr == 287);
        case 289:
            if (IS_UDG(a0)) return udg_send(a0, P(a1), a2, NULL, 0, a3);
            return a0 < MAX_FD && g_fd[a0].type == F_SPAIR ? sys_write(a0, P(a1), a2) : sock_send(a0, P(a1), a2, a3, NULL);
        case 290:
            if (IS_UDG(a0)) return udg_send(a0, P(a1), a2, P(a4), a5, a3);
            return sock_send(a0, P(a1), a2, a3, P(a4));
        case 291:
            if (IS_UDG(a0)) return udg_recv(a0, P(a1), a2, a3, NULL, NULL);
            return a0 < MAX_FD && g_fd[a0].type == F_SPAIR ? sys_read(a0, P(a1), a2) : sock_recv(a0, P(a1), a2, a3, NULL, NULL);
        case 292:
            if (IS_UDG(a0)) return udg_recv(a0, P(a1), a2, a3, P(a4), P(a5));
            return sock_recv(a0, P(a1), a2, a3, P(a4), P(a5));
        case 293: return (a0 < MAX_FD && g_fd[a0].type == F_SPAIR) || IS_UNIX(a0) ? 0 : sys_shutdown(a0, a1);
        case 294: return (a0 < MAX_FD && g_fd[a0].type == F_SPAIR) || IS_UNIX(a0) ? 0 : (sock_fd(a0) < 0 ? sock_fd(a0) : 0);
        case 295:
            if (IS_UNIX(a0) || (a0 < MAX_FD && g_fd[a0].type == F_SPAIR)) {
                u32 *val = P(a3), *len = P(a4);
                if (a1 == 1 && a2 == 17 && *len >= 12) {
                    val[0] = me->pid;
                    val[1] = val[2] = 0;
                    *len = 12;
                    return 0;
                }
                if (*len >= 4) { val[0] = a2 == 3 ? (IS_UDG(a0) ? 2 : 1) : 0; *len = 4; }
                return 0;
            }
            return sys_getsockopt(a0, a1, a2, P(a3), P(a4));
        case 296:
            if (a0 < MAX_FD && g_fd[a0].type == F_SPAIR) return spair_sendmsg(a0, P(a1), a2);
            if (IS_UDG(a0)) return udg_sendmsg(a0, P(a1), a2);
            return sys_sendmsg(a0, P(a1), a2);
        case 297:
            if (a0 < MAX_FD && g_fd[a0].type == F_SPAIR) return spair_recvmsg(a0, P(a1), a2);
            if (IS_UDG(a0)) return udg_recvmsg(a0, P(a1), a2);
            return sys_recvmsg(a0, P(a1), a2);
        case 365: case 417: {
            u32 *mm = P(a1);
            u32 done = 0;
            for (; done < a2; done++, mm += 8) {
                int r = sys_recvmsg(a0, (kmsghdr *)mm, done ? a3 | MSG_DONTWAIT_ : a3);
                if (r < 0) return done ? (int)done : r;
                mm[7] = (u32)r;
            }
            return (int)done;
        }
        case 374: {
            u32 *mm = P(a1);
            u32 done = 0;
            for (; done < a2; done++, mm += 8) {
                int r = sys_sendmsg(a0, (kmsghdr *)mm, a3);
                if (r < 0) return done ? (int)done : r;
                mm[7] = (u32)r;
            }
            return (int)done;
        }
        case 61: {

            return me->uid[1] ? -EPERM : 0;
        }
        case 266: case 267: {

            u32 *o = nr == 266 ? P(a2) : P(a2);
            memset(o, 0, a1);
            o[0] = 0x5346544E;
            o[1] = 4096;
            o[2] = 1u << 24; o[4] = 1u << 23; o[6] = 1u << 23;
            o[8] = 1u << 20; o[10] = 1u << 19;
            o[14] = 255;
            o[15] = 4096;
            return 0;
        }
        case 226: case 227: case 228: return -EOPNOTSUPP_;
        case 229: case 230: case 231: return -61;
        case 232: case 233: case 234: return 0;
        case 235: case 236: case 237: return -61;
        case 184: case 185: return 0;
        case 337: return -EPERM;
        case 373: case 225: return 0;
        case 186: {
            T *ct = CUR;
            u32 *ss = P(a0), *old = P(a1);
            int on = ct->alt_size && c->Sp - ct->alt_sp < ct->alt_size;
            if (old) {
                old[0] = ct->alt_sp;
                old[1] = ct->alt_size ? (on ? 1 : 0) : 2;
                old[2] = ct->alt_size;
            }
            if (ss) {
                if (on) return -EPERM;
                ct->alt_sp = ss[1] & 2 ? 0 : ss[0];
                ct->alt_size = ss[1] & 2 ? 0 : ss[2];
            }
            return 0;
        }
        case 122: {
            char *u = P(a0);
            memset(u, 0, 65 * 6);
            strcpy(u, "Linux");
            strcpy(u + 65, "lumia");
            strcpy(u + 130, "6.6.0-tx");
            strcpy(u + 195, "#1 SMP tx on Windows 10 Mobile");
            strcpy(u + 260, "armv7l");
            return 0;
        }
        case 74: case 75: return 0;
        case 78: {
            if (a0) {
                u64 t = now_100ns() - 116444736000000000ULL;
                long *tv = P(a0);
                tv[0] = (long)(t / 10000000);
                tv[1] = (long)((t % 10000000) / 10);
            }
            return 0;
        }
        case 263: {
            i64 s, n;
            fill_time(a0, &s, &n);
            long *ts = P(a1);
            ts[0] = (long)s;
            ts[1] = (long)n;
            return 0;
        }
        case 403: {
            i64 s, n;
            fill_time(a0, &s, &n);
            i64 *ts = P(a1);
            ts[0] = s;
            ts[1] = n;
            return 0;
        }
        case 264: { long *ts = P(a1); if (ts) { ts[0] = 0; ts[1] = 100; } return 0; }
        case 406: { i64 *ts = P(a1); if (ts) { ts[0] = 0; ts[1] = 100; } return 0; }
        case 162: return do_sleep(ts_to_100ns(P(a0)));
        case 265: case 407: {
            u64 want = nr == 407 ? ts64_to_100ns(P(a2)) : ts_to_100ns(P(a2));
            if (a1 & 1) {
                i64 s, n;
                fill_time(a0, &s, &n);
                u64 now = (u64)s * 10000000 + (u64)n / 100;
                want = want > now ? want - now : 0;
            }
            return do_sleep(want);
        }
        case 389:
            if (a0 == 0) return 8 | 16;
            NtFlushProcessWriteBuffers();
            return 0;
        case 43: {
            u32 *t = P(a0);
            u32 ticks = (u32)(mono_ns() / 10000000ULL);
            if (t) { t[0] = t[1] = t[2] = t[3] = 0; t[0] = ticks; }
            return (int)ticks;
        }
        case 116: {

            ULONG basic[11], perf[96];
            u32 *o = P(a0);
            memset(o, 0, 64);
            memset(basic, 0, sizeof(basic));
            memset(perf, 0, sizeof(perf));
            NtQuerySystemInformation(0, basic, 44, NULL);
            NtQuerySystemInformation(2, perf, sizeof(perf), NULL);
            o[0] = (u32)(mono_ns() / 1000000000ULL);
            o[4] = basic[3];
            o[5] = perf[11];
            o[8] = perf[13] > basic[3] ? perf[13] - basic[3] : 0;
            o[9] = perf[13] > perf[12] ? perf[13] - perf[12] : 0;
            if (o[9] > o[8]) o[9] = o[8];
            *(u16 *)(o + 10) = 1;
            o[13] = PG;
            return 0;
        }
        case 191: case 369: {
            u32 *out = nr == 191 ? P(a1) : P(a3);
            if (nr == 369 && !out) return 0;
            if (nr == 191) { out[0] = out[1] = 0xFFFFFFFF; if (a0 == 7) out[0] = out[1] = MAX_FD; if (a0 == 3) out[0] = 1 << 20; return 0; }
            u64 *o = (u64 *)out;
            o[0] = o[1] = ~0ULL;
            if (a1 == 7) o[0] = o[1] = MAX_FD;
            if (a1 == 3) o[0] = 1 << 20;
            return 0;
        }
        case 384: random_fill(P(a0), a1); return (int)a1;
        case 205: return 0;
        case 215: case 216: return 0;
        case 14: return sys_mknod(AT_FDCWD_, P(a0), a1);
        case 324: return sys_mknod(a0, P(a1), a2);
        case 270: return 0;
        case 77: memset(P(a1), 0, 72); return 0;
        case 219: memset(P(a2), 1, (a1 + PG - 1) / PG); return 0;
        case 452: return sys_chmod(a0, P(a1), a2);
        case 206: return me->uid[1] ? -EPERM : 0;
        case 208: return set_ids(me->uid, a0, a1, a2);
        case 210: return set_ids(me->gid, a0, a1, a2);
        case 203: return set_ids(me->uid, a0, a1, (a0 != ~0u || a1 != ~0u) ? (a1 != ~0u ? a1 : me->uid[1]) : ~0u);
        case 204: return set_ids(me->gid, a0, a1, (a0 != ~0u || a1 != ~0u) ? (a1 != ~0u ? a1 : me->gid[1]) : ~0u);
        case 213: return me->uid[1] ? set_ids(me->uid, ~0u, a0, ~0u) : set_ids(me->uid, a0, a0, a0);
        case 214: return me->uid[1] ? set_ids(me->gid, ~0u, a0, ~0u) : set_ids(me->gid, a0, a0, a0);
        case 209: memcpy(P(a0), &me->uid[0], 4); memcpy(P(a1), &me->uid[1], 4); memcpy(P(a2), &me->uid[2], 4); return 0;
        case 211: memcpy(P(a0), &me->gid[0], 4); memcpy(P(a1), &me->gid[1], 4); memcpy(P(a2), &me->gid[2], 4); return 0;
        case 172: return 0;
        case 96: return 20;
        case 97: return 0;
        case 158: NtYieldExecution(); return 0;
        case 154: case 156: return 0;
        case 155: if (a1) *(int *)P(a1) = 0; return 0;
        case 157: case 160: return 0;
        case 159: return 0;
        case 240: return sys_futex(a0, a1, a2, a3, a4, a5, 0);
        case 422: return sys_futex(a0, a1, a2, a3, a4, a5, 1);
        case 256: CUR->clear_tid = a0; return CUR->tid;
        case 338: return 0;
        case 0xF0005: ((NT_TIB *)NtCurrentTeb())->ArbitraryUserPointer = P(a0); return 0;
        case 0xF0002: NtFlushInstructionCache(SELF, P(a0), a1 - a0); return 0;
        case 242: { if (a1 >= 4) { *(u32 *)P(a2) = 0x3F; return 4; } return -EINVAL; }
    }
    trace("[%d] ENOSYS %u (%x %x %x)\n", me->pid, nr, a0, a1, a2);
    return -ENOSYS;
}

static void __cdecl continue_stub(void) {
    T *t = CUR;
    bkl_lock();
    deliver_signals(&t->cont_ctx);
    if (t->cont_ctx.Cpsr & 0x20) t->cont_ctx.Pc |= 1;
    bkl_unlock();
    NtContinue(&t->cont_ctx, FALSE);
}

static void redirect_context(CONTEXT *c) {
    T *t = CUR;
    t->cont_ctx = *c;
    c->Sp = (DWORD)(ULONG_PTR)t->host_top & ~7u;
    c->Pc = (DWORD)(ULONG_PTR)continue_stub | 1u;
    c->Cpsr = (c->Cpsr & ~0x0600FC00u) | 0x20;
}

static LONG CALLBACK guest_veh(EXCEPTION_POINTERS *e) {
    CONTEXT *c = e->ContextRecord;
    DWORD code = e->ExceptionRecord->ExceptionCode;
    if (!me) return EXCEPTION_CONTINUE_SEARCH;
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION) {

        u32 at = c->Pc & ~1u, word;
        if (c->Cpsr & 0x20) word = ((u32)*(u16 *)(ULONG_PTR)at << 16) | *(u16 *)(ULONG_PTR)(at + 2);
        else word = (*(u32 *)(ULONG_PTR)at & 0x0FFFFFFFu) | 0xE0000000u;
        if (cp15_barrier_arm(word)) {
            MemoryBarrier();
            c->Pc += 4;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION && !(c->Cpsr & 0x20) && *(u32 *)(ULONG_PTR)(c->Pc & ~1u) == FLUSH_TRAP_ARM) {
        NtFlushInstructionCache(SELF, (PVOID)(ULONG_PTR)c->R0, c->R1 - c->R0);
        c->R0 = 0;
        c->Pc += 4;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION && !(c->Cpsr & 0x20) && (*(u32 *)(ULONG_PTR)(c->Pc & ~1u) & ~15u) == TLS_TRAP_ARM) {
        (&c->R0)[*(u32 *)(ULONG_PTR)(c->Pc & ~1u) & 15] = (u32)(ULONG_PTR)((NT_TIB *)NtCurrentTeb())->ArbitraryUserPointer;
        c->Pc += 4;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION && *(u16 *)(ULONG_PTR)(c->Pc & ~1u) == TLS_TRAP) {
        u16 h2 = *(u16 *)(ULONG_PTR)((c->Pc & ~1u) + 2);
        (&c->R0)[h2 >> 12] = (u32)(ULONG_PTR)((NT_TIB *)NtCurrentTeb())->ArbitraryUserPointer;
        c->Pc += 4;
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    if ((c->Pc & ~1u) == ((DWORD)(ULONG_PTR)continue_stub & ~1u)) NtContinue(c, FALSE);
    if (code != EXCEPTION_ILLEGAL_INSTRUCTION && g_trace) {
        struct { PVOID base, alloc; ULONG aprot; SIZE_T size; ULONG state, prot, type; } mi;
        memset(&mi, 0, sizeof(mi));
        u32 where = (u32)e->ExceptionRecord->ExceptionInformation[1];
        NtQueryVirtualMemory(SELF, (PVOID)(ULONG_PTR)where, 0, &mi, sizeof(mi), NULL);
        trace("[%d] exception %08x pc=%08x lr=%08x cpsr=%08x op=%u addr=%08x region=%08x+%x state=%x prot=%x used=%d\n", me->pid, code, c->Pc, c->Lr, c->Cpsr,
              (u32)e->ExceptionRecord->ExceptionInformation[0], where, (u32)(ULONG_PTR)mi.base, (u32)mi.size, mi.state, mi.prot,
              in_arena(where, 1) ? g_page_used[(where - (u32)(ULONG_PTR)g_arena) / PG] : -1);
    }
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION && !(c->Cpsr & 0x20) && *(u32 *)(ULONG_PTR)(c->Pc & ~1u) == SYS_TRAP_ARM_SW) {

        c->Pc = ((u32 *)(ULONG_PTR)(c->Pc & ~1u))[1];
        c->Sp = CUR->guest_sp;
        goto arm_syscall;
    }
    if (code == EXCEPTION_ILLEGAL_INSTRUCTION && ((c->Cpsr & 0x20) ? *(u16 *)(ULONG_PTR)(c->Pc & ~1u) == TRAP_INSN : *(u32 *)(ULONG_PTR)(c->Pc & ~1u) == SYS_TRAP_ARM)) {
arm_syscall:;
        int advance = 1;
        u32 trap_len = TRAP_LEN(c);
        bkl_lock();
        timers_poll();
        u32 r0 = c->R0;
        u32 trap_pc = c->Pc & ~1u;
        u32 nr = c->R7;
        if (g_trace) trace("[%d] > %u(%x,%x,%x)\n", me->pid, nr, r0, c->R1, c->R2);
        int r = do_syscall(c, &advance);
        if (g_trace) trace("[%d] < %d\n", me->pid, r);
        if (advance) {
            if (r == -ERESTART_) {

                u64 ready = (me->pending | CUR->pending) & ~CUR->blocked;
                int sig = 1;
                while (ready && !(ready & SIGBIT(sig))) sig++;
                if (ready && g_sa[sig].handler > 1 && (g_sa[sig].flags & SA_RESTART_)) {
                    c->R0 = r0;
                } else if (ready && (g_sa[sig].handler == 0 || g_sa[sig].handler == 1)) {
                    c->R0 = r0;
                } else {
                    c->R0 = (u32)-EINTR;
                    c->Pc += trap_len;
                }
            } else {
                c->R0 = (u32)r;
                c->Pc += trap_len;
            }
        }

        u32 pc = c->Pc & ~1u;
        if (pc == ((DWORD)(ULONG_PTR)fork_stub & ~1u)) {
            bkl_unlock();
            NtContinue(c, FALSE);
        }
        if (signal_ready() || me->stopped || (pc != trap_pc && pc != trap_pc + trap_len)) {
            redirect_context(c);
            bkl_unlock();
            NtContinue(c, FALSE);
        }
        bkl_unlock();
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_ILLEGAL_INSTRUCTION || code == EXCEPTION_DATATYPE_MISALIGNMENT ||
        code == EXCEPTION_INT_DIVIDE_BY_ZERO || code == EXCEPTION_PRIV_INSTRUCTION || code == EXCEPTION_IN_PAGE_ERROR) {
        int sig = code == EXCEPTION_ACCESS_VIOLATION ? SIGSEGV_ : SIGILL_;
        trace("[%d] fault %08x pc=%08x lr=%08x sp=%08x addr=%08x r7=%u cpsr=%08x exe=%s\n", me ? me->pid : -1, code, c->Pc, c->Lr, c->Sp,
              (u32)e->ExceptionRecord->ExceptionInformation[1], c->R7, c->Cpsr, g_exe);
        if (g_trace && in_arena(c->Sp, 512)) {
            u32 *sw = (u32 *)(ULONG_PTR)c->Sp;
            trace("  regs r0=%08x r1=%08x r2=%08x r3=%08x r4=%08x r5=%08x r6=%08x r8=%08x r9=%08x r10=%08x r11=%08x r12=%08x\n", c->R0, c->R1, c->R2, c->R3, c->R4, c->R5, c->R6, c->R8, c->R9, c->R10, c->R11, c->R12);
            if (in_arena(c->Pc & ~1u, 4)) {
                u32 *cw = (u32 *)(ULONG_PTR)((c->Pc & ~3u) - 0x280);
                trace("  code@%08x", (u32)(ULONG_PTR)cw);
                for (int i = 0; i < 190; i++) trace(" %08x", cw[i]);
                trace("\n");
            }
            trace("  stack");
            for (int i = 0; i < 96; i++) trace(" %08x", sw[i]);
            trace("\n");
        }
        if (g_bkl_owner != NtCurrentTeb()) bkl_lock();
        if (g_sa[sig].handler > 1) {
            T *t = CUR;
            lock_();
            t->pending |= SIGBIT(sig);
            t->blocked &= ~SIGBIT(sig);
            t->fault_addr = code == EXCEPTION_ACCESS_VIOLATION ? (u32)e->ExceptionRecord->ExceptionInformation[1] | (e->ExceptionRecord->ExceptionInformation[1] ? 0 : 1) : 0;
            proc_blocked_update();
            unlock_();
            redirect_context(c);
            bkl_unlock();
            NtContinue(c, FALSE);
        }
        do_exit(sig);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static void tty_setup(Tty *t) {
    memset(&t->t, 0, sizeof(t->t));
    t->t.c_iflag = ICRNL_;
    t->t.c_oflag = OPOST_ | ONLCR_;
    t->t.c_cflag = 0xBF;
    t->t.c_lflag = ISIG_ | ICANON_ | ECHO_ | ECHOE_;
    t->t.c_cc[VINTR_] = 3;
    t->t.c_cc[VQUIT_] = 28;
    t->t.c_cc[VERASE_] = 127;
    t->t.c_cc[VKILL_] = 21;
    t->t.c_cc[VEOF_] = 4;
    t->t.c_cc[VSUSP_] = 26;
    t->t.c_cc[6] = 1;
    t->rows = 40;
    t->cols = 100;
}

static void tty_init(void) {
    tty_setup(&g->ttys[0]);
    g->ttys[0].fg_pgrp = 1;
    g->ttys[0].sid = 1;
}

static int shm_init(void) {
    HANDLE sec;
    LARGE_INTEGER size;
    size.QuadPart = sizeof(Shm);
    OBJECT_ATTRIBUTES oa;
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(oa);
    oa.Attributes = OBJ_INHERIT_;
    NTSTATUS st = NtCreateSection(&sec, SECTION_ALL_ACCESS, &oa, &size, PAGE_READWRITE, SEC_COMMIT, NULL);
    if (!NT_OK(st)) { printf("tx: section %08x\n", st); return -1; }
    PVOID base = NULL;
    SIZE_T view = 0;
    st = NtMapViewOfSection(sec, SELF, &base, 0, 0, NULL, &view, 1  , 0, PAGE_READWRITE);
    if (!NT_OK(st)) { printf("tx: map %08x\n", st); return -1; }
    g = base;
    g_shm_sec = sec;
    memset(g, 0, sizeof(*g));
    g->next_pid = 2;
    tty_init();
    for (int i = 0; i < MAX_PROC; i++) {
        st = NtCreateEvent(&g_events[i], EVENT_ALL_ACCESS, &oa, 1  , FALSE);
        if (!NT_OK(st)) { printf("tx: event %08x\n", st); return -1; }
        g->host_events[i] = (u32)(ULONG_PTR)g_events[i];
    }
    return 0;
}

static int shm_attach(HANDLE sec) {
    PVOID base = NULL;
    SIZE_T view = 0;
    if (!NT_OK(NtMapViewOfSection(sec, SELF, &base, 0, 0, NULL, &view, 1, 0, PAGE_READWRITE))) return -1;
    g = base;
    g_shm_sec = sec;
    for (int i = 0; i < MAX_PROC; i++) g_events[i] = (HANDLE)(ULONG_PTR)g->host_events[i];
    g_ui_event = (HANDLE)(ULONG_PTR)g->host_ui;
    wcsncpy(g_root_dos, g->host_root, 299);
    wcsncpy(g_musl_dos, g->host_musl, 299);
    return 0;
}

static void set_stack_limits(void) {

    NT_TIB *tib = (NT_TIB *)NtCurrentTeb();
    tib->StackBase = (PVOID)0x7FFE0000;
    tib->StackLimit = (PVOID)0x00010000;
}

static void start_init(char **argv, int tty) {
    set_stack_limits();
    {
        T *t = &g_threads[0];
        memset(g_threads, 0, sizeof(g_threads));
        t->used = 1;
        t->tid = me->pid;
        t->host_top = (u8 *)(((ULONG_PTR)(g_host_stack + sizeof(g_host_stack)) - 64) & ~7u);
        ((NT_TIB *)NtCurrentTeb())->FiberData = t;
    }
    static char *envp[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", "HOME=/root", "USER=root", "LOGNAME=root", "NMAP_UNPRIVILEGED=1",
                           "SHELL=/bin/sh", "TERM=xterm-256color", "LANG=C.UTF-8", "TMPDIR=/tmp", NULL};
    lock_();
    for (int i = 0; i < 3; i++) {
        g_fd[i].type = F_TTY;
        g_fd[i].idx = tty;
        g_fd[i].flags = O_RDWR_;
        _snprintf(g_fdpath[i], 255, "/dev/pts/%d", tty);
        ref_count_add(&g_fd[i]);
        ref_mirror_add(&g_fd[i]);
    }
    if (tty) g->ttys[tty].slave_opened = 1;
    unlock_();
    int r = sys_execve(argv[0], argv, envp);
    trace("init exec failed %d\n", r);
    do_exit(127 << 8);
}

static void reap_dead(void) {
    exit_log_poll();

    for (int i = 0; i < MAX_PROC; i++) {
        Proc *p = &g->procs[i];
        if (p->state != 1 || !p->nt_pid) continue;
        HANDLE h = NULL;
        CLIENT_ID id;
        OBJECT_ATTRIBUTES oa;
        memset(&oa, 0, sizeof(oa));
        oa.Length = sizeof(oa);
        id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
        id.UniqueThread = NULL;
        NTSTATUS st = NtOpenProcess(&h, SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, &oa, &id);
        int dead = !NT_OK(st);
        DWORD code = 0;
        if (!dead) {
            LARGE_INTEGER z;
            z.QuadPart = 0;
            if (NtWaitForSingleObject(h, FALSE, &z) == 0) {
                dead = 1;
                GetExitCodeProcess(h, &code);
            }
            NtClose(h);
        }
        if (!dead) continue;
        lock_();
        if (p->state == 1) {
            trace("reaper: pid %d nt %u died without exit, open=%08x nt code %08x\n", p->pid, p->nt_pid, st, code);
            release_refs_locked(p);
            p->status = SIGSEGV_;
            for (int j = 0; j < MAX_PROC; j++)
                if (g->procs[j].state && g->procs[j].ppid == p->pid) reparent_locked(&g->procs[j]);
            p->state = 2;
            Proc *par = proc_by_pid(p->ppid);
            if (par && p->pid != 1) send_signal_locked(par, SIGCHLD_);
            proc_finished_locked(p, SIGSEGV_);
        }
        unlock_();
        wake_all();
    }
}

#include "net_master.inc"

static u32 tty_drain_t(Tty *t, u8 *buf, u32 max) {
    u32 n = 0;
    lock_();
    while (n < max && t->otail != t->ohead) {
        buf[n++] = t->out[t->otail];
        t->otail = (t->otail + 1) % TTY_OUT_SZ;
    }
    unlock_();
    return n;
}

static u32 tty_drain(u8 *buf, u32 max) { return tty_drain_t(&g->ttys[0], buf, max); }

static int spawn_session(char **argv, int tty) {
    HANDLE hp = NULL, ht = NULL;
    lock_();
    int slot = 0;
    if (tty) {
        slot = slot_alloc_locked();
        if (slot < 0) {
            unlock_();
            return -1;
        }
    }
    Proc *p = &g->procs[slot];
    int pid = tty ? g->next_pid++ : 1;
    memset(p, 0, sizeof(*p));
    p->pid = pid;
    p->ppid = 0;
    p->pgid = pid;
    p->sid = pid;
    p->ctty = tty;
    p->state = 1;
    g->ttys[tty].fg_pgrp = pid;
    g->ttys[tty].sid = pid;
    g->ttys[tty].leader_pid = tty ? pid : 0;
    unlock_();
    NTSTATUS st = nt_fork(&hp, &ht);
    if (st == CLONE_CHILD) {
        g_self = slot;
        me = p;
        start_init(argv, tty);
    }
    if (!NT_OK(st)) {
        printf("tx: clone %08x\n", st);
        return -1;
    }
    p->nt_pid = GetProcessId(hp);
    NtResumeThread(ht, NULL);
    NtClose(ht);
    exit_log_add(hp, 1);
    return 0;
}

static int run_command(char *cmd) {
    static char *argv[4];
    static u8 buf[4096];
    DWORD w;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    WSADATA wd;
    WSAStartup(MAKEWORD(2, 2), &wd);
    write_resolv_conf();
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = cmd;
    argv[3] = NULL;
    if (spawn_session(argv, 0)) return 1;
    int tick = 0;
    me = NULL;
    for (;;) {
        u32 n = tty_drain(buf, sizeof(buf));
        if (n) WriteFile(out, buf, n, &w, NULL);
        if (g->done && !n) break;
        int socks = net_pump(n ? 0 : 2);
        if (!n && !socks) {
            LARGE_INTEGER t;
            t.QuadPart = -200000;
            NtWaitForSingleObject(g_events[MAX_PROC - 1], FALSE, &t);
        }
        if (++tick % (socks ? 50 : 5) == 0) reap_dead();
    }
    return (g->exit_status >> 8) & 0xFF ? (g->exit_status >> 8) & 0xFF : g->exit_status & 0x7F;
}

static void kill_all(void) {
    for (int i = 0; i < MAX_PROC; i++) {
        Proc *p = &g->procs[i];
        if (p->state != 1 || !p->nt_pid) continue;
        HANDLE h = NULL;
        CLIENT_ID id;
        OBJECT_ATTRIBUTES oa;
        memset(&oa, 0, sizeof(oa));
        oa.Length = sizeof(oa);
        id.UniqueProcess = (HANDLE)(ULONG_PTR)p->nt_pid;
        id.UniqueThread = NULL;
        if (NT_OK(NtOpenProcess(&h, PROCESS_TERMINATE, &oa, &id))) {
            NtTerminateProcess(h, 9);
            NtClose(h);
        }
        p->state = 0;
    }
}

static int serve_tcp(int port) {
    static char *argv[] = {"/bin/sh", "-l", NULL};
    static const u8 nego[] = {255, 251, 1, 255, 251, 3, 255, 253, 3, 255, 253, 31};
    static u8 buf[8192];
    WSADATA wd;
    WSAStartup(MAKEWORD(2, 2), &wd);
    write_resolv_conf();
    SOCKET ls = socket(AF_INET, SOCK_STREAM, 0);
    SetHandleInformation((HANDLE)ls, HANDLE_FLAG_INHERIT, 0);
    int one = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((u_short)port);
    if (bind(ls, (struct sockaddr *)&a, sizeof(a)) || listen(ls, 1)) {
        printf("tx: can't listen on %d: %d\n", port, WSAGetLastError());
        return 1;
    }
    printf("tx: listening on %d\n", port);
    fflush(stdout);
    for (;;) {
        SOCKET c = accept(ls, NULL, NULL);
        if (c == INVALID_SOCKET) continue;
        setsockopt(c, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));
        SetHandleInformation((HANDLE)c, HANDLE_FLAG_INHERIT, 0);
        net_reset();

        g->done = 0;
        memset(g->procs, 0, sizeof(g->procs));
        memset(g->pipes, 0, sizeof(g->pipes));
        memset(g->ttys, 0, sizeof(g->ttys));
        memset(g->unixs, 0, sizeof(g->unixs));
        memset(g->fifos, 0, sizeof(g->fifos));
        g->next_pid = 2;
        tty_init();
        send(c, (const char *)nego, sizeof(nego), 0);
        if (spawn_session(argv, 0)) { closesocket(c); continue; }
        me = NULL;
        int iac = 0, sb = 0, tick = 0, naws = 0;
        u8 nawsb[4];
        for (;;) {
            u32 n = tty_drain(buf, sizeof(buf));
            if (n) send(c, (const char *)buf, n, 0);
            if (g->done && !n) break;
            fd_set rd;
            FD_ZERO(&rd);
            FD_SET(c, &rd);
            int socks = net_pump(0);
            struct timeval tv = {0, n ? 0 : (socks ? 2000 : 10000)};
            int s = select(0, &rd, NULL, NULL, &tv);
            if (s > 0) {
                int got = recv(c, (char *)buf, sizeof(buf), 0);
                if (got <= 0) break;
                for (int i = 0; i < got; i++) {
                    u8 ch = buf[i];
                    if (iac == 1) {
                        if (ch == 250) { sb = 1; iac = 0; naws = 0; }
                        else if (ch == 240) { sb = 0; iac = 0; }
                        else if (ch >= 251 && ch <= 254) iac = 2;
                        else if (ch == 255) { iac = 0; if (!sb) tty_input(ch); }
                        else iac = 0;
                    } else if (iac == 2) {
                        iac = 0;
                    } else if (ch == 255) {
                        iac = 1;
                    } else if (sb) {
                        if (sb == 1) { sb = ch == 31 ? 2 : 3; }
                        else if (sb == 2 && naws < 4) {
                            nawsb[naws++] = ch;
                            if (naws == 4) {
                                g->ttys[0].cols = (u16)((nawsb[0] << 8) | nawsb[1]);
                                g->ttys[0].rows = (u16)((nawsb[2] << 8) | nawsb[3]);
                                lock_();
                                signal_pgrp_locked(g->ttys[0].fg_pgrp, SIGWINCH_);
                                unlock_();
                            }
                        }
                    } else {
                        if (ch == 0) continue;
                        tty_input(ch);
                    }
                }
            }
            if (++tick % 20 == 0) reap_dead();
        }
        kill_all();
        closesocket(c);
    }
}

static void make_sys_dirs(void) {
    static const char *const sys_dirs[] = {"/sys", "/sys/devices", "/sys/devices/system", "/sys/devices/system/cpu"};
    char cpu[40];
    for (int i = 0; i < 4; i++) mkdir_abs(sys_dirs[i], 0755, 0, 0);

    static const char *const dev_files[] = {"/dev/null", "/dev/zero", "/dev/random", "/dev/urandom", "/dev/tty", "/dev/console", "/dev/ptmx"};
    for (int i = 0; i < 7; i++) {
        HANDLE h;
        if (NT_OK(nt_open(dev_files[i], GENERIC_WRITE | FILE_WRITE_EA, FILE_CREATE_, FO_NONDIR, FILE_ATTRIBUTE_NORMAL, &h))) {
            TxMeta m = {S_IFCHR_ | 0666, 0, 0};
            meta_set(h, &m);
            NtClose(h);
        }
    }
    mkdir_abs("/dev/shm", 01777, 0, 0);
    mkdir_abs("/root", 0700, 0, 0);

    symlink_abs("/usr/bin/clang", "/usr/local/bin/gcc", 0, 0);
    symlink_abs("/usr/bin/clang", "/usr/local/bin/cc", 0, 0);
    symlink_abs("/usr/bin/clang++", "/usr/local/bin/g++", 0, 0);
    symlink_abs("/usr/bin/clang++", "/usr/local/bin/c++", 0, 0);
    symlink_abs("/proc/self/fd", "/dev/fd", 0, 0);
    symlink_abs("/proc/self/fd/0", "/dev/stdin", 0, 0);
    symlink_abs("/proc/self/fd/1", "/dev/stdout", 0, 0);
    symlink_abs("/proc/self/fd/2", "/dev/stderr", 0, 0);
    for (int i = 0; i < 6; i++) {
        _snprintf(cpu, sizeof(cpu), "/sys/devices/system/cpu/cpu%d", i);
        mkdir_abs(cpu, 0755, 0, 0);
    }
}

static u32 octal(const char *s, int n) {
    u32 v = 0;
    for (int i = 0; i < n && s[i] >= '0' && s[i] <= '7'; i++) v = v * 8 + (s[i] - '0');
    return v;
}

static int import_tar(const wchar_t *path) {
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) { printf("tx: can't open tar: %lu\n", GetLastError()); return 1; }
    static wchar_t w[400];
    CreateDirectoryW(g_root_dos, NULL);
    swprintf(w, 400, L"%s\\root", g_root_dos);
    CreateDirectoryW(w, NULL);
    swprintf(w, 400, L"%s\\trash", g_root_dos);
    CreateDirectoryW(w, NULL);
    static char hdr[512], name[512], data[65536];
    int files = 0, dirs = 0, links = 0, errs = 0;
    DWORD got;
    while (ReadFile(f, hdr, 512, &got, NULL) && got == 512 && hdr[0]) {
        u32 size = octal(hdr + 124, 12), mode = octal(hdr + 100, 8), uid = octal(hdr + 108, 8), gid = octal(hdr + 116, 8);
        char type = hdr[156];
        if (hdr[345] && !memcmp(hdr + 257, "ustar", 5)) {
            _snprintf(name, sizeof(name), "%.155s/%.100s", hdr + 345, hdr);
        } else {
            _snprintf(name, sizeof(name), "%.100s", hdr);
        }
        char *abs = name;
        while (abs[0] == '.' && abs[1] == '/') abs += 1;
        if (abs[0] != '/') { memmove(name + 1, abs, strlen(abs) + 1); name[0] = '/'; abs = name; }
        size_t l = strlen(abs);
        while (l > 1 && abs[l - 1] == '/') abs[--l] = 0;
        u32 blocks = (size + 511) / 512;
        int r = 0;
        if (type == '5') {
            if (l > 1) {
                r = mkdir_abs(abs, mode, uid, gid);
                if (r == -EEXIST) r = 0;
            }
            dirs++;
        } else if (type == '2') {
            char target[101];
            memcpy(target, hdr + 157, 100);
            target[100] = 0;
            unlink_abs(abs, 0);
            r = symlink_abs(target, abs, uid, gid);
            links++;
        } else if (type == '0' || type == 0) {
            HANDLE h;
            IO_STATUS_BLOCK io;
            NTSTATUS st = nt_open(abs, GENERIC_WRITE | FILE_WRITE_EA, FILE_OVERWRITE_IF_, FO_NONDIR, FILE_ATTRIBUTE_NORMAL, &h);
            if (!NT_OK(st)) {
                r = -errno_of(st);
            } else {
                u32 left = size;
                while (left) {
                    u32 chunk = left < sizeof(data) ? left : sizeof(data);
                    u32 aligned = (chunk + 511) & ~511u;
                    ReadFile(f, data, aligned, &got, NULL);
                    NtWriteFile(h, NULL, NULL, NULL, &io, data, chunk, NULL, NULL);
                    left -= chunk;
                }
                TxMeta m = {S_IFREG_ | (mode & 07777), uid, gid};
                meta_set(h, &m);
                NtClose(h);
                blocks = 0;
            }
            files++;
        }
        if (r) { printf("tx: %s: error %d\n", abs, r); errs++; }
        if (blocks) SetFilePointer(f, blocks * 512, NULL, FILE_CURRENT);
    }
    CloseHandle(f);
    printf("tx: imported %d files, %d dirs, %d symlinks, %d errors\n", files, dirs, links, errs);
    return errs ? 1 : 0;
}

#ifdef TX_DLL
#include "host_dll.inc"
#else
static int host_serve(HANDLE sec) {
    static char *argv[4];
    static char cmd[4096];
    if (shm_attach(sec)) return 1;
    low_reserve();
    if (musl_prepare()) return 1;
    arena_init();
    if (!g_arena) return 1;
    RtlAddVectoredExceptionHandler(1, guest_veh);
    HANDLE w[2];
    w[0] = (HANDLE)(ULONG_PTR)g->host_req;
    w[1] = OpenProcess(SYNCHRONIZE, FALSE, g->host_owner);
    if (!w[1]) return 1;
    for (;;) {
        DWORD r = WaitForMultipleObjects(2, w, FALSE, 1000);
        if (r == WAIT_OBJECT_0 + 1) return 0;
        int tty = g->req_tty;
        if (!tty || g->req_done) continue;
        argv[0] = "/bin/sh";
        if (g->req_has_cmd) {
            memcpy(cmd, g->req_cmd, sizeof(cmd));
            cmd[sizeof(cmd) - 1] = 0;
            argv[1] = "-c";
            argv[2] = cmd;
            argv[3] = NULL;
        } else {
            argv[1] = "-l";
            argv[2] = NULL;
        }
        g->req_done = spawn_session(argv, tty) ? 2 : 1;
        NtSetEvent(g_ui_event, NULL);
    }
}

static int relaunch_clear(void) {
    static wchar_t self[400], cmd[8192];
    GetModuleFileNameW(NULL, self, 400);
    const wchar_t *rest = GetCommandLineW();
    if (*rest == L'"') {
        rest++;
        while (*rest && *rest != L'"') rest++;
        if (*rest) rest++;
    } else
        while (*rest && *rest != L' ' && *rest != L'\t') rest++;
    if (swprintf(cmd, 8192, L"\"%s\" -R%s", self, rest) < 0) return -1;
    HANDLE job = CreateJobObjectW(NULL, NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION li;
    memset(&li, 0, sizeof(li));
    li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (job) SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof(li));
    HANDLE hp = NULL, rd = NULL, wr = NULL;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    if (!CreatePipe(&rd, &wr, &sa, 0)) return -1;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    if (!NT_OK(nt_spawn_clear(self, cmd, wr, &hp))) {
        NtClose(rd);
        NtClose(wr);
        return -1;
    }
    NtClose(wr);
    if (job) AssignProcessToJobObject(job, hp);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 buf[4096];
    DWORD n, w;
    while (ReadFile(rd, buf, sizeof(buf), &n, NULL) && n) WriteFile(out, buf, n, &w, NULL);
    NtWaitForSingleObject(hp, FALSE, NULL);
    DWORD code = 1;
    GetExitCodeProcess(hp, &code);
    return (int)code;
}

int main(int argc, char **argv) {
    int a = 1;
    setvbuf(stdout, NULL, _IONBF, 0);
    if (a + 1 < argc && !strcmp(argv[a], "-H")) return host_serve((HANDLE)(ULONG_PTR)strtoul(argv[a + 1], NULL, 16));
    if (a < argc && !strcmp(argv[a], "-R")) a++;
    else if (!low_is_clear()) {
        int r = relaunch_clear();
        if (r >= 0) return r;
    }
    if (a < argc && !strcmp(argv[a], "-t")) { g_trace = 1; a++; }
    if (a >= argc) {
        printf("usage: tx [-t] import <tar> | -c <command> | -L <port>\n");
        return 2;
    }
    if (!strcmp(argv[a], "import")) {
        static wchar_t tp[300];
        if (a + 1 >= argc) return 2;
        MultiByteToWideChar(CP_ACP, 0, argv[a + 1], -1, tp, 300);
        return import_tar(tp);
    }
    if (g_trace) {
        static wchar_t tp[400];
        int n = swprintf(tp, 400, L"\\??\\%s\\trace.log", g_root_dos);
        UNICODE_STRING us;
        OBJECT_ATTRIBUTES oa;
        IO_STATUS_BLOCK io;
        init_oa(&oa, &us, tp, n * 2, OBJ_INHERIT_);
        NtCreateFile(&g_trace_h, FILE_APPEND_DATA | SYNCHRONIZE, &oa, &io, NULL, 0, SHARE_ALL, FILE_OVERWRITE_IF_, FO_SYNC | FO_NONDIR, NULL, 0);
    }
    low_reserve();
    if (musl_prepare() || shm_init()) return 1;
    make_sys_dirs();
    arena_init();
    if (!g_arena) { printf("tx: no address space\n"); return 1; }
    RtlAddVectoredExceptionHandler(1, guest_veh);
    if (!strcmp(argv[a], "-c")) return a + 1 < argc ? run_command(argv[a + 1]) : 2;
    if (!strcmp(argv[a], "-L")) return a + 1 < argc ? serve_tcp(atoi(argv[a + 1])) : 2;
    return 2;
}
#endif
