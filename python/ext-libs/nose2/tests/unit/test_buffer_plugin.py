import sys

import six

from nose2.plugins import buffer
from nose2 import events, result, session
from nose2.tests._common import TestCase


class TestBufferPlugin(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.result = result.PluggableTestResult(self.session)
        self.plugin = buffer.OutputBufferPlugin(session=self.session)
        self.plugin.register()

        class Test(TestCase):

            def test_out(self):
                six.print_("hello")
                raise {}["oops"]

            def test_err(self):
                six.print_("goodbye", file=sys.stderr)
        self.case = Test

        class Watcher(events.Plugin):

            def __init__(self):
                self.events = []

            def testOutcome(self, event):
                self.events.append(event)
        self.watcher = Watcher(session=self.session)
        self.watcher.register()

    def test_captures_stdout(self):
        out = sys.stdout
        buf = six.StringIO()
        sys.stdout = buf
        try:
            test = self.case('test_out')
            test(self.result)
            assert "hello" not in buf.getvalue()
            assert "hello" in self.watcher.events[
                0].metadata['stdout'].getvalue()
        finally:
            sys.stdout = out

    def test_captures_stderr_when_configured(self):
        self.plugin.captureStderr = True
        err = sys.stderr
        buf = six.StringIO()
        sys.stderr = buf
        try:
            test = self.case('test_err')
            test(self.result)
            assert "goodbye" not in buf.getvalue()
            assert "goodbye" in self.watcher.events[
                0].metadata['stderr'].getvalue()
        finally:
            sys.stderr = err

    def test_decorates_outcome_detail(self):
        test = self.case('test_out')
        test(self.result)
        evt = events.OutcomeDetailEvent(self.watcher.events[0])
        self.session.hooks.outcomeDetail(evt)
        assert "hello" in "".join(evt.extraDetail)
