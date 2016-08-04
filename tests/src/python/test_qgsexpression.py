# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsExpression.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nathan Woodrow'
__date__ = '4/11/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest
from qgis.utils import qgsfunction
from qgis.core import QgsExpression, QgsFeatureRequest


class TestQgsExpressionCustomFunctions(unittest.TestCase):

    @qgsfunction(1, 'testing', register=False)
    def testfun(values, feature, parent):
        """ Function help """
        return "Testing_%s" % values[0]

    @qgsfunction(args="auto", group='testing', register=False)
    def autocount(value1, value2, value3, feature, parent):
        pass

    @qgsfunction(args="auto", group='testing', register=False)
    def expandargs(value1, value2, value3, feature, parent):
        return value1, value2, value3

    @qgsfunction(args=0, group='testing', register=False)
    def special(values, feature, parent):
        return "test"

    @qgsfunction(1, 'testing', register=False)
    def sqrt(values, feature, parent):
        pass

    @qgsfunction(1, 'testing', register=False, usesgeometry=True)
    def geomtest(values, feature, parent):
        pass

    @qgsfunction(args=0, group='testing', register=False)
    def no_referenced_columns_set(values, feature, parent):
        return 1

    @qgsfunction(args=0, group='testing', register=False, referenced_columns=['a', 'b'])
    def referenced_columns_set(values, feature, parent):
        return 2

    def tearDown(self):
        QgsExpression.unregisterFunction('testfun')

    def testCanBeRegistered(self):
        QgsExpression.registerFunction(self.testfun)
        index = QgsExpression.functionIndex('testfun')
        self.assertTrue(not index == -1)

    def testAutoCountsCorrectArgs(self):
        function = self.autocount
        args = function.params()
        self.assertEqual(args, 3)

    def testAutoArgsAreExpanded(self):
        function = self.expandargs
        args = function.params()
        self.assertEqual(args, 3)
        values = [1, 2, 3]
        exp = QgsExpression("")
        result = function.func(values, None, exp)
        # Make sure there is no eval error
        self.assertEqual(exp.evalErrorString(), "")
        self.assertEqual(result, (1, 2, 3))

    def testCanUnregisterFunction(self):
        QgsExpression.registerFunction(self.testfun)
        index = QgsExpression.functionIndex('testfun')
        self.assertTrue(not index == -1)
        error = QgsExpression.unregisterFunction('testfun')
        self.assertTrue(error)
        index = QgsExpression.functionIndex('testfun')
        self.assertTrue(index == -1)

    def testCanEvaluateFunction(self):
        QgsExpression.registerFunction(self.testfun)
        exp = QgsExpression('testfun(1)')
        result = exp.evaluate()
        self.assertEqual('Testing_1', result)

    def testZeroArgFunctionsTakeNoArgs(self):
        QgsExpression.registerFunction(self.special)
        special = self.special
        self.assertEqual(special.name(), 'special')
        exp = QgsExpression('special()')
        result = exp.evaluate()
        self.assertEqual('test', result)

    def testDecoratorPreservesAttributes(self):
        func = self.testfun
        self.assertEqual(func.name(), 'testfun')
        self.assertEqual(func.group(), 'testing')
        self.assertEqual(func.params(), 1)

    def testCantReregister(self):
        QgsExpression.registerFunction(self.testfun)
        success = QgsExpression.registerFunction(self.testfun)
        self.assertFalse(success)

    def testCanReregisterAfterUnregister(self):
        QgsExpression.registerFunction(self.testfun)
        QgsExpression.unregisterFunction("testfun")
        success = QgsExpression.registerFunction(self.testfun)
        self.assertTrue(success)

    def testCantOverrideBuiltinsWithRegister(self):
        success = QgsExpression.registerFunction(self.sqrt)
        self.assertFalse(success)

    def testCanRegisterGeometryFunction(self):
        success = QgsExpression.registerFunction(self.geomtest)
        self.assertTrue(success)

    def testReferencedColumnsNoSet(self):
        success = QgsExpression.registerFunction(self.no_referenced_columns_set)
        exp = QgsExpression('no_referenced_columns_set()')
        self.assertEqual(exp.referencedColumns(), [QgsFeatureRequest.AllAttributes])

    def testReferencedColumnsSet(self):
        success = QgsExpression.registerFunction(self.referenced_columns_set)
        exp = QgsExpression('referenced_columns_set()')
        self.assertEqual(set(exp.referencedColumns()), set(['a', 'b']))

    def testCantOverrideBuiltinsWithUnregister(self):
        success = QgsExpression.unregisterFunction("sqrt")
        self.assertFalse(success)

    def testDump(self):
        for txt in [
            "id",
            u"idä",
            "\"id abc\"",
            "\"id	abc\"",
            "  abc   ",
            " /* co */ da ",
        ]:
            self.assertEqual(txt, QgsExpression(txt).expression())

    def testBlockComment(self):
        expressions = {
            "'test' /* comment */": 'test',
            "/* comment */'test'": 'test',
            "/* comment */'test*/'": 'test*/',
            "/** comment */'test*/'": 'test*/',
            "/* comment **/'test*/' /* comment */": 'test*/',
            "'test/*'/* comment */": 'test/*',
            """/**
            comment
            **/
            'test*/'""": 'test*/',
            """'test*/'
            /**
            comment
            **/""": 'test*/'
        }
        for e, exp_res in expressions.items():
            exp = QgsExpression(e)
            result = exp.evaluate()
            self.assertEqual(exp_res, result)

    def testComment(self):
        expressions = {
            "'test' -- comment\n": 'test',
            "'test--'\n": 'test--',
            "'--test'\n": '--test',
            "'test' -- comment": 'test',
            "'test--'": 'test--',
            "'--test'": '--test',
        }
        for e, exp_res in expressions.items():
            exp = QgsExpression(e)
            result = exp.evaluate()
            self.assertEqual(exp_res, result)

if __name__ == "__main__":
    unittest.main()
