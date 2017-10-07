#!/bin/sh

set -e

echo "SET client_min_messages TO WARNING;" >> ~/.psqlrc

SCRIPTS="
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_vectorjoin.sql
  tests/testdata/provider/testdata_pg_hstore.sql
  tests/testdata/provider/testdata_pg_array.sql
  tests/testdata/provider/testdata_pg_raster.sql
"

dropdb qgis_test 2> /dev/null || true
createdb qgis_test -E UTF8 -T template0 || exit 1
for f in ${SCRIPTS}; do
  psql -q --echo-errors -f $f qgis_test -v ON_ERROR_STOP=1
done
