//  Author: Eric Xu
//  Licensed under BSD
#include <stdio.h>

#include "IpStdCInterface.h"
#include "IpTypes.h"
#include "numpy/ndarrayobject.h"
#include "Python.h"

#ifndef PY_IPOPT_HOOK_
#define PY_IPOPT_HOOK_

// A series of callback functions used by Ipopt C Interface
Bool eval_f(ipindex n,
	    ipnumber * x, Bool new_x, ipnumber * obj_value, UserDataPtr user_data);

Bool eval_grad_f(ipindex n,
		 ipnumber * x,
		 Bool new_x, ipnumber * grad_f, UserDataPtr user_data);

Bool eval_g(ipindex n, ipnumber * x, Bool new_x, ipindex m, ipnumber * g, UserDataPtr user_data);

Bool eval_jac_g(ipindex n, ipnumber * x, Bool new_x,
		ipindex m, ipindex nele_jac,
		ipindex * iRow, ipindex * jCol, ipnumber * values,
		UserDataPtr user_data);

Bool eval_h(ipindex n, ipnumber * x, Bool new_x, ipnumber obj_factor,
	    ipindex m, ipnumber * lambda, Bool new_lambda,
	    ipindex nele_hess, ipindex * iRow, ipindex * jCol,
	    ipnumber * values, UserDataPtr user_data);

Bool eval_intermediate_callback(ipindex alg_mod,
				ipindex iter_count, ipnumber obj_value,
				ipnumber inf_pr, ipnumber inf_du,
				ipnumber mu, ipnumber d_norm,
				ipnumber regularization_size,
				ipnumber alpha_du, ipnumber alpha_pr,
				ipindex ls_trials, UserDataPtr data);

typedef struct {
	PyObject *eval_f_python;
	PyObject *eval_grad_f_python;
	PyObject *eval_g_python;
	PyObject *eval_jac_g_python;
	PyObject *eval_h_python;
	PyObject *apply_new_python;
	PyObject *eval_intermediate_callback_python;
	PyObject *userdata;
} DispatchData;


#if PY_MAJOR_VERSION < 3
PyObject *problem_getattr(PyObject * self, char *attrname);
#endif

/* Logging */
#define VERBOSE 2
#define IPOPT_OUTPUT 1
#define TERSE 0
extern int user_log_level;
void logger(const char *fmt, ...);

typedef struct {
	PyObject_HEAD IpoptProblem nlp;
	DispatchData *data;
	ipindex n_variables;
	ipindex m_constraints;
} problem;

#endif				//  PY_IPOPT_HOOK_
