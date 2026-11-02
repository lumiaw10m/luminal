import ctypes
import os
import select
import shutil
import struct
import subprocess
import time

libc = ctypes.CDLL(None, use_errno=True)
IN_MODIFY, IN_CLOSE_WRITE, IN_MOVED_FROM, IN_MOVED_TO, IN_CREATE, IN_DELETE, IN_DELETE_SELF, IN_ISDIR = 2, 8, 0x40, 0x80, 0x100, 0x200, 0x400, 0x40000000
ok = n = 0

def check(name, cond, extra=""):
    global ok, n
    n += 1
    ok += bool(cond)
    print(("ok  " if cond else "FAIL") + " " + name + (" " + str(extra) if not cond else ""))

def events(fd, wait=2.0):
    out = []
    end = time.time() + wait
    while time.time() < end:
        r, _, _ = select.select([fd], [], [], max(0, end - time.time()))
        if not r:
            break
        d = os.read(fd, 4096)
        i = 0
        while i < len(d):
            wd, mask, cookie, ln = struct.unpack_from("iIII", d, i)
            name = d[i + 16:i + 16 + ln].rstrip(b"\0").decode()
            out.append((wd, mask, cookie, name))
            i += 16 + ln
        end = min(end, time.time() + 0.4)
    return out

def has(ev, mask, name):
    return any(e[1] & mask and e[3] == name for e in ev)

D = "/tmp/ino"
shutil.rmtree(D, ignore_errors=True)
os.makedirs(D)
fd = libc.inotify_init1(os.O_NONBLOCK | os.O_CLOEXEC)
check("init", fd >= 0)
ALL = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE | IN_DELETE_SELF
wd = libc.inotify_add_watch(fd, D.encode(), ALL)
check("add watch", wd > 0, wd)
check("same path, same wd", libc.inotify_add_watch(fd, D.encode(), ALL) == wd)
check("missing path fails", libc.inotify_add_watch(fd, b"/tmp/no-such", ALL) == -1 and ctypes.get_errno() == 2)
check("nothing yet", events(fd, 0.3) == [])
try:
    os.read(fd, 4096)
    check("nonblocking read", False)
except BlockingIOError:
    check("nonblocking read", True)

open(D + "/a.txt", "w").close()
ev = events(fd)
check("create", has(ev, IN_CREATE, "a.txt"), ev)
with open(D + "/a.txt", "w") as f:
    f.write("hello")
ev = events(fd)
check("modify", has(ev, IN_MODIFY, "a.txt"), ev)
check("close write", has(ev, IN_CLOSE_WRITE, "a.txt"), ev)
os.mkdir(D + "/sub")
ev = events(fd)
check("create folder is marked", has(ev, IN_CREATE | IN_ISDIR, "sub") and any(e[1] & IN_ISDIR for e in ev), ev)
os.rename(D + "/a.txt", D + "/b.txt")
ev = events(fd)
fr = [e for e in ev if e[1] & IN_MOVED_FROM]
to = [e for e in ev if e[1] & IN_MOVED_TO]
check("rename", fr and to and fr[0][3] == "a.txt" and to[0][3] == "b.txt", ev)
check("rename cookie pairs", fr and to and fr[0][2] == to[0][2] != 0, ev)
os.unlink(D + "/b.txt")
ev = events(fd)
check("delete", has(ev, IN_DELETE, "b.txt"), ev)

open(D + "/one", "w").close()
open(D + "/two", "w").close()
events(fd, 0.5)
fd2 = libc.inotify_init1(0)
libc.inotify_add_watch(fd2, (D + "/one").encode(), IN_MODIFY)
with open(D + "/two", "w") as f:
    f.write("x")
with open(D + "/one", "w") as f:
    f.write("x")
ev = events(fd2)
check("file watch: its own file, no name", ev and all(e[3] == "" for e in ev), ev)
check("file watch: one event source", len(ev) >= 1, ev)
check("remove watch", libc.inotify_rm_watch(fd2, 1) == 0)
os.close(fd2)
os.close(fd)

fd = libc.inotify_init1(0)
libc.inotify_add_watch(fd, D.encode(), IN_CREATE)
subprocess.Popen(["sh", "-c", "sleep 1; touch " + D + "/late"])
t = time.time()
d = os.read(fd, 4096)
check("blocking read wakes", b"late" in d and time.time() - t < 5, d)
os.close(fd)

fd = libc.inotify_init1(os.O_NONBLOCK)
libc.inotify_add_watch(fd, D.encode(), IN_CREATE)
for i in range(60):
    open(D + "/f%d" % i, "w").close()
ev = events(fd, 3)
check("burst of 60", len(ev) >= 1, len(ev))
print("burst events:", len(ev))
os.close(fd)
shutil.rmtree(D, ignore_errors=True)
print("RESULT %d/%d" % (ok, n))
