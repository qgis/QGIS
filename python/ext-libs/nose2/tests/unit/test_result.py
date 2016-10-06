from nose2 import result, session
from nose2.tests._common import TestCase


class TestPluggableTestResult(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.result = result.PluggableTestResult(self.session)

    def test_skip_reason_not_discarded(self):
        class Test(TestCase):

            def test(self):
                pass
        plugin = FakePlugin()
        self.session.hooks.register('testOutcome', plugin)
        self.result.addSkip(Test('test'), 'because')
        self.assertEqual(plugin.reason, 'because')


class FakePlugin(object):

    def testOutcome(self, event):
        self.reason = event.reason
