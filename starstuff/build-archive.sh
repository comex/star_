#!/bin/sh
set -xe
root="$1/root"
mkdir -p "$root"/{DEBIAN,boot,private/var/null{,/Applications,/Library,/System,/usr,/private/etc}}
if [ "$4" = "1" ]; then
    cp -a "$1"/union_prelink.dylib "$root"/boot/union_prelink.dylib
fi
cp -a "$1"/../catalog/untether "$root"/boot/untether
cp -a ../white/white_loader "$root"/boot/white_loader
cp -a mount_nulls "$root"/boot/mount_nulls
cat >"$root"/DEBIAN/control << EOF
Package: $3
Name: $6
Priority: required
Maintainer: comex <comexk@gmail.com>
Section: System
Architecture: iphoneos-arm
Version: $5
Tag: role::cydia
Description: support files for JailbreakMe 3.0
Depiction: http://cydia.saurik.com/info/saffron-jailbreak/
EOF
# note that the below script may be running with stuff only in union mounts...
cat >"$root"/DEBIAN/extrainst_ << EOF
#!/bin/sh
if [ "\$1" = install -o "\$1" = upgrade ]; then
    if [ ! -e /usr/libexec/dirhelper ]; then
        ln -ns /boot/untether /usr/libexec/dirhelper || exit 1
    fi
    /boot/mount_nulls >&/dev/null
fi
sync; sync
exit 0
EOF
chmod 755 "$root"/DEBIAN/extrainst_
deb="$root" "$2"-"$5".deb
./fauxsu.sh dpkg-deb -b -z9 "$deb"
tar xvf "$deb" data.tar.gz
gunzip data.tar.gz
mv data.tar "fix-$2-$5".tar
