#!/usr/bin/python3

import random
import socket
import string
import struct

dip1 = '192.168.20.200'
dip2 = '192.168.20.201'
dpo = 53

def dnsq(dom):
  tid = random.randrange(1,65535)
  ds = dom.split('.')
  q = bytearray()
  for k in ds:
    klen = len(k)
    q.extend(struct.pack('B{}s'.format(klen), klen, k.encode('utf-8')))
  q = bytes(q)
  return struct.pack('!H10s{}s5s'.format(len(q)), tid, b'\x01\x20\x00\x01\x00\x0
0\x00\x00\x00\x00', q, b'\x00\x00\x01\x00\x01')


s1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

ltrs = string.ascii_lowercase
tld = ['com','net','edu','org','io']

for i in range(1000000):
  dom = ''.join(random.choice(ltrs) for i in range(random.randint(3,15))) + '.'
+ tld[random.randint(0,4)]
  p = dnsq(dom)
  s1.sendto(p, (dip1, dpo))
  s2.sendto(p, (dip2, dpo))
