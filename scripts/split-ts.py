#! /usr/bin/python
# transpose a transaction db

import sys
import string
import shelve
import os
import util
import transaction
import pattern
from transaction import decode_items, encode_items

if len(sys.argv)!=3:
    print 'usage: prog <db> <k>'
    sys.exit(-1)
else:
    dbname = sys.argv[1]
    k = int(sys.argv[2])


db = transaction.read_db(dbname)

freq_previous = set() # used for support

num_txn = len(db.transactions)
txns_per_part = num_txn/k

txnid = 0
out = None
for txn in db.transactions:
    if txnid % txns_per_part == 0:
        if out:
            out.close()
        out = file('%s.%d' % (dbname, txnid/txns_per_part), 'w')
    out.write(util.strlist(txn) + '\n')
    txnid += 1
