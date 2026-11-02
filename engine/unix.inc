#define EADDRINUSE_ 98
#define ECONNREFUSED_ 111

static int unix_socket(int type) {
    if ((type & 0xF) != 1 && (type & 0xF) != 2) return -93;
    int n = fd_alloc(0);
    if (n < 0) return n;
    lock_();
    int i;
    for (i = 0; i < MAX_UNIX; i++)
        if (!g->unixs[i].used) break;
    if (i == MAX_UNIX) {
        unlock_();
        return -ENFILE;
    }
    memset(&g->unixs[i], 0, sizeof(g->unixs[i]));
    g->unixs[i].used = 1;
    g->unixs[i].dgram = (type & 0xF) == 2;
    memset(&g_fd[n], 0, sizeof(Fd));
    g_fd[n].type = F_UNIX;
    g_fd[n].idx = i;
    g_fd[n].flags = O_RDWR_ | (type & O_NONBLOCK_);
    g_fd[n].cloexec = !!(type & O_CLOEXEC_);
    pipe_ref_add(&g_fd[n]);
    unlock_();
    return n;
}

static int unix_key(const u8 *addr, u32 len, char *out) {
    static char tmp[PATH_MAX_];
    if (len < 3 || *(const u16 *)addr != 1) return -EINVAL;
    u32 plen = len - 2;
    if (plen > 107) plen = 107;
    if (addr[2] == 0) {
        out[0] = '@';
        memcpy(out + 1, addr + 3, plen - 1);
        out[plen] = 0;
        return 1;
    }
    memcpy(tmp, addr + 2, plen);
    tmp[plen] = 0;
    int r = resolve(AT_FDCWD_, tmp, 0, out);
    return r ? r : 0;
}

static int unix_bind(int fd, const u8 *addr, u32 len) {
    static char key[PATH_MAX_];
    Unix *u = &g->unixs[g_fd[fd].idx];
    int kind = unix_key(addr, len, key);
    if (kind < 0) return kind;
    if (strlen(key) >= sizeof(u->path)) return -36;
    if (!kind) {

        if (path_attr(key) != INVALID_FILE_ATTRIBUTES) return -EADDRINUSE_;
        HANDLE h;
        NTSTATUS st = nt_open(key, GENERIC_WRITE | FILE_WRITE_EA, FILE_CREATE_, FO_NONDIR, FILE_ATTRIBUTE_NORMAL, &h);
        if (!NT_OK(st)) return -errno_of(st);
        TxMeta m = {0140000 | 0755, 0, 0};
        meta_set(h, &m);
        NtClose(h);
    }
    lock_();
    strcpy(u->path, key);
    u->bound = 1;
    unlock_();
    return 0;
}

static int unix_listen(int fd) {
    Unix *u = &g->unixs[g_fd[fd].idx];
    if (!u->bound) return -EINVAL;
    u->listening = 1;
    return 0;
}

static int unix_connect(int fd, const u8 *addr, u32 len) {
    static char key[PATH_MAX_];
    int kind = unix_key(addr, len, key);
    if (kind < 0) return kind;
    if (!kind && path_attr(key) == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    if (g->unixs[g_fd[fd].idx].dgram) {
        if (strlen(key) >= sizeof(g->unixs[0].dpeer)) return -36;
        strcpy(g->unixs[g_fd[fd].idx].dpeer, key);
        return 0;
    }
    int fds_a[2], fds_b[2];
    int r = do_pipe(fds_a, 0);
    if (r) return r;
    r = do_pipe(fds_b, 0);
    if (r) {
        fd_close(fds_a[0]);
        fd_close(fds_a[1]);
        return r;
    }
    int pa = g_fd[fds_a[0]].idx, pb = g_fd[fds_b[0]].idx;
    lock_();
    Unix *l = NULL;
    for (int i = 0; i < MAX_UNIX; i++) {
        Unix *c = &g->unixs[i];
        if (c->used && c->listening && !strcmp(c->path, key)) l = c;
    }
    if (l && l->nq < 8) {

        l->qrd[l->nq] = pb;
        l->qwr[l->nq] = pa;
        l->nq++;
        g->pipes[pb].readers++;
        g->pipes[pa].writers++;
    }
    int ok = l && l->nq <= 8 && l->qrd[l->nq - 1] == pb;
    unlock_();
    if (!ok) {
        fd_close(fds_a[0]);
        fd_close(fds_a[1]);
        fd_close(fds_b[0]);
        fd_close(fds_b[1]);
        return -ECONNREFUSED_;
    }
    wake_all();

    int flags = g_fd[fd].flags, cloexec = g_fd[fd].cloexec;
    fd_close(fd);
    Fd *f = &g_fd[fds_a[0]];
    f->type = F_SPAIR;
    f->idx2 = pb;
    f->flags = O_RDWR_ | (flags & O_NONBLOCK_);
    f->cloexec = (u8)cloexec;
    g_fd[fd] = *f;
    memset(f, 0, sizeof(*f));
    memset(&g_fd[fds_b[1]], 0, sizeof(Fd));

    lock_();
    g->pipes[pa].writers--;
    me->wrefs[pa]--;
    g->pipes[pb].readers--;
    me->rrefs[pb]--;
    unlock_();
    memset(&g_fd[fds_a[1]], 0, sizeof(Fd));
    memset(&g_fd[fds_b[0]], 0, sizeof(Fd));
    return 0;
}

static int unix_accept(int fd, u8 *addr, u32 *alen, int flags) {
    Unix *u = &g->unixs[g_fd[fd].idx];
    if (!u->listening) return -EINVAL;
    for (;;) {
        lock_();
        if (u->nq > 0) {
            int n = fd_alloc(0);
            if (n < 0) {
                unlock_();
                return n;
            }
            int rd = u->qrd[0], wr = u->qwr[0];
            u->nq--;
            memmove(u->qrd, u->qrd + 1, u->nq * sizeof(int));
            memmove(u->qwr, u->qwr + 1, u->nq * sizeof(int));
            memset(&g_fd[n], 0, sizeof(Fd));
            g_fd[n].type = F_SPAIR;
            g_fd[n].idx = rd;
            g_fd[n].idx2 = wr;
            g_fd[n].flags = O_RDWR_ | (flags & O_NONBLOCK_);
            g_fd[n].cloexec = !!(flags & O_CLOEXEC_);
            me->rrefs[rd]++;
            me->wrefs[wr]++;
            unlock_();
            if (addr && alen && *alen >= 2) {
                *(u16 *)addr = 1;
                *alen = 2;
            }
            return n;
        }
        unlock_();
        if (g_fd[fd].flags & O_NONBLOCK_) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static void udg_put(Unix *t, const void *src, u32 n) {
    for (u32 i = 0; i < n; i++) {
        t->dbuf[t->dhead] = ((const u8 *)src)[i];
        t->dhead = (t->dhead + 1) % UDG_SZ;
    }
}

static void udg_get(Unix *u, void *dst, u32 n) {
    for (u32 i = 0; i < n; i++) {
        if (dst) ((u8 *)dst)[i] = u->dbuf[u->dtail];
        u->dtail = (u->dtail + 1) % UDG_SZ;
    }
}

static int udg_send(int fd, const u8 *buf, u32 len, const u8 *addr, u32 alen, int flags) {
    static char key[PATH_MAX_];
    Unix *u = &g->unixs[g_fd[fd].idx];
    if (addr && alen) {
        int kind = unix_key(addr, alen, key);
        if (kind < 0) return kind;
        if (!kind && path_attr(key) == INVALID_FILE_ATTRIBUTES) return -ENOENT;
    } else if (u->dpeer[0]) {
        strcpy(key, u->dpeer);
    } else {
        return -89;
    }
    u32 plen = u->bound ? (u32)strlen(u->path) : 0;
    u32 need = 3 + plen + len;
    if (need >= UDG_SZ) return -90;
    for (;;) {
        lock_();
        Unix *t = NULL;
        for (int i = 0; i < MAX_UNIX; i++) {
            Unix *c = &g->unixs[i];
            if (c->used && c->dgram && c->bound && !strcmp(c->path, key)) t = c;
        }
        if (!t) {
            unlock_();
            return -ECONNREFUSED_;
        }
        if ((t->dtail + UDG_SZ - t->dhead - 1) % UDG_SZ >= need) {
            u16 n16 = (u16)len;
            u8 p8 = (u8)plen;
            udg_put(t, &n16, 2);
            udg_put(t, &p8, 1);
            udg_put(t, u->path, plen);
            udg_put(t, buf, len);
            unlock_();
            wake_all();
            return (int)len;
        }
        unlock_();
        if ((g_fd[fd].flags & O_NONBLOCK_) || (flags & MSG_DONTWAIT_)) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static int udg_recv(int fd, u8 *buf, u32 len, int flags, u8 *from, u32 *fromlen) {
    Unix *u = &g->unixs[g_fd[fd].idx];
    for (;;) {
        lock_();
        if (u->dhead != u->dtail) {
            u16 n16;
            u8 p8;
            char name[120];
            udg_get(u, &n16, 2);
            udg_get(u, &p8, 1);
            udg_get(u, name, p8);
            u32 take = n16 < len ? n16 : len;
            udg_get(u, buf, take);
            udg_get(u, NULL, n16 - take);
            unlock_();
            wake_all();
            if (from && fromlen) {
                u32 cap = *fromlen, full = 2;
                if (cap >= 2) *(u16 *)from = 1;
                if (p8) {

                    int abstract = name[0] == '@';
                    u32 copy = cap > 2 ? cap - 2 : 0;
                    if (copy > p8) copy = p8;
                    if (copy) memcpy(from + 2, name, copy);
                    if (abstract && copy) from[2] = 0;
                    if (!abstract && cap > 2u + p8) from[2 + p8] = 0;
                    full = 2 + p8 + (abstract ? 0 : 1);
                }
                *fromlen = full;
            }
            return (int)take;
        }
        unlock_();
        if ((g_fd[fd].flags & O_NONBLOCK_) || (flags & MSG_DONTWAIT_)) return -EAGAIN;
        if (signal_ready()) return -ERESTART_;
        wait_event(100);
    }
}

static int udg_sendmsg(int fd, const kmsghdr *m, int flags) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    u32 total = 0;
    for (u32 i = 0; i < m->iovlen; i++) {
        u32 l = iov[i * 2 + 1];
        if (total + l > sizeof(g_msgbuf)) return -90;
        memcpy(g_msgbuf + total, (void *)(ULONG_PTR)iov[i * 2], l);
        total += l;
    }
    return udg_send(fd, g_msgbuf, total, (const u8 *)(ULONG_PTR)m->name, m->namelen, flags);
}

static int udg_recvmsg(int fd, kmsghdr *m, int flags) {
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    u32 room = 0, nl = m->namelen;
    for (u32 i = 0; i < m->iovlen; i++) room += iov[i * 2 + 1];
    if (room > sizeof(g_msgbuf)) room = sizeof(g_msgbuf);
    int r = udg_recv(fd, g_msgbuf, room, flags, (u8 *)(ULONG_PTR)m->name, m->name ? &nl : NULL);
    if (r < 0) return r;
    u32 off = 0;
    for (u32 i = 0; i < m->iovlen && off < (u32)r; i++) {
        u32 l = iov[i * 2 + 1] < (u32)r - off ? iov[i * 2 + 1] : (u32)r - off;
        memcpy((void *)(ULONG_PTR)iov[i * 2], g_msgbuf + off, l);
        off += l;
    }
    m->namelen = m->name ? nl : 0;
    m->controllen = 0;
    m->flags = 0;
    return r;
}

static void unix_drop_queue(Unix *u) {
    for (int i = 0; i < u->nq; i++) {
        g->pipes[u->qrd[i]].readers--;
        g->pipes[u->qwr[i]].writers--;
    }
    u->nq = 0;
}

static void fds_send(int pipe_i, const int *fds, int n) {
    Pipe *p = &g->pipes[pipe_i];
    lock_();
    for (int i = 0; i < n && p->nfdq < 4; i++) {
        if (fds[i] < 0 || fds[i] >= MAX_FD || !g_fd[fds[i]].type) continue;
        Fd *f = &g_fd[fds[i]];
        FdMsg *m = &p->fdq[p->nfdq++];
        m->type = f->type;
        m->idx = f->idx;
        m->idx2 = f->idx2;
        m->flags = f->flags;
        strncpy(m->path, g_fdpath[fds[i]], sizeof(m->path) - 1);
        m->path[sizeof(m->path) - 1] = 0;
        ref_count_add(f);
    }
    unlock_();
}

static int fds_recv(int pipe_i, int *out, int max) {
    Pipe *p = &g->pipes[pipe_i];
    int got = 0;
    static FdMsg local[4];
    lock_();
    int n = p->nfdq < max ? p->nfdq : max;
    memcpy(local, p->fdq, n * sizeof(FdMsg));
    memmove(p->fdq, p->fdq + n, (p->nfdq - n) * sizeof(FdMsg));
    p->nfdq -= n;
    unlock_();
    for (int i = 0; i < n; i++) {
        FdMsg *m = &local[i];
        int fd;
        if (m->type == F_FILE || m->type == F_DIR) {

            fd = sys_openat(AT_FDCWD_, m->path, m->flags & ~(O_CREAT_ | O_EXCL_ | O_TRUNC_), 0);
        } else {
            fd = fd_alloc(0);
            if (fd >= 0) {
                memset(&g_fd[fd], 0, sizeof(Fd));
                g_fd[fd].type = m->type;
                g_fd[fd].idx = m->idx;
                g_fd[fd].idx2 = m->idx2;
                g_fd[fd].flags = m->flags;
                strcpy(g_fdpath[fd], m->path);
                lock_();
                ref_mirror_add(&g_fd[fd]);
                unlock_();
            }
        }
        if (fd >= 0) out[got++] = fd;
    }
    return got;
}

static int spair_sendmsg(int fd, const kmsghdr *m, int flags) {
    Fd *f = &g_fd[fd];
    if (m->control && m->controllen >= 12) {
        const u8 *cm = (const u8 *)(ULONG_PTR)m->control;
        u32 clen = *(const u32 *)cm;
        if (*(const int *)(cm + 4) == 1 && *(const int *)(cm + 8) == 1 && clen > 12)
            fds_send(f->idx2, (const int *)(cm + 12), (int)((clen - 12) / 4));
    }
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    int total = 0;
    int saved = f->flags;
    if (flags & MSG_DONTWAIT_) f->flags |= O_NONBLOCK_;
    for (u32 i = 0; i < m->iovlen; i++) {
        if (!iov[i * 2 + 1]) continue;
        int r = sys_write(fd, (const u8 *)(ULONG_PTR)iov[i * 2], iov[i * 2 + 1]);
        if (r < 0) {
            f->flags = saved;
            return total ? total : r;
        }
        total += r;
        if ((u32)r < iov[i * 2 + 1]) break;
    }
    f->flags = saved;
    return total;
}

static int spair_recvmsg(int fd, kmsghdr *m, int flags) {
    Fd *f = &g_fd[fd];
    u32 *iov = (u32 *)(ULONG_PTR)m->iov;
    int total = 0;
    int saved = f->flags;
    if (flags & MSG_DONTWAIT_) f->flags |= O_NONBLOCK_;
    for (u32 i = 0; i < m->iovlen; i++) {
        if (!iov[i * 2 + 1]) continue;
        int r = sys_read(fd, (u8 *)(ULONG_PTR)iov[i * 2], iov[i * 2 + 1]);
        if (r < 0) {
            f->flags = saved;
            if (total) break;
            return r;
        }
        total += r;

        if ((u32)r < iov[i * 2 + 1]) break;
        f->flags |= O_NONBLOCK_;
    }
    f->flags = saved;
    u32 clen = 0;
    if (m->control && m->controllen >= 16 && g->pipes[f->idx].nfdq) {
        u8 *cm = (u8 *)(ULONG_PTR)m->control;
        int got = fds_recv(f->idx, (int *)(cm + 12), (int)((m->controllen - 12) / 4));
        if (got) {
            *(u32 *)cm = 12 + got * 4;
            *(int *)(cm + 4) = 1;
            *(int *)(cm + 8) = 1;
            clen = 12 + got * 4;
        }
    }
    m->controllen = clen;
    m->flags = 0;
    m->namelen = 0;
    return total;
}
