import string
import subprocess


# pretty print a list
def strlist(l):
    return string.join(map(lambda x: str(x), l))


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
