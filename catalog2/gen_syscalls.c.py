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
    print >> f, '__attribute__((naked)) void sc_%s() { asm volatile("ldrsh r12, 1f; svc #0x80; bx lr; 1: .short %d"); }' % (name, num)
        
for line in open('syscalls.master'):
    m = re.match('^([0-9]+)\s*AU.*{[^\(]* ([a-zA-Z0-9_]+)\(', line)
    if m:
        go(int(m.group(1)), m.group(2))

for line in open('syscall_sw.h'):
    m = re.match('kernel_trap\(([^,]*),\s*([0-9-]+)', line)
    if m:
        go(int(m.group(2)), m.group(1))
