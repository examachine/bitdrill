#! /usr/bin/python
# perform space analysis on problem instances and store results in a database

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

def insert_data ( key ) :
    f = open('tmp', 'r')
    ls = f.readlines()
    for l in ls :
        comps = string.split(l)
        if len(comps) < 10:
            continue
        if comps[1] == 'usage':
            print '*', l
            heaptotal = int(string.strip(comps[5],','))
            heappeak = int(string.strip(comps[8],','))
            stackpeak = int(string.strip(comps[11],','))
            print ( heaptotal, heappeak, stackpeak )
            d[key] = ( heaptotal, heappeak, stackpeak )
            d.sync()

# generate problem instances

for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    numtxn = int(xl[1])
    if len(xl) > 2:
        supports = xl[2:]
    else:
        supports = default_supports
    for epsilon in supports:
        support = int ( numtxn * float(epsilon) )
        key = 'fpgrowth-tiny:' + datafile + ':' + epsilon
        bin = '../bin/freq_all'
        args = datadir + datafile + ' ' + str(support) + ' out'
        cmd = 'memusage ' + bin + ' ' + args + ' &>tmp'
        if not d.has_key(key):
            print 'running', cmd
            ret = os.system(cmd)
            if ret==0:
                print '* successful execution, memusage measured'
                insert_data(key)
            else:
                print 'fubar'
        key = 'fpgrowth:' + datafile + ':' + epsilon
        bin = '../aux/fpgrowth/fpgrowth'
        args = datadir + datafile + ' 3 ' + str(support) + ' out'
        cmd = 'memusage ' + bin + ' ' + args + ' &>tmp'
        if not d.has_key(key):
            print 'running', cmd
            ret = os.system(cmd)
            if ret==0:
                print '* successful execution, memusage measured'
                insert_data(key)
            else:
                print 'fubar'
d.close()
