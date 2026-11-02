#!/bin/sh
ms() { cut -d' ' -f1 /proc/uptime 2>/dev/null; }
run() {
    s=$(date +%s)
    i=0
    while [ $i -lt $2 ]; do eval "$3"; i=$((i+1)); done
    e=$(date +%s)
    d=$((e-s)); [ $d -eq 0 ] && d=1
    echo "$1: $2 in ${d}s = $(( $2 * 1000 / d / 1000 ))/s approx"
}
run "fork+exit (subshell)" 200 '( : )'
run "fork+exec (/bin/true)" 100 '/bin/true'
run "stat" 2000 '[ -e /etc/passwd ]'
s=$(date +%s); head -c 20000000 /dev/zero | wc -c >/dev/null; e=$(date +%s); echo "pipe 20MB: $((e-s))s"
s=$(date +%s); head -c 20000000 /dev/zero > /tmp/b.bin; e=$(date +%s); echo "file write 20MB: $((e-s))s"
s=$(date +%s); md5sum /tmp/b.bin >/dev/null; e=$(date +%s); echo "md5 20MB: $((e-s))s"; rm /tmp/b.bin
s=$(date +%s); i=0; while [ $i -lt 200000 ]; do i=$((i+1)); done; e=$(date +%s); echo "shell loop 200k: $((e-s))s"
