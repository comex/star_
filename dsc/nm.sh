#!/bin/bash
set -e
test "x$2" != "x"
if [ -e f/System ]; then umount f; fi
./dsc "$1" f
sleep 1
find f -type f | (while IFS='*' read x
do
echo "$x"
(nm "$x" | fgrep -v ' U ') || true
done) | python -c "import sys, os, struct, anydbm
if os.path.exists('$2'): os.remove('$2')
db = anydbm.open('$2', 'c')
for line in sys.stdin:
    line = line.strip().split()
    if len(line) != 3: continue
    addy, tee, sym = line
    db[sym] = struct.pack('I', int(addy, 16))"

