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

//#define TINY

#ifdef TINY
static void do_nothing_with(CFTypeRef r) {}
#define CFRelease do_nothing_with
#endif

extern char dylib[];
extern unsigned int dylib_len;

static void update_state(const char *state, CFStringRef err);
static void run_install();

static double progress = 0.0;
static bool paused;
static size_t downloaded_bytes;

static struct request {
    CFStringRef url;
    const char *output;
    bool is_bz2;
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
    { CFSTR("http://a.qoid.us/test.bz2"), "/tmp/install", true, {}},
    { CFSTR("http://a.qoid.us/test"), "/tmp/foo", false, {}}
}, *const requests_end = requests + sizeof(requests)/sizeof(*requests);

static void did_download(size_t bytes) {
    fprintf(stderr, "did_download %zd\n", bytes);
    downloaded_bytes += bytes;

    size_t total = 0;
    for(struct request *r = requests; r < requests_end; r++) {
        if(!r->content_length) {
            goto nevermind; // stay at 0
        }
        total += r->content_length;
    }
    progress = (double) downloaded_bytes / total;

    nevermind:

    update_state("downloading", NULL);
}

static void pause_it(CFStringRef err) {
    paused = true;
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream) {
            CFReadStreamClose(r->read_stream);
            CFRelease(r->read_stream);
            r->read_stream = NULL;
        }
    }
    update_state("paused", err);
}

// handle an error or completion
static void handle_error(struct request *r, CFStringRef err) {
    if(r->is_bz2) {
        BZ2_bzDecompressEnd(&r->bz);
    }
    close(r->out_fd);
    r->out_fd = 0;
    if(err) {
        pause_it(err);
    } else {
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
        /*if(CFErrorGetDomain(error) == kCFErrorDomainCFNetwork) {

        }*/
        handle_error(r, description);
        CFRelease(error);
        }
        break;
    case kCFStreamEventEndEncountered:
        if(r->is_bz2) {
            if(r->read_stream) { // end of the bz2 file but no BZ_STREAM_END
                handle_error(r, CFSTR("compressed data was truncated"));
            }
        } else {
            handle_error(r, NULL);
        }
        break;
    case kCFStreamEventHasBytesAvailable:
        if(!r->content_length) {
            // we need to record the content-length
            // also, if the server ignored a range request, we might be at the wrong file position, so we have to check
            CFHTTPMessageRef response = (void *) CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
            if(!response) break;
            CFIndex code = CFHTTPMessageGetResponseStatusCode(response);
            if(code == 200) {
                CFStringRef cl = CFHTTPMessageCopyHeaderFieldValue(response, CFSTR("Content-Length"));
                if(!cl) {
                    handle_error(r, CFSTR("server fails"));
                    break;
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
                    break;
                }
                #endif
            } else {
                handle_error(r, CFStringCreateWithFormat(NULL, NULL, CFSTR("HTTP response code %d"), (int) code));
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

            if(!r->is_bz2) {
                _assert((CFIndex) write(r->out_fd, compressed, idx) == idx);
            } else {
                r->bz.avail_in = (unsigned int) idx;
                r->bz.next_in = compressed;
                r->bz.avail_out = sizeof(uncompressed);
                r->bz.next_out = uncompressed;
                while(r->bz.avail_in > 0) {
                    int result = BZ2_bzDecompress(&r->bz);
                    if(result != BZ_STREAM_END && result != BZ_OK) {
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
                        goto end;
                    }

                    size_t towrite = sizeof(uncompressed) - r->bz.avail_out;
                    _assert((size_t) write(r->out_fd, uncompressed, towrite) == towrite);
                    if(result == BZ_STREAM_END) {
                        // we're done
                        handle_error(r, NULL);
                        goto end;
                    }
                }
            }
        }
        end:
        did_download(written);
        break;
    }
}


static void init_requests() {
    paused = false;
    for(struct request *r = requests; r < requests_end; r++) {
        if(r->read_stream) continue;

        if(r->is_bz2) { 
            _assert_zero(BZ2_bzDecompressInit(&r->bz, 0, 0));
        }

        CFURLRef url = _assert(CFURLCreateWithString(NULL, r->url, NULL));
        CFHTTPMessageRef message = _assert(CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), url, kCFHTTPVersion1_1));
        CFRelease(url);

        if(r->out_fd) {
            #ifdef TINY
            lseek(r->out_fd, 0, SEEK_SET);
            #else
            CFStringRef range = CFStringCreateWithFormat(NULL, NULL, CFSTR("bytes %d-%d"), (int) lseek(r->out_fd, 0, SEEK_CUR), (int) r->content_length - 1);
            CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Range"), range);
            CFRelease(range);
            #endif
        } else {
            r->out_fd = _assert(open(r->output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        }

        r->read_stream = _assert(CFReadStreamCreateForHTTPRequest(NULL, message));
        CFRelease(message);
        // creating the read stream should succeed, but there might be an error later
        r->finished = false;

        static CFStreamClientContext context;
        context.info = r;
        CFReadStreamSetClient(r->read_stream, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, request_callback, &context);

        CFReadStreamScheduleWithRunLoop(r->read_stream, CFRunLoopGetMain(), kCFRunLoopCommonModes);
        CFReadStreamOpen(r->read_stream);
        
    }
    update_state("downloading", NULL);
}

static void run_install() {
    signal(SIGUSR1, SIG_IGN);
    progress = 0.0;
    update_state("installing", NULL);
    fprintf(stderr, "installing or something\n");
    exit(0);
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
        char buf[10];
        int pid;
        if(pread(fd, buf, sizeof(buf), 0) != -1 && (pid = atoi(buf))) {
            kill((pid_t) pid, SIGUSR1);
            if(kill((pid_t) pid, SIGUSR1) == 0) {
                // it stayed alive - so it's installing and we should give up
                exit(0);
            }
        }
    }
    flock(fd, LOCK_UN);
    close(fd);
}

static void update_state(const char *state, CFStringRef err) {

    int fd = open("/tmp/locutus.state", O_RDWR | O_CREAT, 0666);
    _assert(fd >= 0);
    _assert_zero(flock(fd, LOCK_EX));
    _assert_zero(ftruncate(fd, 0));
    FILE *fp = fdopen(fd, "w");
    _assert(fp);

    char errs[128];
    if(err) {
        CFStringGetCString(err, errs, sizeof(errs), kCFStringEncodingUTF8);
        CFRelease(err);
    } else {
        strcpy(errs, "ok");
    }

    fprintf(stderr, "%d %s %f %s\n", (int) getpid(), state, progress, errs);

    flock(fd, LOCK_UN);
    close(fd);
}

int main() {
    //syslog(LOG_EMERG, "omg hax\n");
    //printf("omg hax\n");
    //return 0;

    uint32_t one = 1;
    _assert_zero(sysctlbyname("security.mac.vnode_enforce", NULL, NULL, &one, sizeof(one)));

    init_state();

    int ignored;
    notify_register_dispatch("locutus.pause", &ignored, dispatch_get_main_queue(), ^(int token) {
        if(paused) {
            init_requests();
        } else {
            pause_it(NULL);
        }
    });
    notify_register_dispatch("locutus.cancel", &ignored, dispatch_get_main_queue(), ^(int token) {
        update_state("bye", NULL);
        // maybe delete the temporary files?
        exit(0);
    });
        
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
