#! /usr/bin/python
# plot database stats

import sys
import string
import shelve
import os
import perf
import util


(datafiles, load_estimate) = perf.proc_args()

# access databases
d = shelve.open( 'analysis.dbm' )
ds = shelve.open( 'serial-time.dbm' )

# generate problem instances

def gen_problems():

    if not os.access("analysis", os.F_OK):
        os.mkdir("analysis")

    for x in datafiles:
        xl = string.split(x)
        datafile = xl[0]
        numtxn = int(xl[1])
        minprocs = int(xl[2])
        dataname = perf.comp_dataname(datafile)
        f = open('analysis/' + dataname + '.f2stats', 'w')
        if len(xl) > 3:
            supports = xl[3:]
        else:
            supports = default_supports
        f.write( 'Support NumVertices AvgF2deg DevF2deg MinF2Deg MaxF2Deg\n')
        for epsilon in supports:
            key = datafile + ':' + epsilon
            (stats, est) = d[key]
            # print stats
            f.write( epsilon + ' ' + str(stats[4]) + ' ' + str(stats[7])
                     + ' ' + str(stats[8]) + ' ' + str(stats[9]) + ' '
                     + str(stats[10]) + '\n')
        f.close()

    # read all stats in memory
    all_stats = {}
    for x in datafiles:
        xl = string.split(x)
        datafile = xl[0]
        numtxn = int(xl[1])
        minprocs = int(xl[2])
        dataname = perf.comp_dataname(datafile)
        if len(xl) > 3:
            supports = xl[3:]
        else:
            supports = default_supports
        for epsilon in supports:
            key = datafile + ':' + epsilon
            (stats, est) = d[key]
            if not all_stats.has_key(epsilon):
                all_stats[epsilon] = {}
            all_stats[epsilon][dataname] = stats
    
    #print all_stats

    # write all stats

    datanames = [perf.comp_dataname(string.split(x)[0]) for x in datafiles]
    fnv = open('analysis/numvertices.data', 'w')
    fnv.write('Support ' + util.strlist(datanames) + '\n')
    fad = open('analysis/avgdegree.data', 'w')
    fad.write('Support ' + util.strlist(datanames) + '\n')

    supports = all_stats.keys()
    supports.sort()
    print 'supports: ', supports

    for epsilon in supports:
        stats_by_e = all_stats[epsilon]
        print '*', epsilon, stats_by_e
        fnv.write(epsilon + '\t')
        fad.write(epsilon + '\t')
        for dataname in datanames:
            if stats_by_e.has_key(dataname):
                fnv.write(str(int(stats_by_e[dataname][4])) + '\t')
                fad.write(str(stats_by_e[dataname][7]) + '\t')
            else:
                fnv.write('-\t')
                fad.write('-\t')
        fnv.write('\n')
        fad.write('\n')
        
    fnv.close()

gen_problems()

d.close()
