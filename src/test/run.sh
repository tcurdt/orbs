#!/bin/sh

for i in src/test/*_tests ; do
  if test -f $i ; then
    if ./$i 2>&1 > /tmp/test.log ; then
      echo " PASS `basename $i`"
    else
      echo " ERROR `basename $i`:"
      cat /tmp/test.log
      exit 1
    fi
  fi
done

rm -f /tmp/test.log
