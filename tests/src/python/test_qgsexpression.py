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

import qgis
from utilities import unittest, TestCase
from qgis.utils import qgsfunction
from qgis.core import QgsExpression

class TestQgsExpressionCustomFunctions(TestCase):
	@qgsfunction(1, 'testing', register=False)
	def testfun(values, feature, parent):
		""" Function help """
		return "Testing_%s" % values[0]

	@qgsfunction(0, 'testing', register=False)
	def special(values, feature, parent):
		return "test"

	@qgsfunction(1, 'testing', register=False)
	def sqrt(values, feature, parent):
		pass

	def tearDown(self):
		QgsExpression.unregisterFunction('testfun')

	def testCanBeRegistered(self):
		QgsExpression.registerFunction(self.testfun)
		index = QgsExpression.functionIndex('testfun')
		self.assertTrue(not index == -1)

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

	def testZeroArgFunctionsAreSpecialColumns(self):
		special = self.special
		self.assertEqual(special.name(), '$special')

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

	def testCantOverrideBuiltinsWithUnregister(self):
		success = QgsExpression.unregisterFunction("sqrt")
		self.assertFalse(success)

if __name__ == "__main__":
	unittest.main()
