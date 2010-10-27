#!/bin/sh
set -e
if [ "`whoami`" != "root" ]; then touch upgrade-data.deb; exec sudo "$0" "$@"; fdsa; fi
if [ -e upgrade_data ]; then cd upgrade_data; fi
# sanity check
if [ ! -e DEBIAN ]; then echo "omg what"; exit 1; fi
rm -rf root
mkdir root
cp -a DEBIAN root/
rm -rf root/DEBIAN/.*.sw[op]
mkdir -p root/usr/share/data
grep KernelCache ../config/keyz.txt > root/usr/share/data/keyz.txt
cp ../data/data root/usr/share/data/
dpkg-deb -b root upgrade-data.deb
rm -rf root
