#!/bin/sh

DB=${1:-qgis_test}

cd "$(dirname "$0")"/../../../ || exit 1

# IMPORTANT: order matters, don't just sort the files
SCRIPTS="
  tests/testdata/provider/testdata_pg_role.sql
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_relations.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_vectorjoin.sql
  tests/testdata/provider/testdata_pg_hstore.sql
  tests/testdata/provider/testdata_pg_array.sql
  tests/testdata/provider/testdata_pg_raster.sql
  tests/testdata/provider/testdata_pg_topology.sql
  tests/testdata/provider/testdata_pg_domain.sql
  tests/testdata/provider/testdata_pg_json.sql
  tests/testdata/provider/testdata_pg_pointcloud.sql
  tests/testdata/provider/testdata_pg_bigint_pk.sql
  tests/testdata/provider/testdata_pg_hasspatialindex.sql
  tests/testdata/provider/testdata_pg_geography.sql
"

SCRIPTS12="
  tests/testdata/provider/testdata_pg_12_generated.sql
"

echo "Dropping DB $DB"
dropdb --if-exists "${DB}" || exit 1
echo "Creating DB $DB"
createdb "${DB}" -E UTF8 -T template0 || exit 1

export PGDATABASE="${DB}"

for f in ${SCRIPTS}; do
  echo "Loading $f"
  psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f "${f}" -v ON_ERROR_STOP=1 || exit 1
done

PGSERVERVERSION=$(psql -XtA -c 'SHOW server_version_num')
if test "${PGSERVERVERSION}" -gt 120000; then
  for f in ${SCRIPTS12}; do
    echo "Loading $f"
    psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f "${f}" -v ON_ERROR_STOP=1 || exit 1
  done
fi

FINGERPRINT="${DB}-$(date +%s)"
echo "Storing fingerprint '${FINGERPRINT}' in $DB"

psql -q --echo-errors -v ON_ERROR_STOP=1 <<EOF || exit 1
CREATE TABLE qgis_test.schema_info AS SELECT
  'fingerprint' var, '${FINGERPRINT}' val;
EOF

# Test service=qgis_test connects to the just-created database
echo "Checking if we can connect to ${DB} via service=qgis_test"
CHECK=$(psql -wXtA 'service=qgis_test' -c "select val from qgis_test.schema_info where var='fingerprint'")
if test "${CHECK}" != "${FINGERPRINT}"; then
  exec >&2
  echo "ERROR: Could not access the just created test database ${DB} via service=qgis_test"
  echo "HINT: setup a section like the following in ~/.pg_service.conf"
  cat <<EOF
[qgis_test]
  host=localhost
  port=5432
  dbname=${DB}
  user=qgis_test_user
  password=qgis_test_user_password
EOF
  exit 1
fi

# Test service=qgis_test connects via a method which accepts
# username/password
CHECK=$(psql -wXtA 'service=qgis_test user=qgis_test_unprivileged_user password=qgis_test_unprivileged_user_password' -c "select version()")
if test -z "${CHECK}"; then
  exec >&2
  echo "ERROR: Could not access the just created test database ${DB} via service=qgis_test and overriding username/password"
  echo "HINT: make sure password based methods ( scram-sha-256, md4 ) are accepted in pg_hba.conf"
  echo "      for the kind of connection used by the [qgis_test] section of ~/.pg_service.conf"
  echo "      Specifying host=localhost often helps."
  exit 1
fi


echo "Database ${DB} populated and ready for use with 'service=qgis_test'"
