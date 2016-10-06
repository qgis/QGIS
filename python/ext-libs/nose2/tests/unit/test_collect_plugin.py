from nose2.tests._common import FakeStartTestRunEvent, TestCase
from nose2.plugins import collect
from nose2 import session


class TestCollectOnly(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.plugin = collect.CollectOnly(session=self.session)

    def test_startTestRun_sets_executeTests(self):
        event = FakeStartTestRunEvent()
        self.plugin.startTestRun(event)
        self.assertEqual(event.executeTests, self.plugin.collectTests)
