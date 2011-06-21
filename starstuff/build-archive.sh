#!/bin/sh
set -xe
root="$1/root"
mkdir -p "$root"/{DEBIAN,boot,private/var/null}
cp -a "$1"/union_prelink.dylib "$root"/boot/union_prelink.dylib
cp -a "$1"/../catalog/untether "$root"/boot/untether
cp -a ../white/white_loader "$root"/boot/white_loader
cp -a mount_nulls "$root"/boot/mount_nulls
cat >"$root"/DEBIAN/control << EOF
Package: $3
Essential: yes
Priority: required
Maintainer: comex <comexk@gmail.com>
Section: System
Architecture: iphoneos-arm
Version: 1
Description: Untether and other files
EOF
# note that the below script may be running with stuff only in union mounts...
cat >"$root"/DEBIAN/extrainst_ << EOF
#!/bin/sh
cd /
find . -maxdepth 4 -xdev -type d -print0 | (cd /private/var/null; xargs -0 mkdir -p) 
ln -ns /boot/untether /usr/libexec/dirhelper 2>/dev/null
exec /boot/mount_nulls
EOF
chmod 755 "$root"/DEBIAN/extrainst_
./fauxsu.sh dpkg-deb -b -z9 "$root" "$2".deb
