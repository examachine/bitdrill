#! /usr/bin/python
# perform analysis on problem instances and store results in a database
# avoiding multiple invocations of the same instance

import sys
import string
import os

if len(sys.argv)==2:
    datafilename = sys.argv[1]
else:
    datafilename = 'fimi04.data'

f = file(datafilename)

ls = f.readlines()

times = {}
spaces = {}
instances = {}

def begins(x,y):
    return string.find(x, y)==0

def finishes(x,y):
    return string.rfind(x, y)==len(x)-len(y)

filter_low_m = { 'chess.dat': 958, 'connect.dat':37156, 'pumsb.dat':29427,
                 'bmspos.dat': 103, 'accidents.dat':34018,
                 'mushroom.dat':500, 'kosarak.dat':1000,
                 'bmspos.data': 300, 'pumsb-star.dat':15000,
                 'retail.dat': 10, 'bms1.dat': 40, 'bms2.dat': 40, 
                 'T40I10D100K.dat' : 400}

def filter_low(xl):
    data = xl[1]
    support = int(xl[2])
    if filter_low_m.has_key(data):
        return support > filter_low_m[data]
    else:
        return True


for x in ls:
    xl = string.split(x)
    if len(xl)!=10:
        continue
    algorithm = xl[0]
    data = xl[1]
    support = int(xl[2])
    time = float(xl[3])
    mem = float(xl[7]) / (1024*1024)
    if not filter_low(xl):
        continue
    #print xl
    if not times.has_key(algorithm):
        times[algorithm] = time
        spaces[algorithm] = mem
    else:
        #times[algorithm] *= time
        #spaces[algorithm] *= mem
        times[algorithm] += time
        spaces[algorithm] += mem
    if not instances.has_key(algorithm):
        instances[algorithm] = 1
    else:
        instances[algorithm] += 1

instances_list = instances.items()
instances_list.sort(lambda x, y : cmp(y[1],x[1]) )

avgtimes = []
for (algo,time) in times.iteritems():
#    avgtimes.append( (algo, time ** (1.0 / instances[algo]) ) )
    avgtimes.append( (algo, time  / instances[algo] ) )
avgtimes.sort(lambda x, y : cmp(x[1],y[1]) )

avgspaces = []
for (algo,space) in spaces.iteritems():
#    avgspaces.append ( (algo, space ** (1.0 / instances[algo])) )
    avgspaces.append ( (algo, space  / instances[algo]) )
avgspaces.sort(lambda x, y : cmp(x[1],y[1]) )

all_threshold = 80
   
def is_all( (a,d) ):
    return finishes(a, '_all') and instances[a]>=all_threshold

def is_maximal( (a,d) ):
    return finishes(a, '_maximal')

def is_closed( (a,d) ):
    return finishes(a, '_closed')

avgtimes_all = filter(is_all, avgtimes )
avgspaces_all = filter(is_all, avgspaces )
avgrank = {}
for i in range(len(avgtimes_all)):
    k = avgtimes_all[i][0]
    avgrank[k] = float(len(avgtimes_all) - i) / (2 * len(avgtimes_all))
for i in range(len(avgspaces_all)):
    k = avgspaces_all[i][0]
    avgrank[k] = avgrank[k] + float(len(avgspaces_all) - i) / (2*len(avgspaces_all))
avgrank_list = avgrank.items()
avgrank_list.sort(lambda x, y : cmp(y[1],x[1]) )

print 'ALL: average time', avgtimes_all
print
print 'ALL: average space', avgspaces_all
print
print 'ALL: successful instances ',  filter(is_all, instances_list)
print
print 'ALL: balanced rank ', avgrank_list
print


avgtimes_maximal = filter(is_maximal, avgtimes )
avgspaces_maximal = filter(is_maximal, avgspaces )

print 'MAXIMAL: average time', avgtimes_maximal
print
print 'MAXIMAL: average space', avgspaces_maximal
print
print 'MAXIMAL: successful instances ',  filter(is_maximal, instances_list)
print

avgtimes_closed = filter(is_closed, avgtimes )
avgspaces_closed = filter(is_closed, avgspaces )

print 'CLOSED: average time', avgtimes_closed
print
print 'CLOSED: average space', avgspaces_closed
print
print 'CLOSED successful instances ',  filter(is_closed, instances_list)
print



#print times
#print spaces
#print instances

    
