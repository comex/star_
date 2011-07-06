#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "inject.h"
#include <common/common.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <pthread.h>
#include <dispatch/dispatch.h>

static const float download_share = 0.50;

// and SB freezing or hanging on black
// blinking alertview of doom
// if Cydia was not actually created *or locutus crashes*, don't sit on installing forever (so we should use a socket - that would also handle the locking issue)
// but when it /doesn't/ crash, Cydia disappears
// Safari is respawning and reloading it - I think this can be avoided by having JavaScript load the pdf

//#define TINY

#ifdef TINY
#define CFRelease(args...) ((args), (void) 0)
#define NSLog(args...) ((args), (void) 0)
#else
extern void NSLog(CFStringRef fmt, ...);
#endif

extern char dylib[];
extern unsigned int dylib_len;

static int listen_sock;
static FILE *state_fp;
static void update_state(const char *state, CFStringRef err);
static void run_install();

static double progress = 0.0;
static bool paused;
static size_t downloaded_bytes;

static struct request {
    CFStringRef url;
    const char *output;
    CFStringRef content_type;
    union {
        struct {};
        struct {
            CFReadStreamRef read_stream;
            int out_fd;
            size_t content_length;
            bool finished;
            bool got_headers;
        };
    };
} requests[] = {
#if 1
    {CFSTR("http://www.jailbreakme.com/saffron/_/saffron-jailbreak-%s-%s.deb"), "/tmp/saffron-jailbreak.deb", CFSTR("application/x-debian-package"), {}},
    {CFSTR("http://www.jailbreakme.com/saffron/_/freeze.tar.xz"), "/tmp/freeze.tar.xz", CFSTR("application/octet-stream"), {}},
    {CFSTR("http://www.jailbreakme.com/saffron/_/install.dylib"), "/tmp/install.dylib", CFSTR("application/octet-stream"), {}},
#else
    {CFSTR("http://a.qoid.us/omgleak/_/saffron-jailbreak-%s-%s.deb"), "/tmp/saffron-jailbreak.deb", CFSTR("application/x-debian-package"), {}},
    //{CFSTR("http://test.saurik.com/dhowett/Cydia-4.1b1-Srk.txz"), "/tmp/freeze.tar.xz", CFSTR("text/plain"), {}},
    {CFSTR("http://a.qoid.us/omgleak/_/freeze.tar.xz"), "/tmp/freeze.tar.xz", CFSTR("application/x-tar"), {}},
    {CFSTR("http://a.qoid.us/omgleak/_/install.dylib"), "/tmp/install.dylib", CFSTR("text/plain"), {}},
#endif
}, *const requests_end = requests + sizeof(requests)/sizeof(*requests);

static void did_download(size_t bytes) {
    downloaded_bytes += bytes;

    size_t total = 0;
    for(struct request *r = requests; r < requests_end; r++) {
        if(!r->content_length) {
            goto nevermind; // stay at 0
        }
        total += r->content_length;
    }
    progress = download_share * ((double) downloaded_bytes / total);

    nevermind:

    update_state("DOWNLOADING_ICON_LABEL", NULL);
}

static void pause_it(CFStringRef err) {
    if(err) NSLog(CFSTR("err: %@"), err);
    paused = true;
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream) {
            CFReadStreamClose(r->read_stream);
            CFRelease(r->read_stream);
            r->read_stream = NULL;
        }
    }
    update_state("PAUSED_ICON_LABEL", err);
}

// handle an error or completion
static void handle_error(struct request *r, CFStringRef err) {
    close(r->out_fd);
    r->out_fd = 0;
    if(err) {
        pause_it(err);
    } else {
        //rename(basename((char *) r->output), r->output);
        r->finished = true;
        CFReadStreamClose(r->read_stream);
        CFRelease(r->read_stream);
        r->read_stream = NULL;

        for(struct request *r = requests; r < requests_end; r++) {
            if(!r->finished) return;
        }
        // We are all finished.
        run_install();
    }
}

static void request_callback(CFReadStreamRef stream, CFStreamEventType event_type, void *info) {
    struct request *r = info;

    switch(event_type) {
    case kCFStreamEventErrorOccurred:
        {
        CFErrorRef error = _assert(CFReadStreamCopyError(stream));
        CFStringRef description = CFErrorCopyDescription(error);
        handle_error(r, description);
        CFRelease(error);
        }
        break;
    case kCFStreamEventEndEncountered:
        {
        size_t actual_length = lseek(r->out_fd, 0, SEEK_CUR);
        NSLog(CFSTR("[%@] %p, %p, %zd, %zd"), r->url, stream, r->read_stream, actual_length, r->content_length);
        if(actual_length != r->content_length) {
            handle_error(r, CFSTR("Truncated"));
        } else {
            handle_error(r, NULL);
        }
        }
        break;
    case kCFStreamEventHasBytesAvailable:
        if(!r->got_headers) {
            r->got_headers = true;

            // we need to record the content-length
            // also, if the server ignored a range request, we might be at the wrong file position, so we have to check
            CFHTTPMessageRef response = (void *) CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
            if(!response) break;
            CFIndex code = CFHTTPMessageGetResponseStatusCode(response);
            NSLog(CFSTR("[%@] status %d"), r->url, (int) code);
            off_t off = 0;
            if(code == 206) {
                // partial content
                CFStringRef range = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Range"));
                if(range) {
                    //NSLog(CFSTR("got range %@"), range);
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
            } else if(code == 200) {
                CFStringRef cl = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Length"));
                if(!cl) {
                    handle_error(r, CFSTR("Server fails (no length)"));
                    break;
                }
                r->content_length = CFStringGetIntValue(cl);
                CFRelease(cl);
            } else {
                handle_error(r, CFStringCreateWithFormat(NULL, NULL, CFSTR("HTTP response code %d"), (int) code));
                break;
            }
            off_t old = lseek(r->out_fd, 0, SEEK_CUR);
            NSLog(CFSTR("seeking to %lld (%lld)"), off, old);
            if(off != lseek(r->out_fd, off, SEEK_SET)) {
                handle_error(r, CFSTR("Server fails (206)")); 
                break;
            }
            downloaded_bytes -= (old - off);
            CFStringRef content_type = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Type"));
            if(!content_type || kCFCompareEqualTo != CFStringCompare(content_type, r->content_type, kCFCompareCaseInsensitive)) {
                NSLog(CFSTR("got %@, expected %@"), content_type, r->content_type);
                
                handle_error(r, CFStringCreateWithFormat(NULL, NULL, CFSTR("Wrong Content-Type; are you on a fail Wi-Fi network?")));
                break;
            }
        }

        // actually read
        size_t written = 0;
        while(CFReadStreamHasBytesAvailable(r->read_stream)) {
            static char compressed[8192], uncompressed[16384];
            CFIndex idx = CFReadStreamRead(r->read_stream, (void *) compressed, sizeof(compressed));
            if(idx == -1) {
                handle_error(r, CFSTR("Huh?"));
                goto end;
            }

            written += (size_t) idx;

            _assert((CFIndex) write(r->out_fd, compressed, idx) == idx);
        }
        end:
        did_download(written);
        break;
    }
}


static void init_requests() {
    paused = false;
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream || r->finished) continue;

        CFURLRef url = _assert(CFURLCreateWithString(NULL, r->url, NULL));
        CFHTTPMessageRef message = _assert(CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), url, kCFHTTPVersion1_1));
        CFRelease(url);

        int off;
        if(r->out_fd && ((off = (int) lseek(r->out_fd, 0, SEEK_CUR)), off)) {
            CFStringRef range = CFStringCreateWithFormat(NULL, NULL, CFSTR("bytes=%d-"), off);
            NSLog(CFSTR("[%@] sending range %@"), r->url, range);
            CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Range"), range);
            CFRelease(range);
        } else {
            nvm:
            r->out_fd = open(/*basename*/((char *) r->output), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            _assert(r->out_fd != -1);
        }

        // creating the read stream should succeed, but there might be an error later
        r->read_stream = _assert(CFReadStreamCreateForHTTPRequest(NULL, message));
        CFRelease(message);
        r->got_headers = false;

        static CFStreamClientContext context;
        context.info = r;
        CFReadStreamSetClient(r->read_stream, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, request_callback, &context);

        CFReadStreamScheduleWithRunLoop(r->read_stream, CFRunLoopGetMain(), kCFRunLoopCommonModes);
        CFReadStreamOpen(r->read_stream);
        
    }
    update_state("DOWNLOADING_ICON_LABEL", NULL);
}

static void set_progress(float progress_) {
    progress = download_share + progress_ * (1.0 - download_share);
    update_state("INSTALLING_ICON_LABEL", NULL);
}

static void run_install() {
    NSLog(CFSTR("running install"));
    signal(SIGUSR1, SIG_IGN);
    set_progress(0.0);
    void *install = dlopen("/tmp/install.dylib", RTLD_LAZY);
    void (*do_install)(void (*set_progress)(float)) = dlsym(install, "do_install");
    do_install(set_progress);
    update_state("`", NULL);
    exit(0);
}

static pid_t find_springboard() {
    pid_t result = 0;
    pid_t my_pid = getpid();

    static int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t length;
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), NULL, &length, NULL, 0));
    struct kinfo_proc *proc = malloc(length);
    _assert(!sysctl(&name[0], sizeof(name) / sizeof(*name), proc, &length, NULL, 0));
    for(size_t i = 0; i < length/sizeof(*proc); i++) {
        struct extern_proc *ep = &proc[i].kp_proc;
        /*if(!strncmp(ep->p_comm, "CommCenter", sizeof(ep->p_comm))) {
            kill(ep->p_pid, SIGKILL);
        }*/
        if(!strncmp(ep->p_comm, "SpringBoard", sizeof(ep->p_comm))) {
            result = ep->p_pid;
        }
    }
    _assert(result);
    return result;
}

static void *read_state(void *sock_) {
    int sock = (int) sock_;
    while(1) {
        char c;
        int result = read(sock, &c, 1);
        fprintf(stderr, "r=%d\n", result);
        if(result == 1) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if(paused) {
                    init_requests();
                } else {
                    pause_it(NULL);
                }
            });
        } else {
            exit(0);
        }
    }
}

static void do_bind() {
    struct sockaddr_in addr;

    listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    _assert(listen_sock != -1);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1021);
    addr.sin_addr.s_addr = htonl(0x7f000001);
    int one = 1;
    _assert_zero(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)));
    _assert_zero(bind(listen_sock, (void *) &addr, sizeof(addr)));
    _assert_zero(listen(listen_sock, 1));
}

static void do_accept() {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int state_sock = accept(listen_sock, (void *) &addr, &addrlen);
    _assert(state_sock != -1);

    pthread_t thread;
    _assert_zero(pthread_create(&thread, NULL, read_state, (void *) state_sock));

    state_fp = _assert(fdopen(state_sock, "w"));
}

static void update_state(const char *state, CFStringRef err) {
    char errs[128];
    char *errp;
    if(err) {
        CFStringGetCString(err, errs, sizeof(errs), kCFStringEncodingUTF8);
        CFRelease(err);
        errp = errs;
    } else {
        errp = "`";
    }

    fprintf(state_fp, "%s\t%f\t%s\n", state, progress, errp);
    fflush(state_fp);
}

int main(int argc, char **argv) {
    if(argc == 0) {
        // make me root and remove sandbox
        syscall(0);
    }

    NSLog(CFSTR("omg hax"));
    //return 0;
    //mkdir("/tmp/locutus-temp", 0755); // might fail
    //_assert_zero(chdir("/tmp/locutus-temp"));

    // dump our load
    const char *name = tempnam("/tmp/", "locutus.dylib-");
    printf("name = %s\n", name);
    int dylib_fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
    
    do_bind();

    inject(pid, name);

    do_accept();

    NSLog(CFSTR("OK"));

    char machine[32], osversion[32];
    size_t size = 32;
    _assert_zero(sysctlbyname("hw.machine", machine, &size, NULL, 0));
    size = 32;
    _assert_zero(sysctlbyname("kern.osversion", osversion, &size, NULL, 0));
    requests[0].url = CFStringCreateWithFormat(NULL, NULL, requests[0].url, machine, osversion);

    init_requests();
    CFRunLoopRun();

    return 0;
}
