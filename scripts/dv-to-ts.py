#! /usr/bin/python


import sys
import string
import shelve
import os

# pretty print a list
def strlist(l):
    return string.join(map(lambda x: str(x) + ' ', l))

if len(sys.argv)!=2:
    print 'ERROR: need a DV file as input'
    exit(-1)
else:
    infile = sys.argv[1]

def read_txn(txnstr):
    info = string.split(txnstr)
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return items 


def read_dv(dvstr):
    info = string.split(dvstr)
    ndv = int(info.pop(0))
    l1 = [ int(info[i]) for i in range(0, len(info), 2) ]
    l2 = [ int(info[i]) for i in range(1, len(info), 2) ]
    return zip(l1, l2)

fin = file(infile, 'r')
ntrans = 0
for l in fin:
    #print l
    dv = read_dv(l)
    ntrans += 1
    (l1, l2) = zip(*dv)
    print strlist(l1)
