*******************
Labeling Unit Tests
*******************

Design and Organization
=======================

The labeling unit tests are solely written in Python and are organized so that
individual tests are separated from, but inherited by, the output frameworks.
This allows maintaining output-agnostic units, focusing only on the code to be
tested, which the frameworks will use to generate as many tests as necessary,
cross-referencing outputs as needed.

The goal of this design, beyond API and regression testing, is to ensure labels
crafted by users have as close to a WYSIWYG rendering as possible across all
potential outputs and platforms. Exact parity is not achievable; so the test
suite is designed to be flexible enough to maintain a 'best case' scenario.

Modules
-------

test_qgspallabeling_base
    Provides the ``TestQgsPalLabeling`` base class, which is inherited by all
    other test classes. ``TestPALConfig`` tests the configuration of the PAL
    placement engine, and project and map layer settings.

test_qgspallabeling_tests
    Individual unit tests are to be placed here, unless a test *needs* to be
    placed in a specific test subclass. Tests are separated into logical
    groupings for labeling: `single point`, `single line`, `single polygon`,
    `multi-feature`, `placement`. Most label styling tests that are not
    feature-dependent are associated with `single point`.

    Almost all tests produce many images for comparison to controls. To keep
    the proliferation of control images to a minimum, several options can be
    grouped, e.g. SVG background, with buffer, offset and rotation. If such a
    grouping is found to be problematic, it can be separated later.

    Some values for specific, inherited class/function tests can be passed; for
    example, pixel mismatch and color tolerance values for image comparison::

        def test_default_label(self):
            # Default label placement, with text size in points
            self._Mismatches['TestComposerPdfVsComposerPoint'] = 760
            self._ColorTols['TestComposerPdfVsComposerPoint'] = 18
            self.checkTest()

    Values would replace the default values for the module or class, if any, for
    the ``TestComposerPdfVsComposerPoint.test_default_label`` generated test.

test_qgspallabeling_canvas
    ``TestCanvas*`` framework for map canvas output to `image`.

test_qgspallabeling_composer
    ``TestComposer*`` framework for composer map item output to `image`, `SVG`
    and `PDF`. Compares *composition->image* against *canvas->image*, and other
    composer outputs against *composition->image*.

    **Requires:** PDF->image conversion utility, e.g. Poppler, with Cairo
    support: `pdftocairo`.

test_qgspallabeling_server
    ``TestServer*`` framework for ``qgis_mapserv.fcgi`` output to `image`.
    Compares *qgis_mapserv->image* against *canvas->image*. Utilizes the
    ``qgis_local_server`` module.

qgis_local_server
    A local-only, on-demand server process controller to aid unit tests. It is
    launched with a custom configuration and independently manages the HTTP and
    FCGI server processes.

    **Requires:** HTTP and FCGI-spawning utilities, e.g. `lighttpd`
    and `spawn-fcgi`.

test_qgis_local_server
    Unit tests for ``qgis_local_server``.

Running the Suite
=================

Since the overall suite and frameworks will generate many units, making manual
management of label tests quite tedious, there are extra tools provided to aid
unit test authors. The tools are generally triggered via setting environment
variables, though some work sessions may require un/commenting configuration
lines in multiple files.

Test modules can be run on the command line using CTest's regex support. The
CTest name is listed in the module's docstring, e.g. PyQgsPalLabelingCanvas::

    # run just test_qgspallabeling_canvas in verbose mode
    $ ctest -R PyQgsPalLabelingCanvas -V

    # run all PAL test modules; all CTest names start with PyQgsPalLabeling
    $ ctest -R PyQgsPalLabeling

Environment variables
---------------------

These are all flags that only need to be set or unset, e.g. (using bash)::

    # set
    $ export PAL_VERBOSE=1

    # unset (note: export PAL_VERBOSE=0 will NOT work)
    $ unset PAL_VERBOSE

PAL_VERBOSE
    The Python unit test modules, as run via CTest, will not output individual
    class/function test results, only whether the module as a whole succeeded or
    failed. Setting this variable will print individually run class/function
    test results, up to the point where any exception is raised.

    In addition to setting the variable, CTest needs run in verbose mode.

    **Sample session**::

        $ cd <qgis-build-dir>
        $ export PAL_VERBOSE=1
        $ ctest -R PyQgsPalLabelingCanvas -V
        ...
        85: test_default_label (__main__.TestCanvasPoint) ... ok
        85: test_text_size_map_unit (__main__.TestCanvasPoint) ... ok
        85: test_text_color (__main__.TestCanvasPoint) ... ok
        85: test_background_rect (__main__.TestCanvasPoint) ... FAILED
        ...
        85: ----------------------------------------------------------
        85: Ran X tests in X.Xs

        1/1 Test #85: PyQgsPalLabelingCanvas ...........   FAILED    X.XX sec

        The following tests failed:
            PyQgsPalLabelingCanvas

PAL_REPORT
    Setting this variable will open an HTML report of any failed image
    comparisons as a grouped report in your default web browser. This is the
    HTML output from ``QgsRenderChecker`` wrapped in a local report. It is
    **highly recommended** setting this when creating new unit tests to visually
    debug any issues *before* committing. Otherwise, all other nightly test
    machines may build and run tests, flooding the online test collation server
    with possibly avoidable CDash failed test reports.

PAL_SUITE
    Since you cannot define specific class/function tests when running the
    modules via the CTest command, setting this variable will allow defining
    specific tests to run, e.g. any number of class/function tests, suite
    groupings, or all tests.

    All base units and suite groupings are listed in ``suiteTests()`` of
    ``test_qgspallabeling_tests``, with all unit tests commented out by default.
    (Please keep them commented out when committing.)

    Some modules, like ``test_qgspallabeling_composer``, generate tests for
    multiple outputs or cross-reference comparisons. Those files have the test
    suite separately extended, per line, to help define test selection.

    **Sample session**::

        $ cd <qgis-build-dir>
        $ export PAL_VERBOSE=1
        $ export PAL_SUITE=1

        $ nano <qgis-src-dir>/tests/src/python/test_qgspallabeling_tests.py
          # uncomment units you want to test
          # e.g. only 'test_default_label', is now active

        $ nano <qgis-src-dir>/tests/src/python/test_qgspallabeling_composer.py
          # comment-out undesired extended suite lines, i.e. suite.extend(*)
          # e.g. only 'suite.extend(sp_pvs)' is now active
          # note: this step is unnecessary for modules without extended suites
          # or when you wish to test all available suites

        $ ctest -R PyQgsPalLabelingComposer -V

    Above will only run ``TestComposerPdfVsComposerPoint.test_default_label`` in
    verbose mode and no other tests. This is especially useful for debugging a
    single test or group, and for (re)building control images.
    See PAL_CONTROL_IMAGE.

PAL_NO_MISMATCH and PAL_NO_COLORTOL
    Some test classes or units may have a default allowable pixel mismatch
    and/or color tolerance value for image comparison. Reset the allowable
    mismatch or tolerance to *zero* by setting one (or both) of these variables,
    effectively bypassing all defined defaults. Either of these, coupled with
    PAL_REPORT, helps determine actual differences and whether defaults are
    allowing (masking) a false positive result.

PAL_CONTROL_IMAGE
    Setting this variable will (re)build control images for selected tests.
    When being rebuilt, the associated unit test should *always* pass. Any class
    that contains a 'Vs' string, i.e. all cross-comparison checks, will not
    have images built, since the rendered test image is always compared against
    an existing control image of a different test class.

    **CAUTION:** Do not leave this set. Unset it immediately after building any
    needed control images. You can reset any accidentally overwritten control
    images using ``git``, however.

PAL_SERVER_TEMP
    Used only in ``test_qgspallabeling_server``. When set, opens the temporary
    HTML server directory, instead of deleting it, upon test class completion.
    This is useful when debugging tests, since the directory contains server
    process logs and the generated test project file.
