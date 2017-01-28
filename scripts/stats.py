#  python basic statistics

import math

def e(x):
    return sum(x)/len(x)

def var(x):
    return e([(xi-e(x))**2 for xi in x])

def stddev(x):
    return math.sqrt(var(x))

def histogram(a):
    freq = {}
    for x in a:
        if not x in freq:
            freq[x] = 1
        else:
            freq[x] += 1
    return freq

def print_histogram(freq):
    a = freq.keys()
    a.sort()
    print 'value : frequency'
    for x in a:
        print x, ':', freq[x]

