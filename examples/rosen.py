"""
Is the hessian even supported by pyipopt?

There is a comment here
http://www.wstein.org/home/wstein/www/home/was/patches/
openopt-0.24/src/openopt/solvers/CoinOr/ipopt_oo.py
suggesting that the pyipopt hessian support may be buggy.
Also check some bug reports here:
http://code.google.com/p/pyipopt/issues/list
?can=1&q=&colspec=ID+Type+Status+Priority+Milestone+Owner+Summary&cells=tiles
"""

import numpy
import scipy.optimize
import pyipopt

def eval_f(X, user_data=None):
    """
    Directly evaluate the objective function f.
    """
    return scipy.optimize.rosen(X)

def eval_grad_f(X, user_data=None):
    """
    Evaluate the gradient of the objective function f.
    """
    return scipy.optimize.rosen_der(X)

def eval_g(X, user_data=None):
    """
    Evaluate the constraint functions.
    """
    return numpy.array([], dtype=float)

def eval_jac_g(X, flag, user_data=None):
    """
    Evaluate the sparse Jacobian of constraint functions g.
    @param X: parameter values
    @param flag: this asks for the sparsity structure
    """
    print 'eval_jac_g'
    print X
    print flag
    print user_data
    print
    #XXX
    if flag:
        rows = numpy.array([], dtype=int)
        cols = numpy.array([], dtype=int)
        return (rows, cols)
    else:
        return numpy.array([], dtype=float)

def eval_h(X, lagrange, obj_factor, flag, user_data=None):
    """
    Evaluate the sparse hessian of the Lagrangian.
    @param X: parameter values
    @param lagrange: something about the constraints
    @param obj_factor: no clue what this is
    @param flag: this asks for the sparsity structure
    """
    #XXX
    print 'eval_h:'
    print X
    print lagrange
    print obj_factor
    print flag
    print user_data
    print
    rows = numpy.array([0, 1, 1], dtype=int)
    cols = numpy.array([0, 0, 1], dtype=int)
    if flag:
        return (rows, cols)
    else:
        # XXX
        # these values are meaningless
        values = numpy.zeros(3, dtype=float)
        #values[0] = obj_factor*2
        #values[1] = 0
        #values[2] = obj_factor*2
        H = scipy.optimize.rosen_hess(X)
        for i, (r, c) in enumerate(zip(rows, cols)):
            values[i] = H[r, c] * obj_factor
        return values

def apply_new(X):
    """
    What is this?
    """
    #XXX
    print 'apply_new:'
    print X
    print
    return True


def main():

    # verbose
    pyipopt.set_loglevel(2)

    # define the parameters and their box constraints
    nvar = 2
    x_L = numpy.array([-3, -3], dtype=float)
    x_U = numpy.array([3, 3], dtype=float)

    # define the inequality constraints
    ncon = 0
    g_L = numpy.array([], dtype=float)
    g_U = numpy.array([], dtype=float)

    # define the number of nonzeros in the jacobian and in the hessian
    # there are no nonzeros in the constraint jacobian
    nnzj = 0

    # there are maximum nonzeros (nvar*(nvar+1))/2 in the lagrangian hessian
    nnzh = 3

    # create the nonlinear programming model
    nlp = pyipopt.create(
            nvar,
            x_L,
            x_U,
            ncon,
            g_L,
            g_U,
            nnzj,
            nnzh,
            eval_f,
            eval_grad_f,
            eval_g,
            eval_jac_g,
            eval_h,
            apply_new,
            )

    # define the initial guess
    x0 = numpy.array([-1.2, 1], dtype=float)

    # compute the results using ipopt
    results = nlp.solve(x0)

    # free the model
    nlp.close()

    # report the results
    print results


if __name__ == '__main__':
    main()

