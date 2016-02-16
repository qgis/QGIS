from nose2.compat import unittest
from nose2.plugins import layers
from nose2 import events, loader, session
from nose2.tests._common import TestCase


class TestLayers(TestCase):
    tags = ['unit']

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(session=self.session)
        self.session.testLoader = self.loader
        self.plugin = layers.Layers(session=self.session)

    def test_simple_layer_inheritance(self):
        class L1(object):
            pass

        class L2(L1):
            pass

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T2(unittest.TestCase):
            layer = L2

            def test(self):
                pass

        suite = unittest.TestSuite([T2('test'), T1('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = [['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T2)']]]
        self.assertEqual(self.names(event.suite), expect)

    def test_multiple_inheritance(self):
        class L1(object):
            pass

        class L2(L1):
            pass

        class L3(L1):
            pass

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T2(unittest.TestCase):
            layer = L2

            def test(self):
                pass

        class T3(unittest.TestCase):
            layer = L3

            def test(self):
                pass

        suite = unittest.TestSuite([T2('test'), T1('test'), T3('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = [['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T2)'],
                   ['test (nose2.tests.unit.test_layers_plugin.T3)']]]
        self.assertEqual(self.names(event.suite), expect)

    def test_deep_inheritance(self):
        class L1(object):
            pass

        class L2(L1):
            pass

        class L3(L1):
            pass

        class L4(L2, L1):
            pass

        class L5(L4):
            pass

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T2(unittest.TestCase):
            layer = L2

            def test(self):
                pass

        class T3(unittest.TestCase):
            layer = L3

            def test(self):
                pass

        class T4(unittest.TestCase):
            layer = L4

            def test(self):
                pass

        class T5(unittest.TestCase):
            layer = L5

            def test(self):
                pass

        suite = unittest.TestSuite([T2('test'), T1('test'), T3('test'),
                                    T4('test'), T5('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = [['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T2)',
                    ['test (nose2.tests.unit.test_layers_plugin.T4)',
                     ['test (nose2.tests.unit.test_layers_plugin.T5)']]],
                   ['test (nose2.tests.unit.test_layers_plugin.T3)']]]
        self.assertEqual(self.names(event.suite), expect)

    def test_mixed_layers_no_layers(self):
        class L1(object):
            pass

        class L2(L1):
            pass

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T2(unittest.TestCase):
            layer = L2

            def test(self):
                pass

        class T3(unittest.TestCase):

            def test(self):
                pass

        suite = unittest.TestSuite([T2('test'), T1('test'), T3('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = ['test (nose2.tests.unit.test_layers_plugin.T3)',
                  ['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T2)']]]
        self.assertEqual(self.names(event.suite), expect)

    def test_ordered_layers(self):
        class L1(object):
            pass

        class L2(L1):
            position = 1

        class L3(L1):
            position = 2

        class L4(L1):
            position = 3

        class L5(L2):
            position = 4

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T2(unittest.TestCase):
            layer = L2

            def test(self):
                pass

        class T3(unittest.TestCase):
            layer = L3

            def test(self):
                pass

        class T4(unittest.TestCase):
            layer = L4

            def test(self):
                pass

        class T5(unittest.TestCase):
            layer = L5

            def test(self):
                pass
        suite = unittest.TestSuite([T2('test'), T1('test'),
                                    T3('test'), T4('test'), T5('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = [['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T2)',
                    ['test (nose2.tests.unit.test_layers_plugin.T5)', ]],
                   ['test (nose2.tests.unit.test_layers_plugin.T3)', ],
                   ['test (nose2.tests.unit.test_layers_plugin.T4)', ]]]
        self.assertEqual(self.names(event.suite), expect)

    def test_mixin_inheritance(self):
        class L1(object):
            pass

        class L2(object):  # a mixin, doesn't share a base w/L1
            pass

        class L3(L1):
            pass

        class L4(L3):
            pass

        class L5(L4):
            pass

        class L6(L2):
            mixins = (L4,)

        class T1(unittest.TestCase):
            layer = L1

            def test(self):
                pass

        class T3(unittest.TestCase):
            layer = L3

            def test(self):
                pass

        class T4(unittest.TestCase):
            layer = L4

            def test(self):
                pass

        class T5(unittest.TestCase):
            layer = L5

            def test(self):
                pass

        class T6(unittest.TestCase):
            layer = L6

            def test(self):
                pass
        suite = unittest.TestSuite([T6('test'), T1('test'),
                                    T3('test'), T4('test'), T5('test')])
        event = events.StartTestRunEvent(None, suite, None, 0, None)
        self.plugin.startTestRun(event)
        expect = [['test (nose2.tests.unit.test_layers_plugin.T1)',
                   ['test (nose2.tests.unit.test_layers_plugin.T3)',
                    ['test (nose2.tests.unit.test_layers_plugin.T4)',
                     [['test (nose2.tests.unit.test_layers_plugin.T6)']],
                 ['test (nose2.tests.unit.test_layers_plugin.T5)', ]]]]]
        self.assertEqual(self.names(event.suite), expect)

    def names(self, suite):
        return [n for n in self.iternames(suite)]

    def iternames(self, suite):
        for t in suite:
            if isinstance(t, unittest.TestCase):
                yield str(t)
            else:
                yield [n for n in self.iternames(t)]

    def _listset(self, l):
        n = set([])
        for t in l:
            if isinstance(t, list):
                n.add(self._listset(t))
            else:
                n.add(t)
        return frozenset(n)
