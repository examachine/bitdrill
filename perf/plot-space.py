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

d = shelve.open( 'space.dbm' )

for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    numtxn = int(xl[1])
    path = string.split(datafile,'/')
    filename = path[len(path)-1]
    dataname = filename[0:string.rfind(filename,'.')]
    fp = open('serial/' + dataname + '.space', 'w')
    if len(xl) > 2:
        supports = xl[2:]
    else:
        supports = default_supports
    for epsilon in supports:
        (d0,space1,d1) = d['fpgrowth-tiny:' + datafile + ':' + epsilon]
        (d0,space2,d2) = d['fpgrowth:' + datafile + ':' + epsilon]
        use1 = float(space1)/ (1024*1024)
        use2 = float(space2)/ (1024*1024)
        e_percent = float(epsilon) * 100.0
        fp.write(str(e_percent) + ' ' + str(use1) + ' ' + str(use2) + '\n')
    fp.close()
d.close()
