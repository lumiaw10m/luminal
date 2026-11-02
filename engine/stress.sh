#!/bin/sh
got=0
trap 'got=$((got+1))' USR1
r=0
while [ $r -lt 25 ]; do
    i=0
    while [ $i -lt 20 ]; do
        ( : ) &
        i=$((i+1))
    done
    ( kill -USR1 $$ ) &
    j=0; while [ $j -lt 2000 ]; do j=$((j+1)); done
    [ -e /etc/passwd ]
    wait
    r=$((r+1))
done
out=$(seq 1 200 | while read n; do echo $n; done | tail -n 1)
echo "stress-done rounds=$r usr1=$got seq=$out"
