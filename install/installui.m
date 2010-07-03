#include <stdlib.h>
#import <dumpedUIKit/UIAlertView.h>
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <assert.h>
#include <pthread.h>

@interface Dude : NSObject {
    UIAlertView *progressAlertView;
}
@end

@implementation Dude
- (void)doStuff {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSData *google = [NSData dataWithContentsOfURL:[NSURL URLWithString:@"http://google.com"]];
    [google writeToFile:@"/tmp/google.html" atomically:NO];
    sleep(3);
    exit(0);
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    NSLog(@"alertView clickedButtonAtIndex:%d", (int)buttonIndex);
    if(buttonIndex == 0) {
        // The user hit cancel, just crash.
        // TODO: Figure out how to stop it from reloading.
        abort();
    }
    // Okay, we can keep going.
    progressAlertView = [[UIAlertView alloc] initWithTitle:@"Jailbreaking..." message:@"Sit tight." delegate:nil cancelButtonTitle:nil otherButtonTitles:nil];
    [progressAlertView show]; 
    [NSThread detachNewThreadSelector:@selector(doStuff) toTarget:self withObject:nil];
}

- (void)ridiculousHack {
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
    Dude *dude = [[Dude alloc] init];
    [dude performSelectorOnMainThread:@selector(startWithPort:) withObject:[NSNumber numberWithInt:(int)port] waitUntilDone:NO];
    while(1) {
        [[NSRunLoop currentRunLoop] run];  
        sleep(1);
    }
}
