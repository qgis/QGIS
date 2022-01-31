QGIS unit tests
===============

# Building tests

Make sure that you have enabled building of tests in CMake.
`cmake -DENABLE_TESTS=ON ..`

# Setting up the test environment

Some tests require setting up a test environment.
Below some information about how to set such environment up.

## Postgres

Make sure that you have enabled building of postgres test in CMake.
`cmake -DENABLE_TESTS=ON -DENABLE_PGTEST=ON ..`

To test the postgres provider you will need to setup a test database
to which the postgres provider can connect.

By default the tests use the following connection string to access
the test database:

    service=qgis_test

If these do not match your setup you can set the environment variable
`QGIS_PGTEST_DB` to the desired connection string. Note that you can
rely on standard libpq environment variables to tweak host, port user
and password (PGHOST, PGPORT, PGUSER, PGPASSWORD).

The test database must be initialized using the sql-scripts:

    tests/testdata/provider/testdata_pg*.sql

They take care of activating PostGIS and creating some tables containing
test data.

For convenience, a shell script is provided to create the database
and initialize it as needed:

    tests/testdata/provider/testdata_pg.sh

Some tests will attempt to create database users too and expect them to
be allowed to connect using a password.
Most PostgreSQL installation by default use "ident" authentication
model when connecting via unix sockets. Make sure the `qgis_test`
service (or your `QGIS_PGTEST_DB` connection string) uses a connection
method which allows passing username/password pair to connect (and
which allows creating users)

Some tests require a specific PostgreSQL server configuration
(installation of auth certificates) so the most convenient way
to bring up such server would be to (tweak $srcdir appropriately):

    QGIS_WORKSPACE=${srcdir} \
    docker-compose -f .docker/docker-compose-testing-postgres.yml up -d postgres
    export PGHOST=`docker inspect docker_postgres_1 | jq -r .[0].NetworkSettings.Networks.docker_default.IPAddress`
    export PGUSER=docker
    export PGPASSWORD=docker
    tests/testdata/provider/testdata_pg.sh
    echo "${PGHOST} postgres # for qgis_test, docker" | sudo tee -a /etc/hosts


# Running the tests

Once the test environment is setup, you can run all tests using `make check`.
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

Note that the name of the test as reported by `make check` does not
always matches the name of the test as known by `cmake`.
When this happens, the `ctest -R <guessedName>` command will report
`No tests were found!!!` and you'll need to go hunting for the actual name.
A useful hunting tool is `git grep`, which could be used like this:

```
  $ ctest -V -R testSslRequireNoCaCheck
  No tests were found !!!
  $ git grep testSslRequireNoCaCheck # Search for the reported testname
  tests/src/python/test_authmanager_password_postgres.py:    def testSslRequireNoCaCheck(self):
  $ git grep test_authmanager_password_postgres # Search for the file containing the reported testname
  tests/src/python/CMakeLists.txt: ADD_PYTHON_TEST(PyQgsAuthManagerPasswordPostgresTest test_authmanager_password_postgres.py)
  $ ctest -V -R PyQgsAuthManagerPasswordPostgresTest # use the CMakeLists.txt name
```

If you get `Could not connect to any X display` errors it means that your build
machine does not have an X server.  In that case you need to run the test under
`xvfb-run`.  For example:

```
    xvfb-run --server-args=-screen\ 0\ 1024x768x24 ctest -V -R PyQgsServerWMSGetMap
```

## Manually running python tests

For python tests, you can run a specific test inside a unit file
with something like this (tweak $builddir and $srcdir as appropriate)

```
 QGIS_PREFIX_PATH=${builddir}/output PYTHONPATH=${builddir}/output/python:$PYTHONPATH \
   python ${srcdir}/tests/src/python/test_qgsvectorfilewriter.py
   TestQgsVectorLayer.testOverwriteLayer
```

#### Running python tests in GDB

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

# Writing tests

Instructions about writing tests for the processing framework
can be found in a [separate README file](../python/plugins/processing/tests/README.md):

    ${TOP_SRCDIR}/python/plugins/processing/tests/README.md

Information about labeling tests design and organization:

    ${TOP_SRCDIR}/tests/testdata/labeling/README.rst

WCS testing information can be found in:

    ${TOP_SRCDIR}/tests/testdata/raster/README.WCS

About benchmark tests you can read:

    ${TOP_SRCDIR}/tests/bench/README


