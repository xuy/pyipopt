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

xl = nlp.Lvar.tolist()
xu = nlp.Uvar.tolist()
gl = nlp.Lcon.tolist()
gu = nlp.Ucon.tolist()
x0 = nlp.x0.tolist()
pi0 = nlp.pi0.tolist()

m = nlp.m
n = nlp.n
nnzj = nlp.nnzj
nnzh = nlp.nnzh

def eval_f(x):
	return nlp.obj(array(x))

def eval_grad_f(x):
	return nlp.grad(array(x)).tolist()	# return ndarray

def eval_g(x):
	return  nlp.cons(array(x)).tolist()
	
def eval_jac_g(x, flag):
	if flag:
		dummy, row1, col1 = nlp.jac(array(x0))
		return (row1.tolist(), col1.tolist())
	else:
		j, dummyr, dummyc = nlp.jac(array(x))
		return j.tolist()

"""
dummyh, hrow, hcol = _amplpy.eval_H( nlp.x0, nlp.pi0, 1, 1.0 )
print hrow, hcol
dummyh, hrow, hcol = nlp.hess( nlp.x0, nlp.pi0, 1.0 )

def eval_h(x, lagrange, obj_factor, flag):
	if flag:
		return (hrow.tolist(), hcol.tolist())
	else:
		temph, dummyr, dummyc = nlp.hess(array(x), array(lagrange), obj_factor)
		return temph.tolist()
"""
pyipopt.create(n, xl, xu, m, gl, gu, nnzj, 0, eval_f, eval_grad_f, eval_g, eval_jac_g)
pyipopt.solve(x0)
