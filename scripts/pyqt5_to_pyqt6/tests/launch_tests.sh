#!/usr/bin/bash


TEST_ROOT_DIR=/tmp/pyqt5_to_pyqt6_tests
rm -Rf $TEST_ROOT_DIR && mkdir $TEST_ROOT_DIR

SCRIPT_DIR=$(dirname "$0")

RES=0
for testfile in $(ls $SCRIPT_DIR/src/*.py);
do
  TESTNAME=$(basename $testfile .py)
  TESTDIR=$TEST_ROOT_DIR/$TESTNAME
  mkdir -p $TESTDIR
  cp $testfile $TESTDIR

  if python $SCRIPT_DIR/../pyqt5_to_pyqt6.py $TESTDIR &> $TESTDIR/output.log; then
    echo "Test '$TESTNAME' failed: expect error code different from 0"
    RES=1
  elif ! diff $SCRIPT_DIR/expected/$(basename $testfile) $TESTDIR/$(basename $testfile) &> $TESTDIR/diff.log; then
    echo "Test '$TESTNAME' failed: There are differences between actual and expected file"
    cat $TESTDIR/diff.log
    RES=1
  fi
done

rm -Rf $TEST_ROOT_DIR

exit $RES
