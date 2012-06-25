#!/bin/sh

for i in src/test/*_tests ; do
  if test -f $i ; then
    ./$i
  fi
done
