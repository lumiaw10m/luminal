import re
import socket
import sys
import time

sys.stdout.reconfigure(encoding="ascii", errors="replace")
s = socket.create_connection((sys.argv[1], int(sys.argv[2])), timeout=10)
s.setblocking(False)

def read(sec):
    out = b""
    end = time.time() + sec
    while time.time() < end:
        try:
            d = s.recv(65536)
            if not d:
                break
            out += d
        except BlockingIOError:
            time.sleep(0.02)
    return out.decode("utf8", "replace")

def send(b, sec):
    s.sendall(b if isinstance(b, bytes) else b.encode())
    return read(sec)

ANSI = re.compile(r"\x1b[\[(][0-9;?]*[A-Za-z]?|\x1b.|\r")
ok = []

def check(name, cond, got=""):
    ok.append(cond)
    print(("PASS " if cond else "FAIL ") + name + ("" if cond else ": " + repr(got)[-500:]))

read(2.5)
o = ANSI.sub("", send("htop\r", 4.0))
check("htop draws meters", "Mem" in o and "Swp" in o, o)
m = re.search(r"Tasks: (\d+)", o)
check("htop counts tasks", m is not None and int(m.group(1)) >= 2, o)
check("htop has a table row for itself", re.search(r"\d+ root .*htop", o) is not None, o)
check("htop shows 6 cpus", all(f"{i}[" in o.replace(" ", "") for i in range(6)), o)
o2 = ANSI.sub("", read(3.0))
check("htop keeps refreshing", len(o2) > 0, o2)
send("q", 1.0)
o = ANSI.sub("", send("echo back $?\r", 1.0))
check("htop quits cleanly", "back 0" in o, o)
o = ANSI.sub("", send("top -b -n 2 -d 1 | head -n 8; echo top-rc\r", 4.0))
check("top batch runs without a crash", "Arithmetic" not in o and "Mem:" in o and "top-rc" in o, o)
o = ANSI.sub("", send("head -n 3 /proc/stat; grep -E 'MemTotal|MemAvailable' /proc/meminfo\r", 1.0))
print(o.strip()[-400:])
o = ANSI.sub("", send("sleep 7 & ps | grep -c sleep\r", 1.5))
check("ps sees a running sleep", re.search(r"\n[1-9]\n?", o) is not None, o)
print(f"RESULT {sum(ok)}/{len(ok)}")
s.close()
