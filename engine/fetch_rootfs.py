import gzip
import hashlib
import io
import os
import sys
import tarfile
import urllib.request

VERSION = "3.24.3"
BASE = f"https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/armv7/alpine-minirootfs-{VERSION}-armv7.tar.gz"
OVERLAY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "rootfs")
EXECUTABLE = ("usr/bin/", "usr/local/bin/", "etc/luminal/")

out = sys.argv[1]
raw = urllib.request.urlopen(BASE).read()
want = urllib.request.urlopen(BASE + ".sha256").read().split()[0].decode()
got = hashlib.sha256(raw).hexdigest()
if want != got:
    sys.exit(f"sha256 mismatch: {got} != {want}")
tar = gzip.decompress(raw)
path = os.path.join(out, "root.tar")
with open(path, "wb") as f:
    f.write(tar)
with tarfile.open(fileobj=io.BytesIO(tar)) as t:
    with open(os.path.join(out, "musl.so"), "wb") as f:
        f.write(t.extractfile("./lib/ld-musl-armhf.so.1").read())

def reset(info):
    info.uid = info.gid = 0
    info.uname = info.gname = "root"
    return info

with tarfile.open(path, "a", format=tarfile.USTAR_FORMAT) as t:
    for root, dirs, files in os.walk(OVERLAY):
        rel = os.path.relpath(root, OVERLAY).replace(os.sep, "/")
        for name in sorted(dirs):
            info = tarfile.TarInfo("./" + (name if rel == "." else f"{rel}/{name}"))
            info.type = tarfile.DIRTYPE
            info.mode = 0o755
            t.addfile(reset(info))
        for name in sorted(files):
            member = name if rel == "." else f"{rel}/{name}"
            with open(os.path.join(root, name), "rb") as f:
                data = f.read().replace(b"\r\n", b"\n")
            info = tarfile.TarInfo("./" + member)
            info.size = len(data)
            info.mode = 0o755 if member.startswith(EXECUTABLE) else 0o644
            t.addfile(reset(info), io.BytesIO(data))
print("ok", got, os.path.getsize(path))
