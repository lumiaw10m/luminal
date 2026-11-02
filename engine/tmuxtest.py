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
    print(("PASS " if cond else "FAIL ") + name + ("" if cond else ": " + repr(got)[-400:]))

read(2.5)
send("rm -f /tmp/tt; export TERM=xterm-256color\r")
o = send("tmux -S /tmp/tt new-session -s m\r", 4)
check("attach draws a status line", "[m]" in o and "\x1b[" in o, o)
o = send("echo inside-$((6*7))\r", 2)
check("typing reaches the pane", "inside-42" in o, o)
o = send(b"\x02%", 3)
check("split draws a border", "│" in o or "x" in o or "\x1b(0" in o, o)
o = send("tty\r", 2)
check("second pane has its own pty", "/dev/pts/2" in o, o)
send("sleep 60\r", 1)
o = send(b"\x03", 1.5)
o += send("echo stopped-$?\r", 2)
check("ctrl-c reaches the pane's process", "stopped-130" in o, o)
o = send(b"\x02d", 2.5)
check("detach returns to the shell", "detached" in o, o)
send("sleep 1; echo background-check\r", 2.5)
o = send("tmux -S /tmp/tt ls\r", 2)
check("session survives detach", "m: 1 windows" in o, o)
o = send("tmux -S /tmp/tt attach\r", 3)
check("reattach redraws", "[m]" in o, o)
o = send("vi /tmp/in-tmux\r", 2)
send(b"ihello\x1b", 1)
send(":wq\r", 1.5)
o = send("cat /tmp/in-tmux\r", 2)
check("vi inside tmux", "hello" in o, o)
send("rm /tmp/in-tmux\r", 1)
send(b"\x02", 0.5)
send(":", 0.7)
send("kill-server", 0.7)
send("\r", 2.5)
o = send("echo back-$?\r", 2)
check("kill-server returns to the shell", "back-" in o, o)
print(f"RESULT {sum(ok)}/{len(ok)}")
s.close()
