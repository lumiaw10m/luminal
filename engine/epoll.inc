#define MAX_EPOLL 8
#define EPOLL_ITEMS 64
#define EPOLLONESHOT_ 0x40000000u

typedef struct { u32 events; u32 pad; u64 data; } kepoll_event;
typedef struct { int fd; u32 events; u64 data; } EpItem;
typedef struct { int used, n; EpItem items[EPOLL_ITEMS]; } Epoll;

static Epoll g_ep[MAX_EPOLL];

static void g_ep_release(int i) { g_ep[i].used = 0; }

static int sys_epoll_create(int flags) {
    int i, n = fd_alloc(0);
    if (n < 0) return n;
    for (i = 0; i < MAX_EPOLL; i++)
        if (!g_ep[i].used) break;
    if (i == MAX_EPOLL) return -EMFILE;
    g_ep[i].used = 1;
    g_ep[i].n = 0;
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = F_EPOLL;
    g_fd[n].idx = i;
    g_fd[n].flags = O_RDWR_;
    g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
    return n;
}

static int sys_epoll_ctl(int epfd, int op, int fd, const kepoll_event *ev) {
    if (epfd < 0 || epfd >= MAX_FD || g_fd[epfd].type != F_EPOLL) return -EINVAL;
    if (fd < 0 || fd >= MAX_FD || !g_fd[fd].type) return -EBADF;
    Epoll *e = &g_ep[g_fd[epfd].idx];
    int i;
    for (i = 0; i < e->n; i++)
        if (e->items[i].fd == fd) break;
    if (op == 1) {
        if (i < e->n) return -EEXIST;
        if (e->n == EPOLL_ITEMS) return -ENOMEM;
        e->items[e->n].fd = fd;
        e->items[e->n].events = ev->events;
        e->items[e->n].data = ev->data;
        e->n++;
        return 0;
    }
    if (i == e->n) return -ENOENT;
    if (op == 2) {
        e->items[i] = e->items[--e->n];
        return 0;
    }
    if (op == 3) {
        e->items[i].events = ev->events;
        e->items[i].data = ev->data;
        return 0;
    }
    return -EINVAL;
}

static int sys_epoll_wait(int epfd, kepoll_event *out, int max, i64 timeout_100ns) {
    if (epfd < 0 || epfd >= MAX_FD || g_fd[epfd].type != F_EPOLL) return -EINVAL;
    if (max <= 0) return -EINVAL;
    Epoll *e = &g_ep[g_fd[epfd].idx];
    kpollfd pf[EPOLL_ITEMS];
    int map[EPOLL_ITEMS], n = 0;

    for (int i = e->n - 1; i >= 0; i--)
        if (!g_fd[e->items[i].fd].type) e->items[i] = e->items[--e->n];
    for (int i = 0; i < e->n; i++) {
        pf[n].fd = e->items[i].fd;
        pf[n].events = (short)(e->items[i].events & 0x3F);
        pf[n].revents = 0;
        if (e->items[i].events & 0x3F) map[n++] = i;
    }
    int r = do_poll(pf, n, timeout_100ns);
    if (r <= 0) return r;
    int got = 0;
    for (int i = 0; i < n && got < max; i++) {
        if (!pf[i].revents) continue;
        EpItem *it = &e->items[map[i]];
        out[got].events = (u16)pf[i].revents;
        out[got].pad = 0;
        out[got].data = it->data;
        got++;
        if (it->events & EPOLLONESHOT_) it->events = 0;
    }
    return got;
}
