project(
  'edlib',
  'cpp', 'c',
  version : '1.2.6',
  default_options : [
    'buildtype=release',
    'warning_level=3',
    'cpp_std=c++14',
    'b_ndebug=if-release',
  ],
  license : 'MIT',
  meson_version : '>= 0.52.0',
)

edlib_lib_static = static_library('edlib',
  sources : files(['edlib/src/edlib.cpp']),
  include_directories : ['edlib/include'],
  dependencies : [],
  install : true,
)
edlib_lib_shared = shared_library('edlib',
  sources : files(['edlib/src/edlib.cpp']),
  include_directories : ['edlib/include'],
  dependencies : [],
  install : true,
  cpp_args: ['-DEDLIB_SHARED']
)

install_headers('edlib/include/edlib.h')

edlib_dep_static = declare_dependency(
  include_directories : include_directories('edlib/include'),
  link_with : edlib_lib_static
)
edlib_dep_shared = declare_dependency(
  include_directories : include_directories('edlib/include'),
  compile_args : ['-DEDLIB_SHARED'],
  link_with : edlib_lib_shared
)

hello_main = executable(
  'hello-world',
  files(['apps/hello-world/helloWorld.c']),
  dependencies : [edlib_dep_static],
)

if build_machine.system() != 'windows'
  aligner_main = executable(
    'edlib-aligner',
    files(['apps/aligner/aligner.cpp']),
    dependencies : [edlib_dep_static],
    install : true,
  )
endif

runTests_main = executable(
  'runTests',
  files(['test/runTests.cpp']),
  dependencies : [edlib_dep_shared],
  include_directories : include_directories('test'),
)

test('runTests', runTests_main)

test('hello', hello_main)

if build_machine.system() != 'windows'
  test('aligner', aligner_main,
    args : [
      files('apps/aligner/test_data/query.fasta',
            'apps/aligner/test_data/target.fasta'),
    ],
  )
endif

pkg = import('pkgconfig')
pkg.generate(edlib_lib_static,
  description : 'Lightweight and super fast C/C++ library for sequence alignment using edit distance',
)
