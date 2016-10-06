"""
Loader that implements the load_tests protocol.

This plugin implements the load_tests protocol as detailed in the
documentation for unittest2.

See the `load_tests protocol`_ documentation for more information.

.. warning ::

   Test suites using the load_tests protocol do not work correctly
   with the multiprocess plugin as of nose2 04. This will be
   fixed in a future release.

.. _load_tests protocol: http://docs.python.org/library/unittest.html#load-tests-protocol
"""
from fnmatch import fnmatch
import logging

from nose2 import events, util


log = logging.getLogger(__name__)


class LoadTestsLoader(events.Plugin):

    """Loader plugin that implements load_tests."""
    alwaysOn = True
    configSection = 'load_tests'
    _loading = False

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)

    def moduleLoadedSuite(self, event):
        """Run load_tests in a module.

        May add to or filter tests loaded in module.

        """
        module = event.module
        load_tests = getattr(module, 'load_tests', None)
        if not load_tests:
            return
        try:
            event.suite = load_tests(
                event.loader, event.suite, self.session.testFilePattern)
        except Exception as exc:
            log.exception(
                "Failed to load tests from %s via load_tests", module)
            suite = event.loader.suiteClass()
            suite.addTest(event.loader.failedLoadTests(module.__name__, exc))
            event.handled = True
            return suite

    def handleDir(self, event):
        """Run load_tests in packages.

        If a package itself matches the test file pattern, run
        load_tests in its __init__.py, and stop default test
        discovery for that package.

        """
        if self._loading:
            return

        if (self._match(event.name, event.pattern) and
            util.ispackage(event.path)):
            name = util.name_from_path(event.path)
            module = util.module_from_name(name)

            load_tests = getattr(module, 'load_tests', None)
            if not load_tests:
                return
            self._loading = True
            try:
                suite = event.loader.suiteClass()
                try:
                    suite = load_tests(event.loader, suite, event.pattern)
                except Exception as exc:
                    log.exception(
                        "Failed to load tests from %s via load_tests", module)
                    suite.addTest(
                        event.loader.failedLoadTests(module.__name__, exc))

                event.handled = True
                return suite
            finally:
                self._loading = False

    def _match(self, filename, pattern):
        return fnmatch(filename, pattern)
