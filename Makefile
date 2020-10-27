BUILD_DIR ?= meson-build
# Can be 'static' or 'shared'.
LIBRARY_TYPE ?= static
TEST_DATA_DIR ?= apps/aligner/test_data

YELLOW = \033[33m
GREEN  = \033[32m
BOLD   = \033[1m
RESET  = \033[0m

all:
	@printf "${YELLOW}${BOLD}Configuring build setup via Meson...${RESET}\n"
	${MAKE} configure
	@printf "\n${YELLOW}${BOLD}Building ${LIBRARY_TYPE} library and binaries...${RESET}\n"
	${MAKE} build
	@printf "\n${YELLOW}${BOLD}Running tests...${RESET}\n"
	${MAKE} test
	@printf "\n${GREEN}${BOLD}Edlib successfully built! You can find built binaries and libraries in ${BUILD_DIR}/ directory.${RESET}\n"

configure:
	rm -rf ${BUILD_DIR}
	meson setup ${BUILD_DIR} . \
		--backend=ninja \
    -Ddefault_library=${LIBRARY_TYPE}

build:
	meson compile -v -C ${BUILD_DIR}

test:
	meson test -v -C ${BUILD_DIR}

# Valgrind Returns 2 if there was a memory leak/error,
# otherwise returns runTests exit code, which is 0 if
# all went fine or 1 if some of the tests failed.
check-memory-leaks:
	valgrind ${BUILD_DIR}/runTests 2 \
    --quiet --error-exitcode=2 --tool=memcheck --leak-check=full

install:
	meson install -C ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

.PHONY: all build configure test install clean check-memory-leaks
