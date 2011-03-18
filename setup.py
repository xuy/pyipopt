# setup.py

# This file is contributed by Lorne McIntosh
#======================================================
# You should specify the location of IPOPT on your
# system using the setup.cfg file before running this.
#======================================================

from distutils.core import setup
from distutils.extension import Extension

#find the numpy headers automatically
numpy_include = ""
try: import numpy; numpy_include = numpy.get_include()
except: pass

FILES = ["src/callback.c", "src/pyipopt.c"]

setup(name="pyipopt",
    version="1.0",
    description="An IPOPT connector for Python",
    author="Eric You Xu",
    author_email="youxu@wustl.edu",
    url="http://code.google.com/p/pyipopt/",
    ext_modules=[
        Extension("pyipopt",FILES,
        library_dirs=[],
        libraries=[],
        include_dirs=[numpy_include]),
    ]
)
