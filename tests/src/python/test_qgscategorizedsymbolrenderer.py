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
                       QgsField,
                       QgsFields,
                       QgsFeature,
                       QgsRenderContext
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


if __name__ == "__main__":
    unittest.main()
