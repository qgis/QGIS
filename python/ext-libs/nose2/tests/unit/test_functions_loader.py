from nose2.compat import unittest
from nose2 import events, loader, session
from nose2.plugins.loader import functions
from nose2.tests._common import TestCase


class TestFunctionLoader(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(self.session)
        self.plugin = functions.Functions(session=self.session)

    def test_can_load_test_functions_from_module(self):
        class Mod(object):
            pass

        def test():
            pass
        m = Mod()
        m.test = test
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 1)
        assert isinstance(event.extraTests[0], unittest.FunctionTestCase)

    def test_ignores_generator_functions(self):
        class Mod(object):
            pass

        def test():
            yield
        m = Mod()
        m.test = test
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 0)

    def test_ignores_functions_that_take_args(self):
        class Mod(object):
            pass

        def test(a):
            pass
        m = Mod()
        m.test = test
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 0)
