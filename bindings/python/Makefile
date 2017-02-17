default: build

############### FILES #################
FILES=edlib pyedlib.bycython.c setup.py MANIFEST.in README.rst

edlib: ../../edlib
	cp -R ../../edlib .

pyedlib.bycython.c: edlib.pyx cedlib.pxd
	cython edlib.pyx -o edlib.bycython.c
#######################################

############## COMMANDS ###############
build: ${FILES}
	python setup.py build_ext -i

sdist: ${FILES}
	python setup.py sdist

publish: clean sdist
	twine upload dist/*

clean:
	rm -rf edlib dist edlib.egg-info build
	rm -f edlib.c *.bycython.* edlib.*.so
#######################################
