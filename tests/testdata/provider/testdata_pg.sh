#!/bin/sh

set -e

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
  tests/testdata/provider/testdata_pg_pointcloud.sql
"

dropdb --if-exists $DB
createdb $DB -E UTF8 -T template0
for f in ${SCRIPTS}; do
  psql -q --echo-errors -c "SET client_min_messages TO WARNING;" -f $f $DB -v ON_ERROR_STOP=1
done
