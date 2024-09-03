default: build

.PHONY:
edlib: $(shell find ../../edlib)
	# create a clean (maybe updated) copy of edlib src
	rm -rf edlib && cp -r ../../edlib .

pyedlib.bycython.cpp: edlib.pyx cedlib.pxd
	python -m pip install cython
	cython --cplus edlib.pyx -o edlib.bycython.cpp

# To build package, README.rst is needed, because it goes into long description of package,
# which is what is visible on PyPI.
# However, to generate README.rst from README-tmpl.rst, built package is needed (for `import edlib` in cog)!
# Therefore, we first build package without README.rst, use it to generate README.rst,
# and then finally build package again but with README.rst.

BUILD_SOURCE_FILES=edlib pyedlib.bycython.cpp setup.py

buildWithoutREADME.rst: ${BUILD_SOURCE_FILES}
	EDLIB_OMIT_README_RST=1 python setup.py build_ext -i

README.rst: buildWithoutREADME.rst README-tmpl.rst
	python -m pip install cogapp
	cog -d -o README.rst README-tmpl.rst

BUILD_FILES=${BUILD_SOURCE_FILES} README.rst

build: ${BUILD_FILES}
	python setup.py build_ext -i

sdist: edlib pyedlib.bycython.cpp setup.py README.rst MANIFEST.in
	python setup.py sdist

publish: clean sdist
	twine upload dist/*

wheels: sdist
	python -m pip install cibuildwheel==2.20.0
	# TODO: Try using older ubuntu/mac on CI to build wheels, maybe that helps, and then I don't need to set newer manylinux here?
	# Default manylinux image is 2014 but it is quite old and won't build
	# so I explicitly specify a newer version.
	CIBW_MANYLINUX_X86_64_IMAGE=manylinux_2_28 \
	# TODO: I was producing wheels for musllinux before! Why doesn't it work anymore?
	#  Is it possible that `make sdist` is issue?
	CIBW_SKIP="pp* *-manylinux_i686 *-musllinux_*" \
	CIBW_TEST_COMMAND="python3 {project}/test.py" \
	python3 -m cibuildwheel --output-dir wheelhouse

clean:
	rm -rf edlib dist edlib.egg-info build wheelhouse
	rm -f edlib.c *.bycython.* edlib.*.so
	rm -f README.rst
