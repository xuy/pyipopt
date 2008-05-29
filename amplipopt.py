import pyipopt
import amplpy
import sys, getopt
from numpy import *

PROGNAME = sys.argv[0]

def commandline_err( msg ):
    sys.stderr.write( "%s: %s\n" % ( PROGNAME, msg ) )
    sys.exit( 1 )

def parse_cmdline( arglist ):
    if len( arglist ) != 1:
        commandline_err( 'I am hungry for an NL file\n\t Try ./pycpre filename.nl.' )
        return None
    try: options, fname = getopt.getopt( arglist, '' )
    except getopt.error, e:
        commandline_err( "%s" % str( e ) )
        return None
    return fname[0]

ProblemName = parse_cmdline(sys.argv[1:])

nlp = amplpy.AmplModel( ProblemName , opts=1)

xl = nlp.Lvar
xu = nlp.Uvar
gl = nlp.Lcon
gu = nlp.Ucon
x0 = nlp.x0

m = nlp.m
n = nlp.n
nnzj = nlp.nnzj
nnzh = nlp.nnzh

def eval_f(x, data = None):
	return nlp.obj(x)

def eval_grad_f(inx, data= None):
	assert len(inx) == n
	inx = inx*1.0
	return nlp.grad(inx)


def eval_g(x, data= None):
	return nlp.cons(x)

		
def eval_jac_g(x, flag, data=None):
	if flag:
		dummy, row1, col1 = nlp.jac(x0)
		return (row1, col1)
	else:
		j, dummyr, dummyc = nlp.jac(x)
		return j

dummyh, hrow, hcol = nlp.hess( nlp.x0, nlp.pi0, 1.0)

def eval_h(x, lagrange, obj_factor, flag, data = None):
	if flag:
		return (hrow, hcol)
	else:
		#applynew(x)
		temph, dummyr, dummyc = nlp.hess(x, lagrange, obj_factor)
		return temph

def applynew(x):
	return nlp.applynew(x);


#print eval_grad_f
"""
print n, m
print eval_f(x0)
assert len(eval_grad_f(x0)) == n
print eval_grad_f(x0)

assert len(eval_g(x0)) == m
a =  eval_jac_g(x0, True)
print "row, col ", a[0], a[1]
print eval_jac_g(x0, False)

print eval_h(True, True, True, True)

print eval_h(nlp.x0, nlp.pi0, 1.0, True)

def gf2(x, data = None):
	return array([0.0 for i in xrange(m)])

def f2(x, data = None):
	return 0
"""

def applynew(x):
	return True	
	
#print nnzh
problem = pyipopt.create(n, xl, xu, m, gl, gu, nnzj, 0, eval_f, eval_grad_f, eval_g, eval_jac_g)

problem.solve(x0)
#problem.close()
#nlp = pyipopt.create(n, xl, xu, m, gl, gu, nnzj, nnzh, eval_f, eval_grad_f, eval_g, eval_jac_g, eval_h, applynew)
#print nlp.solve(x0)

