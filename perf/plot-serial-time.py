#! /usr/bin/python

import sys
import string
import shelve
import os
import time
import perf

(datafiles, load_estimate) = perf.proc_args()

d = shelve.open( 'serial-time.dbm' )
dp = shelve.open( 'par-time.dbm' )

if not os.path.exists('serial'):
    os.makedirs('serial')

for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    numtxn = int(xl[1])
    minprocs = int(xl[2])
    path = string.split(datafile,'/')
    filename = path[len(path)-1]
    dataname = filename[0:string.rfind(filename,'.')]
    if len(xl) > 3:
        supports = xl[3:]
    else:
        supports = default_supports

    fp = open('serial/' + dataname + '.time', 'w')

    for epsilon in supports:
        time1 = d['kdci:' + datafile + ':' + epsilon]
        time2 = d['2items-kdci:' + datafile + ':' + epsilon]
        e_percent = float(epsilon) * 100.0
        fp.write(str(e_percent) + ' ' + str(time1) + ' ' + str(time2)+ '\n')
    fp.close()
d.close()
