#!/bin/sh

SCRIPTS="
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_vectorjoin.sql
  tests/testdata/provider/testdata_pg_hstore.sql
  tests/testdata/provider/testdata_pg_array.sql
"

dropdb qgis_test 2> /dev/null || true
createdb qgis_test || exit 1
for f in ${SCRIPTS}; do
  psql -f $f qgis_test --set ON_ERROR_STOP=1 || exit 1
done
