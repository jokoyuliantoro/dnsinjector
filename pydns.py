#!/usr/bin/python3

import random
import socket
import struct

dip = '192.168.20.200'
dpo = 53

def dnsq(dom):
  tid = random.randrange(1,65535)
  msg = struct.pack('!H10s',tid,b'\x01\x20\x00\x01\x00\x00\x00\x00\x00\x00')
  ds = dom.split('.')
  q = bytearray()
  for k in ds:
    klen = len(k)
    q.extend(struct.pack('B{}s'.format(klen), klen, k.encode('utf-8')))
  q = bytes(q)
  return struct.pack('!H10s{}s5s'.format(len(q)), tid, b'\x01\x20\x00\x01\x00\x00\x00\x00\x00\x00', q, b'\x00\x00\x01\x00\x01')
  

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

lines = open('domain.txt', 'r').readlines()

for l in lines:
  p = dnsq(l.strip())
  s.sendto(p, (dip, dpo))


