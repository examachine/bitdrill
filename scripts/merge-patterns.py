#! /usr/bin/python

import sys
import string
import shelve
import os

import pattern

if len(sys.argv)<2:
    print 'ERROR: need one or more pattern files to merge'
    exit(-1)
else:
    inputs = sys.argv
    inputs.pop(0)

def merge_patterns(inputs):
    pat_freq = {}
    for infile in inputs:
        fin = file(infile, 'r')
        for l in fin:
	    try:
		(items,count) = pattern.read_pat(l)
	    except:
		continue
	    #print '*', items, count
            if len(items)>0:
		code = pattern.encode_items(items)
		if code in pat_freq:
		    pat_freq[code] = pat_freq[code] + count
		else:
		    pat_freq[code] = count
    return pat_freq

pat_freq = merge_patterns(inputs)
pat_list = pat_freq.items()
pat_list.sort(lambda x,y: len(x[0]) - len(y[0]))
for (code, count) in pat_list:
    print code, '(' + str(count) + ')'

