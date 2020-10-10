project(
  'edlib',
  'cpp', 'c',
  version : '1.2.6',
  default_options : [
    'buildtype=release',
    'warning_level=3',
    'cpp_std=c++14',
    'b_ndebug=if-release',
    'default_library=both'
  ],
  license : 'MIT',
  meson_version : '>= 0.52.0',
)

edlib_lib = library('edlib',
  sources : files(['edlib/src/edlib.cpp']),
  include_directories : ['edlib/include'],
  dependencies : [],
  install : true,
)

install_headers('edlib/include/edlib.h')

edlib_dep = declare_dependency(
  include_directories : ['edlib/include'],
  link_with : edlib_lib
)

hello_main = executable(
  'hello-world',
  files(['apps/hello-world/helloWorld.c']),
  dependencies : [edlib_dep],
)

aligner_main = executable(
  'edlib-aligner',
  files(['apps/aligner/aligner.cpp']),
  dependencies : [edlib_dep],
  install : true,
)

runTests_main = executable(
  'runTests',
  files(['test/runTests.cpp']),
  dependencies : [edlib_dep],
  include_directories : ['test'],
)

test('runTests', runTests_main)

test('hello', hello_main)

test('aligner', aligner_main,
  args : [
    files('apps/aligner/test_data/query.fasta',
          'apps/aligner/test_data/target.fasta'),
  ],
)

pkg = import('pkgconfig')
pkg.generate(edlib_lib,
  description : 'Lightweight and super fast C/C++ library for sequence alignment using edit distance',
)
