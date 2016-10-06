import logging

from nose2.tests._common import TestCase
from nose2.plugins import logcapture
from nose2 import session


log = logging.getLogger(__name__)


class StubLogging(object):

    def __init__(self, name=None):
        self.name = name
        self.handlers = []
        self.level = None

    def getLogger(self, _name=None):
        return self

    def addHandler(self, handler):
        self.handlers.append(handler)

    def setLevel(self, level):
        self.level = level

    def debug(self, message, *arg):
        # import pdb; pdb.set_trace()
        for handler in self.handlers:
            handler.emit(StubRecord(message % arg))


class StubRecord(object):

    def __init__(self, message):
        self.message = message
        self.name = 'stub'
        self.levelname = 'stub'
        self.exc_info = None
        self.exc_text = None
        self.stack_info = None

    def getMessage(self):
        return self.message


class LogCaptureUnitTest(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.plugin = logcapture.LogCapture(session=self.session)
        self.logging = logcapture.logging
        logcapture.logging = StubLogging()

    def tearDown(self):
        logcapture.logging = self.logging

    def event(self, error=True, failed=False):
        e = Event()
        e.metadata = {}
        return e

    def test_buffer_cleared_after_each_test(self):
        self.plugin.startTestRun(None)
        self.plugin.startTest(None)
        logcapture.logging.getLogger('test').debug("hello")
        assert self.plugin.handler.buffer
        self.plugin.setTestOutcome(self.event())
        assert self.plugin.handler.buffer
        self.plugin.stopTest(None)
        assert not self.plugin.handler.buffer

    def test_buffered_logs_attached_to_event(self):
        self.plugin.startTestRun(None)
        self.plugin.startTest(None)
        logcapture.logging.getLogger('test').debug("hello")
        assert self.plugin.handler.buffer
        e = self.event()
        self.plugin.setTestOutcome(e)
        assert 'logs' in e.metadata, "No log in %s" % e.metadata


class Event:
    pass
