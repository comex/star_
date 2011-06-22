import urllib, re, traceback, sys
import keyz
def page(name):
    return urllib.urlopen('http://theiphonewiki.com/wiki/index.php?title=%s&action=raw' % urllib.quote(name)).read()
firmware = page('Firmware')
morekeyz = open('morekeyz.txt', 'w')
urls = open('urls.txt', 'w')
for pagename, _, url in re.findall('\[\[([^\|]+).*\n(.*\[\[[0-9].*\n)?.*(http://appldn[^ ]*) ', firmware):
    #if 'iPad1,1_4.2.1_8C148' not in url: continue
    version = re.search('/([^/]*)_Restore', url).group(1)
    print >> urls, url
    print url,
    orig, sys.stdout = sys.stdout, morekeyz
    try:
        p = page(pagename)
        keyz.importWiki(p, version)
    except Exception, e:
        print >> orig, 'FAIL'
        #print p
        #traceback.print_exc()
    else:
        print >> orig, 'ok'
    finally:
        sys.stdout = orig
