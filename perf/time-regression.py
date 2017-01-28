#! /usr/bin/python

import sys
import string
import shelve
import os
import time
import math
import random

def write_dataset(data, filename):
    f = open(filename, 'w')
    for (stats, estimates, time) in data:
        f.write(str(time))
        attributes = stats + estimates
        i=1
        for x in attributes :
            f.write(' ' + str(i) + ':' + str(x))
            i=i+1
            f.write(' ' + str(i) + ':' + str(math.log(x,2)))
            i=i+1
            f.write(' ' + str(i) + ':' + str(math.sqrt(x)))
            i=i+1
        f.write('\n')

supports = [ '0.0025', '0.003', '0.0035', '0.0045', '0.0075' ]
datadir = '../data/'
f  = open(datadir + 'datafiles', 'r')
datafiles = f.readlines()

d1 = shelve.open( 'analysis.dbm' )
d2 = shelve.open( 'serial-time.dbm' )

# generate problem instances

bin='../bin/freq-bin'

data = []

for datafile in datafiles:

    for epsilon in supports:
        key = datafile.rstrip('\n') + ':' + epsilon
        args = datadir + datafile.rstrip('\n') + ' ' + epsilon
        cmd = bin + ' ' + args
        (stats,estimates) = d1[key]
        time = d2[key]
        data.append( (stats, estimates, time) )

random.seed()
random.shuffle(data)

# separate 9/1

l = len(data)
cut = int( float(l) * 0.7 )
train = data[0:cut]
test = data[cut+1:l-1]

write_dataset(train, "train.svm")
write_dataset(test, "test.svm")
write_dataset(train, "all.svm")

d1.close()
d2.close()
