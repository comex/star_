#define _log(fmt, args...) logger(fmt "\n", ##args)
static int (*logger)(const char *, ...);
#include <common/common.h>
#include <libtar.h>
#include <lzma.h>
#include <pthread.h>
#include <zlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <notify.h>
#include <stdarg.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <fts.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#import <Foundation/Foundation.h>

bool GSSystemHasCapability(CFStringRef capability);

extern void do_copy(char *, char *, ssize_t (*)(int, const void *, size_t));
extern void init();
extern void finish();
static int written_bytes;

static const char *freeze;
static int freeze_len;
static void (*set_progress)(float);

static void wrote_bytes(ssize_t bytes) {
    written_bytes += bytes;
    // xxx figure this out
    float last = 90123213.0;
    set_progress(written_bytes / last);
}

ssize_t my_write(int fd, const void *buf, size_t len) {
    ssize_t ret = write(fd, buf, len);
    if(ret > 0) wrote_bytes(ret);
    return ret;
}

static void remove_files(const char *path) {
    if(access(path, F_OK)) return;
    char *argv[2], *path_;
    argv[0] = path_ = strdup(path);
    argv[1] = NULL;
    FTS *fts = fts_open(argv, FTS_NOCHDIR | FTS_PHYSICAL, NULL);
    FTSENT *ent;
    while((ent = fts_read(fts))) {
        switch(ent->fts_info) {
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_NSOK:
        case FTS_DEFAULT:
            if(unlink(ent->fts_accpath))
                _log("Unlink %s failed", ent->fts_accpath);
            break;
            
            case FTS_DP:
                if(rmdir(ent->fts_accpath))
                    _log("Rmdir %s failed", ent->fts_accpath);
                break;
            
            case FTS_NS:
            case FTS_ERR: // I'm getting errno=0
                return;
                //AST2(fts_err, path, 0);
                //break;
            }
        }
    free(path_);
}

static inline void copy_file(const char *a, const char *b) {
    int fd1 = open(a, O_RDONLY);
    _assert(fd1, a);
    struct stat st;
    fstat(fd1, &st);
    int fd2 = open(b, O_WRONLY | O_CREAT, st.st_mode);
    _assert(fd2, b);
    _assert_zero(fchmod(fd2, st.st_mode), b);
    _assert_zero(fchown(fd2, 0, 0), b);
    char *buf = malloc(st.st_size);
    _assert(st.st_size == read(fd1, buf, st.st_size), a);
    _assert(st.st_size == my_write(fd2, buf, st.st_size), b);
    free(buf);
    close(fd1);
    close(fd2);
}

static inline void copy_files(const char *from, const char *to, bool copy) {
    DIR *dir = opendir(from);
    char *a = malloc(1025 + strlen(from));
    char *b = malloc(1025 + strlen(to));
    _log("copy_files %s -> %s", from, to);
    struct dirent *ent;
    while((ent = readdir(dir))) {
        if(ent->d_type == DT_REG) {
            sprintf(a, "%s/%s", from, ent->d_name);
            sprintf(b, "%s/%s", to, ent->d_name);
            //printf("%s %s -> %s\n", copy ? "Copy" : "Move", a, b);
            if(copy) {
                copy_file(a, b);
            } else {
                _assert(!rename(a, b));
            }
        }
    }
    free(a);
    free(b);
}

static int launchctl(char *what, char *who) {
    char *args[] = {
        "/bin/launchctl",
        what,
        who,
    NULL };
    pid_t pid;
    int stat;
    posix_spawn(&pid, args[0], NULL, NULL, args, NULL);
    waitpid(pid, &stat, 0);
    return stat;
}

unsigned int config_vnode_patch;

static void qstat(const char *path) {
    struct stat st;
    if(lstat(path, &st)) {
        _log("Could not lstat %s: %s\n", path, strerror(errno));
    } else {
        _log("%s: size %d uid %d gid %d mode %04o flags %d\n", path, (int) st.st_size, (int) st.st_uid, (int) st.st_gid, (int) st.st_mode, (int) st.st_flags);
        _log("again, mode is %d", (int) st.st_mode);
        _log("access is %d", R_OK | W_OK | F_OK | X_OK);
    }
}

struct lzmactx {
    lzma_stream strm;
    uint8_t buf[BUFSIZ];
    char *read_buf;
    int read_len;
};

static int lzmaopen(const char *path, int oflag, int foo) {
    int realfd = open(path, O_RDONLY);
    _assert(realfd != -1);
    off_t size = lseek(realfd, 0, SEEK_SET);
    void *ptr = mmap(NULL, (size_t) size, PROT_READ, MAP_SHARED, fd, 0);
    _assert(ptr != MAP_FAILED);

    struct lzmactx *ctx = malloc(sizeof(struct lzmactx));
    ctx->strm = (lzma_stream) LZMA_STREAM_INIT;
    lzma_ret ret;
    _assert(!lzma_stream_decoder(&ctx->strm, 64*1024*1024, 0));

    ctx->strm.avail_in = size;
    ctx->strm.next_in = ptr;
    ctx->strm.next_out = (void *) ctx->buf;
    ctx->strm.avail_out = BUFSIZ;
    ctx->read_buf = (void *) ctx->buf;
    ctx->read_len = 0;

    return (int) ctx;
}

static int lzmaclose(int fd) {
    return 0;
}

static ssize_t lzmaread(int fd, void *buf_, size_t len) {
    /* what is this crap *doing*?
       why can't I just make buf_ the avail_out?
       */
    struct lzmactx *ctx = (void *) fd;
    char *buf = buf_;
    while(len > 0) {
        if(ctx->read_len > 0) {
            size_t bytes_to_read = len < (size_t) ctx->read_len ? len : (size_t) ctx->read_len;
            memcpy(buf, ctx->read_buf, bytes_to_read);
            buf += bytes_to_read;
            ctx->read_buf += bytes_to_read;
            ctx->read_len -= bytes_to_read;
            len -= bytes_to_read;
            continue;                
        }

        _assert(ctx->strm.avail_in != 0);

        if(ctx->strm.avail_out <= 128) {
            ctx->strm.next_out = ctx->buf;
            ctx->strm.avail_out = BUFSIZ;
            ctx->read_buf = (void *) ctx->buf;
        }

        size_t old_avail = ctx->strm.avail_out;

        if(lzma_code(&ctx->strm, LZMA_RUN)) break;
        ctx->read_len = old_avail - ctx->strm.avail_out;
    }

    ssize_t br = buf - (char *) buf_;
    wrote_bytes(br);
    return br;
}

tartype_t xztype = { (openfunc_t) lzmaopen, (closefunc_t) lzmaclose, (readfunc_t) lzmaread, (writefunc_t) NULL };

static void extract() {
    TAR *tar;
    char *fn = "<buf>";
    // TAR_VERBOSE
    if(tar_open(&tar, fn, &xztype, O_RDONLY, 0, TAR_GNU)) {
        _log("could not open %s: %s", fn, strerror(errno));
        exit(3);
    }
    while(!th_read(tar)) {
        char *pathname = th_get_pathname(tar);
        char *full; asprintf(&full, "/%s", pathname);
        tar_extract_file(tar, full);
        if(strstr(full, "LaunchDaemons/") && strstr(full, ".plist")) {
            _log("loading %s", full);
            launchctl("load", full); 
        }
        free(full);
    }

    tar_close(tar);
}

static void mount_rw(const char *path) {
    char *args[] = {
        "/sbin/mount",
        "-u",
        "-o", "rw,suid,dev", (char *) path,
        NULL};
    pid_t pid, pid2;

    _assert_zero(posix_spawn(&pid, args[0], NULL, NULL, args, NULL));
    int stat;
    waitpid(pid, &stat, 0);
    _assert(WIFEXITED(stat));
}
    
static void remount() {
    _log("remount...");
    mount_rw("/");

    NSString *string = _assert([NSString stringWithContentsOfFile:@"/etc/fstab" encoding:NSUTF8StringEncoding error:NULL]);
    string = [string stringByReplacingOccurrencesOfString:@",nosuid,nodev" withString:@""];
    string = [string stringByReplacingOccurrencesOfString:@" ro " withString:@" rw "];
    _assert([string writeToFile:@"/etc/fstab" atomically:YES encoding:NSUTF8StringEncoding error:NULL]);
}

static bool modify_plist(NSString *filename, void (^func)(id)) {
    NSData *data = [NSData dataWithContentsOfFile:filename];
    if(!data) return false;
    NSPropertyListFormat format;
    NSError *error;
    id plist = [NSPropertyListSerialization propertyListWithData:data options:NSPropertyListMutableContainersAndLeaves format:&format error:&error];
    _assert(plist);

    func(plist);

    NSData *new_data = [NSPropertyListSerialization dataWithPropertyList:plist format:format options:0 error:&error];
    _assert(new_data);

    _assert([new_data writeToFile:filename atomically:YES]);

    return true;
}

static void dok48() {
    char model[32];
    size_t model_size = sizeof(model);
    _assert_zero(sysctlbyname("hw.model", model, &model_size, NULL, 0));

    NSString *filename = [NSString stringWithFormat:@"/System/Library/CoreServices/SpringBoard.app/%s.plist", model];
    if(modify_plist(filename, ^(id plist) {
        [[plist objectForKey:@"capabilities"] setObject:[NSNumber numberWithBool:false] forKey:@"hide-non-default-apps"];
    })) {
        _log("%s.plist modified", model);
    }
}

static void add_afc2() {
    _assert(modify_plist(@"/System/Library/Lockdown/Services.plist", ^(id services) {
        NSDictionary *args = [NSDictionary dictionaryWithObjectsAndKeys:
                                [NSArray arrayWithObjects:@"/usr/lib/afcd",
                                                          @"--lockdown",
                                                          @"-d",
                                                          @"/",
                                                          nil], @"ProgramArguments",
                                [NSNumber numberWithBool:true], @"AllowUnauthenticatedServices",
                                @"com.apple.afc2",              @"Label",
                                                                nil];
        [services setValue:args forKey:@"com.apple.afc2"];
    }));
}


void do_install(const char *freeze_, int freeze_len_, int (*logger_)(const char *, ...), void (*set_progress_)(float)) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    set_progress = set_progress_;
    logger = logger_;
    freeze = freeze_;
    freeze_len = freeze_len_;
    
    chdir("/");
    _log("do_install");
    TIME(remount());
    TIME(dok48());
    TIME(add_afc2());
    TIME(extract());
    _log("extract out.");
    TIME(sync());
    _log("written_bytes = %d", written_bytes);
}

