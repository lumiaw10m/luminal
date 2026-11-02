#define NL_BUF 32768
#define NLM_F_DUMP_ 0x300
#define RTM_GETLINK_ 18
#define RTM_GETADDR_ 22
#define RTM_GETROUTE_ 26

static u8 g_nl_buf[NL_BUF];
static u32 g_nl_len, g_nl_pos;
static u8 *g_nl_msg;

static int nl_ready(void) { return g_nl_pos < g_nl_len; }

static int nl_socket(int type, int proto) {
    if (proto != 0) return -93;
    int n = fd_alloc(0);
    if (n < 0) return n;
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = F_NETLINK;
    g_fd[n].flags = O_RDWR_ | (type & O_NONBLOCK_);
    g_fd[n].cloexec = !!(type & O_CLOEXEC_);
    g_fdpath[n][0] = 0;
    g_nl_len = g_nl_pos = 0;
    return n;
}

static void nl_start(u16 type, u16 flags, u32 seq, const void *body, u32 blen) {
    u32 start = (g_nl_len + 3) & ~3u;
    if (start + 16 + blen + 256 > NL_BUF) { g_nl_msg = NULL; return; }
    g_nl_msg = g_nl_buf + start;
    u32 total = 16 + blen;
    memcpy(g_nl_msg, &total, 4);
    memcpy(g_nl_msg + 4, &type, 2);
    memcpy(g_nl_msg + 6, &flags, 2);
    memcpy(g_nl_msg + 8, &seq, 4);
    memcpy(g_nl_msg + 12, &me->pid, 4);
    if (blen) memcpy(g_nl_msg + 16, body, blen);
    g_nl_len = start + total;
}

static void nl_attr(u16 type, const void *data, u32 n) {
    if (!g_nl_msg) return;
    u32 total = *(u32 *)g_nl_msg, at = (total + 3) & ~3u;
    u16 alen = (u16)(4 + n);
    memcpy(g_nl_msg + at, &alen, 2);
    memcpy(g_nl_msg + at + 2, &type, 2);
    memcpy(g_nl_msg + at + 4, data, n);
    memset(g_nl_msg + at + 4 + n, 0, 4);
    total = (at + 4 + n + 3) & ~3u;
    memcpy(g_nl_msg, &total, 4);
    g_nl_len = (u32)(g_nl_msg - g_nl_buf) + total;
}

static void nl_link(NetIf *f, int index, u32 seq, u16 flags) {
    struct { u8 family, pad; u16 type; int index; u32 flags, change; } m = {0, 0, f->loopback ? 772 : 1, index, f->loopback ? 0x10049u : 0x11043u, 0};
    u32 mtu = f->loopback ? 65536 : 1500, q = f->loopback ? 0 : 1000;
    u8 oper = 6, bcast[6] = {255, 255, 255, 255, 255, 255};
    nl_start(16, flags, seq, &m, sizeof(m));
    nl_attr(3, f->name, (u32)strlen(f->name) + 1);
    nl_attr(1, f->mac, 6);
    nl_attr(2, f->loopback ? f->mac : bcast, 6);
    nl_attr(4, &mtu, 4);
    nl_attr(13, &q, 4);
    nl_attr(16, &oper, 1);
}

static int nl_prefix(u32 mask) {
    int n = 0;
    for (u32 m = mask; m; m >>= 1) n += m & 1;
    return n;
}

static void nl_addr(NetIf *f, int index, u32 seq, u16 flags) {
    struct { u8 family, prefix, flags, scope; u32 index; } m = {2, (u8)nl_prefix(f->mask), 0x80, (u8)(f->loopback ? 254 : 0), (u32)index};
    u32 bcast = f->ip | ~f->mask;
    nl_start(20, flags, seq, &m, sizeof(m));
    nl_attr(1, &f->ip, 4);
    nl_attr(2, &f->ip, 4);
    if (!f->loopback) nl_attr(4, &bcast, 4);
    nl_attr(3, f->name, (u32)strlen(f->name) + 1);
}

static void nl_route(NetIf *f, int index, int deflt, u32 seq, u16 flags) {
    struct { u8 family, dst, src, tos, table, proto, scope, type; u32 flags; } m = {2, (u8)(deflt ? 0 : nl_prefix(f->mask)), 0, 0, 254, (u8)(deflt ? 3 : 2), (u8)(deflt ? 0 : 253), 1, 0};
    u32 dst = f->ip & f->mask, table = 254, oif = index;
    nl_start(24, flags, seq, &m, sizeof(m));
    nl_attr(15, &table, 4);
    if (!deflt) nl_attr(1, &dst, 4);
    nl_attr(4, &oif, 4);
    if (deflt) nl_attr(5, &f->gw, 4);
    else nl_attr(7, &f->ip, 4);
}

static void nl_done(u32 seq) {
    int zero = 0;
    nl_start(3, 2, seq, &zero, 4);
}

static void nl_error(u32 seq, int err, const u8 *req) {
    struct { int error; u8 orig[16]; } m;
    m.error = err;
    memcpy(m.orig, req, 16);
    nl_start(2, 0, seq, &m, sizeof(m));
}

static void nl_route_to(u32 ip, u32 seq) {
    int best = -1, idx = 0;
    for (int i = 0; i < g->nnetif; i++) {
        NetIf *f = &g->netifs[i];
        if ((ip & f->mask) == (f->ip & f->mask)) { best = i; break; }
        if (f->gw && best < 0) idx = i + 1;
    }
    int use = best >= 0 ? best : idx - 1;
    if (use < 0) {
        static const u8 none[16];
        nl_error(seq, -101, none);
        return;
    }
    NetIf *f = &g->netifs[use];
    struct { u8 family, dst, src, tos, table, proto, scope, type; u32 flags; } m = {2, 32, 0, 0, 254, 2, 0, 1, 0};
    u32 oif = use + 1;
    nl_start(24, 0, seq, &m, sizeof(m));
    nl_attr(1, &ip, 4);
    nl_attr(4, &oif, 4);
    nl_attr(7, &f->ip, 4);
    if (best < 0) nl_attr(5, &f->gw, 4);
}

static int nl_request(const u8 *buf, u32 len) {
    if (len < 16) return -EINVAL;
    u16 type, flags;
    u32 seq;
    memcpy(&type, buf + 4, 2);
    memcpy(&flags, buf + 6, 2);
    memcpy(&seq, buf + 8, 4);
    g_nl_len = g_nl_pos = 0;
    int dump = (flags & NLM_F_DUMP_) != 0;
    if (type == RTM_GETLINK_ && dump) {
        for (int i = 0; i < g->nnetif; i++) nl_link(&g->netifs[i], i + 1, seq, 2);
        nl_done(seq);
    } else if (type == RTM_GETLINK_) {
        int index = len >= 24 ? *(int *)(buf + 20) : 0;
        if (index >= 1 && index <= g->nnetif) nl_link(&g->netifs[index - 1], index, seq, 0);
        else nl_error(seq, -19, buf);
    } else if (type == RTM_GETADDR_ && dump) {
        for (int i = 0; i < g->nnetif; i++) nl_addr(&g->netifs[i], i + 1, seq, 2);
        nl_done(seq);
    } else if (type == RTM_GETROUTE_ && dump) {
        for (int i = 0; i < g->nnetif; i++) {
            NetIf *f = &g->netifs[i];
            if (f->loopback) continue;
            if (f->gw) nl_route(f, i + 1, 1, seq, 2);
            nl_route(f, i + 1, 0, seq, 2);
        }
        nl_done(seq);
    } else if (type == RTM_GETROUTE_) {

        u32 ip = 0;
        for (u32 at = 28; at + 8 <= len;) {
            u16 alen, atype;
            memcpy(&alen, buf + at, 2);
            memcpy(&atype, buf + at + 2, 2);
            if (alen < 4) break;
            if (atype == 1 && alen >= 8) memcpy(&ip, buf + at + 4, 4);
            at += (alen + 3) & ~3u;
        }
        nl_route_to(ip, seq);
    } else if (dump && (type == 34 || type == 30 || type == 74 || type == 66 || type == 58)) {
        nl_done(seq);
    } else {
        nl_error(seq, -EPERM, buf);
    }
    wake_all();
    return (int)len;
}

static int nl_send(const u8 *buf, u32 len) { return nl_request(buf, len); }

static int nl_recv(int fd, u8 *buf, u32 len, int flags, u8 *from, u32 *fromlen) {
    while (g_nl_pos >= g_nl_len) {

        if ((g_fd[fd].flags & O_NONBLOCK_) || (flags & MSG_DONTWAIT_)) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
    u32 avail = g_nl_len - g_nl_pos, take = 0;

    while (take < avail) {
        u32 m = *(u32 *)(g_nl_buf + g_nl_pos + take), step = (m + 3) & ~3u;
        if (take + m > len) break;
        take += step;
        if (take > avail) take = avail;
    }
    if (!take && !(flags & 0x20)) {
        memcpy(buf, g_nl_buf + g_nl_pos, len);
        g_nl_pos += avail;
        return (int)len;
    }
    if (take) memcpy(buf, g_nl_buf + g_nl_pos, take);
    if (from && fromlen && *fromlen >= 12) {
        memset(from, 0, 12);
        *(u16 *)from = 16;
        *fromlen = 12;
    }
    if (flags & 0x20) {
        u32 whole = avail;
        if (!(flags & MSG_PEEK_)) g_nl_pos += take;
        return (int)whole;
    }
    if (!(flags & MSG_PEEK_)) g_nl_pos += take;
    return (int)take;
}

static int nl_sendmsg(const kmsghdr *m) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov, total = 0;
    for (u32 i = 0; i < m->iovlen; i++) {
        u32 l = iov[i * 2 + 1];
        if (total + l > sizeof(g_msgbuf)) return -90;
        memcpy(g_msgbuf + total, (void *)(ULONG_PTR)iov[i * 2], l);
        total += l;
    }
    return nl_send(g_msgbuf, total);
}

static int nl_recvmsg(int fd, kmsghdr *m, int flags) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov, room = 0, nl = m->namelen;
    for (u32 i = 0; i < m->iovlen; i++) room += iov[i * 2 + 1];
    if (room > sizeof(g_msgbuf)) room = sizeof(g_msgbuf);
    int r = nl_recv(fd, g_msgbuf, room, flags, (u8 *)(ULONG_PTR)m->name, m->name ? &nl : NULL);
    if (r < 0) return r;
    u32 copied = (u32)r < room ? (u32)r : room, off = 0;
    for (u32 i = 0; i < m->iovlen && off < copied; i++) {
        u32 l = iov[i * 2 + 1] < copied - off ? iov[i * 2 + 1] : copied - off;
        memcpy((void *)(ULONG_PTR)iov[i * 2], g_msgbuf + off, l);
        off += l;
    }
    m->namelen = m->name ? nl : 0;
    m->controllen = 0;
    m->flags = (u32)r > room ? 0x20 : 0;
    return r;
}

static int nl_syscall(u32 nr, u32 a0, u32 a1, u32 a2, u32 a3, u32 a4, u32 a5, int *ret) {
    switch (nr) {
        case 282: case 283: case 293: case 294: *ret = 0; return 1;
        case 286: case 287: {
            u8 *sa = (u8 *)(ULONG_PTR)a1;
            memset(sa, 0, 12);
            *(u16 *)sa = 16;
            if (nr == 286) *(u32 *)(sa + 4) = me->pid;
            *(u32 *)(ULONG_PTR)a2 = 12;
            *ret = 0;
            return 1;
        }
        case 289: case 290: *ret = nl_send((const u8 *)(ULONG_PTR)a1, a2); return 1;
        case 291: *ret = nl_recv(a0, (u8 *)(ULONG_PTR)a1, a2, a3, NULL, NULL); return 1;
        case 292: *ret = nl_recv(a0, (u8 *)(ULONG_PTR)a1, a2, a3, (u8 *)(ULONG_PTR)a4, (u32 *)(ULONG_PTR)a5); return 1;
        case 295: {
            u32 *val = (u32 *)(ULONG_PTR)a3, *len = (u32 *)(ULONG_PTR)a4;
            if (*len >= 4) { val[0] = a2 == 3 ? 3 : a1 == 1 && (a2 == 7 || a2 == 8) ? NL_BUF : 0; *len = 4; }
            *ret = 0;
            return 1;
        }
        case 296: *ret = nl_sendmsg((const kmsghdr *)(ULONG_PTR)a1); return 1;
        case 297: *ret = nl_recvmsg(a0, (kmsghdr *)(ULONG_PTR)a1, a2); return 1;
    }
    return 0;
}
