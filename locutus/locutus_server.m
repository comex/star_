#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <dispatch/dispatch.h>
#include <notify.h>
#import <UIKit/UIKit.h>
#include <objc/runtime.h>

static NSString *const bundle_identifier = @"com.saurik.Cydia.notreally";

static notify_handler_t sk_handler;
static int tokens[3];

static Class MyIcon;

static id icon;
static id icon_controller;
static id icon_model;
// Remove Cancel
// Cancel Retry
// quit   pause
static UIAlertView *alert_view;
static NSString *display_name;
static bool is_installing;

static inline NSString *_(NSString *key) {
    NSString *r = [[NSBundle mainBundle] localizedStringForKey:key value:nil table:@"SpringBoard"];
    NSLog(@"_(%@) = %@", key, r);
    return r;
}

@interface SBIconLabel {
}
-(void)setText:(id)text;
@end

@interface SBDownloadingIcon : NSObject {
}
-(id)initWithLeafIdentifier:(id)leafIdentifier;
-(void)setDisplayedIconImage:(id)image; // new
-(void)remove;
-(void)setDelegate:(id)delegate;
-(void)setDownload:(id)download;
-(void)updateDisplayName;
-(id)darkenedIconImage:(id)image alpha:(float)alpha;
-(void)setShowsCloseBox:(BOOL)showsCloseBox;
@end

@interface SBIconController {
}
+(id)sharedInstance;
-(void)addNewIconToDesignatedLocation:(id)icon animate:(BOOL)animate scrollToList:(BOOL)list saveIconState:(BOOL)save;
-(void)setIconToReveal:(id)icon;
//-(void)iconUninstall:(id)icon; // also uninstallIcon:, uninstallIcon:animated:; lots of choice!  this works but we want remove above instead
@end

@interface SBIconModel {
}
+(id)sharedInstance;
-(void)addIcon:(id)icon;
//-(void)removeIcon:(id)icon;
@end

static notify_handler_t sk_handler = ^(int token) {
    NSLog(@"s/k");
    [alert_view dismissWithClickedButtonIndex:0 animated:YES];
    // do something ...
    for(int i = 0; i < 3; i++) {
        notify_cancel(tokens[i]);
    }
    [icon remove];
    icon = nil;
};

static NSString *MyIcon_displayName(id self, SEL sel) {
    return display_name;
}

static NSString *MyIcon_applicationBundleID(id self, SEL sel) {
    return bundle_identifier;
}

static void MyIcon_launch(id self, SEL sel) {
    notify_post("locutus.pause"); 
}

static BOOL MyIcon_allowsUninstall(id self, SEL sel) {
    return !is_installing;
}

static void MyIcon_closeBoxTapped(id self, SEL sel) {
    // don't download behind the user's back
    notify_post("locutus.pause");
    

    alert_view = [[UIAlertView alloc] initWithTitle:@"Remove Download" message:@"Are you sure you want to remove “Cydia”?" delegate:icon cancelButtonTitle:@"Remove" otherButtonTitles:@"Cancel", nil];
    [alert_view show];
}


static void MyIcon_alertView_clickedButtonAtIndex(id self, SEL sel, UIAlertView *alertView, NSInteger buttonIndex) {
    if(buttonIndex == 0) {
        sk_handler(0);
    } else {
        notify_post("locutus.pause"); 
    }
    [alert_view release];
    alert_view = nil;
}

static void set_progress(float progress) {
    id _progress = nil;
    object_getInstanceVariable(icon, "_progress", (void **) &_progress);
    [_progress setProgress:progress];
}

__attribute__((constructor))
static void init() {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSLog(@"i'm alive");

    char name[32];
    sprintf(name, "MyIcon_%p", &init);
    MyIcon = objc_allocateClassPair(objc_getClass("SBDownloadingIcon"), name, 0);
#define OVERRIDE(x) class_replaceMethod(MyIcon, @selector(x), (IMP) MyIcon_##x, "")
    OVERRIDE(displayName);
    OVERRIDE(applicationBundleID);
    OVERRIDE(launch);
    OVERRIDE(allowsUninstall);
    OVERRIDE(closeBoxTapped);
    class_addMethod(MyIcon, @selector(alertView:clickedButtonAtIndex:), (IMP) MyIcon_alertView_clickedButtonAtIndex, "@:@l");
    objc_registerClassPair(MyIcon);

    icon_controller = [objc_getClass("SBIconController") sharedInstance];
    icon_model = [objc_getClass("SBIconModel") sharedInstance];
    NSLog(@"%@", icon_controller);

    NSLog(@"starting notify...");

    notify_register_dispatch("locutus.go-away", &tokens[0], dispatch_get_main_queue(), sk_handler);

    notify_register_dispatch("locutus.updated-state", &tokens[1], dispatch_get_main_queue(), ^(int token) {
        NSString *state = [NSString stringWithContentsOfFile:@"/tmp/locutus.state" encoding:NSUTF8StringEncoding error:nil] ;
        NSLog(@"state = <%@>", state);
        NSArray *bits = [state componentsSeparatedByString:@"\t"];
        if([bits count] < 4) {
            NSLog(@"fail state");
            return;
        }

        NSString *display_key = [bits objectAtIndex:1];
        display_name = _(display_key);

        if(icon) {
            set_progress([[bits objectAtIndex:2] floatValue]);
            [icon updateDisplayName];
        }
        
        if(is_installing = [display_key isEqualToString:@"INSTALLING_ICON_LABEL"]) {
            [icon setShowsCloseBox:NO];
        }

        NSString *err = [bits objectAtIndex:3];
        NSLog(@"err = <%@>", err);
        if(![err isEqualToString:@"ok"]) {
            // don't keep going behind the alert
            [alert_view dismissWithClickedButtonIndex:0 animated:YES]; // shouldn't happen!
            alert_view = [[UIAlertView alloc] initWithTitle:@"There was a problem downloading the jailbreak files." message:err delegate:icon cancelButtonTitle:@"Cancel" otherButtonTitles:@"Retry", nil];
            [alert_view show];
        }

    });

    notify_register_dispatch("locutus.installed", &tokens[2], dispatch_get_main_queue(), ^(int token) {
          
    });

    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"done, MyIcon is now %p", MyIcon);

        icon = [[MyIcon alloc] initWithLeafIdentifier:bundle_identifier];
        [icon setDelegate:icon_controller];
        display_name = _(@"WAITING_ICON_LABEL");
        NSLog(@"%@", icon);
        [icon_model addIcon:icon];
        [icon_controller addNewIconToDesignatedLocation:icon animate:NO scrollToList:NO saveIconState:YES];
        [icon_controller setIconToReveal:icon];
        [icon release];
        
        NSString *icon_url = [[UIScreen mainScreen] scale] > 1.5 ? @"http://a.qoid.us/Cydia@2x.png" : @"http://a.qoid.us/Cydia.png";
        UIImage *icon_image = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:icon_url]]];
        if(icon_image) {
            [icon setDisplayedIconImage:[icon darkenedIconImage:icon_image alpha:0.5]];
        }
    });
    
    [pool release];
}
