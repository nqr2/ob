tests = [
  'empty_suite',
]

foreach t : tests
  exe = executable(
    'test_' + t,
    t + '.c',
    dependencies: [tap]
  )

  test(
    t,
    exe,
    protocol: 'tap'
  )
endforeach
