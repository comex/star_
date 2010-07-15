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

@interface Dude : NSObject {
    UIAlertView *progressAlertView;
    UIProgressBar *progressBar;
    NSMutableData *wad;
    long long expectedLength;
    const char *freeze;
    int freeze_len;
}
@end

static Dude *dude;

@implementation Dude
static void set_progress(float progress) {
    [dude->progressBar setProgress:progress];
}

- (void)doStuff {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    void *handle = dlopen("/tmp/install.dylib", RTLD_LAZY);
    if(!handle) abort();
    void (*do_install)(const char *, int, void (*)(float)) = dlsym(handle, "do_install");
    do_install(freeze, freeze_len, set_progress);

    // Um, I guess it worked.
    [[UIApplication sharedApplication] terminateWithSuccess];
}

- (void)bored {
    if([progressAlertView.message isEqualToString:@"This might take a while."]) {
        progressAlertView.message = @"(*yawn*)";
    }
}

- (void)bored2 {
    if([progressAlertView.message isEqualToString:@"(*yawn*)"]) {
        progressAlertView.message = @"(It's only a few megabytes, get a better internet connection!)";
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
    struct wad *sw = [wad bytes];
    unsigned char sha1[20];
    CC_SHA1(&sw->first_part_size, [wad length] - 20, sha1);
    if(memcmp(sha1, sw->sha1, 20)) goto error;
    [[wad subdataWithRange:NSMakeRange(sizeof(struct wad), sw->first_part_size)] writeToFile:@"/tmp/install.dylib" atomically:NO];
    freeze = &sw->data[sizeof(struct wad) + sw->first_part_size];
    freeze_len = [wad length] - sizeof(struct wad) - sw->first_part_size;
    progressAlertView.title = @"Jailbreaking...";
    progressAlertView.message = @"Sit tight.";
    [progressBar setProgress:0.0];
    [NSThread detachNewThreadSelector:@selector(doStuff) toTarget:self withObject:nil];
    return;
    error:

    [progressAlertView hide];
    [progressAlertView release];
    progressAlertView = nil;

    UIAlertView *errorAlertView = [[UIAlertView alloc] initWithTitle:@"Error" message:@"Invalid file received.  Are you on a fail wi-fi connection?" delegate:self cancelButtonTitle:@"Quit" otherButtonTitles:@"Retry", nil];
    [alertView show];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    NSLog(@"alertView clickedButtonAtIndex:%d", (int)buttonIndex);
    if(buttonIndex == 0) {
        // The user hit cancel, just crash.
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
    [NSURLConnection connectionWithRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://ip/wad.bin"]] delegate:self];

    [NSTimer scheduledTimerWithTimeInterval:20 target:self selector:@selector(bored) userInfo:nil repeats:NO];
    [NSTimer scheduledTimerWithTimeInterval:40 target:self selector:@selector(bored2) userInfo:nil repeats:NO];
}

- (void)ridiculousHack {
    return; //XXX
    // Apparently we get here with a locked thing.
    // This is ridiculous, but
    while(1) {
        thread_act_array_t threads;
        mach_msg_type_number_t threadcount;
        assert(!task_threads(mach_task_self(), &threads, &threadcount));
        //NSLog(@"SEMAPHORE = %p", (void *) CONFIG_SEMAPHORE_WAIT_SIGNAL_TRAP);
        for(int i = 0; i < threadcount; i++) {
            thread_t thread = threads[i];
            mach_msg_type_number_t state_count = ARM_THREAD_STATE_COUNT;
            arm_thread_state_t state;
            assert(!thread_get_state(thread, ARM_THREAD_STATE, (thread_state_t) &state, &state_count));
            
            /*for(int j = 0; j < 13; j++) {
                NSLog(@"Thread %d has R%d = %p", i, j, (void *) state.__r[j]);
            }
            NSLog(@"Thread %d has SP = %p", i, (void *) state.__sp);
            NSLog(@"Thread %d has LR = %p", i, (void *) state.__lr);
            NSLog(@"Thread %d has PC = %p", i, (void *) state.__pc);*/

            if(state.__pc == CONFIG_SEMAPHORE_WAIT_SIGNAL_TRAP + 8) {
                void *ptr = (void *) state.__r[5];
                NSLog(@"Unlocking %p", ptr);
                char *p = ptr;
                *((volatile int *) (p - 4 + 0x64)) = 0;
                assert(!pthread_mutex_unlock(ptr));
                return;
            }
        }
        assert(!vm_deallocate(mach_task_self(), (vm_address_t) threads, threadcount * sizeof(thread_t)));
        usleep(100000);
    }
}

- (void)pipidi:(NSNumber *)port_ {
    return; //XXX
    io_connect_t port = (io_connect_t) [port_ intValue];
    system("killall ptpd");
    sleep(1);
    system("killall ptpd");
    sleep(1);
    IOServiceClose(port);
}

- (void)startWithPort:(NSNumber *)port {
    [NSThread detachNewThreadSelector:@selector(pipidi:) toTarget:self withObject:port];
    [NSThread detachNewThreadSelector:@selector(ridiculousHack) toTarget:self withObject:nil];
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Do you want to jailbreak?" message:@"Only do this if you understand the consequences." delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Jailbreak", nil];
    [alertView show];
}
@end


void iui_go(io_connect_t port) {
    NSLog(@"iui_go: %d", (int) port);
    dude = [[Dude alloc] init];
    [dude performSelectorOnMainThread:@selector(startWithPort:) withObject:[NSNumber numberWithInt:(int)port] waitUntilDone:NO];
    while(1) {
        [[NSRunLoop currentRunLoop] run];  
        sleep(1);
    }
}
