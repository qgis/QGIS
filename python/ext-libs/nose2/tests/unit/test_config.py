from nose2 import config
from nose2.compat import unittest


class TestConfig(unittest.TestCase):

    def setUp(self):
        self.conf = config.Config([
            ('a', ' 1 '), ('b', '  x\n  y '), ('c', '0'),
            ('d', '123')])

    def test_as_int(self):
        self.assertEqual(self.conf.as_int('a'), 1)

    def test_as_str(self):
        self.assertEqual(self.conf.as_str('a'), '1')
        self.assertEqual(self.conf.as_str('b'), 'x\n  y')
        self.assertEqual(self.conf.as_str('missing', 'default'), 'default')

    def test_as_bool(self):
        self.assertEqual(self.conf.as_bool('a'), True)
        self.assertEqual(self.conf.as_bool('c'), False)

    def test_as_float(self):
        self.assertAlmostEqual(self.conf.as_float('a'), 1.0)

    def test_as_list(self):
        self.assertEqual(self.conf.as_list('b'), ['x', 'y'])
        self.assertEqual(self.conf.as_list('a'), ['1'])
        self.assertEqual(self.conf.as_list('d'), ['123'])
