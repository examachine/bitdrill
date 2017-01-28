#! /usr/bin/python
# perform analysis on problem instances and store results in a database
# avoiding multiple invocations of the same instance

import sys
import string
import shelve
import os
import time

default_supports = [ '0.0025', '0.003', '0.0035', '0.0045', '0.0075' ]
datadir = '../data/'
if len(sys.argv)>=2:
    datafilesname = sys.argv[1]
else:
    datafilesname = datadir + 'datafiles-fpgrowth'
print '*** reading problem instances from', datafilesname
f  = open(datafilesname, 'r')
datafiles = f.readlines()

# speed ratio of interface over compute node (500/400=1.25 for borg)
if len(sys.argv)>=3:
    speed_ratio = float(sys.argv[2])
else:
    speed_ratio = 1.0
print '*** speed ratio is ', speed_ratio

d = shelve.open( 'serial-time.dbm' )

def timeit (key, cmd) :
    print 'running', cmd
    t_s = time.time()
    ret = os.system(cmd)
    t_e = time.time()
    if ret==0:
        delta = t_e - t_s
        print '* successful execution, time elapsed: ', delta
        d[key] = delta * speed_ratio
        d.sync()
    else:
        print '*** ERROR!!!'

def run(cmd):
    a = os.popen(cmd)
    lines = a.readlines()
    return not a.close()

# generate problem instances

parfreqbin='../bin/parfreq'
splitbin='../bin/init-pardb'
mpirun='mpirun '
tmpdir='/tmp/'

for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    numtxn = int(xl[1])
    minprocs = int(xl[2])
    #if minprocs > 1:
    #    continue
    
    #binary = true
    #if xl[2]=='binary':
    #    binary = true
    #else:
    #    binary = false  
    if len(xl) > 3:
        supports = xl[3:]
    else:
        supports = default_supports
    for epsilon in supports:

        support = int ( numtxn * float(epsilon) )

        # fpgrowth-tiny
        key =  'fpgrowth-tiny:' + datafile + ':' + epsilon
        args = datadir + datafile + ' ' + str(support) + ' out'
        prog = '../bin/freq'
        cmd = prog + ' ' + args
        if not d.has_key(key):
            timeit(key,cmd)
d.close()
