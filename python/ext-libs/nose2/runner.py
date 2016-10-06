# This module contains some code copied from unittest2/runner.py and other
# code developed in reference to that module and others within unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

import time

from nose2 import events, result


__unittest = True


class PluggableTestRunner(object):

    """Test runner that defers most work to plugins.

    :param session: Test run session

    .. attribute :: resultClass

       Class to instantiate to create test result. Default:
       :class:`nose2.result.PluggableTestResult`.

    """
    resultClass = result.PluggableTestResult

    def __init__(self, session):
        self.session = session

    def run(self, test):
        """Run tests.

        :param test: A unittest TestSuite or TestClass.
        :returns: Test result

        Fires :func:`startTestRun` and :func:`stopTestRun` hooks.

        """
        result = self._makeResult()
        executor = lambda suite, result: suite(result)
        startTime = time.time()
        event = events.StartTestRunEvent(
            self, test, result, startTime, executor)
        self.session.hooks.startTestRun(event)

        # allows startTestRun to modify test suite
        test = event.suite
        # ... and test execution
        executor = event.executeTests
        try:
            if not event.handled:
                executor(test, result)
        finally:
            stopTime = time.time()
            timeTaken = stopTime - startTime
            event = events.StopTestRunEvent(self, result, stopTime, timeTaken)
            self.session.hooks.stopTestRun(event)
            self.session.hooks.afterTestRun(event)
        return result

    def _makeResult(self):
        result = self.resultClass(self.session)
        event = events.ResultCreatedEvent(result)
        self.session.hooks.resultCreated(event)
        self.session.testResult = event.result
        return event.result

    def __repr__(self):
        return '<%s>' % self.__class__.__name__
