#include <stdlib.h>
#import <dumpedUIKit/UIAlertView.h>
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>

@interface Dude : NSObject {
    UIAlertView *progressAlertView;
}
@end

@implementation Dude
- (void)doStuff {
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
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Do you want to jailbreak?" message:@"Only do this if you understand the consequences." delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Jailbreak", nil];
    [alertView show];
}
@end

void iui_go(io_connect_t port) {
    // Apparently we get here with a locked thing

    CGFontCacheUnlock();
    NSLog(@"iui_go: %d", (int) port);
    Dude *dude = [[Dude alloc] init];
    [dude performSelectorOnMainThread:@selector(startWithPort:) withObject:[NSNumber numberWithInt:(int)port] waitUntilDone:NO];
    while(1) {
        [[NSRunLoop currentRunLoop] run];  
        sleep(1);
    }
}
