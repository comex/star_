#!/bin/sh
if [ "x$1" = "x" ]; then echo "no"; exit 1; fi
for i in "$1"/*; do echo $i; lzma -k $i; done
rsync -avz --progress -e ssh "$1" liranuna.com:html/a/bs/"$1"

