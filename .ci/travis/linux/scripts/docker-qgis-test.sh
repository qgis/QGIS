#!/usr/bin/env bash

set -e

##############################
# Run Python Tests on QGIS app
##############################
# Passing cases:
echo "QGIS tests runner"
pushd /root/qgis_test_runner
./qgis_testrunner.sh test_testrunner.run_passing && echo "1/5 succeeded" || exit 1
./qgis_testrunner.sh test_testrunner.run_skipped_and_passing && echo "2/5 succeeded"  || exit 1
# Failing cases:
./qgis_testrunner.sh test_testrunner && exit 1 || echo "3/5 succeeded"
./qgis_testrunner.sh test_testrunner.run_all && exit 1 || echo "5/5 succeeded"
./qgis_testrunner.sh test_testrunner.run_failing && exit 1 || echo "5/5 succeeded"
popd


# Temporarily uncomment to debug ccache issues
# echo "travis_fold:start:ccache-debug"
# cat /tmp/cache.debug
# echo "travis_fold:end:ccache-debug"

############################
# Restore postgres test data
############################
printf "[qgis_test]\nhost=postgres\nport=5432\ndbname=qgis_test\nuser=docker\npassword=docker" > ~/.pg_service.conf
export PGUSER=docker
export PGHOST=postgres
export PGPASSWORD=docker
export PGDATABASE=qgis_test

pushd /root/QGIS > /dev/null
/root/QGIS/tests/testdata/provider/testdata_pg.sh
popd > /dev/null # /root/QGIS

##############################
# Restore SQL Server test data
##############################

echo "Importing SQL Server test data..."

export SQLUSER=sa
export SQLHOST=mssql
export SQLPORT=1433
export SQLPASSWORD='<YourStrong!Passw0rd>'
export SQLDATABASE=qgis_test

export PATH=$PATH:/opt/mssql-tools/bin

pushd /root/QGIS > /dev/null
/root/QGIS/tests/testdata/provider/testdata_mssql.sh
popd > /dev/null # /root/QGIS

echo "Setting up DSN for test SQL Server"

cat <<EOT > /etc/odbc.ini
[ODBC Data Sources]
testsqlserver = ODBC Driver 17 for SQL Server

[testsqlserver]
Driver       = ODBC Driver 17 for SQL Server
Description  = Test SQL Server
Server       = mssql
EOT

###########
# Run tests
###########
CURRENT_TIME=$(date +%s)
TIMEOUT=$((( TRAVIS_AVAILABLE_TIME - TRAVIS_UPLOAD_TIME) * 60 - CURRENT_TIME + TRAVIS_AVAILABLE_TIMESTAMP))
echo "Timeout: ${TIMEOUT}s (started at ${TRAVIS_AVAILABLE_TIMESTAMP}, current: ${CURRENT_TIME})"
timeout ${TIMEOUT}s python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -E "$(cat /root/QGIS/.ci/travis/linux/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure
rv=$?
if [ $rv -eq 124 ] ; then
    printf '\n\n${bold}Build and test timeout. Please restart the build for meaningful results.${endbold}\n'
    exit #$rv
fi



