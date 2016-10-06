from nose2.tests._common import FunctionalTestCase


class TestAttribPlugin(FunctionalTestCase):

    def test_simple_true(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-A',
            'a')
        self.assertTestRunOutputMatches(proc, stderr='Ran 4 tests')
        self.assertTestRunOutputMatches(proc, stderr='test_params_method')
        self.assertTestRunOutputMatches(proc, stderr='test_func')

    def test_simple_false(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-A',
            '!a')
        self.assertTestRunOutputMatches(proc, stderr='Ran 21 tests')

    def test_simple_value(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-A',
            'b=2')
        self.assertTestRunOutputMatches(proc, stderr='Ran 2 tests')

    def test_list_value(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-A',
            'tags=func')
        self.assertTestRunOutputMatches(proc, stderr='Ran 8 tests')
        self.assertTestRunOutputMatches(proc, stderr='test_params_func')
        self.assertTestRunOutputMatches(proc, stderr='test_func')
        self.assertTestRunOutputMatches(proc, stderr='test_gen')

    def test_list_value_negation(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-A',
            '!tags=func')
        self.assertTestRunOutputMatches(proc, stderr='Ran 8 tests')
        self.assertTestRunOutputMatches(proc, stderr='test_gen_method')
        self.assertTestRunOutputMatches(proc, stderr='test_params_method')
        self.assertTestRunOutputMatches(proc, stderr='test_ok')
        self.assertTestRunOutputMatches(proc, stderr='test_failed')
        self.assertTestRunOutputMatches(proc, stderr='test_skippy')
        self.assertTestRunOutputMatches(proc, stderr='test_typeerr')

    def test_eval_expr(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.attrib',
            '-E',
            'a == b and a')
        self.assertTestRunOutputMatches(proc, stderr='Ran 1 test')
        self.assertTestRunOutputMatches(proc, stderr='skippy')
