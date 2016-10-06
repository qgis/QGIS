from nose2 import events, loader, session, util
from nose2.plugins.loader import parameters, testcases
from nose2.tests._common import TestCase
from nose2.tools import cartesian_params, params


class TestParams(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(self.session)
        self.plugin = parameters.Parameters(session=self.session)
        # need testcase loader to make the initial response to load from module
        self.tcl = testcases.TestCaseLoader(session=self.session)

    def test_ignores_ordinary_functions(self):
        class Mod(object):
            pass

        def test():
            pass
        m = Mod()
        m.test = test
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 0)

    def test_can_load_tests_from_parameterized_by_params_functions(self):
        class Mod(object):
            __name__ = 'themod'

        def check(x):
            assert x == 1

        @params(1, 2)
        def test(a):
            check(a)
        m = Mod()
        m.test = test
        test.__module__ = m.__name__
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 2)
        # check that test names are sensible
        self.assertEqual(util.test_name(event.extraTests[0]),
                         'themod.test:1')
        self.assertEqual(util.test_name(event.extraTests[1]),
                         'themod.test:2')

    def test_can_load_tests_from_parameterized_by_cartesian_params_functions(self):
        class Mod(object):
            __name__ = 'themod'

        def check(x, y):
            assert x == y

        @cartesian_params(
            (1, 2),
            (2, 3),
        )
        def test(a, b):
            check(a, b)
        m = Mod()
        m.test = test
        test.__module__ = m.__name__
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 4)
        # check that test names are sensible
        self.assertEqual(util.test_name(event.extraTests[0]),
                         'themod.test:1')
        self.assertEqual(util.test_name(event.extraTests[1]),
                         'themod.test:2')
        self.assertEqual(util.test_name(event.extraTests[2]),
                         'themod.test:3')
        self.assertEqual(util.test_name(event.extraTests[3]),
                         'themod.test:4')

    def test_can_load_tests_from_parameterized_by_params_methods(self):
        class Mod(object):
            __name__ = 'themod'

        class Test(TestCase):

            @params(1, 2)
            def test(self, a):
                assert a == 1
        m = Mod()
        m.Test = Test
        Test.__module__ = m.__name__
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 1)
        self.assertEqual(len(event.extraTests[0]._tests), 2)
        # check that test names are sensible
        self.assertEqual(util.test_name(event.extraTests[0]._tests[0]),
                         'themod.Test.test:1')
        self.assertEqual(util.test_name(event.extraTests[0]._tests[1]),
                         'themod.Test.test:2')

    def test_can_load_tests_from_parameterized_by_cartesian_params_methods(self):
        class Mod(object):
            __name__ = 'themod'

        class Test(TestCase):

            @cartesian_params(
                (1, 2),
                (2, 3),
            )
            def test(self, a, b):
                assert a == b
        m = Mod()
        m.Test = Test
        Test.__module__ = m.__name__
        event = events.LoadFromModuleEvent(self.loader, m)
        self.session.hooks.loadTestsFromModule(event)
        self.assertEqual(len(event.extraTests), 1)
        self.assertEqual(len(event.extraTests[0]._tests), 4)
        # check that test names are sensible
        self.assertEqual(util.test_name(event.extraTests[0]._tests[0]),
                         'themod.Test.test:1')
        self.assertEqual(util.test_name(event.extraTests[0]._tests[1]),
                         'themod.Test.test:2')
        self.assertEqual(util.test_name(event.extraTests[0]._tests[2]),
                         'themod.Test.test:3')
        self.assertEqual(util.test_name(event.extraTests[0]._tests[3]),
                         'themod.Test.test:4')

    def test_params_creates_params_for_function(self):
        @params(
            (1, 2),
            ('a', 'b'),
        )
        def test(a, b):
            assert a == b
        self.assertTupleEqual(tuple(test.paramList), ((1, 2), ('a', 'b')))

    def test_cartesian_params_creates_cartesian_product_of_params_for_function(self):
        @cartesian_params(
            (1, 2),
            ('a', 'b'),
        )
        def test(a, b):
            assert a == b
        self.assertTupleEqual(tuple(test.paramList), ((1, 'a'), (1, 'b'), (2, 'a'), (2, 'b')))

    def test_params_creates_params_for_method(self):
        class Test(TestCase):
            @params(
                (1, 2),
                ('a', 'b'),
            )
            def test(self, a, b):
                assert a == b
        self.assertTupleEqual(tuple(Test.test.paramList), ((1, 2), ('a', 'b')))

    def test_cartesian_params_creates_cartesian_product_of_params_for_method(self):
        class Test(TestCase):
            @cartesian_params(
                (1, 2),
                ('a', 'b'),
            )
            def test(self, a, b):
                assert a == b
        self.assertTupleEqual(tuple(Test.test.paramList), ((1, 'a'), (1, 'b'), (2, 'a'), (2, 'b')))
