import logging

from nose2.tests._common import TestCase
from nose2.plugins import debugger
from nose2 import events, result, session


class NullHandler(logging.Handler):

    def emit(self, record):
        pass


class StubPdb(object):

    def __init__(self):
        self.called = False
        self.tb = None

    def post_mortem(self, tb):
        self.called = True
        self.tb = tb


class NoInteraction(events.Plugin):

    def beforeInteraction(self, event):
        event.handled = True
        return False


class TestDebugger(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.plugin = debugger.Debugger(session=self.session)
        self.result = result.PluggableTestResult(self.session)

        class Test(TestCase):

            def test(self):
                pass

            def test_err(self):
                raise Exception("oops")

            def test_fail(self):
                assert False
        self.case = Test

        self.pdb = self.plugin.pdb
        self.plugin.pdb = StubPdb()

        self.plugin.register()

        super(TestCase, self).setUp()

    def tearDown(self):
        self.plugin.pdb = self.pdb
        super(TestCase, self).tearDown()

    def test_does_not_call_pdb_on_success(self):
        test = self.case('test')
        test(self.result)
        assert not self.plugin.pdb.called, "pdb was called on success"

    def test_does_call_pdb_on_error(self):
        test = self.case('test_err')
        test(self.result)
        assert self.plugin.pdb.called, "pdb was not called on error"

    def test_does_call_pdb_on_failure(self):
        test = self.case('test_fail')
        test(self.result)
        assert self.plugin.pdb.called, "pdb was not called on failure"

    def test_does_not_call_pdb_on_failure_if_config_set(self):
        self.plugin.errorsOnly = True
        test = self.case('test_fail')
        test(self.result)
        assert not self.plugin.pdb.called, \
            "pdb was called on failure when errorsOnly set"

    def test_other_plugins_can_prevent_interaction(self):
        # prevent 'no logger for x' warnings
        debugger.log.addHandler(NullHandler())
        nono = NoInteraction(session=self.session)
        nono.register()
        test = self.case('test_err')
        test(self.result)
        assert not self.plugin.pdb.called, \
            "pdb was called despite beforeInteraction returning False"
