"""
Minimize a standard unconstrained test function.

This example uses algopy for the gradient and hessian.
"""

import functools

import numpy
import algopy

import pyipopt

def wood(X):
    """
    The minimum is at [1, 1, 1, 1].
    """
    x1 = X[0]
    x2 = X[1]
    x3 = X[2]
    x4 = X[3]
    return sum((
        100*(x1*x1 - x2)**2,
        (x1-1)**2,
        (x3-1)**2,
        90*(x3*x3 - x4)**2,
        10.1*((x2-1)**2 + (x4-1)**2),
        19.8*(x2-1)*(x4-1),
        ))

def eval_grad(f, theta):
    theta = algopy.UTPM.init_jacobian(theta)
    return algopy.UTPM.extract_jacobian(f(theta))

def eval_hess(f, theta):
    theta = algopy.UTPM.init_hessian(theta)
    return algopy.UTPM.extract_hessian(len(theta), f(theta))

def main():
    pyipopt.set_loglevel(2)
    x0 = numpy.array([-3, -1, -3, -1], dtype=float)
    results = pyipopt.fmin_unconstrained(
            wood,
            x0,
            fprime=functools.partial(eval_grad, wood),
            fhess=functools.partial(eval_hess, wood),
            )
    print results

if __name__ == '__main__':
    main()
