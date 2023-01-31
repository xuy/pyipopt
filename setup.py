import os
from distutils.core import setup
from distutils.extension import Extension

# IPOPT source code folder. It is where you untar 
# the source code or cloned from the github repo
# https://github.com/coin-or/Ipopt

# The folder to find IpStdCInterface.h.
# Default to the installation path when you run `make install`
# for Ipopt.
####### YOU MAY NEED TO CHANGE THIS ######## 
IPOPT_HEADER_DIR = '/usr/local/include/coin-or/'
####### YOU MAY NEED TO CHANGE THIS ######## 

# The folder to find libipopt.so/la.
# Default to the install path for ipopt
####### YOU MAY NEED TO CHANGE THIS ######## 
IPOPT_LIB_DIR = '/usr/local/lib/'
####### YOU MAY NEED TO CHANGE THIS ######## 

def get_numpy_include_dir():
    import numpy
    return numpy.get_include()

def get_ipopt_include_dir():
    return IPOPT_HEADER_DIR

def get_ipopt_lib_dir():
    return IPOPT_LIB_DIR

source = ['src/callback.c', 'src/pyipoptcoremodule.c']

print(get_numpy_include_dir())
print(get_ipopt_include_dir())


pyipopt_extension = Extension(
    'pyipoptcore',
    source,
    library_dirs=[get_ipopt_lib_dir()],
    libraries=[
        'ipopt', 
        'dl',
        'm',
    ],
    include_dirs=[get_numpy_include_dir(), get_ipopt_include_dir()],
)

setup(
    name="pyipopt",
    version="0.8",
    description="A Python connector for Ipopt",
    author="Eric Xu",
    author_email="xu.mathena@gmail.com",
    url="https://github.com/xuy/pyipopt",
    packages=['pyipopt'],
    package_dir={'pyipopt' : 'pyipoptpackage'},
    ext_package='pyipopt',
    ext_modules=[pyipopt_extension],
)
