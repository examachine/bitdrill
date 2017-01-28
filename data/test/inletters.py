import sys

fname = sys.argv[1]
a=file(fname).readlines()

b = [ [chr(ord('a')+int(x)) for x in y.split('(')[0].split() ] for y in 
a]

print b


