.PHONY: default buildWithoutREADME.rst build sdist publish wheels clean

default: build

# Copies edlib cpp source files into the project, if not copied yet.
edlib:
	if [ ! -d edlib ]; then \
		cp -r ../../edlib . ; \
	fi

pyedlib.bycython.cpp: edlib.pyx cedlib.pxd
	python -m pip install cython
	cython --cplus edlib.pyx -o edlib.bycython.cpp

BUILD_SOURCE_FILES=edlib pyedlib.bycython.cpp setup.py

# To build package, README.rst is needed, because it goes into long description of package,
# which is what is visible on PyPI.
# However, to generate README.rst from README-tmpl.rst, built package is needed (for `import edlib` in cog)!
# Therefore, we first build package without README.rst, use it to generate README.rst,
# and then finally build package again but with README.rst.
buildWithoutREADME.rst: ${BUILD_SOURCE_FILES}
	EDLIB_OMIT_README_RST=1 \
	python -m pip install -e .

README.rst: buildWithoutREADME.rst README-tmpl.rst
	python -m pip install cogapp
	cog -d -o README.rst README-tmpl.rst

BUILD_FILES=${BUILD_SOURCE_FILES} README.rst

build: ${BUILD_FILES}
	python -m pip install -e .

sdist: ${BUILD_FILES} MANIFEST.in
	python -m pip install build
	python -m build --sdist

# cibuildwheel builds wheels in docker containers. These containers are chosen and
# configured in such way that generated wheels are as robust as they can be in regards
# to glibc, musl and similar.
# They also ensure wheels are built in a reproducible way, independent of the host machine.
#
# For cibuildwheel to be able to build the wheels, we need to prepare / prebuild some
# some stuff for it in advance. Good way to handle this is to run `make sdist` which
# certainly builds everything that cibuildwheel might need to build the wheels.
#
# However, we can't just run `make sdist` manually before starting the cibuildwheel, because then
# `make sdist` will build its stuff on our local machine, and once cibuildwheel copies
# those in its container and tries to use them to build wheels, problems can arise due to
# those having been built with different tooling than what is now used to build the wheels!
#
# Instead, we want `make sdist` to run in cibuildwheel's docker containers also,
# so it uses the same tooling (e.g. gcc) as will be used to build the wheels, and we accomplish
# this by using CIBW_BEFORE_BUILD option, which will run given cmd in the container
# before building the wheel.
#
# There is though one thing we do need to "prebuild" on our machine in advance:
# copying edlib/ dir from ../../edlib to here. We can't do it inside the container
# because Docker context's root is . and not ../../, so ../../edlib is not available
# in docker container.
# That is why it is important that we have `edlib` here as dep for `wheels` Makefile target.
# We also `clean` first to remove everything else, so we have clean start in the container,
# since as we said, we want to make sure everything is built in there, with same tooling.
#
# So, first we clean any old build artifacts, then we ensure ../../edlib is copied to
# ./edlib, and then we call cibuildwheel which builds wheels in its docker containers
# running `make sdist` in the containers at the start of every wheel build.
wheels: clean edlib
	python -m pip install cibuildwheel==2.20.0
	CIBW_SKIP="pp* *-manylinux_i686 cp38-*" \
	CIBW_BEFORE_BUILD="make sdist" \
	CIBW_TEST_COMMAND="python3 {project}/test.py" \
	python -m cibuildwheel --output-dir wheelhouse

publish: clean sdist wheels
	python -m twine upload dist/edlib-*.tar.gz wheelhouse/*.whl

clean:
	rm -rf edlib dist edlib.egg-info build wheelhouse
	rm -f edlib.c *.bycython.* edlib.*.so
	rm -f README.rst
