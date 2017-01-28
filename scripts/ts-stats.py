#! /usr/bin/python


import sys
import string
import shelve
import os

if len(sys.argv)!=2:
    print 'ERROR: need an input filename'
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

filesize = float(os.path.getsize(infile)) / (1024*1024)
print 'filesize is %.2f MB' % filesize
fin = file(infile, 'r')
tid = 0
items = {}
num_nonzeros = 0
max_item = 0
max_txnlen = 0
for l in fin:
    #print l
    txn=read_txn(l)
    if tid % 10000 == 0:
	sys.stdout.write('.')
	sys.stdout.flush()
    if len(txn)>0: # dont crash on empty transactions
        num_nonzeros += len(txn)
        tid += 1
        max_item = max(max_item, txn[len(txn)-1])
        max_txnlen = max(max_txnlen, len(txn))

ntrans =  tid
num_items = max_item + 1

print '\nnumber of transactions =', ntrans
print 'number of items =', num_items

avg_txnlen = float(num_nonzeros)/ntrans
print 'average txn length = ', avg_txnlen
print 'maximum txn length = ', max_txnlen
print 'density =', avg_txnlen/num_items * 100.0, '%'
