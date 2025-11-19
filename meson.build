project(
  'ob', 'c',
  default_options: [
    'warning_level=3',
    'c_std=c23',
  ]
)

src = [
  'src/Main.c',
  'src/Allocator.c',
  'src/Object.c',
  'src/Context.c',
  'src/String.c',
  'src/Array.c',
  'src/Interner.c',
  'src/Table.c',
  'src/Hash.c',
]

executable(
  'ob',
  src,
  include_directories: 'src'
)
