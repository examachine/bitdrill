#! /usr/bin/python

import sys
import string
import shelve
import os
import transaction
import util

if len(sys.argv)!=3:
    print 'USAGE: dp-lowpass.py <input-file> <threshold>'
    sys.exit(-1)


infile = sys.argv[1]
threshold = int(sys.argv[2])
outfile = '%s.lp.%d' % (infile, threshold)
count = {}

print 'Reading', infile, 'to calculate item frequencies'

fin = file(infile, 'r')
for tid, l in enumerate(fin):
    txn = transaction.read_txn(l)
    for item in txn:
        if not item in count:
            count[item] = 0
        count[item] += 1
    if tid % 1000 == 0:
	sys.stdout.write('.')
	sys.stdout.flush()

print
print 'Writing', outfile

fout = file(outfile, 'w')
fin = file(infile, 'r')
for tid, l in enumerate(fin):
    txn = transaction.read_txn(l)
    txnout = [x for x in txn if count[x] < threshold]
    fout.write(util.strlist(txnout) + '\n')
    if tid % 1000 == 0:
	sys.stdout.write('.')
	sys.stdout.flush()
