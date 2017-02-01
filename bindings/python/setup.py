from setuptools import setup, Extension
import Cython.Build
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    # Information
    name = "edlib",
    description = "Lightweight, super fast library for sequence alignment using edit (Levenshtein) distance.",
    long_description = long_description,
    version = "1.1.2",
    url = "https://github.com/Martinsos/edlib",
    author = "Martin Sosic",
    author_email = "sosic.martin@gmail.com",
    license = "MIT",
    keywords = "edit distance levehnstein align sequence bioinformatics",
    # Build instructions
    ext_modules = [Extension("edlib",
                             ["edlib.pyx", "edlib/src/edlib.cpp"],
                             include_dirs=["edlib/include"],
                             depends=["edlib/include/edlib.h"],
                             extra_compile_args=["-O3"])],
    setup_requires = ['cython (>=0.25)'],
    cmdclass = {'build_ext': Cython.Build.build_ext}
)
