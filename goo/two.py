import operator, sys
import cPickle as pickle
from goo import *
import dmini

dmini.init(sys.argv[1], True)

data = pickle.load(open(sys.argv[2], 'rb'))
rop = data['segment']
rop_address = data['rop_address']
stack_size = rop_address - 0x1000
linkedit_size = 0x10000
linkedit_address = data['init_sp'] - linkedit_size

rop_relocs = []
if isinstance(data['initializer'], reloc):
    initializer = pointed(I(data['initializer'].value))
    rop_relocs.append(linkedit_address + pointer(initializer))
else:
    initializer = pointed(I(data['initializer']))


def func(value, addr):
    rop_relocs.append(addr)
    return value
reloc_handlers[3] = func

rop = simplify(rop, rop_address)

PROT_READ, PROT_WRITE, PROT_EXECUTE = 1, 2, 4

strtab = ''
symtab = ''
relocs = ''
commands = troll_string()
linkedit_rest = troll_string()
ncmds = 0
segments = troll_string()
dylib_ordinal = -1

linkedit = later_s(lambda addr: I(
    0xfeedface,
    12, # CPU_TYPE_ARM
    6, # CPU_SUBTYPE_ARM_V6,
    2, # MH_EXECUTE
    ncmds,
    len(commands), # sizeofcmds
    ( # flags
        4 # MH_DYLD_LINK
        | 0x10 # MH_PREBOUND
        | 0x80 # MH_TWOLEVEL
    )
) + commands + linkedit_rest)

def command(cmd, stuff):
    global ncmds
    ncmds += 1
    stuff = pad(stuff, 4)
    commands.append(I(cmd, len(stuff) + 8))
    commands.append(stuff)

reloc_base = None

def segment(segname, vmaddr, vmsize, data, maxprot, initprot, sects=[]):
    global reloc_base
    # dumb, but that's the spec!
    if reloc_base is None: reloc_base = vmaddr

    length = len(data)
    if vmsize is None: vmsize = (length + 0xfff) & ~0xfff
    offset = len(segments)
    stuff = segname.ljust(16, '\0') + I(
        vmaddr,
        vmsize,
        offset,
        length,
        maxprot,
        initprot,
        len(sects), # nsects
        0 # flags
    )
    for sect in sects:
        stuff += sect['sectname'].ljust(16, '\0') + segname.ljust(16, '\0') + I(
            vmaddr + sect['offset'], # addr
            sect['size'], # size
            offset + sect['offset'], # offset
            0, # align
            0, # reloff
            0, # nreloc
            sect['flags'], # flags
            0, # reserved1
            0) # reserved2

    command(1, stuff) # LC_SEGMENT 
    segments.append(pad(data, 0x1000))

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

def import_sym(ordinal, name, subtract=0):
    global strtab, symtab
    strx = len(strtab)
    strtab += name + '\0'
    symtab += struct.pack('IBBHI',
        strx,
        0, # n_type = N_UNDF
        0, # n_sect is ignored?
        (ordinal + 1) << 8, # n_desc
        subtract, # n_value (with prebound set, this is subtracted)
    )
    return len(symtab) / 12 - 1

def reloc(sym, address):
    global relocs
    r_pcrel = 0
    r_length = 2
    r_extern = 1
    r_type = 0

    relocs += I(
        (address - reloc_base) & 0xffffffff,
        (r_type << 28) |
        (r_extern << 27) |
        (r_length << 25) |
        (r_pcrel << 24) |
        sym)

segment('__LINKEDIT', 
    linkedit_address,
    linkedit_size,
    linkedit,
    PROT_READ | PROT_WRITE,
    PROT_READ | PROT_WRITE,
    [{'sectname': '__init',
      'offset': pointer(initializer),
      'size': 4,
      'flags': 0x9, # S_MOD_INIT_FUNC_POINTERS
      }]
)

libs = {}
for path in data['libs']:
    libs[path] = load_dylib(path, 0, 0, 0x010000)
sym = import_sym(libs['/usr/lib/libSystem.B.dylib'], '_getpid', subtract=dmini.cur.sym('_getpid')&~1)
for addr in rop_relocs:
    reloc(sym, addr)

symtab = pointed(symtab)
relocs = pointed(relocs)
strtab = pointed(strtab)

command(2, I( # LC_SYMTAB
    pointer(symtab), # symoff
    len(symtab) / 12, # nsyms
    pointer(strtab), # stroff
    len(strtab), # strsize
))

linkedit_rest.append(initializer)
linkedit_rest.append(symtab)
linkedit_rest.append(relocs)
linkedit_rest.append(strtab)

command(0xb, I( # LC_DYSYMTAB
    0, 0, # localsym
    0, 0, # extdefsym
    0, len(symtab) / 12, # undefsym
    0, 0, # toc
    0, 0, # modtab
    0, 0, # extrefsym
    0, 0, # indirectsym
    pointer(relocs), len(relocs) / 8, # extrel
    0, 0, # locrel
))


segment('__ROP',
    rop_address,
    None,
    rop,
    PROT_READ | PROT_WRITE,
    PROT_READ | PROT_WRITE
)

segment('__STACK',
    rop_address - stack_size,
    stack_size,
    '',
    PROT_READ | PROT_WRITE,
    PROT_READ | PROT_WRITE
)

command(5, I( # LC_UNIXTHREAD
    1, # ARM_THREAD_STATE
    17, # ARM_THREAD_STATE_COUNT,
    0, 0, 0, 0, 0, 0, 0, 0, # R0-R7
    0, 0, 0, 0, 0, data['init_sp'], 0, linkedit_address, # R8-R15
    0, # CPSR
))

macho = simplify_times(segments, 0, 4)
    
open(sys.argv[3], 'wb', 0755).write(macho)

