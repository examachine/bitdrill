#! /usr/bin/python

import sys
import string
import os
import copy
import transaction
import pattern
from transaction import decode_items, encode_items

if len(sys.argv)!=5:
    print 'usage: prog <db> <freq itemset1> <freq itemset2> <support>'
    sys.exit(-1)
else:
    dbname = sys.argv[1]
    freq1name = sys.argv[2]
    freq2name = sys.argv[3]
    support = int(sys.argv[4])

db = transaction.read_db(dbname)
freq1 = pattern.read_freq_patterns(freq1name)
freq2 = pattern.read_freq_patterns(freq2name)

# 1st step  F1,1 X F2,1
if (not 1 in freq1) or (not 1 in freq2):
    print "there are no frequent itemsets to merge"
    sys.exit(0)

k = 2 # starting level
freq_previous = set() # used for support
out = file('%s.out.merge.%d' % (dbname, support), 'w')

while 1:

    print 'merging frequent itemsets in level %d \n' % k
    pruned = 0

    candidates = {}
    if k == 2:
	for xs in freq1[1].iterkeys():
	    x = decode_items(xs)
	    for ys in freq2[1].iterkeys():
		y = decode_items(ys)
		z = x + y
		candidates[encode_items(x+y)] = 0 # initial count
    else:
	for xlen in range(1, k):
	    ylen = k - xlen
            if not xlen in freq1:
                continue
	    for xs in freq1[xlen].iterkeys():
		x = decode_items(xs)
                if not ylen in freq2:
                    continue
		for ys in freq2[ylen].iterkeys():
		    y = decode_items(ys)
		    z = x + y # candidate
		    prune = False
		    if xlen > 1:
			for item in x:
			    zprev = copy.copy(z)
			    zprev.remove(item)
			    if not encode_items(zprev) in freq_previous:
				pruned +=1
				prune = True # not supported by subsets
				break
		    if not prune and ylen > 1:
			for item in y:
			    zprev = copy.copy(z)
			    zprev.remove(item)
			    if not encode_items(zprev) in freq_previous:
				pruned +=1
				prune = True # not supported by subsets
				break
		    if not prune:
			candidates[encode_items(x+y)] = 0 # initial count

    print 'pruned %d candidates, remaining %d candidates \n' % (pruned, len(candidates))

    tid = 0
    for txn in db.transactions:
	t = set(txn)
	if tid % 100 == 0:
	    sys.stdout.write('.')
	    sys.stdout.flush()
	for zs in candidates.keys():
	    z = decode_items(zs)
	    if set(z).issubset(t):
		candidates[zs] = candidates[zs] + 1
	tid += 1
    print

    freq_previous = set()

    for (zs, count) in candidates.iteritems():
        print 'candidate', zs
	if count >= support:
	    out.write('%s (%d)\n' % (zs, count))
	    freq_previous.add(zs)

    if len(freq_previous)==0:
	print 'end run'
	break

    print 'number of frequent itemsets is %d\n' % len(freq_previous)

    k += 1

out.close()
