import re

from nose2.tests._common import FunctionalTestCase


class LogCaptureFunctionalTest(FunctionalTestCase):

    def test_package_in_lib(self):
        match = re.compile('>> begin captured logging <<')
        self.assertTestRunOutputMatches(
            self.runIn('scenario/package_in_lib', '--log-capture'),
            stderr=match)

    def test_logging_keeps_copies_of_mutable_objects(self):
        proc = self.runIn('scenario/logging',
                          '-v',
                          '--log-capture',
                          'logging_keeps_copies_of_mutable_objects')
        self.assertTestRunOutputMatches(proc, stderr='Ran 1 test in')
        self.assertTestRunOutputMatches(proc, stderr='FAILED')
        self.assertTestRunOutputMatches(proc, stderr='foo: {}')
