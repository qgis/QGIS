import os.path

from nose2.tests._common import FunctionalTestCase


class TestCoverage(FunctionalTestCase):
    def test_run(self):
        proc = self.runIn(
            'scenario/test_with_module',
            '-v',
            '--with-coverage',
            '--coverage=lib/'
        )
        STATS = '           8      5    38%'

        stdout, stderr = proc.communicate()
        self.assertTestRunOutputMatches(
            proc,
            stderr=os.path.join('lib', 'mod1').replace('\\', r'\\') + STATS)
        self.assertTestRunOutputMatches(
            proc,
            stderr='TOTAL   ' + STATS)
