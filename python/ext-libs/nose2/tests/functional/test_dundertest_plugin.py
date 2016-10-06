from nose2.tests._common import FunctionalTestCase


class TestDunderTestPlugin(FunctionalTestCase):
    def test_dunder(self):
        proc = self.runIn(
            'scenario/dundertest_attribute',
            '-v')
        self.assertTestRunOutputMatches(proc, stderr='Ran 0 tests')
