//  Author: Eric Xu
//  Licensed under BSD

// Modifications made by 
// OpenMDAO at NASA Glenn Research Center, 2010 and 2011

#include "hook.h"

int user_log_level = TERSE;

/* Object Section */
/* sig of this is void foo(PyO*) */
static void problem_dealloc(PyObject * self)
{
	problem *temp = (problem *) self;
	free(temp->data);
}

PyObject *solve(PyObject * self, PyObject * args);
PyObject *set_intermediate_callback(PyObject * self, PyObject * args);
PyObject *close_model(PyObject * self, PyObject * args);

static char PYIPOPT_SOLVE_DOC[] = "solve(x) -> (x, ml, mu, obj)\n \
  \n                                                        \
  Call Ipopt to solve problem created before and return  \n \
  a tuple that contains final solution x, upper and lower\n \
  bound for multiplier, final objective function obj, \n \
  and the return status of ipopt. \n";

static char PYIPOPT_SET_INTERMEDIATE_CALLBACK_DOC[] =
    "set_intermediate_callback(callback_function)\n \
  \n                                              \
  Set the intermediate callback function.         \
  This gets called each iteration.";

static char PYIPOPT_CLOSE_DOC[] = "After all the solving, close the model\n";

static char PYIPOPT_ADD_STR_OPTION_DOC[] =
    "Set the String option for Ipopt. See the document for Ipopt for more information.\n";

static PyObject *add_str_option(PyObject * self, PyObject * args)
{
	problem *temp = (problem *) self;
	IpoptProblem nlp = (IpoptProblem) (temp->nlp);
	char *param;
	char *value;
	Bool ret;

	if (!PyArg_ParseTuple(args, "ss:str_option", &param, &value)) {
		return NULL;
	}
	ret = AddIpoptStrOption(nlp, (char *)param, value);
	if (ret) {
		Py_INCREF(Py_True);
		return Py_True;
	} else {
		return PyErr_Format(PyExc_ValueError,
				    "%s is not a valid string option", param);
	}
}

static char PYIPOPT_ADD_INT_OPTION_DOC[] =
    "Set the Int option for Ipopt. See the document for Ipopt for more information.\n";

static PyObject *add_int_option(PyObject * self, PyObject * args)
{

	problem *temp = (problem *) self;
	IpoptProblem nlp = (IpoptProblem) (temp->nlp);

	char *param;
	int value;

	Bool ret;

	if (!PyArg_ParseTuple(args, "si:int_option", &param, &value)) {
		return NULL;
	}
	ret = AddIpoptIntOption(nlp, (char *)param, value);
	if (ret) {
		Py_INCREF(Py_True);
		return Py_True;
	} else {
		return PyErr_Format(PyExc_ValueError,
				    "%s is not a valid int option", param);
	}
}

static char PYIPOPT_ADD_NUM_OPTION_DOC[] =
    "Set the Number/double option for Ipopt. See the document for Ipopt for more information.\n";

static PyObject *add_num_option(PyObject * self, PyObject * args)
{
	problem *temp = (problem *) self;
	IpoptProblem nlp = (IpoptProblem) (temp->nlp);

	char *param;
	double value;

	Bool ret;

	if (!PyArg_ParseTuple(args, "sd:num_option", &param, &value)) {
		return NULL;
	}
	ret = AddIpoptNumOption(nlp, (char *)param, value);
	if (ret) {
		Py_INCREF(Py_True);
		return Py_True;
	} else {
		return PyErr_Format(PyExc_ValueError,
				    "%s is not a valid num option", param);
	}
}

PyMethodDef problem_methods[] = {
	{"solve", solve, METH_VARARGS, PYIPOPT_SOLVE_DOC}
	,
	{"set_intermediate_callback", set_intermediate_callback, METH_VARARGS,
	 PYIPOPT_SET_INTERMEDIATE_CALLBACK_DOC}
	,
	{"close", close_model, METH_VARARGS, PYIPOPT_CLOSE_DOC}
	,
	{"int_option", add_int_option, METH_VARARGS, PYIPOPT_ADD_INT_OPTION_DOC}
	,
	{"str_option", add_str_option, METH_VARARGS, PYIPOPT_ADD_STR_OPTION_DOC}
	,
	{"num_option", add_num_option, METH_VARARGS, PYIPOPT_ADD_NUM_OPTION_DOC}
	,
	{NULL, NULL}
	,
};

PyObject *problem_getattr(PyObject * self, char *attrname)
{
	PyObject *result = NULL;
	result = Py_FindMethod(problem_methods, self, attrname);
	return result;
}

/*
 * had to replace PyObject_HEAD_INIT(&PyType_Type) in order to get this to
 * compile on Windows
 */
PyTypeObject IpoptProblemType = {
	PyObject_HEAD_INIT(NULL)
	    0,			/* ob_size */
	"pyipopt.Problem",	/* tp_name */
	sizeof(problem),	/* tp_basicsize */
	0,			/* tp_itemsize */
	problem_dealloc,	/* tp_dealloc */
	0,			/* tp_print */
	problem_getattr,	/* tp_getattr */
	0,			/* tp_setattr */
	0,			/* tp_compare */
	0,			/* tp_repr */
	0,			/* tp_as_number */
	0,			/* tp_as_sequence */
	0,			/* tp_as_mapping */
	0,			/* tp_hash */
	0,			/* tp_call */
	0,			/* tp_str */
	0,			/* tp_getattro */
	0,			/* tp_setattro */
	0,			/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,	/* tp_flags */
	"The IPOPT problem object in python",	/* tp_doc */
};

static char PYIPOPT_CREATE_DOC[] =
    "create(n, xl, xu, m, gl, gu, nnzj, nnzh, eval_f, eval_grad_f, eval_g, eval_jac_g) -> Boolean\n \
       \n \
       Create a problem instance and return True if succeed  \n \
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
       nnzh is the number of non-zeros in Hessian matrix, you can set it to 0 \n \
       \n \
       eval_f is the call back function to calculate objective value, \n \
               it takes one single argument x as input vector \n \
       eval_grad_f calculates gradient for objective function \n \
       eval_g calculates the constraint values and return an array \n \
       eval_jac_g calculates the Jacobi matrix. It takes two arguments, \n \
               the first is the variable x and the second is a Boolean flag \n \
               if the flag is true, it supposed to return a tuple (row, col) \n \
                       to indicate the sparse Jacobi matrix's structure. \n \
               if the flag is false if returns the values of the Jacobi matrix \n \
                       with length nnzj \n \
       eval_h calculates the hessian matrix, it's optional. \n \
               if omitted, please set nnzh to 0 and Ipopt will use approximated hessian \n \
               which will make the convergence slower. ";

static char PYIPOPT_LOG_DOC[] = "set_loglevel(level)\n \
	\n \
	Set the log level of PyIPOPT \n \
	levels: \n \
		0:	Terse, 	  no log from pyipopt \n \
		1:	Moderate, logs for ipopt \n \
		2:	Verbose,  logs for both ipopt and pyipopt. \n";

static PyObject *set_loglevel(PyObject * obj, PyObject * args)
{
	int l;
	if (!PyArg_ParseTuple(args, "i", &l)) {
		printf("l is %d \n", l);
		return NULL;
	}
	if (l < 0 || l > 2) {
		return NULL;
	}
	user_log_level = l;
	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *create(PyObject * obj, PyObject * args)
{
	PyObject *f = NULL;
	PyObject *gradf = NULL;
	PyObject *g = NULL;
	PyObject *jacg = NULL;
	PyObject *h = NULL;
	PyObject *applynew = NULL;

	DispatchData myowndata;

	/*
	 * I have to create a new python object here, return this python
	 * object
	 */

	int n;			/* Number of var */
	PyArrayObject *xL = NULL;
	PyArrayObject *xU = NULL;
	int m;			/* Number of con */
	PyArrayObject *gL = NULL;
	PyArrayObject *gU = NULL;

	problem *object = NULL;

	int nele_jac;
	int nele_hess;

	Number *x_L = NULL;	/* lower bounds on x */
	Number *x_U = NULL;	/* upper bounds on x */
	Number *g_L = NULL;	/* lower bounds on g */
	Number *g_U = NULL;	/* upper bounds on g */

	double *xldata, *xudata;
	double *gldata, *gudata;

	int i;

	DispatchData *dp = NULL;

	PyObject *retval = NULL;

	/* Init the myowndata field */
	myowndata.eval_f_python = NULL;
	myowndata.eval_grad_f_python = NULL;
	myowndata.eval_g_python = NULL;
	myowndata.eval_jac_g_python = NULL;
	myowndata.eval_h_python = NULL;
	myowndata.apply_new_python = NULL;
	myowndata.userdata = NULL;

	/* "O!", &PyArray_Type &a_x  */
	if (!PyArg_ParseTuple(args, "iO!O!iO!O!iiOOOO|OO:pyipoptcreate",
			      &n, &PyArray_Type, &xL,
			      &PyArray_Type, &xU,
			      &m,
			      &PyArray_Type, &gL,
			      &PyArray_Type, &gU,
			      &nele_jac, &nele_hess,
			      &f, &gradf, &g, &jacg, &h, &applynew)) {
		retval = NULL;
		goto done;
	}
	if (!PyCallable_Check(f) ||
	    !PyCallable_Check(gradf) ||
	    !PyCallable_Check(g) || !PyCallable_Check(jacg)) {
		PyErr_SetString(PyExc_TypeError,
				"Need a callable object for callback functions");
		retval = NULL;
		goto done;
	}
	myowndata.eval_f_python = f;
	myowndata.eval_grad_f_python = gradf;
	myowndata.eval_g_python = g;
	myowndata.eval_jac_g_python = jacg;

	if (h != NULL) {
		if (PyCallable_Check(h)) {
			myowndata.eval_h_python = h;
		} else {
			PyErr_SetString(PyExc_TypeError,
					"Need a callable object for function h.");
			retval = NULL;
			goto done;
		}
	} else {
		logger("[PyIPOPT] Ipopt will use Hessian approximation.\n");
	}

	if (applynew != NULL) {
		if (PyCallable_Check(applynew)) {
			myowndata.apply_new_python = applynew;
		} else {
			PyErr_SetString(PyExc_TypeError,
					"Need a callable object for function applynew.");
			retval = NULL;
			goto done;
		}
	}
	if (m < 0 || n < 0) {
		PyErr_SetString(PyExc_TypeError, "m or n can't be negative");
		retval = NULL;
		goto done;
	}
	x_L = (Number *) malloc(sizeof(Number) * n);
	x_U = (Number *) malloc(sizeof(Number) * n);
	if (!x_L || !x_U) {
		retval = PyErr_NoMemory();
		goto done;
	}
	xldata = (double *)xL->data;
	xudata = (double *)xU->data;
	for (i = 0; i < n; i++) {
		x_L[i] = xldata[i];
		x_U[i] = xudata[i];
	}

	g_L = (Number *) malloc(sizeof(Number) * m);
	g_U = (Number *) malloc(sizeof(Number) * m);
	if (!g_L || !g_U)
		PyErr_NoMemory();

	gldata = (double *)gL->data;
	gudata = (double *)gU->data;

	for (i = 0; i < m; i++) {
		g_L[i] = gldata[i];
		g_U[i] = gudata[i];
	}

	/* create the Ipopt Problem */

	int C_indexstyle = 0;
	IpoptProblem thisnlp = CreateIpoptProblem(n,
						  x_L, x_U, m, g_L, g_U,
						  nele_jac, nele_hess,
						  C_indexstyle,
						  &eval_f, &eval_g,
						  &eval_grad_f,
						  &eval_jac_g, &eval_h);
	logger("[PyIPOPT] Problem created");
	if (!thisnlp) {
		PyErr_SetString(PyExc_MemoryError,
				"Cannot create IpoptProblem instance");
		retval = NULL;
		goto done;
	}
	object = PyObject_NEW(problem, &IpoptProblemType);

	if (object != NULL) {
		object->nlp = thisnlp;
		dp = malloc(sizeof(DispatchData));
		if (!dp) {
			retval = PyErr_NoMemory();
			goto done;
		}
		memcpy((void *)dp, (void *)&myowndata, sizeof(DispatchData));
		object->data = dp;
		retval = (PyObject *) object;
		goto done;
	} else {
		PyErr_SetString(PyExc_MemoryError,
				"Can't create a new Problem instance");
		retval = NULL;
		goto done;
	}

 done:
	/* Clean up and return */
	free(x_L);
	free(x_U);
	free(g_L);
	free(g_U);

	return retval;
}

PyObject *set_intermediate_callback(PyObject * self, PyObject * args)
{
	PyObject *intermediate_callback;
	problem *temp = (problem *) self;
	IpoptProblem nlp = (IpoptProblem) (temp->nlp);
	DispatchData myowndata;
	DispatchData *bigfield = (DispatchData *) (temp->data);

	/* Init the myowndata field */
	myowndata.eval_intermediate_callback_python = NULL;

	if (!PyArg_ParseTuple(args, "O", &intermediate_callback)) {
		return NULL;
	}
	if (!PyCallable_Check(intermediate_callback)) {
		PyErr_SetString(PyExc_TypeError,
				"Need a callable object for function!");
		return NULL;
	} else {

		bigfield->eval_intermediate_callback_python =
		    intermediate_callback;

		/* Put a Python function object into this data structure */
		/*
		 * myowndata.eval_intermediate_callback_python =
		 * intermediate_callback;
		 */

		/* DispatchData *dp = malloc(sizeof(DispatchData)); */
		/*
		 * memcpy((void*)dp, (void*)&myowndata,
		 * sizeof(DispatchData));
		 */
		/* bigfield = dp; */
		/*
		 * logger( "qqq: inside set_intermediate_callback, bigfield
		 * is %p\n", bigfield ) ;
		 */
		/*
		 * logger("[PyIPOPT] User specified data field to callback
		 * function.\n");
		 */

		SetIntermediateCallback(nlp, eval_intermediate_callback);
		Py_INCREF(Py_True);
		return Py_True;
	}
}

PyObject *solve(PyObject * self, PyObject * args)
{
	enum ApplicationReturnStatus status;	/* Solve return code */
	int i;
	int n;

	/* Return values */
	problem *temp = (problem *) self;

	IpoptProblem nlp = (IpoptProblem) (temp->nlp);
	DispatchData *bigfield = (DispatchData *) (temp->data);

	/* int dX[1]; */
	npy_intp dX[1];

	PyArrayObject *x = NULL, *mL = NULL, *mU = NULL;
	Number obj;		/* objective value */

	PyObject *retval = NULL;
	PyArrayObject *x0 = NULL;

	PyObject *myuserdata = NULL;

	Number *newx0 = NULL;

	if (!PyArg_ParseTuple(args, "O!|O", &PyArray_Type, &x0, &myuserdata)) {
		retval = NULL;
		goto done;
	}
	if (myuserdata != NULL) {
		bigfield->userdata = myuserdata;
		/*
		 * logger("[PyIPOPT] User specified data field to callback
		 * function.\n");
		 */
	}
	if (nlp == NULL) {
		PyErr_SetString(PyExc_TypeError,
				"nlp objective passed to solve is NULL\n Problem created?\n");
		retval = NULL;
		goto done;
	}
	if (bigfield->eval_h_python == NULL) {
		AddIpoptStrOption(nlp, "hessian_approximation",
				  "limited-memory");
		/* logger("Can't find eval_h callback function\n"); */
	}
	/* allocate space for the initial point and set the values */
	npy_intp *dim = ((PyArrayObject *) x0)->dimensions;
	n = dim[0];
	dX[0] = n;

	x = (PyArrayObject *) PyArray_SimpleNew(1, dX, PyArray_DOUBLE);
	if (!x) {
		retval = PyErr_NoMemory();
		goto done;
	}
	newx0 = (Number *) malloc(sizeof(Number) * n);
	if (!newx0) {
		retval = PyErr_NoMemory();
		goto done;
	}
	double *xdata = (double *)x0->data;
	for (i = 0; i < n; i++)
		newx0[i] = xdata[i];

	mL = (PyArrayObject *) PyArray_SimpleNew(1, dX, PyArray_DOUBLE);
	mU = (PyArrayObject *) PyArray_SimpleNew(1, dX, PyArray_DOUBLE);

	/* For status code, see IpReturnCodes_inc.h in Ipopt */

	status =
	    IpoptSolve(nlp, newx0, NULL, &obj, NULL, (double *)mL->data,
		       (double *)mU->data, (UserDataPtr) bigfield);
	double *return_x_data = (double *)x->data;
	for (i = 0; i < n; i++) {
		return_x_data[i] = newx0[i];
	}
	retval = Py_BuildValue("OOOdO",
			       PyArray_Return(x),
			       PyArray_Return(mL),
			       PyArray_Return(mU),
			       obj, Py_BuildValue("i", status)
	    );
	goto done;

 done:
	/* clean up and return */
	if (retval == NULL) {
		Py_XDECREF(x);
		Py_XDECREF(mL);
		Py_XDECREF(mU);
	}
	free(newx0);

	return retval;
}

PyObject *close_model(PyObject * self, PyObject * args)
{
	problem *obj = (problem *) self;
	FreeIpoptProblem(obj->nlp);
	obj->nlp = NULL;
	Py_INCREF(Py_True);
	return Py_True;
}

/* static char PYTEST[] = "TestCreate\n"; */

/* static PyObject *test(PyObject *self, PyObject *args) */
/* { */
/* IpoptProblem thisnlp = NULL; */
/* problem *object = NULL; */
/* object = PyObject_NEW(problem , &IpoptProblemType); */
/* if (object != NULL) */
/* object->nlp = thisnlp; */
/* /\*        problem *object = problem_new(thisnlp); *\/ */
/* return (PyObject *)object; */
/* } */

/* Begin Python Module code section */
static PyMethodDef ipoptMethods[] = {
	/* { "solve", solve, METH_VARARGS, PYIPOPT_SOLVE_DOC}, */
	{"create", create, METH_VARARGS, PYIPOPT_CREATE_DOC},
	/* { "close",  close_model, METH_VARARGS, PYIPOPT_CLOSE_DOC},  */
	/* { "test",   test,                 METH_VARARGS, PYTEST}, */
	{"set_loglevel", set_loglevel, METH_VARARGS, PYIPOPT_LOG_DOC},
	{NULL, NULL}
};

PyMODINIT_FUNC initpyipopt(void)
{

	Py_InitModule3("pyipopt", ipoptMethods,
		       "A hook between Ipopt and Python");

	import_array();		/* Initialize the Numarray module. */
	/* A segfault will occur if I use numarray without this.. */

	if (PyErr_Occurred())
		Py_FatalError("Unable to initialize module pyipopt");

	return;
}

/* End Python Module code section */
