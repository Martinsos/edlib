cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(edlib VERSION 1.2.5)

set(MACOSX (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug mode")
endif()

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Falling back to different standard it not allowed.
set(CMAKE_CXX_EXTENSIONS OFF)  # Make sure no compiler-specific features are used.

# Build edlib as static library by default
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries as shared")

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

  check_cxx_compiler_flag(-Wold-style-cast WOLD_STYLE_CAST)
  if (WOLD_STYLE_CAST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
  endif()
  check_cxx_compiler_flag(-Wshadow WSHADOW)
  if (WSHADOW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Define output directories for created binaries and libraries.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SOURCES "edlib/src/*.cpp")

# Create libraries.
add_library(edlib ${SOURCES})
set_target_properties(edlib
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION})

target_include_directories(edlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/edlib/include>
    $<INSTALL_INTERFACE:include>)

# Build binaries.
add_executable(helloWorld apps/hello-world/helloWorld.c)
target_link_libraries(helloWorld edlib)

include(CTest)
if (BUILD_TESTING)
  add_executable(runTests test/runTests.cpp)
  target_link_libraries(runTests edlib)

  add_test(edlib_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runTests)
endif()

if (NOT WIN32) # If on windows, do not build binaries that do not support windows.
  add_executable(edlib-aligner apps/aligner/aligner.cpp)
  target_link_libraries(edlib-aligner edlib)
endif()


# Create target 'install' for installing libraries.
install(TARGETS edlib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES edlib/include/edlib.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# configure and install pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/edlib.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/edlib-1.pc
    @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/edlib-1.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
