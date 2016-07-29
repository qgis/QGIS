import re

from nose2.tests._common import FunctionalTestCase


class TestPrintHooksPlugin(FunctionalTestCase):

    def test_invocation_by_double_dash_option(self):
        proc = self.runIn(
            'scenario/no_tests',
            '--plugin=nose2.plugins.printhooks',
            '--print-hooks')
        match = re.compile("\n"
                           "handleArgs: "
                           "CommandLineArgsEvent\(handled=False, args=")
        self.assertTestRunOutputMatches(proc, stderr=match)
        self.assertEqual(proc.poll(), 0)

    def test_invocation_by_single_dash_option(self):
        proc = self.runIn(
            'scenario/no_tests',
            '--plugin=nose2.plugins.printhooks',
            '-P')
        match = re.compile("\n"
                           "handleArgs: "
                           "CommandLineArgsEvent\(handled=False, args=")
        self.assertTestRunOutputMatches(proc, stderr=match)
        self.assertEqual(proc.poll(), 0)

    def test_nested_hooks_are_indented(self):
        proc = self.runIn(
            'scenario/no_tests',
            '--plugin=nose2.plugins.printhooks',
            '--print-hooks')
        match = re.compile("\n  handleFile: ")
        self.assertTestRunOutputMatches(proc, stderr=match)
        self.assertEqual(proc.poll(), 0)
