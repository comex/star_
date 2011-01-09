import os, sys, re
os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
f = open('syscalls.c', 'w')

seen = {}
def go(num, name):
    if name in seen:
        assert name in ('nosys', 'enosys') or seen[name] == num
        return
    seen[name] = num
    print >> f, 'void sc_%s() asm("_%s");' % (name, name)
    # HACK
    ev = ''
    #if name in ['exit', '__sysctl', 'execve', 'lseek', 'mach_msg_trap', 'mach_reply_port', 'memcpy', 'mlock', 'mmap', 'open']: ev = ', externally_visible'
    print >> f, '__attribute__((naked%s)) void sc_%s() { asm volatile("mov r12, sp; stmfd sp!, {r4-r6, r8}; ldmia r12, {r4-r6}; ldrsh r12, 1f; svc #0x80; ldmfd sp!, {r4-r6, r8}; bx lr; 1: .short %d"); }' % (ev, name, num)
        
for line in open('syscalls.master'):
    m = re.match('^([0-9]+)\s*AU.*{[^\(]* ([a-zA-Z0-9_]+)\(', line)
    if m:
        go(int(m.group(1)), m.group(2))

for line in open('syscall_sw.h'):
    m = re.match('kernel_trap\(([^,]*),\s*([0-9-]+)', line)
    if m:
        go(int(m.group(2)), m.group(1))
