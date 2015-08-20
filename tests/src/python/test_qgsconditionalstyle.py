
# -*- coding: utf-8 -*-
"""QGIS Unit tests for the memory layer provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nathan.Woodrow'
__date__ = '2015-08-11'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsConditionalStyle, QgsFeature, QgsFields, QgsField, QgsExpressionContextUtils
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       unittest,
                       TestCase,
                       compareWkt)
from PyQt4.QtCore import QVariant
#
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsConditionalStyle(TestCase):
    def test_MatchesReturnsTrueForSimpleMatch(self):
        style = QgsConditionalStyle("@value > 10")
        context = QgsExpressionContextUtils.createFeatureBasedContext( QgsFeature(), QgsFields() )
        assert style.matches(20,context)

    def test_MatchesReturnsTrueForComplexMatch(self):
        style = QgsConditionalStyle("@value > 10 and @value = 20")
        context = QgsExpressionContextUtils.createFeatureBasedContext( QgsFeature(), QgsFields() )
        assert style.matches(20,context)

    def test_MatchesTrueForFields(self):
        feature = QgsFeature()
        fields = QgsFields()
        fields.append(QgsField("testfield", QVariant.Int))
        feature.setFields(fields, True)
        feature["testfield"] = 20
        style = QgsConditionalStyle('"testfield" = @value')
        context = QgsExpressionContextUtils.createFeatureBasedContext(feature,fields)
        assert style.matches(20, context)


if __name__ == '__main__':
    unittest.main()
