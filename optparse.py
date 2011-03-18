import sys
class OptionParser:
    def __init__(self, *args, **kwargs):
        self.sort = 0
    def add_option(self, *args, **kwargs):
        setattr(self, kwargs.get('dest', ''), kwargs.get('default', None))
    def disable_interspersed_args(self): pass
    def __getattr__(self, a):
        return None
    def parse_args(self):
        return (self, sys.argv[1:])
