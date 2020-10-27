cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(edlib VERSION 1.2.6)

option(EDLIB_ENABLE_INSTALL "Generate the install target" ON)
option(EDLIB_BUILD_EXAMPLES "Build examples" ON)
option(EDLIB_BUILD_UTILITIES "Build utilities" ON)

set(MACOSX (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug mode")
endif()

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 14)
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
add_library(edlib::edlib ALIAS edlib)
set_target_properties(edlib
  PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_definitions(edlib PRIVATE DLIB_BUILD)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(edlib PUBLIC EDLIB_SHARED)
endif()

target_include_directories(edlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/edlib/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Build binaries.
if(EDLIB_BUILD_EXAMPLES)
  add_executable(helloWorld apps/hello-world/helloWorld.c)
  target_link_libraries(helloWorld edlib)
endif()

include(CTest)
if (BUILD_TESTING)
  add_executable(runTests test/runTests.cpp)
  target_link_libraries(runTests edlib)

  add_test(edlib_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runTests)
endif()

if(EDLIB_BUILD_UTILITIES)
  if(NOT WIN32) # If on windows, do not build binaries that do not support windows.
    add_executable(edlib-aligner apps/aligner/aligner.cpp)
    target_link_libraries(edlib-aligner edlib)
  endif()
endif()

# configure and install pkg-config file
set(PKG_EDLIB_DEFS "")
get_target_property(EDLIB_INTERFACE_DEFS edlib INTERFACE_COMPILE_DEFINITIONS)
if(NOT (EDLIB_INTERFACE_DEFS STREQUAL "NOTFOUND" OR EDLIB_INTERFACE_DEFS STREQUAL "EDLIB_INTERFACE_DEFS-NOTFOUND"))
  foreach(EDLIB_INTERFACE_DEF ${EDLIB_INTERFACE_DEFS})
    list(APPEND PKG_EDLIB_DEFS "-D${EDLIB_INTERFACE_DEF}")
  endforeach()
endif()
string(REPLACE ";" " " PKG_EDLIB_DEFS "${PKG_EDLIB_DEFS}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/edlib.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/edlib-${PROJECT_VERSION_MAJOR}.pc
    @ONLY)

if(EDLIB_ENABLE_INSTALL)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/edlib-${PROJECT_VERSION_MAJOR}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  include(CMakePackageConfigHelpers)
  set(EDLIB_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/edlib" CACHE STRING
    "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
  set(version_config "${PROJECT_BINARY_DIR}/edlib-config-version.cmake")
  set(project_config "${PROJECT_BINARY_DIR}/edlib-config.cmake")
  set(targets_export_name edlib-targets)

  # Generate the version, config and target files into the build directory.
  write_basic_package_version_file(
    ${version_config}
    VERSION ${VERSION}
    COMPATIBILITY AnyNewerVersion)
  configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/edlib-config.cmake.in
    ${project_config}
    INSTALL_DESTINATION ${EDLIB_CMAKE_DIR})
  export(TARGETS edlib NAMESPACE edlib::
    FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

  # Install version, config and target files.
  # These are cmake config files and they are useful for some consumers, for example vcpkg.
  install(
    FILES ${project_config} ${version_config}
    DESTINATION ${EDLIB_CMAKE_DIR})
  install(EXPORT ${targets_export_name} DESTINATION ${EDLIB_CMAKE_DIR}
    NAMESPACE edlib::)
  
  # Create target 'install' for installing libraries.
  install(
    TARGETS edlib
    EXPORT ${targets_export_name} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(FILES edlib/include/edlib.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
