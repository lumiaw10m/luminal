import concurrent.futures
import os
import queue
import signal
import subprocess
import sys
import threading
import time
import urllib.request

results = []

def t(name, ok, got=""):
    results.append(ok)
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else ": " + repr(got)), flush=True)

out = []
th = [threading.Thread(target=lambda i=i: out.append(i)) for i in range(8)]
[x.start() for x in th]
[x.join() for x in th]
t("start-join", sorted(out) == list(range(8)), out)

s = time.time()
th = [threading.Thread(target=time.sleep, args=(1,)) for _ in range(5)]
[x.start() for x in th]
[x.join() for x in th]
d = time.time() - s
t("sleeps-overlap", d < 2.5, d)

lock = threading.Lock()
n = [0]

def add():
    for _ in range(20000):
        with lock:
            n[0] += 1

th = [threading.Thread(target=add) for _ in range(4)]
[x.start() for x in th]
[x.join() for x in th]
t("lock-count", n[0] == 80000, n[0])

q = queue.Queue(maxsize=4)
got = []

def consumer():
    while True:
        v = q.get()
        if v is None:
            return
        got.append(v)

c = threading.Thread(target=consumer)
c.start()
for i in range(2000):
    q.put(i)
q.put(None)
c.join()
t("queue", got == list(range(2000)), len(got))

cv = threading.Condition()
with cv:
    s = time.time()
    r = cv.wait(0.3)
    d = time.time() - s
t("condition-timeout", not r and 0.2 < d < 1.0, d)
ev = threading.Event()
threading.Timer(0.2, ev.set).start()
t("timer-event", ev.wait(3), "not set")

with concurrent.futures.ThreadPoolExecutor(6) as ex:
    res = list(ex.map(lambda v: v * v, range(200)))
t("thread-pool", res == [v * v for v in range(200)])

loc = threading.local()
seen = []

def tls(v):
    loc.x = v
    time.sleep(0.05)
    seen.append(loc.x == v)

th = [threading.Thread(target=tls, args=(i,)) for i in range(6)]
[x.start() for x in th]
[x.join() for x in th]
t("thread-local", all(seen) and len(seen) == 6, seen)

r, w = os.pipe()
ticks = []
stop = []

def ticker():
    while not stop:
        ticks.append(1)
        time.sleep(0.02)

tk = threading.Thread(target=ticker)
tk.start()
threading.Timer(0.5, lambda: os.write(w, b"x")).start()
os.read(r, 1)
stop.append(1)
tk.join()
t("blocking-read-releases", len(ticks) >= 5, len(ticks))

url = "https://dl-cdn.alpinelinux.org/alpine/last-updated"
with concurrent.futures.ThreadPoolExecutor(4) as ex:
    res = list(ex.map(lambda _: urllib.request.urlopen(url, timeout=30).read(), range(4)))
t("https-threads", all(x.strip().isdigit() for x in res), res)

out = []
th = [threading.Thread(target=lambda: out.append(subprocess.check_output(["echo", "hi"]))) for _ in range(4)]
[x.start() for x in th]
[x.join() for x in th]
t("subprocess-from-threads", out == [b"hi\n"] * 4, out)

pid = os.fork()
if pid == 0:

    x = []
    k = threading.Thread(target=lambda: x.append(threading.active_count()))
    k.start()
    k.join()
    os._exit(0 if x else 3)
_, st = os.waitpid(pid, 0)
t("fork-then-thread", st == 0, st)

hits = []
signal.signal(signal.SIGUSR1, lambda *a: hits.append(1))
stop = []
bg = threading.Thread(target=lambda: [time.sleep(0.01) for _ in iter(lambda: bool(stop), True)])
bg.start()
os.kill(os.getpid(), signal.SIGUSR1)
time.sleep(0.3)
stop.append(1)
bg.join()
t("signal-with-threads", hits == [1], hits)

d = threading.Thread(target=time.sleep, args=(60,), daemon=True)
d.start()
print(f"RESULT {sum(results)}/{len(results)}", flush=True)
sys.stdout.flush()
os._exit(0 if all(results) else 1)
