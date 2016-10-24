QGIS unit tests
===============

# Build tests

Make sure that you have enabled building of tests in CMake.
`cmake -DENABLE_TESTS=ON ..`

# Run tests

You can run all tests using `make check`.

Individual tests can be run using `ctest`.

For example if the output of `make check` ends like this:

```
   The following tests FAILED:
         77 - PyQgsLocalServer (Failed)
```

You could re-run the failing test with:

```
   ctest -V -R PyQgsLocalServer
```

The parameter `-V` enables verbose mode and `-R` takes a regular expression as
parameter and will only run matching tests.


For python tests, you can run a specific test inside a unit file
with something like this:

```
 QGIS_PREFIX_PATH=output PYTHONPATH=output/python:$PYTHONPATH \
   python ${srcdir}/tests/src/python/test_qgsvectorfilewriter.py
   TestQgsVectorLayer.testOverwriteLayer
```


# Advanced configuration

## Postgres

Make sure that you have enabled building of postgres test in CMake.
`cmake -DENABLE_PGTEST=ON ..`

To test the postgres provider you will need to have a database available to
which the postgres provider can connect. The server will need to have postgis
support enabled.

By default the test uses the following connection string:

    dbname=qgis_test

If this does not match your setup you can set the environment variable
`QGIS_PGTEST_DB` to the desired connection string, or you can rely
on standard libpq environment variables to tweak host, port user and
password (PGHOST, PGPORT, PGUSER, PGPASSWORD).

Please note that the database needs to be initialized using
the sql-scripts:

    tests/testdata/provider/testdata_pg*.sql

They take care of activating postgis for the test database and
create some tables containing test data.

For convenience, a shell script is provided to create the database
and initialize it as needed:

    tests/testdata/provider/testdata_pg.sh

# Write tests

Instructions about writing tests for the processing framework
can be found in a local README file:

    ${TOP_SRCDIR}/python/plugins/processing/tests/README.md

Information about labeling tests design and organization:

    ${TOP_SRCDIR}/tests/testdata/labeling/README.rst

WCS testing information can be found in:

    ${TOP_SRCDIR}/tests/testdata/raster/README.WCS

About benchmark tests you can read:

    ${TOP_SRCDIR}/tests/bench/README
