#!/bin/sh
if [ -n "`which fakeroot`" ]; then
    exec fakeroot "$@"
else
    export DYLD_INSERT_LIBRARIES="$(greadlink -f $(dirname $0)/fauxsu.dylib):$DYLD_INSERT_LIBRARIES"
    exec "$@"
fi
