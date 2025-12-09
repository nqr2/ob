doxygen = find_program('doxygen', required: false)

if doxygen.found()
  conf = configuration_data()
  conf.set('PROJECT_NAME', meson.project_name())
  conf.set('VERSION', meson.project_version())
  conf.set('OUTPUT_DIR', meson.project_build_root() / 'doc')
  conf.set('SOURCE_DIR', meson.project_source_root())

  doxyfile = configure_file(
    input: meson.project_source_root() / '.Doxyfile',
    output: 'Doxyfile',
    configuration: conf,
    install: false
  )

  run_target(
    'docs',
     command: [doxygen, doxyfile]
  )
endif
