#! /usr/bin/python
# time problem instances and store results in a database
# avoiding multiple invocations of the same instance
# run from toplevel

import sys
import string
import shelve
import os
import time

import perf
import util

def timeit(key, cmd) :
    print 'running', cmd
    t_s = time.time()
    ret = os.system(cmd)
    t_e = time.time()
    if ret==0:
        delta = t_e - t_s
        print '* successful execution, time elapsed: ', delta
        db[key] = delta
        db.sync()
    else:
        print '*** ERROR!!!'

def run(cmd):
    a = os.popen(cmd)
    lines = a.readlines()
    return not a.close()


def serial_time(workls):
    # generate problem instances

    os.system("rm -rf /tmp/noclique*")
    for x in workls:
        xl = string.split(x)
        datafile = xl[0]
        k = int(xl[1])
        l = int(xl[2])
    
        supports = xl[3:]
        for support in supports:

            path=os.path.join(perf.datadir, datafile)
            # bitdrill up to level k
            key =  'bitdrill:%s:%s:1:%d' % (datafile,support,k)
            cmd = 'opt/freq-bitdrill %s %s %s.pat.%s.level.%d %d' % \
                  (path, support, path, support, k, k)
            if not db.has_key(key):
                timeit(key,cmd)

            # bitdrill from k to k + l
            key =  'bitdrill:%s:%s:%d:%d' % (datafile,support,k,l)
            if l==0:
                cmd = 'opt/freq-bitdrill --freq %s.pat.%s.level.%d %s %s %s.pat.%s.level.%d+' % (path, support, k, path, support, path, support, k)
            else:
                cmd = 'opt/freq-bitdrill --freq %s.pat.%s.level.%s %s %s %s.pat.%s.level.%d.%d %d' % (path, support, k, path, support, path, support, k, l, k+l)
    
            if not db.has_key(key):
                timeit(key,cmd)
                #continue
        
def main():

    (options, workls) = perf.parse_cmd_line()
    global opt
    opt = options
    print 'config: ', opt.config 
    dbname = '%s.serial.%s.db' % (opt.benchmarkfile, opt.config)
    global db
    db = shelve.open( dbname )
    # generate problem instances
    serial_time(workls)
    db.close()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())

