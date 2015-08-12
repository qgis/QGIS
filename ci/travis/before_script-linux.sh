printf "[qgis_test]\nhost=localhost\ndbname=qgis_test\nuser=postgres" > ~/.pg_service.conf
psql -c 'CREATE DATABASE qgis_test;' -U postgres
psql -c 'CREATE EXTENSION postgis;' -U postgres -d qgis_test
psql -f $TRAVIS_BUILD_DIR/tests/testdata/provider/testdata.sql -U postgres -d qgis_test

