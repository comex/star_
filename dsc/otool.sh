#!/bin/bash
# Usage: ./otool.sh iPad1,1_3.2.cache outpad
set -e
if [ -e f/System ]; then umount f; fi
test x"$2" != "x"
./dsc "$1" f
while [ ! -e f/System ]; do :; done
find f -type f | (while IFS='*' read x
do
echo "$x"
otool -tvv "$x" > "$2"/"$(basename "$x" .dylib)".txt
done)

