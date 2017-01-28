#!/usr/bin/python

f = file('parallel/all.dissection')

a = f.readlines()

a.pop(0)

avg = [0.0, 0.0, 0.0, 0.0]

import sys

if len(sys.argv)==2:
    last = int(sys.argv[1])
else:
    last = len(a)

for j in range(last):
    x = a[j]
    cs = x.split()
    for i in range(4):
        avg[i] += float(cs[1 + i])
for i in range(4):
    avg[i] /= len(a)
print avg

