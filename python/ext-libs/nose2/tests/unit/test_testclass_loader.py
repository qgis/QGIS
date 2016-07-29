from nose2.tests._common import TestCase
from nose2.plugins.loader.testclasses import TestClassLoader
from nose2 import events, loader, session

class TestTestClassLoader(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(session=self.session)
        self.plugin = TestClassLoader(session=self.session)

        class Mod(object):
            pass
        self.module = Mod()

        class TestA(object):

            def test(self):
                pass

        class TestB(object):

            def runTest(self):
                pass

        class TestC(object):

            def foo(self):
                pass

        class Test(TestCase):

            def test(self):
                pass

        self.module.TestA = TestA
        self.module.TestB = TestB
        self.module.TestC = TestC
        self.module.Test = Test

    def test_can_find_testclasses_in_module(self):
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 1)  # TestA
        self.assertEqual(len(event.extraTests[1]._tests), 0)  # TestB
        self.assertEqual(len(event.extraTests[2]._tests), 0)  # TestC

    def test_get_testmethod_names_can_override_name_selection(self):
        class FooIsOnlyTest(events.Plugin):

            def getTestMethodNames(self, event):
                event.handled = True
                return ['foo'] if 'foo' in dir(event.testCase) else []
        foo = FooIsOnlyTest(session=self.session)
        foo.register()
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 0)  # TestA
        self.assertEqual(len(event.extraTests[1]._tests), 0)  # TestB
        self.assertEqual(len(event.extraTests[2]._tests), 1)  # TestC

    def test_plugins_can_exclude_test_names(self):
        class Excluder(events.Plugin):

            def getTestMethodNames(self, event):
                event.excludedNames.append('test')
        excl = Excluder(session=self.session)
        excl.register()
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 0)  # TestA
        self.assertEqual(len(event.extraTests[1]._tests), 0)  # TestB
        self.assertEqual(len(event.extraTests[2]._tests), 0)  # TestC


class TestFailingTestClassLoader(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(session=self.session)
        self.plugin = TestClassLoader(session=self.session)

        class Mod(object):
            pass
        self.module = Mod()

        class TestA(object):
            def __init__(self):
                raise RuntimeError('Something bad happened!')

            def test(self):
                pass

        self.module.TestA = TestA

    def test_can_find_testclasses_in_module(self):
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 1)
        self.assertEqual(len(event.extraTests[0]._tests), 1)  # TestA
        self.assertEqual(event.extraTests[0]._tests[0].__class__.__name__, 'LoadTestsFailure')
