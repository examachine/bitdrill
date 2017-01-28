#! /usr/bin/python


import sys
import string

fin = open(sys.argv[1], "r")

def writetxn (txn) :
    for i in txn:
        sys.stdout.write(str(i) + ' ')
    sys.stdout.write('\n')

lf = fin.readline()
txn_no = int(string.split(lf)[0])

fin.close()

fin = open(sys.argv[1], "r")
txn = []
for l in fin:
    info = string.split(l)
    ti = int(info[0])
    item = int(info[1])
    if (ti>txn_no):
        writetxn(txn)
        txn_no = ti
        txn = [item]
    else :
        txn.append(item)

writetxn(txn)

