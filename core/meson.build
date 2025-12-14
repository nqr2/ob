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

install_headers(
  'include/ob/Allocator.h',
  'include/ob/Argparse.h',
  'include/ob/Array.h',
  'include/ob/Assert.h',
  'include/ob/Bytecode.h',
  'include/ob/Context.h',
  'include/ob/ContextFwd.h',
  'include/ob/Exn.h',
  'include/ob/Exncodes.h',
  'include/ob/Hash.h',
  'include/ob/Interner.h',
  'include/ob/Number.h',
  'include/ob/Object.h',
  'include/ob/Parse.h',
  'include/ob/Serial.h',
  'include/ob/String.h',
  'include/ob/Table.h',
  subdir: 'ob'
)

libOb = static_library(
  'ob',
  ob_src,
  include_directories: 'include',
  install: true
)

obCore = declare_dependency(
  link_with: libOb,
  include_directories: 'include'
)

if get_option('build-tests')
  subdir('tests')
endif
