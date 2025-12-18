std_c_src = [
  'src/Std.c',

  'src/AddMethods.c',
  'src/String.c',
  'src/Object.c',
]

libObStd = static_library(
  'ob_std',
  std_c_src,
  include_directories: 'include',
  dependencies: [obCore]
)

obStd = declare_dependency(
  link_with: libObStd,
  include_directories: 'include'
)
