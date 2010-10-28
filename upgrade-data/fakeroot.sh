#!/bin/sh
if (which fakeroot >&/dev/null); then
exec fakeroot "$@"
else
export DYLD_INSERT_LIBRARIES="`pwd`"/fauxsu.dylib
exec "$@"
fi
