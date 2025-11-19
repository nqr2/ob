tests = [
  'empty_suite',
  'passing_test',
  'failing_test',
  'skipped_test',
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
