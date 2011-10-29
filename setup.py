# Originally contributed by Lorne McIntosh.
# Modified by Eric Xu

# ========= Edit The Line Below ==============
IPOPT_DIR='/home/eric/Sketch/Ipopt-3.10.1/'
# ========= Edit The Line Above ==============

# ========= Don't touch things below this ====
from distutils.core import setup
from distutils.extension import Extension

IPOPT_LIB=IPOPT_DIR+"lib"
IPOPT_INC=IPOPT_DIR+"include/coin/"

#find the numpy headers automatically
numpy_include = ""
try: 
  import numpy
  numpy_include = numpy.get_include()
except: pass

FILES = ["src/callback.c", "src/pyipopt.c"]

setup(name="pyipopt",
  version="0.8",
  description="An IPOPT connector for Python",
  author="Eric Xu",
  author_email="xu.mathena@gmail.com",
  url="https://github.com/xuy/pyipopt",
  ext_modules=[
    Extension("pyipopt",FILES,
    extra_link_args=['-Wl,--rpath','-Wl,'+ IPOPT_LIB],
    library_dirs=[IPOPT_LIB],
    libraries=['ipopt','coinblas','coinlapack','coinmetis','coinmumps','dl','m'],
    include_dirs=[numpy_include,IPOPT_INC]),
  ]
)
