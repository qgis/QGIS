#!/bin/sh

DB=${DB:-qgis_test}

SCRIPTS="
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_role.sql
  tests/testdata/provider/testdata_pg_vectorjoin.sql
  tests/testdata/provider/testdata_pg_hstore.sql
  tests/testdata/provider/testdata_pg_array.sql
  tests/testdata/provider/testdata_pg_raster.sql
  tests/testdata/provider/testdata_pg_topology.sql
  tests/testdata/provider/testdata_pg_domain.sql
  tests/testdata/provider/testdata_pg_json.sql
"

dropdb --if-exists $DB
createdb $DB -E UTF8 -T template0 || exit 1
for f in ${SCRIPTS}; do
  psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f $f $DB -v ON_ERROR_STOP=1 || exit 1
done

# Test existence of qgis_test service, and recommend how to set it up
# otherwise
TESTDB=$(psql -XtA 'service=qgis_test' -c "select current_database()")
if test "${TESTDB}" != "${DB}"; then
  exec >&2
  if test -n "${TESTDB}"; then
    echo "WARNING: [qgis_test] service section points to db '${TESTDB}'" \
         "but we populated db '${DB}' instead"
  else
    echo "ERROR: [qgis_test] service not found in ~/.pg_service.conf"
    echo "HINT: create a section like the following:"
    cat <<EOF
  [qgis_test]
  host=localhost
  port=5432
  dbname=${DB}
  user=USERNAME
  password=PASSWORD
EOF
  fi
else
  echo "Database ${DB} populated and ready for use with 'service=qgis_test'"
fi
