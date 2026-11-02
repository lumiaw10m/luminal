import os, socket, threading, time
ok = 0; n = 0
def check(name, cond):
    global ok, n
    n += 1
    ok += bool(cond)
    print(("ok  " if cond else "FAIL") + " " + name)
for p in ("/tmp/ds", "/tmp/dc"):
    try: os.unlink(p)
    except OSError: pass
srv = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM); srv.bind("/tmp/ds")
cli = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM); cli.bind("/tmp/dc")
cli.sendto(b"one", "/tmp/ds"); cli.sendto(b"second", "/tmp/ds")
d, a = srv.recvfrom(100); check("boundaries and sender", d == b"one" and a == "/tmp/dc")
check("second message whole", srv.recv(100) == b"second")
srv.sendto(b"reply", a); check("reply to sender", cli.recv(100) == b"reply")
anon = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM); anon.connect("/tmp/ds"); anon.send(b"conn")
d, a = srv.recvfrom(100); check("connected, unbound sender", d == b"conn" and not a)
anon.sendmsg([b"ab", b"cd"]); d, _, _, _ = srv.recvmsg(100); check("sendmsg and recvmsg", d == b"abcd")
anon.send(b"abcdefgh"); check("truncates to buffer", srv.recv(3) == b"abc")
srv.setblocking(False)
try: srv.recv(10); check("nonblocking EAGAIN", False)
except BlockingIOError: check("nonblocking EAGAIN", True)
srv.setblocking(True)
threading.Timer(0.3, lambda: cli.sendto(b"late", "/tmp/ds")).start()
t = time.time(); check("blocking wakes", srv.recv(10) == b"late" and time.time() - t < 3)
import select
anon.send(b"x"); check("poll sees data", select.select([srv], [], [], 2)[0] == [srv]); srv.recv(10)
pid = os.fork()
if pid == 0:
    anon.send(b"child"); os._exit(0)
os.waitpid(pid, 0); check("across fork", srv.recv(10) == b"child")
ab = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM); ab.bind("\0tx-abs"); cli.sendto(b"abs", "\0tx-abs"); check("abstract name", ab.recv(10) == b"abs")
srv.close()
try: anon.send(b"gone"); check("refused when closed", False)
except OSError as e: check("refused when closed", e.errno == 111)
print("RESULT %d/%d" % (ok, n))
