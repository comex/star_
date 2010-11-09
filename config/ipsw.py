# Usage: python ipsw.py [the ipsw to import]
import zipfile, plistlib, shutil, sys, os, tempfile, atexit, re

def system(x):
    print x
    if os.system(x):
        raise Exception('Command failed')

def go_away():
    try:
        os.rmdir(output)
    except:
        pass
atexit.register(go_away)

input_path = os.path.realpath(sys.argv[1])
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
configdata = os.path.realpath('configdata.py')
out_root = os.path.realpath('../bs')
keyz = os.path.realpath('keyz.txt')
tmpdir = tempfile.mkdtemp()
print 'tmpdir:', tmpdir
os.chdir(tmpdir)

z = zipfile.ZipFile(input_path, 'r', zipfile.ZIP_DEFLATED)
nl = z.namelist()
#print nl
pl = plistlib.readPlistFromString(z.read('Restore.plist'))
identifier = '%s_%s_%s' % (pl['ProductType'], pl['ProductVersion'], pl['ProductBuildVersion'])
output = os.path.join(out_root, identifier)
os.mkdir(output)

kc_key = kc_iv = fs_key = None
for line in open(keyz):
    bits = re.split(':? ', line.strip())
    if bits[0] == identifier + '.KernelCache':
        kc_key = bits[1]
        kc_iv = bits[2]
    elif bits[0] == identifier + '.fs':
        fs_key = bits[1]

if kc_key is None or kc_iv is None:
    print 'Couldn\'t get keys for %s' % identifier
    sys.exit(1)

if fs_key is None:
    print 'Couldn\'t get a FS key for %s, just doing kernelcache' % identifier

print 'kernelcache...'
if pl.get('KernelCachesByPlatform'):
    kc_name = pl['KernelCachesByPlatform'].values()[0]['Release']
else:
    kc_name = pl['KernelCachesByTarget'][pl['DeviceMap'][0]['BoardConfig'][:3]]['Release']
system('unzip -q -o -j "%s" %s' % (input_path, kc_name))
system('~/xpwnbin/xpwntool %s tempkc.e -k %s -iv %s -decrypt' % (kc_name, kc_key, kc_iv)) #!
os.unlink(kc_name)
system('~/xpwnbin/xpwntool tempkc.e %s/kern' % output) #!
os.unlink('tempkc.e')

if fs_key is not None:
    print 'root filesystem...'
    fs_name = pl['SystemRestoreImages']['User']
    system('unzip -q -o -j "%s" %s' % (input_path, fs_name)) # 'unzip' used for speed
    system('~/xpwnbin/dmg extract %s temproot.img -k %s' % (fs_name, fs_key))
    try:
        system('~/xpwnbin/hfsplus temproot.img extract /System/Library/Caches/com.apple.dyld/dyld_shared_cache_armv7 %s/cache || true' % output)
    except:
        system('~/xpwnbin/hfsplus temproot.img extract /System/Library/Caches/com.apple.dyld/dyld_shared_cache_armv6 %s/cache || true' % output)
        arch = 'armv6'
    else:
        arch = 'armv7'
    system('~/xpwnbin/hfsplus temproot.img extract /usr/lib/dyld %s/dyld' % output)

    os.chmod('%s/dyld' % output, 0755)
    os.unlink('temproot.img')
    os.unlink(fs_name)
else:
    print 'assuming armv7'
    arch = 'armv7'

if '3.1.' in identifier:
    arch += '_3.1.x'
#else:
#    arch += '_3.2+'

# allow for customization.
if not eval('{%s}' % open(configdata).read())['all'].has_key(identifier):
    print 'Add this to configdata.py:'
    new = '''
    '*X*': {
        '<': '.*A*',
        '#kern': {
        },
    },
    '''.strip().replace('*A*', arch).replace('*X*', identifier)
    print new

# clean up
os.chdir('/')
os.rmdir(tmpdir)
