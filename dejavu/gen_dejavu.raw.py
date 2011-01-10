subrs = {'main': '''
    -346 42 callothersubr
    div
    setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint setcurrentpoint
    12345 0 2 24 callothersubr
    endchar
''', 0: ''' endchar '''}
num_bca = 4

template = open('dejavu.raw.template').read()
template = template.replace('%BCA%', ('0 ' * num_bca)[:-1])
template = template.replace('%THEPROGRAM%', subrs['main'])
template = template.replace('%NUMSUBRS%', '%d' % (len(subrs) - 1))
subrtext = ''
for num, subr in subrs.iteritems():
    if num == 'main': continue
    subrtext += 'dup %d {\n\t%s\n\t} mark\n' % (num, subr)
template = template.replace('%SUBRS%', subrtext)

open('dejavu.raw', 'w').write(template)
