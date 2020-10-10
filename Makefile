BUILD_DIR=meson-build
TEST_DATA_DIR=apps/aligner/test_data

YELLOW=\033[33m
GREEN=\033[32m
BOLD=\033[1m
RESET=\033[0m

all:
	@printf "${YELLOW}${BOLD}Configuring build setup via Meson...${RESET}\n"
	${MAKE} configure
	@printf "\n${YELLOW}${BOLD}Building binaries and libraries via ninja...${RESET}\n"
	${MAKE} build
	@printf "\n${YELLOW}${BOLD}Running integration tests...${RESET}\n"
	${MAKE} integration-tests
	@printf "\n${GREEN}${BOLD}Edlib successfully built! You can find built binaries and libraries in ${BUILD_DIR}/ directory.${RESET}\n"

configure:
	rm -rf ${BUILD_DIR}
	meson setup ${BUILD_DIR} .

build:
	ninja -v -C ${BUILD_DIR}

test:
	cd ${BUILD_DIR}; meson test -v

integration-tests:
	${BUILD_DIR}/hello-world
	${BUILD_DIR}/edlib-aligner ${TEST_DATA_DIR}/query.fasta ${TEST_DATA_DIR}/target.fasta

install:
	cd ${BUILD_DIR}; meson install

clean:
	rm -rf ${BUILD_DIR}

.PHONY: all build configure test install clean
