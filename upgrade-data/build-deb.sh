#!/bin/sh
set -e
if [ -e upgrade_data ]; then cd upgrade_data; fi
# sanity check
if [ ! -e DEBIAN ]; then echo "omg what"; exit 1; fi
if [ "`whoami`" != "root" ]; then
    # so the output is ours
    touch upgrade-data.deb
    # lol fabricate
    cat ../config/keyz.txt ../data/data lol_mkdir DEBIAN/* >/dev/null
    exec ./fakeroot.sh "$0" "$@"
fi
rm -rf root
mkdir root
cp -a DEBIAN root/
rm -rf root/DEBIAN/.*.sw[op]
mkdir -p root/usr/share/_data
grep KernelCache ../config/keyz.txt > root/usr/share/_data/keyz.txt
cp ../data/data lol_mkdir root/usr/share/_data/
dpkg-deb -b root upgrade-data.deb
rm -rf root
