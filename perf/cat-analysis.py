#! /usr/bin/python
# perform analysis on problem instances and store results in a database
# avoiding multiple invocations of the same instance

import sys
import string
import shelve
import os

d = shelve.open( 'analysis.dbm' )

#print 'list serial time, db size ', len(d) 
for (k,(stats,estimate)) in d.iteritems():
    print '%s \t (%u,%u)\t %f \t %d \t%d' % ( k, stats[4], stats[5],
    stats[7], stats[9], stats[10]) 

d.close()
