# Edlib python package

This README contains only development information, you can check out full README (README.rst) for the latest version of Edlib python package on [Edlib's PyPI page](https://pypi.org/project/edlib/).

README.rst is not commited to git because it is generated from [README-tmpl.rst](./README-tmpl.rst).

## Setup

Ensure you have the version of python you want to use installed. You can use `pyenv` to manage python versions and pick specific one, if needed.

Let's now say that `python` is pointing to the python version you want to use.
What you will most likely want to do now is run (from this dir, bindings/python/):
```sh
python -m venv .venv
```
to create the virtual environment if you don't have it yet, and then
```sh
source .venv/bin/activate
```
to activate it.

This virtual environment will ensure all the python packages are installed locally for this project, and that local python packages are used.
You will want to keep this virtual environment activated for the rest of the commands in this README.

Actual installation of python deps (packages) is not done by the "requirements.txt" as is typical for python projects, but by the `Makefile`: read on for the details on how to build with it.

## Building

Run `make build` to generate an extension module as .so file.
You can test it then by importing it from python interpreter `import edlib` and running `edlib.align(...)` (you have to be positioned in the directory where .so was built).
This is useful for testing while developing.

Run `make sdist` to create a source distribution, but not publish it - it is a tarball in dist/ that will be uploaded to pip on `publish`.
Use this to check that tarball is well structured and contains all needed files, before you publish.
Good way to test it is to run `sudo pip install dist/edlib-*.tar.gz`, which will try to install edlib from it, same way as pip will do it when it is published.

`make clean` removes all generated files.

README.rst is auto-generated from [README-tmpl.rst](./README-tmpl.rst), to run regeneration do `make README.rst`.
README.rst is also automatically regenerated when building package (e.g. `make build`).
This enables us to always have up to date results of code execution and help documentation of edlib methods in readme.

## Publishing
Remember to update version in setup.py before publishing.

To trigger automatic publish to PyPI, create a tag and push it to Github -> Travis will create sdist, build wheels, and push them all to PyPI while publishing new version.

You can also publish new version manually if needed: run `make publish` to create a source distribution and publish it to the PyPI.
