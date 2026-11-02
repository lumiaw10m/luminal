import socket
import sys
import time

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

read(2.5)
for line in sys.argv[3:]:
    s.sendall(line.encode() + b"\r")
    print(repr(read(1.5))[-400:])
s.close()
