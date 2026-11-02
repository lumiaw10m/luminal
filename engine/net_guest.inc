#define MSG_PEEK_ 2
#define MSG_DONTWAIT_ 0x40
#define MSG_WAITALL_ 0x100
#define MSG_NOSIGNAL_ 0x4000
#define EAFNOSUPPORT_ 97
#define EDESTADDRREQ_ 89
#define ENOTSOCK_ 88
#define EOPNOTSUPP_ 95
#define ENOTCONN_ 107
#define EISCONN_ 106
#define EINPROGRESS_ 115
#define EINVAL_LISTEN EINVAL

typedef struct { u16 family, port; u32 addr; u8 zero[8]; } ksockaddr_in;

static u32 ring_used(u32 head, u32 tail, u32 sz) { return (head + sz - tail) % sz; }
static u32 ring_free(u32 head, u32 tail, u32 sz) { return (tail + sz - head - 1) % sz; }

static void ring_put(u8 *ring, u32 sz, volatile u32 *head, const void *src, u32 n) {
    const u8 *s = src;
    u32 h = *head;
    for (u32 i = 0; i < n; i++) ring[(h + i) % sz] = s[i];
    *head = (h + n) % sz;
}

static void ring_get(const u8 *ring, u32 sz, u32 tail, void *dst, u32 n) {
    u8 *d = dst;
    for (u32 i = 0; i < n; i++) d[i] = ring[(tail + i) % sz];
}

static void wake_master(void) { NtSetEvent(g_events[MAX_PROC - 1], NULL); }

static int sock_fd(int fd) {
    if (fd < 0 || fd >= MAX_FD || !g_fd[fd].type) return -EBADF;
    if (g_fd[fd].type != F_SOCK) return -ENOTSOCK_;
    return g_fd[fd].idx;
}

static int sys_socket(int family, int type, int proto) {
    int base = type & 0xF;
    if (family != 2) return -EAFNOSUPPORT_;
    if (base != 1 && base != 2 && base != 3) return -EOPNOTSUPP_;
    if (base == 3 && proto != 1) return -93;
    int n = fd_alloc(0);
    if (n < 0) return n;
    lock_();
    int i;
    for (i = 0; i < MAX_SOCK; i++)
        if (!g->socks[i].state) break;
    if (i == MAX_SOCK) {
        unlock_();
        return -ENFILE;
    }
    Sock *s = &g->socks[i];
    memset(s, 0, sizeof(*s));
    s->state = 1;
    s->type = base == 3 ? 2 : base;
    s->icmp = base == 3 ? 2 : (base == 2 && proto == 1) ? 1 : 0;
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = F_SOCK;
    g_fd[n].idx = i;
    g_fd[n].flags = O_RDWR_ | ((type & O_NONBLOCK_) ? O_NONBLOCK_ : 0);
    g_fd[n].cloexec = !!(type & O_CLOEXEC_);
    g_fdpath[n][0] = 0;
    pipe_ref_add(&g_fd[n]);
    unlock_();
    return n;
}

static int sock_op(int i, int op, u32 ip, u16 port, int arg) {
    Sock *s = &g->socks[i];
    lock_();
    s->op_ip = ip;
    s->op_port = port;
    s->op_arg = arg;
    s->op_done = 0;
    s->op = op;
    unlock_();
    wake_master();
    while (!s->op_done) wait_event(5);
    return s->op_res;
}

static int sys_bind(int fd, const ksockaddr_in *a) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    if (a->family != 2) return -EAFNOSUPPORT_;
    return sock_op(i, OP_BIND, a->addr, a->port, 0);
}

static int sys_listen(int fd, int backlog) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    return sock_op(i, OP_LISTEN, 0, 0, backlog);
}

static int sys_connect(int fd, const ksockaddr_in *a) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    if (a->family != 2) return a->family == 0 ? 0 : -EAFNOSUPPORT_;
    if (s->type == 2) {
        s->raddr = a->addr;
        s->rport = a->port;
        s->conn = 2;
        return 0;
    }
    if (s->conn == 2) return -EISCONN_;
    int r = sock_op(i, OP_CONNECT, a->addr, a->port, 0);
    if (r) return r;
    if (g_fd[fd].flags & O_NONBLOCK_) return -EINPROGRESS_;
    while (s->conn == 1) {
        if (signal_ready()) return -EINTR;
        wait_event(50);
    }
    if (s->err) {
        r = -s->err;
        s->err = 0;
        return r;
    }
    return 0;
}

static int sys_accept(int fd, ksockaddr_in *a, u32 *alen, int flags) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    if (s->conn != 3) return -EINVAL;
    for (;;) {
        lock_();
        if (s->nq > 0) {
            int n = fd_alloc(0);
            if (n < 0) { unlock_(); return n; }
            int c = s->q[0];
            s->nq--;
            memmove(s->q, s->q + 1, s->nq * sizeof(int));
            memset(&g_fd[n], 0, sizeof(Fd));
            g_fd[n].type = F_SOCK;
            g_fd[n].idx = c;
            g_fd[n].flags = O_RDWR_ | ((flags & O_NONBLOCK_) ? O_NONBLOCK_ : 0);
            g_fd[n].cloexec = !!(flags & O_CLOEXEC_);

            me->srefs[c]++;
            unlock_();
            if (a) {
                ksockaddr_in sa = {2, g->socks[c].rport, g->socks[c].raddr};
                if (alen && *alen >= sizeof(sa)) { *a = sa; *alen = sizeof(sa); }
            }
            return n;
        }
        unlock_();
        if (g_fd[fd].flags & O_NONBLOCK_) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static int sock_send(int fd, const u8 *buf, u32 len, int flags, const ksockaddr_in *to) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    int nonblock = (g_fd[fd].flags & O_NONBLOCK_) || (flags & MSG_DONTWAIT_);
    u8 *ring = g->sock_tx[i];
    if (s->type == 2) {
        u32 ip = to ? to->addr : s->raddr;
        u16 port = to ? to->port : s->rport;
        if (!ip) return -EDESTADDRREQ_;
        if (len > SOCK_TX / 2) return -90;
        lock_();
        if (ring_free(s->thead, s->ttail, SOCK_TX) >= len + 8) {
            u16 l = (u16)len;
            ring_put(ring, SOCK_TX, &s->thead, &ip, 4);
            ring_put(ring, SOCK_TX, &s->thead, &port, 2);
            ring_put(ring, SOCK_TX, &s->thead, &l, 2);
            ring_put(ring, SOCK_TX, &s->thead, buf, len);
        }
        unlock_();
        wake_master();
        return (int)len;
    }
    u32 done = 0;
    while (done < len) {
        if (s->err || s->tx_shut) {
            int e = s->err ? s->err : EPIPE;
            if (e == EPIPE || e == 104) {
                if (!(flags & MSG_NOSIGNAL_)) raise_self(SIGPIPE_);
            }
            s->err = 0;
            return done ? (int)done : -e;
        }
        if (s->conn == 0 || s->conn == 3) return -ENOTCONN_;
        lock_();
        u32 space = ring_free(s->thead, s->ttail, SOCK_TX);
        if (s->conn == 2 && space) {
            u32 put = space < len - done ? space : len - done;
            ring_put(ring, SOCK_TX, &s->thead, buf + done, put);
            done += put;
        }
        unlock_();
        wake_master();
        if (done < len) {
            if (nonblock) return done ? (int)done : -EAGAIN;
            if (signal_ready()) return done ? (int)done : -ERESTART_;
            wait_event(50);
        }
    }
    return (int)len;
}

static int sock_recv(int fd, u8 *buf, u32 len, int flags, ksockaddr_in *from, u32 *fromlen) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    int nonblock = (g_fd[fd].flags & O_NONBLOCK_) || (flags & MSG_DONTWAIT_);
    u8 *ring = g->sock_rx[i];
    u32 got = 0;
    for (;;) {
        lock_();
        u32 avail = ring_used(s->rhead, s->rtail, SOCK_RX);
        if (s->type == 2 && avail >= 8) {
            u32 ip;
            u16 port, l;
            ring_get(ring, SOCK_RX, s->rtail, &ip, 4);
            ring_get(ring, SOCK_RX, (s->rtail + 4) % SOCK_RX, &port, 2);
            ring_get(ring, SOCK_RX, (s->rtail + 6) % SOCK_RX, &l, 2);
            u32 take = l < len ? l : len;
            ring_get(ring, SOCK_RX, (s->rtail + 8) % SOCK_RX, buf, take);
            if (!(flags & MSG_PEEK_)) s->rtail = (s->rtail + 8 + l) % SOCK_RX;
            unlock_();
            if (from && fromlen && *fromlen >= sizeof(ksockaddr_in)) {
                ksockaddr_in sa = {2, port, ip};
                *from = sa;
                *fromlen = sizeof(sa);
            }
            return (int)take;
        }
        if (s->type == 1 && avail) {
            u32 take = avail < len - got ? avail : len - got;
            ring_get(ring, SOCK_RX, s->rtail, buf + got, take);
            if (!(flags & MSG_PEEK_)) s->rtail = (s->rtail + take) % SOCK_RX;
            got += take;
            unlock_();
            wake_master();
            if (got == len || !(flags & MSG_WAITALL_)) {
                if (from && fromlen && *fromlen >= sizeof(ksockaddr_in)) {
                    ksockaddr_in sa = {2, s->rport, s->raddr};
                    *from = sa;
                    *fromlen = sizeof(sa);
                }
                return (int)got;
            }
            continue;
        }
        unlock_();
        if (s->err) {
            int e = s->err;
            s->err = 0;
            return got ? (int)got : -e;
        }
        if (s->rx_eof) return (int)got;
        if (s->type == 1 && s->conn == 0) return -ENOTCONN_;
        if (nonblock) return got ? (int)got : -EAGAIN;
        if (signal_ready()) return got ? (int)got : -ERESTART_;
        wait_event(50);
    }
}

typedef struct { u32 name, namelen, iov, iovlen, control, controllen, flags; } kmsghdr;
static u8 g_msgbuf[65536];

static int sys_sendmsg(int fd, const kmsghdr *m, int flags) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    u32 total = 0;
    for (u32 i = 0; i < m->iovlen; i++) {
        u32 l = iov[i * 2 + 1];
        if (total + l > sizeof(g_msgbuf)) return -90;
        memcpy(g_msgbuf + total, (void *)(ULONG_PTR)iov[i * 2], l);
        total += l;
    }
    return sock_send(fd, g_msgbuf, total, flags, m->name ? (const ksockaddr_in *)(ULONG_PTR)m->name : NULL);
}

static int sys_recvmsg(int fd, kmsghdr *m, int flags) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    u32 total = 0;
    for (u32 i = 0; i < m->iovlen; i++) total += iov[i * 2 + 1];
    if (total > sizeof(g_msgbuf)) total = sizeof(g_msgbuf);
    u32 nl = m->namelen;
    int r = sock_recv(fd, g_msgbuf, total, flags, m->name ? (ksockaddr_in *)(ULONG_PTR)m->name : NULL, &nl);
    if (r < 0) return r;
    m->namelen = nl;
    m->controllen = 0;
    m->flags = 0;
    u32 off = 0;
    for (u32 i = 0; i < m->iovlen && off < (u32)r; i++) {
        u32 l = iov[i * 2 + 1];
        if (l > (u32)r - off) l = (u32)r - off;
        memcpy((void *)(ULONG_PTR)iov[i * 2], g_msgbuf + off, l);
        off += l;
    }
    return r;
}

static int sys_sockname(int fd, ksockaddr_in *a, u32 *alen, int peer) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    if (peer && !s->raddr) return -ENOTCONN_;
    if (!peer) sock_op(i, OP_NAME, 0, 0, 0);
    ksockaddr_in sa = {2, peer ? s->rport : s->lport, peer ? s->raddr : s->laddr};
    if (*alen >= sizeof(sa)) *a = sa;
    *alen = sizeof(sa);
    return 0;
}

static int sys_getsockopt(int fd, int level, int name, void *val, u32 *len) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    int v = 0;
    if (level == 0 && name == 4) {
        *len = 0;
        return 0;
    }
    if (level == 1 && name == 4) {
        v = s->err;
        s->err = 0;
    } else if (level == 1 && name == 3) {
        v = s->type;
    } else if (level == 1 && (name == 7 || name == 8)) {
        v = name == 7 ? SOCK_TX : SOCK_RX;
    }
    if (*len >= 4) { *(int *)val = v; *len = 4; }
    return 0;
}

static int sys_shutdown(int fd, int how) {
    int i = sock_fd(fd);
    if (i < 0) return i;
    Sock *s = &g->socks[i];
    if (how == 0 || how == 2) s->rx_eof = 1;
    if (how == 1 || how == 2) s->tx_shut = 1;
    wake_master();
    wake_all();
    return 0;
}
