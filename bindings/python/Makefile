default: build


edlib: ../../edlib
	cp -R ../../edlib .

pyedlib.bycython.cpp: edlib.pyx cedlib.pxd
	cython --cplus edlib.pyx -o edlib.bycython.cpp

build: edlib pyedlib.bycython.cpp setup.py README-tmpl.rst
	python setup.py build_ext -i

README.rst: build README-tmpl.rst
	cog -o README.rst README-tmpl.rst	

sdist: edlib pyedlib.bycython.cpp setup.py README.rst MANIFEST.in
	python setup.py sdist

publish: clean sdist
	twine upload dist/*

clean:
	rm -rf edlib dist edlib.egg-info build
	rm -f edlib.c *.bycython.* edlib.*.so
	rm -f README.rst

