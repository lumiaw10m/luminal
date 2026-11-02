static SOCKET g_msock[MAX_SOCK];
static u8 g_nettmp[65536];
static WSAEVENT g_net_event;

static void net_watch(SOCKET k) {
    if (!g_net_event) g_net_event = WSACreateEvent();
    WSAEventSelect(k, g_net_event, FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
}

static int wsa_errno(int e) {
    switch (e) {
        case WSAECONNREFUSED: return 111;
        case WSAETIMEDOUT: return 110;
        case WSAENETUNREACH: return 101;
        case WSAEHOSTUNREACH: return 113;
        case WSAECONNRESET: return 104;
        case WSAECONNABORTED: return 103;
        case WSAEADDRINUSE: return 98;
        case WSAEADDRNOTAVAIL: return 99;
        case WSAEACCES: return EACCES;
        case WSAEINVAL: return EINVAL;
        case WSAENOTCONN: return ENOTCONN_;
        case WSAENETDOWN: return 100;
    }
    trace("unmapped wsa error %d\n", e);
    return EIO;
}

static SOCKET net_ensure(int i) {
    if (g_msock[i]) return g_msock[i];
    Sock *s = &g->socks[i];
    SOCKET k = socket(AF_INET, s->type == 1 ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (k == INVALID_SOCKET) return 0;
    u_long one = 1;
    ioctlsocket(k, FIONBIO, &one);
    net_watch(k);
    SetHandleInformation((HANDLE)k, HANDLE_FLAG_INHERIT, 0);
    if (s->type == 1) {
        int on = 1;
        setsockopt(k, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
    }
    g_msock[i] = k;
    return k;
}

static void net_update_name(int i) {
    Sock *s = &g->socks[i];
    struct sockaddr_in a;
    int l = sizeof(a);
    if (g_msock[i] && !getsockname(g_msock[i], (struct sockaddr *)&a, &l)) {
        s->laddr = a.sin_addr.s_addr;
        s->lport = a.sin_port;
    }
}

static void net_do_op(int i) {
    Sock *s = &g->socks[i];
    int res = 0;
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = s->op_ip;
    a.sin_port = s->op_port;
    SOCKET k = net_ensure(i);
    if (!k) {
        res = -ENFILE;
    } else if (s->op == OP_BIND) {
        int on = 1;
        setsockopt(k, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        if (bind(k, (struct sockaddr *)&a, sizeof(a))) res = -wsa_errno(WSAGetLastError());
        net_update_name(i);
    } else if (s->op == OP_LISTEN) {
        if (listen(k, s->op_arg > 8 ? 8 : s->op_arg)) res = -wsa_errno(WSAGetLastError());
        else s->conn = 3;
        net_update_name(i);
    } else if (s->op == OP_CONNECT) {
        s->raddr = s->op_ip;
        s->rport = s->op_port;
        if (connect(k, (struct sockaddr *)&a, sizeof(a))) {
            int e = WSAGetLastError();
            if (e == WSAEWOULDBLOCK || e == WSAEINPROGRESS) s->conn = 1;
            else res = -wsa_errno(e);
        } else {
            s->conn = 2;
        }
        net_update_name(i);
    } else if (s->op == OP_NAME) {
        net_update_name(i);
    }
    s->op_res = res;
    s->op = 0;
    s->op_done = 1;
    wake_all();
}

static void net_close(int i) {
    Sock *s = &g->socks[i];
    if (g_msock[i]) {
        closesocket(g_msock[i]);
        g_msock[i] = 0;
    }
    for (int q = 0; q < s->nq; q++) g->socks[s->q[q]].refs--;
    memset(s, 0, sizeof(*s));
}

static void net_accept(int i) {
    Sock *s = &g->socks[i];
    struct sockaddr_in a;
    int l = sizeof(a);
    SOCKET c = accept(g_msock[i], (struct sockaddr *)&a, &l);
    if (c == INVALID_SOCKET) return;
    int n = -1;
    lock_();
    if (s->nq < 8) {
        for (int j = 0; j < MAX_SOCK; j++) {
            if (!g->socks[j].state) { n = j; break; }
        }
    }
    if (n >= 0) {
        Sock *t = &g->socks[n];
        memset(t, 0, sizeof(*t));
        t->state = 1;
        t->type = 1;
        t->refs = 1;
        t->conn = 2;
        t->raddr = a.sin_addr.s_addr;
        t->rport = a.sin_port;
        s->q[s->nq++] = n;
    }
    unlock_();
    if (n < 0) {
        closesocket(c);
        return;
    }
    u_long one = 1;
    ioctlsocket(c, FIONBIO, &one);
    net_watch(c);
    SetHandleInformation((HANDLE)c, HANDLE_FLAG_INHERIT, 0);
    g_msock[n] = c;
    net_update_name(n);
    wake_all();
}

static void net_read_stream(int i) {
    Sock *s = &g->socks[i];
    u32 space = ring_free(s->rhead, s->rtail, SOCK_RX);
    if (!space) return;
    int n = recv(g_msock[i], (char *)g_nettmp, space < sizeof(g_nettmp) ? space : sizeof(g_nettmp), 0);
    if (n > 0) {
        lock_();
        ring_put(g->sock_rx[i], SOCK_RX, &s->rhead, g_nettmp, n);
        unlock_();
    } else if (n == 0) {
        s->rx_eof = 1;
    } else {
        int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) return;
        s->err = wsa_errno(e);
        s->rx_eof = 1;
    }
    wake_all();
}

static void net_read_dgram(int i) {
    Sock *s = &g->socks[i];
    struct sockaddr_in a;
    int l = sizeof(a);
    int n = recvfrom(g_msock[i], (char *)g_nettmp, sizeof(g_nettmp), 0, (struct sockaddr *)&a, &l);
    if (n < 0) {

        return;
    }
    lock_();
    if (ring_free(s->rhead, s->rtail, SOCK_RX) >= (u32)n + 8) {
        u32 ip = a.sin_addr.s_addr;
        u16 port = a.sin_port, len = (u16)n;
        ring_put(g->sock_rx[i], SOCK_RX, &s->rhead, &ip, 4);
        ring_put(g->sock_rx[i], SOCK_RX, &s->rhead, &port, 2);
        ring_put(g->sock_rx[i], SOCK_RX, &s->rhead, &len, 2);
        ring_put(g->sock_rx[i], SOCK_RX, &s->rhead, g_nettmp, n);
    }
    unlock_();
    wake_all();
}

typedef struct { int sock; u32 ip; u16 id, seq, len; u8 data[1472]; } IcmpReq;

static HANDLE(WINAPI *g_icmp_create)(void);
static DWORD(WINAPI *g_icmp_echo)(HANDLE, u32, void *, WORD, void *, void *, DWORD, DWORD);
static BOOL(WINAPI *g_icmp_close)(HANDLE);

static u16 icmp_checksum(const u8 *p, u32 n) {
    u32 sum = 0;
    for (u32 i = 0; i + 1 < n; i += 2) sum += p[i] << 8 | p[i + 1];
    if (n & 1) sum += p[n - 1] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (u16)~sum;
}

static DWORD WINAPI icmp_thread(LPVOID arg) {
    IcmpReq *q = arg;
    static const u32 reply_size = 1472 + 64;
    u8 *reply = malloc(reply_size), *pkt = malloc(1472 + 28);
    HANDLE h = g_icmp_create();

    if (h != INVALID_HANDLE_VALUE && g_icmp_echo(h, q->ip, q->data, q->len, NULL, reply, reply_size, 4000) && !*(u32 *)(reply + 4)) {
        u32 from = *(u32 *)reply;
        u32 dlen = *(u16 *)(reply + 12);
        u8 ttl = reply[20];
        Sock *s = &g->socks[q->sock];
        int raw = s->icmp == 2;
        u8 *icmp = pkt + (raw ? 20 : 0);
        if (dlen > 1472) dlen = 1472;
        icmp[0] = icmp[1] = icmp[2] = icmp[3] = 0;
        icmp[4] = (u8)(q->id >> 8); icmp[5] = (u8)q->id;
        icmp[6] = (u8)(q->seq >> 8); icmp[7] = (u8)q->seq;
        memcpy(icmp + 8, *(u8 **)(reply + 16), dlen);
        u16 ck = icmp_checksum(icmp, 8 + dlen);
        icmp[2] = (u8)(ck >> 8); icmp[3] = (u8)ck;
        u32 total = 8 + dlen + (raw ? 20 : 0);
        if (raw) {
            memset(pkt, 0, 20);
            pkt[0] = 0x45;
            pkt[2] = (u8)(total >> 8); pkt[3] = (u8)total;
            pkt[8] = ttl;
            pkt[9] = 1;
            memcpy(pkt + 12, &from, 4);
        }
        lock_();
        if (s->state && s->icmp && ring_free(s->rhead, s->rtail, SOCK_RX) >= total + 8) {
            u16 port = 0, l = (u16)total;
            ring_put(g->sock_rx[q->sock], SOCK_RX, &s->rhead, &from, 4);
            ring_put(g->sock_rx[q->sock], SOCK_RX, &s->rhead, &port, 2);
            ring_put(g->sock_rx[q->sock], SOCK_RX, &s->rhead, &l, 2);
            ring_put(g->sock_rx[q->sock], SOCK_RX, &s->rhead, pkt, total);
        }
        unlock_();
        wake_all();
    }
    if (h != INVALID_HANDLE_VALUE) g_icmp_close(h);
    free(reply); free(pkt); free(q);
    return 0;
}

static void net_icmp_send(int i, u32 ip, const u8 *msg, u32 len) {
    if (len < 8 || msg[0] != 8) return;
    if (!g_icmp_echo) {
        HMODULE m = LoadLibraryW(L"iphlpapi.dll");
        if (!m) return;
        g_icmp_create = (void *)GetProcAddress(m, "IcmpCreateFile");
        g_icmp_close = (void *)GetProcAddress(m, "IcmpCloseHandle");
        g_icmp_echo = (void *)GetProcAddress(m, "IcmpSendEcho");
        if (!g_icmp_create || !g_icmp_close || !g_icmp_echo) { g_icmp_echo = NULL; return; }
    }
    IcmpReq *q = malloc(sizeof(*q));
    if (!q) return;
    q->sock = i;
    q->ip = ip;
    q->id = (u16)(msg[4] << 8 | msg[5]);
    q->seq = (u16)(msg[6] << 8 | msg[7]);
    q->len = (u16)(len - 8 > sizeof(q->data) ? sizeof(q->data) : len - 8);
    memcpy(q->data, msg + 8, q->len);
    HANDLE t = CreateThread(NULL, 1 << 16, icmp_thread, q, 0, NULL);
    if (t) CloseHandle(t);
    else free(q);
}

static void net_write(int i) {
    Sock *s = &g->socks[i];
    u8 *ring = g->sock_tx[i];
    if (s->type == 2) {
        while (ring_used(s->thead, s->ttail, SOCK_TX) >= 8) {
            u32 ip;
            u16 port, len;
            ring_get(ring, SOCK_TX, s->ttail, &ip, 4);
            ring_get(ring, SOCK_TX, (s->ttail + 4) % SOCK_TX, &port, 2);
            ring_get(ring, SOCK_TX, (s->ttail + 6) % SOCK_TX, &len, 2);
            ring_get(ring, SOCK_TX, (s->ttail + 8) % SOCK_TX, g_nettmp, len);
            struct sockaddr_in a;
            memset(&a, 0, sizeof(a));
            a.sin_family = AF_INET;
            a.sin_addr.s_addr = ip;
            a.sin_port = port;
            if (s->icmp) net_icmp_send(i, ip, g_nettmp, len);
            else sendto(g_msock[i], (char *)g_nettmp, len, 0, (struct sockaddr *)&a, sizeof(a));
            lock_();
            s->ttail = (s->ttail + 8 + len) % SOCK_TX;
            unlock_();
        }
        net_update_name(i);
        return;
    }
    u32 used = ring_used(s->thead, s->ttail, SOCK_TX);
    if (!used) return;
    if (used > sizeof(g_nettmp)) used = sizeof(g_nettmp);
    ring_get(ring, SOCK_TX, s->ttail, g_nettmp, used);
    int n = send(g_msock[i], (char *)g_nettmp, used, 0);
    if (n > 0) {
        lock_();
        s->ttail = (s->ttail + n) % SOCK_TX;
        unlock_();
        wake_all();
    } else if (n < 0) {
        int e = WSAGetLastError();
        if (e != WSAEWOULDBLOCK) {
            s->err = wsa_errno(e);
            s->rx_eof = 1;
            lock_();
            s->ttail = s->thead;
            unlock_();
            wake_all();
        }
    }
}

static int net_pump(int timeout_ms) {
    fd_set rd, wr, ex;
    int active = 0, armed = 0;
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&ex);
    for (int i = 0; i < MAX_SOCK; i++) {
        Sock *s = &g->socks[i];
        if (!s->state) continue;
        active++;
        if (s->op) net_do_op(i);
        int tx = ring_used(s->thead, s->ttail, SOCK_TX) != 0;

        lock_();
        int dead = s->state && s->refs <= 0 && (!tx || s->err || s->conn != 2);
        unlock_();
        if (dead) {
            net_close(i);
            active--;
            continue;
        }
        SOCKET k = g_msock[i];
        if (s->type == 2 && tx) {
            k = net_ensure(i);
        }
        if (!k) continue;
        if (s->tx_shut && !tx && !s->shut_sent && s->conn == 2) {
            shutdown(k, SD_SEND);
            s->shut_sent = 1;
        }
        if (s->conn == 1) {
            FD_SET(k, &wr);
            FD_SET(k, &ex);
            armed = 1;
        } else if (s->conn == 3) {
            FD_SET(k, &rd);
            armed = 1;
        } else if (s->type == 2 || s->conn == 2) {
            if (!s->rx_eof && ring_free(s->rhead, s->rtail, SOCK_RX) > 2048) { FD_SET(k, &rd); armed = 1; }
            if (tx) { FD_SET(k, &wr); armed = 1; }
        }
    }
    int ready = 0;
    if (armed) {
        struct timeval tv = {0, 0};
        ready = select(0, &rd, &wr, &ex, &tv);
    }
    if (ready <= 0) {

        if (active && timeout_ms) {
            HANDLE w[2] = {g_events[MAX_PROC - 1], g_net_event};
            WSAResetEvent(g_net_event);
            WaitForMultipleObjects(g_net_event ? 2 : 1, w, FALSE, 50);
        }
        return active;
    }
    for (int i = 0; i < MAX_SOCK; i++) {
        Sock *s = &g->socks[i];
        SOCKET k = g_msock[i];
        if (!s->state || !k) continue;
        if (s->conn == 1) {
            if (FD_ISSET(k, &ex)) {
                int e = 0, l = sizeof(e);
                getsockopt(k, SOL_SOCKET, SO_ERROR, (char *)&e, &l);
                s->err = wsa_errno(e ? e : WSAECONNREFUSED);
                s->conn = 0;
                s->rx_eof = 1;
                wake_all();
            } else if (FD_ISSET(k, &wr)) {
                s->conn = 2;
                net_update_name(i);
                wake_all();
            }
            continue;
        }
        if (FD_ISSET(k, &rd)) {
            if (s->conn == 3) net_accept(i);
            else if (s->type == 2) net_read_dgram(i);
            else net_read_stream(i);
        }
        if (FD_ISSET(k, &wr)) net_write(i);
    }
    return active;
}

static void net_reset(void) {
    for (int i = 0; i < MAX_SOCK; i++) {
        if (g_msock[i]) closesocket(g_msock[i]);
        g_msock[i] = 0;
    }
    memset(g->socks, 0, sizeof(g->socks));
}

static void net_refresh_ifs(void) {
    static DWORD(WINAPI * get)(void *, ULONG *);
    static u8 buf[16384];
    if (!get) {
        HMODULE ip = LoadLibraryW(L"iphlpapi.dll");
        if (ip) get = (DWORD(WINAPI *)(void *, ULONG *))GetProcAddress(ip, "GetAdaptersInfo");
    }
    static NetIf out[MAX_NETIF];
    int n = 0, wifi = 0, eth = 0;
    memset(out, 0, sizeof(out));
    strcpy(out[0].name, "lo");
    out[0].ip = inet_addr("127.0.0.1");
    out[0].mask = inet_addr("255.0.0.0");
    out[0].loopback = 1;
    n = 1;
    ULONG len = sizeof(buf);
    if (get && !get(buf, &len)) {
        for (u8 *a = buf; a && n < MAX_NETIF; a = *(u8 **)a) {
            u32 addrlen = *(u32 *)(a + 400);
            u32 type = *(u32 *)(a + 416);
            u32 ip = inet_addr((char *)a + 432);
            if (!ip || ip == INADDR_NONE) continue;
            NetIf *f = &out[n++];
            if (type == 71) _snprintf(f->name, sizeof(f->name), "wlan%d", wifi++);
            else _snprintf(f->name, sizeof(f->name), "eth%d", eth++);
            f->ip = ip;
            f->mask = inet_addr((char *)a + 448);
            u32 gw = inet_addr((char *)a + 472);
            f->gw = gw == INADDR_NONE ? 0 : gw;
            memcpy(f->mac, a + 404, addrlen < 6 ? addrlen : 6);
        }
    }
    lock_();
    memcpy(g->netifs, out, sizeof(out));
    g->nnetif = n;
    unlock_();
}

static void write_resolv_conf(void) {
    static char out[512];
    net_refresh_ifs();
    int n = 0;
    HMODULE ip = LoadLibraryW(L"iphlpapi.dll");
    if (ip) {
        DWORD(WINAPI * get)(void *, ULONG *) = (DWORD(WINAPI *)(void *, ULONG *))GetProcAddress(ip, "GetNetworkParams");
        static u8 buf[8192];
        ULONG len = sizeof(buf);
        if (get && !get(buf, &len)) {

            u8 *entry = buf + 268;
            for (int i = 0; entry && i < 3; i++) {
                char *addr = (char *)entry + 4;
                if (addr[0] && strcmp(addr, "0.0.0.0")) n += _snprintf(out + n, sizeof(out) - n, "nameserver %.15s\n", addr);
                entry = *(u8 **)entry;
            }
        }
    }
    if (!n) n = _snprintf(out, sizeof(out), "nameserver 1.1.1.1\n");
    HANDLE h;
    IO_STATUS_BLOCK io;
    if (NT_OK(nt_open("/etc/resolv.conf", GENERIC_WRITE | FILE_WRITE_EA, FILE_OVERWRITE_IF_, FO_NONDIR, FILE_ATTRIBUTE_NORMAL, &h))) {
        TxMeta m = {S_IFREG_ | 0644, 0, 0};
        NtWriteFile(h, NULL, NULL, NULL, &io, out, n, NULL, NULL);
        meta_set(h, &m);
        NtClose(h);
    }
}
