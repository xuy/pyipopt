CC = gcc
CFLAGS = -O3 -fpic -shared
LDFLAGS = -lipopt -lg2c -lm
PY_DIR = /usr/local/lib/python2.5/site-packages

# Change this to your ipopt include path that includes IpStdCInterface.h 
IPOPT_INCLUDE = /Project/ThirdParty/ipopt/Ipopt-3.3.3/include/ipopt/

# Change this to your python dir which includes Python.h
# You might need to download the python source code or install python-dev to get
# this header file. Note that Pyipopt needs this as an extend python module. 
PYTHON_INCLUDE = /usr/local/include/python2.5

# Change this to your numpy include path which contains numpy/arrayobject.h
# If you don't want this and would like to use list. I have a nasty (and unmaintained version) in the package called pyipopt-list.c. You can compile it and use it without numpy. 
# However, numpy is strongly suggested. 

NUMPY_INCLUDE = /usr/lib/python2.5/site-packages/numpy/core/include

pyipopt: callback.c pyipopt.c
	$(CC) -o pyipopt.so -I$(PYTHON_INCLUDE) -I$(IPOPT_INCLUDE) -I$(NUMPY_INCLUDE) $(CFLAGS) $(LDFLAGS) pyipopt.c callback.c

install: pyipopt
	cp ./pyipopt.so $(PY_DIR)
clean:
	rm pyipopt.so 
