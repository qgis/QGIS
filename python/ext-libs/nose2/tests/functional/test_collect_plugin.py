import re

from nose2.tests._common import FunctionalTestCase


class CollectOnlyFunctionalTest(FunctionalTestCase):

    def test_collect_tests_in_package(self):
        self.assertTestRunOutputMatches(
            self.runIn('scenario/tests_in_package', '-v', '--collect-only',
                       '--plugin=nose2.plugins.collect'),
            stderr=EXPECT_LAYOUT1)


# expectations
EXPECT_LAYOUT1 = re.compile("""\
Ran 25 tests in \d.\d+s

OK""")
