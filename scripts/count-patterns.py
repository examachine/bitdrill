#! /usr/bin/python

import sys
import string
import shelve
import os

import pattern

if len(sys.argv)!=2:
    print 'ERROR: need a pattern set to count'
    exit(-1)
else:
    infile = sys.argv[1] 

counts = {}
fin = file(infile, 'r')
ntrans = 0
for l in fin:
    (items,count) = pattern.read_pat(l)
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
    print '%d: %d' % (k,v)


