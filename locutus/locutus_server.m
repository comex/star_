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

@class MyDownload;

static NSString *const bundle_identifier = @"com.saurik.Cydia.notreally";

static notify_handler_t sk_handler;
static int tokens[3];

static MyDownload *download;
static id icon;
static id icon_controller;
static id icon_model;

static UIAlertView *alert_view;

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
-(void)setDisplayedIconImage:(id)image;
-(void)remove;
-(void)setDelegate:(id)delegate;
-(void)setDownload:(id)download;
-(void)updateDisplayName;
-(void)reloadForStatusChange;
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

// setDownload: (unimportant)
// applicationBundleID
// displayName
// _progress->setProgress:
// setDisplayedIcon: or IconImage:

@interface MyDownload : NSObject {
@public
    NSString *_title;
    float _progress;
}
@end
@implementation MyDownload
/*
- (id)status {
    return self;
}
- (BOOL)isFailed {
    // hax
    return YES;
}

- (BOOL)isPaused {
    return NO;
}
*/
- (id)metadata {
    return self;
}
/*
- (NSString *)title {
    return _(_title);
}
*/

/*
- (NSString *)bundleIdentifier {
    return bundle_identifier;
}
*/

- (float)percentComplete {
    return _progress;
}

- (id)activePhase {
    return self;
}

- (int)phaseType {
    return 1;
}

- (id)copy {
    return [self retain];
}

- (id)thumbnailImageData {
    return nil;
}
*/
@end

static notify_handler_t sk_handler = ^(int token) {
    NSLog(@"s/k");
    [alert_view dismissWithClickedButtonIndex:0 animated:YES];
    // do something ...
    for(int i = 0; i < 3; i++) {
        notify_cancel(tokens[i]);
    }
    [icon remove];
    icon = download = nil;
};

__attribute__((constructor))
static void init() {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSLog(@"i'm alive");

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

        /*id label = nil;
        object_getInstanceVariable(icon, "_label", (void **) &label);
        [label setText:_([bits objectAtIndex:1])];*/
        if(download) {
            [download->_title release];
            download->_title = [[bits objectAtIndex:1] retain];
            download->_progress = [[bits objectAtIndex:2] floatValue];
            [icon reloadForStatusChange];
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
    
    notify_register_dispatch("locutus.got-Cydia.png", &tokens[3], dispatch_get_main_queue(), ^(int token) {
        UIImage *image = [UIImage imageWithContentsOfFile:@"/tmp/Cydia.png"];
        [icon setDisplayedIconImage:image];
    });

    NSLog(@"done");

    icon = [[objc_getClass("SBDownloadingIcon") alloc] initWithLeafIdentifier:bundle_identifier];
    [icon setDelegate:icon_controller];
    download = [[MyDownload alloc] init];
    download->_title = @"WAITING_ICON_LABEL";
    [icon setDownload:download];
    [download release];
    NSLog(@"%@", icon);
    [icon_model addIcon:icon];
    [icon_controller addNewIconToDesignatedLocation:icon animate:NO scrollToList:NO saveIconState:YES];
    [icon_controller setIconToReveal:icon];
    [icon release];
    [pool release];
}
