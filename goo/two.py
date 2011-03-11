from goo import *
import operator, sys

baseaddr = 0x33000000
stacksize = 1048576

rop = open(sys.argv[1], 'rb').read()
r7, pc = struct.unpack('II', rop[:8])
rop = rop[8:]

r7 = 0x10101010
pc = 0x33000000

PROT_READ, PROT_WRITE, PROT_EXECUTE = 1, 2, 4

strtab = ''
symtab = ''
relocs = ''
commands = troll_string()
linkedit_rest = troll_string()
ncmds = 0
segments = troll_string()
dylib_ordinal = -1

linkedit = later_s(lambda: I(
    0xfeedface,
    12, # CPU_TYPE_ARM
    6, # CPU_SUBTYPE_ARM_V6,
    2, # MH_EXECUTE
    ncmds,
    len(commands), # sizeofcmds
    ( # flags
        4 # MH_DYLD_LINK
        | 0x80 # MH_TWOLEVEL
    )
) + commands + linkedit_rest, 0x1000)

def pad(x, p):
    l = len(x)
    return x + '\0' * (-l & (p - 1))

def command(cmd, stuff):
    global ncmds
    ncmds += 1
    stuff = pad(stuff, 4)
    commands.append(I(cmd, len(stuff) + 8))
    commands.append(stuff)

def segment(segname, vmaddr, vmsize, data, maxprot, initprot):
    data = pad(data, 0x1000)
    if vmsize is None: vmsize = len(data)
    command(1, # LC_SEGMENT 
        segname.ljust(16, '\0') + I(
            vmaddr,
            vmsize,
            len(segments),
            len(data),
            maxprot,
            initprot,
            0, # nsects
            0 # flags
        )
    )
    segments.append(data)

command(0xe, # LC_LOAD_DYLINKER
        I(12) +
        '/usr/lib/dyld' +
        '\0')

def load_dylib(name, timestamp, current_version, compatibility_version):
    global dylib_ordinal
    command(0xc, # LC_LOAD_DYLIB
        I(24,
          timestamp,
          current_version,
          compatibility_version)
        + name + '\0')
    dylib_ordinal += 1
    return dylib_ordinal

def import_sym(ordinal, name):
    global strtab, symtab
    strx = len(strtab)
    strtab += name + '\0'
    symtab += struct.pack('IBBHI',
        strx,
        0, # n_type = N_UNDF
        0, # n_sect is ignored?
        ordinal << 8, # n_desc
        0, # n_value (we could set prebound to make this an addend)
    )
    return len(symtab) / 12 - 1

def reloc(sym, address):
    r_pcrel = 0
    r_length = 2
    r_extern = 1
    r_type = 0
    relocs += struct.pack('II',
        address,
        (r_type << 28) |
        (r_extern << 27) |
        (r_length << 26) |
        (r_pcrel << 24) |
        sym)

foo = load_dylib('foo.dylib', 0, 0, 0)
s = import_sym(foo, '_hax')

symtab = pointed(symtab)
relocs = pointed(relocs)
strtab = pointed(strtab)

command(0xb, I( # LC_DYSYMTAB
    0, 0, # localsym
    0, 0, # extdefsym
    pointer(symtab), len(symtab) / 12, # undefsym
    0, 0, # toc
    0, 0, # modtab
    0, 0, # extrefsym
    0, 0, # indirectsym
    pointer(relocs), len(relocs) / 8, # extrel
    0, 0, # locrel
))

command(2, I( # LC_SYMTAB
    pointer(symtab), # symoff
    len(symtab) / 12, # nsyms
    pointer(strtab), # stroff
    len(strtab), # strsize
))

linkedit_rest.append(symtab)
linkedit_rest.append(relocs)
linkedit_rest.append(strtab)

segment('__LINKEDIT', 
    (baseaddr - stacksize - 0x1000),
    None,
    linkedit,
    PROT_READ,
    PROT_READ
)

segment('__STACK',
    baseaddr - stacksize,
    stacksize,
    '',
    PROT_READ | PROT_WRITE,
    PROT_READ | PROT_WRITE
)

segment('__ROP',
    baseaddr,
    None,
    rop,
    PROT_READ | PROT_WRITE,
    PROT_READ | PROT_WRITE
)

command(5, I( # LC_UNIXTHREAD
    1, # ARM_THREAD_STATE
    17, # ARM_THREAD_STATE_COUNT,
    0, 0, 0, 0, 0, 0, 0, r7, # R0-R7
    0, 0, 0, 0, 0, 0, 0, pc & ~1, # R8-R15
    0x20 * (pc & 1), # CPSR
))

macho = simplify_times(segments, 0, 4)

open(sys.argv[2], 'wb', 0755).write(macho)

