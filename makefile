CC = gcc
CFLAGS = -O3 -fpic -shared
DFLAGS = -fpic -shared
LDFLAGS = -lipopt -lm -llapack -lblas -lcoinhsl
# You might also need -lgfortran, I don't have it because I use g77 to compile my ipopt
PY_DIR = /usr/local/lib/python2.5/site-packages

# Change this to your ipopt include path that includes IpStdCInterface.h 
#IPOPT_INCLUDE = /Project/ThirdParty/ipopt/Ipopt-3.3.3/include/ipopt/
IPOPT_INCLUDE = /home/eric/Scratch/Ipopt-3.9.2/include/coin/

# Change this to your python dir which includes Python.h
# You might need to download the python source code or install python-dev to get
# this header file. Note that Pyipopt needs this as an extend python module. 
#PYTHON_INCLUDE = /usr/local/include/python2.5
PYTHON_INCLUDE = /usr/include/python2.6/

# This points to your numpy install. 
NUMPY_INCLUDE = /usr/local/lib/python2.6/dist-packages/numpy-1.5.1-py2.6-linux-x86_64.egg/numpy/core/include

pyipopt: src/callback.c src/pyipopt.c
	$(CC) -o pyipopt.so -I$(PYTHON_INCLUDE) -I$(IPOPT_INCLUDE) -I$(NUMPY_INCLUDE) $(CFLAGS) $(LDFLAGS) src/pyipopt.c src/callback.c

install: pyipopt
	cp ./pyipopt.so $(PY_DIR)
clean:
	rm pyipopt.so 
