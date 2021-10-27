#!/bin/sh

DB=${1:-qgis_test}

cd "$(dirname "$0")"/../../../ || exit 1

SCRIPTS="
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_relations.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_role.sql
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
dropdb --if-exists "${DB}"
echo "Creating DB $DB"
# TODO: use service=qgis_test to connect to "template1" and use SQL ?
createdb "${DB}" -E UTF8 -T template0 || exit 1
for f in ${SCRIPTS}; do
  echo "Loading $f"
  psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f "${f}" "${DB}" -v ON_ERROR_STOP=1 || exit 1
done

PGSERVERVERSION=$(psql -XtA -c 'SHOW server_version_num' "${DB}")
if test "${PGSERVERVERSION}" -gt 120000; then
  for f in ${SCRIPTS12}; do
    echo "Loading $f"
    psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f "${f}" "${DB}" -v ON_ERROR_STOP=1 || exit 1
  done
fi

FINGERPRINT="${DB}-$(date +%s)"
echo "Storing fingerprint ${FINGERPRINT} in $DB"
psql -q --echo-errors "${DB}" -v ON_ERROR_STOP=1 <<EOF || exit 1
CREATE TABLE qgis_test.schema_info AS SELECT
  'fingerprint' var, '${FINGERPRINT}' val;
EOF

# Test service=qgis_test connects to the just-created database
CHECK=$(psql -XtA 'service=qgis_test' -c "select val from qgis_test.schema_info where var='fingerprint'")
if test "${CHECK}" != "${FINGERPRINT}"; then
  exec >&2
  echo "ERROR: Could not access the just created test database ${DB} via service=qgis_test"
  echo "HINT: create a section like the following in ~/.pg_service.conf"
  cat <<EOF
[qgis_test]
  host=localhost
  port=5432
  dbname=${DB}
  user=USERNAME
  password=PASSWORD
EOF
  exit 1
fi

# TODO: Test service=qgis_test connects via a method which accepts
# username/password
CHECK=$(psql -XtA 'service=qgis_test user=qgis_test_user password=qgis_test_user_password' -c "select version()")
if test -z "${CHECK}"; then
  exec >&2
  echo "ERROR: Cannot service=qgis_test via username/password"
  echo "HINT: make sure MD5 method is accepted in pg_hba.conf "
  echo "(specifying host=localhost in the [qgis_test] section of "
  echo "'~/.pg_service.conf' often does help)"
  exit 1
fi


echo "Database ${DB} populated and ready for use with 'service=qgis_test'"
