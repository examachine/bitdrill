#! /usr/bin/python

import sys
import string
import shelve
import os

import util
import pattern
from transaction import decode_items, encode_items

if len(sys.argv)!=2:
    print 'ERROR: need a pattern set to sort'
    exit(-1)
else:
    infile = sys.argv[1]

pat = []
fin = file(infile, 'r')
for l in fin:
    (items,count) = pattern.read_pat(l)
    pat.append( (items,count) )

def less_itemsets(a, b):
    if len(a)==len(b):
        for (x,y) in zip(a,b):
            if x < y:
                return -1
            else:
                if x > y:
                    return 1
        return 0
    else:
        return len(a)-len(b)

pat.sort(lambda (i1,c1),(i2,c2) : less_itemsets(i1,i2))
for (items, count) in pat:
    print '%s (%d)' % (util.strlist(items), count)
