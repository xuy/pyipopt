"""
This is a package for a python interface to ipopt.

The underlying C interface is in pyipoptcore.
"""

import functools

import numpy
from pyipoptcore import *

# verbose messages from the C interface
set_loglevel(2)

# http://www.coin-or.org/Ipopt/documentation/node35.html
# FIXME: these are not actually constant but may be changed within ipopt
NLP_LOWER_BOUND_INF = -1e19
NLP_UPPER_BOUND_INF = 1e19


def unconstrained_eval_g(X, user_data=None):
    return numpy.array([], dtype=float)

def unconstrained_eval_jac_g(X, flag, user_data=None):
    rows = numpy.array([], dtype=int)
    cols = numpy.array([], dtype=int)
    if flag:
        return (rows, cols)
    else:
        raise Exception(
                'this should not be called for unconstrained optimization')

def unconstrained_eval_h(
        h, nvar,
        X, lagrange, obj_factor, flag, user_data=None):
    """
    The first group of parameters should be applied using functools.partial.
    The second group of parameters are passed from ipopt.
    @param h: a function to compute the hessian.
    @param nvar: the number of parameters
    @param X: parameter values
    @param lagrange: something about the constraints
    @param obj_factor: no clue what this is
    @param flag: this asks for the sparsity structure
    @param user_data: please do not use this yet
    """

    # Get the nonzero (row, column) entries of a lower triangular matrix.
    # This is related to the fact that the Hessian is symmetric,
    # and that ipopt is designed to work with sparse matrices.
    row_list = []
    col_list = []
    for row in range(nvar):
        for col in range(row+1):
            row_list.append(row)
            col_list.append(col)
    rows = numpy.array(row_list, dtype=int)
    cols = numpy.array(col_list, dtype=int)

    if flag:
        return (rows, cols)
    else:
        if nvar != len(X):
            raise Exception('parameter count mismatch')
        if lagrange:
            raise Exception('only unconstrained is implemented for now...')
        values = numpy.zeros(len(rows), dtype=float)
        H = h(X)
        for i, (r, c) in enumerate(zip(rows, cols)):
            #FIXME: am I using obj_factor correctly?
            # I don't really know what it is...
            values[i] = H[r, c] * obj_factor
        return values

def apply_new(X):
    #FIXME: I don't really know what this does, but ipopt wants it.
    return True

def create_unconstrained(f, nvar, fprime, fhess=None):
    """
    Creates an ipopt nlp object.
    @param f: objective function to minimize
    @param nvar: number of parameters
    @param fprime: computes the gradient of the objective function
    @param fhess: computes the hessian of the objective function
    @return: a pyipopt nlp object which may be solved and then closed
    """

    # no box constraints on the parameters
    x_L = numpy.array([NLP_LOWER_BOUND_INF]*nvar, dtype=float)
    x_U = numpy.array([NLP_UPPER_BOUND_INF]*nvar, dtype=float)
    
    # no other constraints
    ncon = 0
    g_L = numpy.array([], dtype=float)
    g_U = numpy.array([], dtype=float)

    # no constraint jacobian
    nnzj = 0

    # dense lower triangular hessian structure
    nnzh = (nvar * (nvar + 1)) // 2

    # define the nlp creation args
    nlp_args = [
            nvar,
            x_L,
            x_U,
            ncon,
            g_L,
            g_U,
            nnzj,
            nnzh,
            f,
            fprime,
            unconstrained_eval_g,
            unconstrained_eval_jac_g,
            ]
    if fhess:
        partial_h = functools.partial(unconstrained_eval_h, fhess, nvar)
        nlp_args.extend([
            partial_h,
            apply_new,
            ])
    else:
        partial_h = None

    # create the nlp object and the partial_h
    #FIXME: the returning of partial_h works around a bug in the C extension
    return create(*nlp_args), partial_h

def fmin_unconstrained(f, x0, fprime, fhess=None):
    """
    This is a utility function wrapping create_unconstrained.
    @param f: objective function to minimize
    @param x0: initial guess
    @param fprime: computes the gradient of the objective function
    @param fhess: computes the hessian of the objective function
    @return: results in pyipoptcore format
    """
    nvar = len(x0)
    #FIXME:
    # The partial_h seems unnecessary here, but it works around a bug.
    # This bug is in the reference counting within the C extension --
    # the C extension buggily assumes that callbacks will
    # never go out of scope while nlp is active,
    # but this assumption is violated when the callbacks are partial
    # function evaluation objects.
    # This problem can be fixed by debugging the C code
    # so that the nlp object is more grabby with respect to the
    # python callback objects.
    # When this is fixed, create_unconstrained will be able to return
    # only nlp insead of (nlp, partial_h) without segfaulting
    # when the nlp is solved; in other words, the python framework
    # needs to know that nlp still cares about the callback object
    # so that the callback object will not be destroyed
    # until nlp is closed.
    nlp, partial_h = create_unconstrained(f, nvar, fprime, fhess)
    results = nlp.solve(x0)
    nlp.close()
    return results

