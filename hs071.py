#!/usr/bin/python

# Author: Eric Xu. Washington University
#  The same model as Ipopt/examples/hs071

import pyipopt
from numpy import *

nvar = 4
x_L = ones((nvar), dtype=float_) * 1.0
x_U = ones((nvar), dtype=float_) * 5.0

ncon = 2

g_L = array([25.0, 40.0])
g_U = array([2.0*pow(10.0, 19), 40.0]) 

def eval_f(x, user_data = None):
	assert len(x) == 4
	return x[0] * x[3] * (x[0] + x[1] + x[2]) + x[2]

def eval_grad_f(x, user_data = None):
	assert len(x) == 4
	grad_f = array([
  		x[0] * x[3] + x[3] * (x[0] + x[1] + x[2]) , 
  		x[0] * x[3],
  		x[0] * x[3] + 1.0,
  		x[0] * (x[0] + x[1] + x[2])
  		], float_)
	return grad_f;
	
def eval_g(x, user_data= None):
	assert len(x) == 4
	return array([
		x[0] * x[1] * x[2] * x[3], 
		x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3]
	], float_)

nnzj = 8
def eval_jac_g(x, flag, user_data = None):
	if flag:
		return (array([0, 0, 0, 0, 1, 1, 1, 1]), 
			array([0, 1, 2, 3, 0, 1, 2, 3]))
	else:
		assert len(x) == 4
		return array([ x[1]*x[2]*x[3], 
					x[0]*x[2]*x[3], 
					x[0]*x[1]*x[3], 
					x[0]*x[1]*x[2],
					2.0*x[0], 
					2.0*x[1], 
					2.0*x[2], 
					2.0*x[3] ])
		
nnzh = 10
def eval_h(x, lagrange, obj_factor, flag, user_data = None):
	if flag:
		hrow = [0, 1, 1, 2, 2, 2, 3, 3, 3, 3]
		hcol = [0, 0, 1, 0, 1, 2, 0, 1, 2, 3]
		return (array(hcol), array(hrow))
	else:
		values = zeros((10), float_)
		values[0] = obj_factor * (2*x[3])
		values[1] = obj_factor * (x[3])
		values[2] = 0
		values[3] = obj_factor * (x[3])
		values[4] = 0
		values[5] = 0
		values[6] = obj_factor * (2*x[0] + x[1] + x[2])
		values[7] = obj_factor * (x[0])
		values[8] = obj_factor * (x[0])
		values[9] = 0
		values[1] += lagrange[0] * (x[2] * x[3])

		values[3] += lagrange[0] * (x[1] * x[3])
		values[4] += lagrange[0] * (x[0] * x[3])

		values[6] += lagrange[0] * (x[1] * x[2])
		values[7] += lagrange[0] * (x[0] * x[2])
		values[8] += lagrange[0] * (x[0] * x[1])
		values[0] += lagrange[1] * 2
		values[2] += lagrange[1] * 2
		values[5] += lagrange[1] * 2
		values[9] += lagrange[1] * 2
		return values

def apply_new(x):
	return True
	
nlp = pyipopt.create(nvar, x_L, x_U, ncon, g_L, g_U, nnzj, nnzh, eval_f, eval_grad_f, eval_g, eval_jac_g)

x0 = array([1.0, 5.0, 5.0, 1.0])
pi0 = array([1.0, 1.0])

"""
print x0
print nvar, ncon, nnzj
print x_L,  x_U
print g_L, g_U
print eval_f(x0)
print eval_grad_f(x0)
print eval_g(x0)
a =  eval_jac_g(x0, True)
print "a = ", a[1], a[0]
print eval_jac_g(x0, False)
print eval_h(x0, pi0, 1.0, False)
print eval_h(x0, pi0, 1.0, True)
"""

""" You can set Ipopt options by calling nlp.num_option, nlp.str_option
or nlp.int_option. For instance, to set the tolarance by calling

	nlp.num_option('tol', 1e-8)

For a complete list of Ipopt options, refer to

	http://www.coin-or.org/Ipopt/documentation/node59.html

Note that Ipopt distinguishs between Int, Num, and Str options, yet sometimes
does not explicitly tell you which option is which.  If you are not sure about
the option's type, just try it in PyIpopt.  If you try to set one type of
option using the wrong function, Pyipopt will remind you of it. """

print "Going to call solve"
print x0
x, zl, zu, constraint_multipliers, obj, status = nlp.solve(x0)
# import pdb; pdb.set_trace()
nlp.close()

def print_variable(variable_name, value):
  for i in xrange(len(value)):
    print variable_name + "["+str(i)+"] =", value[i]

print
print "Solution of the primal variables, x"
print_variable("x", x)
print
print "Solution of the bound multipliers, z_L and z_U"
print_variable("z_L", zl)
print_variable("z_U", zu)
print
print "Solution of the constraint multipliers, lambda"
print_variable("lambda", constraint_multipliers)
print
print "Objective value"
print "f(x*) =", obj

