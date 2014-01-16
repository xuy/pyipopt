PyIpopt
=======

PyIpopt is a python module that allows you to use [IPOPT](http://www.coin-or.org/Ipopt/) in Python. It is developed by Eric Xu at Washington Univerisity and issued under the BSD license.

Installation
------------

### Dependencies

PyIpopt depends on the following packages:

1. A compiler and a linker, e.g. gcc, ld
2. [Ipopt](https://projects.coin-or.org/Ipopt)
3. [Numpy](http://numpy.scipy.org/)
4. Python.h (part of the python source code, you can download it from [Python.org](http://python.org))

### Install

First, get the latest source code using:

  $ git clone http://github.com/xuy/pyipopt.git

In your PyIpopt folder, edit setup.py to reflect the configuration of your system, then do

	$ python setup.py build
	$ sudo python setup.py install

### Test

  $ python hs071.py

You should be able to see the result of solving the toy problem.

Usage
-----
You can use PyIpopt like this:

	import pyipopt
	# define your call back functions
	nlp = pyipopt.create(...)
	nlp.solve(...)
	nlp.close()

You can also check out hs071.py to see how to use PyIpopt.

PyIpopt as a module comes with docstring. You can poke around 
it by using Python's $help()$ command.

Testing
-------

I have included an example 

To see if you have PyIpopt ready, use the following command under the pyipopt's directory. 

		python hs071.py
	
The file "hs071.py" contains a toy optimization problem. If everything is OK, pyipopt will invoke Ipopt to solve it for you. This python file is self-documented and can be used as a template for writing your own optimization problems. 

Pyipopt is a legitimate Python module, you can inspect it by using standard Python commands like "dir" or "help". All functions in pyipopt are well documented. 

Since Hessian estimation is usually tedious, Ipopt can solve problems without Hessian estimation. Pyipopt also supports this feature. The file "hs071.py" demonstrates the idea. If you provide the pyipopt.create function with an "eval_h" callback function as well as the "apply_new" callback function, Ipopt will delegate the Hessian matrix calculation to your function (otherwise Ipopt will approximate Hessian for you).

Contributing
------------

1. Fork it.
2. Create a branch (`git checkout -b my_pyipopt`)
3. Commit your changes (`git commit -am "your awesome message"`)
4. Push to the branch (`git push origin my_pyipopt`)
5. Create an Issue with a link to your branch
6. Nag me about it if I am lazy.

Troubleshooting
---------------

### Check Ipopt

PyIpopt links to Ipopt's C library. If that library is not available PyIpopt will fail
during module initialization. To check the availability of this library, you can go to
	$IPOPT_DIR/Ipopt/examples/hs071_c/
and issue $make to ensure you can compile and run the toy example supplied by Ipopt. 

### Miscellaneous problems

* Error:
	import pyipopt
	ImportError: can not find  libipopt.so.0

* Solution:
    find it and copy it to a folder that ld can access

* Error:
	import pyipopt
	ImportError: /usr/lib/libipopt.so.0: undefined symbol: _gfortran_XXX

* Solution: 
    check if your hs071_c example work. It is very likely that your ipopt library is not correctly compiled. 


* Error:
	import pyipopt
	ImportError: /usr/lib/libipopt.so.0: undefined symbol: SetIntermediateCallback

* Solution:
	SetIntermediateCallback is a function added since Ipopt 3.9.1. (see https://projects.coin-or.org/Ipopt/changeset/1830 )
	Make sure you have an Ipopt version >= 3.9.1

* Error:
	import pyipopt
	ImportError: /usr/lib/libipopt.so.0: undefined symbol: ma19ad_

* Solution:
	First, use 
		nm /usr/lib/libipopt.so.0 | grep ma19ad_ 
	to see if it is marked with U. It should. This means that libipopt.so.0 is not aware of libcoinhsl.so.0. You can fix this
	by adding -lcoinhsl in the makefile of pyipopt. It seems to me that this happens in the recent versions of ipopt. Eventually
	pyipopt will have a better building mechanism, and I will fix this soon. 

* Error:
	import pyipopt
	ImportError: /usr/lib/libipopt.so.0: undefined symbol: SomeKindOfSymbol
	
* Solution:
	I can assure you that it is NOT a bug of pyipopt. It is very likely that you did not link the right package when compiling pyipopt. 
	First, use 
		nm /usr/lib/libipopt.so.0 | grep SomeKindOfSymbol
	to see if this symbol is indeed missing. Do a Google search to find the library file, and 
	add -lWhateverLibrary in the makefile of pyipopt. 
	
	Ipopt is built using various third-party libraries. Different machines may have different set of libraries. You should 
	try to locate these dependencies and indicate them when compiling pyipopt. This is just a limitation of dynamic linking libraries and 
	is not related to Pyipopt. Please do not report a missing symbol error as a "bug" to me unless you are 100% sure it is the problem  of pyipopt. 
	

Contact
--------

Eric Xu <xu.mathena@gmail.com>

Software Engineer @ Google


