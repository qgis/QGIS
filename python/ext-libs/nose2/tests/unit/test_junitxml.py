from xml.etree import ElementTree as ET
from nose2.tests._common import TestCase
from nose2.compat import unittest
from nose2 import events, loader, result, session, tools
from nose2.plugins import junitxml, logcapture
from nose2.plugins.loader import generators, parameters, testcases

import logging
import os
import six
import sys


class TestJunitXmlPlugin(TestCase):
    _RUN_IN_TEMP = True

    BAD_FOR_XML_U = six.u('A\x07 B\x0B C\x10 D\uD900 '
                          'E\uFFFE F\x80 G\x90 H\uFDDD')
    # UTF-8 string with double null (invalid)
    BAD_FOR_XML_B = six.b('A\x07 B\x0b C\x10 D\xed\xa4\x80 '
                          'E\xef\xbf\xbe F\xc2\x80 G\xc2\x90 H\xef\xb7\x9d '
                          '\x00\x00')

    # "byte" strings in PY2 and unicode in py3 works as expected will
    # will translate surrogates into UTF-16 characters  so BAD_FOR_XML_U
    # should have 8 letters follows by 0xFFFD, but only 4 when keeping
    # the discouraged/restricted ranges. Respectively:
    # "A\uFFFD B\uFFFD C\uFFFD D\uFFFD E\uFFFD F\uFFFD G\uFFFD H\uFFFD"
    # "A\uFFFD B\uFFFD C\uFFFD D\uFFFD E\uFFFD F\x80 G\x90 H\uFDDD"
    #
    # In Python 2 Invalid ascii characters seem to get escaped out as part
    # of tracebace.format_traceback so full and partial replacements are:
    # "A\uFFFD B\uFFFD C\uFFFD D\\\\ud900 E\\\\ufffe F\\\\x80 G\\\\x90 H\\\\ufddd"
    # "A\uFFFD B\uFFFD C\uFFFD D\\\\ud900 E\\\\ufffe F\\\\x80 G\\\\x90 H\\\\ufddd"
    #
    # Byte strings in py3 as errors are replaced by their representation string
    # So these will be safe and not have any replacements
    # "b'A\\x07 B\\x0b C\\x10 D\\xed\\xa4\\x80 E\\xef\\xbf\\xbe F\\xc2\\x80
    # G\\xc2\\x90 H\\xef\\xb7\\x9d \\x00\\x00"

    if sys.maxunicode <= 0xFFFF:
        EXPECTED_RE = six.u("^[\x09\x0A\x0D\x20\x21-\uD7FF\uE000-\uFFFD]*$")
        EXPECTED_RE_SAFE = six.u("^[\x09\x0A\x0D\x20\x21-\x7E\x85"
                                 "\xA0-\uD7FF\uE000-\uFDCF\uFDF0-\uFFFD]*$")
    else:
        EXPECTED_RE = six.u("^[\x09\x0A\x0D\x20\x21-\uD7FF\uE000-\uFFFD"
                            "\u10000-\u10FFFF]*$")
        EXPECTED_RE_SAFE = six.u("^[\x09\x0A\x0D\x20\x21-\x7E\x85"
                                 "\xA0-\uD7FF\uE000-\uFDCF\uFDF0-\uFFFD"
                                 "\u10000-\u1FFFD\u20000-\u2FFFD"
                                 "\u30000-\u3FFFD\u40000-\u4FFFD"
                                 "\u50000-\u5FFFD\u60000-\u6FFFD"
                                 "\u70000-\u7FFFD\u80000-\u8FFFD"
                                 "\u90000-\u8FFFD\uA0000-\uAFFFD"
                                 "\uB0000-\uBFFFD\uC0000-\uCFFFD"
                                 "\uD0000-\uDFFFD\uE0000-\uEFFFD"
                                 "\uF0000-\uFFFFD\u100000-\u10FFFD]*$")

    def setUp(self):
        super(TestJunitXmlPlugin, self).setUp()
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(self.session)
        self.result = result.PluggableTestResult(self.session)
        self.plugin = junitxml.JUnitXmlReporter(session=self.session)
        self.plugin.register()

        # unittest2 needs this
        if not hasattr(self, 'assertRegex'):
            self.assertRegex = self.assertRegexpMatches

        class Test(unittest.TestCase):
            def test(self):
                pass

            def test_chdir(self):
                TEMP_SUBFOLDER = 'test_chdir'

                os.mkdir(TEMP_SUBFOLDER)
                os.chdir(TEMP_SUBFOLDER)

            def test_fail(self):
                assert False

            def test_err(self):
                1 / 0

            def test_skip(self):
                self.skipTest('skip')

            def test_bad_xml(self):
                raise RuntimeError(TestJunitXmlPlugin.BAD_FOR_XML_U)

            def test_bad_xml_b(self):
                raise RuntimeError(TestJunitXmlPlugin.BAD_FOR_XML_B)

            def test_gen(self):
                def check(a, b):
                    self.assertEqual(a, b)

                yield check, 1, 1
                yield check, 1, 2

            @tools.params(1, 2, 3)
            def test_params(self, p):
                self.assertEqual(p, 2)

            def test_with_log(self):
                logging.info('log message')

        self.case = Test

    def test_success_added_to_xml(self):
        test = self.case('test')
        test(self.result)
        self.assertEqual(self.plugin.numtests, 1)
        self.assertEqual(len(self.plugin.tree.findall('testcase')), 1)

    def test_failure_includes_traceback(self):
        test = self.case('test_fail')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        failure = case.find('failure')
        assert failure is not None
        assert 'Traceback' in failure.text

    def test_error_bad_xml(self):
        self.plugin.keep_restricted = False
        test = self.case('test_bad_xml')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        error = case.find('error')
        self.assertRegex(error.text, self.EXPECTED_RE_SAFE)

    def test_error_bad_xml_keep(self):
        self.plugin.keep_restricted = True
        test = self.case('test_bad_xml')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        error = case.find('error')
        self.assertRegex(error.text, self.EXPECTED_RE)

    def test_error_bad_xml_b(self):
        self.plugin.keep_restricted = False
        test = self.case('test_bad_xml_b')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        error = case.find('error')
        ending = six.u(' \uFFFD\uFFFD')
        assert error is not None
        self.assertRegex(error.text, self.EXPECTED_RE_SAFE)

    def test_error_bad_xml_b_keep(self):
        self.plugin.keep_restricted = True
        test = self.case('test_bad_xml_b')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        error = case.find('error')
        assert error is not None
        self.assertRegex(error.text, self.EXPECTED_RE)

    def test_error_includes_traceback(self):
        test = self.case('test_err')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        error = case.find('error')
        assert error is not None
        assert 'Traceback' in error.text

    def test_skip_includes_skipped(self):
        test = self.case('test_skip')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        skip = case.find('skipped')
        assert skip is not None

    def test_generator_test_name_correct(self):
        gen = generators.Generators(session=self.session)
        gen.register()
        event = events.LoadFromTestCaseEvent(self.loader, self.case)
        self.session.hooks.loadTestsFromTestCase(event)
        cases = event.extraTests
        for case in cases:
            case(self.result)
        xml = self.plugin.tree.findall('testcase')
        self.assertEqual(len(xml), 2)
        self.assertEqual(xml[0].get('name'), 'test_gen:1')
        self.assertEqual(xml[1].get('name'), 'test_gen:2')

    def test_params_test_name_correct(self):
        # param test loading is a bit more complex than generator
        # loading. XXX -- can these be reconciled so they both
        # support exclude and also both support loadTestsFromTestCase?
        plug1 = parameters.Parameters(session=self.session)
        plug1.register()
        plug2 = testcases.TestCaseLoader(session=self.session)
        plug2.register()
        # need module to fire top-level event

        class Mod(object):
            pass

        m = Mod()
        m.Test = self.case
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        for case in event.extraTests:
            case(self.result)
        xml = self.plugin.tree.findall('testcase')
        self.assertEqual(len(xml), 12)
        params = [x for x in xml if x.get('name').startswith('test_params')]
        self.assertEqual(len(params), 3)
        self.assertEqual(params[0].get('name'), 'test_params:1')
        self.assertEqual(params[1].get('name'), 'test_params:2')
        self.assertEqual(params[2].get('name'), 'test_params:3')

    def test_writes_xml_file_at_end(self):
        test = self.case('test')
        test(self.result)
        event = events.StopTestRunEvent(None, self.result, 1, 1)
        self.plugin.stopTestRun(event)
        with open(self.plugin.path, 'r') as fh:
            tree = ET.parse(fh).getroot()
        self.assertEqual(len(tree.findall('testcase')), 1)
        case = tree.find('testcase')
        assert 'time' in case.attrib
        assert 'classname' in case.attrib
        self.assertEqual(case.get('name'), 'test')
        self.assertEqual(tree.get('errors'), '0')
        self.assertEqual(tree.get('failures'), '0')
        self.assertEqual(tree.get('skipped'), '0')
        self.assertEqual(tree.get('tests'), '1')
        assert 'time' in tree.attrib

    def test_xml_file_path_is_not_affected_by_chdir_in_test(self):
        inital_dir = os.getcwd()
        test = self.case('test_chdir')
        test(self.result)
        self.assertEqual(inital_dir,
                         os.path.dirname(os.path.realpath(self.plugin.path)))

    def test_xml_contains_empty_system_err_without_logcapture(self):
        test = self.case('test_with_log')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        system_err = case.find('system-err')
        assert system_err is not None
        assert not system_err.text

    def test_xml_contains_log_message_in_system_err_with_logcapture(self):
        self.logcapture_plugin = logcapture.LogCapture(session=self.session)
        self.logcapture_plugin.register()

        test = self.case('test_with_log')
        test(self.result)
        case = self.plugin.tree.find('testcase')
        system_err = case.find('system-err')
        assert system_err is not None
        assert 'log message' in system_err.text
        assert 'INFO' in system_err.text
