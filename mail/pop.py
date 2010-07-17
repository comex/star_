import asyncore, asynchat
import os, socket, string, hashlib

class POPChannel(asynchat.async_chat):
    def __init__(self, server, sock, addr):
        print 'hello'
        asynchat.async_chat.__init__(self, sock)
        self.set_terminator("\r\n")
        self.data = ''
        self.filename = None
        self.push('+OK sup?\r\n')

    def collect_incoming_data(self, data):
        self.data += data
    
    def push(self, x):
        print '>', repr(x)
        asynchat.async_chat.push(self, x)

    def set_username(self, username):
        if username == 'popman':
            self.stuff = open('mail.txt').read()
            self.push('+OK\r\n')
        else:
            self.push('-ERR\r\n')
        
    def found_terminator(self):
        print '<', repr(self.data)
        bits = self.data.split(' ', 1)
        if len(bits) == 2:
            cmd, rest = bits
        else:
            cmd, rest = bits[0], ''
        cmd = cmd.lower()
        if cmd == 'user':
            self.set_username(rest)
        elif cmd == 'pass':
            self.push('+OK\r\n')
        elif cmd == 'apop': 
            self.set_username(rest[:rest.find(' ')])
        elif cmd == 'uidl':
            self.push('+OK\r\n')
            self.push('1 %s\r\n' % hashlib.sha1(self.stuff).hexdigest())
            #self.push('1 %s\r\n' % hashlib.sha1(open('/dev/urandom', 'rb').read(100)).hexdigest())
            self.push('.\r\n')
        elif cmd == 'stat':
            if self.stuff is None:
                self.push('-ERR\r\n')
            else:
                self.push('+OK 1 %d\r\n' % len(self.stuff))
        elif cmd == 'list':
            if self.stuff is None:
                self.push('-ERR\r\n')
            else:
                self.push('+OK\r\n')
                self.push('1 %d\r\n' % len(self.stuff))
                self.push('.\r\n')
        elif cmd == 'retr': 
            if self.stuff is None:
                self.push('-ERR\r\n')
            else:
                self.push('+OK\r\n')
                self.push(self.stuff + '\r\n.\r\n')
        elif cmd == 'top':
            try:
                msg, n = map(int, rest.split(' '))
            except:
                self.push('-ERR\r\n')
            else:
                self.push('+OK\r\n')
                headers, body = self.stuff.split('\r\n\r\n', 1)
                self.push(headers + '\r\n\r\n')
                self.push('\r\n'.join(body.split('\r\n')[:n]))
                self.push('\r\n.\r\n')

        elif cmd == 'dele':
            self.push('+OK\r\n')
        elif cmd == 'quit':
            self.push('+OK\r\n')
            self.close_when_done()
        else:
            self.push('-ERR\r\n')

        self.data = ''

class POPServer(asyncore.dispatcher):
    def __init__(self, port):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.bind(("", port))
        self.listen(5)

    def handle_accept(self):
        conn, addr = self.accept()
        POPChannel(self, conn, addr)

s = POPServer(110)
asyncore.loop()

