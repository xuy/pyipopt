# Originally contributed by Lorne McIntosh.
# Modified by Eric Xu
# Further modification by random internet people.

# You will probably have to edit this file in unpredictable ways
# if you want pyipopt to work for you, sorry.

# When I installed Ipopt from source, I used the
# --prefix=/usr/local
# option, so this is where I want pyipopt to look for my ipopt installation.
# I only installed from source because the ipopt packaging
# for my linux distribution was buggy,
# so by the time you read this the bugs have probably been fixed
# and you will want to specify a different directory here.
IPOPT_DIR = '/usr/local/'

import os
from distutils.core import setup
from distutils.extension import Extension

# NumPy is much easier to install than pyipopt,
# and is a pyipopt dependency, so require it here.
import numpy

# I personally do not need support for lib64 but I'm keeping it in the code.
def get_ipopt_lib():
    for lib_suffix in ('lib', 'lib64'):
        d = os.path.join(IPOPT_DIR, lib_suffix)
        if os.path.isdir(d):
            return d

IPOPT_LIB = get_ipopt_lib()
if IPOPT_LIB is None:
    raise Exception('failed to find ipopt lib')

IPOPT_INC = os.path.join(IPOPT_DIR, 'include/coin/')

# Find the numpy headers automatically.
numpy_include = numpy.get_include()

FILES = ['src/callback.c', 'src/pyipopt.c']

# The extra_link_args is commented out here;
# that line was causing my pyipopt install to not work.
# Also I am using coinmumps instead of coinhsl.
pyipopt_extension = Extension(
        'pyipopt',
        FILES,
        #extra_link_args=['-Wl,--rpath','-Wl,'+ IPOPT_LIB],
        library_dirs=[IPOPT_LIB],
        libraries=[
            'ipopt', 'blas',
            #'coinhsl',
            'coinmumps',
            'coinmetis',
            'lapack','dl','m',
            ],
        include_dirs=[numpy_include, IPOPT_INC],
        )

setup(
        name="pyipopt",
        version="0.8",
        description="An IPOPT connector for Python",
        author="Eric Xu",
        author_email="xu.mathena@gmail.com",
        url="https://github.com/xuy/pyipopt",
        ext_modules=[pyipopt_extension],
        )

