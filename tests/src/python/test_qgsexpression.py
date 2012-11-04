# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsExpression.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nathan Woodrow'
__date__ = '4/11/2012'
__copyright__ = 'Copyright 2012, The Quantum GIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from utilities import unittest, TestCase
from qgis.utils import qgsfunction
from qgis.core import QgsExpression
from PyQt4.QtCore import QString

class TestQgsExpressionCustomFunctions(TestCase):
	@qgsfunction(1, 'testing', register=False)
	def testfun(values, feature, parent):
		""" Function help """
		return "Testing_%s" % str(values[0].toString())

	@qgsfunction(0, 'testing', register=False)
	def special(values, feature, parent):
		return "test"

	def setUp(self):
		QgsExpression.registerFunction(self.testfun)

	def tearDown(self):
		QgsExpression.unregisterFunction('testfun')

	def testCanBeRegistered(self):
		QgsExpression.registerFunction(self.testfun)
		index = QgsExpression.functionIndex('testfun')
		self.assertTrue(not index == -1)

	def testCanUnregisterFunction(self):
		index = QgsExpression.functionIndex('testfun')
		self.assertTrue(not index == -1)
		QgsExpression.unregisterFunction('testfun')
		index = QgsExpression.functionIndex('testfun')
		self.assertTrue(index == -1)

	def testCanEvaluateFunction(self):
		exp = QgsExpression('testfun(1)')
		result = exp.evaluate().toString()
		self.assertEqual(QString('Testing_1'), result)

	def testZeroArgFunctionsAreSpecialColumns(self):
		special = self.special
		self.assertEqual(special.mName, '$special') 

	def testDecoratorPreservesAttributes(self):
		func = self.testfun
		self.assertEqual(func.mName, 'testfun')
		self.assertEqual(func.mGroup, 'testing')
		self.assertEqual(func.mParams, 1)

if __name__ == "__main__":
	unittest.main()