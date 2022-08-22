
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

from qgis.core import (QgsConditionalStyle,
                       QgsFeature,
                       QgsFields,
                       QgsField,
                       QgsExpressionContextUtils,
                       QgsConditionalLayerStyles,
                       QgsMarkerSymbol
                       )
from qgis.testing import (start_app,
                          unittest,
                          )
from utilities import unitTestDataPath
from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QFont, QColor
from qgis.PyQt.QtTest import QSignalSpy

#
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsConditionalStyle(unittest.TestCase):

    def new_context(self):
        feature = QgsFeature()
        fields = QgsFields()
        fields.append(QgsField("testfield", QVariant.Int))
        feature.setFields(fields, True)
        feature["testfield"] = 20
        context = QgsExpressionContextUtils.createFeatureBasedContext(feature, fields)
        return context

    def testDefaultStyle(self):
        style = QgsConditionalStyle()
        self.assertFalse(style.isValid())
        style.setName('x')
        self.assertTrue(style.isValid())
        self.assertFalse(style.textColor().isValid())
        self.assertFalse(style.backgroundColor().isValid())

    def test_MatchesReturnsTrueForSimpleMatch(self):
        style = QgsConditionalStyle("@value > 10")
        context = QgsExpressionContextUtils.createFeatureBasedContext(QgsFeature(), QgsFields())
        self.assertTrue(style.matches(20, context))

    def test_MatchesReturnsTrueForComplexMatch(self):
        style = QgsConditionalStyle("@value > 10 and @value = 20")
        context = QgsExpressionContextUtils.createFeatureBasedContext(QgsFeature(), QgsFields())
        self.assertTrue(style.matches(20, context))

    def test_MatchesTrueForFields(self):
        style = QgsConditionalStyle('"testfield" = @value')
        context = self.new_context()
        self.assertTrue(style.matches(20, context))

    def test_MatchingStylesReturnsListOfCorrectStyles(self):
        styles = []
        style = QgsConditionalStyle("@value > 10")
        style.setName("1")
        styles.append(style)
        style = QgsConditionalStyle("@value > 10")
        style.setName("2")
        styles.append(style)
        style = QgsConditionalStyle("@value < 5")
        style.setName("3")
        styles.append(style)
        context = self.new_context()
        out = QgsConditionalStyle.matchingConditionalStyles(styles, 20, context)
        self.assertEqual([o.name() for o in out], ["1", "2"])

    def testStyleCompression(self):
        style = QgsConditionalStyle.compressStyles([])
        self.assertFalse(style.isValid())
        # invalid styles should not be compressed
        style = QgsConditionalStyle.compressStyles([QgsConditionalStyle(), QgsConditionalStyle()])
        self.assertFalse(style.isValid())

        c = QgsConditionalStyle()
        c.setBackgroundColor(QColor(255, 0, 0))
        c2 = QgsConditionalStyle()
        c2.setTextColor(QColor(0, 255, 0))
        style = QgsConditionalStyle.compressStyles([c])
        self.assertTrue(style.isValid())
        self.assertEqual(style.backgroundColor(), QColor(255, 0, 0))
        self.assertFalse(style.textColor().isValid())
        style = QgsConditionalStyle.compressStyles([c2])
        self.assertTrue(style.isValid())
        self.assertFalse(style.backgroundColor().isValid())
        self.assertEqual(style.textColor(), QColor(0, 255, 0))
        style = QgsConditionalStyle.compressStyles([c, c2])
        self.assertTrue(style.isValid())
        self.assertEqual(style.backgroundColor(), QColor(255, 0, 0))
        self.assertEqual(style.textColor(), QColor(0, 255, 0))

    def testEquality(self):
        c = QgsConditionalStyle()
        c2 = QgsConditionalStyle()
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setName('n')
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setName('n')
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setRule('1=1')
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setRule('1=1')
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        f = QFont()
        f.setPointSize(12)
        c.setFont(f)
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setFont(f)
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setBackgroundColor(QColor(255, 0, 0))
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setBackgroundColor(QColor(255, 0, 0))
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setTextColor(QColor(0, 255, 0))
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setTextColor(QColor(0, 255, 0))
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setSymbol(QgsMarkerSymbol.createSimple({}))
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)
        c2.setSymbol(c.symbol().clone())
        self.assertEqual(c, c2)
        self.assertFalse(c != c2)
        c.setSymbol(None)
        self.assertNotEqual(c, c2)
        self.assertTrue(c != c2)

    def testLayerStyles(self):
        styles = QgsConditionalLayerStyles()
        self.assertFalse(styles.rowStyles())
        self.assertFalse(styles.fieldStyles('test'))
        spy = QSignalSpy(styles.changed)

        styles.setRowStyles([QgsConditionalStyle("@value > 10"), QgsConditionalStyle("@value > 20")])
        self.assertEqual(len(spy), 1)
        self.assertEqual(styles.rowStyles(), [QgsConditionalStyle("@value > 10"), QgsConditionalStyle("@value > 20")])
        styles.setRowStyles(styles.rowStyles())
        self.assertEqual(len(spy), 1)

        styles.setFieldStyles('test', [QgsConditionalStyle("@value > 30"), QgsConditionalStyle("@value > 40")])
        self.assertEqual(len(spy), 2)
        self.assertEqual(styles.fieldStyles('test'), [QgsConditionalStyle("@value > 30"), QgsConditionalStyle("@value > 40")])
        styles.setFieldStyles('test', styles.fieldStyles('test'))
        self.assertEqual(len(spy), 2)
        self.assertFalse(styles.fieldStyles('test2'))
        styles.setFieldStyles('test2', [QgsConditionalStyle("@value > 50")])
        self.assertEqual(len(spy), 3)
        self.assertEqual(styles.fieldStyles('test'), [QgsConditionalStyle("@value > 30"), QgsConditionalStyle("@value > 40")])
        self.assertEqual(styles.fieldStyles('test2'), [QgsConditionalStyle("@value > 50")])

    def testRequiresGeometry(self):

        styles = QgsConditionalLayerStyles()
        styles.setRowStyles([QgsConditionalStyle("@value > 10")])
        self.assertFalse(styles.rulesNeedGeometry())
        styles.setRowStyles([QgsConditionalStyle("@value > 10"), QgsConditionalStyle('$geometry IS NULL')])
        self.assertTrue(styles.rulesNeedGeometry())
        styles.setRowStyles([QgsConditionalStyle('$geometry IS NULL'), QgsConditionalStyle("@value > 10")])
        self.assertTrue(styles.rulesNeedGeometry())


if __name__ == '__main__':
    unittest.main()
