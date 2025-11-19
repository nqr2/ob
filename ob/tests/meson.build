tests = [
]

foreach t : tests
  exe = executable(
    'test_ob_' + t,
    t + '.c',
    dependencies: [ob, tap]
  )

  test(
    t,
    exe,
    protocol: 'tap'
  )
endforeach
