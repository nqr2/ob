_ob_shell = obc.process('lib/Shell.ob')
_ob_shell_header = xxdi.process(_ob_shell)

std_c_src = [
  'src/Std.c',

  'src/AddMethods.c',
  'src/String.c',
  'src/Object.c',
  'src/Method.c',
  'src/Shell.c',

  _ob_shell_header,
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
