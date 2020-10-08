PREFIX=${CURDIR}/PREFIX
BDIR=MESON
TDIR=apps/aligner/test_data

all:
	${MAKE} setup
	${MAKE} build
	${MAKE} integ-test
build:
	ninja -v -C ${BDIR}
test:
	cd ${BDIR}; meson test -v
integ-test:
	${BDIR}/hello-world
	${BDIR}/edlib-aligner ${TDIR}/query.fasta ${TDIR}/target.fasta
install:
	# Install into ${PREFIX}, per setup.
	ninja -C ${BDIR} install
setup:
	rm -rf ${BDIR}
	meson setup ${BDIR} . --prefix=${PREFIX}
.PHONY: all build setup test install
