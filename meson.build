project(
  'edlib',
  'cpp', 'c',
  version : '1.2.6',
  default_options : [
    'buildtype=release',
    'warning_level=3',
    'cpp_std=c++14',
    'b_ndebug=if-release',
    'default_library=static'
  ],
  license : 'MIT',
  meson_version : '>= 0.52.0',
)

project_version_major = meson.project_version().split('.')[0]

###### Libraries ######

edlib_lib_compile_args = ['-DEDLIB_BUILD']
if get_option('default_library') != 'static'
  edlib_lib_compile_args += ['-DEDLIB_SHARED']
endif

# If default_library == 'static', library() builds and returns static library.
# If default_library == 'shared', library() builds and returns shared library.
# If default_library == 'both', library() builds both static and shared libraries,
#   but only shared one is returned as value and therefore used in the rest of the build.
if (get_option('default_library') == 'both')
  error('\'both\' as a value for default_library option is not supported because it would'
        + ' build static library with shared library flags, exporting symbols!'
        + 'Instead, build twice, once with \'static\' and once with \'shared\'.')
endif
edlib_lib = library('edlib',
  sources : files(['edlib/src/edlib.cpp']),
  include_directories : include_directories('edlib/include'),
  dependencies : [],
  install : true,
  cpp_args : edlib_lib_compile_args,
  gnu_symbol_visibility : 'inlineshidden',
  soversion : project_version_major # Used only for shared library.
)
edlib_dep = declare_dependency(
  include_directories : include_directories('edlib/include'),
  link_with : edlib_lib,
  compile_args : edlib_lib_compile_args
)

###### Executables ######

hello_main = executable(
  'hello-world',
  files(['apps/hello-world/helloWorld.c']),
  dependencies : [edlib_dep],
)

if build_machine.system() != 'windows'
  aligner_main = executable(
    'edlib-aligner',
    files(['apps/aligner/aligner.cpp']),
    dependencies : [edlib_dep],
    install : true,
  )
endif

runTests_main = executable(
  'runTests',
  files(['test/runTests.cpp']),
  dependencies : [edlib_dep],
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

pkg = import('pkgconfig')
pkg.generate(edlib_lib,
  name: 'edlib',
  url: 'https://github.com/Martinsos/edlib',
  filebase : 'edlib-' + project_version_major,
  extra_cflags : edlib_lib_compile_args,
  description : 'Lightweight and super fast C/C++ library for sequence alignment using edit distance',
)
