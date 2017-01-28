#! /usr/bin/python

import sys
import string
import shelve
import os
import time

default_supports = [ '0.0025', '0.003', '0.0035', '0.0045', '0.0075' ]
datadir = '../data/'
f  = open(datadir + 'datafiles', 'r')
datafiles = f.readlines()

d = shelve.open( 'serial-time.dbm' )

sum = 0
n = 0
for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    numtxn = int(xl[1])
    path = string.split(datafile,'/')
    filename = path[len(path)-1]
    dataname = filename[0:string.rfind(filename,'.')]
    if len(xl) > 2:
        supports = xl[2:]
    else:
        supports = default_supports
    for epsilon in supports:
        time1 = d['fpgrowth-tiny:' + datafile + ':' + epsilon]
        time2 = d['fpgrowth:' + datafile + ':' + epsilon]
        use = (float(time1)/float(time2)) * 100.0
        print 'running time ', datafile, epsilon, 'is', use , '%'
        sum = sum + use
        n = n + 1
d.close()
print 'average running time = ', sum/n

#        print 'memory saving for ', datafile, epsilon, 'is', (float(space2)/float(space1)-1) * 100.0 , '%'
