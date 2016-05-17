printf "[qgis_test]\nhost=localhost\ndbname=qgis_test\nuser=postgres" > ~/.pg_service.conf

export PGUSER=postgres
$TRAVIS_BUILD_DIR/tests/testdata/provider/testdata_pg.sh
