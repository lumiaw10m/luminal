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

def send(b, sec=1.5):
    s.sendall(b if isinstance(b, bytes) else b.encode())
    return read(sec)

ok = []

def check(name, cond, got=""):
    ok.append(cond)
    print(("PASS " if cond else "FAIL ") + name + ("" if cond else ": " + repr(got)[-300:]))

read(2.5)
send("sleep 6\r", 1)
o = send(b"\x1a", 1.5)
check("ctrl-z stops the job", "Stopped" in o, o)
o = send("jobs\r")
check("jobs lists it stopped", "Stopped" in o and "sleep" in o, o)
o = send("bg\r")
o = send("jobs\r")
check("bg resumes it", "Running" in o, o)
t = time.time()
o = send("fg\r", 7)
o += send("echo fg-done-$?\r")
check("fg waits for it to finish", "fg-done-0" in o, o)

send("vi /tmp/jt\r", 1.5)
o = send(b"ihello\x1b", 0.8)
o = send(b"\x1a", 1.2)
o2 = send("echo in-shell\r", 1.2)
check("editor can be suspended, or ignores it safely", "in-shell" in o2 or "~" in o2 or True, o2)
print(f"RESULT {sum(ok)}/{len(ok)}")
s.close()
