#!/bin/sh -x

#Script that prepares a Redshift cluster for testing.
#Before running this script set up the 
#REDSHIFT_DB_HOST, REDSHIFT_DB_PORT, REDSHIFT_DB_USER, REDSHIFT_DB_PASS environment variables 
#so that the script can access the bootstrap user of the Redshfit cluster.

# TODO(marcel): provide Redshift cluster for testing in CI
export REDSHIFT_DB_PORT=5439
export REDSHIFT_DB_BOOTSTRAP_DATABASE=dev
export REDSHIFT_DB_BOOTSTRAP_USER=admin
export REDSHIFT_DB_BOOTSTRAP_PASS=wuZwJYpVKD25ModJ
export REDSHIFT_QGIS_TEST_USER_PASS=$REDSHIFT_DB_BOOTSTRAP_PASS


REDSHIFT_TESTDATA="${SRCDIR}/tests/testdata/provider/testdata_redshift.sql"
echo $REDSHIFT_TESTDATA
#The script will set the QGIS_RSTEST_DB environment variable which is used in tests later on

PREPARE_DB_SQL_SCRIPT="\
DROP DATABASE qgis_test_db; \
DROP USER qgis_test; \
CREATE DATABASE qgis_test_db; \
CREATE USER qgis_test PASSWORD '$REDSHIFT_QGIS_TEST_USER_PASS'; \
GRANT ALL ON DATABASE qgis_test_db TO qgis_test;
"

echo $PREPARE_DB_SQL_SCRIPT|PGPASSWORD=$REDSHIFT_DB_BOOTSTRAP_PASS psql --host="$REDSHIFT_DB_HOST" --port="$REDSHIFT_DB_PORT" --db="$REDSHIFT_DB_BOOTSTRAP_DATABASE" --username="$REDSHIFT_DB_BOOTSTRAP_USER"
PGPASSWORD=$REDSHIFT_QGIS_TEST_USER_PASS psql --host="$REDSHIFT_DB_HOST" --port="$REDSHIFT_DB_PORT" --db="qgis_test_db" --username="qgis_test" -f $REDSHIFT_TESTDATA

# When run inside a container
export QT_QPA_PLATFORM=offscreen

export QGIS_PREFIX_PATH=$PWD/build/output
export LD_LIBRARY_PATH=$QGIS_PREFIX_PATH/lib
export PYTHONPATH=$QGIS_PREFIX_PATH/python/:$QGIS_PREFIX_PATH/build/output/python/plugins:$PWD/tests/src/python
export QGIS_RSTEST_DB="host=$REDSHIFT_DB_HOST port=$REDSHIFT_DB_PORT dbname=qgis_test_db user=qgis_test password=$REDSHIFT_QGIS_TEST_USER_PASS"
