#! /usr/bin/python
import sys
import shelve

d = shelve.open( 'serial-time.dbm' )

if len(sys.argv) != 2:
    print 'need an algo to erase'

algo = sys.argv[1]

for k in d.iterkeys():
    if k.startswith(algo):
	print 'removing', k
	del d[k]



