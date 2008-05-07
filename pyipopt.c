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
* DISCLAIMEmyowndata. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* Version 0.2 All numpy types */
/* Version 0.3 Change the module to OO 
	now use nlp = pyipopt.create(xxx)
		and nlp.solve
			nlp.close()
			
Worklog
	you can create multiple instance of nlp. [Tested]
2. To allocate more models in a hooker. Planning to move all the pointers to the 
	PyObject (callback function in Python) to the user_data field
	Therefore, the C callback function here can just dispatch it to the python
	callable object in the user_data [DONE]
	[We wrap the user_data twice]
3. Change pyipopt.solve to nlp.solve
	Construct a new object 	
*/


#include "hook.h"

/* Object Section */
void problem_dealloc(problem* self)
{
	free(self->data);
	return;
}

PyObject* solve (PyObject* self, PyObject* args);
PyObject* close_model (PyObject* self, PyObject* args);

static char PYIPOPT_SOLVE_DOC[] = "solve(x) -> (x, ml, mu, obj)\n \
        \n \
        Call ipopt to solve problem created before and return  \n \
        a tuple that contains final solution x, upper and lower\n \
        bound for mulitplier and final objective funtion obj. ";

static char PYIPOPT_CLOSE_DOC[] = "After all the solving, close the model\n";

static char PYIPOPT_ADD_STR_OPTION_DOC[] = "Set the String option for ipopt. See the document for Ipopt for more information.\n";


PyObject *add_str_option(PyObject *self, PyObject *args)
{
  	problem* temp = (problem*)self; 	
  	IpoptProblem nlp = (IpoptProblem)(temp->nlp);
  	
  	char* param;
  	char* value;
  	
  	Bool ret;
  	
  	if (!PyArg_ParseTuple(args, "ss", &param, &value)) 
        return Py_False;
    
  	ret = AddIpoptStrOption(nlp, (char*) param, value);
	if (ret) 
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else 
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}


static char PYIPOPT_ADD_INT_OPTION_DOC[] = "Set the Int option for ipopt. See the document for Ipopt for more information.\n";

PyObject *add_int_option(PyObject *self, PyObject *args)
{
  	problem* temp = (problem*)self; 	
  	IpoptProblem nlp = (IpoptProblem)(temp->nlp);
  	
  	char* param;
  	int value;
  	
  	Bool ret;
  	
  	if (!PyArg_ParseTuple(args, "si", &param, &value))
        return Py_False;
    
  	ret = AddIpoptIntOption(nlp, (char*) param, value);
	if (ret) 
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else 
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}


static char PYIPOPT_ADD_NUM_OPTION_DOC[] = "Set the Number/double option for ipopt. See the document for Ipopt for more information.\n";

PyObject *add_num_option(PyObject *self, PyObject *args)
{
  	problem* temp = (problem*)self; 	
  	IpoptProblem nlp = (IpoptProblem)(temp->nlp);
  	
  	char* param;
  	double value;
  	
  	Bool ret;
  	
  	if (!PyArg_ParseTuple(args, "sd", &param, &value))
        return Py_False;
     
  	ret = AddIpoptIntOption(nlp, (char*) param, value);
	if (ret) 
	{
		Py_INCREF(Py_True);
		return Py_True;
	}
	else 
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
}



PyMethodDef problem_methods[] = {
	{ "solve", 	solve, METH_VARARGS, PYIPOPT_SOLVE_DOC},
	{ "close",  close_model, METH_VARARGS, PYIPOPT_CLOSE_DOC}, 
	{ "int_option", add_int_option, METH_VARARGS, PYIPOPT_ADD_INT_OPTION_DOC},
	{ "str_option", add_str_option, METH_VARARGS, PYIPOPT_ADD_STR_OPTION_DOC},
	{ "num_option", add_num_option, METH_VARARGS, PYIPOPT_ADD_NUM_OPTION_DOC},
	{NULL, NULL},
};

PyObject *problem_getattr(PyObject* self, char* attrname)
{ 
	PyObject *result = NULL;
    result = Py_FindMethod(problem_methods, self, attrname);
    return result;
}

PyTypeObject IpoptProblemType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,                         /*ob_size*/
    "pyipopt.Problem",         /*tp_name*/
    sizeof(problem),    		/*tp_basicsize*/
    0,                         /*tp_itemsize*/
    problem_dealloc,           /*tp_dealloc*/
    0,                         /*tp_print*/
    problem_getattr,           /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "The IPOPT problem object in python", /* tp_doc */
};

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
	
	DispatchData myowndata;
	
	// I have to create a new python object here, return this python object 
	
	int n;			// Number of var
	PyArrayObject *xL;
	PyArrayObject *xU;
	int m;			// Number of con
	PyArrayObject *gL;
	PyArrayObject *gU;
	
	int nele_jac;
	int nele_hess;
	
	double* xldata, *xudata;
	double* gldata, *gudata;
	
    double result;
    int i;
    
    // Init the myowndata field
    myowndata.eval_f_python = NULL;
	myowndata.eval_grad_f_python = NULL; 
	myowndata.eval_g_python = NULL;
	myowndata.eval_jac_g_python = NULL;
	myowndata.eval_h_python = NULL;
	myowndata.apply_new_python = NULL;
	myowndata.userdata = NULL;
    
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
		myowndata.eval_f_python 		= f;
		myowndata.eval_grad_f_python 	= gradf;
		myowndata.eval_g_python 		= g;
		myowndata.eval_jac_g_python 	= jacg;
		 // printf("D field assigned %p\n", &myowndata);
		 // printf("D field assigned %p\n",myowndata.eval_jac_g_python );
		
		if (h !=NULL )
		{
			if (!PyCallable_Check(h))	
				PyErr_SetString(PyExc_TypeError, 
        		 "Need a callable object for function h.");
        	else if (!PyCallable_Check(applynew))
        		PyErr_SetString(PyExc_TypeError, 
        		 "Need a callable object for function applynew.");
        	else
				myowndata.eval_h_python	= h;
				myowndata.apply_new_python = applynew;
		}
		else
		{
			printf("[PyIPOPT] Ipopt will use Hessian approximation.\n");
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
	  	
	  	int C_indexstyle = 0;
	  	printf("[PyIPOPT] nele_hess is %d\n", nele_hess);
		IpoptProblem thisnlp = CreateIpoptProblem(n, x_L, x_U, m, g_L, g_U, nele_jac, nele_hess, C_indexstyle,  &eval_f, &eval_g, &eval_grad_f,  &eval_jac_g, &eval_h);
		logger("[PyIPOPT] Problem created");
		
		problem *object = NULL;
		
		object = PyObject_NEW(problem , &IpoptProblemType);
		
		if (object != NULL)
    	{
    		object->nlp = thisnlp;
    		DispatchData *dp = malloc(sizeof(DispatchData));
			memcpy((void*)dp, (void*)&myowndata, sizeof(DispatchData));
			object->data = dp;
		}
		
		else 
			Py_FatalError("Can't create a new Problem instance");
				
      	free(x_L);
  		free(x_U);
  		free(g_L);
  		free(g_U);
		return (PyObject *)object;
		// return Py_True;
	} // end if
	return Py_False;
}



PyObject *solve(PyObject *self, PyObject *args)
{
    enum ApplicationReturnStatus status; /* Solve return code */
    int i;
    int n;
	
	
  	Number* mult_x_L = NULL;
  	Number* mult_x_U = NULL; 
  	/* Return values */
  	problem* temp = (problem*)self;
  	
  	IpoptProblem nlp = (IpoptProblem)(temp->nlp);
  	DispatchData* bigfield = (DispatchData*)(temp->data);
  	
  	int dX[1];
  	int dL[1];
  	
  	PyArrayObject *x, *mL, *mU;
  	Number obj;                          /* objective value */
  	
  	PyObject* result = Py_False;
	PyArrayObject *x0;
	
	PyObject* myuserdata = NULL;
	
	if (!PyArg_ParseTuple(args, "O!|O", &PyArray_Type, &x0, &myuserdata)) 
    {
		printf("Parameter X0 is expected to be an numpy array type.\n");
		return Py_False;
	}
	
	if (myuserdata != NULL)
	{
		bigfield->userdata = myuserdata;
		printf("[PyIPOPT] User specified data field to callback function.\n");
	}
		
	if (nlp == NULL)
	{
		printf ("nlp objective passed to solve is NULL\n Problem created?\n");
		return Py_False;
	}
 	
	/* set some options */
  	AddIpoptNumOption(nlp, "tol", 1e-8);
  	AddIpoptStrOption(nlp, "mu_strategy", "adaptive");
  	if (bigfield->eval_h_python == NULL)
  	{
  		AddIpoptStrOption(nlp, "hessian_approximation","limited-memory");
		printf("Can't find eval_h callback function\n");
	}
  	/* allocate space for the initial point and set the values */
  	n = (int)((PyArrayObject*)x0->dimensions[0]);
  	// printf("The size of x0 is %d\n", n);
	dX[0]  = n;
	// printf("n is %d, m is %d\n", n, m);
	x = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );
	
	Number* newx0 = (Number*)malloc(sizeof(Number)*n);
	double* xdata = (double*) x0->data;
	for (i =0; i< n; i++)
		newx0[i] = xdata[i];
	
  	mL = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );
	mU = (PyArrayObject *)PyArray_FromDims( 1, dX, PyArray_DOUBLE );
	// printf("Ready to go\n");
			
  	status = IpoptSolve(nlp, newx0, NULL, &obj, NULL, (double*)mL->data, (double*)mU->data, (UserDataPtr)bigfield);
 	// The final parameter is the userdata (void * type)
 
  	if (status == Solve_Succeeded) {
  		printf("Problem solved\n");
		double* xdata = (double*) x->data;
		for (i =0; i< n; i++)
			xdata[i] = newx0[i];
			// FreeIpoptProblem(nlp);
		return Py_BuildValue( "OOOd",
                              PyArray_Return( x ),
                              PyArray_Return( mL ),
                              PyArray_Return( mU ), obj);
  	}
  	else {
  		// FreeIpoptProblem(nlp);
  		return Py_False;
	}
}



        
PyObject *close_model(PyObject *self, PyObject *args)
{
	problem* obj = (problem*) self;
	FreeIpoptProblem(obj->nlp);
	obj->nlp = NULL;
	return Py_True;
}

static char PYTEST[] = "TestCreate\n";

static PyObject *test(PyObject *self, PyObject *args)
{
	IpoptProblem thisnlp = NULL;
  	problem *object = NULL;
	object = PyObject_NEW(problem , &IpoptProblemType);
	if (object != NULL)
    	object->nlp = thisnlp;
// 	problem *object = problem_new(thisnlp);
    return (PyObject *)object;
}

/* Begin Python Module code section */
static PyMethodDef ipoptMethods[] = {
 //    { "solve", solve, METH_VARARGS, PYIPOPT_SOLVE_DOC},
    { "create", create, METH_VARARGS, PYIPOPT_CREATE_DOC},
    // { "close",  close_model, METH_VARARGS, PYIPOPT_CLOSE_DOC}, 
   // { "test",   test, 		METH_VARARGS, PYTEST},
    { NULL, NULL }
};

PyMODINIT_FUNC 
initpyipopt(void)
{

	   PyObject* m = 
	   		Py_InitModule3("pyipopt", ipoptMethods, 
	   			"A hooker between Ipopt and Python");
	   
	   import_array( );         /* Initialize the Numarray module. */
		/* A segfalut will occur if I use numarray without this.. */

	   if (PyErr_Occurred())	
	 	  Py_FatalError("Unable to initialize module pyipopt");
	 	   
    	return;
}
/* End Python Module code section */

