ob_src = [
  'src/Allocator.c',
  'src/Object.c',
  'src/Context.c',
  'src/String.c',
  'src/Array.c',
  'src/Interner.c',
  'src/Table.c',
  'src/Hash.c',
  'src/Bytecode.c',
  'src/Assert.c',
]

libOb = static_library(
  'ob',
  ob_src,
  include_directories: 'include'
)

ob = declare_dependency(
  link_with: libOb,
  include_directories: 'include'
)

subdir('tests')
