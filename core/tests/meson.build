tests = [
  'allocator',
  'argparse',
  'assert',
  'hash',
  'table',
]

foreach t : tests
  exe = executable(
    'test_ob_' + t,
    t + '.c',
    dependencies: [obCore, tap]
  )

  test(
    t,
    exe,
    protocol: 'tap'
  )
endforeach
