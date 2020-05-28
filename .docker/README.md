QGIS Docker images
==================

The QGIS project provides a few official docker images that can be
used for testing purposes.

These dockers are currently used to run continuous integration
tests for the QGIS project itself and to run continuous integration
tests for several third party Python plugins.

The images are automatically built every day and pushed on docker hub
to the QGIS account: https://hub.docker.com/r/qgis/

# Available images

## Dependencies image

`qgis/qgis3-build-deps`

This is a simple base image that contains all the dependencies required to build
QGIS, it is used by the other images.

Multiple versions of this image may be available: the suffix in the image name indicates the Ubuntu version they are based on.

## Main QGIS image

`qgis/qgis`

This is the main image containing a build of QGIS.

The docker tags for this image are assigned for each point release (prefixed with `final-`), for the active development branches (prefixed with `release-`) while the `latest` tag refers to a build of the current master branch.


### Features

The docker file builds QGIS from the current directory and
sets up a testing environment suitable for running tests
inside QGIS.

You can use this docker image to test QGIS and/or to run unit tests inside
QGIS, `xvfb` (A fake X server) is available and running as a service inside
the container to allow for fully automated headless testing in CI pipelines
such as Travis or Circle-CI.

### Building

You can build the image from the main directory of the QGIS source tree with:

```
$ docker build -t qgis/qgis:latest \
    --build-arg DOCKER_TAG=latest \
    -f .docker/qgis.dockerfile \
    .
```

The `DOCKER_TAG` argument, can be used to specify the tag of the dependencies image.


### Running QGIS

You can also use this image to run QGIS on your desktop.

To run a QGIS container, assuming that you want to use your current
display to use QGIS and the image is tagged `qgis/qgis:latest` you can use a script like the one here below:

```bash
# Allow connections from any host
$ xhost +
$ docker run --rm -it --name qgis \
    -v /tmp/.X11-unix:/tmp/.X11-unix  \
    -e DISPLAY=unix$DISPLAY \
    qgis/qgis:latest qgis
```

This code snippet will launch QGIS inside a container and display the
application on your screen.

### Running unit tests inside QGIS

Suppose that you have local directory containing the tests you want to execute into QGIS:

```
/my_tests/travis_tests/
├── __init__.py
└── test_TravisTest.py
```

To run the tests inside the container, you must mount the directory that
contains the tests (e.g. your local directory `/my_tests`) into a volume
that is accessible by the container, see `-v /my_tests/:/tests_directory`
in the example below:

```bash
$ docker run -d --name qgis -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /my_tests/:/tests_directory \
    -e DISPLAY=:99 \
    qgis/qgis:latest
```

Here is an extract of `test_TravisTest.py`:

```python
# -*- coding: utf-8 -*-
import sys
from qgis.testing import unittest

class TestTest(unittest.TestCase):

    def test_passes(self):
        self.assertTrue(True)

def run_all():
    """Default function that is called by the runner if nothing else is specified"""
    suite = unittest.TestSuite()
    suite.addTests(unittest.makeSuite(TestTest, 'test'))
    unittest.TextTestRunner(verbosity=3, stream=sys.stdout).run(suite)

```

When done, you can invoke the test runnner by specifying the test
that you want to run, for instance:

```
$ docker exec -it qgis sh -c "cd /tests_directory && qgis_testrunner.sh travis_tests.test_TravisTest.run_fail"

```

The test can be specified by using a dotted notation, similar to Python
import notation, by default the function named `run_all` will be executed
but you can pass another function name as the last item in the dotted syntax:

```bash
# Call the default function "run_all" inside test_TravisTest module
qgis_testrunner.sh travis_tests.test_TravisTest
# Call the function "run_fail" inside test_TravisTest module
qgis_testrunner.sh travis_tests.test_TravisTest.run_fail
```

Please note that in order to make the test script accessible to Python
the calling command must ensure that the tests are in Python path.
Common patterns are:
- change directory to the one containing the tests (like in the examples above)
- add to `PYTHONPATH` the directory containing the tests

#### Running tests for a Python plugin

All the above considerations applies to this case too, however in order
to simulate the installation of the plugin inside QGIS, you'll need to
make an additional step: call `qgis_setup.sh <YourPluginName>` in the
docker container before actually running the tests (see the paragraph
about Running on Travis for a complete example).

The `qgis_setup.sh` script prepares QGIS to run in headless mode and
simulate the plugin installation process:

- creates the QGIS profile folders
- "installs" the plugin by making a symbolic link from the profiles folder to the plugin folder
- installs `startup.py` monkey patches to prevent blocking dialogs
- enables the plugin

Please note that depending on your plugin repository internal directory structure
you may need to adjust (remove and create) the symbolic link created by `qgis_setup.sh`,
this is required in particular if the real plugin code in your repository is contained
in the main directory and not in a subdirectory with the same name of the plugin
internal name (the name in `metadata.txt`).

#### Options for the test runner

The env var `QGIS_EXTRA_OPTIONS` defaults to an empty string and can
contains extra parameters that are passed to QGIS by the test runner.


#### Running on Travis

Here is a simple example for running unit tests of a small QGIS plugin (named *QuickWKT*), assuming that the tests are in `tests/test_Plugin.py` under
the main directory of the QuickWKT plugin:

```yml
services:
    - docker
install:
    - docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}:/tests_directory -e DISPLAY=:99 qgis/qgis:latest
    - sleep 10 # This is required to allow xvfb to start
    # Setup qgis and enables the plugin
    - docker exec -it qgis-testing-environment sh -c "qgis_setup.sh QuickWKT"
    # Additional steps (for example make or paver setup) here
    # Fix the symlink created by qgis_setup.sh
    - docker exec  -it qgis-testing-environment sh -c "rm -f  /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT"
    - docker exec -it qgis-testing-environment sh -c "ln -s /tests_directory/ /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT"

script:
    - docker exec -it qgis-testing-environment sh -c "cd /tests_directory && qgis_testrunner.sh tests.test_Plugin"
```

Please note that `cd /tests_directory && ` before the call to `qgis_testrunner.sh` could be avoided here, because QGIS automatically
adds the plugin main directory to Python path.


#### Running on Circle-CI

Here is an example for running unit tests of a small QGIS plugin (named *QuickWKT*), assuming
that the tests are in `tests/test_Plugin.py` under the main directory of the QuickWKT plugin:

```yml
version: 2
jobs:
  build:
    docker:
      - image: qgis/qgis:latest
        environment:
          DISPLAY: ":99"
    working_directory: /tests_directory
    steps:
      - checkout
      - run:
          name: Setup plugin
          command: |
            qgis_setup.sh QuickWKT
      - run:
          name: Fix installation path created by qgis_setup.s
          command: |
            rm -f  /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT
            ln -s /tests_directory/ /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/qgisce
      - run:
          name: run tests
          command: |
            sh -c "/usr/bin/Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset -nolisten tcp &"
            qgis_testrunner.sh tests.test_Plugin
```


#### Implementation notes

The main goal of the test runner in this image is to execute unit tests
inside a real instance of QGIS (not a mocked one).

The QGIS tests should be runnable from a Travis/Circle-CI CI job.

The implementation is:

- run the docker, mounting as volumes the unit tests folder in `/tests_directory`
    (or the QGIS plugin folder if the unit tests belong to a plugin and the
    plugin is needed to run the tests)
- execute `qgis_setup.sh MyPluginName` script in docker that sets up QGIS to
  avoid blocking modal dialogs  and installs the plugin into QGIS if needed
    - create config and python plugin folders for QGIS
    - enable the plugin  in the QGIS configuration file
    - install the `startup.py` script to disable python exception modal dialogs
- execute the tests by running `qgis_testrunner.sh MyPluginName.tests.tests_MyTestModule.run_my_tests_function`
- the output of the tests is captured by the `test_runner.sh` script and
  searched for `FAILED` (that is in the standard unit tests output) and other
  string that indicate a failure or success condition, if a failure condition
  is identified, the script exits with `1` otherwise it exits with `0`.

`qgis_testrunner.sh` accepts a dotted notation path to the test module that
can end with the function that has to be called inside the module to run the
tests. The last part (`.run_my_tests_function`) can be omitted and defaults to
`run_all`.
