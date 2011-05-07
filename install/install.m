#define _log(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#include <common/common.h>
#include <libtar.h>
#include <lzma.h>
#include <pthread.h>
#include <zlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <notify.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <fts.h>
#include <signal.h>
#import <Foundation/Foundation.h>

bool GSSystemHasCapability(NSString *capability);

extern void do_copy(char *, char *, ssize_t (*)(int, const void *, size_t));
extern void init();
extern void finish();

static size_t written_bytes;
static void (*set_progress)(float);
static NSMutableArray *to_load;

static void wrote_bytes(ssize_t bytes) {
    static size_t last = 0;
    written_bytes += bytes;
    if(written_bytes - last > 100000) {
        //_log("written_bytes = %zd", written_bytes);
        last = written_bytes;

        // xxx figure this out
        float total = 36938240.0f;
        set_progress(written_bytes / total);
    }
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

static int run(char **args) {
    fprintf(stderr, "run:");
    for(char **p = args; *p; p++) {
        fprintf(stderr, " %s", *p);
    }
    fprintf(stderr, "\n");

    pid_t pid;
    int stat;
    if(posix_spawn(&pid, args[0], NULL, NULL, args, NULL)) return 255;
    if(pid != waitpid(pid, &stat, 0)) return 254;
    if(!WIFEXITED(stat)) return 253;
    return WEXITSTATUS(stat);
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

static int lzmaopen(const char *path, int oflag, int foo) {
    int realfd = open(path, O_RDONLY);
    _assert(realfd != -1);
    off_t size = lseek(realfd, 0, SEEK_END);
    void *ptr = mmap(NULL, (size_t) size, PROT_READ, MAP_SHARED, realfd, 0);
    _assert(ptr != MAP_FAILED);

    lzma_stream *strm = malloc(sizeof(*strm));
    *strm = (lzma_stream) LZMA_STREAM_INIT;

    _assert_zero(lzma_stream_decoder(strm, 64*1024*1024, 0));

    strm->avail_in = size;
    strm->next_in = ptr;

    return (int) strm;
}

static int lzmaclose(int fd) {
    return 0;
}

static ssize_t lzmaread(int fd, void *buf, size_t len) {
    lzma_stream *strm = (void *) fd;
    strm->next_out = (void *) buf;
    strm->avail_out = len;
    while(strm->avail_in > 0 && strm->avail_out > 0) {
        if(lzma_code(strm, LZMA_RUN)) break;
    }

    size_t br = len - strm->avail_out;
    wrote_bytes(br);
    return br;
}

tartype_t xztype = { (openfunc_t) lzmaopen, (closefunc_t) lzmaclose, (readfunc_t) lzmaread, (writefunc_t) NULL };

static void extract(const char *fn, bool use_null) {
    _log("extracting %s", fn);
    TAR *tar;
    // TAR_VERBOSE
    if(tar_open(&tar, (char *) fn, &xztype, O_RDONLY, 0, TAR_GNU)) {
        _log("could not open %s: %s", fn, strerror(errno));
        exit(3);
    }
    while(!th_read(tar)) {
        char *pathname = th_get_pathname(tar);
        while(*pathname == '.' || *pathname == '/') pathname++;
        if(use_null && (
#define O(x) !memcmp(pathname, x, strlen(x))
            O("Applications") ||
            //O("Library") ||
            //O("System") ||
            //O("bin") ||
            //O("sbin") ||
            //O("usr") ||
            //O("private/etc") ||
            0
#undef O
        )) {
            chdir("/private/var/null/");
        } else {
            chdir("/");
        }
        tar_extract_file(tar, pathname);
        if(strstr(pathname, "LaunchDaemons/") && strstr(pathname, ".plist")) {
            _log("queueing ld %s", pathname);
            [to_load addObject:[NSData dataWithBytes:pathname length:strlen(pathname)+1]];
        }
    }

    tar_close(tar);
}

static void remount() {
    _log("remount...");
    _assert_zero(run((char *[]) {"/sbin/mount", "-u", "-o", "rw,suid,dev", "/", NULL}));

    NSString *string = _assert([NSString stringWithContentsOfFile:@"/etc/fstab" encoding:NSUTF8StringEncoding error:NULL]);
    string = [string stringByReplacingOccurrencesOfString:@",nosuid,nodev" withString:@""];
    string = [string stringByReplacingOccurrencesOfString:@" ro " withString:@" rw "];
    _assert([string writeToFile:@"/etc/fstab" atomically:YES encoding:NSUTF8StringEncoding error:NULL]);
}

// returns whether the plist existed
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

static void finish_up() {
    int ret = symlink("/boot/untether", "/usr/libexec/dirhelper");
    if(errno != EEXIST) _assert_zero(ret);
    TIME(_assert_zero(run((char *[]) {"/boot/mount_nulls", NULL})));
    for(NSData *pathname in to_load) {
        chdir("/");
        run((char *[]) {"/bin/launchctl", "load", (char *) [pathname bytes], NULL});
    }
}

void do_install(void (*set_progress_)(float)) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    set_progress = set_progress_;
    to_load = [NSMutableArray array];
    
    chdir("/");
    _log("do_install");
    TIME(remount());
    TIME(dok48());
    TIME(add_afc2());
    TIME(extract("/tmp/starstuff.tar.xz", false));
    TIME(extract("/tmp/freeze.tar.xz", true));
    TIME(finish_up());
    TIME(sync());
    _log("final written_bytes = %zd", written_bytes);
}

