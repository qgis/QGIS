QGIS unit tests
===============

Build tests
-----------

Make sure that you have enabled building of tests in CMake.
`cmake -DENABLE_TESTS=ON ..`

Run tests
---------

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

Advanced configuration
----------------------

### Postgres

Make sure that you have enabled building of postgres test in CMake.
`cmake -DENABLE_PGTEST=ON ..`

To test the postgres provider you will need to have a database available to
which the postgres provider can connect. The server will need to have postgis
support enabled.
By default the test uses the following connection options:
    dbname='qgis_test'
    host=localhost
    port=5432
    user='postgres'
    password='postgres'

If this does not match your setup you can set the environment variable
QGIS_PGTEST_DB to the desired connection string.

Please note that the database needs to be initialized using the sql-script
    tests/testdata/provider/testdata.sql
It takes care of activating postgis for the test database and
creates some tables containing test data.