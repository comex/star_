#!/bin/sh
set -e
class-dump-z -H -o dumpedUIKit /var/sdk/System/Library/Frameworks/UIKit.framework/UIKit
echo '#import <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
typedef void *GSEventRef;' > dumpedUIKit/UIKit-Structs.h
for i in dumpedUIKit/NS*.h; do >$i; done
sed 's/otherButtonTitles:(id)titles/otherButtonTitles:(id)titles, .../g' dumpedUIKit/UIAlertView.h > tmp; mv tmp dumpedUIKit/UIAlertView.h

