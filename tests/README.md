QGIS unit tests
===============

# Build tests

Make sure that you have enabled building of tests in CMake.
`cmake -DENABLE_TESTS=ON ..`

# Run tests

You can run all tests using `make check`.
Note you will need `xvfb-run` for that (sudo apt-get install xvfb).

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

If you get `Could not connect to any X display` errors it means that your build
machine does not have an X server.  In that case you need to run the test under
`xvfb-run`.  For example:

```
    xvfb-run --server-args=-screen\ 0\ 1024x768x24 ctest -V -R PyQgsServerWMSGetMap
```

# Advanced configuration

## Postgres

Make sure that you have enabled building of postgres test in CMake.
`cmake -DENABLE_TESTS=ON -DENABLE_PGTEST=ON ..`

To test the postgres provider you will need to have a database available to
which the postgres provider can connect. The server will need to have PostGIS
support enabled.

By default the tests use the following connection string:

    service=qgis_test

If these do not match your setup you can set the environment variable
`QGIS_PGTEST_DB` to the desired connection string. Note that you can
rely on standard libpq environment variables to tweak host, port user
and password (PGHOST, PGPORT, PGUSER, PGPASSWORD).

Please note that the test database needs to be initialized using
the sql-scripts:

    tests/testdata/provider/testdata_pg*.sql

They take care of activating PostGIS for the test database and
create some tables containing test data.

For convenience, a shell script is provided to create the database
and initialize it as needed:

    tests/testdata/provider/testdata_pg.sh

# Write tests

Instructions about writing tests for the processing framework
can be found in a [separate README file](../python/plugins/processing/tests/README.md):

    ${TOP_SRCDIR}/python/plugins/processing/tests/README.md

Information about labeling tests design and organization:

    ${TOP_SRCDIR}/tests/testdata/labeling/README.rst

WCS testing information can be found in:

    ${TOP_SRCDIR}/tests/testdata/raster/README.WCS

About benchmark tests you can read:

    ${TOP_SRCDIR}/tests/bench/README


# Run python tests in GDB

First find out the required environment variables by running the test outside
the debugger.

    ctest -V -R ProcessingQgisAlgorithmsTest

Which prints for somewhere in the initialization code something like:

    export LD_LIBRARY_PATH=NOTFOUND:/home/m-kuhn/dev/cpp/qgis/build-qt5/output/lib:
    export PYTHONPATH=/home/m-kuhn/dev/cpp/qgis/build-qt5/output/python/:/home/m-kuhn/dev/cpp/qgis/build-qt5/output/python/plugins:/home/m-kuhn/dev/cpp/qgis/QGIS/tests/src/python:

First, run these two commands in the terminal.

On the following line it says something like:

    -- Running /usr/bin/python3 /home/m-kuhn/dev/cpp/qgis/QGIS/python/plugins/processing/tests/QgisAlgorithmsTest.py

Which you can run in gdb with:

    gdb -ex r --args /usr/bin/python3 /home/m-kuhn/dev/cpp/qgis/QGIS/python/plugins/processing/tests/QgisAlgorithmsTest.py

Now you can start using the usual gdb (`bt` etc.) interface or - if you have
installed the [appropriate debug tools (adjust for python3!)](https://wiki.python.org/moin/DebuggingWithGdb)
even allows doing python introspection (`py-bt`).
