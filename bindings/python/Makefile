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
	# TODO:
	# 1. -> try not depending `sdist` here in make, but instead: clean edlib README.rst? Maybe add pyedlib.bycython.cpp? And then also explicitly remove some dirs and .so files? Yeah, remove the most I can, but make sure what I need is created. Yeah, so probably I should just rm -r dist and build and .so files, as per cibuildwheel recommendations, but make sure I have ready stuff that I do need, which is almost certainly edlib, and README.rst it seems, and I guess pyedlib.bycython.cpp should also be needed. Maybe I should understand better how cibuildwheel builds the project -> what does it do, run setup.py? I think they have default build command that can be configure then. I should learn about that.
	# 2. -> I wonder if the problem is that I create `edlib` outside of manylinux docker. Could I instead have it created inside? But CIWB_BEFORE or what was it didn't work when told to run `make sdist`. So I removed that. But maybe I should introduce that back, that might be important! And get it working. Was it ever working though? Yeah, this is almost certainly a problem! Oh and why runnin `make edlib` from cibuildwheel doesn't work is probably because cibuildwheel starts its Dockerfile context from this folder, but make tries to copy edlib source files from two levels above! I could solve this by copying those files first, outside of cibuildwheel, and then `make` should work inside cibuildwheel .
	# 3. -> I can use newer version of manylinux: 2_28. That does seem to help, at least locally, I think I still had issues in the cloud though?
	#    CIBW_MANYLINUX_X86_64_IMAGE=manylinux_2_28 \
	# 4. -> I can use older gcc. Which is easiest to do by just using older Ubuntu.
	CIBW_SKIP="pp* *-manylinux_i686" \
	CIBW_TEST_COMMAND="python3 {project}/test.py" \
	python3 -m cibuildwheel --output-dir wheelhouse

clean:
	rm -rf edlib dist edlib.egg-info build wheelhouse
	rm -f edlib.c *.bycython.* edlib.*.so
	rm -f README.rst
