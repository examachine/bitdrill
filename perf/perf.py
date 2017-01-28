import sys
import getopt
import string
import shelve
import os
import util

# constants 
procs = [4, 8, 16]
default_supports = [ '0.0025', '0.003', '0.0035', '0.0045', '0.0075' ]
default_le = 2

datadir = 'data/'
benchmarkfile = 'perf/benchmark3'
num_trials=5

class options:
    def __init__(self):
        self.sync = False
        self.stop = False
        self.benchmarkfile = None 
        self.config=None # parameter config name
        self.procs = None
        self.procs2d = None
        self.tmpdir = None
        self.mpirun = None
        self.mpiclean = None
        self.profile=False
        self.num_trials=5

configs = {
    'skynet': {'procs': [2,4,8,16],
               'procs2d': [ (2,2), (4,2), (4,4), (8,2)],
               'tmpdir': '/scratch', 'mpirun' : 'mpirun-lam n1-32',
               'mpiclean': 'lamclean -v'
               },
    'cypress': {'procs': [2,4,8,16],
                'procs2d': [ (2,2), (4,2), (4,4), (8,2)],
                'tmpdir': '/tmp', 'mpirun' : 'mpirun', 
                'mpiclean' : 'killall *noclique3*'
                },
    'sirius': {'procs': [2], 'procs2d' : [ (2,1) ], 
               'tmpdir': '/tmp', 'mpirun' : 'mpirun', 
               'mpiclean' : 'killall *noclique3*'
               }
    }

def comp_dataname(datafile):
    path = string.split(datafile,'/')
    filename = path[len(path)-1]
    dataname = filename[0:string.rfind(filename,'.')]
    return dataname

def compute_serial_time(ds, datafile, epsilon, algo):
    key =  algo + ':' + datafile + ':' + epsilon
    if ds.has_key(key):
        serialtime = ds[key]
        return serialtime
    else:
        return None

def get_bitdrill_time(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, 'bitdrill')

def get_bitdrill_level_n_time(ds, datafile, epsilon, n):
    return compute_serial_time(ds, datafile, epsilon, 'bitdrill:level%d' % n)

def get_bitdrill3_time(ds, datafile, support, k, l):
    key = 'bitdrill:%s:%s:%d:%d' % (datafile,support,k,l) 
    if ds.has_key(key):
        serialtime = ds[key]
        return serialtime
    else:
        return None
    
def get_twoitems_time(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, '2items-kdci')

def compute_serial_aim(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, 'aim')

def compute_serial_kdci(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, 'kdci')

def compute_serial_lcm(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, 'lcm')

def compute_serial_fpgrowth(ds, datafile, epsilon):
    return compute_serial_time(ds, datafile, epsilon, 'fpgrowth-tiny')

def compute_serial_best(ds, datafile, epsilon):
    serialtime = None
    key = 'fpgrowth-tiny:' + datafile + ':' + epsilon
    if ds.has_key(key):
        if serialtime==None:
            serialtime = ds[key]
        else:
            serialtime = ds[key]
    key =  'aim:' + datafile + ':' + epsilon
    if ds.has_key(key):
        if serialtime==None:
            serialtime = ds[key]
        else:
            serialtime = min(serialtime, ds[key])
    key =  'kdci:' + datafile + ':' + epsilon
    if ds.has_key(key):
        if serialtime==None:
            serialtime = ds[key]
        else:
            serialtime = min(serialtime, ds[key])
    key =  'lcm20:' + datafile + ':' + epsilon
    if ds.has_key(key):
        if serialtime==None:
            serialtime = ds[key]
        else:
            serialtime = min(serialtime, ds[key])
    return serialtime

# process arguments
def proc_args(datafiles='datafiles'):
    print 'program', sys.argv[0]
    if len(sys.argv)>=2:
        datafilesname = sys.argv[1]
    else:
        datafilesname = datadir + datafiles
    print 'reading problem instances from ', datafilesname
    f  = open(datafilesname, 'r')
    datafiles = f.readlines()
    f.close()
    if len(sys.argv)>=3:
        load_estimate = int(sys.argv[2])
    else:
        load_estimate = default_le
    print 'load estimate: ', load_estimate

    if not os.path.exists('parallel'):
        os.makedirs('parallel')
    if not os.path.exists('serial'):
        os.makedirs('serial')

    
    return (datafiles, load_estimate)



def parse_cmd_line():
    
    # parse command line options
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hSscp", 
                                   ["help", "stop", "sync", 
                                    "config=", "profile"])
    except getopt.error, msg:
        print msg
        print "for help use --help"
        sys.exit(2) #TODO: raise exception instead

    # construct option object
    opt = options()

    # process options
    for o, a in opts:
        if o in ("-h", "--help"):
            print __doc__
            sys.exit(0)
        if o in ("-S", "--stop"):
            opt.stop = True
        if o in ("-s", "--sync"):
            opt.sync = True 
        if o in ("-c", "--config"):
            opt.config=a
        if o in ("-p", "--profile"):
            opt.profile = True 

    # process arguments

    if opt.config==None:
        raise Exception("no config specified") 

    if len(args)>=1:
        opt.benchmarkfile = args[0]
    else:
        opt.benchmarkfile = benchmarkfile
    print 'reading benchmark parameters from ', opt.benchmarkfile
    f  = open(opt.benchmarkfile, 'r')
    workls = f.readlines()
    f.close()
    print 'configuration %s\n' % opt.config 

    configmap = configs[opt.config]
    opt.procs = configmap['procs']
    opt.procs2d = configmap['procs2d']
    opt.tmpdir = configmap['tmpdir']
    opt.mpirun = configmap['mpirun']
    opt.mpiclean = configmap['mpiclean']

    return (opt, workls)
