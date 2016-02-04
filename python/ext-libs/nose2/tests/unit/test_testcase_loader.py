from nose2.tests._common import TestCase
from nose2.plugins.loader.testcases import TestCaseLoader
from nose2 import events, loader, session


class TestTestCaseLoader(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(session=self.session)
        self.plugin = TestCaseLoader(session=self.session)

        class Mod(object):
            pass
        self.module = Mod()

        class A(TestCase):

            def test(self):
                pass

        class B(TestCase):

            def runTest(self):
                pass

        class C(TestCase):

            def foo(self):
                pass

        class Test(object):

            def test(self):
                pass

        self.module.A = A
        self.module.B = B
        self.module.C = C
        self.module.Test = Test

    def test_can_find_testcases_in_module(self):
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 1)  # A
        self.assertEqual(len(event.extraTests[1]._tests), 1)  # B
        self.assertEqual(len(event.extraTests[2]._tests), 0)  # C

    def test_get_testcase_names_can_override_name_selection(self):
        class FooIsOnlyTest(events.Plugin):

            def getTestCaseNames(self, event):
                event.handled = True
                return ['foo'] if 'foo' in dir(event.testCase) else []
        foo = FooIsOnlyTest(session=self.session)
        foo.register()
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 0)  # A
        self.assertEqual(len(event.extraTests[1]._tests), 1)  # B (runTest)
        self.assertEqual(len(event.extraTests[2]._tests), 1)  # C

    def test_plugins_can_exclude_test_names(self):
        class Excluder(events.Plugin):

            def getTestCaseNames(self, event):
                event.excludedNames.append('test')
        excl = Excluder(session=self.session)
        excl.register()
        event = events.LoadFromModuleEvent(self.loader, self.module)
        result = self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(result, None)
        self.assertEqual(len(event.extraTests), 3)
        self.assertEqual(len(event.extraTests[0]._tests), 0)  # A
        self.assertEqual(len(event.extraTests[1]._tests), 1)  # B (runTest)
        self.assertEqual(len(event.extraTests[2]._tests), 0)  # C
