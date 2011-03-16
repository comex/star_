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

static id icon;
static notify_handler_t sk_handler;
static id icon_controller;
static id icon_model;

@interface SBIconLabel {
}
-(void)setText:(id)text;
@end

@interface SBDownloadingIcon {
}
-(id)initWithLeafIdentifier:(id)leafIdentifier;
-(void)setDisplayedIconImage:(id)image;
-(void)remove;
-(void)setDelegate:(id)delegate;
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
    } else {
        notify_post("locutus.pause"); 
    }
}
@end


static inline NSString *_(NSString *key) {
    return [[NSBundle mainBundle] localizedStringForKey:key value:nil table:nil];
}

static notify_handler_t sk_handler = ^(int token_) {
    NSLog(@"s/k");
    // do something ...
    if(icon) {
        NSLog(@"removing the icon");
        [icon remove];
        icon = nil;
    }
};

__attribute__((constructor))
static void init() {
    int token;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSLog(@"i'm alive");

    char name[32];
    sprintf(name, "MyIcon_%p", &init);
    Class MyIcon = objc_allocateClassPair(objc_getClass("SBDownloadingIcon"), name, 0);
    objc_registerClassPair(MyIcon);

    icon_controller = [objc_getClass("SBIconController") sharedInstance];
    icon_model = [objc_getClass("SBIconModel") sharedInstance];
    NSLog(@"%@", icon_controller);

    NSLog(@"starting notify...");

    notify_register_dispatch("locutus.go-away", &token, dispatch_get_main_queue(), sk_handler);

    notify_register_dispatch("locutus.updated-state", &token, dispatch_get_main_queue(), ^(int token_) {
        NSArray *bits = [[NSString stringWithContentsOfFile:@"/tmp/locutus.state" encoding:NSUTF8StringEncoding error:nil] componentsSeparatedByString:@" "];
        if([bits count] != 4) {
            NSLog(@"fail state");
            return;
        }

        id label = nil;
        object_getInstanceVariable(icon, "_label", (void **) &label);
        [label setText:_([bits objectAtIndex:1])];

        NSString *err = [bits objectAtIndex:3];
        if(![err isEqualToString:@"ok"]) {
            UIAlertView *av = [[UIAlertView alloc] initWithTitle:@"" message:err delegate:[[MyDelegate alloc] init] cancelButtonTitle:_(@"DATA_PLAN_FAILED_TRY_LATER") otherButtonTitles:_(@"DATA_PLAN_FAILED_TRY_AGAIN"), nil];
            [av show];
            [av release];
        }
    });
    
    notify_register_dispatch("locutus.got-Cydia.png", &token, dispatch_get_main_queue(), ^(int token_) {
        UIImage *image = [UIImage imageWithContentsOfFile:@"/tmp/Cydia.png"];
        [icon setDisplayedIconImage:image];
    });

    NSLog(@"done");

    icon = [[MyIcon alloc] initWithLeafIdentifier:@"fcom.saurik.Cydia"];
    [icon setDelegate:icon_controller];
    NSLog(@"%@", icon);
    [icon_model addIcon:icon];
    [icon_controller addNewIconToDesignatedLocation:icon animate:NO scrollToList:NO saveIconState:YES];
    [icon_controller setIconToReveal:icon];
    [icon release];
    [pool release];
}
