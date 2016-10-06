"""
This module contains some code copied from unittest2 and other
code developed in reference to unittest2.

unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
Rights Reserved. See: http://docs.python.org/license.html

"""
import itertools

__unittest = True


def cartesian_params(*paramList):
    """Make a test function or method parameterized by cartesian product
    of parameters

    .. code-block :: python

      import unittest

      from nose2.tools import cartesian_params


      @cartesian_params((1, 2, 3), ('a', 'b'))
      def test_nums(num, char):
          assert num < ord(char)


      class Test(unittest.TestCase):

          @cartesian_params((1, 2, 3), ('a', 'b'))
          def test_less_than(self, num, char):
              self.assertLess(num, ord(char))

    Parameters in the list must be defined as iterable objects such as
    tuple or list.

    """
    def decorator(func):
        func.paramList = itertools.product(*paramList)
        return func
    return decorator


def params(*paramList):
    """Make a test function or method parameterized by parameters.

    .. code-block :: python

      import unittest

      from nose2.tools import params


      @params(1, 2, 3)
      def test_nums(num):
          assert num < 4


      class Test(unittest.TestCase):

          @params((1, 2), (2, 3), (4, 5))
          def test_less_than(self, a, b):
              assert a < b

    Parameters in the list may be defined as simple values, or as
    tuples. To pass a tuple as a simple value, wrap it in another tuple.

    """
    def decorator(func):
        func.paramList = paramList
        return func
    return decorator
