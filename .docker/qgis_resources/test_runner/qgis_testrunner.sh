#!/bin/bash
# Run a python test inside QGIS
# Note: the test module and function are specified in dotted notation
# for example, to run the test function run_all (which is the default anyway)
# $ qgis_testrunner.sh tests_folder.test_module.run_all
# tests_folder must be in PYTHONPATH (plugins main folders are automatically added to path)

### Turn on debug mode ###
#set -x

TEST_NAME=$1

echo "Running test $1 ..."
OUTPUT=$(QGIS_TEST_MODULE=${TEST_NAME} unbuffer qgis --version-migration --nologo --code /usr/bin/qgis_testrunner.py "$TEST_NAME"  2>/dev/null | tee /dev/tty)
EXIT_CODE="$?"
if [ -z "$OUTPUT" ]; then
    echo "ERROR: no output from the test runner! (exit code: ${EXIT_CODE})"
    exit 1
fi
echo "$OUTPUT" | grep -q 'FAILED'
IS_FAILED="$?"
echo "$OUTPUT" | grep -q 'OK' && echo "$OUTPUT" | grep -q 'Ran'
IS_PASSED="$?"
echo "$OUTPUT" | grep "QGIS died on signal"
IS_DEAD="$?"
echo "Finished running test $1 (codes: IS_DEAD=$IS_DEAD IS_FAILED=$IS_FAILED IS_PASSED=$IS_PASSED)."
if [ "$IS_PASSED" -eq "0" ] && [ "$IS_FAILED" -eq "1" ] && [ "$IS_DEAD" -eq "1" ]; then
    exit 0;
fi
exit 1
