cmake_minimum_required(VERSION 2.8)
project(edlib)

# Define output directories for created binaries and libraries.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(edlib/include)
file(GLOB SOURCES "edlib/src/*.cpp")

# Create binaries.
add_executable(helloWorld apps/hello-world/helloWorld.c ${SOURCES})
add_executable(aligner apps/aligner/aligner.cpp ${SOURCES})
add_executable(runTests test/runTests.cpp ${SOURCES})

# Create libraries.
add_library(edlib SHARED ${SOURCES})
add_library(edlib_static STATIC ${SOURCES})

# Create target 'install' for installing libraries.
install(TARGETS edlib edlib_static DESTINATION lib)
install(FILES edlib/include/edlib.h DESTINATION include)
