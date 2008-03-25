/* whatever licence, free
	Eric You Xu, WUSTL
	
	RoT: 	 1, start from a relative small core, make sure it works 
			 2, check the objective warp twice
			 		wrap a tuple of callable object  
			 		use "O" instead of 'o', 
			 		warp it with (), etc 
			 3, Exam the python function before hook back
			 4, if that's a single vaule, use pyarg_parse
				 
	TODO:
			0. Deference use valgrind: DONE
			1. Use this code with AMPL, connect nlpy and pyipopt tomorrow. DONE
					2nd dev requires some knowledge
			2. Read openopt code and hook them 
				
	LESSIMPORTANT:
			handle the reference count, is there any memory leak??? 
*/

#include "Python.h"
#include "IpStdCInterface.h"
#include <stdio.h>
#include "hook.h"

static PyObject *eval_f_python 		= NULL;
static PyObject *eval_grad_f_python = NULL;
static PyObject *eval_g_python 		= NULL;
static PyObject *eval_jac_g_python 	= NULL;
static PyObject *eval_h_python		= NULL;

static IpoptProblem nlp = NULL;             /* IpoptProblem */

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
	PyObject *newx = PyList_New(n);
	if (!newx) return FALSE;
	int i;
	for (i=0; i<n; i++)
		PyList_SetItem(newx, i, PyFloat_FromDouble(x[i]));
	
	PyObject* arglist = Py_BuildValue("(O)", newx);
	PyObject* result  = PyObject_CallObject (eval_f_python ,arglist);

	if (!PyFloat_Check(result))
		PyErr_Print();
	
	*obj_value =  PyFloat_AsDouble(result);
	Py_DECREF(result);
  	Py_CLEAR(newx);
	Py_CLEAR(arglist);
  	return TRUE;
}

static Bool eval_grad_f(Index n, Number* x, Bool new_x,
                 Number* grad_f, UserDataPtr user_data)
{
	if (eval_grad_f_python == NULL) PyErr_Print();
	
	PyObject *newx = PyList_New(n);
	int i;
	for (i=0; i<n; i++) 
		PyList_SetItem(newx, i, PyFloat_FromDouble(x[i]));
	
	PyObject* arglist = Py_BuildValue("(O)", newx);
	PyObject* result = PyObject_CallObject (eval_grad_f_python, arglist);
	
	if (!PyList_Check(result))
		PyErr_Print();
	
	for (i = 0; i < n; i++)
		grad_f[i] = PyFloat_AsDouble(PyList_GetItem(result, i));
	
	Py_DECREF(result);
	
  	Py_CLEAR(newx);
	Py_CLEAR(arglist);	
	return TRUE;
}


Bool eval_g(Index n, Number* x, Bool new_x,
            Index m, Number* g, UserDataPtr user_data)
{
	if (eval_g_python == NULL) PyErr_Print();
	PyObject *newx = PyList_New(n);
	
	int i;
	for (i=0; i<n; i++) 
		PyList_SetItem(newx, i, PyFloat_FromDouble(x[i]));
	
	PyObject* arglist = Py_BuildValue("(O)", newx);
	PyObject* result = PyObject_CallObject (eval_g_python, arglist);
	
	if (!PyList_Check(result))
		PyErr_Print();
		
	for (i = 0; i < m; i++) 
		g[i] = PyFloat_AsDouble(PyList_GetItem(result, i));
	
	Py_DECREF(result);
	
  	Py_CLEAR(newx);
	Py_CLEAR(arglist);
	return TRUE;
}


Bool eval_jac_g(Index n, Number *x, Bool new_x,
                Index m, Index nele_jac,
                Index *iRow, Index *jCol, Number *values,
                UserDataPtr user_data)
{
	int i;
	if (eval_grad_f_python == NULL) PyErr_Print();
	
	if (values == NULL) {		
		PyObject *newx = Py_True;
		
		PyObject* arglist = Py_BuildValue("(OO)", newx, Py_True);	
		PyObject* result = PyObject_CallObject (eval_jac_g_python, arglist);
		
		if (!PyTuple_Check(result))
			PyErr_Print();
			
		PyObject* row = PyTuple_GetItem(result, 0);
		PyObject* col = PyTuple_GetItem(result, 1);

		if (!PyList_Check(row) || !PyList_Check(col))
			PyErr_Print();

		for (i = 0; i < nele_jac; i++) {
			PyArg_Parse(PyList_GetItem(row, i), "i", &iRow[i]);
			PyArg_Parse(PyList_GetItem(col, i), "i", &jCol[i]);
		}

		Py_DECREF(result);
		Py_CLEAR(arglist);	
	
	}
	else {	// Assign the jac_g
		
		PyObject *newx = PyList_New(n);
		for (i=0; i<n; i++) 
			PyList_SetItem(newx, i, PyFloat_FromDouble(x[i]));
		
		PyObject* arglist = Py_BuildValue("(OO)", newx, Py_False);
		
		PyObject* result = PyObject_CallObject (eval_jac_g_python, arglist);
		if (!PyTuple_Check(result))
			PyErr_Print();
		
		for (i = 0; i < nele_jac; i++) 
			values[i] = PyFloat_AsDouble(PyList_GetItem(result, i));
		
		Py_DECREF(result);
		Py_CLEAR(newx);
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
	
	if (values == NULL) {
		PyObject *newx = Py_True;
		PyObject *objfactor = Py_BuildValue("d", obj_factor);
		PyObject *lagrange = Py_True;	// Booleans don't need the ref count
		
		PyObject* arglist = Py_BuildValue("(OOOO)", newx, lagrange, objfactor, Py_True);
		PyObject* result = PyObject_CallObject (eval_h_python, arglist);
		if (!PyTuple_Check(result))
			PyErr_Print();
			
		PyObject* row = PyTuple_GetItem(result, 0);	//steal
		PyObject* col = PyTuple_GetItem(result, 1); //steal

		if (!PyList_Check(row))
			PyErr_Print();
		if (!PyList_Check(col))
			PyErr_Print();

		for (i = 0; i < nele_hess; i++) {
			PyArg_Parse(PyList_GetItem(row, i), "i", &iRow[i]);
			PyArg_Parse(PyList_GetItem(col, i), "i", &jCol[i]);
		}
		// newx and other's are just transparent pointer to Py_True
		Py_DECREF(objfactor);
		Py_DECREF(result);
		Py_CLEAR(arglist);
	}
	else {	// Assign the hess
		PyObject *newx = PyList_New(n);
		PyObject *lagrange = PyList_New(m);
		PyObject *objfactor = Py_BuildValue("d", obj_factor);
		
		for (i=0; i<n; i++) {
			PyList_SetItem(newx, i, PyFloat_FromDouble(x[i]));
		}
		
		for (i=0; i<m; i++) {
			PyList_SetItem(lagrange, i, PyFloat_FromDouble(lambda[i]));
		}
		
		PyObject* arglist = Py_BuildValue("(OOOO)", newx, objfactor, lagrange, Py_False);
		
		PyObject* result = PyObject_CallObject (eval_h_python, arglist);
		
		// if (!result) printf("Null pointer\n");
		
		for (i = 0; i < nele_hess; i++) {
			values[i] = PyFloat_AsDouble(PyList_GetItem(result, i));
			// printf("[DEBUG] values[%d] = %f", i, values[i]);
		}
		Py_CLEAR(newx);
		Py_CLEAR(lagrange);
		Py_CLEAR(objfactor);
		// Py_DECREF(result);
		Py_CLEAR(arglist);
	}	
  	return TRUE;
}
/*  Ends Call back function section */

/* Interface to Python */
// Crate problem
static PyObject *create(PyObject *obj, PyObject *args)
{
	PyObject *f;
	PyObject *gradf;
	PyObject *g;
	PyObject *jacg;
	PyObject *h = NULL;
	int n;			// Number of var
	PyObject *xL;
	PyObject *xU;
	int m;			// Number of con
	PyObject *gL;
	PyObject *gU;
	int nele_jac;
	int nele_hess;
	
	
    double result;
    int i;
    
    if (!PyArg_ParseTuple(args, "iOOiOOiiOOOO|O", 
    	&n, &xL, &xU, 
    	&m, &gL, &gU,
    	&nele_jac, &nele_hess,
    	&f, &gradf, &g, &jacg, &h)) 
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
        		"Need a callable object for function!");
        	else 
				eval_h_python	= h;
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
  		/* set the values for the variable bounds */
		for (i = 0; i< n; i++)
		{
			x_L[i] = PyFloat_AsDouble(PyList_GetItem(xL, i));
			x_U[i] = PyFloat_AsDouble(PyList_GetItem(xU, i));
		}
		 
  		g_L = (Number*)malloc(sizeof(Number)*m);
  		g_U = (Number*)malloc(sizeof(Number)*m);
		for (i = 0; i< m; i++)
		{
			g_L[i] = PyFloat_AsDouble(PyList_GetItem(gL, i));
			g_U[i] = PyFloat_AsDouble(PyList_GetItem(gU, i));
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

static PyObject *solve(PyObject *self, PyObject *args)
{
    enum ApplicationReturnStatus status; /* Solve return code */
    int m, n, i;
    
  	Number* x = NULL;                    /* starting point and solution vector */
	Number* mult_x_L = NULL;
  	Number* mult_x_U = NULL; 
  	Number obj;                          /* objective value */
  	
    PyObject* result = Py_False;  	
	PyObject *x0;
    if (!PyArg_ParseTuple(args, "O", &x0)) 
        return Py_False;
        
	if (nlp == NULL)
		return Py_False;
 	
 	/* set some options */
  	AddIpoptNumOption(nlp, "tol", 1e-9);
  	AddIpoptStrOption(nlp, "mu_strategy", "adaptive");
  	if (eval_h_python == NULL)
  		AddIpoptStrOption(nlp, "hessian_approximation","limited-memory");

  	/* allocate space for the initial point and set the values */
  	n = Py_SAFE_DOWNCAST(PyList_Size(x0), Py_ssize_t, int);
	x = (Number*)malloc(sizeof(Number)*n);
	for (i =0; i< n; i++)
		x[i] = PyFloat_AsDouble(PyList_GetItem(x0 , i));
	
  	/* allocate space to store the bound multipliers at the solution */
  	mult_x_L = (Number*)malloc(sizeof(Number)*n);
  	mult_x_U = (Number*)malloc(sizeof(Number)*n);

  	/* solve the problem */
  	printf("I am calling IPOPT\n");
  	status = IpoptSolve(nlp, x, NULL, &obj, NULL, mult_x_L, mult_x_U, NULL);
  
  	if (status == Solve_Succeeded) {
  	  printf("\n\nSolution of the primal variables, x\n");
  	  for (i=0; i<n; i++) {
  	    printf("x[%d] = %e\n", i, x[i]); 
      }

   	  printf("\n\nSolution of the bound multipliers, z_L and z_U\n");
      for (i=0; i<n; i++) {
      	printf("z_L[%d] = %e\n", i, mult_x_L[i]); 
      }
      for (i=0; i<n; i++) {
        printf("z_U[%d] = %e\n", i, mult_x_U[i]); 
      }

      printf("\n\nObjective value\n");
      printf("f(x*) = %e\n", obj);
      result = Py_True;  
  }
  /* free allocated memory */
  FreeIpoptProblem(nlp);
  free(x);
  free(mult_x_L);
  free(mult_x_U);
  return result; 
}



/* Begin Python Module code section */
static PyMethodDef ipoptMethods[] = {
    { "solve", solve, METH_VARARGS},
    { "create", create, METH_VARARGS},
    { NULL, NULL }
};

PyMODINIT_FUNC
initpyipopt(void)
{
   Py_InitModule("pyipopt", ipoptMethods);
}

/* End Python Module code section */
