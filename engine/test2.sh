#!/bin/sh
pass=0; fail=0
t() {
    case "$3" in
        *"$2"*) echo "PASS $1"; pass=$((pass+1)) ;;
        *) echo "FAIL $1: wanted [$2] got [$(echo "$3" | tail -n 3)]"; fail=$((fail+1)) ;;
    esac
}
s=$(date +%s)
out=$(apk update 2>&1); t apk-update "distinct packages" "$out"
out=$(apk add --no-progress --force-overwrite htop vim nano curl python3 git 2>&1); rc=$?
echo "$out" | grep -E 'WARNING|ERROR|errors' | head -n 5
t apk-add-rc 0 "$rc"
t apk-add-clean OK: "$(echo "$out" | tail -n 1)"
echo "install took $(( $(date +%s) - s ))s"
t apk-info 1 "$(apk info -e vim | grep -c vim)"
t vim-edit "hello vim" "$(vim -es -c 'call setline(1,"hello vim")' -c 'wq! /tmp/v.txt' >/dev/null 2>&1; cat /tmp/v.txt; rm -f /tmp/v.txt)"
t nano-version "GNU nano" "$(nano --version)"
t curl-https "1793" "$(curl -sS https://dl-cdn.alpinelinux.org/alpine/last-updated 2>&1)"
t python-math 1267650600228229401496703205376 "$(python3 -c 'print(2**100)' 2>&1)"
t python-json '{"a": [1, 2]}' "$(python3 -c 'import json; print(json.dumps({"a":[1,2]}))' 2>&1)"
t python-ssl "OpenSSL" "$(python3 -c 'import ssl; print(ssl.OPENSSL_VERSION)' 2>&1)"
t python-subprocess hi "$(python3 -c 'import subprocess; print(subprocess.check_output(["echo","hi"]).decode())' 2>&1)"
t python-https 1793 "$(python3 -c 'import urllib.request as u; print(u.urlopen("https://dl-cdn.alpinelinux.org/alpine/last-updated").read().decode())' 2>&1)"
cd /tmp; rm -rf gt; mkdir gt; cd gt
git init -q . 2>&1
git config user.email a@b.c; git config user.name t
echo x > f; git add f 2>&1
t git-commit "1 file changed" "$(git commit -m first 2>&1)"
t git-log first "$(git log --oneline 2>&1)"
cd /; rm -rf /tmp/gt
t ps-lists-self ps "$(ps -o pid,comm 2>&1)"
t proc-meminfo MemTotal "$(cat /proc/meminfo)"
t proc-cpu 6 "$(grep -c processor /proc/cpuinfo)"
t top-batch "Mem" "$(top -b -n 1 2>&1 | head -n 5)"
t apk-del "OK:" "$(apk del --no-progress nano 2>&1 | tail -n 1)"
t apk-del-gone 1 "$([ -e /usr/bin/nano ] && echo 0 || echo 1)"
echo "RESULT pass=$pass fail=$fail"
df -h / 2>/dev/null | tail -n 1
du -sh /usr 2>/dev/null
