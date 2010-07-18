# Usage: python ipsw.py [the ipsw to import]
import zipfile, plistlib, shutil, sys, os, tempfile, atexit

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
fbs = os.path.realpath('FirmwareBundles')
configdata = os.path.realpath('../configdata.py')
out_root = os.path.realpath('../bs')
tmpdir = tempfile.mkdtemp()
print 'tmpdir:', tmpdir
os.chdir(tmpdir)

z = zipfile.ZipFile(input_path, 'r', zipfile.ZIP_DEFLATED)
nl = z.namelist()
#print nl
pl = plistlib.readPlistFromString(z.read('Restore.plist'))
identifier = '%s_%s_%s' % (pl['ProductType'], pl['ProductVersion'], pl['ProductBuildVersion'])
short_identifier = '%s_%s' % (pl['ProductType'], pl['ProductVersion'])
output = os.path.join(out_root, identifier)
os.mkdir(output)
pwnage_pl_fn = '%s/%s.bundle/Info.plist' % (fbs, identifier)
system('plutil -convert xml1 %s' % pwnage_pl_fn)
pwnage_pl = plistlib.readPlist(pwnage_pl_fn)
kc = pwnage_pl['FirmwarePatches']['KernelCache']

print 'kernelcache...'
z.extract(kc['File'])
system('xpwntool %s tempkc.e -k %s -iv %s -decrypt' % (kc['File'], kc['Key'], kc['IV'] )) #!
os.unlink(kc['File'])
system('xpwntool tempkc.e %s/kern' % output) #!
os.unlink('tempkc.e')

print 'root filesystem...'
system('unzip -q -o -j "%s" %s' % (input_path, pwnage_pl['RootFilesystem'])) # for speed
system('vfdecrypt -i %s -k %s -o temproot.dmg' % (pwnage_pl['RootFilesystem'], pwnage_pl['RootFilesystemKey']))
os.mkdir('mnt')
system('hdiutil attach -noverify -mountpoint mnt temproot.dmg')
shutil.copy('mnt/usr/sbin/scutil', '%s/scutil' % output)
os.chmod('%s/scutil' % output, 0755)
shutil.copy('mnt/sbin/launchd', '%s/launchd' % output)
os.chmod('%s/launchd' % output, 0755)
system('hdiutil detach mnt')
os.unlink('temproot.dmg')
os.unlink(pwnage_pl['RootFilesystem'])
p = os.popen('lipo -info %s/scutil' % output)
stuff = p.read().strip()
if stuff.endswith('armv6'):
    arch = 'armv6'
elif stuff.endswith('armv7'):
    arch = 'armv7' 
else:
    raise Exception('I don\'t know how to interpret: ' + stuff)
if '3.1.' in identifier:
    arch += '_3.1.x'
else:
    arch += '_3.2+'

# allow for customization.
if not eval('{%s}' % open(configdata).read()).has_key(identifier):
    new = '''
'*X*': { '<': '.*A*', },
    '''.strip().replace('*I*', identifier).replace('*A*', arch).replace('*X*', short_identifier)
    open(configdata, 'a').write(new + '\n')

# clean up
os.rmdir('mnt')
os.chdir('/')
os.rmdir(tmpdir)
