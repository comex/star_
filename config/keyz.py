import plistlib, glob, re, sys, cStringIO as StringIO
def importOldStuff():
    iDeviceKeys = plistlib.readPlist('iDeviceKeys.plist')
    for device, data1 in iDeviceKeys.items():
        for version, data2 in data1.items():
            string = device + '_' + version
            if data2.has_key('RootFilesystem'):
                print string + '.fs: ' + data2['RootFilesystem']['RootFilesystemKey']
            for img3, data3 in data2.items():
                if not data3.has_key('IV'): continue
                print string + '.' + img3 + ': ' + data3['Key'] + ' ' + data3['IV']
            print
    for thing in glob.glob('FirmwareBundles/*/Info.plist'):
        data = plistlib.readPlistFromString(open(thing).read().replace('=1.0', '="1.0"').replace('=UTF-8', '="UTF-8"').replace(' -//Apple Computer//DTD PLIST 1.0//EN', ' "-//Apple Computer//DTD PLIST 1.0//EN"').replace(' http://www.apple.com/DTDs/PropertyList-1.0.dtd', ' "http://www.apple.com/DTDs/PropertyList-1.0.dtd"'))
        string = re.sub('_[^_]*$', '', data['Name'])
        print string + '.fs: ' + data['RootFilesystemKey']
        for img3, data3 in data['FirmwarePatches'].items():
            if not hasattr(data3, 'has_key') or not data3.has_key('IV'): continue
            print string + '.' + img3 + ': ' + data3['Key'] + ' ' + data3['IV']
        print
def importWiki(data, string):
    # I don't know if these capitalizations mean anything, but "KernelCache" is used by the other plists and I need to normalize
    thingsICareAbout = {
        'applelogo': 'AppleLogo',
        'batterycharging0': 'BatteryCharging0',
        'batterycharging1': 'BatteryCharging1',
        'batteryfull': 'BatteryFull',
        'batterylow0': 'BatteryLow0',
        'batterylow1': 'BatteryLow1',
        'devicetree': 'DeviceTree',
        'glyphcharging': 'GlyphCharging',
        'glyphplugin': 'GlyphPlugin',
        'ibec': 'iBEC',
        'iboot': 'iBoot',
        'ibss': 'iBSS',
        'kernelcache': 'KernelCache',
        'llb': 'LLB',
        'recoverymode': 'RecoveryMode',
        'devicetree': 'DeviceTree',
    }
    f = StringIO.StringIO(data.strip())
    while True:
        line = f.readline().lower()
        if line == '': break
        if 'root filesystem' in line:
            line2 = f.readline()
            print string + '.fs: ' + re.search('VFDecrypt:\s*([a-zA-Z0-9]*)', line2).group(1)
            continue
        for k, v in thingsICareAbout.items():
            if k in line:
                ivline = f.readline()
                keyline = f.readline()
                print string + '.' + v + ': ' + re.search('Key:\s*([a-zA-Z0-9]*)', keyline).group(1) + ' ' + re.search('IV:\s*([a-zA-Z0-9]*)', ivline).group(1)
                break
    print


if sys.argv[1] == 'wiki':
    importWiki(sys.stdin.read(), sys.argv[2])
#importOldStuff()
