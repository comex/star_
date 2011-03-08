#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <notify.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "inject.h"
#include "bzlib.h"
#include <common/common.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

#ifdef TINY
static void do_nothing_with(CFTypeRef r) {}
#define CFRelease do_nothing_with
#endif

extern char dylib[];
extern unsigned int dylib_len;

static void update_state();
static const char *state;
static CFStringRef error_string;

static double get_progress() { return 0; }

static struct request {
    CFStringRef url;
    const char *output;
    union {
        struct {};
        struct {
            CFReadStreamRef read_stream;
            bz_stream bz;
            int out_fd;
            bool finished;
            size_t content_length;
        };
    };
} requests[] = {
    { CFSTR("http://google.com/install"), "/tmp/install", {}},
    { CFSTR("http://google.com/foo"), "/tmp/foo", {}}
}, *const requests_end = requests + sizeof(requests)/sizeof(*requests);

static void pause_it() {
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream) {
            CFRelease(r->read_stream);
            r->read_stream = NULL;
        }
    }
    state = "paused";
    update_state();
}

static void handle_error(struct request *r, CFStringRef s) {
    if(s) {
        if(error_string) CFRelease(error_string);
        error_string = s;
    } else {
        r->finished = true;
    }
    BZ2_bzDecompressEnd(&r->bz);
    CFRelease(r->read_stream);
    r->read_stream = NULL;
    close(r->out_fd);
    r->out_fd = 0;
    if(s) {
        state = NULL;
        pause_it();
    }
}

static void request_callback(CFReadStreamRef stream, CFStreamEventType event_type, void *info) {
    struct request *r = info;
    switch(event_type) {
    case kCFStreamEventOpenCompleted:
        {
        // we need to record the content-length
        // also, if the server ignored a range request, we might be at the wrong file position, so we have to check
        CFHTTPMessageRef response = (void *) CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
        CFIndex code = CFHTTPMessageGetResponseStatusCode(response);
        if(code == 200) {
            CFStringRef cl = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Length"));
            if(!cl) {
                handle_error(r, CFSTR("server fails"));
            } else {
                r->content_length = CFStringGetIntValue(cl);
                CFRelease(cl);
            }
        } else if(code == 206) {
#ifndef TINY
            // partial content
            off_t off = 0;
            CFStringRef range = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Range"));
            if(range) {
                if(kCFCompareEqualTo == CFStringCompareWithOptions(range, CFSTR("bytes "), CFRangeMake(0, 6), kCFCompareCaseInsensitive)) {
                    CFRange r = CFStringFind(range, CFSTR("-"), 0);
                    if(r.length != 0 && r.location > 6) {
                        CFStringRef sub = CFStringCreateWithSubstring(NULL, range, CFRangeMake(6, r.location - 6));
                        off = (off_t) CFStringGetIntValue(sub);
                        CFRelease(sub);
                    }
                }
                CFRelease(range);
            }
            if(off != lseek(r->out_fd, 0, SEEK_SET)) {
                handle_error(r, CFSTR("server fails")); 
            }
#endif
        } // error responses are not reported as "completed"
        break;
    case kCFStreamEventErrorOccurred:
        {
        CFErrorRef error = _assert(CFReadStreamCopyError(stream));
        handle_error(r, CFErrorCopyDescription(error));
        CFRelease(error);
        }
        break;
    case kCFStreamEventEndEncountered:
        if(r->read_stream) { // end of the bz2 file but no BZ_STREAM_END
            handle_error(r, CFSTR("compressed data was truncated"));
        }
        break;
    case kCFStreamEventHasBytesAvailable:
        while(CFReadStreamHasBytesAvailable(r->read_stream)) {
            static char compressed[8192], uncompressed[16384];
            CFIndex idx = CFReadStreamRead(r->read_stream, (void *) compressed, sizeof(compressed));
            if(idx == -1) {
                handle_error(r, CFSTR("Huh?"));
            } else {

                r->bz.avail_in = (unsigned int) idx;
                r->bz.next_in = compressed;
                r->bz.avail_out = sizeof(uncompressed);
                r->bz.next_out = uncompressed;
                while(r->bz.avail_in > 0) {
                    int result = BZ2_bzDecompress(&r->bz);
                    if(result == BZ_STREAM_END) {
                        // we're done
                        handle_error(r, NULL);
                    } else if(result != BZ_OK) {
                        CFStringRef error;
                        switch(result) {
                        case BZ_CONFIG_ERROR:
                        case BZ_SEQUENCE_ERROR:
                        case BZ_PARAM_ERROR:
                            _assert_zero(result); // I screwed up
                        case BZ_MEM_ERROR:
                            error = CFSTR("BZ2 memory error");
                            break;
                        case BZ_DATA_ERROR:
                            error = CFSTR("BZ2 data error (corrupt file)");
                            break;
                        case BZ_DATA_ERROR_MAGIC:
                            error = CFSTR("BZ2 data error (corrupt file, you might be behind a crappy transparent proxy)");
                            break;
                        default:
                            error = CFSTR("unknown BZ2 error");
                            break;
                        }
                        handle_error(r, error);
                        break;
                    }

                    size_t towrite = sizeof(uncompressed) - r->bz.avail_out;
                    _assert((size_t) write(r->out_fd, uncompressed, towrite) == towrite);
                    update_state();
                }
            }
        }
        break;
        }
    }
}


static void init_requests() {
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream) continue;

        _assert_zero(BZ2_bzDecompressInit(&r->bz, 0, 0));

        CFURLRef url = _assert(CFURLCreateWithString(NULL, r->url, NULL));
        CFHTTPMessageRef message = _assert(CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), url, kCFHTTPVersion1_1));

        if(r->out_fd) {
            CFStringRef range = CFStringCreateWithFormat(NULL, NULL, CFSTR("bytes %d-%d"), (int) lseek(r->out_fd, 0, SEEK_CUR), (int) r->content_length - 1);
            CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Range"), range);
            CFRelease(range);
            /*XXX*/
        } else {
            r->out_fd = _assert(open(r->output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        }

        r->read_stream = _assert(CFReadStreamCreateForHTTPRequest(NULL, message));
        // creating the read stream should succeed, but there might be an error later
        r->finished = false;

        CFStreamClientContext context;
        memset(&context, 0, sizeof(context));
        context.info = r;
        CFReadStreamSetClient(r->read_stream, kCFStreamEventHasBytesAvailable | kCFStreamEventOpenCompleted | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, request_callback, &context);
        
        CFRelease(url);
        CFRelease(message);
    }
    state = "downloading";
    update_state();
}

static pid_t find_springboard() {
    static int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t length;
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), NULL, &length, NULL, 0));
    struct kinfo_proc *proc = malloc(length);
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), proc, &length, NULL, 0));
    for(size_t i = 0; i < length; i++) {
        if(!strncmp(proc[i].kp_proc.p_comm, "SpringBoard", sizeof(proc[i].kp_proc.p_comm))) {
            pid_t result = proc[i].kp_proc.p_pid;
            free(proc);
            return result;
        }
    }
    abort();
}

static void init_state() {
    notify_post("locutus.go-away");
    int fd = open("/tmp/locutus.state", O_RDWR | O_CREAT, 0666);
    _assert(fd >= 0);
    _assert_zero(flock(fd, LOCK_EX));
    if(0 == lseek(fd, 0, SEEK_END)) {
        // we created it
    } else {
        lseek(fd, 0, SEEK_SET);
        FILE *fp = _assert(fdopen(fd, "r"));
        char mode[16]; int pid; double progress;
        if(2 != fscanf(fp, "%16s %d", &mode[0], &pid)) {
            // corrupt; do nothing and assume it's dead
        } else if(!strcmp(mode, "installing")) {
            // we don't want to stop it during installation
            exit(0);
        } else {
            // we can kill it
            kill((pid_t) pid, SIGTERM);
        }
    }
    flock(fd, LOCK_UN);
    close(fd);
}

static void update_state() {
    int fd = open("/tmp/locutus.state", O_RDWR | O_CREAT, 0666);
    _assert(fd >= 0);
    _assert_zero(flock(fd, LOCK_EX));
    _assert_zero(ftruncate(fd, 0));
    FILE *fp = fdopen(fd, "w");
    _assert(fp);

    char error[128];
    if(error_string) {
        CFStringGetCString(error_string, error, sizeof(error), kCFStringEncodingUTF8);
    } else {
        strcpy(error, "ok");
    }

    fprintf(fp, "%s %d %f %s\n", state, (int) getpid(), get_progress(), error);
    error_string = NULL;

    flock(fd, LOCK_UN);
    close(fd);
}

static void pause_callback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef user_info) {
    if(state && !strcmp(state, "paused")) {
        init_requests();
    } else {
        pause_it();
    }
}

static void cancel_callback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef user_info) {
    state = "bye";
    update_state();
    exit(0);
}

int main() {
    syslog(LOG_EMERG, "omg hax\n");
    printf("omg hax\n");
    return 0;

    uint32_t one = 1;
    _assert_zero(sysctlbyname("security.mac.vnode_enforce", NULL, NULL, &one, sizeof(one)));

    init_state();

    CFNotificationCenterRef center = CFNotificationCenterGetDarwinNotifyCenter();
    CFNotificationCenterAddObserver(center, NULL, pause_callback, CFSTR("locutus.pause"), NULL, 0);
    CFNotificationCenterAddObserver(center, NULL, cancel_callback, CFSTR("locutus.cancel"), NULL, 0);

    // dump our load
    const char *name = tempnam("/tmp/", "locutus.dylib-");
    int dylib_fd = open(name, O_WRONLY | O_CREAT, 0644);
    _assert(dylib_fd >= 0);

    for(unsigned int i = 0; i < dylib_len - 32; i++) {
        if(!memcmp(dylib + i, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 32)) {
            memcpy(dylib + i, name, 32);
        }
    }

    _assert(write(dylib_fd, dylib, dylib_len) == (ssize_t) dylib_len);
    close(dylib_fd);

    pid_t pid = find_springboard();
    printf("pid = %d\n", (int) pid);

    inject(pid, name);

    unlink(name);

    printf("OK\n");
    init_requests();
    CFRunLoopRun();

    return 0;
}
