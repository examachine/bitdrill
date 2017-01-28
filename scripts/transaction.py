import string
import sys

def read_txn(txnstr):
    info = string.split(txnstr)
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return items 

class Transaction_DB:
    pass

def read_db(fname):
    fin = open(fname)
    tid = 0
    max_item = 0
    db = Transaction_DB()
    t = []
    for l in fin:
	txn=read_txn(l)
	if tid % 100 == 0:
	    sys.stdout.write('.')
	    sys.stdout.flush()
	t.append(txn) 
	max_item = max(max_item, txn[len(txn)-1]) 
	tid += 1
    db.transactions = t
    db.max_item = max_item
    sys.stdout.write('\n')
    sys.stdout.flush()
    return db

def encode_items(items):
    items.sort()
    mystr = string.join(map(lambda x: str(x), items)) # space between items!
    return mystr
#    return items + ':' + str(count)

def decode_items(patstr):
    info = string.split(patstr)
    # count = info.pop()
    #print info
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return items
