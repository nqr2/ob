#!/usr/bin/env sh

BUILD_DIRECTORY='.build'

if [ ! -e $BUILD_DIRECTORY/bin/ob ]
then
  cmake --workflow --preset debug
fi

for f in tests/entries/*.ob
do
  echo "Generating snapshots for $f"

  basename="$(basename -s .ob $f)"

  if [ ! -e tests/stdout/$basename ]
  then
    echo "Capturing stdout..."
    $BUILD_DIRECTORY/bin/ob --verbose=0 $f --capture-stdout tests/stdout/$basename
  fi

  if [ ! -e tests/stderr/$basename ]
  then
    echo "Capturing stderr..."
    $BUILD_DIRECTORY/bin/ob --verbose=0 $f --capture-stderr tests/stderr/$basename
  fi
done
