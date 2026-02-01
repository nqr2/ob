#!/usr/bin/env sh

BUILD_DIRECTORY='.build'

cmake --workflow --preset debug

for f in tests/entries/*.ob
do
  echo "Generating snapshots for $f"

  basename="$(basename $f)"
  prefix="tests/entries"

  if [ ! -e $prefix/basename.stdout ]
  then
    $BUILD_DIRECTORY/bin/ob --verbose=0 $f --capture-stdout $prefix/$basename.stdout
  fi

  if [ ! -e $prefix/basename.stderr ]
  then
    $BUILD_DIRECTORY/bin/ob --verbose=0 $f --capture-stderr $prefix/$basename.stderr
  fi
done
