import string
import transaction

def read_pat(patstr):
    info = string.split(patstr)
    countstr = info.pop()
    count = int(countstr.strip("()"))       # rip parenthesis
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return (items, count) 

def write_pat(fout, pat):
    (items, count) = pat
    for item in items:
        fout.write("%d " % item)
    fout.write("(%d)\n" % count)

def read_freq_patterns(fname):
    fin = file(fname, 'r')
    patterns = {} # by length
    num_patterns = 0
    for l in fin:
	try:
	    (items,count) = read_pat(l)
            num_patterns += 1
	    if not len(items) in patterns:
		patterns[len(items)] = {}
	    patterns[len(items)][transaction.encode_items(items)] = count
	except:
	    continue
    print num_patterns, 'patterns read from', fname
    return patterns


def encode_items(items):
    items.sort()
    mystr = string.join(map(lambda x: str(x), items))
    return mystr
#    return items + ':' + str(count)

def decode_items(patstr):
    info = string.split(patstr)
    # count = info.pop()
    print info
    items = map(lambda x: int(x), info)
    items.sort()                         # in any event
    return items
#    return (items, count) 
