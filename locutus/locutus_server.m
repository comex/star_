#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <stdio.h>
//#include <servers/bootstrap.h>
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

static NSString *display_name;

static UIAlertView *alert_view;

static inline NSString *_(NSString *key) {
    NSString *r = [[NSBundle mainBundle] localizedStringForKey:key value:nil table:@"SpringBoard"];
    NSLog(@"_(%@) = %@", key, r);
    return r;
}

// cancelDownload:
// allowsUninstall

@interface SBIconLabel {
}
-(void)setText:(id)text;
@end

@interface SBDownloadingIcon : NSObject {
}
-(id)initWithLeafIdentifier:(id)leafIdentifier;
-(void)setDisplayedIconImage:(id)image; // new
-(void)setDisplayedIcon:(id)image; // old
-(void)remove;
-(void)setDelegate:(id)delegate;
-(void)setDownload:(id)download;
-(void)updateDisplayName;
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

@interface MyDelegate : NSObject {
}
@end
@implementation MyDelegate
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if(buttonIndex == 0) {
        sk_handler(0);
        [alert_view release];
        alert_view = nil;
    } else {
        notify_post("locutus.pause"); 
    }
}
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
    return 1;
}

static void MyIcon_completeUninstall(id self, SEL sel) {
    sk_handler(0); 
}

static void set_progress(float progress) {
    id _progress = nil;
    object_getInstanceVariable(icon, "_progress", (void **) &_progress);
    [_progress setProgress:progress];
}

static void override(SEL sel, IMP imp) {
    method_setImplementation(class_getInstanceMethod(MyIcon, sel), imp);
}
#define OVERRIDE(x) override(@selector(x), (IMP) MyIcon_##x)

__attribute__((constructor))
static void init() {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSLog(@"i'm alive");

    char name[32];
    sprintf(name, "MyIcon_%p", &init);
    MyIcon = objc_allocateClassPair(objc_getClass("SBDownloadingIcon"), name, 0);
    OVERRIDE(displayName);
    OVERRIDE(applicationBundleID);
    OVERRIDE(launch);
    OVERRIDE(allowsUninstall);
    OVERRIDE(completeUninstall);
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

        if(icon) {
            display_name = _([bits objectAtIndex:1]);
            set_progress([[bits objectAtIndex:2] floatValue]);
            [icon updateDisplayName];
        }

        NSString *err = [bits objectAtIndex:3];
        NSLog(@"err = <%@>", err);
        if(![err isEqualToString:@"ok"]) {
            [alert_view dismissWithClickedButtonIndex:0 animated:YES]; // shouldn't happen!
            alert_view = [[UIAlertView alloc] initWithTitle:@"JailbreakMe" message:err delegate:[[MyDelegate alloc] init] cancelButtonTitle:_(@"DATA_PLAN_FAILED_TRY_LATER") otherButtonTitles:_(@"DATA_PLAN_FAILED_TRY_AGAIN"), nil];
            [alert_view show];
        }
    });
    
    notify_register_dispatch("locutus.got-file", &tokens[2], dispatch_get_main_queue(), ^(int token) {
        UIImage *image = [UIImage imageWithContentsOfFile:@"/tmp/Cydia.png"];
        if(image) {
            if([icon respondsToSelector:@selector(setDisplayedIconImage:)]) {
                [icon setDisplayedIconImage:image];
            } else {
                [icon setDisplayedIcon:image];
            }
            notify_cancel(token);
        }
    });

    NSLog(@"done, MyIcon is now %p", MyIcon);

    icon = [MyIcon alloc];
    NSLog(@"omg");
    icon = [icon initWithLeafIdentifier:bundle_identifier];
    [icon setDelegate:icon_controller];
    display_name = _(@"WAITING_ICON_LABEL");
    NSLog(@"%@", icon);
    [icon_model addIcon:icon];
    //[icon setDownload:[[NSObject alloc] init]];
    //[icon setDelegate:[[NSObject alloc] init]];
    [icon_controller addNewIconToDesignatedLocation:icon animate:NO scrollToList:NO saveIconState:YES];
    [icon_controller setIconToReveal:icon];
    [icon release];
    [pool release];
}
