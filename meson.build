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

project_version_major = meson.project_version().split('.')[0]

###### Libraries ######

edlib_lib_sources = files(['edlib/src/edlib.cpp'])
edlib_lib_include_dirs = include_directories('edlib/include')

edlib_lib_static = static_library('edlib',
  sources : edlib_lib_sources,
  include_directories : edlib_lib_include_dirs,
  dependencies : [],
  install : true,
)
edlib_dep_static = declare_dependency(
  include_directories : edlib_lib_include_dirs,
  link_with : edlib_lib_static
)

edlib_shared_compile_args = ['-DEDLIB_SHARED']
edlib_lib_shared = shared_library('edlib',
  sources : edlib_lib_sources,
  include_directories : edlib_lib_include_dirs,
  dependencies : [],
  install : true,
  cpp_args : edlib_shared_compile_args,
  gnu_symbol_visibility : 'hidden',
  soversion : project_version_major
)
edlib_dep_shared = declare_dependency(
  include_directories : edlib_lib_include_dirs,
  link_with : edlib_lib_shared,
  compile_args : edlib_shared_compile_args
)

###### Executables ######

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

###### Tests ######

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

###### Install ######

install_headers('edlib/include/edlib.h')

# TODO: Should we also generate another pkg-config for static lib?
pkg = import('pkgconfig')
pkg.generate(edlib_lib_shared,
  name: 'libedlib',
  url: 'https://github.com/Martinsos/edlib',
  filebase : 'edlib-' + project_version_major,
  extra_cflags : edlib_shared_compile_args,
  description : 'Lightweight and super fast C/C++ library for sequence alignment using edit distance',
)
