from nose2.tests._common import TestCase
from nose2.plugins import failfast
from nose2 import result, session
from nose2.compat import unittest


class TestFailFast(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.result = result.PluggableTestResult(self.session)
        self.plugin = failfast.FailFast(session=self.session)
        self.plugin.register()

        class Test(TestCase):

            def test(self):
                pass

            def test_err(self):
                raise Exception("oops")

            def test_fail(self):
                assert False

            @unittest.expectedFailure
            def test_fail_expected(self):
                assert False

            @unittest.skipIf(True, "Always skip")
            def test_skip(self):
                pass
        self.case = Test

    def test_sets_shouldstop_on_unexpected_error(self):
        test = self.case('test_err')
        test(self.result)
        assert self.result.shouldStop

    def test_sets_shouldstop_on_unexpected_fail(self):
        test = self.case('test_fail')
        test(self.result)
        assert self.result.shouldStop

    def test_does_not_set_shouldstop_on_expected_fail(self):
        test = self.case('test_fail_expected')
        test(self.result)
        assert not self.result.shouldStop

    def test_does_not_set_shouldstop_on_success(self):
        test = self.case('test')
        test(self.result)
        assert not self.result.shouldStop

    def test_does_not_set_shouldstop_on_skip(self):
        test = self.case('test_skip')
        test(self.result)
        assert not self.result.shouldStop
