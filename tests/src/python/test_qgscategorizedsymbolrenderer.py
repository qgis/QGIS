# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCategorizedSymbolRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

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
                       QgsStyle
                       )
from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QColor

start_app()


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

        # hidden category
        f.setAttributes(['c'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertIsNone(symbol)

        # no matching category
        f.setAttributes(['xxx'])
        symbol = renderer.originalSymbolForFeature(f, context)
        self.assertEqual(symbol.color(), QColor(255, 255, 255)) # default symbol

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
        # add default category
        default_symbol = createMarkerSymbol()
        default_symbol.setColor(QColor(255, 255, 255))
        renderer.addCategory(QgsRendererCategory('', default_symbol, 'default'))

        context = QgsRenderContext()
        context.setRendererScale(0) # simulate counting
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

    def testFilterNeedsGeometry(self):
        renderer = QgsCategorizedSymbolRenderer()

        renderer.setClassAttribute("value")
        self.assertFalse(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("$area")
        self.assertTrue(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("value - $area")
        self.assertTrue(renderer.filterNeedsGeometry())


if __name__ == "__main__":
    unittest.main()
