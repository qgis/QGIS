"""
Collect and report test results.

This plugin implements the primary user interface for nose2. It
collects test outcomes and reports on them to the console, as well as
firing several hooks for other plugins to do their own reporting.

To see this report, nose2 MUST be run with the "verbose" flag::

  nose2 --verbose

This plugin extends standard unittest console reporting slightly by
allowing custom report categories. To put events into a custom
reporting category, change the event.outcome to whatever you
want. Note, however, that customer categories are *not* treated as
errors or failures for the purposes of determining whether a test run
has succeeded.

Don't disable this plugin unless you a) have another one doing the
same job or b) really don't want any test results (and want all test
runs to exit(1))
"""
# This module contains some code copied from unittest2/runner.py and other
# code developed in reference to that module and others within unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

import sys
import unittest

from nose2 import events, result, util

__unittest = True


class ResultReporter(events.Plugin):

    """Result plugin that implements standard unittest console reporting"""
    alwaysOn = True
    configSection = 'test-result'
    separator1 = '=' * 70
    separator2 = '-' * 70

    def __init__(self):
        self.testsRun = 0
        self.reportCategories = {'failures': [],
                                 'errors': [],
                                 'skipped': [],
                                 'expectedFailures': [],
                                 'unexpectedSuccesses': []}
        self.dontReport = set(['errors', 'failures', 'skipped', 'passed',
                               'expectedFailures', 'unexpectedSuccesses'])

        self.stream = util._WritelnDecorator(sys.stderr)
        self.descriptions = self.config.as_bool('descriptions', True)

    def startTest(self, event):
        """Handle startTest hook

        - prints test description if verbosity > 1
        """
        self.testsRun += 1
        self._reportStartTest(event)

    def testOutcome(self, event):
        """Handle testOutcome hook

        - records test outcome in reportCategories
        - prints test outcome label
        - fires reporting hooks (:func:`reportSuccess`, :func:`reportFailure`,
          etc)

        """
        if event.outcome == result.ERROR:
            self.reportCategories['errors'].append(event)
            self._reportError(event)
        elif event.outcome == result.FAIL:
            if not event.expected:
                self.reportCategories['failures'].append(event)
                self._reportFailure(event)
            else:
                self.reportCategories['expectedFailures'].append(event)
                self._reportExpectedFailure(event)
        elif event.outcome == result.SKIP:
            self.reportCategories['skipped'].append(event)
            self._reportSkip(event)
        elif event.outcome == result.PASS:
            if event.expected:
                self._reportSuccess(event)
            else:
                self.reportCategories['unexpectedSuccesses'].append(event)
                self._reportUnexpectedSuccess(event)
        else:
            # generic outcome handling
            self.reportCategories.setdefault(event.outcome, []).append(event)
            self._reportOtherOutcome(event)

    def afterTestRun(self, event):
        """Handle afterTestRun hook

        - prints error lists
        - prints summary
        - fires summary reporting hooks (:func:`beforeErrorList`,
          :func:`beforeSummaryReport`, etc)

        """
        self._reportSummary(event)

    def wasSuccessful(self, event):
        event.success = True
        for name, events in self.reportCategories.items():
            for e in events:
                if (e.outcome == result.ERROR or
                    (e.outcome == result.FAIL and not e.expected)):
                    event.success = False
                    break

    def _reportStartTest(self, event):
        evt = events.ReportTestEvent(event, self.stream)
        self.session.hooks.reportStartTest(evt)
        if evt.handled:
            return
        if self.session.verbosity > 1:
            # allow other plugins to override/spy on stream
            evt.stream.write(self._getDescription(event.test, errorList=False))
            evt.stream.write(' ... ')
            evt.stream.flush()

    def _reportError(self, event):
        self._report(event, 'reportError', 'E', 'ERROR')

    def _reportFailure(self, event):
        self._report(event, 'reportFailure', 'F', 'FAIL')

    def _reportSkip(self, event):
        self._report(event, 'reportSkip', 's', 'skipped %s' % event.reason)

    def _reportExpectedFailure(self, event):
        self._report(event, 'reportExpectedFailure', 'x', 'expected failure')

    def _reportUnexpectedSuccess(self, event):
        self._report(
            event, 'reportUnexpectedSuccess', 'u', 'unexpected success')

    def _reportOtherOutcome(self, event):
        self._report(event, 'reportOtherOutcome', '?', 'unknown outcome')

    def _reportSuccess(self, event):
        self._report(event, 'reportSuccess', '.', 'ok')

    def _reportSummary(self, event):
        # let others print something
        evt = events.ReportSummaryEvent(
            event, self.stream, self.reportCategories)
        self.session.hooks.beforeErrorList(evt)
        # allows other plugins to mess with report categories
        cats = evt.reportCategories
        errors = cats.get('errors', [])
        failures = cats.get('failures', [])
        # use evt.stream so plugins can replace/wrap/spy it
        evt.stream.writeln('')
        self._printErrorList('ERROR', errors, evt.stream)
        self._printErrorList('FAIL', failures, evt.stream)

        for flavour, events_ in cats.items():
            if flavour in self.dontReport:
                continue
            self._printErrorList(flavour.upper(), events_, evt.stream)
        self._printSummary(evt)

    def _printErrorList(self, flavour, events_, stream):
        for event in events_:
            desc = self._getDescription(event.test, errorList=True)
            err = self._getOutcomeDetail(event)
            stream.writeln(self.separator1)
            stream.writeln("%s: %s" % (flavour, desc))
            stream.writeln(self.separator2)
            stream.writeln("%s" % err)

    def _printSummary(self, reportEvent):
        self.session.hooks.beforeSummaryReport(reportEvent)

        stream = reportEvent.stream
        stream.writeln(self.separator2)
        run = self.testsRun
        msg = (
            "Ran %d test%s in %.3fs\n" %
            (run, run != 1 and "s" or "", reportEvent.stopTestEvent.timeTaken))
        stream.writeln(msg)

        infos = []
        extraInfos = []
        if reportEvent.stopTestEvent.result.wasSuccessful():
            stream.write("OK")
        else:
            stream.write("FAILED")

        failed = len(reportEvent.reportCategories.get('failures', []))
        errored = len(reportEvent.reportCategories.get('errors', []))
        skipped = len(reportEvent.reportCategories.get('skipped', []))
        expectedFails = len(
            reportEvent.reportCategories.get('expectedFailures', []))
        unexpectedSuccesses = len(
            reportEvent.reportCategories.get('unexpectedSuccesses', []))

        for flavour, results in reportEvent.reportCategories.items():
            if flavour in self.dontReport:
                continue
            count = len(results)
            if count:
                extraInfos.append("%s=%d" % (flavour, count))

        if failed:
            infos.append("failures=%d" % failed)
        if errored:
            infos.append("errors=%d" % errored)
        if skipped:
            infos.append("skipped=%d" % skipped)
        if expectedFails:
            infos.append("expected failures=%d" % expectedFails)
        if unexpectedSuccesses:
            infos.append("unexpected successes=%d" % unexpectedSuccesses)
        infos.extend(extraInfos)
        if infos:
            reportEvent.stream.writeln(" (%s)" % (", ".join(infos),))
        else:
            reportEvent.stream.writeln('')

        self.session.hooks.afterSummaryReport(reportEvent)

    def _getDescription(self, test, errorList):
        if not isinstance(test, unittest.TestCase):
            return test.__class__.__name__
        doc_first_line = test.shortDescription()
        if self.descriptions and doc_first_line:
            desc = '\n'.join((str(test), doc_first_line))
        else:
            desc = str(test)
        event = events.DescribeTestEvent(
            test, description=desc, errorList=errorList)
        self.session.hooks.describeTest(event)
        return event.description

    def _getOutcomeDetail(self, event):
        evt = events.OutcomeDetailEvent(event)
        result = self.session.hooks.outcomeDetail(evt)
        if evt.handled:
            return result
        exc_info = getattr(event, 'exc_info', None)
        test = getattr(event, 'test', None)
        if exc_info:
            detail = [util.exc_info_to_string(exc_info, test)]
        else:
            detail = []
        if evt.extraDetail:
            detail.extend(evt.extraDetail)
        try:
            return "\n".join(detail)
        except UnicodeDecodeError:
            return "\n".join(util.safe_decode(d) for d in detail)

    def _report(self, event, hook, shortLabel, longLabel):
        evt = events.ReportTestEvent(event, self.stream)
        getattr(self.session.hooks, hook)(evt)
        if evt.handled:
            return
        if self.session.verbosity > 1:
            # event I fired has stream, event I received has labels
            evt.stream.writeln(getattr(event, 'longLabel', None) or longLabel)
        elif self.session.verbosity:
            evt.stream.write(getattr(event, 'shortLabel', None) or shortLabel)
            evt.stream.flush()
