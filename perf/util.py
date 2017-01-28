
import sys
import string
import shelve
import os
import subprocess

def unzip(seq):
    return zip(*seq)

def concat(a,b):
    c = a[:]
    c.extend(b)
    return c

# pretty print a list
def strlist(l):
    return string.join(map(lambda x: str(x) + ' ', l))

# transpose list of lists, slow
def transpose_lol(lol) :
    r = len(lol)
    c = len(lol[0])
    t = []
    for i in range(0,r):
        l = []
        for j in range(0,c):
            l.append(lol[c][r])
        t.append(l)
    return t

def run(cmd, debug=False):
    """Run command and get the return value."""
    if debug:
        print 'running ' + cmd
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    out, err = p.communicate()
    if debug:
        print 'return value for "%s" is %s' % (cmd, p.returncode)
        if out:
            print out
        if err:
            print 'error messages follow:'
            print err
    return (p.returncode, out, err)
