# Edlib python package

This README contains only development information, you can check out full README (README.rst) for the latest version of Edlib python package on [Edlib's PyPI page](https://pypi.org/project/edlib/).

README.rst is not commited to git because it is generated from [README-tmpl.rst](./README-tmpl.rst).


## Development

### Setup

Besides python, you will need `cython` and `cog` to build edlib python package.

Run `pip install cython` to install `cython`, which is used to wrap C/C++ code into python.

Run `pip install cogapp` to install `cog`, which is used to generate README.rst from README-tmpl.rst.

### Building

Run `make build` to generate an extension module as .so file.
You can test it then by importing it from python interpreter `import edlib` and running `edlib.align(...)` (you have to be positioned in the directory where .so was built).
This is useful for testing while developing.

Run `make sdist` to create a source distribution, but not publish it - it is a tarball in dist/ that will be uploaded to pip on `publish`.
Use this to check that tarball is well structured and contains all needed files, before you publish.
Good way to test it is to run `sudo pip install dist/edlib-*.tar.gz`, which will try to install edlib from it, same way as pip will do it when it is published.

Run `make publish` to create a source distribution and publish it to the PyPI. Use this to publish new version of package.
Make sure to bump the version in `setup.py` before publishing, if needed.

`make clean` removes all generated files.

README.rst is auto-generated from [README-tmpl.rst](./README-tmpl.rst), to run regeneration do `make README.rst`.
README.rst is also automatically regenerated when building package (e.g. `make build`).
This enables us to always have up to date results of code execution and help documentation of edlib methods in readme.
