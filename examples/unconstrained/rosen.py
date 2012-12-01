"""
Minimize the Rosenbrock function with the unconstrained minimization interface.

See the rosen.py example for more details.
"""

import numpy
import scipy.optimize

import pyipopt

def main():
    pyipopt.set_loglevel(2)
    x0 = numpy.array([-1.2, 1], dtype=float)
    results = pyipopt.fmin_unconstrained(
            scipy.optimize.rosen,
            x0,
            fprime=scipy.optimize.rosen_der,
            fhess=scipy.optimize.rosen_hess,
            )
    print results

if __name__ == '__main__':
    main()
