ob_src = [
  'src/Allocator.c',
  'src/Argparse.c',
  'src/Array.c',
  'src/Assert.c',
  'src/Bytecode.c',
  'src/Context.c',
  'src/Exn.c',
  'src/Hash.c',
  'src/Interner.c',
  'src/Number.c',
  'src/Object.c',
  'src/Parse.c',
  'src/Serial.c',
  'src/String.c',
  'src/Table.c',
]

libOb = static_library(
  'ob',
  ob_src,
  include_directories: 'include'
)

obCore = declare_dependency(
  link_with: libOb,
  include_directories: 'include'
)

if get_option('build-tests')
  subdir('tests')
endif
