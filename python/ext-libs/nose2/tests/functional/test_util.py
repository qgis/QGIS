from nose2.tests._common import TestCase, support_file
from nose2 import util


class UtilTests(TestCase):

    def test_name_from_path(self):
        self.assertEqual(
            util.name_from_path(support_file('scenario/tests_in_package/pkg1/test/test_things.py')), 'pkg1.test.test_things')
