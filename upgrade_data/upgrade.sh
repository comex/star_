#!/bin/bash
set -e
platform="$(uname -m)_$(sw_vers -productVersion)_$(sw_vers -buildVersion)"
key="$(fgrep "$platform.KernelCache:" /usr/share/data/keyz.txt  | head -n 1 | cut -d' ' -f 2,3)"
if [ -n "$key" ]; then
    echo "! Couldn't find keys for $platform !"
    echo "Please contact comex or someone."
    exit 1
fi
kc=/System/Library/Caches/com.apple.kernelcaches/kernelcache
if [ ! -e "$kc" ]; then 
    echo "! Couldn't find $kc !"
    exit 1
fi
cd /usr/lib
/usr/share/data/data -i $kc $key
./pf2
if [ -n /usr/lib/libgmalloc.dylib ]; then
    echo "You don't have either star or pf2 installed."
    echo "Installing the latter..."
    /usr/share/data/lol_mkdir
elif [ -n /usr/lib/pf2 ]; then
    echo "You have star installed."
    echo "Upgrading you to pf2..."
fi
