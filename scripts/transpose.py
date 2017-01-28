#! /usr/bin/python
# transpose a transaction db

import sys
import string
import shelve
import os

if len(sys.argv)!=3:
    print 'ERROR: need input and output filenames'
    sys.exit(-1)
else:
    infile = sys.argv[1]
    outfile = sys.argv[2]

# pretty print a list
def strlist(l):
    return string.join(map(lambda x: str(x) + ' ', l))

def read_txn(txnstr):
    info = string.split(txnstr)
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return items 

fin = file(infile, 'r')
tid = 0
items = {}
for l in fin:
    #print l
    txn=read_txn(l)
    if tid % 1000 == 0:
	sys.stdout.write('.')
	sys.stdout.flush()
    for x in txn:
	if not x in items:
	    items[x] = []
	items[x].append(tid)
    tid += 1

print 'writing'
fout = file(outfile, 'w')
for tidlist in items.itervalues():
    fout.write(strlist(tidlist) + '\n')
