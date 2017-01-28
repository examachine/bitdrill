#! /usr/bin/python
# perform analysis on problem instances and store results in a database
# avoiding multiple invocations of the same instance

import sys
import string
import shelve
import os

default_supports = [ '0.0025', '0.003', '0.0035', '0.0045', '0.0075' ]

datadir = '../data/'
f  = open(datadir + 'large-datafiles', 'r')
datafiles = f.readlines()

d = shelve.open( 'analysis.dbm' )

# generate problem instances

analyzebin='../bin/analyze-ts'
for datafile in datafiles:
    xl = string.split(datafile)
    datafile = xl[0]
    numtxn = int(xl[1])
    minprocs = int(xl[2])
    if len(xl) > 3:
        supports = xl[3:]
    else:
        supports = default_supports

    for epsilon in supports:
        key = datafile + ':' + epsilon
        args = datadir + datafile.rstrip('\n') + ' ' + epsilon
        cmd = analyzebin + ' ' + args
        if d.has_key(key):
            continue
        print 'running', cmd
        if os.system(cmd)==0:
            print 'successful execution'
            f = open('log.0', 'r')
            ls = f.readlines()
            for l in ls :
                comps = string.split(l)
                if len(comps)==0:
                    continue
                if comps[0] == 'db-stats':
                    stats = comps
                    print stats
                    stats.pop(0)
                if comps[0] == 'load-estimate':
                    estimate = comps
                    print estimate
                    estimate.pop(0)
                    d[key] = ( [string.atof(x) for x in stats],
                               [string.atof(x) for x in estimate] )
        else:
            print 'fubar'
d.close()
