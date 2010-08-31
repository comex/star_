'iPhone3,1_4.0.1': { '<': 'iPhone3,1_4.0', },
'iPhone1,x_4.0': {
    '<': '.armv6_4.x',
    '#kern': {
        'vram_baseaddr': 0xca67d000,     
    },
},
'iPhone1,x_4.0.1': {
    '<': '.armv6_4.x',
    '#kern': {
        'vram_baseaddr': 0xca67d000, 
    },
},
'iPhone2,1_4.0': {
    '<': '.armv7_4.x',
    '#kern': {
        'vram_baseaddr': 0xcd32d000, 
    },
},
'iPhone2,1_4.0.1': {
    '<': '.armv7_4.x',
    '#kern': {
        'vram_baseaddr': 0xcd32d000,    
    },
},
'iPod2,1_4.0': {
    '<': '.armv6_4.x',
    '#kern': {
        'vram_baseaddr': 0xca68d000, 
    },
},
'iPhone2,1_3.1.3': {
    '<': '.armv7_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xed731000,       
    },
},
'iPhone2,1_3.1.2': {
    '<': '.armv7_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xed731000,       
    },
},
'iPod2,1_3.1.2': {
    '<': '.armv6_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xeb849000,
    },
},
'iPod2,1_3.1.3': {
    '<': '.armv6_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xed731000,
    },
},
'iPhone1,x_3.1.3': {
    '<': '.armv6_3.1.x',
    '#kern': {
        'vram_baseaddr': [0xeb811000, 0xeb809000],
    },
},
'iPhone1,x_3.1.2': {
    '<': 'iPhone1,x_3.1.3',
    '#kern': {
        'vram_baseaddr': [0xeb811000, 0xeb809000],
    },
},
'iPod3,1_3.1.3': {
    '<': '.armv7_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xed615000, 
    },
},
'iPod3,1_3.1.2': {
    '<': '.armv7_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xed615000,       
    },
},
'iPod1,1_3.1.2': {
    '<': '.armv6_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xeb919000,   
    },
},
'iPod1,1_3.1.3': {
    '<': '.armv6_3.1.x',
    '#kern': {
        'vram_baseaddr': 0xeb919000,   
    },
},
'iPod3,1_4.0': {
    '<': '.armv7_4.x',
    '#kern': {
        'vram_baseaddr': 0xcd33d000, 
    },
},
'iPad1,1_3.2': {
    '<': '.armv7',
    '#kern': {
        'vram_baseaddr': [0xed6ed000, 0xed6f5000],
    },
    '#cache': {
        'magic_offset': -960,
    },
    'kill_sb': 1,
},
'iPad1,1_3.2.1': {
    '<': 'iPad1,1_3.2',
},

'iPad1,1_3.2_self': {
    '<': 'iPad1,1_3.2',
    '#cache': { '@binary': '/System/Library/Caches/com.apple.dyld/dyld_shared_cache_armv7', },
    '#kern': { '@binary': '/var/mobile/ipadkern', },
    '#launchd': { '@binary': '/sbin/launchd', },
},

'iPhone3,1_4.0': {
    '<': '.armv7_4.x',
    '#kern': {
        'vram_baseaddr': 0xd2675000,
        # ???
        'patch3':       '70 46 13 22 .. 4b 98 47 00 .. -',
        'root_ios_id': 2,
    },
},
