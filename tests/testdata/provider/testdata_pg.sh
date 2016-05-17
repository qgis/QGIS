#!/bin/sh

SCRIPTS="
  tests/testdata/provider/testdata_pg.sql
  tests/testdata/provider/testdata_pg_reltests.sql
  tests/testdata/provider/testdata_pg_vectorjoin.sql
"

createdb qgis_test || exit 1
for f in ${SCRIPTS}; do
  psql -f $f qgis_test --set ON_ERROR_STOP=1 || exit 1
done
