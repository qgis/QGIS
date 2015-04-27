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

To test the postgres provider you will need to have a database available to
which the postgres provider can connect. This will need to have postgis support
enabled and be available as a service called `qgis_test` on the machine you run
the tests on.
