#! /usr/bin/python


import sys
import string

fout = file('user-likesmovie', 'w')

def writetxn(fout, txn) :
    for i in txn:
        fout.write(str(i) + ' ')
    fout.write('\n')

def read_mov(f):
    line = f.readline()
    movieid = int(line.strip(':\n'))
    print '<%s>' % movieid
    lines = f.readlines()
    def parse_rating(line):
	x = line.split(',')
	return (int(x[0]), int(x[1]), x[2])
    ratings = map(parse_rating, lines)
    return (movieid, ratings)

for no in range(1, 17701):
    fmov = open('mv_%07d.txt' % no)
    mvid, ratings = read_mov(fmov)
    uidlist = map(lambda x: x[0], filter(lambda x: x[1]>3, ratings))
    uidlist.sort()
    writetxn(fout, uidlist)

fout.close()

