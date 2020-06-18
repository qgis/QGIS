#!/usr/bin/env bash

set -e

# Temporarily uncomment to debug ccache issues
# echo "travis_fold:start:ccache-debug"
# cat /tmp/cache.debug
# echo "travis_fold:end:ccache-debug"

############################
# Restore postgres test data
############################
echo "travis_fold:start:postgres"
echo "${bold}Load Postgres database...🐘${endbold}"

printf "[qgis_test]\nhost=postgres\nport=5432\ndbname=qgis_test\nuser=docker\npassword=docker" > ~/.pg_service.conf
export PGUSER=docker
export PGHOST=postgres
export PGPASSWORD=docker
export PGDATABASE=qgis_test

# wait for the DB to be available
echo "Wait a moment while loading PostGreSQL database."
while ! PGPASSWORD='docker' psql -h postgres -U docker -p 5432 -l &> /dev/null
do
  printf "🐘"
  sleep 1
done
echo " done 🥩"

pushd /root/QGIS > /dev/null
echo "Restoring postgres test data ..."
/root/QGIS/tests/testdata/provider/testdata_pg.sh
echo "Postgres test data restored ..."
popd > /dev/null # /root/QGIS
echo "travis_fold:end:postgres"

##############################
# Restore Oracle test data
##############################

echo "travis_fold:start:oracle"
echo "${bold}Load Oracle database...🙏${endbold}"

export ORACLE_HOST="oracle"
export QGIS_ORACLETEST_DBNAME="${ORACLE_HOST}/XEPDB1"
export QGIS_ORACLETEST_DB="host=${QGIS_ORACLETEST_DBNAME} port=1521 user='QGIS' password='qgis'"

echo "Wait a moment while loading Oracle database."
while ! echo exit | sqlplus -L SYSTEM/adminpass@$QGIS_ORACLETEST_DBNAME &> /dev/null
do
  printf "🙏"
  sleep 1
done
echo " done 👀"

pushd /root/QGIS > /dev/null
/root/QGIS/tests/testdata/provider/testdata_oracle.sh $ORACLE_HOST
popd > /dev/null # /root/QGIS
echo "travis_fold:end:oracle"

# this is proving very flaky:

##############################
# Restore SQL Server test data
##############################

# echo "Importing SQL Server test data..."

# export SQLUSER=sa
# export SQLHOST=mssql
# export SQLPORT=1433
# export SQLPASSWORD='<YourStrong!Passw0rd>'
# export SQLDATABASE=qgis_test

# export PATH=$PATH:/opt/mssql-tools/bin

# pushd /root/QGIS > /dev/null
# /root/QGIS/tests/testdata/provider/testdata_mssql.sh
# popd > /dev/null # /root/QGIS

# echo "Setting up DSN for test SQL Server"

# cat <<EOT > /etc/odbc.ini
# [ODBC Data Sources]
# testsqlserver = ODBC Driver 17 for SQL Server

# [testsqlserver]
# Driver       = ODBC Driver 17 for SQL Server
# Description  = Test SQL Server
# Server       = mssql
# EOT

###########
# Run tests
###########
CURRENT_TIME=$(date +%s)
TIMEOUT=$((( TRAVIS_AVAILABLE_TIME - TRAVIS_UPLOAD_TIME) * 60 - CURRENT_TIME + TRAVIS_TIMESTAMP))
echo "Timeout: ${TIMEOUT}s (started at ${TRAVIS_TIMESTAMP}, current: ${CURRENT_TIME})"
EXCLUDE_TESTS=$(cat /root/QGIS/.ci/travis/linux/scripts/test_blocklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)
if ! [[ ${RUN_FLAKY_TESTS} =~ ^true$ ]]; then
  echo "Flaky tests are skipped!"
  EXCLUDE_TESTS=${EXCLUDE_TESTS}"|"$(cat /root/QGIS/.ci/travis/linux/scripts/test_flaky.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)
else
  echo "Flaky tests are run!"
fi
echo "List of skipped tests: $EXCLUDE_TESTS"
timeout ${TIMEOUT}s python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -E "${EXCLUDE_TESTS}" -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure
rv=$?
if [ $rv -eq 124 ] ; then
    printf '\n\n${bold}Build and test timeout. Please restart the build for meaningful results.${endbold}\n'
    exit #$rv
fi
