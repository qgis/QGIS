from nose2.tests._common import TestCase, RedirectStdStreams
from nose2 import collector
from textwrap import dedent
import re


class TestCollector(TestCase):
    _RUN_IN_TEMP = True
    tags = ['unit']

    def test_collector_completes_with_no_tests(self):
        with open("unittest.cfg", "w") as ut_file:
            ut_file.write(dedent("""
                [unittest]
                quiet = true
            """))
        test = collector.collector()
        with RedirectStdStreams() as redir:
            self.assertRaises(SystemExit, test.run, None)
        self.assertEqual("", redir.stdout.getvalue())
        self.assertTrue(re.match(r'\n-+\nRan 0 tests in \d.\d\d\ds\n\nOK\n',
                                 redir.stderr.getvalue()))
