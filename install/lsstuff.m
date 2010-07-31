#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
@interface LSApplicationWorkspace : NSObject {
}
+(id)defaultWorkspace;
-(id)applicationsAvailableForOpeningDocument:(id)openingDocument;
-(id)operationToOpenResource:(id)openResource usingApplication:(id)application uniqueDocumentIdentifier:(id)identifier userInfo:(id)info delegate:(id)delegate;
-(id)operationToOpenResource:(id)openResource usingApplication:(id)application uniqueDocumentIdentifier:(id)identifier userInfo:(id)info;
-(id)operationToOpenResource:(id)openResource usingApplication:(id)application userInfo:(id)info;
-(BOOL)registerApplication:(id)application;
-(BOOL)unregisterApplication:(id)application;
@end

void register_application(CFStringRef app) {
    [[objc_getClass("LSApplicationWorkspace") defaultWorkspace] registerApplication:[NSURL fileURLWithPath:(NSString *)app]];
}
