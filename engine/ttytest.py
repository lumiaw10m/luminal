import re
import socket
import sys
import time

s = socket.create_connection((sys.argv[1], int(sys.argv[2])), timeout=10)
s.setblocking(False)

def read(sec=1.0):
    out = b""
    end = time.time() + sec
    while time.time() < end:
        try:
            d = s.recv(65536)
            if not d:
                break
            out += d
            end = max(end, time.time() + 0.25)
        except BlockingIOError:
            time.sleep(0.02)
    return out.decode("utf8", "replace")

def send(b, sec=1.0):
    s.sendall(b if isinstance(b, bytes) else b.encode())
    return read(sec)

ANSI = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]|\xff.|\r")
checks = []

def check(name, cond, got):
    checks.append(cond)
    print(("PASS " if cond else "FAIL ") + name + ("" if cond else ": " + repr(got)[-300:]))

banner = read(2.5)
check("prompt", "#" in banner or "$" in banner, banner)
o = send("echo hi from tty\r")
check("echo", "hi from tty" in ANSI.sub("", o.split("\n", 1)[-1]), o)
o = send("[ -t 0 ] && echo isatty\r")
check("isatty", "\nisatty" in ANSI.sub("", o) or "isatty\n" in ANSI.sub("", o).split("&&")[-1], o)
o = send("stty size\r")
check("stty size", "40 100" in o, o)
o = send("echo abc\x7f\x7fxy\r")
check("line edit backspace", "axy" in ANSI.sub("", o).split("\n")[-2] if o.count("\n") > 1 else "axy" in o, o)
send("echo one\r")
o = send(b"\x1b[A\r")
check("history up arrow", ANSI.sub("", o).count("one") >= 1, o)
t = time.time()
send("sleep 30\r", 0.8)
o = send(b"\x03", 1.0)
check("ctrl-c stops sleep", time.time() - t < 5 and "#" in o, o)
o = send("echo survived $?\r")
check("shell survives ctrl-c", "survived 130" in o, o)
send("while :; do :; done\r", 0.8)
o = send(b"\x03", 1.5)
o2 = send("echo looped $?\r")
check("ctrl-c stops busy loop in shell", "\nlooped" in ANSI.sub("", o2), o + o2)
send("yes | cat > /dev/null &\r", 0.8)
o = send("jobs\r")
check("background job listed", "Running" in o, o)
send("kill %1\r")
o = send("ls /bin | wc -l\r")
check("pipeline in tty", re.search(r"\b\d{2,3}\b", ANSI.sub("", o)) is not None, o)
o = send("vi /tmp/vitest\r", 1.5)
check("vi opens full screen", "\x1b[" in o, o)
send(b"ihello vi\x1b", 0.7)
send(":wq\r", 1.0)
o = send("cat /tmp/vitest\r")
check("vi saved file", "hello vi" in o, o)
send("rm /tmp/vitest\r")
o = send("exit\r", 1.5)
try:
    gone = s.recv(1) == b""
except BlockingIOError:
    gone = False
except OSError:
    gone = True
check("exit closes connection", gone, o)
print(f"RESULT {sum(checks)}/{len(checks)}")
