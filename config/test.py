import pyximport; pyximport.install()
import confighelper
import os
print confighelper.search_for_things('test1', {0: 'a', 'foo': 'bc', None: 'cde'})
