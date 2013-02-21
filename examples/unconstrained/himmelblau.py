"""
Minimize a standard unconstrained test function.

This example uses algopy for the gradient and hessian.
"""

import functools

import numpy
import algopy

import pyipopt

def himmelblau(X):
    """
    http://en.wikipedia.org/wiki/Himmelblau%27s_function
    This function has four local minima where the value of the function is 0.
    """
    x = X[0]
    y = X[1]
    a = x*x + y - 11
    b = x + y*y - 7
    return a*a + b*b

def eval_grad(f, theta):
    theta = algopy.UTPM.init_jacobian(theta)
    return algopy.UTPM.extract_jacobian(f(theta))

def eval_hess(f, theta):
    theta = algopy.UTPM.init_hessian(theta)
    return algopy.UTPM.extract_hessian(len(theta), f(theta))

def main():
    pyipopt.set_loglevel(2)
    x0 = numpy.array([-0.27, -0.9], dtype=float)
    results = pyipopt.fmin_unconstrained(
            himmelblau,
            x0,
            fprime=functools.partial(eval_grad, himmelblau),
            fhess=functools.partial(eval_hess, himmelblau),
            )
    print results

if __name__ == '__main__':
    main()
