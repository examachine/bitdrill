#! /usr/bin/python

import sys
import string
import shelve
import os

import pattern

if len(sys.argv)!=3:
    print 'ERROR: need one or more pattern files to merge'
    exit(-1)
else:
    inputs = sys.argv
    inputs.pop(0)


def diff_patterns(a,b):
    pat_freq = {}
    a_freq = {}
    b_freq = {}
    fina = file(a, 'r')
    for l in fina:
        try:
            (items,count) = pattern.read_pat(l)
        except:
            continue
        if len(items)>0:
            code = pattern.encode_items(items)
            a_freq[code] = count
    finb = file(b, 'r')
    for l in finb:
        try:
            (items,count) = pattern.read_pat(l)
        except:
            continue
        #print '*', items, count
        if len(items)>0:
            code = pattern.encode_items(items)
            b_freq[code] = count

            #print 'sizes ', len(a_freq), len(b_freq)

    for (x,count) in a_freq.iteritems():
        if not b_freq.has_key(x):
            pat_freq[x]=count
    return pat_freq

pat_freq = diff_patterns(inputs[0],inputs[1])
pat_list = pat_freq.items()
pat_list.sort(lambda x,y: len(x[0]) - len(y[0]))
for (code, count) in pat_list:
    print code, '(' + str(count) + ')'

