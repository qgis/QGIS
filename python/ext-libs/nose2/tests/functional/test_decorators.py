from nose2.tests._common import FunctionalTestCase


class DecoratorsTests(FunctionalTestCase):
    def test_with_setup(self):
        process = self.runIn(
            'scenario/decorators', 'test_decorators.test_with_setup')

        self.assertTestRunOutputMatches(process, stderr="Ran 1 test")
        self.assertEqual(process.poll(), 0, process.stderr.getvalue())
        
    def test_with_teardown(self):
        process = self.runIn(
            'scenario/decorators', 'test_decorators.test_with_teardown', 'test_decorators.test_teardown_ran')

        self.assertTestRunOutputMatches(process, stderr="Ran 2 test")
        self.assertEqual(process.poll(), 0, process.stderr.getvalue())
