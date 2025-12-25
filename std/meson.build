_std_headers = []

foreach one : [
  'Boolean',
  'Shell',
]
  path = 'lib' / (one + '.ob')

  bc = obc.process(path)
  header = xxdi.process(bc)

  _std_headers += header
endforeach

std_c_src = [
  'src/Std.c',

  'src/AddMethods.c',
  'src/Activation.c',
  'src/String.c',
  'src/Object.c',
  'src/Method.c',
  'src/Slots.c',
  'src/Shell.c',

  'src/World.c',
] + _std_headers

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
