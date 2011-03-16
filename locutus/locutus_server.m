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

@interface MyIcon : UIView {
}
@end

static id icon;
static notify_handler_t sk_handler;
static id icon_controller;

@interface SBIconLabel {
}
-(void)setText:(id)text;
@end

@interface SBDownloadingIcon {
}
-(id)initWithLeafIdentifier:(id)leafIdentifier;
-(void)setDisplayedIconImage:(id)image;
@end

@interface SBIconController {
}
+(id)sharedInstance;
-(void)addNewIconToDesignatedLocation:(id)icon animate:(BOOL)animate scrollToList:(BOOL)list saveIconState:(BOOL)save;
-(void)removeIcon:(id)icon animate:(BOOL)animate;
@end

@implementation MyIcon
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
    fprintf(stderr, "s/k\n");
    // do something ...
    if(icon) {
        [icon_controller removeIcon:icon animate:YES];
    }
    notify_cancel(token_);
};

__attribute__((constructor))
static void init() {
    int token;
    fprintf(stderr, "I'm alive\n");

    class_setSuperclass([MyIcon class], objc_getClass("SBDownloadingIcon"));

    icon_controller = [objc_getClass("SBIconController") sharedInstance];

    notify_register_dispatch("locutus.go-away", &token, dispatch_get_main_queue(), sk_handler);

    notify_register_dispatch("locutus.updated-state", &token, dispatch_get_main_queue(), ^(int token_) {
        NSCharacterSet *whitespace = [NSCharacterSet whitespaceCharacterSet];
        NSScanner *scanner = [NSScanner scannerWithString:[NSString stringWithContentsOfFile:@"/tmp/locutus.state" encoding:NSUTF8StringEncoding error:nil]];

        int pid; [scanner scanInt:&pid];
        NSString *state; [scanner scanUpToCharactersFromSet:whitespace intoString:&state];
        float progress; [scanner scanFloat:&progress];
        NSString *err; [scanner scanUpToCharactersFromSet:whitespace intoString:&err];

        id label;
        object_getInstanceVariable(icon, "_label", (void **) &label);
        [label setText:_(state)];
        if(err) {
            UIAlertView *av = [[UIAlertView alloc] initWithTitle:@"" message:err delegate:icon cancelButtonTitle:_(@"DATA_PLAN_FAILED_TRY_LATER") otherButtonTitles:_(@"DATA_PLAN_FAILED_TRY_AGAIN"), nil];
        }
    });
    
    notify_register_dispatch("locutus.got-Cydia.png", &token, dispatch_get_main_queue(), ^(int token_) {
        UIImage *image = [UIImage imageWithContentsOfFile:@"/tmp/Cydia.png"];
        [icon setDisplayedIconImage:image];
    });

    icon = [[MyIcon alloc] initWithLeafIdentifier:@"com.saurik.Cydia"];
    NSLog(@"%@", icon);
    [icon_controller addNewIconToDesignatedLocation:icon animate:YES scrollToList:YES saveIconState:NO];
}
