cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(edlib)

set(MACOSX (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug mode")
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
  endif()
endif()

macro(use_cxx11 target)
    if ((CMAKE_VERSION VERSION_LESS "3.2") AND MACOSX)
        # Because CXX_STANDARD is not supported for OSX in Cmake 3.1.
        target_compile_options(${target} PRIVATE "-std=c++11")
    else()
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
    endif()
endmacro(use_cxx11)


# Define output directories for created binaries and libraries.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(edlib/include)
file(GLOB SOURCES "edlib/src/*.cpp")


# Create libraries.
add_library(edlib SHARED ${SOURCES})
use_cxx11(edlib)

add_library(edlib_static STATIC ${SOURCES})
use_cxx11(edlib_static)


# Build binaries.
add_executable(helloWorld apps/hello-world/helloWorld.c)
target_link_libraries(helloWorld edlib_static)

add_executable(runTests test/runTests.cpp)
target_link_libraries(runTests edlib_static)
use_cxx11(runTests)

if (NOT WIN32) # If on windows, do not build binaries that do not support windows.
  add_executable(edlib-aligner apps/aligner/aligner.cpp)
  target_link_libraries(edlib-aligner edlib_static)
  use_cxx11(edlib-aligner)
endif()


# Create target 'install' for installing libraries.
install(TARGETS edlib edlib_static DESTINATION lib)
install(FILES edlib/include/edlib.h DESTINATION include)
