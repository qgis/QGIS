from nose2.plugins import outcomes
from nose2 import events, result, session
from nose2.tests._common import TestCase


class TestOutComesPlugin(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.result = result.PluggableTestResult(self.session)
        self.plugin = outcomes.Outcomes(session=self.session)
        self.plugin.register()

        class Test(TestCase):

            def test_e1(self):
                raise KeyError("k")

            def test_e2(self):
                raise TypeError("x")

            def test_e3(self):
                raise IOError("o")

        self.case = Test

        class Watcher(events.Plugin):

            def __init__(self):
                self.outcomes = {}

            def testOutcome(self, event):
                self.outcomes.setdefault(event.outcome, []).append(event)
        self.watcher = Watcher(session=self.session)
        self.watcher.register()

    def test_labels_upper(self):
        self.assertEqual(self.plugin.labels('xxx'), ('X', 'XXX'))

    def test_can_do_nothing_when_not_configured(self):
        test = self.case('test_e1')
        test(self.result)
        assert self.watcher.outcomes['error']
        assert not 'failed' in self.watcher.outcomes

    def test_can_treat_as_fail(self):
        self.plugin.treatAsFail.add('KeyError')
        test = self.case('test_e1')
        test(self.result)
        assert self.watcher.outcomes['failed']
        assert not 'error' in self.watcher.outcomes

    def test_can_treat_as_skip(self):
        self.plugin.treatAsSkip.add('KeyError')
        test = self.case('test_e1')
        test(self.result)
        assert self.watcher.outcomes['skipped']
        assert not 'error' in self.watcher.outcomes

    def test_can_handle_multiple_events_cleanly(self):
        self.plugin.treatAsSkip.add('KeyError')
        self.plugin.treatAsFail.add('TypeError')
        test = self.case('test_e1')
        test(self.result)
        test = self.case('test_e2')
        test(self.result)
        test = self.case('test_e3')
        test(self.result)
        self.assertEqual(len(self.watcher.outcomes['skipped']), 1)
        self.assertEqual(len(self.watcher.outcomes['error']), 1)
        self.assertEqual(len(self.watcher.outcomes['failed']), 1)
