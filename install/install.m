#define _log(fmt, args...) NSLog(@fmt, ##args)
#import <Foundation/Foundation.h>
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
extern char ***_NSGetEnviron();

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

static int run(char **argv, char **envp) {
    if(envp == NULL) envp = *_NSGetEnviron();
    fprintf(stderr, "run:");
    for(char **p = argv; *p; p++) {
        fprintf(stderr, " %s", *p);
    }
    fprintf(stderr, "\n");

    pid_t pid;
    int stat;
    if(posix_spawn(&pid, argv[0], NULL, NULL, argv, envp)) return 255;
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

static void extract(const char *fn) {
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
        if(USE_NULL && (
#define O(x) !memcmp(pathname, x, strlen(x))
            O("Applications") ||
            O("Library") ||
            O("System") ||
            //O("bin") ||
            //O("sbin") ||
            O("usr") ||
            O("private/etc") ||
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
    _assert_zero(run((char *[]) {"/sbin/mount", "-u", "-o", "rw,suid,dev", "/", NULL}, NULL));
    // can't do this! _assert_zero(run((char *[]) {"/sbin/mount", "-u", "-o", "rw,suid,dev", "/private/var", NULL}, NULL));

    NSString *string = _assert([NSString stringWithContentsOfFile:@"/etc/fstab" encoding:NSUTF8StringEncoding error:NULL]);
    string = [string stringByReplacingOccurrencesOfString:@",nosuid,nodev" withString:@""];
    string = [string stringByReplacingOccurrencesOfString:@" ro " withString:@" rw "];
    _assert([string writeToFile:@"/etc/fstab" atomically:YES encoding:NSUTF8StringEncoding error:NULL]);
}

// returns whether the plist existed
static bool modify_plist(NSString *filename, void (^func)(id)) {
    NSData *data = [NSData dataWithContentsOfFile:filename];
    if(!data) {
        _log("did not modify %@", filename);
        return false;
    }
    NSPropertyListFormat format;
    NSError *error;
    id plist = [NSPropertyListSerialization propertyListWithData:data options:NSPropertyListMutableContainersAndLeaves format:&format error:&error];
    _assert(plist);

    func(plist);

    NSData *new_data = [NSPropertyListSerialization dataWithPropertyList:plist format:format options:0 error:&error];
    _assert(new_data);

    _assert([new_data writeToFile:filename atomically:YES]);

    _log("modified %@", filename);
    return true;
}

static void dok48() {
    char model[32];
    size_t model_size = sizeof(model);
    _assert_zero(sysctlbyname("hw.model", model, &model_size, NULL, 0));

    NSString *filename = [NSString stringWithFormat:@"/System/Library/CoreServices/SpringBoard.app/%s.plist", model];
    modify_plist(filename, ^(id plist) {
        [[plist objectForKey:@"capabilities"] setObject:[NSNumber numberWithBool:false] forKey:@"hide-non-default-apps"];
    });
}

static void add_afc2() {
    _assert(modify_plist(@"/System/Library/Lockdown/Services.plist", ^(id services) {
        NSDictionary *args = [NSDictionary dictionaryWithObjectsAndKeys:
                                [NSArray arrayWithObjects:@"/usr/libexec/afcd",
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
    return;
    for(NSData *pathname in to_load) {
        chdir("/");
        run((char *[]) {"/bin/launchctl", "load", (char *) [pathname bytes], NULL}, NULL);
    }
}

@interface LSApplicationWorkspace {
}
+(LSApplicationWorkspace *)defaultWorkspace;
-(BOOL)registerApplication:(id)application;
-(BOOL)unregisterApplication:(id)application;
@end


static void uicache() {
    // I am not using uicache because I want loc_s to do the reloading

    // probably not safe:
    NSMutableDictionary *cache = [NSMutableDictionary dictionaryWithContentsOfFile:@"/var/mobile/Library/Caches/com.apple.mobile.installation.plist"];
    if(cache) {
        NSMutableDictionary *cydia = _assert([NSMutableDictionary dictionaryWithContentsOfFile:@"/Applications/Cydia.app/Info.plist"]);
        [cydia setObject:@"/Applications/Cydia.app" forKey:@"Path"];
        [cydia setObject:@"System" forKey:@"ApplicationType"];
        id system = [cache objectForKey:@"System"];
        if([system respondsToSelector:@selector(addObject:)])
            [system addObject:cydia];
        else
            [system setObject:cydia forKey:@"com.saurik.Cydia"];
        [cache writeToFile:@"/var/mobile/Library/Caches/com.apple.mobile.installation.plist" atomically:YES];
    }

    NSURL *url = [NSURL fileURLWithPath:@"/Applications/Cydia.app"];
    LSApplicationWorkspace *workspace = [LSApplicationWorkspace defaultWorkspace];
    [workspace unregisterApplication:url];
    [workspace registerApplication:url];
    
    system("killall installd");
}

// the user can optionally specify this and it will be run after everything else; I use it to install SSH etc
// AutoInstall would work too
static void post_jailbreak() {
    char *fn = "/var/mobile/Media/post-jailbreak";
    if(!access(fn, R_OK)) {
        chmod(fn, 0755);
        run((char *[]) {fn, NULL}, (char *[]) {"PATH=/usr/bin:/usr/sbin:/bin:/sbin", NULL});
    }
}

static void install_starstuff() {
    _assert_zero(run((char *[]) {USE_NULL ? "/private/var/null/usr/bin/dpkg" : "/usr/bin/dpkg", "-i", "/tmp/saffron-jailbreak.deb", NULL}, (char *[]) {"DYLD_LIBRARY_PATH=/private/var/null/usr/lib", "PATH=/private/var/null/usr/bin:/private/var/null/usr/sbin:/usr/bin:/usr/sbin:/bin:/sbin", NULL}));
}

static void make_nulls() {
    mkdir("/private/var/null", 0755);
    mkdir("/private/var/null/Applications", 0755);
    mkdir("/private/var/null/Library", 0755);
    mkdir("/private/var/null/System", 0755);
    mkdir("/private/var/null/usr", 0755);
    mkdir("/private/var/null/private", 0755);
    mkdir("/private/var/null/private/etc", 0755);
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
    TIME(make_nulls());
    TIME(extract("/tmp/freeze.tar.xz"));
    TIME(install_starstuff());
    TIME(finish_up());
    TIME(uicache());
    TIME(post_jailbreak());
    TIME(sync());
    _log("final written_bytes = %zd", written_bytes);
}

