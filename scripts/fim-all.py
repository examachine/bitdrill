#! /usr/bin/python
# transpose a transaction db

import sys
import string
import shelve
import os
import copy
import transaction
import pattern
from transaction import decode_items, encode_items

if len(sys.argv)!=4:
    print 'usage: prog <db> <support> <out>'
    sys.exit(-1)
else:
    dbname = sys.argv[1]
    support = int(sys.argv[2])
    outfn = sys.argv[3]


db = transaction.read_db(dbname)

k = 1 # starting level
freq_previous = set() # used for support
out = file(outfn, 'w')

while 1:

    print 'mining level %d \n' % k
    pruned = 0

    candidates = {}
    if k == 1:
	for x in range(0, db.max_item + 1):
	    candidates[encode_items([x])] = 0 # initial count
    else:
	for xs in freq_previous:
	    x = decode_items(xs)
	    for ys in freq_previous:
		y = decode_items(ys)
		sz = set(x).union(set(y))
		if len(sz) != k:
		    continue
		z = list(sz) # candidate
		z.sort()
		prune = False
		if len(z) > 1:
		    for item in z:
			zprev = copy.copy(z)
			zprev.remove(item)
			if not encode_items(zprev) in freq_previous:
			    pruned +=1
			    prune = True # not supported by subsets
			    break
		if not prune:
		    candidates[encode_items(z)] = 0 # initial count

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
	if count >= support:
	    out.write('%s (%d)\n' % (zs, count))
	    freq_previous.add(zs)

    if len(freq_previous)==0:
	print 'end run'
	break

    print 'number of frequent itemsets is %d\n' % len(freq_previous)

    k += 1

out.close()
