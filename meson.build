project(
  'ob', 'c',
  default_options: [
    'warning_level=3',
    'c_std=c2y',
  ]
)

if get_option('build-docs')
  subdir('doc')
endif

subdir('tap')
subdir('core')
subdir('tools')
subdir('bin')
