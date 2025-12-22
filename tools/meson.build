obc_exe = executable(
  'obc',
  'src/obc.c',
  dependencies: [obCore]
)

obc = generator(
  obc_exe,
  capture: true,
  arguments: ['@INPUT@'],
  output: ['@BASENAME@.obc']
)

xxdi_exe = executable(
  'xxdi',
  'src/xxdi.c'
)

xxdi = generator(
  xxdi_exe,
  arguments: ['@INPUT@', '@OUTPUT@', '@BASENAME@'],
  output: ['@BASENAME@.h']
)
