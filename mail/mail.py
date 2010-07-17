#!/opt/local/bin/python2.6
import email
from email.mime.multipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import formatdate
from email import Encoders
import sys

body, attachment, outfile = sys.argv[1:]
msg = MIMEMultipart()
msg['From'] = 'jailbreakme.com <jailbreakme@jailbreakme.com>'
msg['To'] = 'you <you@you.com>'
msg['Date'] = formatdate(localtime=True)
msg['Subject'] = 'Jailbreak'

text = open(body).read()
msg.attach(MIMEText(text))

part = MIMEBase('application', 'pdf')
part.set_payload(open(attachment, 'rb').read())
Encoders.encode_base64(part)
part.add_header('Content-Disposition', 'attachment; filename="jailbreak.pdf')
msg.attach(part)

open(outfile, 'w').write((msg.as_string() + '\n').replace('\n', '\r\n'))

