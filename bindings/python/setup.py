from setuptools import setup, Extension
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
    version = "1.1.2-1",
    url = "https://github.com/Martinsos/edlib",
    author = "Martin Sosic",
    author_email = "sosic.martin@gmail.com",
    license = "MIT",
    keywords = "edit distance levenshtein align sequence bioinformatics",
    # Build instructions
    ext_modules = [Extension("edlib",
                             ["pyedlib.pyx", "edlib/src/edlib.cpp"],
                             include_dirs=["edlib/include"],
                             depends=["edlib/include/edlib.h"],
                             extra_compile_args=["-O3"])],
    # Since 18.0, setuptools supports having Cython as a build dependency.
    # If we have Cython in setup_requires and .pyx files listed in extension sources,
    # setuptools will make sure to install Cython as a local egg and cythonize
    # the source files on build - there is no need to explicitely call `cythonize()`.
    setup_requires = ['setuptools>=18.0', 'cython>=0.25']
)
