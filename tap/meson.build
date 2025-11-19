tap_src = [
  'src/Tap.c',
]

libTap = static_library(
  'tap',
  tap_src,
  include_directories: 'src'
)

tap = declare_dependency(
  link_with: libTap,
  include_directories: 'src'
)

subdir('tests')
