#!/usr/bin/env bash

set -e

# Temporarily uncomment to debug ccache issues
# cat /tmp/cache.debug

##################################
# Prepare HANA database connection
##################################

if [ ${HANA_TESTS_ENABLED:-"OFF"} == "ON" ] ; then
  echo "::group::hana"
  echo "${bold}Load HANA database...${endbold}"

  export HANA_HOST=917df316-4e01-4a10-be54-eac1b6ab15fb.hana.prod-us10.hanacloud.ondemand.com
  export HANA_PORT=443
  export HANA_USER=QGIS_CI
  export HANA_PASSWORD="3w8dkX:NDrs&"

  export QGIS_HANA_TEST_DB='driver='/usr/sap/hdbclient/libodbcHDB.so' host='${HANA_HOST}' port='${HANA_PORT}' user='${HANA_USER}' password='${HANA_PASSWORD}' sslEnabled=true sslValidateCertificate=False'

  # wait for the DB to be available
  echo "Wait a moment while trying to connect to a HANA database."
  while ! echo exit | hdbsql -n '${HANA_HOST}:${HANA_PORT}' -u '${HANA_USER}' -p '${HANA_PASSWORD}' &> /dev/null
  do
    printf "⚘"
    sleep 1
  done
  echo "🌊 done"

  echo "::endgroup::"
fi

############################
# Restore postgres test data
############################
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

##############################
# Restore Oracle test data
##############################

echo "${bold}Load Oracle database...🙏${endbold}"

export ORACLE_HOST="oracle"
export QGIS_ORACLETEST_DBNAME="${ORACLE_HOST}/XEPDB1"
export QGIS_ORACLETEST_DB="host=${QGIS_ORACLETEST_DBNAME} port=1521 user='QGIS' password='qgis'"

echo "Wait a moment while loading Oracle database."
COUNT=0
while ! echo exit | sqlplus -L SYSTEM/adminpass@$QGIS_ORACLETEST_DBNAME &> /dev/null
do
  printf "🙏"
  sleep 5
  if [[ $(( COUNT++ )) -eq 200 ]]; then
    break
  fi
done
if [[ ${COUNT} -eq 201 ]]; then
  echo "timeout, no oracle, no 🙏"
else
  echo " done 👀"
  pushd /root/QGIS > /dev/null
  /root/QGIS/tests/testdata/provider/testdata_oracle.sh $ORACLE_HOST
  popd > /dev/null # /root/QGIS
fi

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
EXCLUDE_TESTS=$(cat /root/QGIS/.ci/test_blocklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)
if ! [[ ${RUN_FLAKY_TESTS} =~ ^true$ ]]; then
  echo "Flaky tests are skipped!"
  EXCLUDE_TESTS=${EXCLUDE_TESTS}"|"$(cat /root/QGIS/.ci/test_flaky.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)
else
  echo "Flaky tests are run!"
fi
echo "List of skipped tests: $EXCLUDE_TESTS"
python3 /root/QGIS/.ci/ctest2ci.py xvfb-run ctest -V -E "${EXCLUDE_TESTS}" -S /root/QGIS/.ci/config.ctest --output-on-failure

