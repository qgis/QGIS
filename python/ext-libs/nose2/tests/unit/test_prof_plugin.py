from nose2 import session
from nose2.plugins import prof
from nose2.events import StartTestRunEvent
from nose2.tests._common import Stub, TestCase


class TestProfPlugin(TestCase):
    tags = ['unit']

    def setUp(self):
        self.plugin = prof.Profiler(session=session.Session())
        self.hotshot = prof.hotshot
        self.stats = prof.stats
        prof.hotshot = Stub()
        prof.stats = Stub()

    def tearDown(self):
        prof.hotshot = self.hotshot
        prof.stats = self.stats

    def test_startTestRun_sets_executeTests(self):
        _prof = Stub()
        _prof.runcall = object()
        prof.hotshot.Profile = lambda filename: _prof
        event = StartTestRunEvent(runner=None, suite=None, result=None,
                                  startTime=None, executeTests=None)
        self.plugin.startTestRun(event)
        assert event.executeTests is _prof.runcall, \
            "executeTests was not replaced"
