"""
Originally contributed by Lorne McIntosh.
Modified by Eric Xu
Further modification by random internet people.

You will probably have to edit this file in unpredictable ways
if you want pyipopt to work for you, sorry.
"""

import os
import subprocess
import warnings
from distutils.core import setup
from distutils.extension import Extension

# NumPy is much easier to install than pyipopt,
# and is a pyipopt dependency, so require it here.
# We need it to tell us where the numpy header files are.
from numpy import get_include as numpy_get_include


def get_compiler_flags():
    """
    Tries to find all needed compiler flags needed to compile the extension
    """
    numpy_include = numpy_get_include()
    try:
        return pkgconfig("ipopt", include_dirs=[numpy_include])
    except (RuntimeError, FileNotFoundError) as e:
        warnings.warn("pkg-config not installed or malformed pc file.\n"
                      "Trying with fallback values. This will probably not work.\n"
                      "Message from pkg-config:\n{}".format(e.args[0]))
        return fallback_compiler_flags()


def pkgconfig(*packages, **kwargs):
    """
    Calls pkg-config returning a dict containing all arguments
    for Extension() needed to compile the extension
    """
    flag_map = {b'-I': 'include_dirs',
                b'-L': 'library_dirs',
                b'-l': 'libraries',
                b'-D': 'define_macros'}
    res = subprocess.run(
        ("pkg-config", "--libs", "--cflags")
        + packages, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    if res.stderr:
        raise RuntimeError(res.stderr.decode())
    for token in res.stdout.split():
        kwargs.setdefault(flag_map.get(token[:2]), []).append(
            token[2:].decode())
    define_macros = kwargs.get('define_macros')
    if define_macros:
        kwargs['define_macros'] = [tuple(d.split()) for d in define_macros]
    undefined_flags = kwargs.pop(None, None)
    if undefined_flags:
        warnings.warn(
            "Ignoring flags {} from pkg-config".format(", ".join(undefined_flags)))
    return kwargs


def fallback_compiler_flags():
    """
    This gets called if pkg-config is not installed on your system.
    Comments:
    When I installed Ipopt from source, I used the
    --prefix=/usr/local
    option, so this is where I want pyipopt to look for my ipopt installation.
    I only installed from source because the ipopt packaging
    for my linux distribution was buggy,
    so by the time you read this the bugs have probably been fixed
    and you will want to specify a different directory here.
    The extra_link_args is commented out here;
    that line was causing my pyipopt install to not work.
    Also I am using coinmumps instead of coinhsl.
    """
    IPOPT_DIR = '/usr/local/'
    IPOPT_INC = os.path.join(IPOPT_DIR, 'include/coin/')
    return {
        "libraries": [
            'ipopt', 'coinblas',
            # 'coinhsl',
            'coinmumps',
            'coinmetis',
            'coinlapack', 'dl', 'm',
        ],
        "include_dirs": [numpy_get_include(), IPOPT_INC],
        "library_dirs": [get_ipopt_lib(IPOPT_DIR)]
        # ,"extra_link_args": ['-Wl,--rpath','-Wl,'+ IPOPT_LIB]
    }


def get_ipopt_lib(IPOPT_DIR):
    for lib_suffix in ('lib', 'lib64'):
        d = os.path.join(IPOPT_DIR, lib_suffix)
        if os.path.isdir(d):
            return d
    raise FileNotFoundError('failed to find ipopt lib')


pyipopt_extension = Extension(
    'pyipoptcore',
    sources=['src/callback.c', 'src/pyipoptcoremodule.c'],
    **get_compiler_flags()
)

setup(
    name="pyipopt",
    version="0.8",
    description="An IPOPT connector for Python",
    author="Eric Xu",
    author_email="xu.mathena@gmail.com",
    url="https://github.com/xuy/pyipopt",
    packages=['pyipopt'],
    package_dir={'pyipopt': 'pyipoptpackage'},
    ext_package='pyipopt',
    ext_modules=[pyipopt_extension],
)
