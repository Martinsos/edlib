cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(edlib)

set(MACOSX (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug mode")
endif()

# Set strict warnings.
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  message("Setting warning flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Define macro for setting cxx11 as standard for building a target.
macro(use_cxx11 target)
  if ((CMAKE_VERSION VERSION_LESS "3.2") AND MACOSX)
    # Because CXX_STANDARD is not supported for OSX in Cmake 3.1.
    target_compile_options(${target} PRIVATE "-std=c++11")
  else()
    set_target_properties(${target} PROPERTIES
      CXX_STANDARD 11
      CXX_STANDARD_REQUIRED ON
      CXX_EXENSIONS NO)
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
