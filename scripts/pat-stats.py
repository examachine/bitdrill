#! /usr/bin/python


import sys
import string
import shelve
import os

if len(sys.argv)!=2:
    print 'ERROR: need a pattern set to count'
    exit(-1)
else:
    infile = sys.argv[1]

def read_txn(txnstr):
    info = string.split(txnstr)
    count = info.pop()
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return (items, count) 

counts = {}
fin = file(infile, 'r')
ntrans = 0
for l in fin:
    (items,count) = read_txn(l)
    ntrans+=1
    length = len(items)
    if not counts.has_key(length):
        counts[length] = 1
    else:
        counts[length] = counts[length]+1
        
cl = [x for x in counts.iteritems()]
cl.sort(lambda (k,v),(k2,v2): k < k2)
#print ntrans
sum=sum([v for (k,v) in cl])
print 'total number of patterns=', sum
for (k,v) in cl:
    print v


