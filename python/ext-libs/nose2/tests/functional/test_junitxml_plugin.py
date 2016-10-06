import os

from nose2.tests._common import FunctionalTestCase
from nose2.tests._common import TestCase
from nose2.tests._common import support_file


class JunitXmlPluginFunctionalTest(FunctionalTestCase, TestCase):
    _RUN_IN_TEMP = True

    def run_with_junitxml_loaded(self, scenario, *args):
        work_dir = os.getcwd()
        test_dir = support_file(*scenario)
        junit_report = os.path.join(work_dir, 'nose2-junit.xml')
        if os.path.exists(junit_report):
            os.remove(junit_report)
        proc = self.runIn(work_dir,
                          '-s%s' % test_dir,
                          '--plugin=nose2.plugins.junitxml',
                          '-v',
                          *args)
        return junit_report, proc

    def test_invocation_by_double_dash_option(self):
        junit_report, proc = self.run_with_junitxml_loaded(
            ('scenario', 'junitxml', 'happyday'),
            '--junit-xml')

        self.assertTestRunOutputMatches(
            proc, stderr='test \(test_junitxml_happyday.Test\) ... ok')
        self.assertTestRunOutputMatches(
            proc, stderr='Ran 1 test')
        self.assertEqual(proc.poll(), 0)

        self.assertTrue(os.path.isfile(junit_report),
                        "junitxml report wasn't found in working directory. "
                        "Searched for " + junit_report)

    def test_invocation_by_single_dash_option(self):
        junit_report, proc = self.run_with_junitxml_loaded(
            ('scenario', 'junitxml', 'happyday'),
            '-X')

        self.assertTestRunOutputMatches(
            proc, stderr='test \(test_junitxml_happyday.Test\) ... ok')
        self.assertTestRunOutputMatches(
            proc, stderr='Ran 1 test')
        self.assertEqual(proc.poll(), 0)

        self.assertTrue(os.path.isfile(junit_report),
                        "junitxml report wasn't found in working directory. "
                        "Searched for " + junit_report)

    def test_no_report_written_if_loaded_but_not_invoked(self):
        junit_report, proc = self.run_with_junitxml_loaded(
            ('scenario', 'junitxml', 'happyday'))

        self.assertTestRunOutputMatches(
            proc, stderr='test \(test_junitxml_happyday.Test\) ... ok')
        self.assertTestRunOutputMatches(
            proc, stderr='Ran 1 test')
        self.assertEqual(proc.poll(), 0)

        self.assertFalse(os.path.isfile(junit_report),
                         "junitxml report was found in working directory. "
                         "Report file: " + junit_report)

    def test_report_location_should_be_resilent_to_chdir_in_tests(self):
        junit_report, proc = self.run_with_junitxml_loaded(
            ('scenario', 'junitxml', 'chdir'), '--junit-xml')

        self.assertTestRunOutputMatches(
            proc,
            stderr='test_chdir \(test_junitxml_chdir.Test\) \.* ok')
        self.assertTestRunOutputMatches(
            proc, stderr='Ran 1 test')
        self.assertEqual(proc.poll(), 0)

        self.assertTrue(os.path.isfile(junit_report),
                        "junitxml report wasn't found in working directory. "
                        "Searched for " + junit_report)


class JunitXmlPluginFunctionalFailureTest(FunctionalTestCase, TestCase):
    def test_failure_to_write_report(self):
        proc = self.runIn('scenario/junitxml/fail_to_write',
                          '--plugin=nose2.plugins.junitxml',
                          '-v',
                          '--junit-xml')
        self.assertEqual(proc.poll(), 1)

        self.assertTestRunOutputMatches(
            proc,
            stderr='test \(test_junitxml_fail_to_write.Test\) \.* ok')
        self.assertTestRunOutputMatches(
            proc, stderr=r'Internal Error: runTests aborted: \[Errno 2\] JUnitXML: Parent folder does not exist for file: \'/does/not/exist\.xml\'')


