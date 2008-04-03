import pyipopt
import amplpy, _amplpy
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

def eval_f(x):
	return nlp.obj(x)

def eval_grad_f(x):
	return nlp.grad(x)	# return ndarray

def eval_g(x):
	return nlp.cons(x)
	
def eval_jac_g(x, flag):
	if flag:
		dummy, row1, col1 = nlp.jac(array(x0))
		return (row1, col1)
	else:
		j, dummyr, dummyc = nlp.jac(x)
		return j

dummyh, hrow, hcol = nlp.hess( nlp.x0, nlp.pi0, 1.0 )

def eval_h(x, lagrange, obj_factor, flag):
	if flag:
		return (hrow.tolist(), hcol.tolist())
	else:
		#applynew(x)
		temph, dummyr, dummyc = nlp.hess(x, lagrange, obj_factor)
		return temph

def applynew(x):
	return nlp.applynew(x);

pyipopt.create(n, xl, xu, m, gl, gu, nnzj, 0, eval_f, eval_grad_f, eval_g, eval_jac_g)
print pyipopt.solve(x0)

pyipopt.create(n, xl, xu, m, gl, gu, nnzj, nnzh, eval_f, eval_grad_f, eval_g, eval_jac_g, eval_h, applynew)
print pyipopt.solve(x0)

