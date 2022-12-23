# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCategorizedSymbolRenderer

From build dir, run: ctest -R PyQgsCategorizedSymbolRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

import os

from qgis.testing import unittest, start_app
from qgis.core import (QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsMarkerSymbol,
                       QgsLineSymbol,
                       QgsFillSymbol,
                       QgsField,
                       QgsFields,
                       QgsFeature,
                       QgsRenderContext,
                       QgsSymbol,
                       QgsStyle,
                       QgsVectorLayer,
                       QgsEditorWidgetSetup,
                       QgsReadWriteContext,
                       QgsProject,
                       QgsSimpleMarkerSymbolLayer,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsMapSettings,
                       QgsRectangle,
                       QgsRenderContext,
                       QgsEmbeddedSymbolRenderer,
                       QgsGeometry
                       )
from qgis.PyQt.QtCore import Qt, QVariant, QSize, QLocale, QTemporaryDir
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


def createMarkerSymbol():
    symbol = QgsMarkerSymbol.createSimple({
        "color": "100,150,50",
        "name": "square",
        "size": "3.0"
    })
    return symbol


def createLineSymbol():
    symbol = QgsLineSymbol.createSimple({
        "color": "100,150,50"
    })
    return symbol


def createFillSymbol():
    symbol = QgsFillSymbol.createSimple({
        "color": "100,150,50"
    })
    return symbol


class TestQgsCategorizedSymbolRenderer(unittest.TestCase):

    def testFilter(self):
        """Test filter creation"""
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('field')

        renderer.addCategory(QgsRendererCategory('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        # add default category
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))

        fields = QgsFields()
        fields.append(QgsField('field', QVariant.String))
        fields.append(QgsField('num', QVariant.Double))

        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b','c') OR (\"field\") IS NULL")
        # remove default category
        assert renderer.updateCategoryRenderState(3, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        # add back other categories, leaving default disabled
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b')")
        assert renderer.updateCategoryRenderState(2, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b','c')")

        renderer.deleteAllCategories()
        # just default category
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), 'FALSE')

        renderer.deleteAllCategories()
        # no default category
        renderer.addCategory(QgsRendererCategory('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b','c')")
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('b','c')")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('b')")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        renderer.deleteAllCategories()
        renderer.setClassAttribute('num')
        # numeric categories
        renderer.addCategory(QgsRendererCategory(1, createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory(2, createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory(3, createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), '(\"num\") IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (2,3)")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (2)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        # with value lists
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('field')

        renderer.addCategory(QgsRendererCategory(['a', 'b'], createMarkerSymbol(), 'ab'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b','c') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b','c')")
        assert renderer.updateCategoryRenderState(1, False)
        assert renderer.updateCategoryRenderState(2, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('c') OR (\"field\") IS NULL")
        renderer.deleteAllCategories()
        renderer.setClassAttribute('num')
        renderer.addCategory(QgsRendererCategory([1, 2], createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory(3, createMarkerSymbol(), 'b'))
        self.assertEqual(renderer.filter(fields), '(\"num\") IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (3)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (1,2)")

    def testFilterExpression(self):
        """Test filter creation with expression"""
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('field + field2')

        renderer.addCategory(QgsRendererCategory('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        # add default category
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))

        fields = QgsFields()
        fields.append(QgsField('field', QVariant.String))

        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a') OR (field + field2) IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b') OR (field + field2) IS NULL")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b','c') OR (field + field2) IS NULL")
        # remove default category
        assert renderer.updateCategoryRenderState(3, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        # add back other categories, leaving default disabled
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b')")
        assert renderer.updateCategoryRenderState(2, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b','c')")

        renderer.deleteAllCategories()
        # just default category
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), 'FALSE')

        renderer.deleteAllCategories()
        # no default category
        renderer.addCategory(QgsRendererCategory('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b','c')")
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('b','c')")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('b')")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        renderer.deleteAllCategories()
        # numeric categories
        renderer.addCategory(QgsRendererCategory(1, createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory(2, createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategory(3, createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), '(field + field2) IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (2,3)")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (2)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        # with value lists
        renderer.deleteAllCategories()
        renderer.addCategory(QgsRendererCategory(['a', 'b'], createMarkerSymbol(), 'ab'))
        renderer.addCategory(QgsRendererCategory('c', createMarkerSymbol(), 'c'))
        renderer.addCategory(QgsRendererCategory('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b') OR (field + field2) IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b','c') OR (field + field2) IS NULL")
        # remove default category
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        # add back other categories, leaving default disabled
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b','c')")
        renderer.deleteAllCategories()
        # numeric categories
        renderer.addCategory(QgsRendererCategory([1, 2], createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategory(3, createMarkerSymbol(), 'b'))
        self.assertEqual(renderer.filter(fields), '(field + field2) IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (3)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (1,2)")

    def testSymbolForValue(self):
        """Test symbolForValue"""
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('field')

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 0, 0))
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        symbol_b.setColor(QColor(0, 255, 0))
        renderer.addCategory(QgsRendererCategory('b', symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        symbol_c.setColor(QColor(0, 0, 255))
        renderer.addCategory(QgsRendererCategory('c', symbol_c, 'c', False))
        symbol_d = createMarkerSymbol()
        symbol_d.setColor(QColor(255, 0, 255))
        renderer.addCategory(QgsRendererCategory(['d', 'e'], symbol_d, 'de'))

        # add default category
        default_symbol = createMarkerSymbol()
        default_symbol.setColor(QColor(255, 255, 255))
        renderer.addCategory(QgsRendererCategory('', default_symbol, 'default'))

        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())

        symbol, ok = renderer.symbolForValue2('a')
        self.assertEqual(symbol.color(), QColor(255, 0, 0))
        self.assertTrue(ok)
        symbol, ok = renderer.symbolForValue2('b')
        self.assertEqual(symbol.color(), QColor(0, 255, 0))
        self.assertTrue(ok)

        # hidden category
        symbol, ok = renderer.symbolForValue2('c')
        self.assertIsNone(symbol)
        self.assertTrue(ok)

        # list
        symbol, ok = renderer.symbolForValue2('d')
        self.assertEqual(symbol.color(), QColor(255, 0, 255))
        self.assertTrue(ok)
        symbol, ok = renderer.symbolForValue2('e')
        self.assertEqual(symbol.color(), QColor(255, 0, 255))
        self.assertTrue(ok)

        # no matching category
        symbol, ok = renderer.symbolForValue2('xxxx')
        self.assertIsNone(symbol)
        self.assertFalse(ok)

        renderer.stopRender(context)

    def testOriginalSymbolForFeature(self):
        # test renderer with features
        fields = QgsFields()
        fields.append(QgsField('x'))

        # setup renderer
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('x')

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 0, 0))
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        symbol_b.setColor(QColor(0, 255, 0))
        renderer.addCategory(QgsRendererCategory('b', symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        symbol_c.setColor(QColor(0, 0, 255))
        renderer.addCategory(QgsRendererCategory('c', symbol_c, 'c', False))
        symbol_d = createMarkerSymbol()
        symbol_d.setColor(QColor(255, 0, 255))
        renderer.addCategory(QgsRendererCategory(['d', 'e'], symbol_d, 'de'))
        # add default category
        default_symbol = createMarkerSymbol()
        default_symbol.setColor(QColor(255, 255, 255))
        renderer.addCategory(QgsRendererCategory('', default_symbol, 'default'))

        context = QgsRenderContext()
        renderer.startRender(context, fields)

        f = QgsFeature(fields)
        f.setAttributes(['a'])

        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(255, 0, 0))

        f.setAttributes(['b'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(0, 255, 0))

        # list
        f.setAttributes(['d'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(255, 0, 255))
        f.setAttributes(['e'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(255, 0, 255))

        # hidden category
        f.setAttributes(['c'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertIsNone(symbol)

        # no matching category
        f.setAttributes(['xxx'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(255, 255, 255))  # default symbol

        renderer.stopRender(context)

    def testLegendKeysWhileCounting(self):
        # test determining legend keys for features, while counting features
        fields = QgsFields()
        fields.append(QgsField('x'))

        # setup renderer
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('x')

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 0, 0))
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        symbol_b.setColor(QColor(0, 255, 0))
        renderer.addCategory(QgsRendererCategory('b', symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        symbol_c.setColor(QColor(0, 0, 255))
        renderer.addCategory(QgsRendererCategory('c', symbol_c, 'c', False))
        symbol_d = createMarkerSymbol()
        symbol_d.setColor(QColor(255, 0, 255))
        renderer.addCategory(QgsRendererCategory(['d', 'e'], symbol_d, 'de'))
        # add default category
        default_symbol = createMarkerSymbol()
        default_symbol.setColor(QColor(255, 255, 255))
        renderer.addCategory(QgsRendererCategory('', default_symbol, 'default'))

        context = QgsRenderContext()
        context.setRendererScale(0)  # simulate counting
        renderer.startRender(context, fields)

        f = QgsFeature(fields)
        f.setAttributes(['a'])

        keys = renderer.legendKeysForFeature(f, context)
        self.assertEqual(keys, {'0'})

        f.setAttributes(['b'])
        keys = renderer.legendKeysForFeature(f, context)
        self.assertEqual(keys, {'1'})

        # hidden category, should still return keys
        f.setAttributes(['c'])
        keys = renderer.legendKeysForFeature(f, context)
        self.assertEqual(keys, {'2'})

        # list
        f.setAttributes(['d'])
        keys = renderer.legendKeysForFeature(f, context)
        self.assertEqual(keys, {'3'})
        f.setAttributes(['e'])
        keys = renderer.legendKeysForFeature(f, context)
        self.assertEqual(keys, {'3'})

        # no matching category
        f.setAttributes(['xxx'])
        keys = renderer.legendKeysForFeature(f, context)
        self.assertFalse(keys)

        renderer.stopRender(context)

    def testMatchToSymbols(self):
        """
        Test QgsCategorizedSymbolRender.matchToSymbols
        """
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('x')

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 0, 0))
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        symbol_b.setColor(QColor(0, 255, 0))
        renderer.addCategory(QgsRendererCategory('b', symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        symbol_c.setColor(QColor(0, 0, 255))
        renderer.addCategory(QgsRendererCategory('c ', symbol_c, 'c'))

        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(None, QgsSymbol.Marker)
        self.assertEqual(matched, 0)

        style = QgsStyle()
        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol('a', symbol_a))
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('B ', symbol_B))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('b', symbol_b))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('C', symbol_C))
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol(' ----c/- ', symbol_C))

        # non-matching symbol type
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Line)
        self.assertEqual(matched, 0)
        self.assertEqual(unmatched_cats, ['a', 'b', 'c '])
        self.assertEqual(unmatched_symbols, [' ----c/- ', 'B ', 'C', 'a', 'b'])

        # exact match
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Marker)
        self.assertEqual(matched, 1)
        self.assertEqual(unmatched_cats, ['b', 'c '])
        self.assertEqual(unmatched_symbols, [' ----c/- ', 'B ', 'C', 'b'])

        # make sure symbol was applied
        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())
        symbol, ok = renderer.symbolForValue2('a')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#ff0a0a')
        renderer.stopRender(context)

        # case insensitive match
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Marker, False)
        self.assertEqual(matched, 2)
        self.assertEqual(unmatched_cats, ['c '])
        self.assertEqual(unmatched_symbols, [' ----c/- ', 'C', 'b'])

        # make sure symbols were applied
        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())
        symbol, ok = renderer.symbolForValue2('a')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#ff0a0a')
        symbol, ok = renderer.symbolForValue2('b')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#0aff0a')
        renderer.stopRender(context)

        # case insensitive match
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Marker, False)
        self.assertEqual(matched, 2)
        self.assertEqual(unmatched_cats, ['c '])
        self.assertEqual(unmatched_symbols, [' ----c/- ', 'C', 'b'])

        # make sure symbols were applied
        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())
        symbol, ok = renderer.symbolForValue2('a')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#ff0a0a')
        symbol, ok = renderer.symbolForValue2('b')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#0aff0a')
        renderer.stopRender(context)

        # tolerant match
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Marker, True, True)
        self.assertEqual(matched, 2)
        self.assertEqual(unmatched_cats, ['b'])
        self.assertEqual(unmatched_symbols, ['B ', 'C', 'b'])

        # make sure symbols were applied
        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())
        symbol, ok = renderer.symbolForValue2('a')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#ff0a0a')
        symbol, ok = renderer.symbolForValue2('c ')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#0aff0a')
        renderer.stopRender(context)

        # tolerant match, case insensitive
        matched, unmatched_cats, unmatched_symbols = renderer.matchToSymbols(style, QgsSymbol.Marker, False, True)
        self.assertEqual(matched, 3)
        self.assertFalse(unmatched_cats)
        self.assertEqual(unmatched_symbols, ['C', 'b'])

        # make sure symbols were applied
        context = QgsRenderContext()
        renderer.startRender(context, QgsFields())
        symbol, ok = renderer.symbolForValue2('a')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#ff0a0a')
        symbol, ok = renderer.symbolForValue2('b')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#0aff0a')
        symbol, ok = renderer.symbolForValue2('c ')
        self.assertTrue(ok)
        self.assertEqual(symbol.color().name(), '#0aff0a')
        renderer.stopRender(context)

    def testUsedAttributes(self):
        renderer = QgsCategorizedSymbolRenderer()
        ctx = QgsRenderContext()

        # attribute can contain either attribute name or an expression.
        # Sometimes it is not possible to distinguish between those two,
        # e.g. "a - b" can be both a valid attribute name or expression.
        # Since we do not have access to fields here, the method should return both options.
        renderer.setClassAttribute("value")
        self.assertEqual(renderer.usedAttributes(ctx), {"value"})
        renderer.setClassAttribute("value - 1")
        self.assertEqual(renderer.usedAttributes(ctx), {"value", "value - 1"})
        renderer.setClassAttribute("valuea - valueb")
        self.assertEqual(renderer.usedAttributes(ctx), {"valuea", "valuea - valueb", "valueb"})

    def testPointsUsedAttributes(self):
        points_shp = os.path.join(TEST_DATA_DIR, 'points.shp')
        points_layer = QgsVectorLayer(points_shp, 'Points', 'ogr')
        QgsProject.instance().addMapLayer(points_layer)

        cats = []
        sym1 = QgsMarkerSymbol()
        l1 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l1.setColor(QColor(255, 0, 0))
        l1.setStrokeStyle(Qt.NoPen)
        l1.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym1.changeSymbolLayer(0, l1)
        cats.append(QgsRendererCategory("B52", sym1, "B52"))
        sym2 = QgsMarkerSymbol()
        l2 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l2.setColor(QColor(0, 255, 0))
        l2.setStrokeStyle(Qt.NoPen)
        l2.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym2.changeSymbolLayer(0, l2)
        cats.append(QgsRendererCategory("Biplane", sym2, "Biplane"))
        sym3 = QgsMarkerSymbol()
        l3 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l3.setColor(QColor(0, 0, 255))
        l3.setStrokeStyle(Qt.NoPen)
        l3.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym3.changeSymbolLayer(0, l3)
        cats.append(QgsRendererCategory("Jet", sym3, "Jet"))

        renderer = QgsCategorizedSymbolRenderer("Class", cats)

        points_layer.setRenderer(renderer)

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([points_layer])

        ctx = QgsRenderContext.fromMapSettings(ms)
        ctx.expressionContext().appendScope(points_layer.createExpressionContextScope())

        # for symbol layer
        self.assertCountEqual(l1.usedAttributes(ctx), {'Heading'})
        # for symbol
        self.assertCountEqual(sym1.usedAttributes(ctx), {'Heading'})
        # for symbol renderer
        self.assertCountEqual(renderer.usedAttributes(ctx), {'Class', 'Heading'})

        QgsProject.instance().removeMapLayer(points_layer)

    def testFilterNeedsGeometry(self):
        renderer = QgsCategorizedSymbolRenderer()

        renderer.setClassAttribute("value")
        self.assertFalse(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("$area")
        self.assertTrue(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("value - $area")
        self.assertTrue(renderer.filterNeedsGeometry())

    def testCategories(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory")
        layer.setEditorWidgetSetup(1, QgsEditorWidgetSetup("ValueMap", {'map': [{'One': '1'}, {'Two': '2'}]}))

        result = QgsCategorizedSymbolRenderer.createCategories([1, 2, 3], QgsMarkerSymbol(), layer, 'fldint')

        self.assertEqual(result[0].label(), 'One')
        self.assertEqual(result[1].label(), 'Two')
        self.assertEqual(result[2].label(), '(3)')

    def testWriteReadXml(self):
        # test writing renderer to xml and restoring

        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('x')

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 0, 0))
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        symbol_b.setColor(QColor(0, 255, 0))
        renderer.addCategory(QgsRendererCategory('b', symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        symbol_c.setColor(QColor(0, 0, 255))
        renderer.addCategory(QgsRendererCategory('c', symbol_c, 'c', False))
        symbol_d = createMarkerSymbol()
        symbol_d.setColor(QColor(255, 0, 255))
        renderer.addCategory(QgsRendererCategory(['d', 'e'], symbol_d, 'de'))
        # add default category
        default_symbol = createMarkerSymbol()
        default_symbol.setColor(QColor(255, 255, 255))
        renderer.addCategory(QgsRendererCategory('', default_symbol, 'default'))

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        renderer2 = QgsCategorizedSymbolRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(renderer2.classAttribute(), 'x')
        self.assertEqual([l.label() for l in renderer2.categories()], ['a', 'b', 'c', 'de', 'default'])
        self.assertEqual([l.value() for l in renderer2.categories()], ['a', 'b', 'c', ['d', 'e'], ''])
        self.assertEqual([l.symbol().color().name() for l in renderer2.categories()],
                         ['#ff0000', '#00ff00', '#0000ff', '#ff00ff', '#ffffff'])

    def testConvertFromEmbedded(self):
        """
        Test converting an embedded symbol renderer to a categorized renderer
        """
        points_layer = QgsVectorLayer('Point', 'Polys', 'memory')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('Point(-100 30)'))
        f.setEmbeddedSymbol(
            QgsMarkerSymbol.createSimple({'name': 'triangle', 'size': 10, 'color': '#ff0000', 'outline_style': 'no'}))
        self.assertTrue(points_layer.dataProvider().addFeature(f))
        f.setGeometry(QgsGeometry.fromWkt('Point(-110 40)'))
        f.setEmbeddedSymbol(
            QgsMarkerSymbol.createSimple({'name': 'square', 'size': 7, 'color': '#00ff00', 'outline_style': 'no'}))
        self.assertTrue(points_layer.dataProvider().addFeature(f))
        f.setGeometry(QgsGeometry.fromWkt('Point(-90 50)'))
        f.setEmbeddedSymbol(None)
        self.assertTrue(points_layer.dataProvider().addFeature(f))

        renderer = QgsEmbeddedSymbolRenderer(defaultSymbol=QgsMarkerSymbol.createSimple({'name': 'star', 'size': 10, 'color': '#ff00ff', 'outline_style': 'no'}))
        points_layer.setRenderer(renderer)

        categorized = QgsCategorizedSymbolRenderer.convertFromRenderer(renderer, points_layer)
        self.assertEqual(categorized.classAttribute(), '$id')
        self.assertEqual(len(categorized.categories()), 3)
        cc = categorized.categories()[0]
        self.assertEqual(cc.value(), 1)
        self.assertEqual(cc.label(), '1')
        self.assertEqual(cc.symbol().color().name(), '#ff0000')
        cc = categorized.categories()[1]
        self.assertEqual(cc.value(), 2)
        self.assertEqual(cc.label(), '2')
        self.assertEqual(cc.symbol().color().name(), '#00ff00')
        cc = categorized.categories()[2]
        self.assertEqual(cc.value(), None)
        self.assertEqual(cc.label(), '')
        self.assertEqual(cc.symbol().color().name(), '#ff00ff')

    def test_displayString(self):
        """Test the displayString method"""

        # Default locale for tests is EN
        original_locale = QLocale()
        locale = QLocale(QLocale.English)
        locale.setNumberOptions(QLocale.DefaultNumberOptions)
        QLocale().setDefault(locale)

        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234.56), "1,234.56")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234.56, 4), "1,234.5600")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567), "1,234,567")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567.0, 4), "1,234,567.0000")
        # Precision is ignored for integers
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567, 4), "1,234,567")

        # Test list
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567, 891234], 4), "1,234,567;891,234")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567.123, 891234.123], 4), "1,234,567.1230;891,234.1230")

        locale.setNumberOptions(QLocale.OmitGroupSeparator)
        QLocale().setDefault(locale)
        self.assertTrue(QLocale().numberOptions() & QLocale.OmitGroupSeparator)
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567, 891234], 4), "1234567;891234")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567.123, 891234.123], 4), "1234567.1230;891234.1230")

        # Test a non-dot locale
        locale = QLocale(QLocale.Italian)
        locale.setNumberOptions(QLocale.DefaultNumberOptions)
        QLocale().setDefault(locale)
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234.56), "1.234,56")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234.56, 4), "1.234,5600")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567), "1.234.567")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567.0, 4), "1.234.567,0000")
        # Precision is ignored for integers
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString(1234567, 4), "1.234.567")

        # Test list
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567, 891234], 4), "1.234.567;891.234")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567.123, 891234.123], 4), "1.234.567,1230;891.234,1230")

        locale.setNumberOptions(QLocale.OmitGroupSeparator)
        QLocale().setDefault(locale)
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567, 891234], 4), "1234567;891234")
        self.assertEqual(QgsCategorizedSymbolRenderer.displayString([1234567.123, 891234.123], 4), "1234567,1230;891234,1230")

        QLocale().setDefault(original_locale)

    def test_localizedCategories(self):

        # Default locale for tests is EN
        original_locale = QLocale()
        locale = QLocale(QLocale.English)
        locale.setNumberOptions(QLocale.DefaultNumberOptions)
        QLocale().setDefault(locale)

        layer = QgsVectorLayer("Point?field=flddbl:double&field=fldint:integer", "addfeat", "memory")
        result = QgsCategorizedSymbolRenderer.createCategories([1234.5, 2345.6, 3456.7], QgsMarkerSymbol(), layer, 'flddouble')

        self.assertEqual(result[0].label(), '1,234.5')
        self.assertEqual(result[1].label(), '2,345.6')
        self.assertEqual(result[2].label(), '3,456.7')

        # Test a non-dot locale
        QLocale().setDefault(QLocale(QLocale.Italian))

        result = QgsCategorizedSymbolRenderer.createCategories([[1234.5, 6789.1], 2345.6, 3456.7], QgsMarkerSymbol(), layer, 'flddouble')

        self.assertEqual(result[0].label(), '1.234,5;6.789,1')
        self.assertEqual(result[1].label(), '2.345,6')
        self.assertEqual(result[2].label(), '3.456,7')

        # Test round trip
        temp_dir = QTemporaryDir()
        temp_file = os.path.join(temp_dir.path(), 'project.qgs')

        project = QgsProject()
        layer.setRenderer(QgsCategorizedSymbolRenderer('Class', result))
        project.addMapLayers([layer])
        project.write(temp_file)

        QLocale().setDefault(original_locale)

        project = QgsProject()
        project.read(temp_file)
        results = project.mapLayersByName('addfeat')[0].renderer().categories()

        self.assertEqual(result[0].label(), '1.234,5;6.789,1')
        self.assertEqual(result[1].label(), '2.345,6')
        self.assertEqual(result[2].label(), '3.456,7')
        self.assertEqual(result[0].value(), [1234.5, 6789.1])
        self.assertEqual(result[1].value(), 2345.6)
        self.assertEqual(result[2].value(), 3456.7)

    def test_legend_key_to_expression(self):
        renderer = QgsCategorizedSymbolRenderer()
        renderer.setClassAttribute('field_name')

        exp, ok = renderer.legendKeyToExpression('xxxx', None)
        self.assertFalse(ok)

        # no categories
        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertFalse(ok)

        symbol_a = createMarkerSymbol()
        renderer.addCategory(QgsRendererCategory('a', symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        renderer.addCategory(QgsRendererCategory(5, symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        renderer.addCategory(QgsRendererCategory(5.5, symbol_c, 'c', False))
        symbol_d = createMarkerSymbol()
        renderer.addCategory(QgsRendererCategory(['d', 'e'], symbol_d, 'de'))

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "field_name = 'a'")

        exp, ok = renderer.legendKeyToExpression('1', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "field_name = 5")

        exp, ok = renderer.legendKeyToExpression('2', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "field_name = 5.5")

        exp, ok = renderer.legendKeyToExpression('3', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "field_name IN ('d', 'e')")

        layer = QgsVectorLayer("Point?field=field_name:double&field=fldint:integer", "addfeat", "memory")
        # with layer
        exp, ok = renderer.legendKeyToExpression('3', layer)
        self.assertTrue(ok)
        self.assertEqual(exp, "\"field_name\" IN ('d', 'e')")

        # with expression as attribute
        renderer.setClassAttribute('upper("field_name")')

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """upper("field_name") = 'a'""")

        exp, ok = renderer.legendKeyToExpression('1', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """upper("field_name") = 5""")

        exp, ok = renderer.legendKeyToExpression('2', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """upper("field_name") = 5.5""")

        exp, ok = renderer.legendKeyToExpression('3', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """upper("field_name") IN ('d', 'e')""")

        exp, ok = renderer.legendKeyToExpression('3', layer)
        self.assertTrue(ok)
        self.assertEqual(exp, """upper("field_name") IN ('d', 'e')""")


if __name__ == "__main__":
    unittest.main()
