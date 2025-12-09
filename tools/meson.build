obc = executable(
  'obc',
  'src/obc.c',
  dependencies: [obCore]
)

ob_compiler = generator(
  obc,
  capture: true,
  arguments: ['@INPUT@'],
  output: ['@BASENAME@.obc']
)
