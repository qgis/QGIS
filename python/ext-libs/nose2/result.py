import time

from nose2 import events


ERROR = 'error'
FAIL = 'failed'
SKIP = 'skipped'
PASS = 'passed'
__unittest = True


class PluggableTestResult(object):

    """Test result that defers to plugins.

    All test outcome recording and reporting is deferred to plugins,
    which are expected to implement startTest, stopTest, testOutcome,
    and wasSuccessful.

    :param session: Test run session.

    .. attribute :: shouldStop

       When True, test run should stop before running another test.

    """

    def __init__(self, session):
        self.session = session
        self.shouldStop = False

    def startTest(self, test):
        """Start a test case.

        Fires :func:`startTest` hook.

        """
        event = events.StartTestEvent(test, self, time.time())
        self.session.hooks.startTest(event)

    def stopTest(self, test):
        """Stop a test case.

        Fires :func:`stopTest` hook.

        """
        event = events.StopTestEvent(test, self, time.time())
        self.session.hooks.stopTest(event)

    def addError(self, test, err):
        """Test case resulted in error.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, ERROR, err)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def addFailure(self, test, err):
        """Test case resulted in failure.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, FAIL, err)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def addSuccess(self, test):
        """Test case resulted in success.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, PASS, expected=True)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def addSkip(self, test, reason):
        """Test case was skipped.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, SKIP, reason=reason)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def addExpectedFailure(self, test, err):
        """Test case resulted in expected failure.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, FAIL, err, expected=True)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def addUnexpectedSuccess(self, test):
        """Test case resulted in unexpected success.

        Fires :func:`setTestOutcome` and :func:`testOutcome` hooks.

        """
        event = events.TestOutcomeEvent(test, self, PASS)
        self.session.hooks.setTestOutcome(event)
        self.session.hooks.testOutcome(event)

    def wasSuccessful(self):
        """Was test run successful?

        Fires :func:`wasSuccessful` hook, returns ``event.success``.

        """
        # assume failure, plugins must affirmatively declare success
        try:
            return self._success
        except AttributeError:
            event = events.ResultSuccessEvent(self, False)
            self.session.hooks.wasSuccessful(event)
            self._success = event.success
            return self._success

    def stop(self):
        """Stop test run.

        Fires :func:`resultStop` hook, sets ``self.shouldStop`` to
        ``event.shouldStop``.

        """
        event = events.ResultStopEvent(self, True)
        self.session.hooks.resultStop(event)
        self.shouldStop = event.shouldStop

    def __repr__(self):
        return '<%s>' % self.__class__.__name__
