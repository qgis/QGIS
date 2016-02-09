# Adapted from unittest2/loader.py from the unittest2 plugins branch.
# This module contains some code copied from unittest2/loader.py and other
# code developed in reference to that module and others within unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

import logging
import traceback

from nose2 import events
from nose2.compat import unittest


log = logging.getLogger(__name__)
__unittest = True


class PluggableTestLoader(object):

    """Test loader that defers all loading to plugins

    :param session: Test run session.

    .. attribute :: suiteClass

       Suite class to use. Default: :class:`unittest.TestSuite`.

    """
    suiteClass = unittest.TestSuite

    def __init__(self, session):
        self.session = session

    def loadTestsFromModule(self, module):
        """Load tests from module.

        Fires :func:`loadTestsFromModule` hook.

        """
        evt = events.LoadFromModuleEvent(self, module)
        result = self.session.hooks.loadTestsFromModule(evt)
        if evt.handled:
            suite = result or self.suiteClass()
        else:
            suite = self.suiteClass(evt.extraTests)
        filterevt = events.ModuleSuiteEvent(self, module, suite)
        result = self.session.hooks.moduleLoadedSuite(filterevt)
        if result:
            return result or self.suiteClass()
        return filterevt.suite

    def loadTestsFromNames(self, testNames, module=None):
        """Load tests from test names.

        Fires :func:`loadTestsFromNames` hook.

        """
        event = events.LoadFromNamesEvent(
            self, testNames, module)
        result = self.session.hooks.loadTestsFromNames(event)
        log.debug('loadTestsFromNames event %s result %s', event, result)
        if event.handled:
            suites = result or []
        else:
            if event.names:
                suites = [self.loadTestsFromName(name, module)
                          for name in event.names]
            elif module:
                suites = self.loadTestsFromModule(module)
        if event.extraTests:
            suites.extend(event.extraTests)
        return self.suiteClass(suites)

    def loadTestsFromName(self, name, module=None):
        """Load tests from test name.

        Fires :func:`loadTestsFromName` hook.

        """
        log.debug('loadTestsFromName %s/%s', name, module)
        event = events.LoadFromNameEvent(self, name, module)
        result = self.session.hooks.loadTestsFromName(event)
        if event.handled:
            suite = result or self.suiteClass()
            return suite
        return self.suiteClass(event.extraTests)

    def failedImport(self, name):
        """Make test case representing a failed import."""
        message = 'Failed to import test module: %s' % name
        if hasattr(traceback, 'format_exc'):
            # Python 2.3 compatibility
            # format_exc returns two frames of discover.py as well XXX ?
            message += '\n%s' % traceback.format_exc()
        return self._makeFailedTest(
            'ModuleImportFailure', name, ImportError(message))

    def failedLoadTests(self, name, exception):
        """Make test case representing a failed test load."""
        return self._makeFailedTest('LoadTestsFailure', name, exception)

    def sortTestMethodsUsing(self, name):
        """Sort key for test case test methods."""
        return name.lower()

    def discover(self, start_dir=None, pattern=None):
        """Compatibility shim for load_tests protocol."""
        try:
            oldsd = self.session.startDir
            self.session.startDir = start_dir
            return self.loadTestsFromNames([])
        finally:
            self.session.startDir = oldsd

    def _makeFailedTest(self, classname, methodname, exception):
        def testFailure(self):
            raise exception
        attrs = {methodname: testFailure}
        TestClass = type(classname, (unittest.TestCase,), attrs)
        return self.suiteClass((TestClass(methodname),))

    def __repr__(self):
        return '<%s>' % self.__class__.__name__
