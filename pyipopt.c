/* Copyright (c) 2008, Eric You Xu, Washington Univeristy
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Washington Univeristy nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* Version 0.2 All numpy types */

#include "Python.h"
#include "IpStdCInterface.h"
#include <stdio.h>
#include "hook.h"
#include "numpy/arrayobject.h"       /* NumPy header */

static PyObject *eval_f_python 		= NULL;
static PyObject *eval_grad_f_python = NULL;
static PyObject *eval_g_python 		= NULL;
static PyObject *eval_jac_g_python 	= NULL;
static PyObject *eval_h_python		= NULL;
static PyObject *apply_new_python	= NULL;

static IpoptProblem nlp = NULL;             /* IpoptProblem */
static int n;
static int m;


/*  Call back function section 
	Adapters that delegate actual  function to Python 
	
	C callback function interface 
		|- Prepare Python object
		|- Call Python Function
		|- Return c type value
	use pyclear to clean any data I created (I control the ownship)
	use py-deref to decrease other data (get from other function, etc)
*/

Bool eval_f(Index n, Number* x, Bool new_x,
            Number* obj_value, UserDataPtr user_data)
{
	
	int dims[1];
	dims[0] = n;
	
	PyObject *arrayx = PyArray_FromDimsAndData(1, dims, PyArray_DOUBLE , (char*) x);
	if (!arrayx) return FALSE;

	if (new_x && apply_new_python) {
		/* Call the python function to applynew */
		PyObject* arg1 = Py_BuildValue("(O)", arrayx);
		PyObject* tempresult = PyObject_CallObject (apply_new_python, arg1);
		if (!tempresult) {
			printf("[Error] Python function apply_new returns a None\n");
			Py_DECREF(arg1);	
			return FALSE;
		}
		Py_DECREF(arg1);
		Py_DECREF(tempresult);
	}

	PyObject* arglist = Py_BuildValue("(O)", arrayx);
	PyObject* result  = PyObject_CallObject (eval_f_python ,arglist);

	if (!PyFloat_Check(result))
		PyErr_Print();
	
	*obj_value =  PyFloat_AsDouble(result);
	Py_DECREF(result);
  	Py_DECREF(arrayx);
	Py_CLEAR(arglist);
  	return TRUE;
}

static Bool eval_grad_f(Index n, Number* x, Bool new_x,
                 Number* grad_f, UserDataPtr user_data)
{
	if (eval_grad_f_python == NULL) PyErr_Print();
	
	int dims[1];
	dims[0] = n;
	
	PyObject *arrayx = PyArray_FromDimsAndData(1, dims, PyArray_DOUBLE , (char*) x);
	if (!arrayx) return FALSE;
	
	if (new_x && apply_new_python) {
		/* Call the python function to applynew */
		PyObject* arg1 = Py_BuildValue("(O)", arrayx);
		PyObject* tempresult = PyObject_CallObject (apply_new_python, arg1);
		if (!tempresult) {
			printf("[Error] Python function apply_new returns a None\n");
			Py_DECREF(arg1);	
			return FALSE;
		}
		Py_DECREF(arg1);
		Py_DECREF(tempresult);
	}	
	
	PyObject* arglist = Py_BuildValue("(O)", arrayx);
	PyArrayObject* result = (PyArrayObject*) PyObject_CallObject (eval_grad_f_python, arglist);
	
	if (!result) 
		PyErr_Print();
		
	if (!PyArray_Check(result))
		PyErr_Print();
	
	double *data = (double*)result->data;
	int i;
	for (i = 0; i < n; i++)
		grad_f[i] = data[i];
		
	Py_DECREF(result);
  	Py_CLEAR(arrayx);
	Py_CLEAR(arglist);	
	return TRUE;
}


Bool eval_g(Index n, Number* x, Bool new_x,
            Index m, Number* g, UserDataPtr user_data)
{
	if (eval_g_python == NULL) 
		PyErr_Print();
	
	int dims[1];
	dims[0] = n;
	
	PyObject *arrayx = PyArray_FromDimsAndData(1, dims, PyArray_DOUBLE , (char*) x);
	if (!arrayx) return FALSE;
	
	if (new_x && apply_new_python) {
		/* Call the python function to applynew */
		PyObject* arg1 = Py_BuildValue("(O)", arrayx);
		PyObject* tempresult = PyObject_CallObject (apply_new_python, arg1);
		if (!tempresult) {
			printf("[Error] Python function apply_new returns a None\n");
			Py_DECREF(arg1);	
			return FALSE;
		}
		Py_DECREF(arg1);
		Py_DECREF(tempresult);
	}
	
	PyObject* arglist = Py_BuildValue("(O)", arrayx);
	PyArrayObject* result = (PyArrayObject*) PyObject_CallObject (eval_g_python, arglist);
	
	if (!result) 
		PyErr_Print();
		
	if (!PyArray_Check(result))
		PyErr_Print();
	
	double *data = (double*)result->data;
	int i;
	for (i = 0; i < m; i++)
		g[i] = data[i];
		
	Py_DECREF(result);
  	Py_CLEAR(arrayx);
	Py_CLEAR(arglist);
	return TRUE;
}


Bool eval_jac_g(Index n, Number *x, Bool new_x,
                Index m, Index nele_jac,
                Index *iRow, Index *jCol, Number *values,
                UserDataPtr user_data)
{
	int i;
	long* rowd = NULL; 
	long* cold = NULL;
	
	int dims[1];
	dims[0] = n;
	
	double *data;

	if (eval_grad_f_python == NULL) 
		PyErr_Print();
		
	if (values == NULL) {		
		PyObject *newx = Py_True;
		
		PyObject* arglist = Py_BuildValue("(OO)", newx, Py_True);	
		PyObject* result = PyObject_CallObject (eval_jac_g_python, arglist);
		
		
		if (!PyTuple_Check(result))
			PyErr_Print();
			
		PyArrayObject* row = (PyArrayObject*) PyTuple_GetItem(result, 0);
		PyArrayObject* col = (PyArrayObject*) PyTuple_GetItem(result, 1);
		
		if (!row || !col || !PyList_Check(row) || !PyList_Check(col))
			PyErr_Print();

		rowd = (long*) row->data;
		cold = (long*) col->data;
		
		//printf("I am here, before copy\n");
		for (i = 0; i < nele_jac; i++) {
			iRow[i] = (int) rowd[i];
			jCol[i] = (int) cold[i];
		}
		
		Py_DECREF(result);
		Py_CLEAR(arglist);
		
	}
	else {	// Assign the jac_g
		
		PyObject *arrayx = PyArray_FromDimsAndData(1, dims, PyArray_DOUBLE , (char*) x);
		if (!arrayx) return FALSE;
		
		if (new_x && apply_new_python) {
			/* Call the python function to applynew */
			PyObject* arg1 = Py_BuildValue("(O)", arrayx);
			PyObject* tempresult = PyObject_CallObject (apply_new_python, arg1);
			if (!tempresult) {
				printf("[Error] Python function apply_new returns a None\n");
				Py_DECREF(arg1);	
				return FALSE;
			}
			Py_DECREF(arg1);
			Py_DECREF(tempresult);
		}
		
		PyObject* arglist = Py_BuildValue("(OO)", arrayx, Py_False);
		PyArrayObject* result = (PyArrayObject*) 
					PyObject_CallObject (eval_jac_g_python, arglist);
		
		if (!result || !PyArray_Check(result)) 
			PyErr_Print();
		
		data = (double*)result->data;
		
		for (i = 0; i < nele_jac; i++)
			values[i] = data[i];
		
		Py_DECREF(result);
		Py_CLEAR(arrayx);
		Py_CLEAR(arglist);
	}
  	return TRUE;
}

static Bool eval_h(Index n, Number *x, Bool new_x, Number obj_factor,
            Index m, Number *lambda, Bool new_lambda,
            Index nele_hess, Index *iRow, Index *jCol,
            Number *values, UserDataPtr user_data)
{
	int i;
	if (eval_h_python == NULL)
		return FALSE;
	
	
	// printf("I am in eval_h\n");
	if (values == NULL) {
		PyObject *newx = Py_True;
		PyObject *objfactor = Py_BuildValue("d", obj_factor);
		PyObject *lagrange = Py_True;	// Booleans don't need the ref count
		
		PyObject* arglist = Py_BuildValue("(OOOO)", newx, lagrange, objfactor, Py_True);
		PyObject* result = PyObject_CallObject (eval_h_python, arglist);
		if (!PyTuple_Check(result))
			PyErr_Print();
			
		PyArrayObject* row = (PyArrayObject*)PyTuple_GetItem(result, 0);	//steal
		PyArrayObject* col = (PyArrayObject*)PyTuple_GetItem(result, 1); //steal

/*
		if (!PyList_Check(row))
			PyErr_Print();
		if (!PyList_Check(col))
			PyErr_Print();
*/

		double* rdata = (double*)row->data;
		double* cdata = (double*)col->data;
		
		for (i = 0; i < nele_hess; i++) {
			iRow[i] = (int)rdata[i];
			jCol[i] = (int)cdata[i];
			// PyArg_Parse(PyList_GetItem(row, i), "i", &iRow[i]);
			// PyArg_Parse(PyList_GetItem(col, i), "i", &jCol[i]);
		}

		// newx and other's are just transparent pointer to Py_True
		Py_DECREF(objfactor);
		Py_DECREF(result);
		Py_CLEAR(arglist);
	}
	else {	// Assign the hess
		// PyObject *newx = PyList_New(n);
		// PyObject *lagrange = PyList_New(m);
		PyObject *objfactor = Py_BuildValue("d", obj_factor);
		
		int dims[1];
		dims[0] = n;
		PyObject *arrayx = PyArray_FromDimsAndData(1, dims, PyArray_DOUBLE , (char*) x);
		if (!arrayx) return FALSE;
		
		if (new_x && apply_new_python) {
			/* Call the python function to applynew */
			PyObject* arg1 = Py_BuildValue("(O)", arrayx);
			PyObject* tempresult = PyObject_CallObject (apply_new_python, arg1);
			if (!tempresult) {
				printf("[Error] Python function apply_new returns a None\n");
				Py_DECREF(arg1);	
				return FALSE;
			}
			Py_DECREF(arg1);
			Py_DECREF(tempresult);
		}
		
		int dims2[1];
		dims2[0] = m;
		PyObject *lagrangex = PyArray_FromDimsAndData(1, dims2, PyArray_DOUBLE , (char*) lambda);
		if (!lagrangex) return FALSE;
		
		PyObject* arglist = Py_BuildValue("(OOOO)", arrayx, lagrangex, objfactor, Py_False);
		PyArrayObject* result = (PyArrayObject*) PyObject_CallObject (eval_h_python, arglist);
		
		if (!result) printf("[Error] Python function eval_h returns a None\n");
		
		double* data = (double*)result->data;
		for (i = 0; i < nele_hess; i++)
			values[i] = data[i];
			
		Py_CLEAR(arrayx);
		Py_CLEAR(lagrangex);
		Py_CLEAR(objfactor);
		Py_DECREF(result);
		Py_CLEAR(arglist);
	}	                         
  	return TRUE;
}
/*  Ends Call back function section */

/* Interface to Python */
// Crate problem

static char PYIPOPT_CREATE_DOC[] = "create(n, xl, xu, m, gl, gu, nnzj, nnzh, eval_f, eval_grad_f, eval_g, eval_jac_g) -> Boolean\n \
        \n \
        Create a problem instance and return True if successed  \n \
        \n \
        n is the number of variables, \n \
        xl is the lower bound of x as bounded constraints \n \
        xu is the upper bound of x as bounded constraints \n \
        	both xl, xu should be one dimension arrays with length n \n \
        \n \
        m is the number of constraints, \n \
        gl is the lower bound of constraints \n \
        gu is the upper bound of constraints \n \
        	both gl, gu should be one dimension arrays with length m \n \
        nnzj is the number of nonzeros in Jacobi matrix \n \
        nnzh is the number of nonzeros in Hessian matrix, you can set it to 0 \n \
        \n \
        eval_f is the call back function to calculate objective value, \n \
        	it takes one single argument x as input vector \n \
        eval_grad_f calculates gradient for objective function \n \
        eval_g calculates the constraint values and return an array \n \
        eval_jac_g calculates the jacobi matrix. It takes two arguments, \n \
        	the first is the variable x and the second is a Boolean flag \n \
        	if the flag is true, it supposed to return a tuple (row, col) \n \
        		to indicate the sparse Jacobian matrix's structure. \n \
        	if the flag is false if returns the valuse of the jacobian matrix \n \
        		with length nnzj \n \
        eval_h calculates the hessian matrix, it's optional. \n \
        	if omitted, please set nnzh to 0 and ipopt will use approximated hessian \n \
        	which will make the convergence slower. ";
        	
static PyObject *create(PyObject *obj, PyObject *args)
{
	PyObject *f; 
	PyObject *gradf;
	PyObject *g;
	PyObject *jacg;
	PyObject *h = NULL;
	PyObject *applynew = NULL;
	//int n;			// Number of var
	PyArrayObject *xL;
	PyArrayObject *xU;
	//int m;			// Number of con
	PyArrayObject *gL;
	PyArrayObject *gU;
	
	int nele_jac;
	int nele_hess;
	
	double* xldata, *xudata;
	double* gldata, *gudata;
	
    double result;
    int i;
    // "O!", &PyArray_Type &a_x 
    if (!PyArg_ParseTuple(args, "iO!O!iO!O!iiOOOO|OO", 
    	&n, &PyArray_Type, &xL, 
    		&PyArray_Type, &xU, 
    		&m, 
    		&PyArray_Type, &gL,
    		&PyArray_Type, &gU,
    		&nele_jac, &nele_hess,
    		&f, &gradf, &g, &jacg, 
    		&h, &applynew)) 
        return Py_False;
        
        
    if (!PyCallable_Check(f) 		||
    	!PyCallable_Check(gradf) 	|| 
    	!PyCallable_Check(g)		||
    	!PyCallable_Check(jacg))
        	PyErr_SetString(PyExc_TypeError, 
        		"Need a callable object for function!");
    else {
		eval_f_python 		= f;
		eval_grad_f_python 	= gradf;
		eval_g_python 		= g;
		eval_jac_g_python 	= jacg;
		
		if (h !=NULL )
		{
			if (!PyCallable_Check(h))	
				PyErr_SetString(PyExc_TypeError, 
        		 "Need a callable object for function h!");
        	else if (!PyCallable_Check(applynew))
        		PyErr_SetString(PyExc_TypeError, 
        		 "Need a callable object for function applynew!");
        	else
				eval_h_python	= h;
				apply_new_python = applynew;
		}
		else 
		{
			printf("Ipopt will use Hessian approximation!\n");
		}
		
		Number* x_L = NULL;                  /* lower bounds on x */
  		Number* x_U = NULL;                  /* upper bounds on x */
  		Number* g_L = NULL;                  /* lower bounds on g */
  		Number* g_U = NULL;                  /* upper bounds on g */
		
		if (m <=0 || n<=0 )
			return Py_False;
		
		x_L = (Number*)malloc(sizeof(Number)*n);
  		x_U = (Number*)malloc(sizeof(Number)*n);
  		if (!x_L || !x_U) PyErr_Print();
  			
		xldata = (double*)xL->data;
		xudata = (double*)xU->data;
		for (i = 0; i< n; i++) {
			x_L[i] = xldata[i];
			x_U[i] = xudata[i];
		}
		 
  		g_L = (Number*)malloc(sizeof(Number)*m);
  		g_U = (Number*)malloc(sizeof(Number)*m);		
  		if (!g_L || !g_U) PyErr_Print();
		
		gldata = (double*)gL->data;
		gudata = (double*)gU->data;
		
		for (i = 0; i< m; i++)
		{
			g_L[i] = gldata[i];
			g_U[i] = gudata[i];
		}

	  	/* create the IpoptProblem */
	  	
		nlp = CreateIpoptProblem(n, x_L, x_U, m, g_L, g_U, nele_jac, 0, 
				0,  &eval_f, &eval_g, &eval_grad_f,  &eval_jac_g, &eval_h);
		printf("Problem created\n");
		
  		free(x_L);
  		free(x_U);
  		free(g_L);
  		free(g_U);
		return Py_True;
	} // end if
	return Py_False;
}

static char PYIPOPT_SOLVE_DOC[] = "solve(x) -> (x, ml, mu, obj)\n \
        \n \
        Call ipopt to solve problem created before and return  \n \
        a tuple that contains final solution x, upper and lower\n \
        bound for mulitplier and final objective funtion obj. ";

static PyObject *solve(PyObject *self, PyObject *args)
{
    enum ApplicationReturnStatus status; /* Solve return code */
    // int m, n, i;
    
    int i;
    
  	// Number* x = NULL;                    /* starting point and solution vector */
	Number* mult_x_L = NULL;
  	Number* mult_x_U = NULL; 
  	/* Return values */
  	
  	int dX[1];
  	int dL[1];
  	
  	PyArrayObject *x, *mL, *mU;
  	Number obj;                          /* objective value */
  	
  	PyObject* result = Py_False;  	
	PyArrayObject *x0;
    
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &x0)) 
        return Py_False;
        
	if (nlp == NULL)
		return Py_False;
 	
 	/* set some options */
  	AddIpoptNumOption(nlp, "tol", 1e-9);
  	AddIpoptStrOption(nlp, "mu_strategy", "adaptive");
  	if (eval_h_python == NULL)
  		AddIpoptStrOption(nlp, "hessian_approximation","limited-memory");

  	/* allocate space for the initial point and set the values */
	dX[0]  = n;
	printf("n is %d, m is %d\n", n, m);
	
	x = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );
	
	Number* newx0 = (Number*)malloc(sizeof(Number)*n);
	double* xdata = (double*) x0->data;
	for (i =0; i< n; i++)
		newx0[i] = xdata[i];
	
  	mL = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );
	mU = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );

  	status = IpoptSolve(nlp, newx0, NULL, &obj, NULL, (double*)mL->data, (double*)mU->data, NULL);
 
  	if (status == Solve_Succeeded) {
  		printf("Problem solved\n");
		double* xdata = (double*) x->data;
		for (i =0; i< n; i++)
			xdata[i] = newx0[i];
		FreeIpoptProblem(nlp);
		return Py_BuildValue( "OOOd",
                              PyArray_Return( x ),
                              PyArray_Return( mL ),
                              PyArray_Return( mU ), obj);
  	}
  	else {
  		FreeIpoptProblem(nlp);
  		return Py_False;
	}
}

/* Begin Python Module code section */
static PyMethodDef ipoptMethods[] = {
    { "solve", solve, METH_VARARGS, PYIPOPT_SOLVE_DOC},
    { "create", create, METH_VARARGS, PYIPOPT_CREATE_DOC},
    { NULL, NULL }
};


PyMODINIT_FUNC
initpyipopt(void)
{
	   Py_InitModule("pyipopt", ipoptMethods);
	   import_array( );         /* Initialize the Numarray module. */
		/* A segfalut will occur if I use numarray without this.. 
	    /* Check for errors */
	    if (PyErr_Occurred())	
	 	   Py_FatalError("Unable to initialize module pyipopt");
	 	   
    	return;
}
/* End Python Module code section */

