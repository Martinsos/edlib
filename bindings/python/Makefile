default: build

FILES=edlib *.pyx *.pxd setup.py MANIFEST.in README.rst

edlib: ../../edlib
	cp -R ../../edlib .

build: ${FILES}
	OPT="" python setup.py build_ext -i

sdist: ${FILES}
	cp -R ../../edlib .
	python setup.py sdist

publish: sdist
	twine upload dist/*

clean:
	rm -rf edlib dist edlib.egg-info build
	rm -f edlib.c edlib.*.so
