project(
  'ob', 'c',
  version: '0',
  default_options: [
    'warning_level=3',
    'c_std=c2y',
  ]
)

if get_option('build-docs')
  subdir('doc')
endif

if get_option('build-tests')
  subdir('tap')
endif

subdir('core')
subdir('tools')
subdir('std')
subdir('bin')
