#! /usr/bin/python
# time problem instances and store results in a database
# avoiding multiple invocations of the same instance
# run from toplevel

import sys
import string
import shelve
import os
import time

datadir = 'data/'
if len(sys.argv)>=2:
    datafilesname = sys.argv[1]
else:
    datafilesname = datadir + 'datafiles2'
print '*** reading problem instances from', datafilesname
f  = open(datafilesname, 'r')
datafiles = f.readlines()

# speed ratio of interface over compute node (500/400=1.25 for borg)
if len(sys.argv)>=3:
    speed_ratio = float(sys.argv[2])
else:
    speed_ratio = 1.0
print '*** speed ratio is ', speed_ratio

d = shelve.open('perf/serial-time.dbm')

def timeit(key, cmd) :
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

os.system("rm -rf /tmp/noclique*")
for x in datafiles:
    xl = string.split(x)
    datafile = xl[0]
    level = int(xl[1])
    
    supports = xl[3:]
    for support in supports:

        path=datadir+datafile
        # bitdrill up to level k
        key =  'bitdrill:level%d:%s:%s' % (level,datafile,support)
        cmd = 'opt/freq-bitdrill %s %s %s.pat.%s.level.%d %d' % \
              (path, support, path, support, level, level)
        if not d.has_key(key):
            timeit(key,cmd)

        # bitdrill
        key =  'bitdrill:%s:%s' % (datafile,support)
        cmd = 'opt/freq-bitdrill %s %s %s.pat.%s' % (path, support, path, support)
        if not d.has_key(key):
            timeit(key,cmd)
        #continue

d.close()
