#! /usr/bin/python
# transpose a transaction db

import sys
import string
import shelve
import os

if len(sys.argv)!=2:
    print 'ERROR: need input filename'
    sys.exit(-1)
else:
    infile = sys.argv[1]

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
max_item = 0
t = []
for l in fin:
    #print l
    txn=read_txn(l)
    if tid % 1000 == 0:
	sys.stdout.write('.')
	sys.stdout.flush()
    t.append(txn) 
    if txn[len(txn)-1] > max_item:
	max_item = txn[len(txn)-1] 
    tid += 1

print
mid_item = max_item/2


print 'writing file 1'
fout1 = file(infile + '.I1' , 'w')
for txn in t:    
    fout1.write(strlist([x for x in txn if x <= mid_item ]) + '\n')
fout1.close()

print 'writing file 2'
fout2 = file(infile + '.I2', 'w')
for txn in t:    
    fout2.write(strlist([x for x in txn if x > mid_item]) + '\n')
fout2.close()
